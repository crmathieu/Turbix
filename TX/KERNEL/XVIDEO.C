/* xvideo.c - procedures utilisateurs de gestion video */

#include "sys.h"
#include "conf.h"
#include "tty.h"
#include "window.h"
#include "io.h"
#include "const.h"
#include "console.h"

#include "fsopen.h"
#include "dynlink.h"

/* pointeur sur tableau de localisation des pages en memoire vive */
/*int *VStab;*/

LOCAL uchar txtreg[] = {0x61,0x50,0x52,0x0F,0x19,0x06,0x19,0x19,0x02,0x0D,
                        0x0B,0x0C,0x08,0x00};

LOCAL uchar grpreg[] = {0x35,0x2D,0x2E,0x07,0x5B,0x02,0x57,0x57,0x02,0x03,
                        0x00,0x00,0x00,0x00};

int activtxtP        = VS0;
extern int currP;

LOCAL unsigned char *p1 = GRAFIC1;
LOCAL int display       = TEXTE;

LOCAL int far *ptrcolor = (int far *)0xb8000000;
LOCAL int far *ptrmono  = (int far *)0xb0000000;

char sInputName[] = "Input ";

extern int      adapter;
extern unsigned vid_base;

/* MACROS */

#define isnum(c)      (c < 0x40 && c > 0x29)
#define isalpha(c)    (c < 0x7B && c > 0x40)


#define  M_MOVMENT         1
#define  M_LEFT_PRESS      2
#define  M_LEFT_RELEASE    4
#define  M_RIGHT_PRESS     8
#define  M_RIGHT_RELEASE  16
#define  M_MIDDL_PRESS    32
#define  M_MIDDL_RELEASE  64

#define MOU_GET      0x00
#define MOU_SHOW     0x01
#define MOU_HIDE     0x02
#define MOU_OPEN     0x03
#define MOU_CLOSE    0x04
#define MOU_VIOSAVE  0x05
#define MOU_VIORST   0x06


/*----------------------------------------------------------------------------
 * init_vs - initialise un ecran virtuel
 *----------------------------------------------------------------------------
 */
_init_vs(ptty)
struct tty *ptty;
{
   char *vs;
   int i;
   static char vsNum = '\x30';

   /* allouer un buffer de sauvegarde pour l'ecran */
   if (((int *)ptty->vsbuf = _XSmalloc(4000)) == (int *)NULL)
   {
       m_Printf(errIniSessStr);
       m_Exit(0);
   }
   ptty->vsinit = TRUE;
   sInputName[strlen(sInputName)-1] = vsNum;
   vsNum++;
   ptty->isem   = m_Creatsem(0, sInputName);
/*   ptty->MouSem = m_Creatsem(0,"_mtsks");*/
/*   ptty->tlock  = m_Creatsem(1,"LockINP");  /* sem d'exclusion mutuelle */


   /* init contenu de la page */
   for (i=0;i<2000;i++)
      *(ptty->vsbuf+i) = ptty->cvatt<<8;

}


/*----------------------------------------------------------------------------
 * m_Gotoxy - place le curseur au point (x,y)
 *----------------------------------------------------------------------------
 */
BIBLIO m_Gotoxy( x , y)
int x,y;
{
      stream_entry   *sp;

      sp = Tasktab[RUNpid].tfd[stdout];
      if (sp->s_streamtyp != FILESTREAM)
          if (sp->s_minor < TTY1)
              _moveCursor(&tty[sp->s_minor],x,y);
          else
              m_Printf("\033[%d;%dH",y,x);
}

/*----------------------------------------------------------------------------
 * m_Getpos -  donne la position courante du curseur
 *----------------------------------------------------------------------------
 */
BIBLIO m_Getpos( x , y)
int *x,*y;
{
      stream_entry   *sp;

      sp = Tasktab[RUNpid].tfd[stdout];
      *x = tty[sp->s_minor].cursX;
      *y = tty[sp->s_minor].cursY;
}


/*----------------------------------------------------------------------------
 * m_Setcva - set current video attribut
 *----------------------------------------------------------------------------
 */
BIBLIO m_Setcva(newva)
uchar newva;        /* new video attribut */
{

     stream_entry   *sp;

     if ((sp = Tasktab[RUNpid].tfd[stdout]) == NULLSTREAM) return(RERR);
     tty[sp->s_minor].cvatt = newva;
     return(ROK);
}

/*----------------------------------------------------------------------------
 * _set_CurrP - change de page courante en mode MONO ou COULEUR
 *----------------------------------------------------------------------------
 */
_set_CurrP( newpage )
int newpage;
{
   extern int prolog_it_mouse();
   int i, j, ps;
   struct tty *ptty;
   struct window *pwin;

/*   ps = _disable();  /* debut de section critique */
/*   if (adapter == MONO)
   {
       if (display == GRAFIC)
          set_Mdisplay(TEXTE);
   }
   else
   {
      if (display == GRAFIC)
          set_Cdisplay(TEXTE);
   }*/

   if ((activtxtP == newpage) || (tty[newpage].vsinit == FALSE))
       return(activtxtP);

    /* si la souris etait initialisee pour cette session
     * marquer sa position
     */
/*    if (tty[activtxtP].mouActiv > 0) {
        /* envouyer message au demon souris */
/*        msgsndI(MOU_TASK, MOU_VIOSAVE, FALSE);*/

/*        _m_getPos(&tty[activtxtP].mouX, &tty[activtxtP].mouY);
        _m_installed(&i);  /* reset souris */
    /*}*/

   _flush(&tty[newpage]);
   _flush(&tty[activtxtP]);

   /* si mode window initialis‚, ne pas sauvegarder l'ecran actif */
   _swap_page(FP_SEG(tty[newpage].vsbuf),
             FP_SEG(tty[activtxtP].vsbuf),
             tty[activtxtP].wmode);

   /* si l'ancienne page n'est plus valide, liberer son buffer d'ecran */
   if (tty[activtxtP].vsinit == FALSE)
       _xfree(tty[activtxtP].vsbuf, RUNpid);


   ptty       = &tty[newpage];
   activtxtP  = newpage;

   /* mettre a jour offset d'ecriture en page 0 et curseur */
   _moveCursor(ptty,ptty->cursX,ptty->cursY);
   _doCursorShape(ptty->curs_shape);

    /* si la souris etait initialisee pour cette session
     * marquer sa position
     */
/*    if (tty[activtxtP].mouActiv > 0) {
        _m_installed(&i);  /* reset souris */
/*        _m_setPos(tty[activtxtP].mouX, tty[activtxtP].mouY);
        j = tty[activtxtP].mouCpt;
        if (j >= 0)
                for (i=-1; i<= j; i++)
                        _showMouse();
        else
                for (i=0; i>= j; i--)
                        _hideMouse();

        _m_callMaskAndAddress(tty[activtxtP].mask, prolog_it_mouse);
        /* envouyer message au demon souris */
/*        msgsndI(MOU_TASK, MOU_VIORST, FALSE);

    }*/

   /* si mode window, rafraichir toutes les windows sauf stdscr(qui l'a
    * deja ete lors du swap
    */

   if (ptty->wmode)
       for (pwin = ptty->curwin; pwin != ptty->pwin; pwin = pwin->_WLOWER) {
            (*winfunc[WIN_BOX])(pwin, 0, 0, NO_CHECK, activtxtP);
            (*winfunc[WIN_TOUCH])(pwin, NO_CHECK, activtxtP);
       }

/*   _restore(ps);*/
   return(newpage);
}


/*----------------------------------------------------------------------------
 * m_CursorShape - modifie l'aspect du curseur si l'output est la sortie active
 *               si non change l'attribut de forme dans le slot tty
 *----------------------------------------------------------------------------
 */
BIBLIO m_CursorShape( aspect )
int aspect;
{
   stream_entry   *sp;
   struct tty     *ptty;

   /*  si on est sur la page active , effectuer la modification ,
    *  sinon , placer l'aspect dans le slot tty
    */
   Tasktab[RUNpid].terrno = 0;
   switch(aspect) {
   case CURS_SMALL:
   case CURS_LARGE:
   case CURS_HIDE : break;
   default        : Tasktab[RUNpid].terrno = EINVAL;
                    return(RERR);
   }
   sp = Tasktab[RUNpid].tfd[stdout];
   if (sp->s_minor != activtxtP)
       tty[sp->s_minor].curs_shape = aspect;
   else
       _doCursorShape(aspect);

   return(ROK);
}

/*----------------------------------------------------------------------------
 * _doCursorShape - modifie l'aspect du curseur ou le rend invisible
 *----------------------------------------------------------------------------
 */
_doCursorShape( aspect )
int aspect;
{
   int ega_correction;
   if (adapter != CGA)
   {
       ega_correction = (adapter == EGA ? 1 : 0);
       switch(aspect)
       {
         case CURS_SMALL:
                         _reg6845(CURSOR_START, S_MONO_SMALL-ega_correction);
                         _reg6845(CURSOR_END  , E_MONO);break;
         case CURS_LARGE:
                         _reg6845(CURSOR_START, S_MONO_LARGE-ega_correction);
                         _reg6845(CURSOR_END  , E_MONO);break;
         case CURS_HIDE :
                         _reg6845(CURSOR_START, CURS_HIDE);
       }
   }
   else
   {
       switch(aspect)
       {
         case CURS_SMALL:
                         _reg6845(CURSOR_START, S_COLOR_SMALL);
                         _reg6845(CURSOR_END  , E_COLOR);break;
         case CURS_LARGE:
                         _reg6845(CURSOR_START, S_COLOR_LARGE);
                         _reg6845(CURSOR_END  , E_COLOR);break;
         case CURS_HIDE :
                         _reg6845(CURSOR_START, CURS_HIDE);
       }
   }
}

/*---------------------------------------------------------------------------
 *  _scroll_screen
 *---------------------------------------------------------------------------
 */
_scroll_screen(ptty, nline)
struct tty *ptty;
int nline;             /* si nline > 0 : scrolling bas vers haut */
                       /* si nline < 0 : scrolling bas vers haut */
{
    unsigned amount, offset;
    int ps;long res;

    ps = _disable();

    if (ptty->ttyminor == activtxtP && !ptty->wmode)
    {                /* scrolling en RAM SCREEN */
       if (adapter == CGA)
           _scroll_Cscr( nline , ptty->cvatt);
       else
           _scroll_Mscr( nline , ptty->cvatt);
    }
    else             /* scrolling en RAM */
           _quick_scroll(FP_SEG(ptty->vsbuf), nline , ptty->cvatt);

    ptty->offsetRAM -= 2 * LINE_WIDTH * nline;
    _restore(ps);
}


/*---------------------------------------------------------------------------
 *  _flush - vider le tampon de sortie video en memoire d'ecran
 *---------------------------------------------------------------------------
 */
_flush(ptty)
struct tty *ptty;
{
    int ps,ps2,nwords;

/*  if (ptty->ttyminor == VSG)
    {
        Gflush(ptty);
        return;
    }*/
    ps = _itDis();
    if (ptty->ttyminor >= TTY1)
    {
        if (ptty->oetat == OINACTIF) /* autoriser IT en sortie */
        {
            ps2 = _disable();
            outp((ptty->ioaddr)->csetit , S_ENABLE);
            ptty->oetat = OACTIF;
            _restore(ps2);
        };
        _itRes(ps);
        return;
    }

    if (ptty->Oin == 0)
    {
        _itRes(ps);
        return;
    }

    /*  rajouter le nb de caracteres a flusher pour calculer le nb de
     *  scrolling a effectuer ( on scroll avant de flusher )
     */

    ptty->cursX += ptty->Oin;
    while (ptty->cursX >= LINE_WIDTH)
    {
         ptty->cursY++;
         ptty->cursX -= LINE_WIDTH;
    }
    for ( ;ptty->cursY >= SCR_LINES ; ptty->cursY--)
                                    _scroll_screen(ptty, 1);


    /* flusher le buffer de sortie sur l'ecran ou en memoire */
    if ((nwords = ptty->Oin - ptty->Oout) > 0)
    {
      if (ptty->ttyminor == activtxtP) {
         if (!ptty->wmode) /* ecriture en RAM SCREEN */
                _vscopy(ptty->Obuff, vid_base, ptty->offsetRAM, nwords);
      }
      else                                 /* ecriture en RAM */
         _vscopy(ptty->Obuff, FP_SEG(ptty->vsbuf), ptty->offsetRAM, nwords);

      ptty->offsetRAM += 2 * nwords;
    }

    /* mettre a jour la position du curseur */
    if (ptty->ttyminor == activtxtP && !ptty->wmode)
        _videoHard(CURSOR, ptty->offsetRAM >> 1);
    ptty->Oin = ptty->Oout = 0;
    _itRes(ps);
}

/*---------------------------------------------------------------------------
 *  _ansiSequence - definition des sequences d'echappement
 *---------------------------------------------------------------------------
 */
_ansiSequence(ptty,ch)
struct tty *ptty;
char ch;
{
   switch(ptty->oesc) {
   case  1 :
             if (ch == 0x5B)  {  /* '[' */
                 ptty->oesc  = 2;
                 ptty->nansi = 0;
                 return;
             }
/*           if (ch == '\033') break;*/
             ptty->oesc = 0;
             return;
   case  2 :
             if (isalpha(ch)) {  /* si oui la sequence est termin‚e */
                 _doAnsi(ptty,ch);
                 ptty->oesc = 0;
                 return;
             }
             /* stocker le caratere */
             if (ch != '\033') {
                 ptty->ansiC[ptty->nansi++] = ch;
                 return;
             }
             return; /*break;*/
   }

   /* une nouvelle sequence d'echappement se superpose a la 1ere :
    * bloquer la tache qui en est l'origine (BUGS)
    */
    while (ptty->oesc != 0) m_Gsleep(5);  /* attente active */
    ptty->oesc = 1; /* repositionner l'etat sequence en debut */
}

/*---------------------------------------------------------------------------
 *  doAnsi - executer la sequence ANSI
 *---------------------------------------------------------------------------
 */
_doAnsi(ptty,ch)
struct tty *ptty;
char ch;
{
   int codeAtt,i,X,Y;

   ptty->ansiC[ptty->nansi] = NULLCH;
   switch(ch)  {     /* type d'op‚ration … effectuer */
   case  'A' :   /* cursor UP */
                 i = atoi(ptty->ansiC);   /* nb de lignes */
                 if (ptty->cursY-i >= 0)
                     _moveCursor(ptty,ptty->cursX,ptty->cursY-i);
                 else
                     _moveCursor(ptty,ptty->cursX,0);
                 break;

   case  'B' :   /* cursor DOWN */
                 i = atoi(ptty->ansiC);   /* nb de lignes */
                 if (ptty->cursY+i < SCR_LINES)
                     _moveCursor(ptty,ptty->cursX,ptty->cursY+i);
                 else
                     _moveCursor(ptty,ptty->cursX,SCR_LINES-1);
                 break;

   case  'C' :   /* cursor RIGHT */
                 i = atoi(ptty->ansiC);   /* nb de lignes */
                 if (ptty->cursX+i < LINE_WIDTH)
                     _moveCursor(ptty,ptty->cursX+i,ptty->cursY);
                 else
                     _moveCursor(ptty,LINE_WIDTH-1,ptty->cursY);
                 break;

   case  'D' :   /* cursor LEFT  */
                 i = atoi(ptty->ansiC);   /* nb de lignes */
                 if (ptty->cursX-i >= 0)
                     _moveCursor(ptty,ptty->cursX-i,ptty->cursY);
                 else
                     _moveCursor(ptty,0,ptty->cursY);
                 break;

   case  'H' :
   case  'f' :   /* MAJ cursor POS */
                 /* separer coordonnees Y et X */
                 for (i=0; i<ptty->nansi ; i++)
                      if (ptty->ansiC[i] == ';') {
                          ptty->ansiC[i] = NULLCH;
                          break;
                      };
                 Y = atoi(ptty->ansiC);
                 X = atoi(&ptty->ansiC[i+1]);
                 _moveCursor(ptty,X,Y);
                 break;

   case  'J' :   /* clear screen */
                 if (ptty->ttyminor < TTY1) {
                    if (ptty->ttyminor == activtxtP && !ptty->wmode)
                        _clr_scr(vid_base,0,2000,ptty->cvatt);
                    else
                        _clr_scr(FP_SEG(ptty->vsbuf),0,2000,ptty->cvatt);
                 }
                 break;

   case  'K' :   /* erase line */
                 if (ptty->ttyminor < TTY1) {
                    if (ptty->ttyminor == activtxtP && !ptty->wmode)
                        _clr_scr(FP_SEG(ptty->vsbuf),ptty->offsetRAM,
                                LINE_WIDTH-ptty->cursX,ptty->cvatt);
                    else
                        _clr_scr(vid_base,ptty->offsetRAM,
                                LINE_WIDTH-ptty->cursX,ptty->cvatt);
                 }
                 break;

   case  'm' :   /* set attribut video */
                 /* separer attributs  */
                 for (i=0; i<ptty->nansi ; i++)
                      if (ptty->ansiC[i] == ';') {
                          ptty->ansiC[i] = NULLCH;
                      };

                 for (i=0; i<ptty->nansi ; ) {
                      codeAtt = atoi(&ptty->ansiC[i]);
                      i += (strlen(&ptty->ansiC[i])+1);
                      switch(codeAtt) {
                      case ANSI_MONO  : ptty->cvatt  = F_WHITE;break;   /* normal    */

                      /* Monochrome  codes */
                      case 1  : ptty->cvatt |= INTENSITY;break; /* surintensite      */
                      case 2  : ptty->cvatt &= ~INTENSITY;break;/* intensite normale */
                      case 5  : ptty->cvatt |= BLINK;break;     /* clignotement      */
                      case ANSI_MONO_REVERSE : ptty->cvatt  = B_WHITE;break;   /* inverse           */
                      case 8  : ptty->cvatt  = 0;break;         /* invisible         */

                      /* foreground codes */
                      case ANSI_F_BLACK  : ptty->cvatt &= 0xf8;break;      /* black F */
                      case ANSI_F_RED    : ptty->cvatt  = (ptty->cvatt&0xf8)|F_RED;break;
                      case ANSI_F_GREEN  : ptty->cvatt  = (ptty->cvatt&0xf8)|F_GREEN;break;
                      case ANSI_F_YELLOW : ptty->cvatt  = (ptty->cvatt&0xf8)|F_YELLOW;break;
                      case ANSI_F_BLUE   : ptty->cvatt  = (ptty->cvatt&0xf8)|F_BLUE;break;
                      case ANSI_F_MAGENTA: ptty->cvatt  = (ptty->cvatt&0xf8)|F_MAGENTA;break;
                      case ANSI_F_CYAN   : ptty->cvatt  = (ptty->cvatt&0xf8)|F_CYAN;break;
                      case ANSI_F_WHITE  : ptty->cvatt  = (ptty->cvatt&0xf8)|F_WHITE;break;

                      /* background codes */
                      case ANSI_B_BLACK  : ptty->cvatt &= 0x8f;break;      /* black B */
                      case ANSI_B_RED    : ptty->cvatt  = (ptty->cvatt&0x8f)|B_RED;break;
                      case ANSI_B_GREEN  : ptty->cvatt  = (ptty->cvatt&0x8f)|B_GREEN;break;
                      case ANSI_B_YELLOW : ptty->cvatt  = (ptty->cvatt&0x8f)|B_YELLOW;break;
                      case ANSI_B_BLUE   : ptty->cvatt  = (ptty->cvatt&0x8f)|B_BLUE;break;
                      case ANSI_B_MAGENTA: ptty->cvatt  = (ptty->cvatt&0x8f)|B_MAGENTA;break;
                      case ANSI_B_CYAN   : ptty->cvatt  = (ptty->cvatt&0x8f)|B_CYAN;break;
                      case ANSI_B_WHITE  : ptty->cvatt  = (ptty->cvatt&0x8f)|B_WHITE;break;

                      default : break;         /* defaut ne rien faire */
                      }
                 }
                 break;
   default   :   break;/*m_Printf(badAnsiStr,ch);*/
   }
}

/*---------------------------------------------------------------------------
 *  _moveCursor  -  deplace le curseur en (x,y)
 *---------------------------------------------------------------------------
 */
_moveCursor(ptty, x, y)
struct tty *ptty;
int x;
int y;
{
    int ps,typ;
    ps = _itDis();
    _flush(ptty);

    if (x < 0 || x >= LINE_WIDTH || y < 0 || y >= SCR_LINES)
    {
         _itRes(ps);
         return(ROK);
    }
    ptty->cursX = x;
    ptty->cursY = y;

    /* calcul nelle position curseur */
    if ((typ = ptty->ttyminor) < TTY1)
    {
        ptty->offsetRAM = (y * 2 * LINE_WIDTH) + (2 * x);
        if ((typ == activtxtP) && !ptty->wmode)
                             _videoHard(CURSOR, ptty->offsetRAM >> 1);
    }
    _itRes(ps);
    return(ROK);
}

/*--------------------------------------------------------------------- ----
 *  m_Beep - emet un signal sonore
 *---------------------------------------------------------------------------
 */
BIBLIO m_Beep(f)
int f;
{
    int x, k;
    int ps;
    ps = _itDis();
    f <<= 1;
    outp(TIMER3, 0xb6);
    outp(TIMER2, f& LOWBYTE);
    outp(TIMER2, (f >> 8) & LOWBYTE);
    x = inp(PORT_B);
    outp(PORT_B, x | 3);
    for(k = 0; k < B_TIME; k++);
    outp(PORT_B, x);
    _itRes(ps);
}

/*---------------------------------------------------------------------------
 *  _videoHard - initialise 2 registres successifs du 6845
 *---------------------------------------------------------------------------
 */
_videoHard(reg, val)
int reg;
unsigned val;
{
    outp(vid_port + INDEX, reg);
    outp(vid_port + DATA, (val >> 8) & 0xff);
    outp(vid_port + INDEX, reg + 1);
    outp(vid_port + DATA, val & 0xff);
}

/*---------------------------------------------------------------------------
 *  _reg6845 - initialise 1 registre du 6845
 *---------------------------------------------------------------------------
 */
_reg6845(reg, val)
int reg;
unsigned val;
{
    outp(vid_port + INDEX, reg);
    outp(vid_port + DATA, val);
}



/*----------------------------------------------------------------------------
 * m_GetSession - ouvrir une session non utilisee et l'affecter a la tache
 *              appelante
 *----------------------------------------------------------------------------
 */
BIBLIO m_GetSession()
{
#define SESSION 1
     int i, fd0, fd;

/*     for (i=0 ; i<NVS-1 ; i++) {
       if (tty[i].streamptr == NULLSTREAM || (tty[i].streamptr->s_count == 0))
           return(_openDev(RUNpid,i,O_RDWR,SESSION));
     }
     return(RERR);*/

     Tasktab[RUNpid].terrno = 0;
     fd0 = m_Dup(0);
     m_Close(0);
     for (i=0 ; i<NVS-1 ; i++)
       if ((tty[i].streamptr == NULLSTREAM) || (tty[i].streamptr->s_count == 0)) {
           if ((fd =_openSession(RUNpid,i,O_RDWR,SESSION)) < 0) {
                m_Dup(fd0);
                m_Close(fd0);
                return(RERR);
           }
           m_Close(1);
           m_Close(2);
           m_Dup(fd);
           m_Dup(fd);
           m_Close(fd0);
           return(ROK);
       }
     m_Dup(fd0);
     m_Close(fd0);
     Tasktab[RUNpid].terrno = EMSESS;
     return(RERR);
}
/*----------------------------------------------------------------------------
 * forceGetSession - forcer l'ouverture d' une session non utilisee et
 *                   l'affecter a la tache appelante
 *----------------------------------------------------------------------------
 */
_forceGetSession(sessionNumber)
{
#define SESSION 1
     int i, fd0, fd;

     fd0 = m_Dup(0);
     m_Close(0);
     if ((fd =_openSession(RUNpid,sessionNumber,O_RDWR,SESSION)) < 0) {
                m_Dup(fd0);
                m_Close(fd0);
                return(RERR);
     }
     m_Close(1);
     m_Close(2);
     m_Dup(fd);
     m_Dup(fd);
     m_Close(fd0);
     return(ROK);
}

/*----------------------------------------------------------------------------
 * _getSessionHandle - retourne le numero de session de la tache appelante
 *----------------------------------------------------------------------------
 */
_getSessionHandle()
{
     return(Tasktab[RUNpid].tgrp);
}

/*----------------------------------------------------------------------------
 * ChgActiveSession -  place la session passee en parametre comme session
 *                     active
 *----------------------------------------------------------------------------
 */
BIBLIO m_ChgActiveSession(sn)
{
        /* rendre impossible la selection de la session d'erreur */
        return(_chgActiveSessionSys(sn, VS8));
}

/*----------------------------------------------------------------------------
 * _ChgActiveSessionSys -  place la session passee en parametre comme session
 *                     active
 *----------------------------------------------------------------------------
 */
_chgActiveSessionSys(sn, limite)
{
        unsigned regState;

        Tasktab[RUNpid].terrno = 0;
        if ((sn < VS0) || (sn > limite)) {
             Tasktab[RUNpid].terrno = EINVAL;
             return(RERR);
        }
        regState = _disable();
        _set_CurrP(sn);
        currP = activtxtP = sn;
        _restore(regState);
        return(ROK);
}

/*----------------------------------------------------------------------------
 * m_GetTaskSessionHandle -  retourne la session de la tache passee en parametre
 *----------------------------------------------------------------------------
 */
BIBLIO m_GetProcSessionHandle(pid)
{
        Tasktab[RUNpid].terrno = 0;
        if (isbadpid(pid)) {
                Tasktab[RUNpid].terrno = EINVAL;
                return(RERR);
        }
        return(Tasktab[pid].tgrp);
}

/*---------------------------------------------------------------------------
 * scrollWin(updown,xg,yg,xd,yd,att)
 *---------------------------------------------------------------------------
 *//* INUTILISE
_scrollWin(xg,yg,xd,yd,nl,att)
{
 int minor;
/*    int i,j,k;
    unsigned *ptr;

    if (color) ptr = 0xB0000000;
    else       ptr = 0xB8000000;

    for (j=yg; j<yd; j++) {
         k = j * 80;
         for (i=xg; i<=xd; i++)
              *(ptr+k+i) = *(ptr+k+80+i);
    }
    k = yd * 80;
    j = (att << 8);
    for (i=xg; i<=xd; i++)
         *(ptr+k+i) = j;*/
/*    if ((minor = Tasktab[m_Getpid()].tfd[1]->s_minor) != activtxtP)
        _qscrollWin(xg,yg,xd,yd,nl,att,FP_SEG(tty[minor].vsbuf));
    else
        _qscrollWin(xg,yg,xd,yd,nl,att,vid_base);
}*/

/*---------------------------------------------------------------------------
 * set_Cdisplay - passer en mode graphique ou texte sur carte COULEUR
 *---------------------------------------------------------------------------
 *//* NON UTILISE
set_Cdisplay(mode)
int mode;
{
     if (mode == TEXTE)    set_mode(0x07);
     else                  set_mode(0x0A);
}*/

/*----------------------------------------------------------------------------
 * set_grP - display vs graphique
 *----------------------------------------------------------------------------
 *//* NON UTILISE
set_grP()
{
   int i,ps;
   struct tty *ptty;

   ps = _itDis();
   if (adapter == MONO)
      if (display == TEXTE)
          set_Mdisplay(GRAFIC);
   _itRes(ps);
}*/


/*---------------------------------------------------------------------------
 * set_Mdisplay - passer en mode graphique ou texte sur carte MONO
 *---------------------------------------------------------------------------
 *//* NON UTILISE
set_Mdisplay(mode)
int mode;
{
     int i;
     if (mode == TEXTE)
     {
        /*  repasser en plan 0 */
/*        outp(M_6845+CONF,2);

        /*  inhiber l'ecran */
/*        outp(M_6845+MODE,0x20);

        /*  initialiser le 6845 */
/*        for ( i = 0;i<=9;i++)
        {
           outp(M_6845+INDEX,i) ;
           outp(M_6845+DATA,txtreg[i]) ;
        }

        /* activer l'ecran */
/*        outp(M_6845+MODE,0x20|8) ;

        display = TEXTE;
     }
     else
     {
        /*  autoriser la selection du mode graphique et du plan 1 */
/*        outp(M_6845+CONF,2|1) ;

        /*  inhiber l'ecran */
/*        outp(M_6845+MODE,2);

        /*  initialiser le 6845 */
/*        for ( i = 0;i<=9;i++)
        {
           outp(M_6845+INDEX,i) ;
           outp(M_6845+DATA,grpreg[i]) ;
        }

        /* activer l'ecran */
/*        outp(M_6845+MODE,2|0x88);

        display = GRAFIC;
     }

}*/

/*---------------------------------------------------------------------------
 * raz_P1 - clear le plan graphique 1
 *---------------------------------------------------------------------------
 *//* NON UTILISE
raz_P1(pattern)
uchar pattern;
{
  int x,y,addlig;
  uchar *page1;
      page1 = GRAFIC1;

      outp(M_6845+CONF,2);

      for ( y = 0;y < 348;y++)
      {
            addlig = (0x2000 * (y % 4)) + (90 * (y/4));
            for ( x = 0; x < 90 ; x++)
                *(page1+(addlig+x)) = pattern;
      }
}*/


/*---------------------------------------------------------------------------
 * Cswap_page - swapping page en code C (utilise pour test )
 *---------------------------------------------------------------------------
 *//* NON UTILISE
Cswap_page(new,old,vs)
int new,old,vs;
{
    int i;
    int *pvid ;
     pvid = 0xB8000000;

    vid_off();
    for (i=0;i<2000;i++)
       *(VStab+i+(old*2000)) = *(pvid+i);

    for (i=0;i<2000;i++)
       *(pvid+i) = *(VStab+i+(new*2000));
    vid_on();
}*/

/*---------------------------------------------------------------------------
 * Cquick_scroll - scrolling en memoire vive ( utilise en phase test )
 *---------------------------------------------------------------------------
 *//* NON UTILISE
Cquick_scroll(base,org,nline,attrib)
int base,org,nline,attrib;
{
   int i;
   nline = nline * 80;
   for (i=0;i<2000 -nline;i++)
         *(VStab+i+(2000*org)) = *(VStab+i+(2000*org)+nline);
   for (;i<2000;i++)
       *(VStab+i+(2000*org)) = attrib << 8;
}*/

/*---------------------------------------------------------------------------
 * Cscroll_Cscr - scrolling en memoire vive ( utilise en phase test )
 *---------------------------------------------------------------------------
 *//* NON UTILISE
Cscroll_Cscr(nline,attrib)
int nline,attrib;
{
   int i;
   nline = nline * 80;

   vid_off();
   for (i=0;i<2000 -nline;i++)
         *(ptrcolor+i) = *(ptrcolor+i+nline);
   vid_on();
   vid_wait();
   for (;i<2000;i++)
       *(ptrcolor+i) = attrib << 8;
}*/


/*---------------------------------------------------------------------------
 * Cscroll_Mscr - scrolling en memoire vive ( utilise en phase test )
 *---------------------------------------------------------------------------
 */ /*  NON UTILISE
Cscroll_Mscr(nline,attrib)
int nline,attrib;
{
   int i;
   nline = nline * 80;

   for (i=0;i<2000 -nline;i++)
         *(ptrmono+i) = *(ptrmono+i+nline);
   for (;i<2000;i++)
       *(ptrmono+i) = attrib << 8;

}*/
