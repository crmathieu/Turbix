/* help.c : gestion du help */

#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>
#include <errno.h>

#define XFILE  60
#define HELP    0
#define PREAMB  1
#define INTERF  2
#define MAKE    3
#define INDEX   4
#define INDEXA  5
#define ASCII   6

extern char *helpFName[];

/*ÄÄÄÄÄÄÄÄÄÄ
 * doHelp
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doHelp(mp, num)
struct OBJ *mp;
{

#define FIRST_CHAR 4  /* Position du 1er caractere de la func dans linebuf */

   extern int prolog_it_mouse();
   unsigned ret_value;
   int i, erreur;
   char work[40], hlpName[40];

   if (fflag & FF_HELP_MOD)  /* on est deja en mode help edit */
          edHelp(HCLOSE);


   /* sauvegarder les parametres globaux d'edition */
   saveEdit(&sedHelp);


   if ((bigbuf = malloc(BIGBUF_SIZE)) == NULL) {
        restoreEdit(&sedHelp);
        pushPop_MESS(Enomem, ERRMSG);
        return(FUNC|F6);
   }

   /* reinitialiser le pointeur de la fenetre d'edition
    * sur la fenetre d'aide
    */
   pedwin = &whelp;

   erreur = FALSE;

   /* charger la directory d'aide */
   strcpy(work, conf.u.pd.c_h);
   if (strlen(work) > 0)
        strcat(work, "\\");

   strcpy(hlpName, hlpStr); /* nom par defaut */

   /* si sous menu LIBRAIRIE charger d'abord le fichier de selection */
   if (helpInLine)
        num = -1;
   helpInLine = 0;

   /* charger le nom de fichier help */
   if (num >= 0)
        strcat(work, helpFName[num]);


   /* TESTER si Index d'AIDE >>>>>>>>>>>>>>>>>>>>>>>>>>>> LIB multitache */

   /* selection d'une fonction de la librairie */
   if ((num == INDEX) || (num == INDEXA)) {
       /* reinitialiser la fenetre editeur au format help SELECT */
       /* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TITLE ÄÄÄ       X  Y  L  H  BTYPE   OMBRAGE BARRE   ARROW INK        PAPER         WBORDER       BLOCK ATT ÄÄ */
       initWin(pedwin, 0, hlpSelectStr, 2, 3,74,17, TYP1,   FALSE,  FALSE,  TRUE, WINHLPink, WINHLPpaper,  WINHLPborder, WINHLPblock);

       /* placer press Bouton Droit */
       pedwin->M_Dpr  = m_editDpr;
       pedwin->M_Gpr  = m_editGpr;
       pedwin->M_Gre  = m_editGre;
       pedwin->M_CGpr = m_editCGpr;
       pedwin->M_CDpr = m_editCDpr;
       pedwin->mouse_ev_flag |= M_MOVMENT;

       winPopUp(pedwin, W_OPEN, OBJ_CHAINAGE, TYP1);

       /* inhiber les flags */
       fflag = 0;

       if (loadFile(0, work, work, 0, V_NOCREAT) < 0) {
           ret_value = FUNC|F6;
           goto SKIP_HELP;
       }
       /* rendre saisie impossible */
       fflag = (FF_INHIBE_MOD|FF_HELP_MOD);

       formatt_line(linebuf, bigbuf, current, lnlen(pedwin, current), EOS, nb_car_per_line * 2);

       /* initialiser la barre de selection */
       topBlock        = 0;
       bottomBlock     = lnnext(pedwin, 0);

       /* recuperer la ligne selectionn‚e (current_line_no) */
       if (num == INDEX)
                ret_value = get_helpCommand(pedwin);
       else
                ret_value = get_helpCommandA(pedwin);

       if (ret_value == 0) {
           /* R‚cuperer le nom de la fonction */
           for (i=FIRST_CHAR; i<nb_car_per_line; i++)
                  if ((work[i - FIRST_CHAR] = linebuf[i]) == ' ') {
                               work[i - FIRST_CHAR] = '\0';
                               break;
                  }
                  /* R‚cuperer le nom de fichier */
                  for (i=0; strlen(hlpFile[i]) ; i += 2) {
                       if (stricmp(work, hlpFile[i]) == 0)
                           break;
                  }
                  /* copier le nom de fichier */
                  if (strlen(hlpFile[i])) {
                        strcpy(work, conf.u.pd.c_h);
                        if (strlen(work) > 0)
                                strcat(work, "\\");
                        strcat(work, hlpFile[i+1]);
                        strcpy(hlpName, hlpFile[i]);
                  }
                  else
                        erreur = TRUE;
       }
       else
            goto SKIP_HELP;

       winPopUp(pedwin, W_CLOSE);
   }
   else
        if (num < 0) { /* appel direct */
            strcpy(work, defhlp);

            /* R‚cuperer le nom de fichier */
            for (i=0; strlen(hlpFile[i]); i += 2) {
                 if (stricmp(work, hlpFile[i]) == 0)
                     break;
            }
            /* copier le nom de fichier */
            if (strlen(hlpFile[i])) {
                strcpy(work, conf.u.pd.c_h);
                if (strlen(work) > 0)
                        strcat(work, "\\");
                strcat(work, hlpFile[i+1]);
                strcpy(hlpName, hlpFile[i]);
            }
            else
                erreur = TRUE;
        }

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  FIN LIB multitache */

   if (erreur) {
       pushPop_MESS(UnknownStr, ERRMSG);
       ret_value = FUNC|F6;
       goto SKIP_HELP;
   }

   /* Charger le HELP Final */
   /* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TITLE ÄÄÄ       X  Y  L  H  BTYPE   OMBRAGE BARRE   ARROW INK        PAPER         WBORDER       BLOCK ATT ÄÄ */
   initWin(pedwin, FUNC|F6, hlpName,        2, 3,74,17, TYP1,   FALSE,  FALSE,  TRUE, WINHLPink, WINHLPpaper,  WINHLPborder, WINHLPblock);

   /* placer press Bouton Droit */
   pedwin->M_Dpr  = m_editDpr;
   pedwin->M_Gpr  = m_editGpr;
   pedwin->M_Gre  = m_editGre;
   pedwin->M_CGpr = m_editCGpr;
   pedwin->M_CDpr = m_editCDpr;
   pedwin->mouse_ev_flag |= M_MOVMENT;


   winPopUp(pedwin, W_OPEN, OBJ_CHAINAGE, TYP1);

   /* inhiber les flags */
   fflag = 0;

   /* charger le fichier final */
   if (loadFile(0, work, work, 0, V_NOCREAT) < 0) {
        ret_value = FUNC|F6;
        goto SKIP_HELP;
   }

   /* indiquer mode help avec interdiction de saisie */
   fflag = (FF_INHIBE_MOD|FF_HELP_MOD);

   formatt_line(linebuf, bigbuf, current, lnlen(pedwin, current), EOS, nb_car_per_line * 2);

   ret_value = get_command(pedwin);
   if (ret_value != (FUNC|F6)) { /* on ne retourne pas dans l'editeur */
        hide_cursor();
        return(ret_value);
   }
/*   pushPop_MESS("HELP : Fin de cet ITEM...", ERRMSG);*/

SKIP_HELP:

   winPopUp(pedwin, W_CLOSE);

SKIP_HELP2:

   free(bigbuf);

   /* restaurer le pointeur sur la fenetre d'edition */
   pedwin = &edwin;

   /* restaurer les parametres globaux d'edition */
   restoreEdit(&sedHelp);

   /* retour a l'editeur */
   hide_cursor();
   razMouseBuf();
   return(ret_value);
}

/*ÄÄÄÄÄÄÄÄ
 * edHelp
 *ÄÄÄÄÄÄÄÄ
 */
edHelp(action)
{
   extern int prolog_it_mouse();
   unsigned ret_value;

   ret_value = FUNC|F6;  /* init valeur de retour */
   /*fflag &= ~FF_HELP_MOD;*/
   if (action == HREOPEN) {

        /* chainer l'OBJ editeur comme
         * objet courant
         */

        currObj    = pedwin;
        currObj->mouse_ev_flag |= M_MOVMENT;
        m_callMaskAndAddress(currObj->mouse_ev_flag, prolog_it_mouse);

        formatt_line(linebuf, bigbuf, current, lnlen(pedwin, current), EOS, nb_car_per_line * 2);
        write_status(pedwin);

        ret_value = get_command(pedwin);
        if (ret_value != (FUNC|F6)) { /* on ne retourne pas dans l'editeur */
                hide_cursor();
                return(ret_value);
        }
   }
   /* HCLOSE */
   fflag &= ~FF_HELP_MOD;
   winPopUp(pedwin, W_CLOSE);

   free(bigbuf);

   /* restaurer le pointeur sur la fenetre d'edition */
   pedwin = &edwin;

   /* restaurer les parametres globaux d'edition */
   restoreEdit(&sedHelp);

   /* retour a l'editeur */
   hide_cursor();
   return(ret_value);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * get_helpCommand - balayer la fenetre de selection
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
get_helpCommandA(wp)
struct OBJ *wp;
{
      unsigned lnnext();
      int val, x, y;

      int i,go,ret;
      unsigned  ChL,ChX,Key,Yreference;

      fflag |= FF_EDACTIF|FF_BLOCKDEF;

      /* initialiser la position de la barre de selection
       * sur la premiere fonction a selectionner
       */
      Yreference = 0;
      while (1) {

Continue:
             hide_cursor(wp);
             write_page_att(wp);
             Key = getkey();

SKIP_KEY:
             write_page_att(wp);
             ChL = (Key & 0x00ff);
             ChX = (Key & 0xff00);
             switch (ChX) {
             case NUMPAD: switch(ChL) {
                          case PGUP   : for (i=0; (current_line_no != 1) &&
                                                  (i < wp->nline); i++)
                                                if (arrow_up(wp))
                                                        goto Continue;
                                        break;

                          case PGDN   : for (i=0; (current_line_no != bottom_line_no) &&
                                                  (i < wp->nline); i++)
                                                if (arrow_down(wp))
                                                        goto Continue;
                                        /*if (current_line_no != bottom_line_no) {
                                                Key = NUMPAD|ARROWU;
                                                goto SKIP_KEY;
                                        };*/
                                        break;

                          case ARROWU : if (current_line_no == 1)
                                                continue;
                                        do
                                                if (arrow_up(wp)) {
                                                        Key = NUMPAD|ARROWD;
                                                        goto SKIP_KEY;
                                                }
                                        while (linebuf[0] != '.');
                                        break;
                          case ARROWD : if (current_line_no == bottom_line_no)
                                                continue;
                                        do
                                                if (arrow_down(wp))
                                                        goto Continue;
                                        while (linebuf[0] != '.');
                                        break;
                          case ARROWL : line_home(wp);
/*                                      arrow_left(wp);
                                        if (wp->leftCh)
                                                wp->leftCh--;
                                        set_win(wp);*/
                                        break;
                          case ARROWR : wp->leftCh = wp->ncol - 4;
                                        set_win(wp);
/*                                      arrow_right(wp);
                                        wp->leftCh++;
                                        set_win(wp);*/
                                        break;
                          case MOUSE_L :
                          case MOUSE_LP:
                                        m_setMinMaxVerCursorPos(wp->ul_y * 8, (wp->ul_y + wp->nline + 1) * 8);
                                        m_setMinMaxHorCursorPos(wp->ul_x * 8, (wp->ul_x + wp->ncol  + 1) * 8);
                                        /* tester presence sur le contour */
                                        if (posX == wp->ul_x)
                                                Key = NUMPAD|ARROWL;
                                        else
                                                if (posX == wp->ul_x + wp->ncol + 1)
                                                        Key = NUMPAD|ARROWR;
                                                else
                                                        if (posY == wp->ul_y)
                                                                Key = NUMPAD|ARROWU;
                                                        else
                                                                if (posY == wp->ul_y + wp->nline + 1)
                                                                        Key = NUMPAD|ARROWD;
                                                                else  {
                                                                        moveBloc(wp);
                                                                        break;
                                                                }
                                        hide_cursor(wp);
                                        ticks = 0;
                                        while (ticks < 1) ;
                                        mouseEvent++;  /* boucler sur la pression souris */
/*                                      topBlock = current;
                                        bottomBlock = lnnext(wp, current);*/
                                        goto SKIP_KEY;

                          case MOUSE_LR : /* relache bouton gauche */
                                        m_setMinMaxVerCursorPos(0, 192);
                                        m_setMinMaxHorCursorPos(0, 639);
                                        if (posX == XMOUSE   && posY == YMOUSE &&
                                            YMOUSE - (wp->ul_y + 1) == Yreference) {
                                                /* la position de la souris est
                                                 * inchangee depuis la pression :
                                                 * positionner le curseur … cet emplacement
                                                 */
                                                Key = NORMAL|RETURN;
                                                goto SKIP_KEY;
                                        }
                                        Yreference = wp->curY;
                                        moveBloc(wp);
                                        break;

                          case MOUSE_R :
                          case MOUSE_RP:
                                        m_setMinMaxVerCursorPos(wp->ul_y * 8, (wp->ul_y + wp->nline + 1) * 8);
                                        m_setMinMaxHorCursorPos(wp->ul_x * 8, (wp->ul_x + wp->ncol  + 1) * 8);
                                        /* tester presence sur le contour */
                                        if (posX == wp->ul_x)
                                                Key = NUMPAD|ARROWL;
                                        else
                                                if (posX == wp->ul_x + wp->ncol + 1)
                                                        Key = NUMPAD|ARROWR;
                                                else
                                                        if (posY == wp->ul_y)
                                                                Key = NUMPAD|PGUP;
                                                        else
                                                                if (posY == wp->ul_y + wp->nline + 1)
                                                                        Key = NUMPAD|PGDN;
                                                                else
                                                                        break;
                                        goto SKIP_KEY;

                          case MOUSE_RR : /* relache bouton droit */
                                        m_setMinMaxVerCursorPos(0, 192);
                                        m_setMinMaxHorCursorPos(0, 639);
                                        break;

                          default     : continue;
                          }
                          hide_cursor(wp);
                          if (linebuf[0] == '.') {
                                topBlock = current;
                                bottomBlock = lnnext(wp, current);
                          }
/*                        write_page_att(wp); */
                          break;
             case NORMAL: switch(ChL) {
                          case ESC : return(FUNC|F6);
                          case RETURN : /* linebuf contient la
                                         * fonction selectionnee
                                         */
                                        return(0);
                          default: break;
                          }
                          break;
             case ALT    :
             case ALTFUNC:
             case FUNC   :
                          hide_cursor();
                          return(Key);
             }
      }
}

get_helpCommand(wp)
struct OBJ *wp;
{
      unsigned lnnext();
      int val, x, y;

      int i,go,ret;
      unsigned  ChL,ChX,Key,Yreference;

      fflag |= FF_EDACTIF|FF_BLOCKDEF;

      /* initialiser la position de la barre de selection
       * sur la premiere fonction a selectionner
       */
      Key = NUMPAD|ARROWD;
      Yreference = 0;
      goto SKIP_KEY;


      while (1) {

Continue:
             hide_cursor(wp);
             write_page_att(wp);
             Key = getkey();

SKIP_KEY:
             ChL = (Key & 0x00ff);
             ChX = (Key & 0xff00);
             switch (ChX) {
             case NUMPAD: switch(ChL) {

                          case PGUP   : for (i=0; (current_line_no != 1) &&
                                                  (i < wp->nline); i++)
                                                if (arrow_up(wp))
                                                        goto Continue;
                                        Key = NUMPAD|ARROWD;
                                        goto SKIP_KEY;

                          case PGDN   : for (i=0; (current_line_no != bottom_line_no) &&
                                                  (i < wp->nline); i++)
                                                if (arrow_down(wp))
                                                        goto Continue;
                                        if (current_line_no != bottom_line_no) {
                                                Key = NUMPAD|ARROWU;
                                                goto SKIP_KEY;
                                        };
                                        break;

                          case ARROWU : if (current_line_no == 1)
                                                continue;
                                        do
                                                if (arrow_up(wp)) {
                                                        Key = NUMPAD|ARROWD;
                                                        goto SKIP_KEY;
                                                }
                                        while (linebuf[0] != '.');
                                        break;
                          case ARROWD : if (current_line_no == bottom_line_no)
                                                continue;
                                        do
                                                if (arrow_down(wp))
                                                        goto Continue;
                                        while (linebuf[0] != '.');
                                        break;
                          case ARROWL : line_home(wp);
/*                                      arrow_left(wp);
                                        if (wp->leftCh)
                                                wp->leftCh--;
                                        set_win(wp);*/
                                        break;
                          case ARROWR : wp->leftCh = wp->ncol - 4;
                                        set_win(wp);
/*                                      arrow_right(wp);
                                        wp->leftCh++;
                                        set_win(wp);*/
                                        break;
                          case MOUSE_L :
                          case MOUSE_LP:
                                        m_setMinMaxVerCursorPos(wp->ul_y * 8, (wp->ul_y + wp->nline + 1) * 8);
                                        m_setMinMaxHorCursorPos(wp->ul_x * 8, (wp->ul_x + wp->ncol  + 1) * 8);
                                        /* tester presence sur le contour */
                                        if (posX == wp->ul_x)
                                                Key = NUMPAD|ARROWL;
                                        else
                                                if (posX == wp->ul_x + wp->ncol + 1)
                                                        Key = NUMPAD|ARROWR;
                                                else
                                                        if (posY == wp->ul_y)
                                                                Key = NUMPAD|ARROWU;
                                                        else
                                                                if (posY == wp->ul_y + wp->nline + 1)
                                                                        Key = NUMPAD|ARROWD;
                                                                else  {
                                                                        moveBloc(wp);
                                                                        break;
                                                                }
                                        hide_cursor(wp);
                                        ticks = 0;
                                        while (ticks < 1) ;
                                        mouseEvent++;  /* boucler sur la pression souris */
/*                                      topBlock = current;
                                        bottomBlock = lnnext(wp, current);*/
                                        goto SKIP_KEY;

                          case MOUSE_LR : /* relache bouton gauche */
                                        m_setMinMaxVerCursorPos(0, 192);
                                        m_setMinMaxHorCursorPos(0, 639);
                                        if (posX == XMOUSE   && posY == YMOUSE &&
                                            YMOUSE - (wp->ul_y + 1) == Yreference) {
                                                /* la position de la souris est
                                                 * inchangee depuis la pression :
                                                 * positionner le curseur … cet emplacement
                                                 */
                                                Key = NORMAL|RETURN;
                                                goto SKIP_KEY;
                                        }
                                        Yreference = wp->curY;
                                        moveBloc(wp);
                                        break;

                          case MOUSE_R :
                          case MOUSE_RP:
                                        m_setMinMaxVerCursorPos(wp->ul_y * 8, (wp->ul_y + wp->nline + 1) * 8);
                                        m_setMinMaxHorCursorPos(wp->ul_x * 8, (wp->ul_x + wp->ncol  + 1) * 8);
                                        /* tester presence sur le contour */
                                        if (posX == wp->ul_x)
                                                Key = NUMPAD|ARROWL;
                                        else
                                                if (posX == wp->ul_x + wp->ncol + 1)
                                                        Key = NUMPAD|ARROWR;
                                                else
                                                        if (posY == wp->ul_y)
                                                                Key = NUMPAD|PGUP;
                                                        else
                                                                if (posY == wp->ul_y + wp->nline + 1)
                                                                        Key = NUMPAD|PGDN;
                                                                else
                                                                        break;
                                        goto SKIP_KEY;

                          case MOUSE_RR : /* relache bouton droit */
                                        m_setMinMaxVerCursorPos(0, 192);
                                        m_setMinMaxHorCursorPos(0, 639);
                                        break;

                          default     : continue;
                          }
                          hide_cursor(wp);
                          if (linebuf[0] == '.') {
                                topBlock = current;
                                bottomBlock = lnnext(wp, current);
                          }
/*                        write_page_att(wp); */
                          break;
             case NORMAL: switch(ChL) {
                          case ESC : return(FUNC|F6);
                          case RETURN : /* linebuf contient la
                                         * fonction selectionnee
                                         */
                                        return(0);
                          default: break;
                          }
                          break;
             case ALT    :
             case ALTFUNC:
             case FUNC   :
                          hide_cursor();
                          return(Key);
             }
      }
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * moveBloc
 *ÄÄÄÄÄÄÄÄÄÄ
 */
moveBloc(wp)
struct OBJ *wp;
{
        int x, y, X, Y;
        unsigned savCur;

        x = wp->curX; y = wp->curY;
        if (posY == wp->ul_y + wp->nline + 1)
                Y = posY-1; /*BIZARRE */
        else
                Y = posY;
        if ((Y = Y - (wp->ul_y + 1)) < 0)
                Y = 0;
        if ((X = (posX) - (wp->ul_x + 1)) < 0)
                X = 0;
        if (X == x && Y == y)
                return(1);

        savCur = current_line_no;

        /* verifier qu'on est toujours dans le fichier */
        if (bottom_line_no >= (current_line_no + Y - wp->curY))
                current_line_no += (Y - wp->curY);
        else
                return(1);
        wmgotoxy(wp, X, Y);

        if (bigbuf[current] != '.') {
                wmgotoxy(wp,x,y);
                current_line_no = savCur;
        }
        formatt_line(linebuf, bigbuf, current, lnlen(wp,current), EOS, nb_car_per_line * 2);
        return(0);

}

