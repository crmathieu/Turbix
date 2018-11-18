/* MKD : gestion de la souris */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "signal.h"
#include "floppy.h"
#include "console.h"
#include "shell.h"

#include "const.h"
#include "stat.h"

#include "setjmp.h"
#include "ipc.h"
#include "msg.h"

/**
 **    FONCTIONNEMENT INTERNE
 **
 **/

struct WORDREGS {
        unsigned int    ax, bx, cx, dx, si, di, cflag, flags;
};

struct BYTEREGS {
        unsigned char   al, ah, bl, bh, cl, ch, dl, dh;
};

union   REGS    {
        struct  WORDREGS x;
        struct  BYTEREGS h;
};

struct  SREGS   {
        unsigned int    es;
        unsigned int    cs;
        unsigned int    ss;
        unsigned int    ds;
};

struct  REGPACK {
        unsigned        r_ax, r_bx, r_cx, r_dx;
        unsigned        r_bp, r_si, r_di, r_ds, r_es, r_flags;
};

/* Definition de constantes "TYPES D'ACTIONS":
 * on utilise les 2 bits de poids forts.
 * le reste des bits sert … d‚finir le masque
 * d'‚vŠnements dans laction MOU_MASK
 */
#define MOU_GET      0x00
#define MOU_SHOW     0x01
#define MOU_HIDE     0x02
#define MOU_OPEN     0x03
#define MOU_CLOSE    0x04
#define MOU_VIOSAVE  0x05
#define MOU_VIORST   0x06

#define MOU_MASK     0xC0

#define TEST_ACTION  0xC0
#define GET_MASK     0x3F

/* Type d'Evenement arriv‚ */
#define  LEFT_BUTTON_DOWN   1
#define  RIGHT_BUTTON_DOWN  2
#define  CENTER_BUTTON_DOWN 4
#define  MOUSE_MOVE         8

/* definitions des valeurs de declenchement de l'IT souris */
#define  M_REPOS           0
#define  M_MOVE            1     /* bit 1 */
#define  M_LEFT_PRESS      2
#define  M_LEFT_RELEASE    4
#define  M_RIGHT_PRESS     8
#define  M_RIGHT_RELEASE  16
#define  M_MIDDL_PRESS    32
#define  M_MIDDL_RELEASE  64


int mouseEvent;         /* nombre d'evenement souris */
int mouse_present;
struct mouEvent ib;
extern int currP;
SEM MouSem;  /* semaphore evenements souris */


/*---------------------------------------------------------------------------
 * it_mouse -
 *---------------------------------------------------------------------------
 */
it_mouse()
{
    /* envouyer message au demon souris */
    msgsync(MOU_TASK, MOU_GET); /* indiquer recuperation au demon */
}

/*---------------------------------------------------------------------------
 * _mou_task - demon de recuperation des messages souris
 *---------------------------------------------------------------------------
 */
TASK _mou_task()
{
   union  REGS  rin, rout;
   uchar mess;
   int    stat, j, i, nbutton, Min, ps;
   unsigned Page;
   struct tty *ptty;
   extern ulong msdosv_mou;
   ulong aux;
   extern int prolog_it_mouse();
   extern interrupt _dummy_interrupt();
   extern int (far *_getvector());

       /*--------------------------------------------------------------
        * m_callMaskAndAddress - chaine … l'IT souris, le traitement
        *                       utilisateur a appeler si les conditions
        *                       exprim‚es dans le masque sont reunies:
        *                              - bit 0 : changement de position
        *                              - bit 1 : bouton Gauche press‚
        *                              - bit 2 : bouton Gauche relach‚
        *                              - bit 3 : bouton Droit press‚
        *                              - bit 4 : bouton Droit relach‚
        *                              - bit 5 : bouton centre press‚
        *                              - bit 6 : bouton centre relach‚
        *--------------------------------------------------------------
        */

   /* check mouse */
   mouse_present = FALSE;
   msdosv_mou = _getvector(0x33);
   if (msdosv_mou != 0 && (*(uchar far *)msdosv_mou != 0xCF)) {
        /* Driver install‚ :
         *        tester la presence de la souris
         */
        if (!_m_installed(&nbutton)) {
                /* pas de souris, installer
                 * l'interruption DUMMY
                 */
                _sasVector(0x33, _dummy_interrupt, &msdosv_mou);
                /*printf("Souris non connect‚e ...\n");*/
        }
        else {
                /*printf("SOURIS CONNECTEE\n");*/
                mouse_present = TRUE;
       }
   }
/*   else
        printf("Driver non install‚ \n");*/

   if (!mouse_present)
        while (TRUE)
               /* attente bidon */
               msgwait(&mess, -1, &stat);

   disable();

   while (TRUE) {



        /* attendre message du driver */
        msgwait(&mess, -1, &stat);

        ptty = &tty[currP];
        if (mess & TEST_ACTION) /* MOU_MASK (0b11xx xxxx) */
                _m_callMaskAndAddress((mess & GET_MASK), prolog_it_mouse);
        else
        switch (mess) {
        case MOU_GET :
                /* placer les coordonnees souris
                 * dans la session courante
                 */
                ptty->mouse_state &= ~MOUSE_MOVE;
                if (ptty->mouse_state == (ib.event & ptty->testMask)) { /*& 0x07)) {*/
                        if (ptty->mask & M_MOVE)
                                ptty->mouse_state |= MOUSE_MOVE;
                        else
                                break;
                }
                else
                        ptty->mouse_state = ib.event & ptty->testMask; /* & 0x07; */

                if (ptty->mouActiv) {
                        ptty->MouEvent++;
                        Min = ptty->Min;
                        ptty->MouBuf[Min].event = ptty->mouse_state;
                        ptty->MouBuf[Min].mouX  = ib.mouX / 8;
                        ptty->MouBuf[Min].mouY  = ib.mouY / 8;
                        ptty->Min = (ptty->Min + 1) & 0x0f;
                        /* mettre a jour l'etat de la queue d'evenement */
                        if (ptty->MouEvent > 16) {
                                ptty->MouEvent--;
                                ptty->Mout = (ptty->Mout + 1) & 0x0f;
                        }
                        sigsem(ptty->MouSem);  /* liberer tƒche en attente d'EV */
                }
                break;
        case MOU_SHOW:
                _showMouse();break;
        case MOU_HIDE:
                _hideMouse();break;
        case MOU_OPEN:
                _m_installed(&nbutton);  /* reset souris */
                _m_callMaskAndAddress(ptty->mask, prolog_it_mouse);
                break;

        case MOU_CLOSE:
                _m_installed(&nbutton);  /* reset souris */
                break;
        default:
                Page = mess >> 8;
                mess &= 0xf;
                ptty = &tty[Page];
                if (mess == MOU_VIOSAVE) {
                        _m_getPos(&ptty->mouX, &ptty->mouY);
                        _m_installed(&nbutton);  /* reset souris */
                }
                else
                        if (mess == MOU_VIORST) {
                                _m_installed(&i);  /* reset souris */
                                _m_setPos(ptty->mouX, ptty->mouY);
                                j = ptty->mouCpt;
/*                              if (j >= 0)
                                        for (i=-1; i<= j; i++)
                                                _showMouse();
                                else
                                        for (i=0; i>= j; i--)
                                                _hideMouse();*/

                                if (j >= 0)
                                        _showMouse();
                                _m_callMaskAndAddress(ptty->mask, prolog_it_mouse);
                }
                break;
        }

   }
}
/*--------------------------------------------------------------
 * m_installed - teste si le driver souris est present. si oui,
 *               retourne le nombre de boutons
 *--------------------------------------------------------------
 */
_m_installed(nbutton)
unsigned *nbutton;
{
 union  REGS  rin,rout;

 rin.x.ax  = 0;
 int86(0x33, &rin, &rout);
 *nbutton = rout.x.bx;
 return(rout.x.ax);
}

/*--------------------------------------------------------------
 * showMouse -
 *
 *--------------------------------------------------------------
 */
_showMouse()
{
 union  REGS  rin,rout;

 rin.x.ax = 01;
 int86(0x33, &rin, &rout);
}

/*--------------------------------------------------------------
 * hideMouse -
 *
 *--------------------------------------------------------------
 */
_hideMouse()
{
 union  REGS  rin,rout;

 rin.x.ax = 0x02;
 int86(0x33, &rin, &rout);
}



/*--------------------------------------------------------------
 * m_callMaskAndAddress - chaine … l'IT souris, le traitement
 *                       utilisateur a appeler si les conditions
 *                       exprim‚es dans le masque sont reunies:
 *                              - bit 0 : changement de position
 *                              - bit 1 : bouton Gauche press‚
 *                              - bit 2 : bouton Gauche relach‚
 *                              - bit 3 : bouton Droit press‚
 *                              - bit 4 : bouton Droit relach‚
 *--------------------------------------------------------------
 */
_m_callMaskAndAddress(callMask, subroutine)
unsigned callMask;
int (far * subroutine)();
{
 union  REGS  rin,rout;
 struct SREGS segreg;


        rin.x.ax  = 0x0c;
        rin.x.cx  = callMask;
        rin.x.dx  = FP_OFF(subroutine) ;
        segreg.es = FP_SEG(subroutine);
        int86x(0x33, &rin, &rout,&segreg);

}
/*--------------------------------------------------------------
 * m_getPos                - retourne la position et le status:
 *                        le bit 0 represente celui de droite
 *                        le bit 1 represente celui de gauche
 *                        Pour Chacun des 2 bits :
 *                        si … 1 -> enfonc‚,   0 -> relach‚
 *--------------------------------------------------------------
 */
_m_getPos(posX, posY)
int *posX, *posY;
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x03;
        int86(0x33, &rin, &rout);
        *posX = rout.x.cx;
        *posY = rout.x.dx;
}

/*--------------------------------------------------------------
 * m_setPos -
 *
 *--------------------------------------------------------------
 */
_m_setPos(newPosX, newPosY)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x04;
        rin.x.cx  = newPosX;
        rin.x.dx  = newPosY;
        int86(0x33, &rin, &rout);

}


/**
 **    BIBLIOTHEQUE SOURIS
 **
 **/

/*----------------------------------------------------------------------------
 * m_OpenMou - initialiser l'utilisation de la souris dans la session
 *              ou s'execute le programme
 *----------------------------------------------------------------------------
 */
BIBLIO m_OpenMou()
{
    int i;
    struct tty *ptty;


    if (!mouse_present)
        return(-1);

    if ((ptty = &tty[Tasktab[RUNpid].tgrp])->mouActiv == 0) {
        ptty->mask = M_LEFT_PRESS|M_LEFT_RELEASE|M_RIGHT_PRESS|M_RIGHT_RELEASE/*|M_MIDDL_PRESS|M_MIDDL_RELEASE*/|M_MOVE;
        setCheckMsk(ptty);
        _mouEnqueue(MOU_OPEN);
    }
    ptty->mouActiv++;
}

/*----------------------------------------------------------------------------
 * m_CloseMou - invalider l'utilisation de la souris dans la session
 *               ou s'execute le programme
 *----------------------------------------------------------------------------
 */
BIBLIO m_CloseMou()
{
    struct tty *ptty;
    int i;

    if (!mouse_present)
        return(-1);

    if ((ptty = &tty[Tasktab[RUNpid].tgrp])->mouActiv > 0)
        ptty->mouActiv--;
    else
        _mouEnqueue(MOU_CLOSE);
    return(0);
}


/*----------------------------------------------------------------------------
 * m_ShowMou
 *----------------------------------------------------------------------------
 */
BIBLIO m_ShowMou()
{
    extern SEM bsem;
    struct biosreq hbuff;
    struct tty *ptty;
    int ps,i,stat;
    uchar null;

    if (!mouse_present)
        return(-1);

    ps = disable();
    if ((ptty = &tty[Tasktab[RUNpid].tgrp])->mouCpt >= 0) {
        ptty->mouCpt++;
        restore(ps);
        return(0);
    }
    ptty->mouCpt++;

    _mouEnqueue(MOU_SHOW);
    restore(ps);
    return(0);
}
_mouEnqueue(nofunc)
unsigned nofunc;
{
    extern SEM bsem;
    struct biosreq hbuff;
    struct tty *ptty;
    int ps,i,stat;
    uchar null;

    hbuff.b_pid    = RUNpid;
    hbuff.b_func   = MOU_SRV;
    hbuff.m_nofunc = nofunc;
    _biosenq(&hbuff);
    msgclr();
    sigsem(bsem);
    msgwait(&null, -1, &stat);
}

/*----------------------------------------------------------------------------
 * m_HideMou
 *----------------------------------------------------------------------------
 */
BIBLIO m_HideMou()
{
    extern SEM bsem;
    struct biosreq hbuff;
    int ps,i,stat;
    uchar null;
    struct tty *ptty;


    if (!mouse_present)
        return(-1);

    ps = disable();
    if ((ptty = &tty[Tasktab[RUNpid].tgrp])->mouCpt < 0) {
        ptty->mouCpt--;
        restore(ps);
        return(0);
    }
    ptty->mouCpt--;

    _mouEnqueue(MOU_HIDE);
    restore(ps);
    return(0);
}

/*----------------------------------------------------------------------------
 * m_ReadEventMou
 *----------------------------------------------------------------------------
 */
BIBLIO m_ReadEventMou(pMouEvent, fwait)
struct mouEvent *pMouEvent;
{
    struct tty *ptty;
    int ps,i,stat;
    uchar null;

    if (!mouse_present)
        return(-1);

    ps = disable();
    if ((ptty = &tty[Tasktab[RUNpid].tgrp])->mouActiv) {
        if (fwait == FALSE) {
            /* tester si il y a un ‚vŠnement PRET */
            countsem(ptty->MouSem, &i);
            if (i <= 0) { /* pas d'ev */
                restore(ps);
                return(-1);
            }
        }
        waitsem(ptty->MouSem);
        ptty->MouEvent--;
        memcpy(pMouEvent, &ptty->MouBuf[ptty->Mout], sizeof(struct mouEvent));
        ptty->Mout = (ptty->Mout + 1) & 0x0f;
        restore(ps);
        return(0);
    }
    restore(ps);
    return(-1);
}

/*----------------------------------------------------------------------------
 * m_MaskEventMou
 *----------------------------------------------------------------------------
 */
BIBLIO m_EventMaskMou(mask)
unsigned mask;
{
    struct tty *ptty;
    int ps,i,stat;
    uchar null;

    if (!mouse_present)
        return(-1);
    ps = disable();
    if ((ptty = &tty[Tasktab[RUNpid].tgrp])->mouActiv) {
        ptty->mask = mask;
        setCheckMsk(ptty);
        _mouEnqueue(MOU_MASK|mask);
        restore(ps);
        return(0);
    }
    restore(ps);
    return(-1);
}

setCheckMsk(ptty)
struct tty *ptty;
{
        ptty->testMask = 0;
        if (ptty->mask & 0x06){
                ptty->testMask |= 1;   /* bouton gauche */
               /* printf("* B GAUCHE *");*/
        }
        if (ptty->mask & 0x18) {
                ptty->testMask |= 2;   /* bouton droit  */
                /* printf("* B DROIT *");*/
        }
        if (ptty->mask & 0x60) {
                ptty->testMask |= 4;   /* bouton centre */
                /* printf("* B CENTRE *");*/
        }
        if (ptty->mask & 0x01) {
                ptty->testMask |= 8;   /* move */
                /* printf("* MOVE *"); */
        }
        /*printf("__%x__", ptty->testMask);*/
}