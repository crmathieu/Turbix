/* console.c */
#include "sys.h"
#include "conf.h"
#include "tty.h"
#include "window.h"
#include "sem.h"
#include "shell.h"
#include "io.h"
#include "kbd.h"
#include "console.h"
#include "signal.h"
#include "const.h"

#include "fsopen.h"
#include "dynlink.h"

extern       int sysrun;
unsigned     vid_retrace;
unsigned     vid_base;
unsigned     vid_mask;
unsigned     vid_port;
int          currP;        /* ecran virtuel actif */
int          adapter;


unsigned char maskbit[] = {'\x80','\x40','\x20','\x10','\x08','\x04','\x02','\x01'};


char *ttyname[] = {"/DEV/VS0","/DEV/VS1","/DEV/VS2","/DEV/VS3","/DEV/VS4",
                   "/DEV/VS5","/DEV/VS6","/DEV/VS7","/DEV/VS8","/DEV/VS9",
                   "/DEV/VSG","/DEV/tty1","/DEV/tty2","/DEV/tty3","/DEV/tty4"
                  };

extern  int   *cronState;
#define CTRLD  EOF
#define Dcod   32
#define isconsole(x)   ((x) < TTY1)

/* Status clavier du Bios */
unsigned far *BIOS_KBF = (unsigned far *)(0x00400017);

/* control keys */
#define R_SHIFDOWN 0x01 /* SHIFT droit enfonc‚  */
#define L_SHIFDOWN 0x02 /* SHIFT gauche enfonc‚ */
#define R_CTRLDOWN 0x04 /* CTRL droit enfonc‚   */
#define R_ALTDOWN  0x08 /* ALT  droit enfonc‚   */

/* toggle keys */
#define SCRL      0x10 /* scroll lock */
#define NUML      0x20 /* num lock    */
#define CAPL      0x40 /* caps lock   */
#define INS       0x80 /* insert lock */

/* control keys */
#define L_CTRLDOWN 0x100 /* CTRL gauche enfonc‚ */
#define L_ALTDOWN  0x200 /* ALT gauche enfonc‚  */
#define SREQDOWN   0x400 /* sys req enfonc‚     */

/* toggle key */
#define PAUSE     0x800 /* pause lock */

/* control key */
#define SCRLDOWN  0x1000 /* scroll enfonc‚ */
#define NUMLDOWN  0x2000 /* num enfonc‚    */
#define CAPLDOWN  0x4000 /* caps enfonc‚   */
#define INSDOWN   0x8000 /* insert enfonc‚ */

#define SET_LED 1
#define CLR_LED 2
#define DUM_LED 3

unsigned global_KBD=0;
static unsigned far *pOut, *pIn, *pIn2, *pBuf, *ksop, *keop;

/*---------------------------------------------------------------------------
 *  _VIOinit - initialise les buffers pour une ligne tty
 *---------------------------------------------------------------------------
 */
_VIOinit()
{
    register struct tty *ptty;
    int i, minor;
    extern int _itkbd(); extern long msdosv_kbd;

    switch(adapter = _init_video()) {
    case  CGA:
    case  EGA:
         vid_base    = COLOR_BASE;
         vid_mask    = C_VID_MASK;
         vid_port    = C_6845;
         vid_retrace = C_RETRACE;
         break;
    default:   /* MONO */
         vid_base    = MONO_BASE;
         vid_mask    = M_VID_MASK;
         vid_port    = M_6845;
         vid_retrace = M_RETRACE;
    };

    /* initialiser tous les ecrans virtuels */
    for (minor = 0; minor < NTTY; minor++)
    {
         ptty            = &tty[minor];
         ptty->streamptr = NULLSTREAM;
         ptty->ttyminor  = minor;
         for (i=5;i<DEVNAMELEN;i++)
             if ((ptty->ttyname[i-5] = ttyname[minor][i]) == '\0')  break;

         ptty->vsinit = FALSE;
         ptty->vsbuf  = (unsigned *)0;

         ptty->ITin      = ptty->ITout    = 0;
         ptty->Iin       = ptty->Iout     = 0;
         ptty->Oin       = ptty->Oout     = 0;
         ptty->tlock     = FALSE;
/*         ptty->isem      = m_Creatsem(0,"SconINP");*/

         if (!isconsole(minor))
              ptty->osem = m_Creatsem(OBUFLEN,"SconOUT");

         /* init caracteres speciaux de controle */
         ptty->ctty.c_cc[C_INTR]  = INTRC;
         ptty->ctty.c_cc[C_QUIT]  = QUITC;
         ptty->ctty.c_cc[C_ERASE] = ERASEC;
         ptty->ctty.c_cc[C_KILL]  = KILLC;
         ptty->ctty.c_cc[C_EOF]   = EOFC;
         ptty->ctty.c_cc[C_EOL]   = EOLC;


         /* init mode local : mode canonique et filtrage INTRC et QUITC */
         ptty->ctty.c_lflag = (ICANON|ISIG|ECHO|ECHOK|ECHOE);

         /* init mode input : */
         ptty->ctty.c_iflag = ICRNL;
         ptty->icursor      = 0;
         ptty->curs_shape   = CURS_SMALL;

         /* init mode output */
         ptty->ctty.c_oflag = ONLCR;
         ptty->odsend       = 0;
         ptty->oetat        = OINACTIF;
         if (!isconsole(minor))
              ptty->cursX = 0;

         /* init ram screen variables */
         ptty->oesc         = 0;
         if (adapter)  {
             ptty->cvatt = (B_BLACK|F_WHITE);
             ptty->ansiB = ANSI_B_BLACK;
             ptty->ansiF = ANSI_F_WHITE;
         }
         else {
             ptty->cvatt = F_WHITE;
             ptty->ansiB = ANSI_MONO;  /* 0;*/
             ptty->ansiF = ANSI_MONO;/*37;*/
         }
         /* init variables souris */
         ptty->mouCpt   = -1;
         ptty->Min      = 0;
         ptty->Mout     = 0;
         ptty->mouActiv = 0;
         ptty->mouX     = 0;
         ptty->mouY     = 0;


         /* init variables window */
         ptty->wmode   = FALSE;
         ptty->nbwin   = 0;
         ptty->pwin    = NULLWIN;
         ptty->curwin  = NULLWIN;
         ptty->errwin  = NULLWIN;
         ptty->tailwin = NULLWIN;

         /* numero utilisateur des consoles virtuelles : 0 */
         if (isconsole(minor))   ptty->user   = 0;
         else                    ptty->user   = minor-(NVS-1);

         /* demarrer sur page concernee */
         if (isconsole(minor))
                /* deplace le curseur en haut a gauche */
                _moveCursor(ptty, 0, 0);

         /* ON SUPPRIME LA LIAISON SERIE AU PROFIT DE LA SOURIS */
/*         else
                _initcomx(minor-(NVS)); */

    }
    _clr_scr(vid_base, 0 , 2000,ptty->cvatt);  /* effacer Ram screen */
    if (adapter)
         outp(vid_port+MODE,0x29); /* valider blinking mode + 80x25 COLOR */
    else
    {
         outp(vid_port+MODE,0x28); /* valider blinking mode + 80x25 MONO  */
        /* raz_P1(0); NON UTILISE POUR LE MOMENT */
    }
    currP = VS0;
    _videoHard(VID_ORG, 0);          /* demarrer toujours sur VS0 */

    _sasVector(XENKBD, _itkbd , &msdosv_kbd);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*
 * gestion du clavier : le controleur 8048 du kbd declenche                 *
 * une it quand une touche est enfoncee ET quand une touche                 *
 * est relachee .                                                           *
 *     Touche ENFONCEE : le controleur emet le code de la touche            *
 *     Touche RELACHEE : le controleur emet le meme code avec le            *
 *                       8eme BIT forc‚ … 1                                 *
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*
 */
INTERRUPT _keyboard()
{
    int code,val,ps;

    ps = _itDis();
    code = inp(KBD_IN);           /* lecture du code emis par le kbd     */
    val  = inp(KBD_CTL);          /* lire REGISTRE  de Controle clavier  */

    outp( KBD_CTL,( val | 0x80));  /* envoyer ack au controleur du kbd    */
    outp( KBD_CTL, val );          /* ?? mais il faut le faire            */

/*    _msgsndI(VIO,code,((code <= F10cod) && (code >= F1cod)?FALSE:TRUE));*/
/*    _msgsndI(VIO,code,((code <= F10cod) && (code >= F1cod)?TRUE:FALSE));*/
    _msgsndI(VIO, code, FALSE); /*TRUE);*/

    _itRes(ps);
    return(0);
}


/*---------------------------------------------------------------------------
 *  _Input_ch - gestion caracteres en INPUT
 *---------------------------------------------------------------------------
 */
_Input_ch(ptty)
struct tty *ptty;
{
    int  ct, cnt;
    unsigned ch, biosCh;

    biosCh = ptty->ITbuff[ptty->ITout++];
    ptty->ITout &= ITBUFMASK;
    ch = biosCh & 0xff;   /* traiter la partie basse (pas le SCAN CODE) */

    /* les caracteres speciaux (ARROW etc..) ne sont disponibles
     * que si le mode RAW ou CBREAK sont selectionn‚s
     */
    if (israw(ptty))
    {
         /*  RAW mode : on stocke le caractere dans le buffer
          *  d'entree ( si c'est possible ) et on incremente le
          *  semaphore d'entree pour liberer une eventuelle tache
          *  PAS d'ECHO POSSIBLE
          */
         m_Countsem(ptty->isem, &cnt);
         if (cnt >= IBUFLEN)
                 return;
         ptty->Ibuff[ptty->Iin++] = biosCh;
         ptty->Iin &= BUFMASK;
         m_Sigsem(ptty->isem);
    }
    else {
         if ( ch == RETURN  && (ptty->ctty.c_iflag & ICRNL))
              ch = NEWLINE;

         /* replacer eventuelle translation */
         biosCh &= 0xff00;
         biosCh |= ch;


         if (ch == ptty->ctty.c_cc[C_INTR])
         {
              /* envoyer SIGINT a toutes les taches associees
               * a cet ecran virtuel
               */
               for (ct = NSYSTASK-1;ct < NTASK; ct++)
                    if ((Tasktab[ct].tevsig & SigMask[SIGINT]) &&
                        (Tasktab[ct].tgrp == ptty->ttyminor))
                         _sendsig(ct,SIGINT);
               return;
         }
         if (ch == ptty->ctty.c_cc[C_QUIT])
         {
              /* envoyer SIGQUIT a toutes les taches associees
               * a cet ecran virtuel
               */
               for (ct = NSYSTASK-1;ct < NTASK; ct++)
                    if ((Tasktab[ct].tevsig & SigMask[SIGQUIT]) &&
                        (Tasktab[ct].tgrp == ptty->ttyminor))
                         _sendsig(ct,SIGQUIT);
               return;
         }
         if (iscanon(ptty))   /* mode canonique */
         {
               /*  mode CANONIQUE : on gere le caractere de suppression
                *  de ligne , le BACKSP si le mode est valide , on
                *  stocke le caractere d'entree (BELL si plein) , on
                *  fait ECHO du caractere si le mode est valide ,
                *  MAIS on incremente pas le semaphore d'entree - on
                *  incremente a la place "icursor" . lorsque le buffer
                *  est plein ou NEWLINE , on incremente "icursor" fois
                *  le semaphore ( liberant ainsi les taches en attente)
                */
               if (ch == 0)
                    return; /* ICANON ne gere pas les ch SPE */

               if (ch == ptty->ctty.c_cc[C_KILL]) {   /* tuer la ligne */
                   ptty->Iin -= ptty->icursor;
                   if (ptty->Iin < 0)      ptty->Iin += IBUFLEN;
                   ptty->icursor = 0;
                   if (ptty->ctty.c_lflag & ECHOK) { /* Echo kill line */
                       _Xputc(ptty, RETURN);
                       _Xputc(ptty, NEWLINE);
                   }
                   _flush(ptty);
                   return;
               }
               if (ch == NEWLINE || ch == RETURN || ch == ptty->ctty.c_cc[C_EOF])
               {
                   if (isecho(ptty))
                       if (ch != ptty->ctty.c_cc[C_EOF])    _Xecho(ptty, ch);

                   ptty->Ibuff[ptty->Iin++] = biosCh;
                   ptty->Iin &= BUFMASK;
                   ct              = ptty->icursor + 1; /* +1 pour \n ou \r */
                   ptty->icursor   = 0;
                   _signsem(ptty->isem, ct);
                   return;
               }
               if (ch == ptty->ctty.c_cc[C_ERASE])
               {
                   if (ptty->icursor > 0)
                   {
                       ptty->icursor--;
                       _erase(ptty);
                   }
                   return;
               }
               m_Countsem(ptty->isem, &ct);
               ct = ct < 0 ? 0 : ct;
               if ( (ct + ptty->icursor) >= IBUFLEN - 1)
               {
                   /* buffer d'entree plein : envoyer BELL */
                    _Xputc(ptty,BELL);
                    _flush(ptty);
                    return;
               }
               /* a ce stade le caractere n'est un carac de controle */
               if (isecho(ptty))           _Xecho(ptty, ch);
               ptty->icursor++;
               ptty->Ibuff[ptty->Iin++] = biosCh;
               ptty->Iin &= BUFMASK;
         }                     /* fin mode canonique */
         else
         {
               /*  CBREAK mode : on stocke le caractere dans le
                *  buffer d'entree (BELL si plein ou si > MAX DESIRE)
                *  on incremente le semaphore d'entree et on fait
                *  ECHO si le mode echo est valide
                */
               m_Countsem(ptty->isem, &cnt);
               if (cnt >= IBUFLEN) {
                     _Xputc(ptty,BELL);
                     _flush(ptty);
                     return;
               }
               ptty->Ibuff[ptty->Iin++] = biosCh;
               ptty->Iin &= BUFMASK;
               if (isecho(ptty))
                   _Xecho(ptty, ch);
               if (cnt < IBUFLEN)
                   m_Sigsem(ptty->isem);
         }
    }
}

/*---------------------------------------------------------------------------
 *  _Xputc - ecrit un caractere sur ecran virtuel ou tty
 *---------------------------------------------------------------------------
 */
_Xputc(ptty,ch)
struct tty *ptty;
char ch;
{
     if (ptty->ttyminor < TTY1)
         _Cputc(ptty,ch);
     else
         _Sputc(ptty,ch);
}


/*---------------------------------------------------------------------------
 *  _Cputc - ecrit un caractere a l'ecran
 *---------------------------------------------------------------------------
 */
_Cputc(ptty, ch)
struct tty *ptty;
unsigned char ch;
{
    int ps;

    ps = _itDis();

/*    m_Countsem(ptty->tlock, &cnt);
    if (cnt <= 0) {
        /* lock session en cours */
/*        if (ptty->Cpid != m_Getpid()) /* on doit se bloquer */
/*                lockSession();
        /* on a la session */
/*    }*/

    /* sequence d'echappement */
    if (ptty->oesc != 0) {
         _ansiSequence(ptty,ch);
         _itRes(ps);
         return;
    }
    switch(ch)
    {
         case '\x07':          /* ring bell */
                     _flush(ptty);
                     m_Beep(BEEP_FREQ);
                     break;
         case '\x0b':          /* Ctrl-K */
                     _moveCursor(ptty, ptty->cursX, ptty->cursY - 1);
                     break;
         case '\x0e':          /* Ctrl-N */
                     _moveCursor(ptty, ptty->cursX + 1, ptty->cursY);
                     break;
         case '\b'  :          /* backspace */
                     _moveCursor(ptty, ptty->cursX - 1, ptty->cursY);
                     break;
         case '\n'  :          /* line feed */
                     if (ptty->ctty.c_oflag & ONLCR) /* NL -> CR+NL */
                         _moveCursor(ptty, 0, ptty->cursY);

                     if (++ptty->cursY >= SCR_LINES)
                     {
                        _scroll_screen(ptty, 1);
                        ptty->cursY--;
                     }
                     _moveCursor(ptty, ptty->cursX, ptty->cursY);
                     break;
         case '\r'  :          /* carriage return */
                     if (ptty->ctty.c_oflag & OCRNL) /* CR -> NL */
                     {
                         if (++ptty->cursY >= SCR_LINES)
                         {
                             _scroll_screen(ptty, 1);
                             ptty->cursY--;
                         }
                         _moveCursor(ptty, ptty->cursX, ptty->cursY);
                     }
                     else
                         _moveCursor(ptty, 0, ptty->cursY);
                     break;
         case '\t'  :          /* tab */
                     _Cwrite(ptty,"        ",8);
                     break;
         case '\x0c':
                     ptty->Oin = 0;
                     if (currP != ptty->ttyminor)
                         _clr_scr(FP_SEG(ptty->vsbuf), 0, 2000, ptty->cvatt);
                     else
                         _clr_scr(vid_base, 0, 2000, ptty->cvatt);

                     _moveCursor(ptty, 0, 0);
                     break;
         case '\x1b':          /* escape */
                     _flush(ptty);
                     ptty->oesc = 1;
                     break;
         default    :          /* caracteres imprimables */
                     ptty->Obuff[ptty->Oin++] = (ptty->cvatt << 8) | ch;
                     if (ptty->Oin >= OBUFLEN)
                         _flush(ptty);
                     break;
    }
    _itRes(ps);
}

/*---------------------------------------------------------------------------
 *  _Cwrite - ecrire un caractere ou plus d'un tty device
 *---------------------------------------------------------------------------
 */
_Cwrite(ptty, buff, count)
struct tty *ptty;
char   *buff;
int    count;
{
    int ps;
    int i, j;
    char *buff2;

    if (count < 0)       return(RERR);
    if (count == 0)      return(ROK);

    while(count--)
    {
         ps = _itDis();
         /* filtrer si caractere de controle ou sequence d'echappement */
         if (*buff < ' ' || ptty->oesc > 0)   _Cputc(ptty, *buff++);
         else
         {
             ptty->Obuff[ptty->Oin++] = (ptty->cvatt << 8) | *buff++;
             if (ptty->Oin >= OBUFLEN)     _flush(ptty);
         }
         _itRes(ps);
    }
    return(ROK);
}

/*---------------------------------------------------------------------------
 *  _erase - supprime un caractere venant du backspace
 *---------------------------------------------------------------------------
 */
_erase(ptty)
struct tty *ptty;
{
    char ch;

    if (--(ptty->Iin) < 0)      ptty->Iin += IBUFLEN;
    if (ptty->wmode && (ptty->curwin->_flags & W_ECHO)) {
        (* winfunc[WIN_ADDCH])(ptty->curwin, ERASEC, currP);
        (* winfunc[WIN_ADDCH])(ptty->curwin, BLANK, currP);
        (* winfunc[WIN_ADDCH])(ptty->curwin, ERASEC, currP);
        return;
    }
    if (isecho(ptty))
    {
        _Xputc(ptty, ERASEC);
        if (ptty->ctty.c_lflag & ECHOE)   /* echo erase */
        {
            _Xputc(ptty, BLANK);
            _Xputc(ptty, ERASEC);
            if (Vscreen(ptty))
                _moveCursor(ptty, ptty->cursX + 1, ptty->cursY);
        }
    }
    _flush(ptty);
}

/*---------------------------------------------------------------------------
 *  _Xecho - echo du caracter avec les options
 *---------------------------------------------------------------------------
 */
_Xecho(ptty, ch)
struct tty *ptty;
char ch;
{
    if (ptty->ttyminor < TTY1)  {
        if (ptty->wmode) {
             if (wcur(_getSessionHandle())->_flags & W_ECHO)
                (* winfunc[WIN_ADDCH])(ptty->curwin, ch, currP);
        }
        else {
                _Cputc(ptty,ch);
                _flush(ptty);
        }
    }
    else
        _Sputc(ptty,ch);
}

/*---------------------------------------------------------------------------
 *  _CSgetc - lire un caractere en mode bufferise ( commun a tty et console )
 *---------------------------------------------------------------------------
 */
_CSgetc(ptty)
struct tty *ptty;
{
   return(_CSsub(ptty, STAIL, ptty->ctty.c_lflag|ICANON));
}

/*---------------------------------------------------------------------------
 *  _S_CSgetc - lire un caractere en mode bufferise prioritaire
 *---------------------------------------------------------------------------
 */
_S_CSgetc(ptty)
struct tty *ptty;
{
       return(_CSsub(ptty, SHEAD, ptty->ctty.c_lflag|ICANON));
}


/*---------------------------------------------------------------------------
 *  _CSgetch - lire un caractere en mode non bufferise sans echo ( commun a tty et con )
 *---------------------------------------------------------------------------
 */
_CSgetch(ptty)
struct tty *ptty;
{
    return(_CSsub(ptty, STAIL, (ptty->ctty.c_lflag & ~(ICANON|ECHO))));
}
/*---------------------------------------------------------------------------
 *  _CSgetche - lire un caractere en mode non bufferise + echo ( commun a tty et con )
 *---------------------------------------------------------------------------
 */
_CSgetche(ptty)
struct tty *ptty;
{
    return(_CSsub(ptty, STAIL, ((ptty->ctty.c_lflag|ECHO) & ~(ICANON))));
}

/*---------------------------------------------------------------------------
 *  _S_CSgetch - lire un caractere en mode non bufferise prioritaire sans echo
 *---------------------------------------------------------------------------
 */
_S_CSgetch(ptty)
struct tty *ptty;
{
    return(_CSsub(ptty, SHEAD, (ptty->ctty.c_lflag & ~(ICANON|ECHO))));
}
/*---------------------------------------------------------------------------
 *  _S_CSgetche - lire un caractere en mode non bufferise prioritaire + echo
 *---------------------------------------------------------------------------
 */
_S_CSgetche(ptty)
struct tty *ptty;
{
    return(_CSsub(ptty, SHEAD, ((ptty->ctty.c_lflag|ECHO) & ~(ICANON))));
}

/*---------------------------------------------------------------------------
 *  _CSsub -
 *---------------------------------------------------------------------------
 */
_CSsub(ptty, semtyp, flag)
struct tty *ptty;
{
    int ps;
    int  ch;
    unsigned short savflag;

    ps = _itDis();
    savflag = ptty->ctty.c_lflag;
    ptty->ctty.c_lflag = flag;
    _flush(ptty);
    _waitsem(ptty->isem,semtyp);    /* attendre  caractere */
    ch = LOWBYTE & ptty->Ibuff[ptty->Iout++];
    ptty->Iout &= BUFMASK;
    if (ch == ptty->ctty.c_cc[C_EOF])
        ch = EOF;
    ptty->ctty.c_lflag = savflag;
    _itRes(ps);
    return(ch);
}

/*---------------------------------------------------------------------------
 *  _CSread - lire un caractere ou plus d'un tty device - lorsque count = 0
 *            ttyread place dans le buffer tous les carac presents en buffer
 *            systeme d'entree ( commun TTY et CON )
 *---------------------------------------------------------------------------
 */
_CSread(ptty, buff, count)
struct tty *ptty;
int count;
char *buff;
{
    int  avail, nread;
    char ch, eofch;
    int  donow, dolater;
    int ps;

    if (count < 0)         return(RERR);
    if (count == 0)        return(0);
    ps = _itDis();
    _flush(ptty);                     /* vider sortie */
    nread = 0;
    while (count--)
    {
       _waitsem(ptty->isem,STAIL);
       ch = ptty->Ibuff[ptty->Iout++];
       *buff++ = ch;
       ptty->Iout &= BUFMASK;
       nread++;
       if (ch == NEWLINE || ch == RETURN) break;
       if (ch == ptty->ctty.c_cc[C_EOF]) {
           *--buff = EOF;
            break;
       }
    }
    _itRes(ps);
    return(nread);
}


/*---------------------------------------------------------------------------
 *  m_Ioctl - control a tty device by setting modes
 *---------------------------------------------------------------------------
 */
BIBLIO m_Ioctl(fd, cmd, arg)
int  fd;
int  cmd;
struct termio *arg;
{
    struct tty *ptty;
    int      ps,len,out, ct;

    stream_entry   *sp;

    ps = _itDis();
    Tasktab[RUNpid].terrno = 0;
    if ((isbadfd(fd))||((sp = Tasktab[RUNpid].tfd[fd]) == NULLSTREAM)||
       (sp->s_streamtyp != TTYSTREAM)) {
       Tasktab[RUNpid].terrno = EINVAL;
       _itRes(ps);
       return(RERR);
    }
    ptty = &tty[sp->s_minor];
    switch (cmd) {
    case TCGETA :
                   *arg = *(struct termio *)&ptty->ctty;
                   break;
    case TCSETA  :
                   *(struct termio *)&ptty->ctty = *arg;
                   break;
    case TCRAWM  :
                   ptty->ctty.c_lflag &= ~(ICANON | ISIG | ECHO);
                   _flush(ptty);
                   break;
    case TCCANM  :
                   ptty->ctty.c_lflag |= (ICANON | ISIG | ECHO);
                   break;
    case TCECHO  :
                   ptty->ctty.c_lflag |= ECHO;
                   break;
    case TCNOECHO:
                   ptty->ctty.c_lflag &= ~ECHO;
                   break;
    case TCICNT  : /* nb de caracteres en buffer d'entree */
                   m_Countsem(ptty->isem, &ct);
                   _itRes(ps);
                   return(ct);
    case TCICPY  : /* copier input char jusqu'au delimiteur de fin de ligne
                    * et retourner le nombre de caracteres copies
                    */
    default      :
                   out = ptty->Iout;
                   len = 0;
                   while (out != ptty->Iin) {
                          if ((*((char *)arg+len++) = ptty->Ibuff[out++]) == '\n')
                               break;
                          out &= BUFMASK;
                   }
                   _itRes(ps);
                   return(len);
    }
    _itRes(ps);
    return(ROK);
}

/*---------------------------------------------------------------------------
 *  ntty() - retourne le nbre d'‚l‚ment dans ttyname[]
 *---------------------------------------------------------------------------
 */
_ntty()
{
  return(sizeof(ttyname)/sizeof(char *));
}

/*---------------------------------------------------------------------------
 *  _init_video - determine le type de carte video
 *---------------------------------------------------------------------------
 */
_init_video()
{
    int adapter,mode;

    mode = get_mode();
    if (mode == 7)       adapter = MONO;
    else
        if (mode == 15)  adapter = MONO;
        else             adapter = _is_ega();
    return(adapter);
}

/*---------------------------------------------------------------------------
 *  _is_ega - determine si carte EGA ou CGA
 *---------------------------------------------------------------------------
 */
_is_ega()
{
    char far *ega_byte = (char far *) 0x487;
    int  ega_inactive;

    if (get_color() == 0)  return(CGA);
    ega_inactive = *ega_byte & 0x8;
    if (ega_inactive)      return (CGA);
    return (EGA);
}

/*---------------------------------------------------------------------------
 *  _vio : gestion des entrees clavier et switch ecran
 *---------------------------------------------------------------------------
 */
TASK _vio()
{
    int val,ch,k,ct,pid,ps, vs, stat;
    int flagLED;
    static int ascii_pad = 0;
    unsigned statusKBD;
    char *taskname;
    uchar code;
    struct tty *ptty;

     /* augmenter la priorite de la tache VIO / aux taches USER */
/*    _setprio(VIO,12);*/

    ps = _itDis();
    while(1) {
       ch = 0;
       m_Msgwait(&code, -1, &stat);
       /*m_printf("VIO: char Input\n");*/
       if (code == 0)  /* caractere venant de tty */
           _Input_ch(&tty[TTY1]);
       else
           if (code > 127) { /* IT sur relachement */
               if (code == 0xE0)        /* fonction etendue */
                   continue;
               if (code == ALTcodR) {
                   ch = ascii_pad;
                   goto chReady;
               }
           }
           else { /* IT sur enfoncement */
               kbd_state = *BIOS_KBF;
               if (kbd_state & (R_SHIFDOWN | L_SHIFDOWN)) {
                   if (code <= F10cod && code >= F1cod) {      /* switcher DISPLAY */
                       switch(code) {
                       case F10cod : vs = VS0;break;
                       case F1cod  : vs = VS1;break;
                       case 60     : vs = VS2;break;
                       case 61     : vs = VS3;break;
                       case 62     : vs = VS4;break;
                       case 63     : vs = VS5;break;
                       case 64     : vs = VS6;break;
                       case 65     : vs = VS7;break;
                       case 66     : vs = VS8;break;
/*                       case 67     : vs = VS9;break;*/
                       default     : ch = global_KBD;
                                     goto chReady;
                       }
                       currP = _set_currP(vs);
                       continue;
                   }
               }
               if (kbd_state & (R_CTRLDOWN | L_CTRLDOWN)) {
                   if (code == RETURNcod)
                       m_Msgsync(SHTDWN);  /* Retour au Dos */
                   else {
                       ch = global_KBD;
                       goto chReady;
                   }
               }
               if (kbd_state & (R_ALTDOWN | L_ALTDOWN)) {
                   if (code == TABcod) {
                       if (tty[currP].wmode)
                           (* winfunc[WIN_SELEC])(tty[currP].pwin->_WUPPER, NO_CHECK, currP);
                       continue;
                   }
                   else
                       if ((code >= NLOCKcod)/* && (kbd_state & L_ALTDOWN)*/)
                            ascii_pad = (ascii_pad * 10) + (keyTab[code].numlck & 0x0f);
                       else
                            ch = global_KBD;
               }
               ch = global_KBD;
           }
chReady:

           if (ch != 0) {
               if ((ch & 0xff) >= 0xE0)
                   ch &= 0xff00;
               if (ascii_pad)
                   ascii_pad = 0;
/*             else {
                   m_Printf("CH av = %x PIN = %d  POUt = %d\n",ch, *pIn, *pOut);
                   m_Printf("CH ap = %x\n",ch);
               }*/
               ptty = &tty[currP];
               if (ptty->wmode)  { /* window mode */
                   ptty->curwin->INPbuf[ptty->curwin->INPI++] = ch;
                   ptty->curwin->INPI &= WINBUFMASK;

                   if (ptty->curwin->_flags & W_SUSP) {
                       ptty->curwin->_flags &= ~W_SUSP;
                      _setrdy(ptty->curwin->_pid,NOSCHEDUL);
                   }
               }
               else { /* envoi carac dans tampon session */
                   ptty->ITbuff[ptty->ITin++] = ch;
                   ptty->ITin &= ITBUFMASK;
                   _Input_ch(ptty);
               }
           }
    }
}


#define  BIOSTAB 0x00400000   /* adresse debut table du BIOS */
#define  HEADptr 0x0040001a   /* adresse pointeur de queue  */
#define  TAILptr 0x0040001c   /* adresse pointeur de tete   */
#define  KEYbuf  0x0040001e   /* adresse du buffer clavier */
#define  LEN     0x10         /* longueur du buffer (en word) */
#define  KSOP    0x00400080   /* key buf start offset pointer */
#define  KEOP    0x00400082   /* key buf end offset pointer */

_flush_kbd()
{
/*    unsigned far *pOut, *pIn, *pIn2, *pBuf, *ksop, *keop;*/

    pOut  = (unsigned far *)HEADptr;
    pIn   = (unsigned far *)TAILptr;
    *pOut  = *pIn;
}


_get_char()
{
    unsigned save, POUv;
    unsigned far *pbiot;

    pbiot = (unsigned far *)BIOSTAB;
    pOut  = (unsigned far *)HEADptr;
    pIn   = (unsigned far *)TAILptr;
    pBuf  = (unsigned far *)KEYbuf;
    ksop  = (unsigned far *)KSOP;
    keop  = (unsigned far *)KEOP;

    if (*pIn == *pOut)  /* buffer vide */
         global_KBD = 0;
    else {
         POUv     = *pOut;  /* offset output */
         global_KBD = *(pbiot + (POUv/2));
         *pOut   += 2;
         if (*pOut >= *keop)
             *pOut = *ksop;
    }
}

_emulateInt9(scanChar)
unsigned scanChar;
{
    unsigned save,  PINv;
    unsigned far *pbiot;

    pbiot = (unsigned far *)BIOSTAB;
    pOut  = (unsigned far *)HEADptr;
    pIn   = (unsigned far *)TAILptr;
    pBuf  = (unsigned far *)KEYbuf;
    ksop  = (unsigned far *)KSOP;
    keop  = (unsigned far *)KEOP;
    PINv  = *pIn;
    save  = *(pbiot + (PINv/2));
    *(pbiot + (PINv/2)) = scanChar;
    *pIn               += 2;
    if (*pIn > *keop)
        *pIn = *ksop;
    if (*pIn == *pOut) {
        *(pbiot + (PINv/2)) = save;
        *pIn = PINv;
    }
}
