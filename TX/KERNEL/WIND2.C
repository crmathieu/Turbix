/* wind2.c */

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


#define  SUB       1
#define  NOSUB     0
#define  WBUF      1
#define  NWBUF     0

extern unsigned vid_base;
extern unsigned *Pvideo;
extern int activtxtP; /* activtxtP */

#define BACKSP ERASEC

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wdelch - supprimer le caractere courant et decaller les caracteres de
 *          droite d'un cran a gauche
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_wdelch(pwin)
struct window *pwin;
{
    int i,j;
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    for (i=pwin->_curx,j=(pwin->_cury * (pwin->_maxx)); i<pwin->_maxx; i++)
         pwin->_wbuff[i+j] = pwin->_wbuff[i+j+1];

    /* mettre a blanc le dernier caractere de la ligne */
    pwin->_wbuff[j+pwin->_maxx-1] = ((pwin->ink|pwin->paper)<<8)|0x20;

    /* ajuster lastch et firstch */
    pwin->_firstch = (pwin->_cury * (pwin->_maxx)) + pwin->_curx;
    pwin->_lastch  = ((pwin->_cury+1) * (pwin->_maxx));
    return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_winsch - ajouter un caractere sur la position courante et decaller les
 *          caracteres de droite d'un cran a droite
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_winsch(pwin,ch)
struct window *pwin;
unsigned char ch;
{
    int i,j;
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    for (i=pwin->_maxx-1,j=(pwin->_cury * (pwin->_maxx)); i>pwin->_curx; i--)
         pwin->_wbuff[i+j] = pwin->_wbuff[i+j-1];

    /* rajouter le caractere sur la position courante */
    pwin->_wbuff[j+pwin->_curx] = ((pwin->ink|pwin->paper)<<8)|ch;

    /* ajuster lastch et firstch */
    pwin->_firstch = (pwin->_cury * (pwin->_maxx)) + pwin->_curx;
    pwin->_lastch  = ((pwin->_cury+1) * (pwin->_maxx));
    return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wdeleteln - supprime la ligne courante et remonter les lignes inferieures
 *             d'un cran
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_wdeleteln(pwin)
struct window *pwin;
{
    int j;
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    j = (pwin->_cury * (pwin->_maxx));
    _scrollbuff(pwin->_wbuff+j,pwin->wsize-j,pwin->_maxx,
                    1,pwin->ink|pwin->paper);

    pwin->_flags |= W_GLOBALR;
    return(ROK);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_winsertln - ajouter une sur la ligne courante et descendre les lignes
 *             inferieures d'un cran
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_winsertln(pwin)
struct window *pwin;
{
    int j;
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    j = (pwin->_cury * (pwin->_maxx));
    _scrollbuff(pwin->_wbuff+j,pwin->wsize-j,pwin->_maxx,
                    -1,pwin->ink|pwin->paper);

    pwin->_flags |= W_GLOBALR;
    return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_werase - mettre une window a blanc en 0,0
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_werase(pwin)
struct window *pwin;
{
    int i,j;
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    j = ((pwin->ink|pwin->paper)<<8)|0x20;
    for (i=0; i<pwin->wsize; i++)
         pwin->_wbuff[i] = j;
    pwin->_curx = pwin->_cury = 0;
    pwin->_flags |= W_GLOBALR;
    return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wclrtobot - clear de la position courante jusqu'au bout du buffer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_wclrtobot(pwin)
struct window *pwin;
{
   int i,j;
   int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

   j = ((pwin->ink|pwin->paper)<<8)|0x20;
   for (i=(pwin->_curx + (pwin->_cury * pwin->_maxx)); i<pwin->wsize; i++)
        pwin->_wbuff[i] = j;

    /* ajuster lastch et firstch */
    pwin->_firstch = (pwin->_cury * (pwin->_maxx)) + pwin->_curx;
    pwin->_lastch  = pwin->wsize;
    return(ROK);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wclrtoeol - clear de la position courante jusqu'au bout de la ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_wclrtoeol(pwin)
struct window *pwin;
{
   int i,j;
   int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

   j = ((pwin->ink|pwin->paper)<<8)|0x20;
   for (i=(pwin->_curx + (pwin->_cury * pwin->_maxx));
        i<((pwin->_cury+1) * pwin->_maxx); i++)
        pwin->_wbuff[i] = j;

    /* ajuster lastch et firstch */
    pwin->_firstch = (pwin->_cury * (pwin->_maxx)) + pwin->_curx;
    pwin->_lastch  = ((pwin->_cury+1) * (pwin->_maxx));
    return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_winch - lire le caractere sur la position courante
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_winch(pwin)
struct window *pwin;
{
   int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    return(pwin->_wbuff[pwin->_curx + (pwin->_cury * pwin->_maxx)]);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wprintw - ecrire dans une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
m_wprintw(pwin,strfmt,args)
struct window *pwin;
char *strfmt;
{

    int m_waddch();
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    _Prntf(strfmt, m_waddch ,&args, WOUTPUT, pwin);
    return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wscanw - saisie formattee dans une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
m_wscanw(pwin,strfmt,args)
struct window *pwin;
char *strfmt;
{

    int savflag, _wscanaddch(), _wscangetch();
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    savflag = pwin->_flags;
/*    pwin->_flags |= W_ECHO;*/
    _Scnf(strfmt, _wscangetch , _wscanaddch, &args, stdin, WOUTPUT,pwin);
    pwin->_flags = savflag;
    return(ROK);
}

/*---------------------------------------------------------------------------
 *  _wgetch - lire un caractere lorsque la window est ACTIVE
 *---------------------------------------------------------------------------
 */
_wgetch(pwin, flag)
struct window *pwin;
{
    int  ps;
    int  ch;
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    ps = _itDis();
    if (pwin->INPI == pwin->INPO) {  /* pas de caractere a lire */
          pwin->_flags |= W_SUSP;
          Tasktab[pwin->_pid = m_Getpid()].tstate = SLEEP;
          Tasktab[pwin->_pid].tevent              = EV_SUSP;
          _swpProc();
    }
    /* analyser que la partie ascii (supprimer le scan code) */
    if ((ch = (pwin->INPbuf[pwin->INPO++] & 0xff)) == RETURN) ch = NEWLINE;
    pwin->INPO &= WINBUFMASK;

    /* faire echo si flag positionn‚ */
    if (flag && pwin->_flags & W_ECHO)  {
        m_waddch(pwin,ch);
        if (ch == BACKSP)
                _touchwin(pwin, NO_CHECK, s);
        else
                _wrefresh(pwin, NO_CHECK, s);
    }
    _itRes(ps);
    return(ch);
}
/*---------------------------------------------------------------------------
 *  m_wgetch - lire un caractere lorsque la window est ACTIVE
 *---------------------------------------------------------------------------
 */
m_wgetch(pwin)
struct window *pwin;
{
        _wgetch(pwin, TRUE);
}
/*---------------------------------------------------------------------------
 *  m_wscangetch - lire un caractere dans wScanf
 *---------------------------------------------------------------------------
 */
_wscangetch(pwin)
struct window *pwin;
{
        _wgetch(pwin, FALSE);
}
/*---------------------------------------------------------------------------
 *  m_wscanaddch - ecrire dans wscanf
 *---------------------------------------------------------------------------
 */
_wscanaddch(pwin, ch)
struct window *pwin;
{
        m_waddch(pwin, ch);
        m_wrefresh(pwin);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_box - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_box(pwin, Vchar, Hchar)
struct window *pwin;
unsigned char   Vchar,Hchar;
{
        return(_box(pwin, Vchar, Hchar, CHECK));
}

BIBLIO _box(pwin,Vchar,Hchar,_checking, session)
struct window *pwin;
char   Vchar,Hchar;
{
   int s,ps,i,j,tl;
   unsigned att;
   unsigned char ulc,llc,urc,lrc;

   s = _getSessionHandle();
   if (_checking == NO_CHECK)
       s = session;
   if ((Vchar == 0) && (Hchar == 0)) {
       Vchar = pwin->Vborder;
       Hchar = pwin->Hborder;
   }
   else {
       pwin->Vborder = Vchar;
       pwin->Hborder = Hchar;
   }

   if ((s != activtxtP)||!(pwin->_flags & W_VISIBLE))
        return;

   if (Vchar == 'º')
       if (Hchar == 'Í') {
           ulc = 'É'; urc = '»'; llc = 'È'; lrc = '¼';
       }
       else {
           ulc = 'Ö'; urc = '·'; llc = 'Ó'; lrc = '½';
       }
   else
       if (Hchar == 'Í') {
           ulc = 'Õ'; urc = '¸'; llc = 'Ô'; lrc = '¾';
       }
       else {
           ulc = 'Ú'; urc = '¿'; llc = 'À'; lrc = 'Ù';
           Vchar = '³'; Hchar = 'Ä';
       }

   Pvideo = 0;
   FP_SEG(Pvideo) = vid_base;
   att = pwin->ink|pwin->paper;
   if (pwin->_flags & W_FOREGRND)
       att       = F_LGREEN|pwin->paper;
   else
       att       = F_LWHITE|pwin->paper;

   ps = _itDis();

   /* COINS */
   if (pwin->_mask[pwin->_begy*10 + pwin->_begx/8]&maskbit[(pwin->_begx)%8])
       *(Pvideo+((pwin->_begy)*80) + pwin->_begx) = (att<<8)|(uchar)ulc;

   if (pwin->_mask[pwin->_begy*10 + (pwin->_begx+pwin->_maxx+1)/8]&maskbit[(pwin->_begx+pwin->_maxx+1)%8])
       *(Pvideo+((pwin->_begy)*80) + pwin->_begx + pwin->_maxx+1) = (att<<8)|(uchar)urc;

   if (pwin->_mask[(pwin->_begy+pwin->_maxy+1)*10 + pwin->_begx/8]&maskbit[(pwin->_begx)%8])
       *(Pvideo+((pwin->_begy+pwin->_maxy+1)*80) + pwin->_begx) = (att<<8)|llc;

   if (pwin->_mask[(pwin->_begy+pwin->_maxy+1)*10 + (pwin->_begx+pwin->_maxx+1)/8]&maskbit[(pwin->_begx+pwin->_maxx+1)%8])
       *(Pvideo+((pwin->_begy+pwin->_maxy+1)*80) + pwin->_begx+pwin->_maxx+1) = (att<<8)|lrc;

   /* HORIZONTALES */
   for (i=pwin->_begx+1; i<=pwin->_begx+pwin->_maxx; i++) {
        if (pwin->_mask[pwin->_begy*10 + i/8]&maskbit[i%8])
            *(Pvideo+((pwin->_begy)*80 + i)) = (att<<8)|(uchar)Hchar;
        if (pwin->_mask[(pwin->_begy+pwin->_maxy+1)*10 + i/8]&maskbit[i%8])
            *(Pvideo+(pwin->_begy+pwin->_maxy+1)*80 + i) = (att<<8)|(uchar)Hchar;
   }

   /* DETERMINER SI LE TITRE PEUT RENTRER DANS LA LARGEUR */
   tl = strlen(pwin->title);
/*   if ((tl > 0) && (pwin->_maxx-2 >= tl)) {
       i=pwin->_begx+(pwin->_maxx-(tl+2))/2+1;
       if (pwin->_mask[(pwin->_begy)*10 + (i-1)/8]&maskbit[(i-1)%8])
           *(Pvideo + (pwin->_begy)*80 + i - 1) = (att<<8)|0x20;
       for (j=0; j<tl; j++)
            if (pwin->_mask[(pwin->_begy)*10 + (i+j)/8]&maskbit[(i+j)%8])
                *(Pvideo+((pwin->_begy)*80 + i + j)) = (att<<8)|pwin->title[j];
       if (pwin->_mask[(pwin->_begy)*10 + (i+j)/8]&maskbit[(i+j)%8])
           *(Pvideo + (pwin->_begy)*80 + i + j) = (att<<8)|0x20;
   }*/
   if ((tl > 0) && (pwin->_maxx-2 >= tl)) {
       i=pwin->_begx+(pwin->_maxx-tl)/2+1;
       for (j=0; j<tl; j++)
            if (pwin->_mask[(pwin->_begy)*10 + (i+j)/8]&maskbit[(i+j)%8])
                *(Pvideo+((pwin->_begy)*80 + i + j)) = (att<<8)|pwin->title[j];
   }

   /* VERTICALES */
   for (i=pwin->_begy+1; i<=pwin->_begy+pwin->_maxy;i++ ) {
       if (pwin->_mask[i*10 + pwin->_begx/8]&maskbit[pwin->_begx%8])
           *(Pvideo+((i*80) + pwin->_begx)) = (att<<8)|(uchar)Vchar;
       if (pwin->_mask[i*10 + (pwin->_begx+pwin->_maxx+1)/8]&maskbit[(pwin->_begx+pwin->_maxx+1)%8])
           *(Pvideo+((i*80) + pwin->_begx+pwin->_maxx+1)) = (att<<8)|(uchar)Vchar;
   }
   _itRes(ps);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wautocrlf - retour automatique sur bord droit
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_wautocrlf(pwin,yes)
struct window *pwin;
{
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

   if (yes)  pwin->_flags |=  W_AUTOCRLF;
   else      pwin->_flags &= ~W_AUTOCRLF;
   return(ROK);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wecho - echo sur modification du tampon
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_wecho(pwin,yes)
struct window *pwin;
{
    int s;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

   if (yes)  pwin->_flags |=  W_ECHO;
   else      pwin->_flags &= ~W_ECHO;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_touchwin - faire un refresh global
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
BIBLIO m_touchwin(pwin)
struct window *pwin;
{
        return(_touchwin(pwin, CHECK));
}
BIBLIO _touchwin(pwin, _checking, session)
struct window *pwin;
{
    int s, ret;

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

     pwin->_flags |= W_GLOBALR;
     ret = _wrefresh(pwin, _checking, session);
     pwin->_flags &= ~W_GLOBALR;
     return(ret);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * showMask -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 *//* INUTILISE
showMask(pwin)
struct window *pwin;
{
    int x,y;
    m_Printf("\033[2J\033[0;0H");
    for (y=0;y<25;y++) {
    for (x=0;x<79;x++)
         if (pwin->_mask[y*10+x/8]&maskbit[x%8])
            putchar('1');
         else
            putchar('0');
     if (y < 24) putchar('\n');
     else        _flush(&tty[0]);
    }
}*/
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * m_wattach - attacher une Application FULL screen dans une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
m_wattach(pwin, func)
struct window *pwin;
int (* func)();
{


   int fd, i, j, k, s, pp[2], pp1[2], n, ps;
   char c, ch, *ptr, buff[80], *argv[2];

    s = _getSessionHandle();
    Tasktab[RUNpid].terrno = 0;
    if (!windowMode(s)) {
         Tasktab[RUNpid].terrno = EINWM;
         return(RERR);
    }

    m_Pipe(pp);
    if (m_Fork() == 0) {
            m_Pipe(pp1);
            if (m_Fork() == 0) {  /* lire clavier et ecrire ds PIPE PP1 */
                m_Close(pp[0]);
                m_Close(pp[1]);
                m_Close(pp1[0]);
                m_Close(1);
                m_Dup(pp1[1]);
                m_Close(pp1[1]);
                pwin->_flags |= W_ECHO;
                while (1)  {
                      i = 0;
                      do {
                          buff[i++] = (char)m_wgetch(pwin);
                          if (buff[i-1] == BACKSP) {
                                if (i > 2)
                                        i-=2;
                                else    i--;
                                continue;
                          }
                      }
                      while (buff[i-1] != '\n' && (i<80));
                      m_Write(1,buff,i);
                }
            }
            /* lire PIPE PP1 et ecrire dans PIPE PP */
            m_Close(pp[0]);
            m_Close(1);
            m_Dup(pp[1]);
            m_Close(pp[1]);

            m_Close(pp1[1]);
            m_Close(0);
            m_Dup(pp1[0]);
            m_Close(pp1[0]);
            /*argv[0] = "wfunc";
            argv[1] = NULLPTR;
            m_Execv(func, argv);*/
            (*func)();
    }
    /* lecture depuis PP et ecriture dans fenetre */
    m_Close(0);
    m_Dup(pp[0]);
    m_Close(pp[0]);
    m_Close(pp[1]);
    while (1) {
           m_Read(0,buff,1);
           m_waddch(pwin,buff[0]);
           _wrefresh(pwin, NO_CHECK, s);
    }
}
