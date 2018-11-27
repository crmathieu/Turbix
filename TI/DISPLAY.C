/* DISPLAY.C  :   fichier de gestion de l'affichage */
#include "xed.h"
#include "ext_var.h"


/*----------------------------------------------------------------------
 * delete_to_end_of_line - supprimer Ch de la position courante a la fin
 *----------------------------------------------------------------------
 */
delete_to_end_of_line(wp)
struct OBJ *wp;
{
    int nb = wp->curX;

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
struct OBJ *wp;
{
    unsigned pos;
    int i, nb = to - (i = from);

    while (i < to)
           linebuf[i++] = BLANK;

    if (fflag & FF_BLOCKDEF) {
        if (((pos = current + from) >= topBlock) &&
             (pos < bottomBlock))
              bottomBlock += nb;
        else
/*             if (pos <= topBlock) {
                 topBlock += nb;
                 bottomBlock += nb;
               }*/
             if (pos < topBlock) {
                 topBlock += nb;
                 bottomBlock += nb;
              }
              else
                if (pos == topBlock)
                        bottomBlock += nb;

    }
}

/*--------------------------------------------------------------------------
 * fill_line - remplir tout ou partie du buffer ligne avec Ch
 *--------------------------------------------------------------------------
 */
fill_line(line,nb,Ch)
char *line;
int nb;
char Ch;
{
    memset(line, Ch, nb);
}


/*---------------------------------------------------------------------------
 * erase_trailing_blank - supprimer d'eventuels caracteres ' 'en fin de ligne
 *                        retourne la nouvelle taille
 *---------------------------------------------------------------------------
 */
erase_trailing_blank(wp)
struct OBJ *wp;
{
    int i;
    i = _lnlen(linebuf);
    while (linebuf[--i] == BLANK)
           linebuf[i] = EOS;
    return(i+1);
}


/*--------------------------------------------------------------------------
 * getnextline - retourne la position de la N ieme ligne apres offline
 *               ou bien la position de la derniere ligne
 *--------------------------------------------------------------------------
 */
unsigned getnextline(wp,offline,n)
struct OBJ *wp;
unsigned offline;
{
    unsigned X;
    while ((n-- > 0) && ((X = lnnext(wp,offline)) != NIL_LN))
            offline = X;
    return(offline);
}


/*--------------------------------------------------------------------------
 * getprevline - retourne la position de la N ieme ligne avant offline
 *               ou bien la position de la premiere ligne
 *--------------------------------------------------------------------------
 */
unsigned getprevline(wp,offline,n)
struct OBJ *wp;
unsigned offline;
int n;
{
    unsigned X,Y;
    X = Y = offline;

    while (n-- > 0)
           if ((X = lnprev(wp,X)) == NIL_LN) break;
           else Y = X;
    return(Y);
}

/*---------------------------------------------
 * set_win - rafraichir la fenetre entierement
 *---------------------------------------------
 */
FUNCTION set_win(wp,debug)
struct OBJ *wp;
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
         formatt_line(lineTab, bigbuf, pt, len, BLANK, nb_car_per_line * 2);
         _wstringDB( wp->ul_x + 1,
                     wp->ul_y + 1 + lig,
                     wp->ncol,
                     OLDATT,
                     att,
                     lineTab + wp->leftCh);  /* sauter les Ch cach-s */

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

/*---------------------------------------------------------------------
 * formatt_line - formatter une ligne avec les caracteres de Tabulation
 *---------------------------------------------------------------------
 */
formatt_line(dest, src, pt, len, fillCh, ninit)
char *dest, *src; /* "dest" doit etre lineTab */
unsigned pt;
char fillCh;      /* BLANK pour lineTab ou EOS pour linebuf */
int ninit;        /* # de caracteres a initialiser avant le formattage */
{
    int i, j, k;
    unsigned  n;
    struct OBJ errwin;

    fflag &= ~FF_TAB_ON_LINE; /* reinitialiser flag de tabulation */
    memset(dest, fillCh, ninit);
    if (len >= nb_car_per_line) {
        len = nb_car_per_line - 1;
        if (dest == linebuf) {
                verify_window(&errwin, lineTooLongStr,
                                      nullstr, NO_REP_REQ, ERRMSG);
                while ((getkey() & 0x00ff) != ESC)
                        beep(1000);
                winPopUp(messwin, W_CLOSE);

                n = current + len;
                if ((long)fsize + 1 >= BIGBUF_SIZE - 1) {
                        pushPop_MESS(outofmem, ERRMSG);
                        return;
                }
                memmove(bigbuf + n + 1,
                        bigbuf + n,
                        fsize  - n);
                bigbuf[n] = EOS;
                fsize++;
                if (bottom != current)
                        bottom++;
                else
                        bottom += (len + 1);
                bottom_line_no++;
                set_win(pedwin);
        }
    }

    for (i = j = 0; j <= len; j++, i++)
         if ((dest[i] = src[pt+j]) == TAB) {
              fflag |= FF_TAB_ON_LINE;  /* indique tabulation sur la ligne */
              strncpy(&dest[i], fillstr, tablength - (k = i % tablength));
              i += tablength - k - 1; /* -1 because l'incrementation de boucle */
         }
         else if (dest[i] == EOS)
                  dest[i] = BLANK;
    dest[i-1] = fillCh;
    return(i-1); /* retourner le nb de caracteres reels */
}

/*---------------------------------------------
 * compute_Tab - calcule le nb de caracteres
 *              en expans-s
 *---------------------------------------------
 */
compute_tab(wp, pt)
struct OBJ *wp;
unsigned pt;
{
    int i, j, k, limite;

    if ((limite = _lnlen(&bigbuf[pt])) == 0)
        return(0);

    for (i = j = 0; j < limite; j++, i++)
        if ((bigbuf[pt + j]) == TAB) {
                k = i % tablength;
                i += tablength - k - 1; /* -1 because l'incrementation de boucle */
        }
    return(i-1); /* retourner le nb de caracteres reels */

}



/*---------------------------------------------
 * linelen - calcul la taille d'une ligne
 *---------------------------------------------
 */
linelen(wp)
struct OBJ *wp;
{
/*   if (fflag & FF_LINE_UPDATE)      return(_lnlen(linebuf));
   else                             return(lnlen(wp,current));*/
   _lnlen(linebuf);
}

/*----------------------------------------------------
 * lnlen - calcul la taille d'une ligne du big buffer
 *----------------------------------------------------
 */
lnlen(wp,offline)
struct OBJ *wp;
unsigned offline;
{
   char c;
   unsigned i = offline;

   if (offline == bottom) {
       while (((c = bigbuf[i++]) != EOS) && (c != EOFM)) ;
       return(i - (offline + 1));
   }
   else
       return(_lnlen(&bigbuf[offline]));
}

/*------------------------------------------------
 * lnbuflen - calcul la taille de la ligne tampon
 *------------------------------------------------
 */
lnbuflen2(wp)
struct OBJ *wp;
{
   char c;
/*   int i = 0;
   while (linebuf[i++] != EOS) ;
   return(i-1);*/
   return(_lnlen(linebuf));
}

/*----------------------------------------------------------------------
 * lncpy - copier la ligne courante ds le tampon ligne (delimiteur = EOS = '\n')
 *----------------------------------------------------------------------
 */
lncpy(wp)
struct OBJ *wp;
{
   char c;
   int i;
   unsigned offline = current;

   i = 0;
   while ((c = bigbuf[offline++]) != EOS && c != EOFM)
           linebuf[i++] = c;
   linebuf[i] = EOS;
}

/*-----------------------------------------------f----
 * lnprev - retourne l'offset de la ligne precedente
 *---------------------------------------------------
 */
unsigned lnprev(wp,offline,debug)
struct OBJ *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X = offline;

   if (X == 0)
        return(NIL_LN);
   else
        X--;
   while (X >= 1) {
          if (bigbuf[X-1] == EOS) {
              return(X);
          }
          X--;
   }
   return(0); /* offset de debut fichier */
}

/*---------------------------------------------------
 * getlast_ln - retourne l'offset de la derniere ligne (A  TERMINER)
 *---------------------------------------------------
 */
unsigned getlast_ln(wp)
struct OBJ *wp;
{
   unsigned pos = fsize-1;  /* le dernier Ch est EOFM */

   while (pos > 0 && bigbuf[pos - 1] != '\n')
        pos--;
   return(pos);

}

/*----------------------------------------------------------------
 * get_line_no - retourne le nb de lignes correspondant a l'offset
 *----------------------------------------------------------------
 */
get_line_no(offset)
unsigned offset;
{
   int i, j;
   unsigned pos = offset; /*offset - 1;*/

   if (offset == 0)
        return(1);
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
/*----------------------------------------------------
 * getnln - retourne le nb de lignes entre deux lignes
 *----------------------------------------------------
 */
getnln(wp,first,last)
struct OBJ *wp;
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

/*---------------------------------------------------
 * lnnext - retourne l'offset de la ligne suivante
 *---------------------------------------------------
 */
unsigned lnnext(wp,offline)
struct OBJ *wp;
unsigned offline;   /* offset ligne de reference */
{
   unsigned X;
   if (((X = lnlen(wp,offline) + 1) + offline) >= fsize)
        return(NIL_LN);   /* on est sur la derniere ligne */
   return(offline + X);
}

/*------------------------------------
 * initWin : initialiser une fenetre
 *------------------------------------
 */
FUNCTION initWin(wp,m_key, title,ul_x,ul_y,ncols,nlines,btyp, ombrage,bar,arrow,ink,paper,border,batt)
unsigned m_key;
struct OBJ *wp;
char *title;
{

   wp->M_Gpr         = m_Gpr;
   wp->M_CGpr        = m_CGpr;
   wp->M_Gre         = m_Gre;
   wp->M_Dpr         = m_Dpr;
   wp->M_CDpr        = m_CDpr;
   wp->M_Dre         = m_Dre;
   wp->M_Mov         = m_Mov;

   wp->m_key         = m_key;
   wp->mouse_ev_flag = M_LEFT_PRESS|M_LEFT_RELEASE|M_RIGHT_PRESS|M_RIGHT_RELEASE|M_MOVMENT;
   wp->prev          = wp->next = (struct OBJ *)NULL;
   wp->objet         = WINDOW;
   wp->nline         = nlines;
   wp->leftCh        = 0;
   wp->curX          = wp->curY = 0;
   wp->border        = border;
   wp->ink           = ink;
   wp->paper         = paper;
   wp->ul_x          = ul_x;
   wp->ul_y          = ul_y;
   wp->ncol          = ncols;
   wp->pushed        = FALSE;
   wp->title         = title;
   wp->save          = NULL;
   wp->f_ombrage     = ombrage;
   wp->f_bar         = bar;
   wp->f_arrow       = arrow;
   wp->borderTyp     = btyp;
   wp->blockAtt      = batt;
   fillCh            = 0;

}

/*-----------------------------------------------------------
 * winPopUp - ouvrir / fermer une fenetre
 *-----------------------------------------------------------
 */
winPopUp(wp, mode, chainage, borderType)
struct OBJ *wp;
{
    extern int prolog_it_mouse();

    if (mode == W_OPEN) {
        /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
        if (wp->save == NULL) /* check presence tampon */
            if ((wp->save = malloc(2 * (wp->ncol + 4) * (wp->nline + 3))) == NULL) {
                 pushPop_MESS(Enomem, ERRMSG);
                 return(FUNC|F6);
            }

        wp->pushed = TRUE;

        /* chainer - l'objet precedent
         * et MAJ objet courant
         */
        if (chainage) {
                wp->prev      = tailObj;
                if (tailObj != (struct OBJ *)NULL)
                        tailObj->next = wp;
                tailObj = currObj = wp;
        }

        saveScreen(wp);
        write_wbox(wp, borderType);
        clrwin(wp, BLANK);
    }
    else {
        wp->pushed = FALSE;
        if (wp->save != NULL) {
            restoreScreen(wp);
            free(wp->save);

            /* MAJ objet courant et file des objets */
            if (tailObj == currObj)
                tailObj = currObj = wp->prev;
            else
                tailObj = wp->prev;
            if (tailObj != (struct OBJ *)NULL)
                tailObj->next = (struct OBJ *)NULL;
            wp->save = NULL;
        }
    }

    /* positionner le masque souris pour cet OBJ */
    m_callMaskAndAddress(currObj->mouse_ev_flag, prolog_it_mouse);

}


/*-----------------------------------------------------------
 * clrwin - clear window
 *-----------------------------------------------------------
 */
FUNCTION clrwin(wp, Ch)
struct OBJ *wp;
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

/*-----------------------------------------------------------
 * scrollwinUP - scrolling window UP
 *-----------------------------------------------------------
 */
FUNCTION scrollwinUP(wp)
struct OBJ *wp;
{
       _window_up(1,wp->ul_x+1,
                  wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper);
}

/*-----------------------------------------------------------
 * scrollwinDWN - scrolling window DOWN
 *-----------------------------------------------------------
 */
FUNCTION scrollwinDWN(wp)
struct OBJ *wp;
{
       _window_down(1,wp->ul_x+1,
                    wp->ul_y+1,
                    wp->ul_x + wp->ncol,
                    wp->ul_y + wp->nline,
                    wp->ink|wp->paper);
}


/*-----------------------------------------------
 * write_wbox - dessiner le contour de la fenetre
 *-----------------------------------------------
 */
FUNCTION write_wbox(wp, type)
struct OBJ *wp;
{
      write_box(wp->title, wp->ul_x, wp->ul_y, wp->ncol, wp->nline,
                wp->border, wp->f_bar, wp->f_arrow, type);
}


/*-----------------------------------------------
 * arrow_up - ED arrow UP
 *-----------------------------------------------
 */
FUNCTION arrow_up(wp)
struct OBJ *wp;
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
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
       wcurrln(wp);
   }
   else {
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY - 1);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
   }
   put_att_BP(wp,current,wp->curY,lnlen(wp,current));

   return(0);
}

/*-----------------------------------------------
 * arrow_down - ED arrow DWN
 *-----------------------------------------------
 */
FUNCTION arrow_down(wp)
struct OBJ *wp;
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
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
       wcurrln(wp);
   }
   else {
       /* milieu de page */
       wmgotoxy(wp,wp->curX,wp->curY + 1);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
   }
   put_att_BP(wp,current,wp->curY,lnlen(wp,current));
   return(0);
}

/*-----------------------------------------------
 * arrow_right - ED arrow RIGHT
 *-----------------------------------------------
 */
FUNCTION arrow_right(wp)
struct OBJ *wp;
{
   if ((wp->leftCh + wp->curX) >= nb_car_per_line - 1) {
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


/*-----------------------------------------------
 * arrow_left - ED arrow LEFT
 *-----------------------------------------------
 */
FUNCTION arrow_left(wp)
struct OBJ *wp;
{
   if ((wp->leftCh + wp->curX) == 0 )
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

/*-----------------------------------------------
 * page_up - ED Page UP
 *-----------------------------------------------
 */
FUNCTION page_up(wp)
struct OBJ *wp;
{
   int down,up;

   if (current == 0)
       return;

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

   if (topPage == 0) {
       /* debut fichier : se placer sur la 1ere ligne */
       wmgotoxy(wp,wp->curX,0);
       formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
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
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
   set_win(wp);
}

/*-----------------------------------------------
 * page_down - ED Page DWN
 *-----------------------------------------------
 */
FUNCTION page_down(wp)
struct OBJ *wp;
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
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
   set_win(wp);
}

/*-----------------------------------------------
 * line_end - ED line END
 *-----------------------------------------------
 */
FUNCTION line_end(wp)
struct OBJ *wp;
{
  int nb, len;
  /* si la ligne est en cours de modification
   * prendre la longueur du tampon
   */
/*  if (fflag & FF_LINE_UPDATE)  len = _lnlen(linebuf);
  else                         len = lnlen(wp,current);*/


   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

/*  insert_lnbuf(wp, ERASE);*/
  len = lnlen(wp, current);

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

/*-----------------------------------------------
 * line_home - ED line HOME
 *-----------------------------------------------
 */
FUNCTION line_home(wp)
struct OBJ *wp;
{
  int nb;


   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp,ERASE);

/*  insert_lnbuf(wp, ERASE);*/

  if (wp->leftCh > 0) {
      wp->leftCh = 0;
      set_win(wp);
  }
  wmgotoxy(wp,0,wp->curY);
/*  printf("cur = %d   bot = %d ",current_line_no,bottom_line_no);*/
}

/*--------------------------------------------------------------------------
 * wmgotoxy - se positionner dans la page avec mise a
 *            jour de la ligne courante en memoire
 *--------------------------------------------------------------------------
 */
FUNCTION wmgotoxy(wp,x,y)
struct OBJ *wp;
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

/*--------------------------------------------------------------------------
 * wgotoxy - se positionner dans la page sans mise a
 *           jour de la ligne courante en memoire
 *--------------------------------------------------------------------------
 */
FUNCTION wgotoxy(wp,x,y)
struct OBJ *wp;
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


/*---------------------------------------------------------------------
 * wcurrln - ecrire la ligne courante
 *---------------------------------------------------------------------
 */
FUNCTION wcurrln(wp)
struct OBJ *wp;
{
    int len, att;

    att = wp->ink|wp->paper;
    formatt_line(lineTab, linebuf, 0, len = _lnlen(linebuf), BLANK, nb_car_per_line * 2);
    _wstringDB( wp->ul_x + 1,
                wp->ul_y + 1 + wp->curY,
                wp->ncol,
                OLDATT,
                att,
                lineTab + wp->leftCh);  /* sauter les Ch cach-s */

    /* gestion marquage Block */
    put_att_BP(wp,current,wp->curY,len);
}

/*---------------------------------------------------------------------
 * wprintln - ecrire une ligne  (ou une partie) dans la fenetre
 *---------------------------------------------------------------------
 */
FUNCTION wprintln(wp, pt)
struct OBJ *wp;
unsigned pt;
{
    int len, att;

    att = wp->ink|wp->paper;
    formatt_line(lineTab, bigbuf, pt, len = lnlen(wp, pt), BLANK, nb_car_per_line * 2);
    _wstringDB( wp->ul_x + 1,
                wp->ul_y + 1 + wp->curY,
                wp->ncol,
                OLDATT,
                att,
                lineTab + wp->leftCh);  /* sauter les Ch cach-s */

    /* gestion marquage Block */
    put_att_BP(wp,pt,wp->curY,len);
}


/*------------------------------------------------------
 * winsch() - inserts a normal caracter in a line XXXX
 *------------------------------------------------------
 */
winsch(wp,Ch)
struct OBJ *wp;
int Ch;
{
     int i, p, line_pos, plen, k, inc, dec, endline;
     char c;

     if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
     }
     dec = 0;
     endline = FALSE;
     switch (Ch) {
     case  RETURN  :  winsln(wp,ON_MODE);return;
     case  BACKSP  :  wbacksp(wp);return;
     case  TAB     :
                      k = (wp->curX + wp->leftCh) % tablength;
                      inc = tablength - k;
                      if ((wp->curX + wp->leftCh + inc) >= nb_car_per_line - 1) {
                          wp->leftCh = nb_car_per_line - wp->ncol;
                          set_win(wp);
                          wgotoxy(wp, wp->ncol - 1, wp->curY);
                          beep(1000);  /* BELL */
                          return;
                      }

                      /* si mode insertion, inserer (tablength - k) BLANCS */

                      if (!(fflag & FF_INSERT)) {
                          if ((wp->curX += inc) < wp->ncol)
                               wgotoxy(wp, wp->curX, wp->curY);
                          else {
                               wp->leftCh += wp->curX - (wp->ncol - 1);
                               wgotoxy(wp, wp->ncol - 1, wp->curY);
                               set_win(wp);
                          }
                          return;
                      }
                      Ch = BLANK;
                      break; /*goto INS;*/
     default       :  inc = 1;
     }
INS:
                     if ((line_pos = wp->curX + wp->leftCh) >= nb_car_per_line - inc) {
                         beep(1000);  /* BELL */
                         return;
                     }

                     fflag |= FF_LINE_UPDATE;

                     /* tester si position courante apres fin de ligne */
                     if ((plen = _lnlen(linebuf)) <= line_pos) {
                          /* combler par des blancs */
                          add_blank(wp,plen,line_pos);
                          endline = TRUE;
                     }

                     if (fflag & FF_INSERT) {

                         /*-------------*
                          * MODE INSERT *
                          *-------------*/

                         if (plen >= nb_car_per_line - inc) {
                             beep(1000);  /* BELL */
                             return;
                         }

                         /* gestion decalage des carac a gauche */
                         k = inc;
                         while (k) {
                             i = line_pos;
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
                             k--;
                             line_pos++;
                         }

                     }
                     else {

                         /*----------------*
                          * MODE OVERWRITE *
                          *----------------*/

                        if (wp->leftCh + wp->curX >= nb_car_per_line - 1) {
                             winsln(wp,INSERT_MODE); /* COUILLE */
                             return;
                         }
                         linebuf[line_pos] = Ch;
                         dec = 1;

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
/*     }*/
        /* GESTION BLOCK */
        if (fflag & FF_BLOCKDEF) {
                if (current + wp->curX + wp->leftCh < topBlock) {
                        if (!(fflag & FF_INSERT) && (endline))
                              dec = 0;
                        topBlock += inc - dec;
                        bottomBlock += inc - dec;
                }
                else   /* sup debut block */
                        if (current + wp->curX + wp->leftCh < bottomBlock) {
                                if (!(fflag & FF_INSERT) && (endline))
                                        dec = 0;
                                bottomBlock += inc - dec;
                        }
        }


     fflag |= FF_DIRTY;
     if (wp->curX < wp->ncol - inc) {
         wmgotoxy(wp,wp->curX + inc,wp->curY);
         wcurrln(wp);
     }
     else {
       wp->leftCh += inc;
       set_win(wp);
     }
     return(0);
}


/*------------------------------------------------------
 * wdelch() - supprime le Ch courant
 *------------------------------------------------------
 */
wdelch(wp)
struct OBJ *wp;
{
    int i,j;

    /* tester si buffer ligne deja charg- */
/*    if (!(fflag & FF_LINE_UPDATE)) {
         fill_line(linebuf,(nb_car_per_line -  1) * 2,EOS);
         lncpy(wp);
         fflag |= FF_LINE_UPDATE;
     }*/
     if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
     }

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
struct OBJ *wp;
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
                     INF(lnlen(wp,current), (nb_car_per_line * 2) - id), EOS,
                     (nb_car_per_line * 2) - id);
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
                          INF(lnlen(wp,current), (nb_car_per_line * 2) - id), EOS,
                          (nb_car_per_line * 2) - id);
             wgotoxy(wp, 0, wp->curY);
        }
        else {
            wp->leftCh = 0;
            add_blank(wp, 0, id);
            formatt_line(linebuf+ id, bigbuf, current,
                         INF(lnlen(wp,current), (nb_car_per_line * 2) - id), EOS,
                         (nb_car_per_line * 2) - id);
            wgotoxy(wp, id, wp->curY);
        }
        if (id)
            fflag |= FF_LINE_UPDATE;
        set_win(wp);
    }
    fflag |= FF_DIRTY;
}

/*-------------------------------------------------------------
 * indent - calcule la position du 1er Char de la ligne donn-e
 *-------------------------------------------------------------
 */
indent(wp,offline)
struct OBJ *wp;
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
struct OBJ *wp;
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
        formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
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
    formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
    if (pt != NIL_LN) {
        /*formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);*/
        wgotoxy(wp,0,wp->nline - 1);
        wprintln(wp,pt);
        wgotoxy(wp,0,ycur);
    }
    write_status(wp);
}


wbackspblk(wp)
struct OBJ *wp;
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
struct OBJ *wp;
{
    int i,j, plen;
    unsigned pt, absolu;

     if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
     }

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
             (j = lnlen(wp,pt))) < nb_car_per_line - 1) {

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
         formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
         return;
    }
    else {      /* cas general */
         /* tester si buffer ligne deja charg- */
/*         if (!(fflag & FF_LINE_UPDATE)) {
             fill_line(linebuf,(nb_car_per_line -  1) * 2,EOS);
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
