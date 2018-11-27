#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>

/*------------------
 * init_editor
 *------------------
 */
init_editor()
{

   /* init variables editeur */
   if ((bigbuf = malloc(BIGBUF_SIZE)) == NULL) {
        pushPop_MESS(errmalloc, NMXMSG);
        exit(7);
   }
   if (conf.nb_car_per_line == 0)
        conf.nb_car_per_line = DEFAULT_NB_CAR_PER_LINE;

   nb_car_per_line = conf.nb_car_per_line;

   resize_line();
   init_edVar(0);
}
resize_line()
{
   if ((fillstr = malloc(nb_car_per_line + 2)) == NULL) {
        pushPop_MESS(errmalloc, NMXMSG);
        exit(7);
   }
   if ((clippbuf = malloc(nb_car_per_line * 2)) == NULL) {
        pushPop_MESS(errmalloc, NMXMSG);
        exit(7);
   }
   if ((lineTab = malloc(nb_car_per_line * 2)) == NULL) {
        pushPop_MESS(errmalloc, NMXMSG);
        exit(7);
   }
   if ((linebuf = malloc(nb_car_per_line * 2)) == NULL) {
        pushPop_MESS(errmalloc, NMXMSG);
        exit(7);
   }
   memset(fillstr,  0x20, nb_car_per_line);
   memset(clippbuf, EOS,  nb_car_per_line);
}

/*------------------
 * init_edVar
 *------------------
 */
init_edVar(status)
{
   fflag           = FF_INSERT;
   current         = topPage         = bottom         = 0;
   topBlock        = bottomBlock     = NIL_LN;
   current_line_no = topPage_line_no = 1;
   pedwin->curX    = pedwin->curY    = pedwin->leftCh = 0;
   if (status)
        write_status(pedwin);
}

/*----------------------------
 * ed - rentrer dans l'editeur
 *----------------------------
 */
ed(wp)
struct OBJ *wp;
{
    extern int prolog_it_mouse();
    int ret_value;

                /* chainer l'OBJ editeur comme
                 * objet courant
                 */

                currObj    = wp;
                wp->mouse_ev_flag |= M_MOVMENT;
                m_callMaskAndAddress(currObj->mouse_ev_flag, prolog_it_mouse);

                write_wbox(wp, TYP2);   /* activer fenetre editeur */
                formatt_line(linebuf, bigbuf, current, lnlen(wp, current), EOS, nb_car_per_line * 2);
                write_status(wp);
                ret_value = get_command(wp);
                hide_cursor(wp);
/*                write_wbox(wp, TYP1);   /* desactiver */

       return(ret_value);
}


/*------------------
 * get_command
 *------------------
 */
get_command(wp)
struct OBJ *wp;
{
      int i,go,ret;
      int ChL,ChX,Key;

      setcursor(wp);
      fflag |= FF_EDACTIF;
      while (fflag & FF_EDACTIF) {
             Key = getkey();
             ChL = (Key & 0x00ff);
             ChX = (Key & 0xff00);
             switch (ChX) {
             case NUMPAD: numpad(wp,ChL);break;
             case CTRL  : control(wp,ChL);break;
             case NORMAL: if (ChL == ESC) {
                            /*    if (fflag & FF_HELP_MOD) {/* 10/11/91 */
                                        /* STOPPER le mode help ! */

                                        /* Placer la fenetre d'edition
                                         * en objet courant
                                         */
                            /*            currObj = &edwin;*/

                                        /* declencher son ouverture */
                            /*            setMouseBuf(edwin.m_key);*/

                                        /* Supprimer l'AIDE */
                            /*            edHelp(HCLOSE);*/
                            /*    }*/
                                if (fflag & FF_LINE_UPDATE)
                                        insert_lnbuf(wp, ERASE);
                                return(FUNC|F6);
                          }
                          normalCh(wp,ChL);break;
             default    : /*switch(ChX) {
                          case CTRLNUMPAD : ctrlnumpad(wp,ChL);ret = 0;break;
                          case ALTFUNC    : ret = altfc(wp,ChL);break;
                          case SHIFTFUNC  : shiftfc(wp,ChL);ret = 0;break;
                          case FUNC       : ret = func(wp,ChL);break;
                          case CTRLFUNC   : ctrlfc(wp,ChL);ret = 0;break;
                          case ALT        : ret = altch(wp,ChL,ChL-16);break;
                          } */
                          ret = special_key(wp, ChX, ChL);
                          if (ret == 0)  break;
                          else  {
                                if (fflag & FF_LINE_UPDATE)
                                        insert_lnbuf(wp, ERASE);
                                return(ret);
                          }
             }
             write_status(wp);
      }
}

/*------------------
 * special_key
 *------------------
 */
unsigned special_key(wp, ChX, ChL)
struct OBJ *wp;
{
    unsigned ret = 0;

    switch(ChX) {
    case CTRLNUMPAD : ctrlnumpad(wp,ChL);break;
    case ALTFUNC    : ret = altfc(wp,ChL);break;
    case SHIFTFUNC  : shiftfc(wp,ChL);break;
    case FUNC       : ret = func(wp,ChL);break;
    case CTRLFUNC   : ret = ctrlfc(wp,ChL);break;
    case ALT        : ret = altch(wp,ChL,ChL-16);break;
    }
    return(ret);
}

/*----------------------------------------------------------
 * 2- numpad
 *----------------------------------------------------------
 */
int numpad(wp,Ch)
struct OBJ *wp;
int Ch;
{   int val, lastCh, firstCh, buttonStatus, contour;
    static int BlocEnCours = 0;

    switch(Ch) {
    case ARROWU : arrow_up(wp);break;
    case ARROWD : arrow_down(wp);break;
    case ARROWR : arrow_right(wp);break;
    case ARROWL : arrow_left(wp);break;
    case PGUP   : page_up(wp);break;
    case PGDN   : page_down(wp);break;
    case END    : line_end(wp);break;
    case HOME   : line_home(wp);break;
    case INSERT : switch_insert();break;
    case DELETE : ctrlg(wp);break;
    case MOUSE_RP:
    case MOUSE_RR: break;
    case MOUSE_R: /* tester presence sur le contour */
                  if (posX == wp->ul_x) {
                        gotoMouse(wp);
                        line_home(wp);
                  }
                  else
                        if (posX == wp->ul_x + wp->ncol + 1) {
                                gotoMouse(wp);
                                line_end(wp);
                        }
                        else
                                if (posY == wp->ul_y)
                                        page_up(wp);
                                else
                                        if (posY == wp->ul_y + wp->nline + 1)
                                                page_down(wp);
                                        else
                                                break;

                  ticks = 0;
                  while (ticks < 3) ;
                  mouseEvent++;  /* boucler sur la pression souris */
                  break;
    case MOUSE_L:
    case MOUSE_LP: /* tester presence sur le contour */
                  contour = TRUE;
                  if (posX == wp->ul_x)
                        arrow_left(wp);
                  else
                        if (posX == wp->ul_x + wp->ncol + 1)
                                arrow_right(wp);
                        else
                                if (posY == wp->ul_y)
                                        arrow_up(wp);
                                else
                                        if (posY == wp->ul_y + wp->nline + 1)
                                                arrow_down(wp);
                                        else {
                                                 contour = FALSE;
                                                 gotoMouse(wp);
                                                 if ((posX == XMOUSE) && (posY == YMOUSE))
                                                        /* la position de la souris est
                                                         * inchangee depuis la pression
                                                         * NE RIEN FAIRE
                                                         */
                                                         break;

                                                 else {
                                                         if (!BlocEnCours) {
                                                                m_setMinMaxVerCursorPos(wp->ul_y * 8, (wp->ul_y + wp->nline + 1) * 8);
                                                                m_setMinMaxHorCursorPos(wp->ul_x * 8, (wp->ul_x + wp->ncol  + 1) * 8);
                                                         }
                                                         blocMouse(wp);
                                                         BlocEnCours = TRUE;
                                                         /*ticks = 0;
                                                         while (ticks < 1) ;*/
                                                         /* mouseEvent++;  /* boucler sur la pression souris */
                                                         break;
                                                 }
                                        }
                  if (BlocEnCours)
                        BlocMouse(wp);
                  if (contour) {
                        ticks = 0;
                        while (ticks < 1) ;
                                mouseEvent++;  /* boucler sur la pression souris */
                  }
                  break;

    case MOUSE_LR : /* relache bouton gauche */
                    if (posX == XMOUSE && posY == YMOUSE) {
                        /* la position de la souris est
                         * inchangee depuis la pression :
                         * positionner le curseur - cet emplacement
                         * si on est pas sur le cadre
                         */
                        if ((posX == wp->ul_x) || (posX == wp->ul_x + wp->ncol + 1) ||
                            (posY == wp->ul_y) || (posY == wp->ul_y + wp->nline + 1))
                                ; /* ne rien faire */
                        else
                                gotoMouse(wp);
                    }
                    BlocEnCours = FALSE;
                    m_setMinMaxVerCursorPos(0, 192);
                    m_setMinMaxHorCursorPos(0, 639);
                    break;

    default     : fflag &= ~FF_EDACTIF;
    }
    write_status(wp);
}

blocMouse(wp)
struct OBJ *wp;
{
    int lastCh, firstCh, buttonStatus, i, j;
    unsigned aux;

    /* mouvement de la souris */
    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,ERASE);

    /* pression gauche en cours :
     * Marquer "debut de bloc" avec les
     * valeurs enregistrees
     */
     if ((firstCh = INF((i = LMOUSE + XMOUSE - (wp->ul_x + 1)),j = lnlen(wp,CMOUSE))) < 0)
                firstCh = 0;
     topBlock   = CMOUSE + firstCh;
     if ((firstCh <= i) && (i >= j) && (topBlock != CMOUSE))
        topBlock++;

     if ((lastCh = INF(wp->leftCh + posX - (wp->ul_x + 1),lnlen(wp,current))) < 0)
                lastCh = 0;
     bottomBlock = current + lastCh;


     if (fflag & FF_BLOCKDEF) {  /* un bloc est deja defini */
        if (bottomBlock < topBlock) {
                aux         = topBlock;
                topBlock    = bottomBlock;
                bottomBlock = aux;
        }
     }



     fflag        &= ~FF_BLOCKDEF;
     if (topBlock  != NIL_LN ) /* on a deja marqu- un debut bloc */
        if (topBlock <= bottomBlock)
                fflag |= FF_BLOCKDEF;

     write_page_att(wp);
         return(0);
}

gotoMouse(wp)
struct OBJ *wp;
{
   int val, X, Y;

   if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(wp,ERASE);
   if ((Y = (posY) - (wp->ul_y + 1)) < 0)
        Y = 0;
   if ((X = (posX) - (wp->ul_x + 1)) < 0)
        X = 0;

   /* verifier qu'on est toujours dans le fichier */
   if (bottom_line_no >= (current_line_no + Y - wp->curY))
        current_line_no += (Y - wp->curY);
   else
        return;
   wmgotoxy(wp, X, Y);
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
}

/*----------------------------------------------------------
 * 2- normalCh
 *----------------------------------------------------------
 */
int normalCh(wp,Ch)
struct OBJ *wp;
int Ch;
{
   switch(Ch) {
   case ESC    : break;
   default     : winsch(wp,Ch);break;
   }
}


/*----------------------------------------------------------
 * 2- ctrlnumpad
 *----------------------------------------------------------
 */
int ctrlnumpad(wp,cl)
struct OBJ *wp;
int cl;
{
   switch(cl) {
   case CTRL_ARRL : ctrla(wp);  break;  /* previous word  */
   case CTRL_ARRR : ctrlf(wp);  break;  /* next word      */
   case CTRL_PGUP : ctrlqr(wp); break;  /* top of file    */
   case CTRL_PGDN : ctrlqc(wp); break;  /* bottom of file */
   case CTRL_HOME : ctrlqe(wp); break;  /* top of page    */
   case CTRL_END  : ctrlqx(wp); break;  /* bottom of page */
   default        :             break;
   }
   write_status(wp);
}


/*----------------------------------------------------------
 * 2- control : switches for the execution of CTRL COMMANDS
 *----------------------------------------------------------
 */
int control(wp,cl)
struct OBJ *wp;
int cl;
{
   int key;
   switch(cl) {
      case 'A' : case 'a' : ctrla(wp);          break; /* word Backward */
      case 'D' : case 'd' : arrow_right(wp);    break; /* same as right arrow */
      case 'E' : case 'e' : arrow_up(wp);       break; /* same as up arrow */
      case 'F' : case 'f' : ctrlf(wp);          break; /* word Foreward */
      case 'G' : case 'g' : ctrlg(wp);          break; /* erase current Ch */
      case 'H' : case 'h' : wbacksp(wp);        break; /* erase previous Ch */
      case 'J' : case 'j' : ctrlj(wp);          break; /* look right side of file */
      case 'K' : case 'k' : ctrlk(wp);          break; /* exit block or paste managment */
      case 'L' : case 'l' : ctrll(wp);          break; /* repeat last find */
      case 'N' : case 'n' : ctrln(wp,ON_MODE);  break; /* same as RETURN without moving the cursor */
      case 'Q' : case 'q' : ctrlq(wp);          break; /* CTRL Q commands */
      case 'S' : case 's' : arrow_left(wp);     break; /* same as left  arrow */
      case 'T' : case 't' : ctrlt(wp);          break; /* delete for current pos to end of word */
      case 'V' : case 'v' : switch_insert();    break; /* switch INS/OVERW mode */
      case 'X' : case 'x' : arrow_down(wp);     break; /* same as down arrow */
      case 'Y' : case 'y' : ctrly(wp);          break; /* delete current line */
      case RETURN         : print_cp0412(TRUE); break; /* Author */
      default  :                                break;
   }
}

/*------------------------------------------
 * 3-  ctrlg : ^G  delete current caractere
 *------------------------------------------
 */
ctrlg(wp)
struct OBJ *wp;
{
      if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
      }

      wdelch(wp);
      fflag |= FF_DIRTY;
}

/*------------------------------------------
 * 3-  ctrly : ^Y  delete current line
 *------------------------------------------
 */
ctrly(wp)
struct OBJ *wp;
{
      if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
      }

      wdeleteln(wp);
      fflag |= FF_DIRTY;
}

/*------------------------------------------------
 * 4- ctrlj : ^J  display the right side of page
 *------------------------------------------------
 */
ctrlj(wp)
struct OBJ *wp;
{
      wp->leftCh = nb_car_per_line - wp->ncol;
      set_win(wp);
}


/*------------------------------------------------*/
/* 6- quit : CTRL K D AND F1  END THE EDITOR      */
/*------------------------------------------------*/
ReturnToMainMenu()
{/*
    continu_command = continu = FALSE ;*/
}

/*-----------------------------------------------
 * 7- ctrln : add line without moving the cursor
 *-----------------------------------------------
 */
ctrln(wp,insert)
struct OBJ *wp;
{
      int savX,savLeft;

      if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
      }

      savX = wp->curX;
      savLeft = wp->leftCh;
      winsln(wp,insert);
      arrow_up(wp);
      wp->leftCh = savLeft;
      wmgotoxy(wp,savX,wp->curY);
      fflag |= FF_DIRTY;
}

/*------------------------------
 * 8- ctrlq : enter ^Q commands
 *------------------------------
 */
int ctrlq(wp)
struct OBJ *wp;
{
   int Key,ChX,ChL;

      Key  = getkey();
      ChL = (Key & 0x00ff);
      ChX = (Key & 0xff00);
      switch (ChX) {
      case CTRL :
         switch (ChL) {
         case 'A' : case 'a': ctrlqa(wp);       break; /* find / replace */
         case 'C' : case 'c': ctrlqc(wp);       break; /* se placer en bottom */
         case 'E' : case 'e': ctrlqe(wp);       break; /* se placer en debut */
         case 'F' : case 'f': ctrlqf(wp, FIND); break; /* CTRL Q F  */
         case 'G' : case 'g': ctrlqg(wp, 15);   break; /* goto line */
         case 'X' : case 'x': ctrlqx(wp);       break; /* aller en bas de page */
         case 'Y' : case 'y': ctrlqy(wp);       break; /* erase de la cur pos jusqu'en fin de ligne */
         case 'L' : case 'l': ctrlql(wp);       break; /* change line length */
         default : break;
         } break ;
/*   case ALT : if ( cl == F1 ){
                help(10,FALSE);ctrlq();break;}*/
   default : break;
   }
}

/*----------------------------------------------
 * 10 - ctrlqy() : ^Q^Y  supprimer fin de ligne
 *----------------------------------------------
 */
ctrlqy(wp)
struct OBJ *wp;
{
         int plen, val;
         unsigned debln, absolu;


         int i = wp->curX + wp->leftCh;

        if (fflag & FF_INHIBE_MOD) {
                beep(1000);
                return;
        }

         /* indiquer modification sur la ligne courante */
         fflag |= FF_LINE_UPDATE;

         /* tester si position courante apres fin de ligne */
         if ((plen = _lnlen(linebuf)) < i)
              return;
         else {
              /* recuperer le debut de la ligne */
              debln = getdebln(wp,topBlock);

              delete_to_end_of_line(wp);
              val = plen - i;
              if (fflag & FF_BLOCKDEF) {
                if (((absolu = current + wp->curX + wp->leftCh) >= topBlock) &&
                        (absolu < bottomBlock))
                        bottomBlock -= val;
                else {
                        if (absolu < topBlock) {
                                topBlock -= val;
                                bottomBlock -= val;
                        }
                }
              }
              memset(&linebuf[i], EOS, ((nb_car_per_line - 1) * 2) - i);
              insert_lnbuf(wp, ERASE);

              /* Si le debut de bloc est le carac EOS: AVANCER */
              if (debln != getdebln(wp,topBlock))
                        topBlock++;

              put_att_BP(wp,current,wp->curY,lnlen(wp,current));
         }
}

/*-------------------------------------------------------------------
 * 10 - ctrlqr() : ^Q^R / ^PGUP  goto top of file & write it
 *-------------------------------------------------------------------
 */
ctrlqr(wp)
struct OBJ *wp;
{
      if (current_line_no == 1) return;
      if (fflag & FF_LINE_UPDATE)
            insert_lnbuf(wp, ERASE);
      wgotoxy(wp,wp->curX,0);
      current = topPage = 0;
      current_line_no = topPage_line_no = 1;
      formatt_line(linebuf, bigbuf, 0, lnlen(wp, 0), EOS, nb_car_per_line * 2);
      set_win(wp);
}

/*------------------------------
 * 8- ctrlk : enter ^K commands
 *------------------------------
 */
int ctrlk(wp)
struct OBJ *wp;
{
    int ChL,ChX,Key;

    Key = getkey();
    ChL = (Key & 0x00ff);
    ChX = (Key & 0xff00);
    switch (ChX) {
    case CTRL  :
    case NORMAL:
         switch(ChL) {
         case 'k' : case 'K' : ctrlkk(wp);break; /* mark block end */
         case 'b' : case 'B' : ctrlkb(wp);break; /* mark block begin */
         case 'h' : case 'H' : ctrlkh(wp);break; /* hide block */
         case 'v' : case 'V' : ctrlkv(wp);break; /* move block */
         case 'y' : case 'Y' : ctrlky(wp);break; /* delete block */
         case 'c' : case 'C' : ctrlkc(wp);break; /* copy block */
         case 'r' : case 'R' : ctrlkr(wp);break; /* read block from paste file */
         case 'w' : case 'W' : ctrlkw(wp);break; /* write block to paste file */
         default  : break;
         }
         break;
    default : break;
    }
}

/*----------------------------------------------------------------------
 * 11- ctrlqc(): ^Q^C / ^PGDN goto bottom of file & write it
 *----------------------------------------------------------------------
 */
ctrlqc(wp)
struct OBJ *wp;
{
     int i;
     if ((bottom_line_no < wp->nline)
          || (topPage_line_no + wp->nline > bottom_line_no)) {
          /* si taille fichier < taille d'une page ou si on est dans la
           * derniere page : faire ctrl END
           */
          ctrlqx(wp);
          return;
     }

     /* cas general */
     if (fflag & FF_LINE_UPDATE)
         insert_lnbuf(wp, ERASE);

     topPage = bottom;
     for (i = 0 ; i < wp->nline - 1 ; i++ )
          topPage = lnprev(wp,topPage);

     topPage_line_no = bottom_line_no - (wp->nline - 1);
     current = bottom ;
     current_line_no = bottom_line_no;
     wgotoxy(wp,wp->curX, i);
     formatt_line(linebuf, bigbuf, current, lnlen(wp, current), EOS, nb_car_per_line * 2);
     set_win(wp);
}

/*---------------------------------
 * 12- ctrlqe : ^Q^E  top of page
 *---------------------------------
 */
ctrlqe(wp)
struct OBJ *wp;
{
      if (wp->curY == 0)   return;
      if (fflag & FF_LINE_UPDATE)
          insert_lnbuf(wp, ERASE);

      current = topPage;
      current_line_no = topPage_line_no;
      wgotoxy(wp,wp->curX,0);
      formatt_line(linebuf, bigbuf, current, lnlen(wp, current), EOS, nb_car_per_line * 2);
}

/*---------------------------------
 * 14- ctrlqx : ^Q^X  end of page
 *---------------------------------
 */
ctrlqx(wp)
struct OBJ *wp;
{
   int i, noline;

      if (wp->curY == wp->nline - 1) return;

      if (fflag & FF_LINE_UPDATE)
          insert_lnbuf(wp, ERASE);

      noline = current_line_no;
      for ( i = 1 ; i < wp->nline - (noline - topPage_line_no); i++)
          if (lnnext(wp,current) == NIL_LN)  break;
          else current = lnnext(wp,current);

      current_line_no += (i - 1);
      wgotoxy(wp,wp->curX,wp->curY + (i - 1));
      formatt_line(linebuf, bigbuf, current, lnlen(wp, current), EOS, nb_car_per_line * 2);
}


/*---------------------
 *  switch_insert
 *---------------------
 */
switch_insert()
{
      if (fflag & FF_INSERT)
          fflag &= ~FF_INSERT;
      else
          fflag |=  FF_INSERT;
}

/*---------------------------------------------------------
 * 17- function_key : SWITCH FO FUNCTION KEYS
       if cl+100 : Block_Paste Mode
 *--------------------------------------------------------*/
function_key(cl)
int cl;
{
}

altch(wp,c,cbis)
struct OBJ *wp;
{
  int extra, type;
  if ((extra = getItem(c,0, &type)) != NP)
      /* un extra caractere a ete frapp- */
      return(ALT|c);
  return(0);
}

/*--------------------------------------------------
 * 20-  altfc
 *-------------------------------------------------
 */
altfc(wp,ch)
struct OBJ *wp;
int ch;
{
    switch (ch){
    case ALT_F1  : return(ALTFUNC|ALT_F1);
    case ALT_F2  : break;
    case ALT_F3  : fflag &= ~FF_EDACTIF;
                   return(ALTFUNC|ALT_F3);
    case ALT_F4  :
    case ALT_F5  :
    case ALT_F6  :
    case ALT_F7  :
    case ALT_F8  :
    case ALT_F9  : break;
    case ALT_F10 : return(ALT|ALT_F10);
    }
    return(0);
}

/*--------------------------------------------------
 * 20-  shiftfc
 *-------------------------------------------------
 */
shiftfc(wp,ch)
struct OBJ *wp;
int ch;
{
    switch (ch){
    case SHIFT_F1  :
    case SHIFT_F2  :
    case SHIFT_F3  :
    case SHIFT_F4  :
    case SHIFT_F5  :
    case SHIFT_F6  :
    case SHIFT_F7  :
    case SHIFT_F8  :
    case SHIFT_F9  :
    case SHIFT_F10 :
    default        : break;
    }
}


/*--------------------------------------------------
 * 20-  ctrlfc
 *-------------------------------------------------
 */
ctrlfc(wp,ch)
struct OBJ *wp;
char ch;
{
    char work[80], c;
    int i, j;

    i = 0;
    j = wp->curX + wp->leftCh;

    /* recuperer le mot courant */
    while (isalnum(c = linebuf[j]) ||  c == '_') j--;
    j++;
    while (isalnum(c = linebuf[j + i])|| c == '_') {
           work[i] = c;
           if (++i >= 80) {
                 i--;
                 break;
           }
    }
    work[i] = 0;

    switch (ch){
    case CTRL_F1  : strcpy(defhlp, work);

                    /* indiquer help en ligne */
                    helpInLine = TRUE;
                    return(FUNC|ALT_F1);

    case CTRL_F2  : strupr(work);
                    return(_doGotoDef(wp, 0, work));

    case CTRL_F3  : strupr(work);
                    return(_doDispRef(wp, 0, work));

    case CTRL_F4  :
    case CTRL_F5  :
    case CTRL_F6  :
    case CTRL_F7  :
    case CTRL_F8  :
    case CTRL_F9  :
    case CTRL_F10 :
    default        : break;
    }
    return(0);
}

/*--------------------------------------------------
 * 20-  func
 *-------------------------------------------------
 */
func(wp,ch)
struct OBJ *wp;
char ch;
{
    switch (ch){
    case F1  : return(FUNC|F1);
    case F2  : /* positionner retour sur l'editeur */
               ReturnFromSaveFile = (FUNC|F6);
               return(FUNC|F2);       /* SAVE */
    case F3  : fflag &= ~FF_EDACTIF;
               return(FUNC|F3);       /* LOAD */
    case F4  : return(FUNC|F4);
    case F5  : return(FUNC|F5);       /* LOAD LAST FILE */
    case F6  : return(FUNC|F6);       /* EDITOR */
    case F7  : return(FUNC|F7);
    case F8  : return(FUNC|F8);
    case F9  : return(FUNC|F9);       /* MAKE */
    case F10 : return(FUNC|F10);      /* return to MAIN MENU */
    default        : break;
    }
     return(0);
}


/*--------------------------------------------------
 * write_status - ecrire la status line editor
 *-------------------------------------------------
 */
write_status(wp)
struct OBJ *wp;
{
  unsigned long vPercent, hPercent;
  unsigned char barre, anneau;
  int h,l;
  static int barPosY = 0;
  static int barPosX = 0;


  /* ecriture barres */
  if (wp->f_bar) {
        anneau = '-';
        barre =  '-';

        /* calcul position dans le fichier */
        if (fsize) {
                if ((vPercent = (unsigned long)(((unsigned long)(current /*+ wp->curX + wp->leftCh*/)) * 100) / fsize) > 100)
                        vPercent = 100L;
                }
                else
                        vPercent = 0L;

        if ((hPercent = (unsigned long)(((unsigned long)(wp->curX + wp->leftCh)) * 100) / nb_car_per_line) > 100)
                hPercent = 100L;

        h = wp->nline;
        l = wp->ncol;

        /* barre verticale */
        _wstringDB(wp->ul_x + wp->ncol + 1, wp->ul_y + (h-BARRE_SIZE)/2 + barPosY, 1, NEWATT, WINborder, &barre);
        if ((barPosY = (((BARRE_SIZE+1) * vPercent) / 100) ) >= (BARRE_SIZE+1))
                barPosY = BARRE_SIZE+1;
        _wstringDB(wp->ul_x + wp->ncol + 1, wp->ul_y + (h-BARRE_SIZE)/2 + barPosY, 1, NEWATT, WINborder, &anneau);

        /* barre horizontale */
        _wstringDB(wp->ul_x + (l-BARRE_SIZE)/2 + barPosX, wp->ul_y + wp->nline + 1, 1, NEWATT, WINborder, &barre);
        if ((barPosX = (((BARRE_SIZE+1) * hPercent) / 100)) >= (BARRE_SIZE+1))
                barPosX = BARRE_SIZE+1;
        _wstringDB(wp->ul_x + (l-BARRE_SIZE)/2 + barPosX, wp->ul_y + wp->nline + 1, 1, NEWATT, WINborder, &anneau);
  }

  /* ecriture ligne colonne */
  _wstringDB(0, 24, 80, NEWATT, MENUink|MENUpaper, status);
  itoa(current_line_no, lines, 10);
  itoa(wp->curX + wp->leftCh + 1, cols, 10);
/*  ltoa(vPercent, perc, 10);*/

  _wstringDB(XLINE, 24, strlen(lines), OLDATT, 0, lines);
  _wstringDB(XCOL,  24, strlen(cols),  OLDATT, 0, cols);
/*  _wstringDB(XPER,  24, strlen(perc),  OLDATT, 0, perc);*/


  /* ecriture insert */
  if (fflag & FF_INSERT)
      _wstringDB(XINS, 24, 7, OLDATT, 0, insStr);
  else
      _wstringDB(XINS, 24, 7, OLDATT, 0, fillstr);
}

/*--------------------------------------------------
 * clear_status_line  -
 *-------------------------------------------------
 */
clear_status_line()
{
  _wstringDB(0, 24, 80, NEWATT, MENUink|MENUpaper, fillstr);
}

/*--------------------------------------------------
 * print_YesNo  -
 *-------------------------------------------------
 */
print_yesno()
{
   int c, heigh, cl, ch, i, j, Y, doIt, size, yn0(), yn1(), ret;
   struct OBJ errwin, *pob;
   struct  MOUSE_FIELD mfYN[5];

   heigh = 5;
   i = pedwin->curY;
   j = (22 - heigh)/2;
   if (i >= j) {
        if (i < (j + heigh))
                Y = pedwin->ul_y+1;
        else    Y = j;
   }
   else
        Y = j;

   size = strlen(AskYN)+2;
   initWin(&errwin, 0, nullstr, (80 - size)/2, Y, size, heigh, TYP1,
                TRUE, FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder,
                WINSYSblock, W_WIN);

   pob = &errwin;

   /* initialiser la zone Champ */
   pob->mField = mfYN;

   /*             f#  x   y  sz  (* )() */
   initField(pob,  0, 6,  3, 4,  yn0);
   initField(pob,  1, 19, 3, 4,  yn1);
   FIELD(pob, 2)->size      = 0;    /* marque la fin des champs */

   /* initialiser les actions souris */
   pob->M_Dpr  = m_editDpr;
   pob->M_Gpr  = m_editGpr;
   pob->M_Gre  = m_editGre;
   pob->M_CGpr = m_editCGpr;
   pob->M_CDpr = m_editCDpr;

   Yes_answ = TRUE;
   No_answ  = FALSE;

   winPopUp(pob, W_OPEN, OBJ_CHAINAGE, TYP1);
   print_at(pob, (pob->ncol - strlen(YesNo))/2, 1, YesNo, OLDATT);
   print_at(pob, 1, 3, AskYN, OLDATT);

   doIt = TRUE;
   while (doIt) {
        razMouseBuf();
        c = getkey();
        cl = c & 0x00ff;
        ch = c & 0xff00;
        switch(ch) {
        case NUMPAD: switch(cl) {
                          case MOUSE_L :
                          case MOUSE_R :
                          case MOUSE_RP:
                          case MOUSE_LP: continue;

                          case MOUSE_RR :
                          case MOUSE_LR : /* relache bouton gauche */
                                        if ((i = isOnField(pob)) >= 0) {
                                            (* FIELD(pob, i)->MouAction)(pob, i);
                                            if (Yes_answ)
                                                 c = 'y';
                                            else c = 'n';
                                            doIt = FALSE;
                                        }
                          default:
                                        break;
                          }
                          break;
        default:
                if (cl == ESC) {
                        c = 0;
                        break;
                }
                for (i=0; i<4; i++)
                        if (c == y_o[country][i]) {
                                /* retourner le type U.S. */
                                c = y_o[0][i];
                                doIt = FALSE;
                                break;
                        }
                break;
        }
        if (doIt)
                beep(1000);
   }
   winPopUp(pob, W_CLOSE);
   return(c);
}

yn0(p, no)
struct OBJ *p;
{
   return(1);
}
yn1(p, no)
struct OBJ *p;
{
   Yes_answ = FALSE;
   return(1);
}

print_yesno2()
{
   int c, i, Y, doIt, size;
   struct OBJ errwin, *pob;


   Y = 11;
   if ((pedwin->curY + pedwin->ul_y) >= 10)
        Y = Y - 5;

   size = strlen(YesNo);
   initWin(&errwin, 0, nullstr, (80 - size)/2, Y, size, 3, TYP1,
                TRUE, FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder,
                WINSYSblock, W_WIN);

   pob = &errwin;

   winPopUp(pob, W_OPEN, OBJ_CHAINAGE, TYP1);
   print_at(pob, 0, 1, YesNo, OLDATT);


   doIt = TRUE;
   while (doIt) {
        razMouseBuf();
        c = (getkey() & 0x00ff);
        if (c == ESC) {
                c = 0;
                break;
        }
        for (i=0; i<4; i++)
                if (c == y_o[country][i]) {
                        /* retourner le type U.S. */
                        c = y_o[0][i];
                        doIt = FALSE;
                        break;
                }
        if (doIt)
                beep(1000);
   }
   winPopUp(pob, W_CLOSE);
   return(c);
}


/*-----------------------
 * ctrlf - word foreword
 *-----------------------
 */
ctrlf(wp)
struct OBJ *wp;
{
     int i, firstTime;
     unsigned pt;
     char *line;

      firstTime = TRUE;
      i = INF(wp->curX + wp->leftCh,linelen(wp));

Continue :
      pt = current;
      line = linebuf;

/*      i = INF(wp->curX + wp->leftCh,linelen(wp));*/

      /* read the char until a white space */
      if (firstTime) {
          firstTime = FALSE;
          while (isalnum(line[i])) i++;
      }
      if ((line[i] == EOS || line[i] == EOFM) && lnnext(wp,pt) == NIL_LN)
           return ;         /* no more words in the file */

      if (line[i] == EOS ) {  /* la fin de ligne est un separateur */
          arrow_down(wp) ;
          line_home(wp);
          i = 0;
          goto Continue;
      }
      while (!isalnum (line[i])) {    /* read until next word */
             if (line[i] == EOS) {
                 if (arrow_down(wp) < 0)
                     return;              /* no more words in the file */
                 line_home(wp);
                 i = 0;
                 goto Continue;
             }
             else  i++ ;
      }
      if (i - (wp->ncol - 1) > wp->leftCh) {
          wp->leftCh = i - (wp->ncol - 1);
          set_win(wp);
      }
      wgotoxy(wp,i-wp->leftCh,wp->curY);
}



/*-----------------------
 * ctrla - word backward
 *-----------------------
 */
ctrla(wp)
struct OBJ *wp;
{
    int i, noline;
    unsigned pt;
    char *line;

      i = INF(wp->leftCh + wp->curX,linelen(wp));
Continue :
      pt = current;
      noline = current_line_no;
      line = linebuf;

      /* si on est en debut de mot, reculer d'un caractere */
      if (i && !isalnum(line[i-1]))
          i--;

      /* si on est en debut de ligne, faire CTRLA sur la ligne precedente */
      if (i == 0) {
          if (lnprev(wp,pt) != NIL_LN) {
              arrow_up(wp);
              line_end(wp);
              i = lnlen(wp,current);
              goto Continue;
          } else {
              wgotoxy(wp,0,0);
              return;
          }
      }

      /* reculer tant qu'on est sur du NON alphanumerique */
      while (!isalnum(line[i]) && i >= 0) {
             if (i == 0) {
                 if (lnprev(wp,pt) == NIL_LN) {  /* top of file */
                     wgotoxy(wp,0,0);
                     return;
                 }
                 arrow_up(wp);
                 line_end(wp);
                 i = lnlen(wp,current);
                 goto Continue;
             }
             i--;
      }

      /* se placer en debut de mot */
      while (isalnum(line[i])  && (i >= 0)) {
             if ((noline == 1) && (i == 0))
                 break ;
             i-- ;
      }
      if (i > 0 || noline > 1)   i++;

      /* mise a jour de l'affichage */
      if (i < wp->leftCh) {
          wp->leftCh = i;
          set_win(wp);
      }
      else
      if (i - (wp->ncol - 1) > wp->leftCh) {
          wp->leftCh = i - (wp->ncol - 1);
          set_win(wp);
      }
      wgotoxy(wp,i-wp->leftCh,wp->curY);
}
/*######################################
#######################################*/
unsigned char Xchg0[] = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,25,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',91,92,93,94,95,96,
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',123,124,125,126,127,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,
159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,
190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
252,253,254,255
};
unsigned char Xchg1[] = {
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,25,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',91,92,93,94,95,96,
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',123,124,125,126,127,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,
159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,
190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,
221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
252,253,254,255
};

unsigned char *pXchg; /* conversion table pointer */

unsigned char deltaTab[256];
/*#####################################
#######################################*/
/*-----------------------
 * mapString - fill the deltaTab
 *-----------------------
 */
mapString(str, size, findRepl)
char *str;
int size;
{
   int i, k, limit;
   unsigned char j;

   /* mapper la string dans la table */
   memset(deltaTab, 0, 256);
   for (i = 1, limit = INF(size-1, 255); i <= limit; i++) {
        if (((j = (unsigned char)str[size-i-1]) > 0x40) && (j < 0x5B))
                k = 0x20;
        else
                if ((j > 0x60) && (j < 0x7B))
                        k = -0x20;
                else    k = 0;

        /* insertion du code "case sensitive" */
        if (caseSensitive && (findRepl == REPLACE)) {
                pXchg = Xchg0;
                k = 0;
        }
        else
                pXchg = Xchg1;
        /* FIN insertion du code "case sensitive" */

        if (deltaTab[j] || deltaTab[j+k])
                continue;
        deltaTab[j] = deltaTab[j+k] = i;
   }
}

/*-----------------------------------------------------------
 * ctrlqf - get string to search and / or replace
 *-----------------------------------------------------------
 */
ctrlqf(wpe, mode)
struct OBJ *wpe;
{


   struct OBJ *wp, *ws, wsaisie;
   struct save_edit sed;
   char *str, *strcomm;
   unsigned Key, c;
   int ret, i, j;

   /* modif du 5/1/92 - */
   if (fflag & FF_LINE_UPDATE)
           insert_lnbuf(pedwin, ERASE);


   str     = findSTR;
   strcomm = find;

Continue:

   ret = doEnter(strcomm, str, FALSE, TRUE);

   hide_cursor();
   Key = ret;
   if (Key == (FUNC|F6)) {
       wgotoxy(pedwin, pedwin->curX, pedwin->curY);
       return(FUNC|F6);
   }

   if (mode == FIND) {
       wgotoxy(pedwin, pedwin->curX, pedwin->curY);
       ctrll(pedwin);
   }
   else
       if (mode == REPLACE) { /* mode FIND / REPLACE */
           str     = replaceSTR;
           strcomm = replace;
           mode    = MODE;       /* get mode */
           goto Continue;
       }
       else {
           saveEdit(&sed);
           ret = doChooseRepl(wp);
           restoreEdit(&sed);
           wgotoxy(pedwin, pedwin->curX, pedwin->curY);
           return(ret);
       }
}


/*-----------------------------------------------------------
 * ctrlqa - get searching string and replacing string
 *-----------------------------------------------------------
 */
ctrlqa(wp)
struct OBJ *wp;
{
   int mode, ret;

      if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
      }

   if (((mode = ctrlqf(wp, REPLACE)) & 0xff) == F6)
        return(FUNC|F6);

   /* saisie de la string a remplacer */
   if (mode < 2) {   /* recherche globale */
       current = wp->curX = wp->leftCh = wp->curY = topPage = 0;
       current_line_no = topPage_line_no = 1;
       set_win(wp);
   }
   while (1) {

         if (doFind(wp->curX + wp->leftCh, REPLACE) == 0) {
                /* fin du replace */
                hide_cursor(wp);
                pushPop_MESS(strNotFound, NMXMSG);
                setcursor(wp);
                formatt_line(linebuf, bigbuf, current, lnlen(wp, current), EOS, nb_car_per_line * 2);
                return;
         }

         if (mode == 0 || mode == 2) { /*!mode & 01) { /* mode Verify */
             if ((ret = print_yesno()) == 'Y' || ret == 'y')
                        doReplace(wp);
             else
                        if (ret == 'N' || ret == 'n')
                                continue;
                         else
                                break;  /* On Stop le Replace */
         }
         else    doReplace(wp);
   }
}
/*-----------------------------------------------------------
 * ctrlqg - goto line
 *-----------------------------------------------------------
 */
ctrlqg(wp, num)
struct OBJ *wp;
{
   char work[80];
   int line;
   unsigned i, j, ret;

   memset(work, '\0', 80);
   ret = doEnter(editstr[num].str, work, FALSE, FALSE);
   if (ret)
        return(ret);
   line = atoi(work);
   doGotoLine(wp, line);
}
/*-----------------------------------------------------------
 * ctrlql - line Length
 *-----------------------------------------------------------
 */
ctrlql(wp, num)
struct OBJ *wp;
{
   char work[80], work2[80];
   unsigned ret;
   int i;

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(wp, ERASE);

   memset(work, '\0', 80);
   strcpy(work2, " Enter line length - Current is ");
   strcat(work2, itoa(nb_car_per_line, NULL, 10));
   ret = doEnter(work2, work, FALSE, FALSE);
/*   hide_cursor();*/
   if (ret)
        return(ret);
   if ((i = atoi(work)) > 0)
        nb_car_per_line = i;

   free(fillstr);
   free(clippbuf);
   free(lineTab);
   free(linebuf);
   resize_line();
   pedwin->leftCh = pedwin->curX = 0;
   set_win(pedwin);
   return(FUNC|F6);
}

doGotoLine(wp, line)
struct OBJ *wp;
{
   current = topPage = getnextline(pedwin, 0, line - 1);
   current_line_no = topPage_line_no = get_line_no(current);
   pedwin->curX      = pedwin->curY      = pedwin->leftCh = 0;
   formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
   wgotoxy(wp, 0, 0);
   write_status(wp);
   set_win(pedwin);
}

/*-----------------------------------------------------------
 * ctrll - find string
 *-----------------------------------------------------------
 */
ctrll(wp)
struct OBJ *wp;
{
    if (doFind(wp->curX + wp->leftCh, FIND) == 0) {
        hide_cursor(wp);
        pushPop_MESS(strNotFound, NMXMSG);
        setcursor(wp);
        /* BBBUUUGGG peut etre trouv- */
        formatt_line(linebuf, bigbuf, current, lnlen(wp, current), EOS, nb_car_per_line * 2);

    }
}

/*-----------------------------------------------------------
 * doReplace - remplacer une chaine par une autre
 *-----------------------------------------------------------
 */
doReplace(wp)
struct OBJ *wp;
{
    int ecart, old, new, total, type;
    unsigned absolu;

    fflag |= FF_LINE_UPDATE;
    ecart = (old = strlen(findSTR)) - (new = strlen(replaceSTR));
    if (ecart)
        if (fflag & FF_BLOCKDEF) {
            if (((absolu = current + wp->curX + wp->leftCh) >= topBlock) &&
                 (absolu < bottomBlock))
                  bottomBlock -= ecart;
             else {
                 if (absolu < topBlock) {
                     topBlock -= ecart;
                     bottomBlock -= ecart;
                 }
            }
        }
    memmove(linebuf + wp->curX + wp->leftCh - ecart,
            linebuf + wp->curX + wp->leftCh ,
            ((nb_car_per_line - 1) * 2) - (wp->curX + wp->leftCh - ecart));
    memmove(linebuf + wp->curX + wp->leftCh - old,
            replaceSTR,
            new);
    insert_lnbuf(wp, ERASE);
    if ((total = wp->curX - ecart) > wp->ncol - 1) {
       wp->leftCh += total - wp->ncol + 1;
       set_win(wp);
    }
    else
        wcurrln(wp);
    wgotoxy(wp, wp->curX - ecart, wp->curY);
}
/*-----------------------------------------------------------
 * boyerMoore (m, offset, n)
        m      # de cara dans pattern
        n      # de cara dans string
        offset    offset de debut dans string
 *-----------------------------------------------------------
 */
int boyerMoore(m, offset, n)
int m,n;
unsigned offset;
{
        int i, j, k, delta;
        unsigned char ch, chT;
/*
        sprintf(work, " offset = %u ", offset);
        pushPop_MESS(work, ERRMSG);
*/
        if ((j = m-1+offset) > n-1) { /* la pattern depasse la ligne */
/*
                sprintf(work, " PB: j = %d i = %d ", j,i);
                pushPop_MESS(work, ERRMSG);
*/
                return(-1);
        }
/*
        sprintf(work, " DEBUT  size = %d ", j+1);
        pushPop_MESS(work, ERRMSG);
*/
        for (i = m-1, k = 0; j <= n-1;) {
/*
                sprintf(work, " indexPattern = %d Ch = \'%c\' *** indexStr = %d Ch = \'%c\' ", i, findSTR[i], j, linebuf[j]);
                pushPop_MESS(work, ERRMSG);
*/
                if  (((ch = findSTR[i]) != (chT = linebuf[j])) &&
                     (pXchg[ch] != chT)) {
                        if (delta = deltaTab[chT]){
                               /* le carac existe dans findSTR */
                               if (k > (delta - 1)) {
/*
                                        sprintf(work, " increment = %d ", k);
*/
                                        j += k + 1;
                               }
                               else {
/*
                                        sprintf(work, " increment = %d ", delta-1);
*/
                                        j += (delta - 1);
                               }
                        }
                        else {
/*
                               sprintf(work, " le carac n'est pas dans patt: inc = %d ", m);
*/
                               j += m;
                        }
/*
                        pushPop_MESS(work, ERRMSG);
*/
                        i = m-1;
                        k = 0;
                 }
                 else { /* egalite */
                        if (i == 0)   {
/*
                                sprintf(work, " Match: indexStr = %d ", j);
                                pushPop_MESS(work, ERRMSG);
*/
                                return(j);
                        }
                        k++;
                        i--;
                        j--;
                 }
        }
/*
        sprintf(work, " Non trouve:  indexPattern = %d  indexStr = %d ",i, j);
        pushPop_MESS(work, ERRMSG);
*/
        return(-1);
}

/*-----------------------------------------------------------
 * doFind - algo de recherche
 *-----------------------------------------------------------
 */
doFind(where, findRepl)
unsigned where;
{
    unsigned  pt, len, slen, extlen;
    int X, Y, refresh, diff, i;

    if (fflag & FF_LINE_UPDATE)
        insert_lnbuf(pedwin,ERASE);

    if ((slen = strlen(findSTR)) == 0)
        return(0);

    mapString(findSTR, slen+1, findRepl);

    for (pt = current; pt <= bottom; pt += len + 1) {
         /* formater chaque ligne dans le buffer ligne avant la recherche */
         len = lnlen(pedwin, pt);
         extlen = formatt_line(linebuf, bigbuf, pt, len, EOS, nb_car_per_line * 2);
         if ((i = boyerMoore(slen, INF(where, extlen), extlen)) >= 0) {
                  refresh = FALSE;
                  current = pt;
                  current_line_no = get_line_no(current);
                  if ((diff = current_line_no - topPage_line_no) >= pedwin->nline - 1) {
                       topPage_line_no = current_line_no - (pedwin->nline - 2);
                       topPage = getprevline(pedwin, current, pedwin->nline - 2);
                       refresh = TRUE;
                       Y = pedwin->nline - 2;
                  }
                  else
                       Y = diff;
                  if ((X = i + slen) > pedwin->ncol - 1) {
                       pedwin->leftCh = X - (pedwin->ncol - 1);
                       X -= pedwin->leftCh;
                       refresh = TRUE;
                  }
                  else
                      if (pedwin->leftCh) {
                          pedwin->leftCh = 0;
                          refresh = TRUE;
                      }
                  if (refresh) {
                      set_win(pedwin);
                      /*getch();*/
                  }
                  wgotoxy(pedwin, X, Y);
                  write_status(pedwin);
                  return(1);  /* FIND ! */
         }
/*        pushPop_MESS("Exit dofind", ERRMSG);*/
        where = 0;
    }
    return(0); /* unFIND */
}

/*-----------------------
 * ctrlt - delete word
 *-----------------------
 */
ctrlt(wp)
struct OBJ *wp;
{
     int plen, j, i, firstTime;
     char *line;
     unsigned pt;

      if (fflag & FF_INHIBE_MOD) {
        beep(1000);
        return;
      }

    /* tester si buffer ligne deja charg- */
     fflag |= (FF_LINE_UPDATE | FF_DIRTY);

     /* tester si position courante apres fin de ligne */
     if ((i = wp->curX + wp->leftCh) >= (plen = _lnlen(linebuf))) {
          add_blank(wp,plen,i);
          insert_lnbuf(wp,NO_ERASE);
          if ((pt = lnnext(wp,current)) != NIL_LN) {
               arrow_down(wp);
               wgotoxy(wp,0,wp->curY);
               wbacksp(wp);
               return;
          }
     }

     line = linebuf;
     j = i;
     /* determiner le caractere courant */
     if (isalnum(line[i]))  {
         /* on est sur un caractere : supprimer
          * tant qu'on est sur du caractere
          */
         while (isalnum(line[i]))  {
                /* supprimer caractere */
                supChar(wp,line, i);
         }
     }
     else {
         /* on est entre 2 mots ou sur de la ponctuation: supprimer
          * tant qu'on est sur du non caractere
          */
/*         while (!isalnum(line[i]) && line[i] != EOS)  {*/
         if (line[i] == ' ')
                while (line[i] == ' ' && line[i] != EOS)  {
                        /* supprimer caractere */
                        supChar(wp, line, i);
                }
         else
                while (ispunct(line[i]) && line[i] != EOS)  {
                        /* supprimer caractere */
                        supChar(wp, line, i);
                }

     }
     wcurrln(wp);
}

supChar(wp, line, i)
struct OBJ *wp;
char *line;
{
        int j;
                        /* supprimer caractere */
                        j = i;
                        while (line[j] != EOS) {
                                line[j] = line[j+1];
                                j++;
                        }
                        if (i < j) ctrltblk(wp);
}

/*----------------------------------------------------
 * ctrltblk - delete word : gestion marquage des blocs
 *----------------------------------------------------
 */
ctrltblk(wp)
struct OBJ *wp;
{
    unsigned absolu;

    /* GESTION BLOCK */
    if (fflag & FF_BLOCKDEF) {
        if (((absolu = current + wp->curX + wp->leftCh) >= topBlock) &&
             (absolu < bottomBlock))
              bottomBlock--;
        else {
             if (absolu < topBlock) {
                 topBlock--;
                 bottomBlock--;
             }
        }
    }
}

/*wwwwwwwwwwwwwwww*/
