/* xed0.c : gestion des menus */

#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>


/*---------------------------------------------------
 * global_init : initialisation GENERALE et lancement
 *---------------------------------------------------
 */
FUNCTION global_init(ffile, clearScreen)
char *ffile;
{
#define  NOLCOM_ATT   FALSE
#define  LCOM_ATT     TRUE

   int ret, fmsg, fdconf, confFlag, i, initKey, X, Y;
   struct OBJ *ob;

   extern int  disk_err(), mv0(), mv1(), mv2(), prolog_it_mouse();
   extern void interrupt _itcritical();
   void interrupt Dummy_interrupt();
   extern void interrupt prolog_it_doubleClic();
   extern void interrupt it_clk();

   /* config machine */
/*   if (clearScreen)
        set_mode_base();
   else {*/
        /* recuperer les parametres video */
        SEGvideo = vl->segVideo;
        adapter  = vl->adapter;

/*   }*/
/*   _harderr(disk_err);*/
   equip_list = _bios_equiplist();
   nflp_phy = ((equip_list & 0x00c0) >> 6) + 1;

   /* inhiber le ^C */
   _dos_setvect(0x23, Dummy_interrupt);

   /* changer la gestion des erreurs Hardwares */
   DosInt24 = (unsigned long)_dos_getvect(0x24);
   _dos_setvect(0x24, _itcritical);

   /* recuperer le vecteur int10h */
   BiosInt10 = (unsigned long)_dos_getvect(0x10);

   /* GESTION SOURIS */
/*--------------------------------------------------------------
 * m_callMaskAndAddress - chaine - l'IT souris, le traitement
 *                       utilisateur a appeler si les conditions
 *                       exprim-es dans le masque sont reunies:
 *                              - bit 0 : changement de position
 *                              - bit 1 : bouton Gauche press-
 *                              - bit 2 : bouton Gauche relach-
 *                              - bit 3 : bouton Droit press-
 *                              - bit 4 : bouton Droit relach-
 *--------------------------------------------------------------

   /* check mouse */
   m_getPosAndButtonStatus(&X, &Y, &i);
   vl->mouY = Y;
   vl->mouX = X;

   if (!m_installed(&nbuttom)) {
        /* pas de souris, installer
         * l'interruption DUMMY
         */
        BiosInt33 = (unsigned long)_dos_getvect(0x33);
        _dos_setvect(0x33, Dummy_interrupt);
        mouse_present = FALSE;
   }
   else {
        /* positionner l 'interruption souris editeur */
/*        m_callMaskAndAddress(M_LEFT_RELEASE|M_LEFT_PRESS, prolog_it_mouse);*/

        M_in = M_out = 0;     /* pointeurs buffer souris */
        mouse_present = TRUE;
        m_setCursorPos(vl->mouX, vl->mouY);
        showMouse();
   }
   /* recuperer le vecteur int1Ch */
   BiosIntclk = (unsigned long)_dos_getvect(0x1c);
   _dos_setvect(0x1c, it_clk);




   /* initialiser le pick defaut */
   pckdefault.current         = 0;
   pckdefault.topPage         = 0;
   pckdefault.topBlock        = 0;
   pckdefault.bottomBlock     = 0;
   pckdefault.current_line_no = 1;
   pckdefault.topPage_line_no = 1;
   pckdefault.leftCh          = 0;
   pckdefault.curX            = 0;
   pckdefault.curY            = 0;
   pckdefault.fsize           = 0;
   pckdefault.fflag           = FF_INSERT;

   /* chainer les objets "menus" avec leur contenu */
   mainMenu.items  = mainstr;
   fileMenu.items  = filestr;
   editMenu.items  = editstr;
   pickMenu.items  = pickstr;
   compMenu.items  = comstr;
   bltMenu.items   = bltstr;
   runMenu.items   = runstr;
   optMenu.items   = optstr;
   helpMenu.items  = helpstr;
   instMenu.items  = confstr;


   if (adapter != MONO) {          /* EGA / CGA */
       /* fenetre editeur */
       WINink         =  F_LYELLOW;
       WINpaper       =  B_BLUE;
       WINborder      =  F_WHITE|B_BLACK;
       WINblock       =  F_BLUE|B_WHITE;

       /* fenetre de messages */
       WINSYSink      =  F_BLACK;
       WINSYSpaper    =  B_WHITE;
       WINSYSborder   =  F_BLACK|B_WHITE;
       WINSYSblock    =  F_LWHITE;   /*F_LYELLOW|B_BLUE;*/

       /* fenetre de message d'erreur */
       WINmessInk     =  F_LWHITE;
       WINmessPaper   =  B_RED;
       WINmessBorder  =  F_BLACK|B_RED;

       /* attributs des DIRECTORIES dans la fenetre de dialogue */
       WINDIRink      =  F_LWHITE;
       WINDIRpaper    =  WINSYSpaper;

       /* fenetre d'AIDE */
       WINHLPink      =  F_BLACK;
       WINHLPpaper    =  B_WHITE;
       WINHLPborder   =  F_LWHITE|B_WHITE;
       WINHLPblock    =  F_WHITE|B_BLACK;

       /* attributs communs - tous les menus */
       MENUborder     =  F_BLACK|B_WHITE;
       MENUselected   =  F_WHITE|B_BLACK;
       MENUlcom       =  F_LWHITE|B_WHITE;
       MENUink        =  F_BLACK;
       MENUpaper      =  B_WHITE;
   }
   else {                         /* MONO */
       /* fenetre editeur */
       WINink         =  F_WHITE;
       WINpaper       =  B_BLACK;
       WINborder      =  F_WHITE|B_BLACK;
       WINblock       =  F_BLACK|B_WHITE;

       /* fenetre de messages */
       WINSYSink      =  F_BLACK;
       WINSYSpaper    =  B_WHITE;
       WINSYSborder   =  F_BLACK|B_WHITE;
       WINSYSblock    =  F_LWHITE|B_BLACK;

       /* fenetre de message d'erreur */
       WINmessInk     =  F_WHITE;
       WINmessPaper   =  B_BLACK;
       WINmessBorder  =  F_WHITE|B_BLACK;

       /* attributs des DIRECTORIES dans la fenetre de dialogue */
       WINDIRink      =  F_BLACK;
       WINDIRpaper    =  WINSYSpaper;

       /* fenetre d'AIDE */
       WINHLPink      =  F_BLACK;
       WINHLPpaper    =  B_WHITE;
       WINHLPborder   =  F_BLACK|B_WHITE;
       WINHLPblock    =  F_LWHITE|B_BLACK;

       /* attributs communs - tous les menus */
       MENUborder     =  F_WHITE|B_BLACK;
       MENUselected   =  F_BLACK|B_WHITE;
       MENUlcom       =  F_LWHITE|B_BLACK;
       MENUink        =  F_WHITE;
       MENUpaper      =  B_BLACK;
   }

   /* charger fichier de configuration */
   confFlag = TRUE;
   if ((fdconf = open(configFile,O_RDONLY|O_BINARY)) != -1)
        read(fdconf, &conf, sizeof(struct configuration));
   else
        if ((fdconf = open(configFile,O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE)) < 0) {
                beep(1000);
                pushPop_MESS(confErrStr,ERRMSG);
                confFlag = FALSE;
        }
   close(fdconf);

   init_pick(ffile, confFlag);
   init_comp();

/* ------------------- m_key -- TITLE ---------------------------------- UX  UY -- ink -- paper ---- border -- selected Item - command letter att --- flagLcom */
   initMenu(&mainMenu, FUNC|F10, "main",     NULL,      HOR, FIXM, NULL,    0,  0, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);
   initMenu(&fileMenu, ALT|ALT_F,nullstr,    &mainMenu, VER, FIXM, NULL,    4,  1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);
   initMenu(&editMenu, ALT|ALT_E,nullstr,    &mainMenu, VER, FIXM, NULL,    14, 1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);
   initMenu(&pickMenu, ALT|ALT_F3,picktitle, &fileMenu, VER, VARM, videoBuf,20, 5, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE, NOLCOM_ATT);
   update_menu(&pickMenu);
   initMenu(&compMenu, ALT|ALT_C,nullstr,    &mainMenu, VER, FIXM, NULL,    24, 1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE, LCOM_ATT);
    initMenu(&bltMenu,  ALT|ALT_B,nullstr,    &mainMenu, VER, FIXM, NULL,    37, 1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);
   initMenu(&runMenu,  ALT|ALT_R,nullstr,    &mainMenu, VER, FIXM, NULL,    50, 1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE, LCOM_ATT);
   initMenu(&optMenu,  ALT|ALT_O,nullstr,    &mainMenu, VER, FIXM, NULL,    48, 1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);
   initMenu(&helpMenu, ALT|ALT_H,nullstr,    &mainMenu, VER, FIXM, NULL,    50, 1, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);
   initMenu(&instMenu, ALT|ALT_I,nullstr,    &optMenu,  VER, FIXM, NULL,    34, 3, MENUink, MENUpaper, MENUborder, MENUselected, MENUlcom, TRUE);

   /* ---------------- m_key       TITLE --- X  Y  L  H  BTYPE, OMBRAGE BARRE   ARROW INK       PAPER         WBORDER      BLOCK ATT -- */
   initWin(&edwin,     FUNC|F6,   absfname,  0, 1,78,21, TYP1,  FALSE,  TRUE,   TRUE, WINink,    WINpaper,    WINborder,    WINblock);

   /* placer press Bouton Droit et masque d'appel */
   edwin.M_Dpr  = m_editDpr;
   edwin.M_Gpr  = m_editGpr;
   edwin.M_Gre  = m_editGre;
   edwin.M_CGpr = m_editCGpr;
   edwin.M_CDpr = m_editCDpr;

   initWin(&dirwin,    NORMAL|TAB,cdirname, 10,10,60, 8, TYP1,  FALSE,   FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock);
   dirwin.M_Gre  = m_editGre;
   dirwin.M_CGpr = m_editCGpr;
   dirwin.M_Gpr  = m_editGpr;

   initWin(&dialwin,   FUNC|F3,   nullstr,   8, 4,64,15, TYP1,  TRUE,   FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock);



   initWin(&rdialwin,  0,         nullstr,  20, 8,40, 6, TYP1,  TRUE,  FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock);
   rdialwin.M_Gpr  = m_saisieGpr;

   /* initialiser la zone Champ */
   rdialwin.mField = mfDir;

   /*                   f#  x   y  sz  (* )() */
   initField(&rdialwin,  0, 3,  5, 9,  mv0);
   FIELD(&rdialwin, 1)->size      = 0;    /* marque la fin des champs */

   pedwin = &edwin;
   init_editor();
   hide_cursor();
/*   if (clearScreen) {
        _clrwin(0, 0, 80, 23, 0x07, BLANK);

   }*/
   memset(fillstr, 0x20, nb_car_per_line);

   /* get current working directory */
   getcwd(homedir,79);
   strupr(homedir);
   strcpy(cdirname,homedir);
   nullstr[0] = '\0';

   /* Defaut masque "*.C" */
   strcpy(mask, defaultMask);

   /* paste file par defaut : PASTEF.ED */
   if (strlen(conf.u.pd.c_p) == 0)
       strcpy(conf.u.pd.c_p, "paste.ed");

   strcpy(pastefile, conf.u.pd.c_p);

   /* recuperer les drives disponibles */
   _initDriveTable();

   /* initialiser - NULL le pointeur sur l'objet courant */
   currObj = headObj = tailObj = (struct OBJ *)NULL;


}

/*--------------------------------------------------------------------------
 * initDriveTable - recupere les chemins par defaut sur tous les drives
 *                  disponible;
 *--------------------------------------------------------------------------
 */
_initDriveTable()
{
    int i;
    char aux[65];
    struct DriveTable *pdrive;
    int lastdrive = 26;

    if ((pdt = (struct DriveTable *)malloc(sizeof(struct DriveTable) * (lastdrive+1))) == (struct DriveTable *)NULL)
        return(-1);

    pdrive = pdt;
    for (i = 3; i < lastdrive+1; i++) {
        if (_getcwdir(aux, i))  /* erreur */
                (pdrive+i)->valide = FALSE;
        else {
                (pdrive+i)->valide = TRUE;
                (pdrive+i)->drive  = 'A'+ (i-1);
        }
    }
    (pdrive+1)->valide = TRUE;
    (pdrive+1)->drive  = 'A';
    if (nflp_phy < 2)
        (pdrive+2)->valide = FALSE;
    else {
        (pdrive+2)->valide = TRUE;
        (pdrive+2)->drive  = 'B';
    }
}

LOGO()
{
      _wstringDB((80 - strlen(logoStr))/2, 12, strlen(logoStr), NEWATT, 0x07, logoStr);
}

/*------------------------------------
 * initMenu : initialiser un sous menu
 *------------------------------------
 */
FUNCTION initMenu(ob, m_key, title, UPm, depl, typ, Badd, ul_x, ul_y, ink, paper,
                  border, blockAtt, lcom_att, ombrage, comLetterFlag)
unsigned m_key;
char *title;
struct OBJ *ob,*UPm;       /* UPm : menu pere */
unsigned *Badd;             /* buffer de sauvegarde (pour Menus Variables) */
{
   ob->objet    = MENU;
   ob->UPm      = UPm;
   ob->lcom_att = lcom_att;
   ob->title    = title;

   /* mettre en place les actions souris par defaut */
   ob->M_Gpr        = m_Gpr;
   ob->M_CGpr       = m_CGpr;
   ob->M_Gre        = m_Gre;
   ob->M_Dpr        = m_Dpr;
   ob->M_CDpr       = m_CDpr;
   ob->M_Dre        = m_Dre;
   ob->M_Mov        = m_Mov;

   ob->m_key        = m_key;
   ob->mouse_ev_flag = M_LEFT_PRESS|M_LEFT_RELEASE|M_RIGHT_PRESS|M_RIGHT_RELEASE|M_MOVMENT;
   ob->next = ob->prev = (struct OBJ *)NULL;

   if (depl == VER)  initMenuVER(ob, typ, Badd, ul_x, ul_y, ink, paper,
                                 border, blockAtt, ombrage, comLetterFlag);
   else              initMenuHOR(ob, ul_x, ul_y, ink, paper,
                                 border, blockAtt);
}

/*---------------------------------------------------------------
 * xcoor - donne l'offset X de la rubrique (uniquement pour HOR)
 *---------------------------------------------------------------
 */
xcoor(ob,rub)
struct OBJ *ob;
{
    int i, cumul;
    i = cumul = 0;
    while (i < rub) {
           cumul += (ITEM(ob,i)->ecart + strlen(ITEM(ob,i)->str));
           i++;
    }
    return(cumul + ITEM(ob,rub)->ecart);
}

/*--------------------------------------------------------------
 * initMenuVER : initialiser un sous menu a deplacement VERTICAL
 *--------------------------------------------------------------
 */
initMenuVER(ob, typ, Badd, ul_x, ul_y, ink, paper, border, blockAtt, ombrage, comLetterFlag )
struct OBJ *ob;
unsigned *Badd;
{
   int i ,j , k, att;
   unsigned  *tamp;
   i = j = k = 0;

   /* calcul taille max des strings */
   while ((j = strlen(ITEM(ob,k++)->str)) > 0)
          if (j > i) i = j;

   ob->typ       = typ;
   ob->nline     = k-1;
   ob->curY     = 0;
   ob->blockAtt = blockAtt;
   ob->border   = border;
   ob->ink       = ink;
   ob->paper     = paper;
   ob->mov       = VER;
   ob->pushed   = 0;
   ob->smenu     = 0;
   ob->ul_x      = ul_x;
   ob->ul_y      = ul_y;
   ob->flcom     = comLetterFlag;
   ob->f_ombrage = ombrage;
   ob->ncol     = i;

   /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
/*   if (typ == FIXM) { /* menu de taille fixe : allouer juste la taille */
/*       if ((ob->save = malloc(2 * (ob->ncol+4) * (ob->nline+4))) == NULL)
            exit(0);
   }
   else  /* menu de taille variable : donner le buffer pass- en parametre */
/*       ob->save = Badd;*/

}

/*----------------------------------------------------------------
 * initMenuHOR : initialiser un sous menu a deplacement HORIZONTAL
 *----------------------------------------------------------------
 */
initMenuHOR(ob,ul_x,ul_y,ink,paper,border,blockAtt)
struct OBJ *ob;
{
   int i ,j , k, cumul,att;
   unsigned *tamp;
   i = j = k = 0;

   /* calcul taille de la  string */
   while ((j = strlen(ITEM(ob,k++)->str)) > 0)
           i += (j + ITEM(ob,k-1)->ecart);

   ob->typ       = FIXM;
   ob->nline     = k-2;
   ob->curY     = 0;
   ob->blockAtt = blockAtt;
   ob->mov       = HOR;
   ob->smenu     = 0;
   ob->pushed   = 0;
   ob->border   = border;
   ob->ink       = ink;
   ob->paper     = paper;
   ob->ul_x      = ul_x;
   ob->ul_y      = ul_y;
   ob->f_ombrage = FALSE;
   ob->ncol     = i;

   /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
/*   if ((ob->save = malloc(2 * ob->ncol)) == NULL)
        exit(0);*/

}

/*-----------------------------------------------------
 * CharToInt - copier STRING dans BUFFER de type ECRAN
 *-----------------------------------------------------
 */
CharToInt2(dest,src,att)
int *dest;
char *src;
{
   char c;
   while ((c = *src++) != '\0')
         *dest++ = att<<8|c;
}

/*---------------------------------------------
 * update_menu - mettre a jour un menu variable
 *---------------------------------------------
 */
FUNCTION update_menu(ob)
struct OBJ *ob;
{
   int i ,j , k;
   i = j = k = 0;

   /* calcul taille max des strings */
   while ((j = strlen(ITEM(ob,k++)->str)) > 0)
          if (j > i) i = j;

   ob->nline    = k-1;
   ob->curY     = 0;
   ob->ncol     = i;
   ob->ul_x     = (80 - i)/2;
   ob->ul_y     = (25 - k)/2;
}

writeMitem(ob, no_item)
struct OBJ *ob;
{
              _wstringDB( ob->ul_x + 1,
                          ob->ul_y + 1 + no_item,
                          ob->ncol,
                          NEWATT,
                          ob->ink|ob->paper,
                          ITEM(ob,no_item)->str);
              if ((ob->typ != VARM) || ob->flcom)
              _brighten(ob->ul_x + 1 + ITEM(ob,no_item)->offlcom,
                        ob->ul_y + 1 + no_item,
                        1,
                        ob->lcom_att);
}

/*---------------------------------------------
 * pushSM - faire apparaitre un SOUS MENU
 *---------------------------------------------
 */
FUNCTION pushSM(ob)
struct OBJ *ob;
{
     int offx,offy,i, j, cumul, len;
     unsigned char hide_att;

     if (ob == NULL)
         return;

     if (ob->typ == VARM)
         ob->curY = 0;

     if (!ob->pushed) {
         if ((ob->save = malloc(2 * (ob->ncol+4) * (ob->nline+4))) == NULL) {
              pushPop_MESS(Enomem, ERRMSG);
              return(FUNC|F6);
         }
         saveScreen(ob);
         ob->pushed = 1;
     }

     /* chainer - l'objet precedent
      * et MAJ objet courant
      */
      ob->prev      = tailObj;
      if (tailObj != (struct OBJ *)NULL)
          tailObj->next = ob;
      tailObj       = currObj = ob;


     /* positionner le masque souris pour cet OBJ */
     m_callMaskAndAddress(currObj->mouse_ev_flag, prolog_it_mouse);

     /* ecrire le cadre, puis les rubriques */
     if (ob->mov == VER) {
         write_mbox(ob);

         for (offy = 0; offy < ob->nline; offy++) {
              _wstringDB( ob->ul_x + 1,
                          ob->ul_y + 1 + offy,
                          ob->ncol,
                          NEWATT,
                          ob->ink|ob->paper,
                          ITEM(ob,offy)->str);

              if (((ob->typ != VARM) || ob->flcom) && ITEM(ob,offy)->offlcom != 0)
              _brighten(ob->ul_x + 1 + ITEM(ob,offy)->offlcom,
                        ob->ul_y + 1 + offy,
                        1,
                        ob->lcom_att);
              if ((unsigned char)ITEM(ob,offy)->str[0] == '-') {
                _wstringDB(ob->ul_x,
                           ob->ul_y + offy + 1,
                           1,
                           OLDATT,
                           0,
                           &boxtype[0][6]);
                _wstringDB(ob->ul_x + ob->ncol + 1,
                           ob->ul_y + offy + 1,
                           1,
                           OLDATT,
                           0,
                           &boxtype[0][7]);
              }
         }
     }
     else {
          _brighten(ob->ul_x,
                    ob->ul_y,
                    ob->ncol,
                    ob->ink|ob->paper);

          for (i=0,j=0 , cumul = 0;i<ob->nline; i++) {
               j = ITEM(ob,i)->ecart;
              _wstringDB( ob->ul_x + j + cumul,
                          ob->ul_y,
                          strlen(ITEM(ob,i)->str),
                          NEWATT,
                          ob->ink|ob->paper,
                          ITEM(ob,i)->str);
              _brighten(ob->ul_x + j + cumul + ITEM(ob,i)->offlcom,
                        ob->ul_y,
                        1,
                        ob->lcom_att);
               cumul += (strlen(ITEM(ob,i)->str) + j);
          }
     }
     if (ob->mov == VER)  {
         offx = ob->ul_x + 1;
         offy = ob->ul_y + ob->curY + 1;
         len  = ob->ncol;
     }
     else {
         offx = ob->ul_x + xcoor(ob,ob->curY) - 1; /**/
         offy = ob->ul_y;
         len  = strlen(ITEM(ob,ob->curY)->str) + 2;
     }
     _brighten(offx, offy, len, ob->blockAtt);
}

/*---------------------------------------------
 * popSM - faire disparaitre un SOUS MENU
 *---------------------------------------------
 */
FUNCTION popSM(ob)
struct OBJ *ob;
{
    if (ob == NULL || ob->pushed == 0)  return;
/*    for (offy = ob->nline + 1, i = ob->ul_x+1 ; i <= ob->ncol + 1; i++)
         _brighten(i, ob->ul_y + 1 + offy, 1, ob->lcom_att);*/

    restoreScreen(ob);
    free(ob->save);

    /* MAJ file des objets */
    tailObj = ob->prev;
    if (tailObj != (struct OBJ *)NULL)
        tailObj->next = (struct OBJ *)NULL;


    /* positionner le masque souris pour cet OBJ */
    m_callMaskAndAddress(currObj->mouse_ev_flag, prolog_it_mouse);

    ob->save = NULL;
    ob->pushed = 0;
}

/*--------------------------------------------------------------------------
 * saveScreen - SAUVEGARDER la partie de l'ecran correspondant au SOUS MENU
 *              ou a la WINDOW
 *--------------------------------------------------------------------------
 */
saveScreen(ob)
struct OBJ *ob;
{
    if (ob->objet == MENU) {
        if (ob->mov == VER)
            _save_screen(ob->ul_x,
                     ob->ul_y,
                     ob->ul_x + ob->ncol+3,
                     ob->ul_y + ob->nline+2,
                     ob->save);
        else
            _rstringDW(  ob->ul_x,
                     ob->ul_y,
                     ob->ncol,
                     ob->save);

    }
    else
        _save_screen(ob->ul_x,
                      ob->ul_y,
                      ob->ul_x + ob->ncol + 3,
                      ob->ul_y + ob->nline + 2,
                      ob->save);
   if (ob->f_ombrage)
       write_ombrage(ob->ul_x, ob->ul_y, ob->ncol, ob->nline);

}

/*------------------------------------
 * write_ombrage - dessiner l'ombrage
 *------------------------------------
 */
write_ombrage(x,y,l,h)
{
   int offy, i;
   unsigned char hide_att;

   /* ombrage horizontal */
   _brighten(x + 1, y + h + 2, l + 2, (adapter? 0x08: 0x07));

   /* ombrage vertical */
   for (offy = 0; offy <= h + 1; offy++)
        _brighten(x + 2 + l,
                  y + 1 + offy,
                  2,
                  (adapter? 0x08 : 0x07));

}


/*------------------------------------
 * write_ombrage - dessiner l'ombrage
 *------------------------------------
 */
write_bar(ob)
struct OBJ *ob;
{
   int offy, i;
   unsigned char hide_att;

   /* ombrage vertical */
   for (offy = 0; offy <= ob->nline + 1; offy++)
        _brighten(ob->ul_x + ob->ncol,
                  ob->ul_y + offy,
                  1,
                  ob->ink|0x07);

}

/*-----------------------------------------------
 * write_mbox - dessiner le contour d'un menu
 *-----------------------------------------------
 */
FUNCTION write_mbox(ob)
struct OBJ *ob;
{
  /* type '-' */
  write_box(ob->title, ob->ul_x, ob->ul_y, ob->ncol, ob->nline,
            ob->border, 0, 0, 0);
}

/*-----------------------------------------------
 * write_box - dessiner le contour d'une BOXE
 *-----------------------------------------------
 */
FUNCTION write_box(title, x, y, l, h, att, bar, arrow, typ)
char *title;
unsigned char att;
{
   int ps,i,j,tl;
   unsigned char ulc, llc, urc, lrc, Vchar,
                 Hchar, hBARchar, vBARchar,
                 Ch1, Ch2;

   ulc   = boxtype[typ][0];
   urc   = boxtype[typ][1];
   llc   = boxtype[typ][2];
   lrc   = boxtype[typ][3];
   Vchar = boxtype[typ][4];
   Hchar = boxtype[typ][5];
   Ch1   = boxtype[typ][7];
   Ch2   = boxtype[typ][6];

   if (bar) {
        vBARchar = hBARchar = '-';
   }
   else {
        vBARchar = Vchar;
        hBARchar = Hchar;
   }

   /* COINS */
   _wstringDB(x        , y        , 1, NEWATT, att, &ulc);
   _wstringDB(x + l + 1, y        , 1, NEWATT, att, &urc);
   _wstringDB(x        , y + h + 1, 1, NEWATT, att, &llc);
   _wstringDB(x + l + 1, y + h + 1, 1, NEWATT, att, &lrc);

   /* HORIZONTALES */
   for (i = x+1; i <= x+l; i++) {
        _wstringDB(i, y        , 1, NEWATT, att, &Hchar);
        _wstringDB(i, y + h + 1, 1, NEWATT, att, &Hchar);
   }

   /* DETERMINER SI LE TITRE PEUT RENTRER DANS LA LARGEUR */
   tl = strlen(title);
   if ((tl > 0) && (l - 4 >= tl)) {
        i = x + (l - tl)/2;
        Ch1 = Ch2 = BLANK;
        _wstringDB(i++   , y, 1 , NEWATT, att, &Ch1);
        _wstringDB(i     , y, tl, NEWATT, att, title);
        _wstringDB(i + tl, y, 1 , NEWATT, att, &Ch2);
   }


   /* VERTICALES */
   for (i = y+1; i <= y+h; i++ ) {
        _wstringDB(x        , i, 1, NEWATT, att, &Vchar);
        _wstringDB(x + l + 1, i, 1, NEWATT, att, &Vchar); /*vBARchar);*/
   }

   if (bar) {

        /* ecrire barre H */
        for (i = x+((l-BARRE_SIZE)/2); i <= (x+((l+BARRE_SIZE)/2)); i++)
                _wstringDB(i, y + h + 1, 1, NEWATT, att, &hBARchar);

        /* ecrire barre V */
        for (i = y+((h-BARRE_SIZE)/2); i <= (y+((h+BARRE_SIZE)/2)); i++ )
                _wstringDB(x + l + 1, i, 1, NEWATT, att, &vBARchar);
   }
   if (arrow) {
        _wstringDB(x        , y + 2    , 1, NEWATT, att, &arrLeft);
        _wstringDB(x        , y + h - 1, 1, NEWATT, att, &arrLeft);
        _wstringDB(x + l + 1, y + 2    , 1, NEWATT, att, &arrRight);
        _wstringDB(x + l + 1, y + h - 1, 1, NEWATT, att, &arrRight);
        _wstringDB(x + 2    , y        , 1, NEWATT, att, &arrUp);
        _wstringDB(x + l - 1, y        , 1, NEWATT, att, &arrUp);
        _wstringDB(x + 2    , y + h + 1, 1, NEWATT, att, &arrDwn);
        _wstringDB(x + l - 1, y + h + 1, 1, NEWATT, att, &arrDwn);

   }

}

/*-------------------------------------------------------
 * arrowDWN - changer de rubrique courante en descendant
 *-------------------------------------------------------
 */
FUNCTION arrowDWN(ob)
struct OBJ *ob;
{
     if (ob == NULL || ob->mov == HOR)
         return;
     else
         if (!ob->pushed) {
              pushSM(ob);
              return;
         }

     /* remettre precedente rubrique en attribut normal */
     currItemOFF(ob);
     ob->curY = (ob->curY + 1) % ob->nline;
     if ((unsigned char)ITEM(ob,ob->curY)->str[0] == '-')
        return(arrowDWN(ob));


     /* placer nouvelle rubrique en attribut de selection */
     currItemON(ob);
}


/*---------------------------------------------------
 * arrowUP - changer de rubrique courante en
 *---------------------------------------------------
 */
FUNCTION arrowUP(ob)
struct OBJ *ob;
{
     if (ob == NULL || ob->mov == HOR)
         return;
     else
         if (!ob->pushed) {
              pushSM(ob);
              return;
         }

     currItemOFF(ob);
     if (--ob->curY < 0)
         ob->curY = ob->nline - 1;

     if ((unsigned char)ITEM(ob,ob->curY)->str[0] == '-')
        return(arrowUP(ob));

     currItemON(ob);
}

refreshObj(ob, push_first)
struct OBJ *ob;
{
     int item;
     CurrItemOFF(ob);

     /* mettre a jour l'ITEM courant */
     if (getCurrItem(ob, &item, ib.hCursor / 8, ib.vCursor / 8))
        /* verifier qu'on est pas sur une ligne de separation */
        if ((unsigned char)ITEM(ob,item)->str[0] != '-')
                ob->curY = item;

     if (!(push_first && ITEM(ob,ob->curY)->sm))
         CurrItemON(ob);
}

/*-----------------------------------------------------
 * arrowL - changer de rubrique courante sur la gauche
 *-----------------------------------------------------
 */
FUNCTION arrowL(ob,push_first)
struct OBJ *ob;
{
     if (ob == NULL || !ob->pushed || ob->mov == VER)      return;
     CurrItemOFF(ob);
     if (--ob->curY < 0)
         ob->curY = ob->nline - 1;
     if (!(push_first && ITEM(ob,ob->curY)->sm))
         CurrItemON(ob);
}

/*-----------------------------------------------------
 * arrowR - changer de rubrique courante sur la droite
 *-----------------------------------------------------
 */
FUNCTION arrowR(ob,push_first)
struct OBJ *ob;
{
     if (ob == NULL || !ob->pushed || ob->mov == VER)      return;
     CurrItemOFF(ob);
     if (++ob->curY >= ob->nline)
         ob->curY = 0;
     if (!(push_first && ITEM(ob,ob->curY)->sm))
         CurrItemON(ob);
}

/*-----------------------------------------------------------
 * restoreFromMenu- ecriture PHYSIQUE d'un tampon sur l'ecran
 *-----------------------------------------------------------
 */
restoreFromMenu(ob,buf)
struct OBJ *ob;
int        *buf;
{
   if (ob->mov == VER)
       _refresh(  ob->ul_x,
                  ob->ul_y,
                  ob->ul_x + ob->ncol+3,
                  ob->ul_y + ob->nline+2,
                  buf);
   else
       _wstringDW(ob->ul_x,
                  ob->ul_y,
                  ob->ncol,
                  buf);
}

/*------------------------------------------------------------------------
 * restoreScreen - RESTITUER la partie de l'ecran ecras- par le SOUS MENU
 *                 ou la WINDOW
 *------------------------------------------------------------------------
 */
restoreScreen(ob)
struct OBJ *ob;
{

    if (ob->objet == MENU)
        restoreFromMenu(ob,ob->save);
    else
        _refresh( ob->ul_x,
                  ob->ul_y,
                  ob->ul_x + ob->ncol + 3,
                  ob->ul_y + ob->nline + 2,
                  ob->save);

}

/*---------------------------------------------------------------------------
 *  init_pick - initialiser les pointeurs du pick menu
 *---------------------------------------------------------------------------
 */
init_pick(ffile, confExist)
char *ffile;
{
  int i;

  /* faire pointer le pick menu sur la PICK TABLE */
  for (i = 0; i < 16; i++)
       ITEM(&pickMenu, i)->str = picktab[i].str;

  /* initialiser la 1ere string */
  if (confExist && ((ffile[0] == '/') || conf.autoSave)) /* /k ou /m ou auto Save */
      memmove(&picktab[0], &conf.reload[0], PICKSIZE * 17);
  else {
      strcpy(picktab[0].str, pickload);
      for (i = 1; i < 16; i++)
           strcpy(picktab[i].str, "");
  }
}
doClearPick(ob)
struct OBJ *ob;
{
      int i;

      for (i = 0; i < 16; i++)
        memset(picktab[i].str, '\0', 80);
      strcpy(picktab[0].str, pickload);
      update_menu(&pickMenu);
      return(FUNC|F6);
}

/*---------------------------------------------------------------------------
 *  init_comp - initialiser les pointeurs du comp menu
 *---------------------------------------------------------------------------
 */
init_comp()
{
  /* faire pointer la zone variable du comp menu sur la COMP TABLE */
/*  ITEM(&compMenu, 0)->str = comptab[0];
  ITEM(&compMenu, 1)->str = comptab[1];
  ITEM(&compMenu, 2)->str = comptab[2];
  strcpy(comptab[0], "");
  strcpy(comptab[1], "");
  strcpy(comptab[2], " Show messages ");*/

  /* init type de compilateur */
/*  CompilerTradeMark[0] = CTM[0];
  CompilerTradeMark[1] = CTM[1];

  ITEM(&instMenu, 0)->str = CompilerTradeMark[toggle = conf.c_t];*/
  conf.c_t = 1; /* BORLAND */
}

/*---------------------------------------------------------------------------
 *  push_pick - pusher un fichier dans la PICK STACK
 *---------------------------------------------------------------------------
 */
push_pick(fname)
char *fname;
{
   int i;
   struct OBJ *ob;

   if (fflag & FF_NO_GROUP) {
        fflag &= ~FF_NO_GROUP;
        return;
   }

   for (i = INF((ob = &pickMenu)->nline - 1, 16); i >= 0; i--) {
/*        memset(picktab[i+1], '\0', 80);*/
        memmove(&picktab[i+1], &picktab[i], PICKSIZE);
   }
   /* placer les parametres du fichier dans la PICK TABLE */
   memset(picktab[0].str, '\0', 80);
   strcpy(picktab[0].str, " ");
   strcat(picktab[0].str, fname);
   strcat(picktab[0].str, " ");
   picktab[0].current         = current;
   picktab[0].topPage         = topPage;
   picktab[0].topBlock        = topBlock;
   picktab[0].bottomBlock     = bottomBlock;
   picktab[0].fsize           = fsize;
   picktab[0].current_line_no = current_line_no;
   picktab[0].topPage_line_no = topPage_line_no;
   picktab[0].leftCh          = pedwin->leftCh;
   picktab[0].curX            = pedwin->curX;
   picktab[0].curY            = pedwin->curY;
   picktab[0].fflag           = fflag;

   update_menu(ob);
   if (ob->nline == 16) {
       memset(picktab[15].str, '\0', 80);
       strcpy(picktab[15].str, pickload);
   }
}

/*---------------------------------------------------------------------------
 *  pop_pick - poper un fichier de la PICK STACK (le nom du fichier a ete
 *             prealablement copi- dans "file_name")
 *---------------------------------------------------------------------------
 */
struct pick pop_pick(item_no)
{
   struct OBJ *ob;
   static struct pick  pck;
   int i;


   /* copier l'element - retirer */
   memmove(&pck, &picktab[item_no], PICKSIZE);

   /* decaler tous les fichiers "sous" le fichier - retirer */
   for (ob = &pickMenu, i = item_no; i < ob->nline - 1; i++)
        memmove(&picktab[i], &picktab[i+1], PICKSIZE);


   memset(picktab[i].str, '\0', 80);
   update_menu(ob);
   return(pck);
}

/*---------------------------------------------------------------------------
 *  check_pick - teste si le fichier fname est deja dans la PICK TABLE
 *               si oui, retourne sa position, sinon -1
 *---------------------------------------------------------------------------
 */
check_pick(fname)
char *fname;
{
   extern char work[];
   struct OBJ *ob;
   int i;

   /* comparer le nom de fichier a ceux de la PICK TABLE */
   for (ob = &pickMenu, i = 0; i < ob->nline - 1; i++) {
        strcpy(work, &picktab[i].str[1]);
        work[strlen(&picktab[i].str[1]) - 1] = '\0';
        if (stricmp(work, fname) == 0)
            return(i);
   }
   return(-1);
}

inhibe_screen()
{
   void interrupt Dummy_interrupt();

   _dos_setvect(0x10, Dummy_interrupt);
   /* se placer dans la directory courante
    * ( utilis- pour creer les objets dans la m-me directory
    * que les sources
    */
/*   chdir(cdirname);*/

   /* liberer bigbuf pour faire de la place */
   free(bigbuf);
}

enable_screen()
{
   extern char errmalloc[];
   struct OBJ errwin;

   _dos_setvect(0x10, BiosInt10);
/*   chdir(homedir);*/
   if ((bigbuf = malloc(BIGBUF_SIZE)) == NULL) {
        pushPop_MESS(errmalloc, ERRMSG);
        exit(7); /* retour au dos */
   }
}

void interrupt Dummy_interrupt()
{
}
