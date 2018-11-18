/* wlink.c : gestion du Link */

#include "xed.h"
#include "ext_var.h"

/*ÄÄÄÄÄÄÄÄÄÄ
 * isOnField - determine si le curseur souris est sur un champs
 *             si oui, retourne l'action … declencher pour ce champs
 *             si non, retourne la fonction nulle
 *ÄÄÄÄÄÄÄÄÄÄ
 */
isOnField(ob)
struct OBJ *ob;
{

   int x,i=0;

   while (FIELD(ob, i)->size) {
          if ((YMOUSE == FIELD(ob, i)->rY + ob->ul_y + 1) && ((XMOUSE >= (x = FIELD(ob, i)->rX + ob->ul_x + 1))
                &&  (XMOUSE < x + FIELD(ob, i)->size)))
                /* la souris est sur un champs */
                return(i);
          else
                i++;
   }
   return(-1);
}

initField(pob, no, rx, ry, size, action)
struct OBJ *pob;
int (* action)();
{
   FIELD(pob, no)->rX  = rx;
   FIELD(pob, no)->rY  = ry;
   FIELD(pob, no)->size = size;
   FIELD(pob, no)->MouAction = action;
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doLnkChoice
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doLnkChoice(mp, num)
struct OBJ *mp;
{

   extern int prolog_it_mouse();
   struct OBJ ob, *pob;
   int ret_value;
   int i;
   char work[40], hlpName[40];

   struct  MOUSE_FIELD mf[5];
   int maL0(), mal01(), maL1(), maL2(), maL3();

   /* charger la directory d'aide */
   strcpy(work, conf.u.pd.c_h);
   if (strlen(work) > 0)
        strcat(work, "\\");

   /* initialiser la fenetre CHOIX de LINK */
   /* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TITLE ÄÄÄ        X  Y  L  H  BTYPE   OMBRAGE BARRE   ARROW INK        PAPER         WBORDER       BLOCK ATT ÄÄ */
   initWin(pob = &ob, 0, lnkChoiceStr,15,8,52, 5, TYP1,   FALSE,  FALSE,  FALSE, WINHLPink, WINHLPpaper,  WINHLPborder, WINHLPblock);

   /* initialiser la zone Champ */
   pob->mField = mf;
   initField(pob, 0, 4, 1, 1, maL0);
   initField(pob, 1, 4, 2, 1, maL1);

/* initField(pob, 1, 4, 2, 1, maL1);
   initField(pob, 2, 4, 4, 9, maL2);
   initField(pob, 3, 19,4, 9, maL3);
   FIELD(pob, 4)->size      = 0;    /* marque la fin des champs */

   initField(pob, 2, 4, 4, 9, maL2);
   initField(pob, 3, 19,4, 9, maL3);

   FIELD(pob, 4)->size      = 0;    /* marque la fin des champs */

   /* initialiser les actions souris */
   pob->M_Dpr  = m_editDpr;
   pob->M_Gpr  = m_editGpr;
   pob->M_Gre  = m_editGre;
   pob->M_CGpr = m_editCGpr;
   pob->M_CDpr = m_editCDpr;
/*   pob->mouse_ev_flag |= M_MOVMENT;*/

   winPopUp(pob, W_OPEN, OBJ_CHAINAGE, TYP1);
   print_at(pob, 1, 1, lnkChoiceInt, OLDATT);
/*   print_at(pob, 1, 2, lnkChoiceFS,  OLDATT);*/
   print_at(pob, 1, 2, lnkAutoAsk,  OLDATT);
   print_at(pob, 1, 4, lnkChoiceRep, OLDATT);
   if (mkd_shell = conf.mkd_shell)
        print_at(pob, FIELD(pob, 0)->rX, FIELD(pob, 0)->rY, "X", OLDATT);

/*   if (mkd_FS = conf.mkd_FS)
        print_at(pob, FIELD(pob, 1)->rX, FIELD(pob, 1)->rY, "X", OLDATT);*/

   if (mkd_AutoAsk = conf.mkd_AutoAsk)
        print_at(pob, FIELD(pob, 1)->rX, FIELD(pob, 1)->rY, "X", OLDATT);

   ret_value = get_answer(pob, &i);

   winPopUp(pob, W_CLOSE);

   /* retour a l'editeur */
   hide_cursor();
   razMouseBuf();
   return(ret_value); /*FUNC|F6);*/
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * get_answer - recuperer une reponse dans une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
get_answer(wp, field)
struct OBJ *wp;
int *field;
{
      int val, x, y, lig;

      int i,go,ret,nfield;
      unsigned  ChL,ChX,Key,Yreference;

      for (nfield = 0; FIELD(wp, nfield)->size; nfield++) ;

      i = 0;
      wgotoxy(wp, FIELD(wp, i)->rX, FIELD(wp, i)->rY);
      goto SKIP_KEY;


      while (1) {

Continue:

             Key = getkey();

SKIP_KEY:
             ChL = (Key & 0x00ff);
             ChX = (Key & 0xff00);
             switch (ChX) {
             case NUMPAD: switch(ChL) {
                          case MOUSE_L :
                          case MOUSE_LP: break;

                          case MOUSE_LR : /* relache bouton gauche */
                                        if ((i = isOnField(wp)) >= 0) {
                                                Key = (NORMAL|RETURN);
                                                goto SKIP_KEY;
                                        }
                                        else
                                                break;

                          default     : continue;
                          }

             case NORMAL: switch(ChL) {
                          case ESC    : return(FUNC|F6);
                          case TAB    : i = (i + 1) % nfield;
                                        wgotoxy(wp, FIELD(wp, i)->rX, FIELD(wp, i)->rY);
                                        break;
                          case BLANK  :
                          case RETURN :
                                        if ((ret = (* FIELD(wp, i)->MouAction)(wp, i))) {
                                                *field = i;
                                                return(i);
                                        }
                                        break;
                          default:      break;
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

maL0(p, no)
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!mkd_shell)
        print_at(p, x, y, "X", OLDATT);
   else
        print_at(p, x, y, " ", OLDATT);
   mkd_shell = ~mkd_shell;
   return(0);
}

maL1(p, no)  /* Auto Ask */
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!mkd_AutoAsk)
        print_at(p, x, y, "X", OLDATT);
   else
        print_at(p, x, y, " ", OLDATT);
   mkd_AutoAsk = ~mkd_AutoAsk;
   return(0);
}

maL2(p)
struct OBJ *p;
{
   /* enregistrer dans la configuration */
   conf.mkd_shell   = mkd_shell;
   conf.mkd_AutoAsk = mkd_AutoAsk;
   return(1);
}
maL3(p)
struct OBJ *p;
{
/*   mkd_shell = mkd_FS = FALSE;*/
   return(-1);
}
