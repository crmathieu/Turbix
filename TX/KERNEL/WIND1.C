/* window.c */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "window.h"
#include "signal.h"
#include "console.h"
#include "shell.h"

#include "const.h"
#include "stat.h"

#include "dynlink.h"


#define  SUB       1
#define  NOSUB     0
#define  WBUF      1
#define  NWBUF     0

#define isalpha(x)  ((x > 0x40 && x < 0x5B) || (x > 0x60 && x < 0x7B))

/*struct window swin[5];      /* 0:mere 1..n:filles */
extern unsigned vid_base;
struct window *_initwin();
unsigned *Pvideo;
extern int activtxtP; /* activtxtP */


/* PB: lorsqu'on initialise le mode window sur une session autre que
 * la session initiale qui est la session de toutes les taches systemes,
 * une erreur se produit lorsqu'un message est envoy- par une des taches
 * systemes vers la session windowed.
 * En effet l'apparition de la fenetre d'erreur etant conditionn-e par le
 * mode d'affichage de la session dans laquelle s'execute le process qui
 * envoie le message, un conflit apparait si les sessions sont de types
 * differents ...
 *
/*---------------------------------------------------------------------------
 * m_initscr - initialiser le windowing pour l'ecran virtuel ou s'execute la
 *           tache appelante
 *---------------------------------------------------------------------------
 */
BIBLIO m_initscr(ink,paper)
{
    struct tty    *ptty;
    struct window *pwin;
    int i,session, ch;
    int _wgeterr(),_werror(),_touchwin(),_box(),_wselect(),m_wgetch(),
        waddchSys();

    session = _getSessionHandle();
    if (stdscr2(session) != NULLWIN) return(RERR);
    ptty = &tty[session];
    if ((pwin = _initwin(FALSE, 25,80,-1,-1,NOSUB,ink,paper,NWBUF,"")) == NULLWIN)
         return(RERR);

    /* etablir lien dynamique (fonctions appelees dans des modules etrang-s
     * - la gestion des windows)
     */
    winfunc[WIN_SELEC]   = _wselect;    /* console.c  */
    winfunc[WIN_BOX]     = _box;        /* xvideo.c   */
    winfunc[WIN_TOUCH]   = _touchwin;   /* xvideo.c   */
    winfunc[WIN_PERR]    = _werror;     /* printf.c   */
    winfunc[WIN_GERR]    = _wgeterr;    /* console.c  */
    winfunc[WIN_GETCH]   = m_wgetch;    /* syscmd.c   */
    winfunc[WIN_ADDCH]   = waddchSys;   /* console.c  */

    /* le buffer de la fenetre est celui de l'ecran virtuel */
    ptty->pwin     = pwin;
    pwin->_wbuff   = ptty->vsbuf;
    ptty->wmode    = TRUE;
    ptty->nbwin    = 1;
    pwin->_QPREV   = NULLWIN;
    wtail(session) = wcur(session)  = pwin; /* initialiser les pointeurs de controle */

    for (i=0; i<pwin->wsize ; i++)
         pwin->_wbuff[i] = 0;

    /* reserver masque */
    if (((int *)pwin->_mask = _MXmalloc(RUNpid, 250, NORMALB)) == (int *)NULL) {
          Tasktab[RUNpid].terrno = ENOMEM;
/*          sysmsg(winIScrStr);*/
          return(RERR);
    }

    /* initialiser le masque  de l'ecran standard */
    for (i=0 ; i<250; i++) pwin->_mask[i] = 0xff;

    pwin->_flags  |= (W_SCROLLON|W_FOREGRND|W_VISIBLE);
    Pvideo         = 0;
    FP_SEG(Pvideo) = vid_base;

    /* initialiser la window systeme */
    if ((pwin = _initwin(TRUE, 5,70,8,5,NOSUB,F_BLACK,B_WHITE,WBUF,winSysmsgStr)) == NULLWIN)
         return(RERR);
    pwin->_flags  = (W_GLOBALR|W_FOREGRND);
    pwin->Hborder = '-';
    pwin->Vborder = '-';
    wtail(session)->_QNEXT = pwin;   /* chainer la window ERROR a la liste */
    pwin->_QPREV  = wtail(session);
    wtail(session)         = ptty->errwin  = pwin;

    m_wrefresh(stdscr2(session));

    /* DEMON DE DISPATCHING INPUT */
    /* demarrer le demon de dispatching */
    if ((ptty->pidDemon = m_Fork()) == 0) {
        _setprio(m_Getpid(),15);
        strcpy(Tasktab[RUNpid].tname, "idaemon");
        while (ptty->wmode) { /* dispatcher tant que mode Window */
                   ch = (uchar)m_Getc(0);
                   ptty->curwin->INPbuf[ptty->curwin->INPI++] = ch;
                   ptty->curwin->INPI &= WINBUFMASK;

                   if (ptty->curwin->_flags & W_SUSP) {
                       ptty->curwin->_flags &= ~W_SUSP;
                      _setrdy(ptty->curwin->_pid,NOSCHEDUL);
                   }
        }
    }
    Tasktab[RUNpid].terrno = 0;
    return(ROK);
}
/*---------------------------------------------------------------------------
 * m_initStdscrPTR - initialise le pointeur de standard screen
 *---------------------------------------------------------------------------
 */
BIBLIO struct window *m_initStdscrPTR()
{
        return(tty[Tasktab[RUNpid].tgrp].pwin);
}

/*---------------------------------------------------------------------------
 * m_endwin - stoppe l'utilisation des windows sur l'ecran virtuel ou
 *          s'execute la tache appelante
 *---------------------------------------------------------------------------
 */
BIBLIO m_endwin()
{
  struct window *pwin;
  int ps,s;

  /* eliminer les windows a partir de la queue de liste */
  s = _getSessionHandle();
  ps = _itDis();
  for (pwin = wtail(s); pwin != stdscr2(s) ; pwin = wtail(s))
       m_delwin(pwin);

  _xfree(stdscr2(s)->_mask, RUNpid); /* liberer masque de STDSCR */
  _xfree(stdscr2(s), RUNpid);
  windowMode(s) = FALSE;
  m_Printf("\033[2J");
  _itRes(ps);
}

/*---------------------------------------------------------------------------
 * m_newwin - creer une nouvelle window
 *---------------------------------------------------------------------------
 */
BIBLIO struct window *m_newwin(lines,cols,begY,begX,ink,paper,name)
char *name;
{
   int i,ps,s;
   struct window *pwin;

   s = _getSessionHandle();
   Tasktab[RUNpid].terrno = 0;
   if (!windowMode(s)) {
       Tasktab[RUNpid].terrno = EINWM;
       return(NULLWIN);
   }

   if ((pwin = _initwin(TRUE, lines,cols,begY,begX,NOSUB,ink,paper,WBUF,name)) == NULLWIN)
        return(NULLWIN);

   /* chainer cette window aux autres windows */
   ps = _itDis();
   wtail(s)->_QNEXT = pwin;
   pwin->_QPREV  = wtail(s);
   wtail(s)         = pwin;

   _itRes(ps);
   return(pwin);
}

/*---------------------------------------------------------------------------
 * subwin -
 *---------------------------------------------------------------------------
 */
BIBLIO struct window *m_subwin(win_mere,lines,cols,begY,begX,ink,paper,name)
struct window *win_mere;
char *name;
{
   int i, hauteur, largeur,ps, s;
   struct window *pwin;

   s = _getSessionHandle();
   Tasktab[RUNpid].terrno = 0;
   if (!windowMode(s)) {
       Tasktab[RUNpid].terrno = EINWM;
       return(NULLWIN);
   }
   /* controler si la window est bien inscrite dans la window mere */
   if ((begX < win_mere->_begx) || (begY < win_mere->_begy) ||
       (cols+begX  > win_mere->_begx+win_mere->_maxx) ||
       (lines+begY > win_mere->_begy+win_mere->_maxy))
       return(NULLWIN);

   if ((pwin = _initwin(TRUE, lines,cols,begY,begX,SUB,ink,paper,WBUF,name)) == NULLWIN)
        return(NULLWIN);

   /* corriger les coordonnees (les rendre relatives / wparent) */
   pwin->_begx += win_mere->_begx;
   pwin->_begy += win_mere->_begy;

   /* donner le masque de la window mere */
   pwin->_mask    = win_mere->_mask;

   /* chainer cette window a sa window mere */
   pwin->_SWLOWER     = win_mere;
   pwin->_SWUPPER     = win_mere->_SWUPPER;
   if (win_mere->_SWUPPER != NULLWIN)
        win_mere->_SWUPPER->_SWLOWER = pwin;
   win_mere->_SWUPPER = pwin;

/*   wtail(s)->_QNEXT  = pwin;
   pwin->_QPREV   = wtail(s);
   wtail(s)          = pwin;*/

   pwin->_flags   = (win_mere->_flags|W_SUBWIN);
   return(pwin);
}

/*---------------------------------------------------------------------------
 * _initwin -
 *---------------------------------------------------------------------------
 */
struct window *_initwin(checkfit, lines,cols,begY,begX,sub,ink,paper,resbuf,name)
char *name;
{
   int i;
   struct window *pwin;



   /* verifier si la fenetre tient dans l'ecran */
   if (checkfit) {
        _wfit(&begX, &cols, 79);
        _wfit(&begY, &lines, 24);
   }

   if (((int *)pwin = _MXmalloc(RUNpid, WINSIZE, NORMALB)) == (int *)NULL) {
        Tasktab[RUNpid].terrno = ENOMEM;
        /*sysmsg(winInit1Str);*/
        return(NULLWIN);
   }
   for (i=0; i<WINSIZE; i++)
        *((char *)pwin+i) = 0;

   pwin->paper    = paper;
   pwin->ink      = ink;
   pwin->_begx    = begX;
   pwin->_begy    = begY;
   pwin->_firstch = pwin->_lastch = 0;
   pwin->_maxx    = cols;
   pwin->_maxy    = lines;
   pwin->INPI     = 0;
   pwin->INPO     = 0;
   pwin->ansiS    = 0;
   pwin->wsize    = pwin->_maxx * pwin->_maxy;
   pwin->_WUPPER  = NULLWIN;
   pwin->_WLOWER  = NULLWIN;
   pwin->_SWUPPER = NULLWIN;
   pwin->_SWLOWER = NULLWIN;
   pwin->_QNEXT   = NULLWIN;

   /* allouer un tampon */
   if (resbuf) {
       if (((int *)pwin->_wbuff = _MXmalloc(RUNpid, pwin->wsize * 2, NORMALB)) == (int *)NULL) {
             Tasktab[RUNpid].terrno = ENOMEM;
             /*sysmsg(winInit2Str);*/
             return(NULLWIN);
       }
       for (i=0; i<pwin->wsize ; i++)
            pwin->_wbuff[i] = (paper|ink)<<8;

       /* reserver le masque et l'initialiser si on est pas sur une SUB WIN */
       if(!sub) {
          if (((int *)pwin->_mask = _MXmalloc(RUNpid, 250, NORMALB)) == (int *)NULL) {
                Tasktab[RUNpid].terrno = ENOMEM;
                /*sysmsg(winIScrStr);*/
                return(NULLWIN);;
          }
          _initmask(pwin);
       }
   }

   /* nommer la fenetre */
   if (strlen(name) == 0)   pwin->title[0] = '\0';
   else
   {
         for (i=0 ; i<min(TITLE_SIZE-2,strlen(name)+2) ; i++)
              pwin->title[i+1] = name[i];

         pwin->title[i-1] = pwin->title[0] = 0x20;
         pwin->title[i] = pwin->title[TITLE_SIZE-1] = '\0';
   }
   Tasktab[RUNpid].terrno = 0;
   return(pwin);
}

/*---------------------------------------------------------------------------
 * m_waddstr - ecrire une chaine de caracteres dans une fenetre
 *---------------------------------------------------------------------------
 */
BIBLIO m_waddstr(pwin,str)
struct window *pwin;
char *str;
{
    int ret;
    unsigned char ch;
    while ((ch = *str++) != '\0')
          ret = m_waddch(pwin,ch);
    return(ret);
}

/*---------------------------------------------------------------------------
 * m_waddch -
 *---------------------------------------------------------------------------
 */
BIBLIO m_waddch(pwin,ch)
struct window *pwin;
unsigned char ch;
{
   return(waddchSys(pwin,ch,_getSessionHandle()));
}

/*---------------------------------------------------------------------------
 * waddchSys -
 *---------------------------------------------------------------------------
 */
BIBLIO waddchSys(pwin,ch,s)
struct window *pwin;
unsigned char ch;
{
     unsigned ps,i;


     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

     if (pwin->ansiS != 0)  {
               if (isalpha(ch))
                   pwin->ansiS = 0;
               return(ROK);
     }

     switch(ch) {
     case  '\r' : break;
     case  '\n' :
                  pwin->_curx = 0;
                  if (++pwin->_cury >= pwin->_maxy)
                  {
                        pwin->_cury = pwin->_maxy-1;
                        m_wscroll(pwin);
                  }
                  break;
     case '\033': pwin->ansiS = 1;
                  return(ROK);
     case '\b'  : if (pwin->_curx - 1 >= 0) {
                        pwin->_wbuff[pwin->_lastch - 1] = ((pwin->ink|pwin->paper)<<8)|BLANK;
                        pwin->_lastch--;
                        pwin->_curx--;
/*                        if (pwin->_flags & W_ECHO)
                                _touchwin(pwin,NO_CHECK,s);*/
                  }
                  break;

     default :
                  if (pwin->_curx >= pwin->_maxx)
                  {
                        if (!(pwin->_flags & W_AUTOCRLF)) return(ROK);
                        pwin->_curx = 0;
                        if (++pwin->_cury >= pwin->_maxy)
                        {
                            pwin->_cury = pwin->_maxy-1;
                            m_wscroll(pwin);
                        }
                  }
                  pwin->_lastch = pwin->_curx + (pwin->_cury * (pwin->_maxx));
                  pwin->_wbuff[pwin->_lastch++] = ((pwin->ink|pwin->paper)<<8)|ch;
                  pwin->_curx++;
     }
/*     if (pwin->_flags & W_ECHO)
        _wrefresh(pwin,NO_CHECK,s);*/
     return(ROK);
}


/*---------------------------------------------------------------------------
 * m_wmove - deplacer la position courante dans la fenetre donnee par pwin
 *---------------------------------------------------------------------------
 */
BIBLIO m_wmove(pwin,y,x)
struct window *pwin;
{
    int ps, s;

    s = _getSessionHandle();

     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

    if (x < 0 || x >= pwin->_maxx || y < 0 || y >= pwin->_maxy)
        return(ROK);
    m_wrefresh(pwin);
    pwin->_curx = x;
    pwin->_cury = y;
    pwin->_lastch = pwin->_firstch = pwin->_curx + (pwin->_cury * (pwin->_maxx));

    /* calcul nelle position curseur si window active */
    ps = _itDis();
    if ((pwin->_flags & W_FOREGRND) && (/*minorSCR(pid)*/ s == activtxtP))
        _videoHard(CURSOR,
        ((pwin->_begy+y+1) * 2 * LINE_WIDTH) + (2 * (pwin->_begx+1+x)) >> 1);

    _itRes(ps);
    return(ROK);
}


/*---------------------------------------------------------------------------
 * m_wrefresh - rafraichir la fenetre a partir du tampon
 *---------------------------------------------------------------------------
 */
BIBLIO m_wrefresh(pwin)
struct window *pwin;
{
        return(_wrefresh(pwin, CHECK));
}

BIBLIO _wrefresh(pwin, _checking, session)
struct window *pwin;
{
    int ps, s;
    int *pvideo;
    int z,i,j,k,offwline;

    s = _getSessionHandle();
    if (_checking == NO_CHECK)
        s = session;


     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

    /* si l'ecran virtuel n'est pas ACTIF ou si la fenetre n'est pas sur
     * la liste des VISIBLES, ne pas effectuer le rafraichissement
     */
     ps = _disable();
     if ((/*minorSCR(pid)*/ s != activtxtP)||!(pwin->_flags & W_VISIBLE)) {
          _restore(ps);
          return(ROK);
     }

    z = pwin->_firstch;
    pvideo = 0;
    FP_SEG(pvideo) = vid_base;

    if (pwin->_flags & W_GLOBALR)    /* rafraichissement global */
    {
        pwin->_flags &= ~W_GLOBALR;
        if (pwin->_WUPPER == NULLWIN) /* rafraichir en bloc */
               _refresh(pwin->_begx+1,pwin->_begy+1,
                        pwin->_begx+pwin->_maxx ,
                        pwin->_begy+pwin->_maxy,
                        vid_base,FP_SEG(pwin->_wbuff));
        else   /* rafraichir en tenant compte du masque */
                _refreshM(pwin->_begx+1,pwin->_begy+1,
                         pwin->_begx+pwin->_maxx ,
                         pwin->_begy+pwin->_maxy,
                         vid_base,FP_SEG(pwin->_wbuff),
                         pwin->_mask);
    }
    else    /* rafraichir une partie du tampon */
    {
/*        if (pwin->_firstch != pwin->_lastch)
        {*/
            offwline = pwin->_firstch % pwin->_maxx;
            for (j  =  pwin->_begy+1+(pwin->_firstch/pwin->_maxx);
                 j <=  pwin->_begy+1+(pwin->_lastch/pwin->_maxx); j++) {
                 k = j * 80;
                 for (i  =  pwin->_begx + 1 + offwline;
                     (i <=  pwin->_begx+pwin->_maxx) && ( z < pwin->_lastch);
                      i++) {
                      if (pwin->_mask[j*10+i/8] & maskbit[i%8])
                          *(pvideo+k+i) = *(pwin->_wbuff+z);
                      z++;
                 }
                 offwline = 0;
            }
            pwin->_firstch = pwin->_lastch;
/*        }*/
    }
    if (pwin->_flags & W_FOREGRND)   /* MAJ position Curseur */
        _videoHard(CURSOR,
                  ((pwin->_begy+pwin->_cury+1) * 2 * LINE_WIDTH) +
                  (2 * (pwin->_begx+pwin->_curx+1)) >> 1);

    _restore(ps);
    return(ROK);
}

/*---------------------------------------------------------------------------
 * _refreshM -
 *---------------------------------------------------------------------------
 */
_refreshM(xg,yg,xd,yd,SEGvideo,SEGbuf,mask)
unsigned SEGvideo,SEGbuf;
uchar *mask;
{
    int *pvideo,*pbuff;
    int z,i,j,k;
    z = 0;
    pvideo = pbuff = 0;
    FP_SEG(pvideo) = SEGvideo;
    FP_SEG(pbuff)  = SEGbuf;

    for (j=yg; j<=yd; j++) {
         k = j * 80;
         for (i=xg; i<=xd; i++) {
              if (mask[j*10+i/8]&maskbit[i%8])
                  *(pvideo+k+i) = *(pbuff+z);
              z++;
         }
    }
}

/*---------------------------------------------------------------------------
 * _initmask -
 *---------------------------------------------------------------------------
 */
_initmask(pwin)
struct window *pwin;
{
    int y,x;
    for (y=0 ; y<250; y++)
         pwin->_mask[y] = 0;
    for (y = pwin->_begy; y <= pwin->_begy+pwin->_maxy+1; y++)
    for (x = pwin->_begx; x <= pwin->_begx+pwin->_maxx+1; x++)
             pwin->_mask[(y*10) + (x/8)] |= maskbit[x%8];
}


/*---------------------------------------------------------------------------
 * _pushwin - gestion PUSH window
 *---------------------------------------------------------------------------
 */
_pushwin(pwin,flag,s)
struct window *pwin;
{
    struct window *lower;
    int ps;

    if (pwin == stdscr2(s))
        return;

    ps = _itDis();

    /* chainer la nouvelle window a l'ancienne courante */
    if (flag) {
        pwin->_WLOWER   = wcur(s);
        wcur(s)->_WUPPER   = pwin;
        wcur(s)            = pwin;
        pwin->_WUPPER   = NULLWIN;
    }

    /* mettre a jour les masques de toutes les fenetres */
    for (lower = pwin->_WLOWER; lower != NULLWIN ; lower = lower->_WLOWER)
         _pushmask(pwin,lower);

    _itRes(ps);
}

/*---------------------------------------------------------------------------
 * _popwin - gestion POP window
 *---------------------------------------------------------------------------
 */
_popwin(pwin,flag,s)
struct window *pwin;
{
    struct window *lower;
    int ps;

    ps = _itDis();
    if (flag)  {
        wcur(s) = pwin->_WLOWER;
        wcur(s)->_WUPPER = NULLWIN;
    }
    for (lower = pwin->_WLOWER; lower != NULLWIN ; lower = lower->_WLOWER)
         _popmask(pwin,lower);

    for (lower = pwin->_WLOWER; lower != NULLWIN/*stdscr2(s)*/ ; lower = lower->_WLOWER)
         _pushwin(lower,0,s);

    if (flag) _redraw(pwin->_WLOWER,s);
    _itRes(ps);
}

/*---------------------------------------------------------------------------
 * _redraw - retrace les box des differentes windows
 *---------------------------------------------------------------------------
 */
_redraw(pwin, session)
struct window *pwin;
{

   if (pwin != stdscr2(session))
       _redraw(pwin->_WLOWER, session);
   else {
       pwin->_flags |= W_GLOBALR;
       _wrefresh(pwin,NO_CHECK, session);
       return;
   }
   pwin->_flags |= W_GLOBALR;
   _wrefresh(pwin, NO_CHECK, session);
   _box(pwin,0,0, NO_CHECK, session);
}


/*---------------------------------------------------------------------------
 * _pushmask - push un masque sur un autre
 *---------------------------------------------------------------------------
 */
_pushmask(pwinUPPER,pwinLOWER)
struct window *pwinUPPER,*pwinLOWER;
{
    int y,x;

    for (y = pwinUPPER->_begy; y <= pwinUPPER->_begy+pwinUPPER->_maxy+1; y++)
    for (x = pwinUPPER->_begx; x <= pwinUPPER->_begx+pwinUPPER->_maxx+1; x++)
              pwinLOWER->_mask[(y*10) + (x/8)] &= ~maskbit[x%8];
}


/*---------------------------------------------------------------------------
 * _popmask - pop un masque sur un autre
 *---------------------------------------------------------------------------
 */
_popmask(pwinUPPER,pwinLOWER)
struct window *pwinUPPER,*pwinLOWER;
{
    int y,x,bornY,bornX;

    bornY = pwinLOWER->_begy+pwinLOWER->_maxy+1;
    bornX = pwinLOWER->_begx+pwinLOWER->_maxx+1;

    for (y = pwinUPPER->_begy; y <= pwinUPPER->_begy+pwinUPPER->_maxy+1; y++)
         if (y <= bornY)
             for (x = pwinUPPER->_begx;x <= pwinUPPER->_begx+pwinUPPER->_maxx+1; x++)
                  if (x <= bornX)
                      pwinLOWER->_mask[(y*10) + (x/8)] |= /*maskbit[x%8];*/
                      pwinUPPER->_mask[(y*10) + (x/8)];
                  else
                      break;
         else
             break;
}
/*---------------------------------------------------------------------------
 * _borderOn - restitue le masque du cadre
 *---------------------------------------------------------------------------
 */
_borderOn(pwin)
struct window *pwin;
{
    int y,x;

    for (y = pwin->_begy; y <= pwin->_begy+pwin->_maxy+1; y++) {
             pwin->_mask[(y*10) + (pwin->_begx/8)] |= maskbit[pwin->_begx%8];
             pwin->_mask[(y*10) + ((pwin->_begx+pwin->_maxx+1)/8)] |= maskbit[(pwin->_begx+pwin->_maxx+1)%8];
    }

    for (x = pwin->_begx;x <= pwin->_begx+pwin->_maxx+1; x++) {
             pwin->_mask[(pwin->_begy*10) + x/8] |= maskbit[x%8];
             pwin->_mask[((pwin->_begy+pwin->_maxy+1)*10) + x/8] |= maskbit[x%8];
    }
}

/*---------------------------------------------------------------------------
 * m_wscroll -
 *---------------------------------------------------------------------------
 */
BIBLIO m_wscroll(pwin)
struct window *pwin;
{
     int s;

     s = _getSessionHandle();
     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

     if (pwin->_flags & W_SCROLLON) {
         _scrollbuff(pwin->_wbuff,pwin->wsize,pwin->_maxx,
                    1,pwin->ink|pwin->paper);
         pwin->_flags |= W_GLOBALR;
     }
     pwin->_firstch = pwin->_lastch = pwin->wsize - pwin->_maxx;
}


/*---------------------------------------------------------------------------
 * m_getyx - donne la position courante dans la fenetre
 *---------------------------------------------------------------------------
 */
BIBLIO m_getyx(pwin,y,x)
struct window *pwin;
int *y,*x;
{
     int s;

     s = _getSessionHandle();
     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }
     *y = pwin->_cury;
     *x = pwin->_curx;
}

/*---------------------------------------------------------------------------
 * m_wselect - placer une fenetre au dessus (en fenetre ACTIVE)
 *---------------------------------------------------------------------------
 */
BIBLIO m_wselect(pwin)
struct window *pwin;
{
        return(_wselect(pwin, CHECK));
}

BIBLIO _wselect(pwin, _checking, session)
struct window *pwin;
int session;  /* session a utiliser si _checking positionn- - NO_CHECK */
{
   int ps, s;
   struct window *p;

   s = _getSessionHandle();
   if (_checking == NO_CHECK)
        s = session;


     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

   ps = _itDis();
   if (pwin == wcur(s) || pwin == stdscr2(s) || !(pwin->_flags & W_VISIBLE)) {
        _itRes(ps);
        return(ROK);
   }

   /* M A J  chainage */
   if (pwin->_WUPPER != NULLWIN)
       pwin->_WUPPER->_WLOWER = pwin->_WLOWER;
   if (pwin->_WLOWER != NULLWIN)
       pwin->_WLOWER->_WUPPER = pwin->_WUPPER;

   /* placer la fenetre selectionne au sommet du chainage */
   pwin->_WUPPER   = NULLWIN;
   pwin->_WLOWER   = wcur(s);
   wcur(s)->_WUPPER = pwin;

   wcur(s)          = pwin;

   _initmask(pwin);
   _pushwin(pwin, 0, s);

   /* placer la window comme window active */
   pwin->_WLOWER->_flags &= ~(W_FOREGRND);
   pwin->_flags          |=  (W_FOREGRND);

   /* actualiser flag des sub windows */
   for (p = pwin->_SWUPPER; p != NULLWIN; p = p->_SWUPPER)
            p->_flags = (pwin->_flags|W_SUBWIN);

   /* positionner le curseur (en ramenant en coordonnees absolues) */
   _videoHard(CURSOR,
             ((pwin->_begy+pwin->_cury+1) * 2 * LINE_WIDTH) +
             (2 * (pwin->_begx+pwin->_curx+1)) >> 1);

   _redraw(pwin, s);
   _itRes(ps);
   return(ROK);
}

/*---------------------------------------------------------------------------
 * m_delwin - supprimer une fenetre
 *---------------------------------------------------------------------------
 */
BIBLIO m_delwin(pwin)
struct window *pwin;
{
   int ps,s;
   struct window *lower, *p, *q;


   s = _getSessionHandle();

     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

   ps = _itDis();
   if (pwin == stdscr2(s) || (pwin->_flags & W_SUBWIN)) {
        _itRes(ps);
        return(ROK);
   }
   if (pwin->_WUPPER != NULLWIN)
       pwin->_WUPPER->_WLOWER = pwin->_WLOWER;
   if (pwin->_WLOWER != NULLWIN)
       pwin->_WLOWER->_WUPPER = pwin->_WUPPER;
   if (pwin->_QNEXT != NULLWIN)
       pwin->_QNEXT->_QPREV = pwin->_QPREV;
   else
       /* c'est la derniere de la liste : MAJ wtail */
       wtail(s) = pwin->_QPREV;

   pwin->_QPREV->_QNEXT = pwin->_QNEXT;  /* obligatoirement OK */

   _borderOn(pwin);
   if (pwin == wcur(s)) {
       _popwin(pwin,1,s);
       pwin->_WLOWER->_flags |= (W_FOREGRND);
   }
   else
   {
       _popwin(pwin,0,s);
       for (lower = wcur(s); lower != stdscr2(s) ; lower = lower->_WLOWER)
            _pushwin(lower,0,s);
   }
   _redraw(wcur(s),s);

   /* liberer masque */
   _xfree(pwin->_mask, RUNpid);

   /* release the window and all subwindows */
   for (p = pwin; p != NULLWIN; p = q) {
       _xfree(p->_wbuff, RUNpid);  /* liberer TAMPON */
       q = p->_SWUPPER;
       _xfree(p, RUNpid);          /* liberer SLOT   */
   }
   _itRes(ps);
   return(ROK);
}

/*---------------------------------------------------------------------------
 * m_mvwin - deplace une fenetre
 *---------------------------------------------------------------------------
 */
BIBLIO m_mvwin(pwin,y,x)
struct window *pwin;
{
   int ps,s;
   struct window *lower, *p;

   s = _getSessionHandle();

     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

   if ((y+pwin->_maxy > 24) || (x+pwin->_maxx > 79))
        return(ROK);


   ps = _itDis();

   /* actualiser les coor des sub windows */
/*   m_Seprintf(1, "--- debut ---");*/
   for (p = pwin->_SWUPPER; p != NULLWIN; p = p->_SWUPPER) {
        p->_begx += (x - pwin->_begx);
        p->_begy += (y - pwin->_begy);
/*        m_Seprintf(1, "p = %lx\n", p);
        m_sessionGetch(1);*/
   }
   /*m_Seprintf(1, "*** fin ***");*/

   if (pwin == wcur(s)) {
       _popwin(pwin,0,s);
       pwin->_begx = x;
       pwin->_begy = y;
       _borderOn(pwin);
       _pushwin(pwin,0,s);
   }
   else {
       _popwin(pwin,0,s);
       pwin->_begx = x;
       pwin->_begy = y;
       _initmask(pwin); /**/
       _pushwin(pwin,0,s);   /* push sur les fenetres INF */
       for (lower = wcur(s); lower != pwin->_WLOWER ; lower = lower->_WLOWER)
            _pushwin(lower,0,s);   /* pusher toutes les fenetres SUP */
   }
   _redraw(wcur(s),s);
   _itRes(ps);
   return(ROK);
}

/*---------------------------------------------------------------------------
 * m_resizewin - changer taille d'une fenetre
 *---------------------------------------------------------------------------
 */
BIBLIO m_wresize(pwin, nLines, nCols, y, x)
struct window *pwin;
{
   int i, cursor, s;
   unsigned *p, ps;
   struct window *lower;

   s = _getSessionHandle();

   /* verifier si les nelles coor tiennent dans l'ecran */
   _wfit(&x, &nCols, 79);
   _wfit(&y, &nLines, 24);

   /* reserver un nouveau tampon */
   if (((int *)p = _MXmalloc(RUNpid, (nLines * nCols) * 2, NORMALB)) == (int *)NULL) {
        Tasktab[RUNpid].terrno = ENOMEM;
        return(RERR);
   }
   for (i=0; i < (nLines * nCols); i++)
            p[i] = (pwin->paper|pwin->ink)<<8;
   for (i=0; i < min(pwin->wsize, nLines * nCols) ; i++)
            p[i] = pwin->_wbuff[i];

   ps = _itDis();

   /* liberer ancien tampon */
   _xfree(pwin->_wbuff, RUNpid);

   /* poper la window */
   _popwin(pwin,0,s);

   pwin->_wbuff   = p;
   pwin->_begx    = x;
   pwin->_begy    = y;

   /* redefinir (x, y) courants */
   if ((cursor = pwin->_curx + (pwin->_cury * pwin->_maxx)) > (nLines * nCols))
        cursor = nLines * nCols;

   pwin->_curx = cursor % nCols;
   pwin->_cury = cursor / nCols;
   pwin->_maxx    = nCols;
   pwin->_maxy    = nLines;
   pwin->wsize    = pwin->_maxx * pwin->_maxy;
   pwin->_lastch  = pwin->_firstch = cursor;

   pwin->INPI     = 0;
   pwin->INPO     = 0;
   pwin->ansiS    = 0;

   if (pwin == wcur(s)) {
       _initmask(pwin);
       _borderOn(pwin);
       _pushwin(pwin,0,s);
   }
   else {
       _initmask(pwin);
       _pushwin(pwin,0,s);   /* push sur les fenetres INF */
       for (lower = wcur(s); lower != pwin->_WLOWER ; lower = lower->_WLOWER)
            _pushwin(lower,0,s);   /* pusher toutes les fenetres SUP */
   }
   _redraw(wcur(s),s);
   _itRes(ps);
   Tasktab[RUNpid].terrno = 0;
   return(ROK);
}

/*--------------------------------------------------------------------------
 * m_wpush - placer une window deja cree en tete de la chaine
 *---------------------------------------------------------------------------
 */
BIBLIO m_wpush(pwin)
struct window *pwin;
{
   return(wpushSys(pwin, _getSessionHandle()));
}
/*--------------------------------------------------------------------------
 * wpushSys - placer une window deja cree en tete de la chaine
 *---------------------------------------------------------------------------
 */
BIBLIO wpushSys(pwin, s)
struct window *pwin;
int    s;  /* session */
{
   int ps;
   struct window *p;

     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s) || (pwin->_SWLOWER)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

   /* chainer cette window aux autres windows */
   if (pwin != stdscr2(s)) {
       ps = _itDis();
       pwin->_WUPPER   = NULLWIN;
       pwin->_WLOWER   = NULLWIN;
       /*pwin->_SWUPPER  = NULLWIN;
       pwin->_SWLOWER  = NULLWIN;*/

       /* initialiser le masque */
       _initmask(pwin);

       _pushwin(pwin,1,s);
       pwin->_WLOWER->_flags &= ~(W_FOREGRND);
       pwin->_flags          |=  (W_SCROLLON|W_FOREGRND|W_VISIBLE);

       /* actualiser flag des sub windows */
       for (p = pwin->_SWUPPER; p != NULLWIN; p = p->_SWUPPER)
                p->_flags = (pwin->_flags|W_SUBWIN);

       /* afficher le contenu de la window a l'ecran */
       _box(pwin,0,0,NO_CHECK,s);
       _touchwin(pwin,NO_CHECK,s);
       _itRes(ps);
   }
   return(ROK);

}

/*---------------------------------------------------------------------------
 * m_wpop - retirer la window superieure sans la detruire
 *---------------------------------------------------------------------------
 */
BIBLIO m_wpop(pwin)
struct window *pwin;
{
   return(wpopSys(pwin, _getSessionHandle()));
}

/*---------------------------------------------------------------------------
 * wpopSys - retirer la window superieure sans la detruire
 *---------------------------------------------------------------------------
 */
wpopSys(pwin, s)
struct window *pwin;
{
   int ps;
   struct window *p;


     Tasktab[RUNpid].terrno = 0;
     if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
     }

   ps = _itDis();
   if (pwin != wcur(s)) {
        _itRes(ps);
        return(ROK);
   }
   if (pwin->_WUPPER != NULLWIN)
       pwin->_WUPPER->_WLOWER = pwin->_WLOWER;
   if (pwin->_WLOWER != NULLWIN)
       pwin->_WLOWER->_WUPPER = pwin->_WUPPER;

   _borderOn(pwin);
   _popwin(pwin,1,s);
   pwin->_flags &= ~W_VISIBLE;  /* la fenetre n'est plus visible */
   pwin->_WLOWER->_flags |= (W_FOREGRND);

   /* actualiser flag des sub windows */
   for (p = pwin->_SWUPPER; p != NULLWIN; p = p->_SWUPPER)
            p->_flags = (pwin->_flags|W_SUBWIN);

   _redraw(wcur(s),s);
   _itRes(ps);
   return(ROK);
}

/*---------------------------------------------------------------------------
 * _wgeterr - lire a partir de window erreur
 *---------------------------------------------------------------------------
 */
_wgeterr(s)
{
   char ch;

   ch = m_wgetch(werr(s));
   wpopSys(werr(s),s);
   return(ch);
}


/*-----------------------------------------------------------------------------
 * _werror - ecrire sur window erreur
 *-----------------------------------------------------------------------------
 */
_werror(reponse,session,ptty,strfmt,args)
struct tty *ptty;
char *strfmt;
int  *args;
{
    int waddchSys();

    _Prntf(strfmt, waddchSys ,args, WOUTPUT,ptty->errwin,session);
    if (ptty->curwin != ptty->errwin)
        wpushSys(ptty->errwin,session);
    else
        _touchwin(ptty->errwin,NO_CHECK,session);

    if (!reponse) {
        m_Sleep(3);
        wpopSys(ptty->errwin,session);
    }
}
/* verifier si varO+varS ne depassent pas MAX */
_wfit(varO, varS, max)
int *varO, *varS;
{
   if (*varS > max) {
        *varO = 0;
        *varS = max-1;
   }
   else {
        if (*varO + *varS > max)
                if ((*varO = max - *varS) < 0) {
                        *varO = 0;
                        *varS = max-1;
                }
   }
}