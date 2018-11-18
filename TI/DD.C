/* DISPLAY.C  :   fichier de gestion de l'affichage */
#include "xed.h"
#include <mem.h>

/*----------------------------------------------------------------------
 * delete_to_end_of_line - supprimer Ch de la position courante a la fin
 *----------------------------------------------------------------------
 */
delete_to_end_of_line(wp)
struct Win *wp;
{
    int nb = wp->leftCh + wp->curX;
/*    while (wp->linebuf[i] != EOS)
           wp->linebuf[i++] = EOS;
    wprintln(wp,wp->current);*/
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                wp->ink|wp->paper,
                fillstr);

    wp->fflag |= FF_DIRTY;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * fill_line - remplir tout ou partie du buffer ligne avec Ch
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
fill_line(line,nb,Ch)
char *line;
int nb;
char Ch;
{
    while (nb-- > 0)
         *line++ = Ch;
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * erase_trailing_blank - supprimer d'eventuels caracteres ' 'en fin de ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
erase_trailing_blank(line)
char *line;
{
    int i;
    i = lnlen(line);
    while (line[--i] == BLANK)
           line[i] = EOS;
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getnextline - retourne la position de la N ieme ligne apres offline
 *               ou bien la position de la derniere ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getnextline(wp,offline,n)
struct Win *wp;
unsigned offline;
{
    unsigned X;
    while ((n-- > 0) && ((X = lnnext(wp,offline)) != offline))
            offline = X;
    return(offline);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getprevline - retourne la position de la N ieme ligne avant offline
 *               ou bien la position de la premiere ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getprevline(wp,offline,n)
struct Win *wp;
unsigned offline;
int n;
{
    unsigned X = offline;
    while (n-- > 0)
           if ((X = lnprev(wp,X)) == 0) break;
    return(X);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * set_win - rafraichir la fenetre entierement
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION set_win(wp,debug)
struct Win *wp;
unsigned debug;
{
    int nb, lig, attrib,att,flagatt;
    unsigned pt;

    lig = 0 ;
    pt = wp->topPage;
    att = wp->ink|wp->paper;
    for ( ; (lig < wp->nline) && (pt <= wp->bottom); pt += lnlen(wp,pt) + 1, lig++) {
/*         if ((wp->fflag & FF_BLOCKDEF) && ISINBLK(wp,pt) )
               attrib  = wp->blockatt;
         else */ attrib  = att;

         if ((nb = INF(lnlen(wp,pt) - wp->leftCh, wp->ncol)) > 0)
              _wstringDB( wp->ul_x + 1,
                          wp->ul_y + 1 + lig,
                          nb,
                          NEWATT,/*flagatt,*/
                          attrib,/*wp->ink|wp->paper,*/
                          wp->bigbuf + pt + wp->leftCh);  /* sauter les Ch cach‚s */
         else
              nb = 0;

         /* completer avec des blancs */
         if (nb < wp->ncol)
             _wstringDB( wp->ul_x + 1 + nb,
                         wp->ul_y + 1 + lig,
                         wp->ncol - nb,
                         NEWATT,/*flagatt,*/
                         attrib,/*wp->ink|wp->paper,*/
                         fillstr);

         /* gestion marquage Block & Paste */
/*         put_att_BP(wp,pt,lig);*/
    }

    /*  si on est en fin de fichier, remplir a blanc les lignes suivantes */
    while (lig < wp->nline)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + 1 + lig++,
                     wp->ncol,
                     NEWATT,/*flagatt,*/
                     wp->ink|wp->paper,
                     fillstr);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnlen - calcul la taille d'une ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lnlen(wp,offline)
struct Win *wp;
unsigned offline;
{
   char c;
   unsigned i = offline;
   while ((c = wp->bigbuf[i++]) != '\n'&& c != EOFM) ;
   return(i - (offline + 1));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnbuflen - calcul la taille de la ligne tampon
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lnbuflen(wp)
struct Win *wp;
{
   char c;
   int i = 0;
   while (wp->linebuf[i++] != EOS) ;
   return(i-1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lncpy - copier une ligne ds le tampon ligne (delimiteur = EOS = '\n')
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lncpy(wp,dest,offline)
struct Win *wp;
char *dest;
unsigned offline;
{
   char c;
   while ((c = wp->bigbuf[offline++]) != EOS && c != EOFM);
           *dest++ = c;
   *dest = EOS;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnprev - retourne l'offset de la ligne precedente
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned lnprev(wp,offline,debug)
struct Win *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X = offline;
   if (X > 1)             X--;
   else                   return(0);
   while (X >= 1) {
          if (wp->bigbuf[X - 1] == '\n') {
              return(X);
          }
          X--;
   }
   return(0); /* offset de debut fichier */

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getlast_ln - retourne l'offset de la derniere ligne (A  TERMINER)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getlast_ln(wp)
struct Win *wp;
{
   unsigned pos = wp->fsize-1;  /* le dernier Ch est EOFM */

   while (pos > 0 && wp->bigbuf[pos - 1] != '\n') pos--;
   return(pos);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getlast_line_no - retourne le nb de lignes
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
getlast_line_no(wp)
struct Win *wp;
{
   int i;
   unsigned pos = wp->fsize - 1;
   i = 1;
   while (pos > 0)
          if (wp->bigbuf[pos--] == '\n')
              i++;
   return(i);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnnext - retourne l'offset de la ligne suivante
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned lnnext(wp,offline)
struct Win *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X;
   if (((X = lnlen(wp,offline) + 1) + offline) > wp->fsize) /*>= wp->fsize)*/
        return(offline); /* on est sur la derniere ligne */
   return(offline + X);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * initWin : initialiser une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION initWin(wp,ul_x,ul_y,ncols,nlines,ink,paper,wborder,batt)
struct Win *wp;
{
   wp->fflag      = FF_INSERT;
   wp->nline      = nlines;
   wp->leftCh     = 0;
   wp->curX       = wp->curY = 0;
   wp->wborder    = wborder;
   wp->ink        = ink;
   wp->paper      = paper;
   wp->ul_x       = ul_x;
   wp->ul_y       = ul_y;
   wp->ncol       = ncols;
   wp->blockatt   = batt;

   wp->current = wp->topPage = wp->bottom = 0;
   wp->current_line_no = wp->topPage_line_no = 1;

   wp->topBlock = wp->bottomBlock = 0;
   wp->topPaste = wp->bottomPaste = 0;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * clrwin Ä clear window
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION clrwin(wp)
struct Win *wp;
{
       int i;
       _clrwin(   wp->ul_x+1,
  	          wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * scrollwinUP Ä scrolling window UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION scrollwinUP(wp)
struct Win *wp;
{
       _window_up(1,wp->ul_x+1,
  	          wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * scrollwinDWN Ä scrolling window DOWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION scrollwinDWN(wp)
struct Win *wp;
{
       _window_down(1,wp->ul_x+1,
  	            wp->ul_y+1,
                    wp->ul_x + wp->ncol,
                    wp->ul_y + wp->nline,
                    wp->ink|wp->paper);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_box - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_box(wp)
struct Win *wp;
{
   int ps,i,j,tl;
   char   Vchar,Hchar;
   unsigned att;
   unsigned char ulc,llc,urc,lrc;

   ulc = 'Õ'; urc = '¸'; llc = 'Ô'; lrc = '¾';/**/
   Vchar = '³'; Hchar = 'Í';
   att = wp->wborder;

   /* COINS */
   *(Pvideo+((wp->ul_y)*80) + wp->ul_x) = (att<<8)|(unsigned char)ulc;
   *(Pvideo+((wp->ul_y)*80) + wp->ul_x + wp->ncol+1) = (att<<8)|(unsigned char)urc;
   *(Pvideo+((wp->ul_y+wp->nline+1)*80) + wp->ul_x) = (att<<8)|llc;
   *(Pvideo+((wp->ul_y+wp->nline+1)*80) + wp->ul_x+wp->ncol+1) = (att<<8)|lrc;

   /* HORIZONTALES */
   for (i= wp->ul_x+1; i <= wp->ul_x+wp->ncol; i++) {
        *(Pvideo+((wp->ul_y)*80 + i)) = (att<<8)|(unsigned char)Hchar;
        *(Pvideo+(wp->ul_y+wp->nline+1)*80 + i) = (att<<8)|(unsigned char)Hchar;
   }

   /* VERTICALES */
   for (i = wp->ul_y+1; i <= wp->ul_y+wp->nline;i++ ) {
        *(Pvideo+((i*80) + wp->ul_x)) = (att<<8)|(unsigned char)Vchar;
        *(Pvideo+((i*80) + wp->ul_x+wp->ncol+1)) = (att<<8)|(unsigned char)Vchar;
   }
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_up - ED arrow UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_up(wp)
struct Win *wp;
{
   if (wp->current == 0)  /* debut de fichier */
       return(-1);

   wp->current_line_no--;
   if (wp->curY == 0) {         /* debut de page    */
       scrollwinDWN(wp);
       wp->current = lnprev(wp,wp->current);
       wp->topPage = wp->current;
       wp->topPage_line_no = wp->current_line_no;
       wprintln(wp,wp->current);
       /* RAJOUTER PLUS TARD LA GESTION DES ATTRIBUTS POUR BLK ET PASTE */
   }
   else
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY - 1);
/*   put_att_BP(wp,wp->current,wp->curY);*/
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_down - ED arrow DWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_down(wp)
struct Win *wp;
{
 /*  printf("C = %u  B = %u",wp->current,wp->bottom);*/
   if (wp->current >= wp->bottom) { /* fin de fichier */
       printf("cno = %d  bno = %d ",wp->current_line_no,wp->bottom_line_no);
       return(-1);
   }
   wp->current_line_no++;
   if (wp->curY == wp->nline - 1) {    /* fin de page    */
       scrollwinUP(wp);
       wp->current = lnnext(wp,wp->current);
       wp->topPage = lnnext(wp,wp->topPage);
       wp->topPage_line_no++;
       wprintln(wp,wp->current);
       /* RAJOUTER PLUS TARD LA GESTION DES ATTRIBUTS POUR BLK ET PASTE */
   }
   else
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY + 1);
/*   put_att_BP(wp,wp->current,wp->curY);*/
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_right - ED arrow RIGHT
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_right(wp)
struct Win *wp;
{
   if ((wp->leftCh + wp->curX) >= NB_CAR_PER_LINE)
        return(-1);

   if (wp->curX < wp->ncol - 1) {
       wmgotoxy(wp,wp->curX + 1,wp->curY);
       return(0);
   }
   else {
       wp->leftCh++;
       set_win(wp);
   }
   return(1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_left - ED arrow LEFT
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_left(wp)
struct Win *wp;
{
   if ((wp->leftCh + wp->curX) == 0)
        return(-1);

   if (wp->curX > 0) {
       wmgotoxy(wp,wp->curX - 1,wp->curY);
       return(0);
   }
   else {
       wp->leftCh--;
       set_win(wp);
   }
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * page_up - ED Page UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION page_up(wp)
struct Win *wp;
{
   int down,up;

   if (wp->current == 0)
       return;

   if (wp->topPage == 0) {
       /* debut fichier : se placer sur la 1ere ligne */
       wmgotoxy(wp,wp->curX,0);
       wp->current_line_no = 1;
       return;
   }
   if (wp->topPage_line_no - wp->nline < 0) {
       /* On est ds la 1ere Page fichier : placer le sommet de page
        * en debut fichier, puis positionner la ligne courante
        * au meme endroit
        */
/*       printf("TOPLINE = %d ",wp->topPage_line_no);
       getch();*/
       wp->topPage = wp->current = 0;
       wp->topPage_line_no = 1;
       wp->current = getnextline(wp,wp->current,wp->curY);
       wp->current_line_no = wp->curY + 1;
   }
   else {
       /* cas general : remonter d'une page */
/*       printf("TOPPAGE AVANT = %u  NLINE = %d",wp->topPage,wp->nline-1);
       getch();*/

       wp->topPage = getprevline(wp,wp->topPage,wp->nline - 1);/*+1);*/
/*       printf("TOPPAGE APRES = %u ",wp->topPage);
       getch();*/

       wp->topPage_line_no -= wp->nline - 1;
       wp->current = getprevline(wp,wp->current,wp->nline - 1);/*+1);*/
       wp->current_line_no -= wp->nline - 1;
   }
   set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * page_down - ED Page DWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION page_down(wp)
struct Win *wp;
{
   int down,up,y;

   if (wp->current == wp->bottom)
       return;

   if (wp->topPage_line_no + wp->nline >= wp->bottom_line_no) {
       /* on est sur la derniere page : placer le sommet de page et
        * la ligne courante sur la derniere ligne
        */
       wp->topPage = wp->current = wp->bottom;
       wp->topPage_line_no = wp->current_line_no = wp->bottom_line_no;
       wgotoxy(wp,wp->curX,0);
   }
   else {
       /* cas general : placer le sommet de page une page plus loin et
        * ajuster la position de la ligne courante
        */
       wp->topPage = getnextline(wp,wp->topPage,wp->nline - 1);/* + 1);*/
       wp->topPage_line_no += wp->nline - 1;

       if (wp->current_line_no + wp->nline - 1/* + 1*/ >= wp->bottom_line_no)
           wgotoxy(wp,wp->curX,wp->bottom_line_no - wp->topPage_line_no);

       wp->current = getnextline(wp,wp->current,wp->nline - 1);/* + 1);*/
       wp->current_line_no += wp->nline - 1;
   }
   set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * line_end - ED line END
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION line_end(wp)
struct Win *wp;
{
  int nb;

  if ((nb = lnlen(wp,wp->current) - wp->leftCh) < wp->ncol) {
        if (nb > 0)
            /* se placer simplement en fin
             * de ligne
             */
            wgotoxy(wp,nb,wp->curY);
        else {
            /* la ligne est petite et la
             * position courante est grande
             */
            wp->leftCh = lnlen(wp,wp->current);
            wgotoxy(wp,0,wp->curY);
            set_win(wp);
        }
  }
  else {
       /* la fin de ligne est plus loin que la
        * taille de la fenetre
        */
       wp->leftCh = lnlen(wp,wp->current) - wp->ncol + 1;
       wgotoxy(wp,wp->ncol-1,wp->curY);
       set_win(wp);
  }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * line_home - ED line HOME
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION line_home(wp)
struct Win *wp;
{
  int nb;

  if (wp->leftCh > 0) {
      wp->leftCh = 0;
      set_win(wp);
  }
  wmgotoxy(wp,0,wp->curY);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wmgotoxy Ä se positionner dans la page avec mise a
 *            jour de la ligne courante en memoire
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wmgotoxy(wp,x,y)
struct Win *wp;
{
   int aux;

   if (x >= wp->ncol)    x = wp->ncol - 1;
   if (y >= wp->nline)   y = wp->nline - 1;
   wp->curX = x;
   if (wp->curY != y) {
       if (wp->curY > y)
           wp->current = getprevline(wp,wp->current,wp->curY - y);
       else {
/*           putchar('@');*/
           wp->current = getnextline(wp,wp->current,y - wp->curY);
       }
       wp->curY = y;
       wprintln(wp,wp->current);  /*** VERIF  A SAQUER ***/
/*       printf("CURRENT = %d\n",wp->current);*/
   }
   gotoxy(wp->ul_x+x+1,wp->ul_y+y+1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wgotoxy Ä se positionner dans la page sans mise a
 *           jour de la ligne courante en memoire
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wgotoxy(wp,x,y)
struct Win *wp;
{
   int aux;

   if (x >= wp->ncol)    x = wp->ncol - 1;
   if (y >= wp->nline)   y = wp->nline - 1;
   wp->curX = x;
   wp->curY = y;
   gotoxy(wp->ul_x+x+1,wp->ul_y+y+1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintln - ecrire une ligne  (ou une partie) dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintln(wp,offline)
struct Win *wp;
unsigned offline;
{
    int attrib , nb;
/*    if (ISINBLK(wp,pt) && (wp->fflag & FF_BLOCKDEF))
          attrib  = wp->blockatt;
    else*/  attrib  = wp->ink|wp->paper;

    if ((nb = INF(lnlen(wp,offline) - (wp->leftCh), wp->ncol)) > 0)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + wp->curY + 1,
                     nb,
                     NEWATT,
                     attrib,
                     wp->bigbuf + offline + wp->leftCh);  /* sauter les Ch cach‚s */
    else
         nb = 0;

    /* si la ligne a une taille inferieure a la fenetre, completer a blanc */
    if (nb < wp->ncol)
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                attrib,
                fillstr);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintlnbuf - ecrire la ligne tampon dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintlnbuf(wp)
struct Win *wp;
{
    int attrib , nb;
/*    if (ISINBLK(wp,pt) && (wp->fflag & FF_BLOCKDEF))
          attrib  = wp->blockatt;
    else*/  attrib  = wp->ink|wp->paper;

    if ((nb = INF(lnbuflen(wp) - (wp->leftCh), wp->ncol)) > 0)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + wp->curY + 1,
                     nb,
                     NEWATT,
                     attrib,
                     wp->linebuf + wp->leftCh);  /* sauter les Ch cach‚s */
    else
         nb = 0;

    /* si la ligne a une taille inferieure a la fenetre, completer a blanc */
    if (nb < wp->ncol)
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                attrib,
                fillstr);
}


/*------------------------------------------------------
 * winsln() - inserts a line
 *------------------------------------------------------
 */
winsln(wp,insert)
struct Win *wp;
{
    int i, lcb;

    if (wp->fflag & FF_LINE_UPDATE) {
        /* prise en compte modifications de la ligne courante */
        insert_lnbuf(wp);
        wp->fflag &= ~FF_LINE_UPDATE;
    }

    if (!(wp->fflag & FF_INSERT) && insert == ON_MODE) { /* mode Overwrite */
        if (wp->leftCh) {
            wp->leftCh = 0;
            set_win(wp);
        }
        arrow_down(wp);
        wgotoxy(wp,0,wp->curY);
        return;
    }

    /* tester si la ligne a ete coupee */
    if (wp->curX + wp->leftCh <= lnlen(wp,wp->current))
        delete_to_end_of_line(wp);

    /* mode Insert */
    newline(wp);

    /* ajouter indentation automatique */

    /* GESTION BLOCK */
/*    if ((wp->fflag & FF_BLOCKDEF) && insert == ON_MODE) {
        if (wp->current == wp->bottomBlock &&
            wp->lastCh > (wp->curX + wp->leftCh)) {
            wp->lastCh -= (wp->curX + wp->leftCh);
            wp->bottomBlock = pt;
        }
        if (wp->current == wp->topBlock &&
            wp->firstCh > (wp->curX + wp->leftCh)) {
            wp->firstCh -= (wp->curX + wp->leftCh);
            wp->topBlock = pt;
        }
    }*/

    if (wp->curY < wp->nline - 1) {
        _window_down(1,wp->ul_x + 1,
  	               wp->ul_y + 1 + (wp->curY + 1),
                       wp->ul_x + wp->ncol,
                       wp->ul_y + wp->nline,
                       wp->ink|wp->paper);
        wp->curY++;
    }
    else {
        _window_up(  1,wp->ul_x + 1,
  	               wp->ul_y + 1,
                       wp->ul_x + wp->ncol,
                       wp->ul_y + wp->nline,
                       wp->ink|wp->paper);
        wp->topPage = lnnext(wp,wp->topPage);
        wp->topPage_line_no++;
    }
    wgotoxy(wp,0,wp->curY);
    if (wp->leftCh == 0) {
        wprintln(wp,wp->current);
/*        put_att_BP(wp,wp->current->prev,wp->curY - 1);
        put_att_BP(wp,wp->current,wp->curY);*/
    }
    else {
        wp->leftCh = 0;
        set_win(wp);
    }
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * essai - tente d'inserer des caracteres dans le buffer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION essai(wp)
struct Win *wp;
{
    int i;
    unsigned n = wp->current + wp->curX;
    memmove(wp->bigbuf + n + 3,
           wp->bigbuf + n,
           wp->fsize - n);
    for (i=0;i<3;i++)
      wp->bigbuf[n+i] = '@';
    wp->fsize += 3;
    if (wp->bottom != wp->current)
        wp->bottom += 3;
    set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * newline - ajouter EOS dans le bigbuf
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION newline(wp)
struct Win *wp;
{
    int i;
    unsigned n = wp->current + wp->curX + wp->leftCh;
    if (wp->fsize + 1 >= BIGBUF_SIZE - 1) {
        printf("OUT OF MEMORY");
        return;
    }
    memmove(wp->bigbuf + n + 1,
            wp->bigbuf + n,
            wp->fsize  - n);
    wp->bigbuf[n] = EOS;
    wp->fsize++;
    if (wp->bottom != wp->current)
        wp->bottom++;
    wp->current += n;
    wp->current_line_no++;
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * insert_lnbuf - tente d'inserer des caracteres dans le buffer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION insert_lnbuf(wp)
struct Win *wp;
{
    int i, Plen, Nlen, diff;
    unsigned pos;
    Plen = lnlen(wp,wp->current) + 1;
    Nlen = lnbuflen(wp) + 1;
    if ((diff = Nlen - Plen) == 0) {
        /* la ligne a la meme taille : copier sans decaler */
        copy_lnbuf_to_bigbuf(wp,Plen);
        return;
    }
    /* la ligne modifiee n'a pas la meme taille que celle dans bigbuf */
    if (wp->fsize + diff >= BIGBUF_SIZE - 1) {
        printf("OUT OF MEMORY");
        return;
    }
    pos = wp->current + Plen;
    memmove(wp->bigbuf + pos + diff,
           wp->bigbuf + pos,
           wp->fsize - pos);
    copy_lnbuf_to_bigbuf(wp,Nlen);
    wp->fsize += diff;
    if (wp->bottom != wp->current)
        wp->bottom += diff;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * copy_lnbuf_to_bigbuf - copier le buffer ligne a la place de la ligne
 *                        courante dans bigbuf
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION copy_lnbuf_to_bigbuf(wp,size)
struct Win *wp;
{
   char *line;
   char far *pos;
   pos = wp->bigbuf + wp->current;
   line = wp->linebuf;
   while (size-- > 0)
          *pos++ = *line++;
}
/* DISPLAY.C  :   fichier de gestion de l'affichage */
#include "xed.h"
#include <mem.h>

/*----------------------------------------------------------------------
 * delete_to_end_of_line - supprimer Ch de la position courante a la fin
 *----------------------------------------------------------------------
 */
delete_to_end_of_line(wp)
struct Win *wp;
{
    int nb = wp->leftCh + wp->curX;
/*    while (wp->linebuf[i] != EOS)
           wp->linebuf[i++] = EOS;
    wprintln(wp,wp->current);*/
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                wp->ink|wp->paper,
                fillstr);

    wp->fflag |= FF_DIRTY;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * fill_line - remplir tout ou partie du buffer ligne avec Ch
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
fill_line(line,nb,Ch)
char *line;
int nb;
char Ch;
{
    while (nb-- > 0)
         *line++ = Ch;
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * erase_trailing_blank - supprimer d'eventuels caracteres ' 'en fin de ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
erase_trailing_blank(line)
char *line;
{
    int i;
    i = lnlen(line);
    while (line[--i] == BLANK)
           line[i] = EOS;
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getnextline - retourne la position de la N ieme ligne apres offline
 *               ou bien la position de la derniere ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getnextline(wp,offline,n)
struct Win *wp;
unsigned offline;
{
    unsigned X;
    while ((n-- > 0) && ((X = lnnext(wp,offline)) != offline))
            offline = X;
    return(offline);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getprevline - retourne la position de la N ieme ligne avant offline
 *               ou bien la position de la premiere ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getprevline(wp,offline,n)
struct Win *wp;
unsigned offline;
int n;
{
    unsigned X = offline;
    while (n-- > 0)
           if ((X = lnprev(wp,X)) == 0) break;
    return(X);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * set_win - rafraichir la fenetre entierement
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION set_win(wp,debug)
struct Win *wp;
unsigned debug;
{
    int nb, lig, attrib,att,flagatt;
    unsigned pt;

    lig = 0 ;
    pt = wp->topPage;
    att = wp->ink|wp->paper;
    for ( ; (lig < wp->nline) && (pt <= wp->bottom); pt += lnlen(wp,pt) + 1, lig++) {
/*         if ((wp->fflag & FF_BLOCKDEF) && ISINBLK(wp,pt) )
               attrib  = wp->blockatt;
         else */ attrib  = att;

         if ((nb = INF(lnlen(wp,pt) - wp->leftCh, wp->ncol)) > 0)
              _wstringDB( wp->ul_x + 1,
                          wp->ul_y + 1 + lig,
                          nb,
                          NEWATT,/*flagatt,*/
                          attrib,/*wp->ink|wp->paper,*/
                          wp->bigbuf + pt + wp->leftCh);  /* sauter les Ch cach‚s */
         else
              nb = 0;

         /* completer avec des blancs */
         if (nb < wp->ncol)
             _wstringDB( wp->ul_x + 1 + nb,
                         wp->ul_y + 1 + lig,
                         wp->ncol - nb,
                         NEWATT,/*flagatt,*/
                         attrib,/*wp->ink|wp->paper,*/
                         fillstr);

         /* gestion marquage Block & Paste */
/*         put_att_BP(wp,pt,lig);*/
    }

    /*  si on est en fin de fichier, remplir a blanc les lignes suivantes */
    while (lig < wp->nline)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + 1 + lig++,
                     wp->ncol,
                     NEWATT,/*flagatt,*/
                     wp->ink|wp->paper,
                     fillstr);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnlen - calcul la taille d'une ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lnlen(wp,offline)
struct Win *wp;
unsigned offline;
{
   char c;
   unsigned i = offline;
   while ((c = wp->bigbuf[i++]) != '\n'&& c != EOFM) ;
   return(i - (offline + 1));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnbuflen - calcul la taille de la ligne tampon
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lnbuflen(wp)
struct Win *wp;
{
   char c;
   int i = 0;
   while (wp->linebuf[i++] != EOS) ;
   return(i-1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lncpy - copier une ligne ds le tampon ligne (delimiteur = EOS = '\n')
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lncpy(wp,dest,offline)
struct Win *wp;
char *dest;
unsigned offline;
{
   char c;
   while ((c = wp->bigbuf[offline++]) != EOS && c != EOFM);
           *dest++ = c;
   *dest = EOS;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnprev - retourne l'offset de la ligne precedente
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned lnprev(wp,offline,debug)
struct Win *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X = offline;
   if (X > 1)             X--;
   else                   return(0);
   while (X >= 1) {
          if (wp->bigbuf[X - 1] == '\n') {
              return(X);
          }
          X--;
   }
   return(0); /* offset de debut fichier */

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getlast_ln - retourne l'offset de la derniere ligne (A  TERMINER)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getlast_ln(wp)
struct Win *wp;
{
   unsigned pos = wp->fsize-1;  /* le dernier Ch est EOFM */

   while (pos > 0 && wp->bigbuf[pos - 1] != '\n') pos--;
   return(pos);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getlast_line_no - retourne le nb de lignes
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
getlast_line_no(wp)
struct Win *wp;
{
   int i;
   unsigned pos = wp->fsize - 1;
   i = 1;
   while (pos > 0)
          if (wp->bigbuf[pos--] == '\n')
              i++;
   return(i);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnnext - retourne l'offset de la ligne suivante
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned lnnext(wp,offline)
struct Win *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X;
   if (((X = lnlen(wp,offline) + 1) + offline) > wp->fsize) /*>= wp->fsize)*/
        return(offline); /* on est sur la derniere ligne */
   return(offline + X);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * initWin : initialiser une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION initWin(wp,ul_x,ul_y,ncols,nlines,ink,paper,wborder,batt)
struct Win *wp;
{
   wp->fflag      = FF_INSERT;
   wp->nline      = nlines;
   wp->leftCh     = 0;
   wp->curX       = wp->curY = 0;
   wp->wborder    = wborder;
   wp->ink        = ink;
   wp->paper      = paper;
   wp->ul_x       = ul_x;
   wp->ul_y       = ul_y;
   wp->ncol       = ncols;
   wp->blockatt   = batt;

   wp->current = wp->topPage = wp->bottom = 0;
   wp->current_line_no = wp->topPage_line_no = 1;

   wp->topBlock = wp->bottomBlock = 0;
   wp->topPaste = wp->bottomPaste = 0;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * clrwin Ä clear window
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION clrwin(wp)
struct Win *wp;
{
       int i;
       _clrwin(   wp->ul_x+1,
  	          wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * scrollwinUP Ä scrolling window UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION scrollwinUP(wp)
struct Win *wp;
{
       _window_up(1,wp->ul_x+1,
  	          wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * scrollwinDWN Ä scrolling window DOWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION scrollwinDWN(wp)
struct Win *wp;
{
       _window_down(1,wp->ul_x+1,
  	            wp->ul_y+1,
                    wp->ul_x + wp->ncol,
                    wp->ul_y + wp->nline,
                    wp->ink|wp->paper);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_box - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_box(wp)
struct Win *wp;
{
   int ps,i,j,tl;
   char   Vchar,Hchar;
   unsigned att;
   unsigned char ulc,llc,urc,lrc;

   ulc = 'Õ'; urc = '¸'; llc = 'Ô'; lrc = '¾';/**/
   Vchar = '³'; Hchar = 'Í';
   att = wp->wborder;

   /* COINS */
   *(Pvideo+((wp->ul_y)*80) + wp->ul_x) = (att<<8)|(unsigned char)ulc;
   *(Pvideo+((wp->ul_y)*80) + wp->ul_x + wp->ncol+1) = (att<<8)|(unsigned char)urc;
   *(Pvideo+((wp->ul_y+wp->nline+1)*80) + wp->ul_x) = (att<<8)|llc;
   *(Pvideo+((wp->ul_y+wp->nline+1)*80) + wp->ul_x+wp->ncol+1) = (att<<8)|lrc;

   /* HORIZONTALES */
   for (i= wp->ul_x+1; i <= wp->ul_x+wp->ncol; i++) {
        *(Pvideo+((wp->ul_y)*80 + i)) = (att<<8)|(unsigned char)Hchar;
        *(Pvideo+(wp->ul_y+wp->nline+1)*80 + i) = (att<<8)|(unsigned char)Hchar;
   }

   /* VERTICALES */
   for (i = wp->ul_y+1; i <= wp->ul_y+wp->nline;i++ ) {
        *(Pvideo+((i*80) + wp->ul_x)) = (att<<8)|(unsigned char)Vchar;
        *(Pvideo+((i*80) + wp->ul_x+wp->ncol+1)) = (att<<8)|(unsigned char)Vchar;
   }
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_up - ED arrow UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_up(wp)
struct Win *wp;
{
   if (wp->current == 0)  /* debut de fichier */
       return(-1);

   wp->current_line_no--;
   if (wp->curY == 0) {         /* debut de page    */
       scrollwinDWN(wp);
       wp->current = lnprev(wp,wp->current);
       wp->topPage = wp->current;
       wp->topPage_line_no = wp->current_line_no;
       wprintln(wp,wp->current);
       /* RAJOUTER PLUS TARD LA GESTION DES ATTRIBUTS POUR BLK ET PASTE */
   }
   else
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY - 1);
/*   put_att_BP(wp,wp->current,wp->curY);*/
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_down - ED arrow DWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_down(wp)
struct Win *wp;
{
 /*  printf("C = %u  B = %u",wp->current,wp->bottom);*/
   if (wp->current >= wp->bottom) { /* fin de fichier */
       printf("cno = %d  bno = %d ",wp->current_line_no,wp->bottom_line_no);
       return(-1);
   }
   wp->current_line_no++;
   if (wp->curY == wp->nline - 1) {    /* fin de page    */
       scrollwinUP(wp);
       wp->current = lnnext(wp,wp->current);
       wp->topPage = lnnext(wp,wp->topPage);
       wp->topPage_line_no++;
       wprintln(wp,wp->current);
       /* RAJOUTER PLUS TARD LA GESTION DES ATTRIBUTS POUR BLK ET PASTE */
   }
   else
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY + 1);
/*   put_att_BP(wp,wp->current,wp->curY);*/
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_right - ED arrow RIGHT
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_right(wp)
struct Win *wp;
{
   if ((wp->leftCh + wp->curX) >= NB_CAR_PER_LINE)
        return(-1);

   if (wp->curX < wp->ncol - 1) {
       wmgotoxy(wp,wp->curX + 1,wp->curY);
       return(0);
   }
   else {
       wp->leftCh++;
       set_win(wp);
   }
   return(1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_left - ED arrow LEFT
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_left(wp)
struct Win *wp;
{
   if ((wp->leftCh + wp->curX) == 0)
        return(-1);

   if (wp->curX > 0) {
       wmgotoxy(wp,wp->curX - 1,wp->curY);
       return(0);
   }
   else {
       wp->leftCh--;
       set_win(wp);
   }
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * page_up - ED Page UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION page_up(wp)
struct Win *wp;
{
   int down,up;

   if (wp->current == 0)
       return;

   if (wp->topPage == 0) {
       /* debut fichier : se placer sur la 1ere ligne */
       wmgotoxy(wp,wp->curX,0);
       wp->current_line_no = 1;
       return;
   }
   if (wp->topPage_line_no - wp->nline < 0) {
       /* On est ds la 1ere Page fichier : placer le sommet de page
        * en debut fichier, puis positionner la ligne courante
        * au meme endroit
        */
/*       printf("TOPLINE = %d ",wp->topPage_line_no);
       getch();*/
       wp->topPage = wp->current = 0;
       wp->topPage_line_no = 1;
       wp->current = getnextline(wp,wp->current,wp->curY);
       wp->current_line_no = wp->curY + 1;
   }
   else {
       /* cas general : remonter d'une page */
/*       printf("TOPPAGE AVANT = %u  NLINE = %d",wp->topPage,wp->nline-1);
       getch();*/

       wp->topPage = getprevline(wp,wp->topPage,wp->nline - 1);/*+1);*/
/*       printf("TOPPAGE APRES = %u ",wp->topPage);
       getch();*/

       wp->topPage_line_no -= wp->nline - 1;
       wp->current = getprevline(wp,wp->current,wp->nline - 1);/*+1);*/
       wp->current_line_no -= wp->nline - 1;
   }
   set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * page_down - ED Page DWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION page_down(wp)
struct Win *wp;
{
   int down,up,y;

   if (wp->current == wp->bottom)
       return;

   if (wp->topPage_line_no + wp->nline >= wp->bottom_line_no) {
       /* on est sur la derniere page : placer le sommet de page et
        * la ligne courante sur la derniere ligne
        */
       wp->topPage = wp->current = wp->bottom;
       wp->topPage_line_no = wp->current_line_no = wp->bottom_line_no;
       wgotoxy(wp,wp->curX,0);
   }
   else {
       /* cas general : placer le sommet de page une page plus loin et
        * ajuster la position de la ligne courante
        */
       wp->topPage = getnextline(wp,wp->topPage,wp->nline - 1);/* + 1);*/
       wp->topPage_line_no += wp->nline - 1;

       if (wp->current_line_no + wp->nline - 1/* + 1*/ >= wp->bottom_line_no)
           wgotoxy(wp,wp->curX,wp->bottom_line_no - wp->topPage_line_no);

       wp->current = getnextline(wp,wp->current,wp->nline - 1);/* + 1);*/
       wp->current_line_no += wp->nline - 1;
   }
   set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * line_end - ED line END
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION line_end(wp)
struct Win *wp;
{
  int nb;

  if ((nb = lnlen(wp,wp->current) - wp->leftCh) < wp->ncol) {
        if (nb > 0)
            /* se placer simplement en fin
             * de ligne
             */
            wgotoxy(wp,nb,wp->curY);
        else {
            /* la ligne est petite et la
             * position courante est grande
             */
            wp->leftCh = lnlen(wp,wp->current);
            wgotoxy(wp,0,wp->curY);
            set_win(wp);
        }
  }
  else {
       /* la fin de ligne est plus loin que la
        * taille de la fenetre
        */
       wp->leftCh = lnlen(wp,wp->current) - wp->ncol + 1;
       wgotoxy(wp,wp->ncol-1,wp->curY);
       set_win(wp);
  }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * line_home - ED line HOME
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION line_home(wp)
struct Win *wp;
{
  int nb;

  if (wp->leftCh > 0) {
      wp->leftCh = 0;
      set_win(wp);
  }
  wmgotoxy(wp,0,wp->curY);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wmgotoxy Ä se positionner dans la page avec mise a
 *            jour de la ligne courante en memoire
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wmgotoxy(wp,x,y)
struct Win *wp;
{
   int aux;

   if (x >= wp->ncol)    x = wp->ncol - 1;
   if (y >= wp->nline)   y = wp->nline - 1;
   wp->curX = x;
   if (wp->curY != y) {
       if (wp->curY > y)
           wp->current = getprevline(wp,wp->current,wp->curY - y);
       else {
/*           putchar('@');*/
           wp->current = getnextline(wp,wp->current,y - wp->curY);
       }
       wp->curY = y;
       wprintln(wp,wp->current);  /*** VERIF  A SAQUER ***/
/*       printf("CURRENT = %d\n",wp->current);*/
   }
   gotoxy(wp->ul_x+x+1,wp->ul_y+y+1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wgotoxy Ä se positionner dans la page sans mise a
 *           jour de la ligne courante en memoire
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wgotoxy(wp,x,y)
struct Win *wp;
{
   int aux;

   if (x >= wp->ncol)    x = wp->ncol - 1;
   if (y >= wp->nline)   y = wp->nline - 1;
   wp->curX = x;
   wp->curY = y;
   gotoxy(wp->ul_x+x+1,wp->ul_y+y+1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintln - ecrire une ligne  (ou une partie) dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintln(wp,offline)
struct Win *wp;
unsigned offline;
{
    int attrib , nb;
/*    if (ISINBLK(wp,pt) && (wp->fflag & FF_BLOCKDEF))
          attrib  = wp->blockatt;
    else*/  attrib  = wp->ink|wp->paper;

    if ((nb = INF(lnlen(wp,offline) - (wp->leftCh), wp->ncol)) > 0)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + wp->curY + 1,
                     nb,
                     NEWATT,
                     attrib,
                     wp->bigbuf + offline + wp->leftCh);  /* sauter les Ch cach‚s */
    else
         nb = 0;

    /* si la ligne a une taille inferieure a la fenetre, completer a blanc */
    if (nb < wp->ncol)
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                attrib,
                fillstr);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintlnbuf - ecrire la ligne tampon dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintlnbuf(wp)
struct Win *wp;
{
    int attrib , nb;
/*    if (ISINBLK(wp,pt) && (wp->fflag & FF_BLOCKDEF))
          attrib  = wp->blockatt;
    else*/  attrib  = wp->ink|wp->paper;

    if ((nb = INF(lnbuflen(wp) - (wp->leftCh), wp->ncol)) > 0)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + wp->curY + 1,
                     nb,
                     NEWATT,
                     attrib,
                     wp->linebuf + wp->leftCh);  /* sauter les Ch cach‚s */
    else
         nb = 0;

    /* si la ligne a une taille inferieure a la fenetre, completer a blanc */
    if (nb < wp->ncol)
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                attrib,
                fillstr);
}


/*------------------------------------------------------
 * winsln() - inserts a line
 *------------------------------------------------------
 */
winsln(wp,insert)
struct Win *wp;
{
    int i, lcb;

    if (wp->fflag & FF_LINE_UPDATE) {
        /* prise en compte modifications de la ligne courante */
        insert_lnbuf(wp);
        wp->fflag &= ~FF_LINE_UPDATE;
    }

    if (!(wp->fflag & FF_INSERT) && insert == ON_MODE) { /* mode Overwrite */
        if (wp->leftCh) {
            wp->leftCh = 0;
            set_win(wp);
        }
        arrow_down(wp);
        wgotoxy(wp,0,wp->curY);
        return;
    }

    /* tester si la ligne a ete coupee */
    if (wp->curX + wp->leftCh <= lnlen(wp,wp->current))
        delete_to_end_of_line(wp);

    /* mode Insert */
    newline(wp);

    /* ajouter indentation automatique */

    /* GESTION BLOCK */
/*    if ((wp->fflag & FF_BLOCKDEF) && insert == ON_MODE) {
        if (wp->current == wp->bottomBlock &&
            wp->lastCh > (wp->curX + wp->leftCh)) {
            wp->lastCh -= (wp->curX + wp->leftCh);
            wp->bottomBlock = pt;
        }
        if (wp->current == wp->topBlock &&
            wp->firstCh > (wp->curX + wp->leftCh)) {
            wp->firstCh -= (wp->curX + wp->leftCh);
            wp->topBlock = pt;
        }
    }*/

    if (wp->curY < wp->nline - 1) {
        _window_down(1,wp->ul_x + 1,
  	               wp->ul_y + 1 + (wp->curY + 1),
                       wp->ul_x + wp->ncol,
                       wp->ul_y + wp->nline,
                       wp->ink|wp->paper);
        wp->curY++;
    }
    else {
        _window_up(  1,wp->ul_x + 1,
  	               wp->ul_y + 1,
                       wp->ul_x + wp->ncol,
                       wp->ul_y + wp->nline,
                       wp->ink|wp->paper);
        wp->topPage = lnnext(wp,wp->topPage);
        wp->topPage_line_no++;
    }
    wgotoxy(wp,0,wp->curY);
    if (wp->leftCh == 0) {
        wprintln(wp,wp->current);
/*        put_att_BP(wp,wp->current->prev,wp->curY - 1);
        put_att_BP(wp,wp->current,wp->curY);*/
    }
    else {
        wp->leftCh = 0;
        set_win(wp);
    }
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * essai - tente d'inserer des caracteres dans le buffer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION essai(wp)
struct Win *wp;
{
    int i;
    unsigned n = wp->current + wp->curX;
    memmove(wp->bigbuf + n + 3,
           wp->bigbuf + n,
           wp->fsize - n);
    for (i=0;i<3;i++)
      wp->bigbuf[n+i] = '@';
    wp->fsize += 3;
    if (wp->bottom != wp->current)
        wp->bottom += 3;
    set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * newline - ajouter EOS dans le bigbuf
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION newline(wp)
struct Win *wp;
{
    int i;
    unsigned n = wp->current + wp->curX + wp->leftCh;
    if (wp->fsize + 1 >= BIGBUF_SIZE - 1) {
        printf("OUT OF MEMORY");
        return;
    }
    memmove(wp->bigbuf + n + 1,
            wp->bigbuf + n,
            wp->fsize  - n);
    wp->bigbuf[n] = EOS;
    wp->fsize++;
    if (wp->bottom != wp->current)
        wp->bottom++;
    wp->current += n;
    wp->current_line_no++;
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * insert_lnbuf - tente d'inserer des caracteres dans le buffer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION insert_lnbuf(wp)
struct Win *wp;
{
    int i, Plen, Nlen, diff;
    unsigned pos;
    Plen = lnlen(wp,wp->current) + 1;
    Nlen = lnbuflen(wp) + 1;
    if ((diff = Nlen - Plen) == 0) {
        /* la ligne a la meme taille : copier sans decaler */
        copy_lnbuf_to_bigbuf(wp,Plen);
        return;
    }
    /* la ligne modifiee n'a pas la meme taille que celle dans bigbuf */
    if (wp->fsize + diff >= BIGBUF_SIZE - 1) {
        printf("OUT OF MEMORY");
        return;
    }
    pos = wp->current + Plen;
    memmove(wp->bigbuf + pos + diff,
           wp->bigbuf + pos,
           wp->fsize - pos);
    copy_lnbuf_to_bigbuf(wp,Nlen);
    wp->fsize += diff;
    if (wp->bottom != wp->current)
        wp->bottom += diff;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * copy_lnbuf_to_bigbuf - copier le buffer ligne a la place de la ligne
 *                        courante dans bigbuf
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION copy_lnbuf_to_bigbuf(wp,size)
struct Win *wp;
{
   char *line;
   char far *pos;
   pos = wp->bigbuf + wp->current;
   line = wp->linebuf;
   while (size-- > 0)
          *pos++ = *line++;
}
 Impossible de copier un fichier sur lui-mˆme
        0 fichier(s) copi‚(s)
/* DISPLAY.C  :   fichier de gestion de l'affichage */
#include "xed.h"

char lineTab[NB_CAR_PER_LINE * 2];
		
/*----------------------------------------------------------------------
 * delete_to_end_of_line - supprimer Ch de la position courante a la fin
 *----------------------------------------------------------------------
 */
delete_to_end_of_line(wp)
struct Win *wp;
{
    int nb = wp->curX;

/*    while (linebuf[i] != EOS)
           linebuf[i++] = EOS;
    wcurrln(wp);*/
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                NEWATT,
                wp->ink|wp->paper,
                fillstr);

    fflag |= FF_DIRTY;
}

/*----------------------------------------------------------------------
 * add_blank - ajouter des blancs pour combler l'ecart entre fin de ligne
 *             et position courante sur la ligne
 *----------------------------------------------------------------------
 */
add_blank(wp,from,to)
struct Win *wp;
{
    unsigned pos;
    int i, nb = to - (i = from);

    while (i < to)
           linebuf[i++] = BLANK;

    if (fflag & FF_BLOCKDEF) {
        if (((pos = current + from) > topBlock) &&
             (pos < bottomBlock))
              bottomBlock += nb;
        else
             if (pos <= topBlock) {
/*                 topBlock -= nb;
                 bottomBlock -= nb;*/
                 topBlock += nb;
                 bottomBlock += nb;
             }
    }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * fill_line - remplir tout ou partie du buffer ligne avec Ch
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
fill_line(line,nb,Ch)
char *line;
int nb;
char Ch;
{
    memset(line, Ch, nb);
/*    while (nb-- > 0)
         *line++ = Ch;*/
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * erase_trailing_blank - supprimer d'eventuels caracteres ' 'en fin de ligne
 *                        retourne la nouvelle taille
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
erase_trailing_blank(wp)
struct Win *wp;
{
    int i;
    i = _lnlen(linebuf);
    while (linebuf[--i] == BLANK)
           linebuf[i] = EOS;
    return(i+1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getnextline - retourne la position de la N ieme ligne apres offline
 *               ou bien la position de la derniere ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getnextline(wp,offline,n)
struct Win *wp;
unsigned offline;
{
    unsigned X;
    while ((n-- > 0) && ((X = lnnext(wp,offline)) != NIL_LN))
            offline = X;
    return(offline);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getprevline - retourne la position de la N ieme ligne avant offline
 *               ou bien la position de la premiere ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getprevline(wp,offline,n)
struct Win *wp;
unsigned offline;
int n;
{
    unsigned X,Y;
    X = Y = offline;
/*    while (n-- > 0)
           if ((X = lnprev(wp,X)) == 0) break;
    return(X);*/
    while (n-- > 0)
           if ((X = lnprev(wp,X)) == NIL_LN) break;
           else Y = X;
    return(Y);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * set_win - rafraichir la fenetre entierement
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION set_win(wp,debug)
struct Win *wp;
unsigned debug;
{
    int nb, lig, len, attrib, att, flagatt;
    unsigned pt;

    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,ERASE);

    for (lig = 0, pt = topPage, att = wp->ink|wp->paper;
         (lig < wp->nline) && (pt <= bottom);
         pt += len + 1, lig++) {
    	 /* formater chaque ligne dans le buffer ligne avant ecriture sur l'ecran */
	 len = lnlen(wp, pt);
	 formatt_line(lineTab, bigbuf, pt, len, BLANK, NB_CAR_PER_LINE * 2);
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + 1 + lig,
                     wp->ncol,
                     OLDATT,
                     att,
                     lineTab + wp->leftCh);  /* sauter les Ch cach‚s */

         /* gestion marquage Block */
          put_att_BP(wp,pt,lig,len);
    }

    /*  si on est en fin de fichier, remplir a blanc les lignes suivantes */
    while (lig < wp->nline)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + 1 + lig++,
                     wp->ncol,
                     NEWATT,
                     att,
		     fillstr);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * formatt_line - formatter une ligne avec les caracteres de Tabulation
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
formatt_line(dest, src, pt, len, fillCh, ninit)
char *dest, *src; /* "dest" doit etre lineTab ou linebuf */igne
 *                        retourne la nouvelle taille
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
erase_trailing_blank(wp)
struct Win *wp;
{
    int i;
    i = _lnlen(linebuf);
    while (linebuf[--i] == BLANK)
           linebuf[i] = EOS;
    return(i+1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getnextline - retourne la position de la N ieme ligne apres offline
 *               ou bien la position de la  ligne */
/*	      strncpy(&dest[i], fillstr, 8 - (k = i % 8));
	      i += 8 - k - 1; /* -1 because l'incrementation de boucle */
	      strncpy(&dest[i], tab, 1);
	 }
         else if (dest[i] == EOS)
                  dest[i] = BLANK;
    dest[i-1] = fillCh;
    return(i-1); /* retourner le nb de caracteres reels */
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * linelen - calcul la taille d'une ligne
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
linelen(wp)
struct Win *wp;
{
/*   if (fflag & FF_LINE_UPDATE)      return(_lnlen(linebuf));
   else                             return(lnlen(wp,current));*/
   _lnlen(linebuf);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnlen - calcul la taille d'une ligne du big buffer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lnlen(wp,offline)
struct Win *wp;
unsigned offline;
{
   char c;
   unsigned i = offline;

/*   while (((c = bigbuf[i++]) != EOS) && (c != EOFM)) ;
   return(i - (offline + 1));*/
   if (offline == bottom) {
       while (((c = bigbuf[i++]) != EOS) && (c != EOFM)) ;
       return(i - (offline + 1));
/*       return(fsize - (offline + 1));*/
   }
   else
       return(_lnlen(&bigbuf[offline]));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnbuflen - calcul la taille de la ligne tampon
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lnbuflen2(wp)
struct Win *wp;
{
   char c;
/*   int i = 0;
   while (linebuf[i++] != EOS) ;
   return(i-1);*/
   return(_lnlen(linebuf));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lncpy - copier la ligne courante ds le tampon ligne (delimiteur = EOS = '\n')
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lncpy(wp)
struct Win *wp;
{
   char c;
   int i;
   unsigned offline = current;

   i = 0;
   while ((c = bigbuf[offline++]) != EOS && c != EOFM)
           linebuf[i++] = c;
   linebuf[i] = EOS;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnprev - retourne l'offset de la ligne precedente
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned lnprev(wp,offline,debug)
struct Win *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X = offline;
/*   if (X >= 1)             X--;
   else                    return(NIL_LN);
   while (X >= 1) {
          if (bigbuf[X - 1] == EOS) {
              return(X);
          }
          X--;
   }*/
   if (X == 0)   return(NIL_LN);
   else          X--;
   while (X >= 1) {
          if (bigbuf[X-1] == EOS) {
              return(X);
          }
          X--;
   }
   return(0); /* offset de debut fichier */
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getlast_ln - retourne l'offset de la derniere ligne (A  TERMINER)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned getlast_ln(wp)
struct Win *wp;
{
   unsigned pos = fsize-1;  /* le dernier Ch est EOFM */

   while (pos > 0 && bigbuf[pos - 1] != '\n') pos--;
   return(pos);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * get_line_no - retourne le nb de lignes correspondant a l'offset
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
get_line_no(offset)
unsigned offset;
{
   int i, j;
/*   unsigned pos = fsize - 1;*/
   unsigned pos = offset; /*offset - 1;*/

   if (offset == 0) return(1);
   i = j = 0;
/*   while (pos > 0)
          if (bigbuf[pos--] == EOS)
              i++;
   if (bigbuf[0] == EOS) i++;*/
/*   while (pos-- > 0)
          if (bigbuf[j++] == EOS)
              i++;*/
   i = _cpt_EOS(&bigbuf[0], offset); /*offset+1);*/
   return(i+1);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getnln - retourne le nb de lignes entre deux lignes
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
getnln(wp,first,last)
struct Win *wp;
unsigned first,last;
{
   int i;
   unsigned pos = fsize - 1;
   i = 0;
   while (first < last)
          if (bigbuf[first++] == EOS)
              i++;
   return(i);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lnnext - retourne l'offset de la ligne suivante
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
unsigned lnnext(wp,offline)
struct Win *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X;
   if (((X = lnlen(wp,offline) + 1) + offline) >= fsize)
        return(NIL_LN);/*return(offline); /* on est sur la derniere ligne */
   return(offline + X);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * initWin : initialiser une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION initWin(wp,title,ul_x,ul_y,ncols,nlines,ombrage,ink,paper,wborder,batt)
struct Win *wp;
char *title;
{
   wp->nline      = nlines;
   wp->leftCh     = 0;
   wp->curX       = wp->curY = 0;
   wp->wborder    = wborder;
   wp->ink        = ink;
   wp->paper      = paper;
   wp->ul_x       = ul_x;
   wp->ul_y       = ul_y;
   wp->ncol       = ncols;
   wp->wpushed    = FALSE;
   wp->title      = title;
   wp->save 	  = NULL;
   wp->f_ombrage  = ombrage;
   blockatt       = batt;
   fillCh         = 0;

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * winPopUp - ouvrir / fermer une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
winPopUp(wp, mode, borderType)
struct Win *wp;
{
    if (mode == W_OPEN) {
        /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
	if (wp->save == NULL) /* check presence tampon */
	    if ((wp->save = malloc(2 * (wp->ncol + 4) * (wp->nline + 3))) == NULL)
		 exit(0);  /* !!!!!!!!!!!!!!! ?????????? */

    	wp->wpushed = TRUE;
	saveScreen(wp, WINDOW);
        write_wbox(wp, borderType);
        clrwin(wp, BLANK);
    }
    else {
        wp->wpushed = FALSE;
	if (wp->save != NULL) {
	    restoreScreen(wp, WINDOW);
	    free(wp->save);
	    wp->save = NULL;
	}
    }
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * clrwin Ä clear window
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION clrwin(wp, Ch)
struct Win *wp;
unsigned char Ch;
{
       int i;
       _clrwin(   wp->ul_x+1,
  	          wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper,
                  Ch);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * scrollwinUP Ä scrolling window UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION scrollwinUP(wp)
struct Win *wp;
{
       _window_up(1,wp->ul_x+1,
  	          wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * scrollwinDWN Ä scrolling window DOWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION scrollwinDWN(wp)
struct Win *wp;
{
       _window_down(1,wp->ul_x+1,
  	            wp->ul_y+1,
                    wp->ul_x + wp->ncol,
                    wp->ul_y + wp->nline,
                    wp->ink|wp->paper);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_wbox - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_wbox(wp, type)
struct Win *wp;
{
      write_box(wp->title, wp->ul_x, wp->ul_y, wp->ncol, wp->nline, wp->wborder, type);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_up - ED arrow UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_up(wp)
struct Win *wp;
{
   if (current == 0)  /* debut de fichier */
       return(-1);

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   current_line_no--;
   if (wp->curY == 0) {         /* debut de page    */
       scrollwinDWN(wp);
       current = lnprev(wp,current);
       topPage = current;
       topPage_line_no--;
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
       wcurrln(wp);
   }
   else {
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY - 1);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
   }
   put_att_BP(wp,current,wp->curY,lnlen(wp,current));

   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_down - ED arrow DWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_down(wp)
struct Win *wp;
{
   if (current >= bottom) { /* fin de fichier */
/*       printf("cno = %d  bno = %d ",current_line_no,bottom_line_no);*/
       return(-1);
   }
   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   current_line_no++;
   if (wp->curY == wp->nline - 1) {    /* fin de page    */
       scrollwinUP(wp);
       current = lnnext(wp,current);
       topPage = lnnext(wp,topPage);
       topPage_line_no++;
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
       wcurrln(wp);
   }
   else {
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY + 1);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
   }
   put_att_BP(wp,current,wp->curY,lnlen(wp,current));
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_right - ED arrow RIGHT
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_right(wp)
struct Win *wp;
{
   if ((wp->leftCh + wp->curX) >= NB_CAR_PER_LINE - 1) {
        beep(1000);
        return(-1);
   }
   if (wp->curX < wp->ncol - 1) {
       wmgotoxy(wp,wp->curX + 1,wp->curY);
       return(0);
   }
   else {
       wp->leftCh++;
       set_win(wp);
   }
   return(1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * arrow_left - ED arrow LEFT
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION arrow_left(wp)
struct Win *wp;
{
   if ((wp->leftCh + wp->curX) == 0)
        return(-1);

   if (wp->curX > 0) {
       wmgotoxy(wp,wp->curX - 1,wp->curY);
       return(0);
   }
   else {
       wp->leftCh--;
       set_win(wp);
   }
   return(1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * page_up - ED Page UP
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION page_up(wp)
struct Win *wp;
{
   int down,up;

   if (current == 0)
       return;

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   if (topPage == 0) {
       /* debut fichier : se placer sur la 1ere ligne */
       wmgotoxy(wp,wp->curX,0);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
       current_line_no = 1;
       return;
   }
   if (topPage_line_no - wp->nline < 0) {
       /* On est ds la 1ere Page fichier : placer le sommet de page
        * en debut fichier, puis positionner la ligne courante
        * au meme endroit
        */
/*       printf("TOPLINE = %d ",topPage_line_no);
       getch();*/
       topPage = current = 0;
       topPage_line_no = 1;
       current = getnextline(wp,current,wp->curY);
       current_line_no = wp->curY + 1;
   }
   else {
       /* cas general : remonter d'une page */
       topPage = getprevline(wp,topPage,wp->nline - 1);/*+1);*/
       topPage_line_no -= wp->nline - 1;
       current = getprevline(wp,current,wp->nline - 1);/*+1);*/
       current_line_no -= wp->nline - 1;
   }
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
   set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * page_down - ED Page DWN
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION page_down(wp)
struct Win *wp;
{
   int down,up,y;

   if (current == bottom)
       return;

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   if (topPage_line_no + wp->nline >= bottom_line_no) {
       /* on est sur la derniere page : placer le sommet de page et
        * la ligne courante sur la derniere ligne
        */
       topPage = current = bottom;
       topPage_line_no = current_line_no = bottom_line_no;
       wgotoxy(wp,wp->curX,0);
   }
   else {
       /* cas general : placer le sommet de page une page plus loin et
        * ajuster la position de la ligne courante
        */
       topPage = getnextline(wp,topPage,wp->nline - 1);/* + 1);*/
       topPage_line_no += wp->nline - 1;

       if (current_line_no + wp->nline - 1/* + 1*/ >= bottom_line_no) {
           wgotoxy(wp,wp->curX,bottom_line_no - topPage_line_no);
           current_line_no = bottom_line_no;
       }
       else
           current_line_no += wp->nline - 1;
       current = getnextline(wp,current,wp->nline - 1);/* + 1);*/
   }
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
   set_win(wp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * line_end - ED line END
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION line_end(wp)
struct Win *wp;
{
  int nb, len;
  /* si la ligne est en cours de modification
   * prendre la longueur du tampon
   */
  if (fflag & (FF_LINE_UPDATE|FF_TAB_ON_LINE))  len = _lnlen(linebuf);
  else                              		len = lnlen(wp,current);

  if ((nb = len - wp->leftCh) < wp->ncol) {
        if (nb > 0)
            /* se placer simplement en fin  de ligne */
            wgotoxy(wp,nb,wp->curY);
        else {
            /* la ligne est petite et la position courante est grande */
            wp->leftCh = len;
            wgotoxy(wp,0,wp->curY);
            set_win(wp);
        }
  }
  else {
       /* la fin de ligne est plus loin que la
        * taille de la fenetre
        */
       wp->leftCh = len - wp->ncol + 1;
       wgotoxy(wp,wp->ncol-1,wp->curY);
       set_win(wp);
  }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * line_home - ED line HOME
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION line_home(wp)
struct Win *wp;
{
  int nb;

  if (wp->leftCh > 0) {
      wp->leftCh = 0;
      set_win(wp);
  }
  wmgotoxy(wp,0,wp->curY);
/*  printf("cur = %d   bot = %d ",current_line_no,bottom_line_no);*/
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wmgotoxy Ä se positionner dans la page avec mise a
 *            jour de la ligne courante en memoire
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wmgotoxy(wp,x,y)
struct Win *wp;
{
   int aux;

   if (x >= wp->ncol)    x = wp->ncol - 1;
   if (y >= wp->nline)   y = wp->nline - 1;
   wp->curX = x;
   if (wp->curY != y) {
       if (wp->curY > y)
           current = getprevline(wp,current,wp->curY - y);
       else
           current = getnextline(wp,current,y - wp->curY);
       wp->curY = y;
/*       wcurrln(wp);  /*** VERIF  A SAQUER ***/
   }
   gotoxy(wp->ul_x+x+1,wp->ul_y+y+1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wgotoxy Ä se positionner dans la page sans mise a
 *           jour de la ligne courante en memoire
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wgotoxy(wp,x,y)
struct Win *wp;
{
   int aux;

   if (x >= wp->ncol)  {
/*       wp->leftCh += x - (wp->ncol - 1);*/
       x           = wp->ncol - 1;
   }
   if (y >= wp->nline)   y = wp->nline - 1;
   wp->curX = x;
   wp->curY = y;
   gotoxy(wp->ul_x+x+1,wp->ul_y+y+1);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wcurrln - ecrire la ligne courante
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wcurrln(wp)
struct Win *wp;
{
    int len, att;

    att = wp->ink|wp->paper;
    formatt_line(lineTab, linebuf, 0, len = _lnlen(linebuf), BLANK, NB_CAR_PER_LINE * 2);
    _wstringDB( wp->ul_x + 1,
                wp->ul_y + 1 + wp->curY,
                wp->ncol,
                OLDATT,
                att,
                lineTab + wp->leftCh);  /* sauter les Ch cach‚s */

    /* gestion marquage Block */
    put_att_BP(wp,current,wp->curY,len);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintln - ecrire une ligne  (ou une partie) dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintln(wp, pt)
struct Win *wp;
unsigned pt;
{
    int len, att;

    att = wp->ink|wp->paper;
    formatt_line(lineTab, bigbuf, pt, len = lnlen(wp, pt), BLANK, NB_CAR_PER_LINE * 2);
    _wstringDB( wp->ul_x + 1,
                wp->ul_y + 1 + wp->curY,
                wp->ncol,
                OLDATT,
                att,
                lineTab + wp->leftCh);  /* sauter les Ch cach‚s */

    /* gestion marquage Block */
    put_att_BP(wp,pt,wp->curY,len);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintln2 - ecrire une ligne  (ou une partie) dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintln2(wp,offline)
struct Win *wp;
unsigned offline;
{
    int attrib , len, nb;

    if (fflag & FF_LINE_UPDATE) {
        wprintlnbuf2(wp);
        return;
    }

    attrib  = wp->ink|wp->paper;
    len = lnlen(wp,offline);
    if ((nb = INF(len - wp->leftCh, wp->ncol)) > 0)
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + wp->curY + 1,
                     nb,
                     0,
                     attrib,
                     bigbuf + offline + wp->leftCh);  /* sauter les Ch cach‚s */
    else
         nb = 0;

    /* si la ligne a une taille inferieure a la fenetre, completer a blanc */
    if (nb < wp->ncol)
    _wstringDB( wp->ul_x + 1 + nb,
                wp->ul_y + wp->curY + 1,
                wp->ncol - nb,
                0,
                attrib,
                fillstr);
    put_att_BP(wp,offline,wp->curY,len);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wprintlnbuf2 - ecrire la ligne tampon dans la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION wprintlnbuf2(wp)
struct Win *wp;
{
}

/*------------------------------------------------------
 * winsch() - inserts a normal caracter in a line
 *------------------------------------------------------
 */
winsch(wp,Ch)
struct Win *wp;
int Ch;
{
     int i, p, line_pos, plen, k;
     char c;

     switch (Ch) {
     case  RETURN  :  winsln(wp,ON_MODE);return;
     case  BACKSP  :  wbacksp(wp);return;
     case  TAB	   :  /*if (wp->curX + wp->leftCh < lnlen(wp, lnprev(wp,current)))
     			  wgotoxy(wp,
     				  indent(wp,lnprev(wp,current) + wp->curX + wp->leftCh),
     				  wp->curY);*/
                      k = (wp->curX + wp->leftCh) % 8;
                      if ((wp->curX + wp->leftCh + (8 - k)) >= NB_CAR_PER_LINE - 1) {
                          wp->leftCh = NB_CAR_PER_LINE - wp->ncol;
                          set_win(wp);
                          wgotoxy(wp, wp->ncol - 1, wp->curY);
                          beep(1000);  /* BELL */
                          return;
                      }
                      if ((wp->curX += (8 - k)) < wp->ncol)
                          wgotoxy(wp, wp->curX, wp->curY);
                      else {
                          wp->leftCh += wp->curX - (wp->ncol - 1);
                          wgotoxy(wp, wp->ncol - 1, wp->curY);
                          set_win(wp);
                      }
     		      return;
     default       :
                     if ((line_pos = wp->curX + wp->leftCh) >= NB_CAR_PER_LINE - 1) {
                         beep(1000);  /* BELL */
                         return;
                     }

                     fflag |= FF_LINE_UPDATE;

                     /* tester si position courante apres fin de ligne */
                     if ((plen = _lnlen(linebuf)) < line_pos)
                          /* combler par des blancs */
                          add_blank(wp,plen,line_pos);

                     if (fflag & FF_INSERT) {

                         /*-------------*
                          * MODE INSERT *
                          *-------------*/

                         if (plen >= NB_CAR_PER_LINE - 1) {
                             beep(1000);  /* BELL */
                             return;
                         }
                         /* GESTION BLOCK */
                         if (fflag & FF_BLOCKDEF) {
                             if (current + wp->curX + wp->leftCh < topBlock) {
                                 topBlock++;
                                 bottomBlock++;
                             }
                             else   /* sup debut block */
                                 if (current + wp->curX + wp->leftCh < bottomBlock)
                                     bottomBlock++;
                         }
                         i = line_pos;

                         /* gestion decalage des carac a gauche */
                         if (linebuf[i] != EOS) {
                             while (linebuf[i++] != EOS);
                             while (i >= line_pos ) { /* decaler ! */
                                    linebuf[i+1] = linebuf[i];
                                    i--;
                             }
                             linebuf[line_pos] = Ch ;
                         }
                         else {  /* inserer apres fin de string */
                             linebuf[line_pos]    = Ch ;
                             linebuf[line_pos+1]  = EOS ;
                         }
                     }
                     else {

                         /*----------------*
                          * MODE OVERWRITE *
                          *----------------*/

                        if (wp->leftCh + wp->curX >= NB_CAR_PER_LINE - 1) {
                             winsln(wp,INSERT_MODE); /* COUILLE */
                             return;
                         }
                         linebuf[line_pos] = Ch;
/*                         if (wp->curX + 1 > wp->ncol - 1) {
                             wp->leftCh++;
                             set_win(wp);
                         }
                         else {
                              wgotoxy(wp,wp->curX + 1,wp->curY);*/
                             /*wprintlnbuf(wp);*/
                             /*put_att_BP(wp,current,wp->curY);*/
                       /*  }*/
                     }
/*                    if (fflag & FF_BLOCKDEF) {
                        /* inserer d'abord avant de placer les attributs */
/*                        if (fflag & FF_LINE_UPDATE)
                            insert_lnbuf(wp,NO_ERASE);
                    }*/
     }
     fflag |= FF_DIRTY;
/*     if (Ch == RETURN) {
         wcurrln(wp);
         return;
     }*/
     arrow_right(wp);
     wcurrln(wp);
     return(0);
}

/*------------------------------------------------------
 * wdelch() - supprime le Ch courant
 *------------------------------------------------------
 */
wdelch(wp)
struct Win *wp;
{
    int i,j;

    /* tester si buffer ligne deja charg‚ */
/*    if (!(fflag & FF_LINE_UPDATE)) {
         fill_line(linebuf,(NB_CAR_PER_LINE -  1) * 2,EOS);
         lncpy(wp);
         fflag |= FF_LINE_UPDATE;
     }*/
     fflag |= FF_LINE_UPDATE;

    if (linebuf[i = wp->curX + wp->leftCh] == EOS) {
        if (i == 0)
            wdeleteln(wp);
        else {
            arrow_down(wp);
            line_home(wp);
            wbacksp(wp);
        }
    }
    else {
       arrow_right(wp);
       wbacksp(wp);
    }
}


/*------------------------------------------------------
 * winsln() - inserts a line
 *------------------------------------------------------
 */
winsln(wp,insert)
struct Win *wp;
{
    int i, lcb, pos, id, len;
    unsigned pt;

    /* prise en compte modifications de la ligne courante */
    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,ERASE);

    if (!(fflag & FF_INSERT) && (insert == ON_MODE)) { /* mode Overwrite */
        if (wp->leftCh) {
            wp->leftCh = 0;
            set_win(wp);
        }
        arrow_down(wp);
        wgotoxy(wp,0,wp->curY);
        return;
    }
    /* Mode insert */

    /* tester si la ligne a ete coupee */
    if ((pos = wp->curX + wp->leftCh) <= (len = lnlen(wp,current)))
	delete_to_end_of_line(wp);
    else
	pos = len;

    /* GESTION BLOCK */
    if ((fflag & FF_BLOCKDEF) && (insert == ON_MODE)) {
	if (current + pos < topBlock) {
            topBlock++;
            bottomBlock++;
        }
        else
	    if (current + pos < bottomBlock)
                bottomBlock++;
     }

    pt = current;
    newline(wp,pos);

    if (wp->curY < wp->nline - 1) {
        _window_down(1,wp->ul_x + 1,
  	               wp->ul_y + 1 + (wp->curY + 1),
                       wp->ul_x + wp->ncol,
                       wp->ul_y + wp->nline,
                       wp->ink|wp->paper);
        wp->curY++;
    }
    else {
        _window_up(  1,wp->ul_x + 1,
  	               wp->ul_y + 1,
                       wp->ul_x + wp->ncol,
                       wp->ul_y + wp->nline,
                       wp->ink|wp->paper);
        topPage = lnnext(wp,topPage);
        topPage_line_no++;
    }
    /* GESTION DE L'INDENTATION */
    if (wp->leftCh == 0) {

	/* identation */
	wgotoxy(wp, id = indent(wp,lnprev(wp,current)), wp->curY);
        add_blank(wp, 0, id);
	formatt_line(linebuf+ id, bigbuf, current,
		     INF(lnlen(wp,current), (NB_CAR_PER_LINE * 2) - id), EOS,
		     (NB_CAR_PER_LINE * 2) - id);
	if (id)
	    fflag |= FF_LINE_UPDATE;
	wcurrln(wp);
        put_att_BP(wp,pt,wp->curY - 1,lnlen(wp,pt));

    }
    else {
	if ((id = indent(wp,lnprev(wp,current))) >= wp->ncol) {
	     wp->leftCh = id;
	     add_blank(wp, 0, id);
	     formatt_line(linebuf+ id, bigbuf, current,
			  INF(lnlen(wp,current), (NB_CAR_PER_LINE * 2) - id), EOS,
			  (NB_CAR_PER_LINE * 2) - id);
	     wgotoxy(wp, 0, wp->curY);
        }
        else {
	    wp->leftCh = 0;
	    add_blank(wp, 0, id);
	    formatt_line(linebuf+ id, bigbuf, current,
			 INF(lnlen(wp,current), (NB_CAR_PER_LINE * 2) - id), EOS,
			 (NB_CAR_PER_LINE * 2) - id);
	    wgotoxy(wp, id, wp->curY);
        }
	if (id)
	    fflag |= FF_LINE_UPDATE;
	set_win(wp);
    }
    fflag |= FF_DIRTY;
}

/*-------------------------------------------------------------
 * indent - calcule la position du 1er Char de la ligne donn‚e
 *-------------------------------------------------------------
 */
indent(wp,offline)
struct Win *wp;
unsigned offline;
{
   int i, j;
   i = j = lnlen(wp,offline);
   while (i > 0 && bigbuf[offline + (j - i)] == BLANK) i--;
   if (i <= 0) return(0);
   else        return(j - i);
}

/*------------------------------------------------------
 * wdeleteln() - supprimer la ligne courante
 *------------------------------------------------------
 */
wdeleteln(wp)
struct Win *wp;
{
    int ycur, i, len, bsize;
    unsigned pt, next, prev;

    fflag |= FF_DIRTY;

    if ((bottom == 0) || (current == bottom)) {
        wgotoxy(wp,0,wp->curY);
        fsize -= lnlen(wp,bottom);
        bigbuf[bottom] = EOFM;
        delete_to_end_of_line(wp);
        if ((fflag & FF_BLOCKDEF) && bottom < bottomBlock)
            bottomBlock = bottom;
	formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
        return;
    }
    /* prendre en compte buffer ligne pour synchroniser les block marks */
    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,NO_ERASE);

    /* GESTION BLOCKS */
    if (fflag & FF_BLOCKDEF) {
        if (current <= topBlock) {
            if ((len = linelen(wp)) >= topBlock - current) {
                 /* le block demarre sur la ligne courante */
                 if ((next = lnnext(wp,current)) > bottomBlock) {
                      /* debut et fin block sur la meme ligne */
                      bottomBlock = topBlock = NIL_LN;
                      fflag &= ~FF_BLOCKDEF;
                 }
                 else {
                      topBlock = current;
                      bottomBlock -= len+1;
                 }
            }
            else { /* le block est au dela de cette ligne */
                topBlock    -= len+1;
                bottomBlock -= len+1;
            }
        }
        else  {
            if (current < bottomBlock) {
                if ((len = linelen(wp)) >= bottomBlock - current) {
                     /* le block termine sur la ligne courante */
                     prev = lnprev(wp,current);
                     bottomBlock = prev + lnlen(wp,prev) + 1;
                }
                else
                     bottomBlock -= len+1;
            }
        }
    }
    deleteline(wp);    /* supprimer la ligne dans le big buffer */
    _window_up(  1,wp->ul_x + 1,
  	           wp->ul_y + 1 + wp->curY,
                   wp->ul_x + wp->ncol,
                   wp->ul_y + wp->nline,
                   wp->ink|wp->paper);

/*    if (fflag & FF_BLOCKDEF)
        write_page_att(wp);   /* reecrire attributs de page */

    /* se positionner en bas de page et ecrire la ligne de bas de page */
    i = wp->nline - wp->curY - 1;
    pt = current;
    ycur = wp->curY;
    while (i-- > 0)
           if ((pt = lnnext(wp,pt)) == NIL_LN) break;

    /* si on est pas en derniere ligne, ecrire la ligne suivante */
    if (pt != NIL_LN) {
	formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
        wgotoxy(wp,0,wp->nline - 1);
        wprintln(wp,pt);
/*        put_att_BP(wp,pt,wp->nline - 1);*/
        wgotoxy(wp,0,ycur);
/*        wcurrln(wp); /* VERIF A SAQUER */
/*        put_att_BP(wp,current,wp->curY);*/
    }
    write_status();
}


wbackspblk(wp)
struct Win *wp;
{
    unsigned absolu;

    /* GESTION BLOCK */
    if (fflag & FF_BLOCKDEF) {
        if (((absolu = current + wp->curX + wp->leftCh) > topBlock) &&
             (absolu <= bottomBlock))
              bottomBlock--;
        else {
             if (absolu <= topBlock) {
                 topBlock--;
                 bottomBlock--;
             }
        }
    }
}

/*------------------------------------------------------
 * wbacksp() - supprime le Ch precedent
 *------------------------------------------------------
 */
wbacksp(wp)
struct Win *wp;
{
    int i,j, plen;
    unsigned pt, absolu;

    if ((i = wp->curX + wp->leftCh) == 0) {  /* bord gauche */
         if (wp->curY == 0) {                /* en haut     */
             if (current == 0)           /* deb fichier */
                 return;
	     else { /* remonter d'une ligne (gestion du sommet de page) */
		 arrow_up(wp);
		 arrow_down(wp);
	     }
	 }
         pt = lnprev(wp,current);
         if ((i = linelen(wp) +
             (j = lnlen(wp,pt))) < NB_CAR_PER_LINE - 1) {

              /* concatainer la ligne courante a la precedente */
              wbackspblk(wp);
              concat(wp,pt);

              wgotoxy(wp,wp->curX,wp->curY - 1);
              if ((wp->curX = j) >= wp->ncol) {
                  wp->leftCh = wp->curX - (wp->ncol - 1);
                  wp->curX = wp->ncol - 1; /**/
              }
              set_win(wp);
         }
         else {
              concat(wp,pt);
              line_too_big(wp);
              set_win(wp);
         }
         i = wp->curX;
         wgotoxy(wp,i,wp->curY); /**/
         fflag |= FF_DIRTY; /**/
	 formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, NB_CAR_PER_LINE * 2);
         return;
    }
    else {      /* cas general */
         /* tester si buffer ligne deja charg‚ */
/*         if (!(fflag & FF_LINE_UPDATE)) {
             fill_line(linebuf,(NB_CAR_PER_LINE -  1) * 2,EOS);
             lncpy(wp);
             fflag |= FF_LINE_UPDATE;
         }*/
         fflag |= FF_LINE_UPDATE;
         /* tester si position courante apres fin de ligne */
         if ((plen = _lnlen(linebuf)) < i)
              /* combler par des blancs */
              add_blank(wp,plen,i);

         if (linebuf[i = wp->curX + wp->leftCh - 1] != EOS) {
             while (linebuf[i] != EOS) {
                    linebuf[i] = linebuf[i+1];
                    i++;
             }
             linebuf[i] = EOS;
         }
         wbackspblk(wp);
         arrow_left(wp);
         i = wp->curX;
    }
    wgotoxy(wp,i,wp->curY);
    fflag |= FF_DIRTY;
/*    if (fflag & FF_BLOCKDEF) {
         /* inserer d'abord avant de placer les attributs */
/*         if (fflag & FF_LINE_UPDATE)
             insert_lnbuf(wp,NO_ERASE);
    }*/
    wcurrln(wp);
/*    put_att_BP(wp,current,wp->curY);*/
}

