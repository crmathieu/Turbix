/* wtools.c : choix de l'assembleur */

#include "xed.h"
#include "ext_var.h"
static int      confmkd_masm,
                confmkd_tasm,
                confautoSave,
                confkbdFast;
static int      replaceMode;

/*ÄÄÄÄÄÄÄÄÄÄ
 * doToolsChoice
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doToolsChoice(mp, num)
struct OBJ *mp;
{

   extern int prolog_it_mouse();
   struct OBJ ob, *pob;
   int ret_value;
   int i;
   char work[40], hlpName[40];

   struct  MOUSE_FIELD mf[7];
   int maA0(), maA1(), maA2(), maA3(), maA4(), maA5();

   /* charger la directory d'aide */
   strcpy(work, conf.u.pd.c_h);
   if (strlen(work) > 0)
        strcat(work, "\\");

   /* initialiser la fenetre CHOIX d'environnement */
   /* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TITLE ÄÄÄ        X  Y  L  H  BTYPE   OMBRAGE BARRE   ARROW INK        PAPER         WBORDER       BLOCK ATT ÄÄ */
   initWin(pob = &ob, 0, tooChoiceStr,20,6,40, 10, TYP1,   FALSE,  FALSE,  FALSE, WINHLPink, WINHLPpaper,  WINHLPborder, WINHLPblock);

   /* initialiser la zone Champ */
   pob->mField = mf;

   /*            f#  x   y  sz  (* )() */
   initField(pob, 0, 4,  3, 1,  maA0);
   initField(pob, 1, 4,  4, 1,  maA1);

   initField(pob, 2, 4,  5, 1,  maA4);
   initField(pob, 3, 4,  6, 1,  maA5);

   initField(pob, 4, 4,  8, 9,  maA2);
   initField(pob, 5, 19, 8, 9,  maA3);

   FIELD(pob, 6)->size      = 0;    /* marque la fin des champs */

   /* initialiser les actions souris */
   pob->M_Dpr  = m_editDpr;
   pob->M_Gpr  = m_editGpr;
   pob->M_Gre  = m_editGre;
   pob->M_CGpr = m_editCGpr;
   pob->M_CDpr = m_editCDpr;
   pob->mouse_ev_flag |= M_MOVMENT;

   winPopUp(pob, W_OPEN, OBJ_CHAINAGE, TYP1);
   print_at(pob, 1, 1, tooChoiceCmp, OLDATT);
   print_at(pob, 1, 3, tooChoiceAs1, OLDATT);
   print_at(pob, 1, 4, tooChoiceAs2, OLDATT);

   print_at(pob, 1, 5, tooKbdFastStr, OLDATT);
   print_at(pob, 1, 6, tooAutoSavStr, OLDATT);

   print_at(pob, 1, 8, lnkChoiceRep, OLDATT);
   if (conf.mkd_masm)
        print_at(pob, FIELD(pob, 0)->rX, FIELD(pob, 0)->rY, "X", OLDATT);

   if (conf.mkd_tasm)
        print_at(pob, FIELD(pob, 1)->rX, FIELD(pob, 1)->rY, "X", OLDATT);

   if (conf.autoSave)
        print_at(pob, FIELD(pob, 3)->rX, FIELD(pob, 3)->rY, "X", OLDATT);

   if (conf.kbdFast)
        print_at(pob, FIELD(pob, 2)->rX, FIELD(pob, 2)->rY, "X", OLDATT);

    confmkd_masm = conf.mkd_masm;
    confmkd_tasm = conf.mkd_tasm;
    confautoSave = conf.autoSave;
    confkbdFast  = conf.kbdFast;

   get_answer(pob, &i);
   winPopUp(pob, W_CLOSE);

   /* retour a l'editeur */
   hide_cursor();
   razMouseBuf();
   return(FUNC|F6);
}


maA0(p, no) /* MASM */
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!confmkd_masm) {
        confmkd_masm = TRUE;
        print_at(p, x, y, "X", OLDATT);
        print_at(p, x, FIELD(p, no+1)->rY, " ", OLDATT);
        confmkd_tasm = FALSE;
   }
   else {
        confmkd_masm = FALSE;
        print_at(p, x, y, " ", OLDATT);
   }
   return(0);
}

maA1(p, no)  /* TASM */
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!confmkd_tasm) {
        confmkd_tasm = TRUE;
        print_at(p, x, y, "X", OLDATT);
        print_at(p, x, FIELD(p, no-1)->rY, " ", OLDATT);
        confmkd_masm = FALSE;
   }
   else {
        confmkd_tasm = FALSE;
        print_at(p, x, y, " ", OLDATT);
   }
   return(0);
}

maA4(p, no)       /* vitesse clavier */
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!confkbdFast) {
        confkbdFast = TRUE;
        print_at(p, x, y, "X", OLDATT);
   }
   else {
        confkbdFast = FALSE;
        print_at(p, x, y, " ", OLDATT);
   }
   return(0);
}

maA5(p, no)  /* auto SAVE */
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!confautoSave) {
        confautoSave = TRUE;
        print_at(p, x, y, "X", OLDATT);
   }
   else {
        confautoSave = FALSE;
        print_at(p, x, y, " ", OLDATT);
   }
   return(0);
}

maA2(p)  /* Champs <OK> */
struct OBJ *p;
{
   /* enregistrer dans la configuration */
    conf.mkd_masm = confmkd_masm;
    conf.mkd_tasm = confmkd_tasm;
    conf.autoSave = confautoSave;
    conf.kbdFast  = confkbdFast;

   doKbSpeed(conf.kbdFast);
   return(1);
}
maA3(p)  /* champs <cancel> */
struct OBJ *p;
{
   return(-1);
}

/* <<<<<<<<<<<<<<<<<<< Choix du type de replace >>>>>>>>>>>>>>>>>>>>> */
/*ÄÄÄÄÄÄÄÄÄÄ
 * doChooseRepl
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doChooseRepl(mp, num)
struct OBJ *mp;
{

   extern int prolog_it_mouse();
   struct OBJ ob, *pob;
   int ret_value;
   int i;
   char work[40], hlpName[40];

   struct  MOUSE_FIELD mf[7];
   int mrepl0(), mrepl1(), mrepl2();

   /* initialiser le mode */
   replaceMode = 0;

   /* charger la directory d'aide */
   strcpy(work, conf.u.pd.c_h);
   if (strlen(work) > 0)
        strcat(work, "\\");

   /* initialiser la fenetre CHOIX de LINK */
   /* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ TITLE ÄÄÄ        X  Y  L  H  BTYPE   OMBRAGE BARRE   ARROW INK        PAPER         WBORDER       BLOCK ATT ÄÄ */
   i = strlen(validRP)+4;
   initWin(pob = &ob, 0, "",(80-i)/2,8,i, 8, TYP1,   FALSE,  FALSE,  FALSE, WINHLPink, WINHLPpaper,  WINHLPborder, WINHLPblock);

   /* initialiser la zone Champ */
   pob->mField = mf;

   /*            f#  x   y  sz  (* )() */
   initField(pob, 0, 4,  1, 1,  mrepl0);
   initField(pob, 1, 4,  2, 1,  mrepl0);
   initField(pob, 2, 4,  3, 1,  mrepl0);
   initField(pob, 3, 4,  4, 1,  mrepl0);
   initField(pob, 4, 4,  6, 9,  mrepl1);
   initField(pob, 5, 28, 6, 1,  mrepl2);

   FIELD(pob, 6)->size      = 0;    /* marque la fin des champs */

   /* initialiser les actions souris */
   pob->M_Dpr  = m_editDpr;
   pob->M_Gpr  = m_editGpr;
   pob->M_Gre  = m_editGre;
   pob->M_CGpr = m_editCGpr;
   pob->M_CDpr = m_editCDpr;
/*   pob->mouse_ev_flag |= M_MOVMENT;*/

   winPopUp(pob, W_OPEN, OBJ_CHAINAGE, TYP1);
   print_at(pob, 1, 1, rplmode[0], OLDATT);
   print_at(pob, 1, 2, rplmode[1], OLDATT);
   print_at(pob, 1, 3, rplmode[2], OLDATT);
   print_at(pob, 1, 4, rplmode[3], OLDATT);
   print_at(pob, 1, 5, caseSens,   OLDATT);
   print_at(pob, 1, 6, validRP,    OLDATT);
   if (caseSensitive)
        print_at(pob, 28,6, "X", OLDATT);

   ret_value = get_answer(pob, &i);
   winPopUp(pob, W_CLOSE);

   /* retour … replace*/
   hide_cursor();
   razMouseBuf();

   if (ret_value == (FUNC|F6))
        return(ret_value);

   return(replaceMode);
}

mrepl0(p, no)
struct OBJ *p;
{
   int x, y, i;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   replaceMode = no;
   for (i=0; i < 4; i++)
        if (i == no)
                print_at(p, x, y, "X", OLDATT);
        else
                print_at(p, x, FIELD(p, i)->rY, " ", OLDATT);
   return(0);
}

mrepl1(p)
struct OBJ *p;
{
   return(1);
}

mrepl2(p, no)
struct OBJ *p;
{
   int x, y, i;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (caseSensitive)
        print_at(p, x, y, " ", OLDATT);
   else
        print_at(p, x, y, "X", OLDATT);
   caseSensitive = (caseSensitive? FALSE : TRUE);
   return(0);
}
