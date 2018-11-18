#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>


/*---------------------------------------*
 * OPTIONS POSSIBLES au Rechargement :   *
 *      p(1)    /k : RELOAD from KERNEL  *
 *      p(1)    /m : RELOAD from MAKE    *
 *      p(1)    /l : RELOAD from LINK    *
 *      p(1)    /c : RELOAD from COMP    *
 *      p(1)    /s : RELOAD from SHELL   *
 *      p(1)    /b : RELOAD from BROWSER *
 *---------------------------------------*
 */
main(argc,argv)
char *argv[];
{
   int     option, prechargement, clearScreen, ret, i, c, initKey;
   char   *pt, op;
   char aux[80];

   /* recuperer l'@ du vecteur de communication */
   vl = (struct varLink *)_dos_getvect(0xf2);

   clearScreen   = TRUE;
   option        = FALSE;
   prechargement = FALSE;
   if (argc > 1) {
        pt = argv[1];

        /* si on revient du make ou de
         * compilation, ne pas effacer l'ecran
         */
        if (pt[0] == '/') {
                option        = TRUE;
                prechargement = TRUE;
                strcpy(defaultMask, vl->currmask);


                /* retour Make / comp / Ass  - ou Err kernel */
                clearScreen = FALSE;
                switch (op = pt[1]) {
                case 'k' : if (vl->spawnRet >= 0)
                                clearScreen = TRUE;
                case 's' : clearScreen = TRUE;break;
                case 'l' : strcpy(vl->currExe, vl->exename);
                case 'b' : break;
                case 'c' : memset(vl->currExe, 0, 80);
                           break;
                case 'm' : strcpy(exename, vl->conf.projExe);
                           break;
                }
                /* se placer sur la dir courante */ /*home directory */
                strcpy(aux, vl->cdirname); /*homedir);*/
                _performChgDir(aux);


        }
        else {
                strcpy(exename, conf.projExe);
                strcpy(defaultMask, Cmask);
        }
   }
   else {
        strcpy(defaultMask, Cmask);
        pt = NULL;
        memset(exename, 0, 80);
   }


   /* initialiser les menus et les fenetres - faire apparaitre
    * la fenetre de copyright
    */
   global_init(pt, clearScreen);

   if (!option)
        print_cp0412(FALSE);   /* fenetre du Copyright ON */

   /* gestion de l'auto Sauvegarde */
   if (conf.autoSave && (pt == NULL)) {
           /* si aucun fichier selectionn‚ et auto save
            * charger le 1er fichier du pick, s'il existe
            */
           strcpy(file_name, &picktab[0].str[1]);
           file_name[strlen(&picktab[0].str[1]) - 1] = '\0';
           if (strcmp(file_name, pickload) != 0) { /* historique present */
                prechargement = TRUE;
                pt = pickload;  /* leurer en pt != NULL */
           }
           file_name[0] = '\0';
   }
   /* gestion vitesse clavier */
   doKbSpeed(conf.kbdFast);

   if (clearScreen)
        _clrwin(0, 0, 80, 23, 0x07, BLANK);

   clear_status_line();
   pushSM(&mainMenu);

   /* tester si on doit charger le fichier d'erreur */
   if (vl->retCode == (MKE|BERR)) {
/*      option = FALSE;
        pt = vl->fError;*/

        /* construire le NOM ABSOLU de fError */
        strcpy(findSTR, vl->ferror_name); /* recuperer le nom de fichier
                                           * en erreur (gestion du FIND)
                                           */
        absfname[0] = '\0';   /* Loadfile : gestion du pick */

        strcpy(file_name, vl->fError);
        strcpy(aux, conf.u.c_p[5]);
        if (get_deepth(aux) > 0)
                strcat(aux, "\\");
        strcat(aux, file_name);
        strcpy(file_name, aux);

        fflag |= FF_NO_GROUP;
        loadFile(0, file_name, absfname, NOMSG, V_CREAT);
        doFind(0, FIND);
        line_home(pedwin);

        /* replacer l'environnement de find/replace */
        strcpy(findSTR, vl->findStr);
        strcpy(replaceSTR, vl->replStr);

        /* entrer dans la danse !*/
        MasterMenu(&mainMenu, 0);
   }
   /* pr‚initialiser les Options ASS / LINK / COMP */
   vl->mkd_shell = conf.mkd_shell;
   vl->mkd_FS    = conf.mkd_FS;
   vl->mkd_tasm  = conf.mkd_tasm;
   vl->mkd_masm  = conf.mkd_masm;


   memset(relname, '\0', 80);
   initKey = 0;
   if (pt == NULL)  {

Unnamed:
       strcpy(cdirname, conf.u.c_p[5]); /* init sur dir USR */
       strcpy(absfname, cdirname);
       if (get_deepth(absfname) > 0)
           strcat(absfname, "\\");
       strcpy(relname,"unnamed.c");
       strcat(absfname, relname);
       strcpy(file_name, absfname);

       /* placer absfname a NULL pour gestion du pick */
       absfname[0] = '\0'; /**/
   }
   else {
       /* gestion rechargement apres execution */
       if (prechargement) {
           if (strcmp(picktab[0].str, pickload) == 0)
                goto Unnamed;
           strcpy(file_name, &picktab[0].str[1]);
           file_name[strlen(&picktab[0].str[1]) - 1] = '\0';
           ret = 0;
       }
       else
           strcpy(file_name, pt);
       if (((ret = set_path()) == TOK_DIR) || (ret == TOK_WILD)) {
                if (prechargement)
                        pop_pick(0);
                /*strcpy(cdirname, homedir);*/
                goto Unnamed;
       }

       /* placer absfname a NULL pour gestion du pick */
       absfname[0] = '\0';
   }
   ret = loadFile(0, file_name, absfname, (option ? NOMSG : WINMSG), V_CREAT);
   /*strcpy(cdirname, homedir);*/
   if (ret < 0)
       goto Unnamed;                         /**/

   strcpy(findSTR, vl->findStr);
   strcpy(replaceSTR, vl->replStr);
   MasterMenu(&mainMenu, (option ? op : 0));

}

retrieve_Opt()
{
      mkd_shell   = vl->mkd_shell;
      mkd_FS      = vl->mkd_FS;
      mkd_AutoAsk = vl->mkd_AutoAsk;
      mkd_tasm    = vl->mkd_tasm;
      mkd_masm    = vl->mkd_masm;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * getItem - recuperer la rubrique correspondant a l'extra caractere pour
 *           la profondeur deepth
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
getItem(extraCh, deepth, type)
unsigned extraCh;
int      deepth;
int      *type;
{
  int i;
  for (i = 0; Ctab[i].extra_com != NOKEY; i++)
       if (Ctab[i].extra_com == extraCh) {
/*           sprintf(work, " big = %lX Cur = %X TYPE = %lX ", bigbuf, current, type);
           pushPop_MESS(work, ERRMSG);*/
           *type = (int)Ctab[i].type;
           return(Ctab[i].path[deepth]);
       }
  return(NP);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lcomToItem - convertir une lettre de commande en numero de rubrique
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
lcomToItem(mp,Ch)
struct OBJ *mp;
char Ch;
{
  int i,j;
  char c;
  j = mp->curY;
  for (i = 0; i < mp->nline; i++)
       if (((c = ITEM(mp,i)->lcom) == Ch) || (c + 0x20 == Ch))
           return(i);
  return(j);
}



/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * MasterMenu - menu principal
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
MasterMenu(mp, initKey)
struct OBJ *mp;
char initKey;      /* utilis‚ pour les fichiers lanc‚s au chargement */
{
      int i, go, item, extra, type, push_first;
      int ChL, ChX, Key;
      char wkfile[80], *pt;
      struct OBJ *ss;

      go         = TRUE;
      push_first = FALSE;
      extra      = FALSE;
      type       = NOTERMINAL;
      hide_cursor();
      currItemOFF(&mainMenu);
/*****/
/*****/
      /* ouvrir l'objet editeur en pseudo OBJ FREE -
       * c'est l'editeur qui se chainera lui meme
       * a la liste des OBJETS
       */
      winPopUp(pedwin, W_OPEN, OBJ_CHAINAGE, TYP1); /*OBJ_FREE);*/
      set_win(pedwin);
/****/
      switch (initKey) {
      case 'l' :
      case 'c' :
      case 'm' :        /* gestion Make */
                        Key = (FUNC|F6);
                        if (vl->spawnRet < 0)  /* retour Make en Erreur */
                                pushPop_MESS(errToolsStr, ERRMSG);
                        if (vl->recomp || vl->relink)
                                doRunExe(mp);
                        if (vl->rebuild)
                                doRunPrj(mp);
                        break;


      case 'k' :        /* retour Kernel */
                        Key = (FUNC|F6);
                        if (vl->spawnRet < 0) { /* retour Kernel en erreur */
                                strcpy(work, errKernelStr);
                                strcpy(wkfile,vl->exename);
                                strcat(work, wkfile);
                                strcat(work, " ");
                                pushPop_MESS(work, ERRMSG);
                        }
                        break;

      case 'b' :        /* retour Creation DATABASE */
                        if (vl->spawnRet < 0) { /* retour browser en erreur */
                                strcpy(work, " Cannot run browser ");
                                pushPop_MESS(work, ERRMSG);
                        }
                        conf.buildDB = FALSE;
                        Key = (* vl->Bfunc)();
                        break;
      case 's' :        /* retour Shell */
                        Key = (FUNC|F6);break;
      default  :        /* rentrer dans l'editeur */
                        memset(conf.projExe, 0, 80);
                        Key = (FUNC|F6);break;
      }
/*      retrieve_Opt(); /* recuperer les options MASM / LINK / COMP */

/****/
      goto Continue;
      while (go) {
             Key = getkey();
Continue:    ChL = (Key & LBYTE);
             ChX = (Key & HBYTE);
             menu_deepth = 0;
             switch (ChX) {
             case NUMPAD:
                          switch( ChL) {
                          case ARROWD : push_first = TRUE;break;
                          case ARROWL : arrowL(mp,push_first);break;
                          case ARROWR : arrowR(mp,push_first);break;
                          case MOUSE_LP:
                          case MOUSE_L :
                          case MOUSE_LR:
                                        push_first = TRUE;
                                        refreshObj(mp, push_first);break;
                          default     : continue;
                          }
                          if (push_first) {
                              if (ITEM(mp,mp->curY)->sm) {
                                  /* un sous menu existe */
                                  pushSM((ss = ITEM(mp,mp->curY)->SM));
                                  blockON(mp);
                                  Key = (* ITEM(mp,mp->curY)->SMaction)(ss,FALSE);
                                  popSM(ss);
                                  blockOFF(mp,TRUE);
                                  if ((Key & HBYTE) != NUMPAD)
                                       push_first = FALSE;
                                  goto Continue;
                              }
                              /* fonction terminale : ne rien faire */
                          }
                          break;
             case NORMAL:
                         switch (ChL) {
                         case RETURN:
                              if (ITEM(mp,mp->curY)->sm) {
                                  /* un sous menu existe :
                                   * pusher le menu si (mode NON EXTRA) ou
                                   * (mode EXTRA et type NON TERMINAL)
                                   */
                                  ss = ITEM(mp,mp->curY)->SM;
                                  if ((extra && (type == NOTERMINAL)) ||
                                       !(extra)) {
                                       push_first = TRUE;
                                       pushSM(ss);
                                       blockON(mp);
                                  }
                                  Key = (* ITEM(mp,mp->curY)->SMaction)(ss, extra, 1, type);
                                  popSM(ss);
                                  blockOFF(mp,TRUE);
                                  extra = FALSE;
                                  type  = NOTERMINAL;
                                  if ((Key & HBYTE) != NUMPAD)
                                       push_first = FALSE;
                                  goto Continue;
                              }
                              /* fonction terminale : la lancer */
                              CurrItemOFF(mp);
                              Key = (* ITEM(mp,mp->curY)->SMaction)(ss);
                              CurrItemON(mp);
                              extra = FALSE;
                              type  = NOTERMINAL;
                              goto Continue;

                         case ESC : Key = (FUNC|F6); /* relancer l'editeur */
                                    goto Continue;
                         default:
                                  /* cas des lettres de commandes */
                                  item = lcomToItem(mp,ChL);
                                  CurrItemOFF(mp);
                                  mp->curY = item;
                                  CurrItemON(mp);
                                  Key = (NORMAL|RETURN);
                                  goto Continue;
                         }
                         break;
             case FUNC:
             case ALTFUNC:
             case SHIFTFUNC:
             case CTRLFUNC:
             case ALT:
                          /* determiner le chemin de la
                           * rubrique a declencher
                           */
                          if ((item = getItem(ChL,0,&type)) != NP) {
                               CurrItemOFF(mp);
                               mp->curY = item;
                               CurrItemON(mp);
                               extra = ChL;

                               /* forcer le declenchement du sous menu */
                               Key = (NORMAL|RETURN);
                               goto Continue;
                          }
                          break;
             }
      }
}



/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * ScanVmenu - lire un menu VERTICAL
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
ScanVmenu(mp, extra, deepth, type)
struct OBJ *mp;
{
      int i, go;
      int ChL, ChX, Key, item, typeBIS;
      struct OBJ *ss;

      /* si mode extra, pusher directement le sous menu
       * de la rubrique donn‚e par le path.
       * (si l'action finale est terminale, ne pas pusher
       * les sous menus intermediaires)
       */
      typeBIS = type;
      menu_deepth++;
      if (extra)  {
          /* si on est arriv‚ au sommet du path, continuer normalement */
          if ((item = getItem(extra, deepth, &typeBIS)) != NP) {
               /* si type NON TERMINAL, positionner la rubrique courante */
               if (typeBIS == NOTERMINAL) {
                   if (item != mp->curY) {
                       CurrItemOFF(mp);
                       mp->curY = item;
                       CurrItemON(mp);
                   }
               }
               else {
                   if (item != mp->curY)
                       mp->curY = item;
               }
               Key = (NORMAL|RETURN);
               goto Continue;
          }
          extra   = FALSE;
          typeBIS = NOTERMINAL;
      }

      while (1) {
             Key = getkey();
Continue:    ChL = (Key & LBYTE);
             ChX = (Key & HBYTE);
             switch (ChX) {
             case NUMPAD:
                          switch(ChL) {
                          case ARROWU : arrowUP(mp);break;
                          case ARROWD : arrowDWN(mp);break;
                          case MOUSE_DRAG: return(NUMPAD);
                          case MOUSE_LR:
                          case MOUSE_L :
                          case MOUSE_LP:
                                        refreshObj(mp, 0);break;
                          default     : if (menu_deepth == 1)
                                            return(Key);
                          }
                          break;
             case NORMAL:
                          switch(ChL) {
                          case RETURN :
                               if (ITEM(mp,mp->curY)->sm) {
                                   /* un sous menu existe */
                                   ss = ITEM(mp,mp->curY)->SM;
                                   if ((extra && (typeBIS == NOTERMINAL)) ||
                                       !(extra))
                                       pushSM(ss);
                                   Key = (* ITEM(mp,mp->curY)->SMaction)(ss,extra,deepth+1, typeBIS);
                                   popSM(ss);
                               }
                               else {
                                   /* fonction terminale :
                                    * retirer les sous menus
                                    */
                                   ss = mp;
                                   while (ss->UPm != NULL) {
                                          CurrItemOFF(ss->UPm);
                                          popSM(ss);
                                          ss = ss->UPm;
                                   }
                                   blockOFF(ss,FALSE);
                                   /* lancer f. terminale */
                                   Key = (* ITEM(mp,mp->curY)->SMaction)(mp, mp->curY);/*ss);*/
                                   /*CurrItemON(ss);*/
                               }
                               if ((Key & LBYTE) == ESC) break;
                               else                     goto Continue;

                          case ESC    : menu_deepth--;
                                        return(Key);

                          default :  /* cas des lettres de commandes */
                                     item = lcomToItem(mp,ChL);
                                     CurrItemOFF(mp);
                                     mp->curY = item;
                                     CurrItemON(mp);
                                     Key = (NORMAL|RETURN);
                                     goto Continue;
                          }
                          break;
             case FUNC:
             case ALTFUNC:
             case SHIFTFUNC:
             case CTRLFUNC:
             case ALT   :
                          if (mp->pushed)
                              popSM(mp);
                          menu_deepth--;
                          return(Key);

             }
      }
}



/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * CurrItemON
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
CurrItemON(mp)
struct OBJ *mp;
{
   if (mp->mov == HOR)
       _brighten(mp->ul_x + xcoor(mp,mp->curY) - 1,
                 mp->ul_y,
                 strlen(ITEM(mp,mp->curY)->str) + 2,
                 mp->blockAtt);
   else
       _brighten(mp->ul_x + 1,
                 mp->ul_y + mp->curY + 1,
/*               strlen(ITEM(mp,mp->curY)->str),*/
                 mp->ncol,
                 mp->blockAtt);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * blockON - encadrer la rubrique selectionnee (MASTER MENU seulement)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
blockON(mp)
struct OBJ *mp;
{
   char up, dwnG, dwnD;
   unsigned char attSM;
   int SMxg, SMxd;

   int first = mp->ul_x + xcoor(mp,mp->curY) - 1;
   SMxg = ITEM(mp,mp->curY)->SM->ul_x;
   SMxd = ITEM(mp,mp->curY)->SM->ul_x + ITEM(mp,mp->curY)->SM->ncol;
   up  = '³';
   if (SMxg == first) {
        dwnG = 'Ã';
        dwnD = 'Á';
   }
   else
        if (SMxd == first + strlen(ITEM(mp,mp->curY)->str)) {
                dwnG = 'Á';
                dwnD = '´';
        }
        else {
                dwnG = 'Á';
                dwnD = 'Á';
        }

   attSM = ITEM(mp,mp->curY)->SM->border;

   CurrItemOFF(mp);
   /* entourer la rubrique selectionnee */
   _wstringDB(first,
              mp->ul_y,
              1,
              NEWATT,
              attSM,
              &up);
   _wstringDB(first + strlen(ITEM(mp,mp->curY)->str) + 1,
              mp->ul_y,
              1,
              NEWATT, /*OLDATT,*/
              attSM, /*mp->ink|mp->paper,*/
              &up);

   /* ajuster contour du sous menu */
   _wstringDB(first,
              mp->ul_y + 1,
              1,
              OLDATT,
              attSM, /*mp->ink|mp->paper,*/
              &dwnG);
   _wstringDB(first + strlen(ITEM(mp,mp->curY)->str) + 1,
              mp->ul_y + 1,
              1,
              OLDATT,
              attSM, /*mp->ink|mp->paper,*/
              &dwnD);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * blockOFF - d‚sencadrer la rubrique d‚selectionn‚e (MASTER MENU seulement)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
blockOFF(mp,affCurrItem)
struct OBJ *mp;
{
   char up;
   int first = mp->ul_x + xcoor(mp,mp->curY) - 1;
   up = ' ';

   /* d‚sentourer la rubrique selectionnee */
   _wstringDB(first,
              mp->ul_y,
              1,
              NEWATT, /*OLDATT,*/
              mp->ink|mp->paper,
              &up);
   _wstringDB(first + strlen(ITEM(mp,mp->curY)->str) + 1,
              mp->ul_y,
              1,
              NEWATT, /*OLDATT,*/
              mp->ink|mp->paper,
              &up);

   if (affCurrItem)
       CurrItemON(mp);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * CurrItemOFF
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
CurrItemOFF(mp)
struct OBJ *mp;
{
   if (mp->mov == HOR) {
       _brighten(mp->ul_x + xcoor(mp,mp->curY) - 1,
                 mp->ul_y,
                 strlen(ITEM(mp,mp->curY)->str) + 2,
                 mp->ink|mp->paper);

       _brighten(mp->ul_x + xcoor(mp,mp->curY) + ITEM(mp,mp->curY)->offlcom,
                 mp->ul_y,
                 1,
                 mp->lcom_att);
   }
   else {
       _brighten(mp->ul_x + 1,
                 mp->ul_y + mp->curY + 1,
/*               strlen(ITEM(mp,mp->curY)->str),*/
                 mp->ncol,
                 mp->ink|mp->paper);

       /* replacer attribut lettre de commande */
       if (((mp->typ != VARM) || mp->flcom) && ITEM(mp,mp->curY)->offlcom != 0)
            _brighten(  mp->ul_x + 1 + ITEM(mp,mp->curY)->offlcom,
                        mp->ul_y + mp->curY + 1,
                        1,
                        mp->lcom_att);
   }

}

print_cp0412(author)
{
        struct OBJ cpr, *p;
        int width, hight, size;
        char decrypStr[80];



        if (author) {
                width = strlen(cp0412);
                hight = 3;
        }
        else {
/*                width = size = strlen(productName) + 5; /* a cause du cara cod‚ en 0 */
                width = size = strlen(_lg56); /* a cause du cara cod‚ en 0 */
                hight = 3; /* 5; */
        }
        initWin(p = &cpr,0,"", (80-width)/2,(24-hight)/2,width, hight, TYP1,  FALSE,   FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock);
        winPopUp(p, W_OPEN, OBJ_CHAINAGE, TYP1);
        if (!author) {
/*                decrypte(Corp,        decrypStr, size);
                print_at(p, p->curX, p->curY+1, decrypStr, OLDATT);
                decrypte(productName, decrypStr, size);
                print_at(p, p->curX, p->curY+3, decrypStr, OLDATT);*/
                decrypte(_lg56,      decrypStr, strlen(_lg56));
                print_at(p, p->curX, p->curY+1, decrypStr, OLDATT);

        }
        else {
                decrypte(cp0412,      decrypStr, strlen(cp0412));
                print_at(p, p->curX, p->curY+1, decrypStr, OLDATT);
        }
        delay(5);
        winPopUp(p, W_CLOSE);
}

decrypte(str, strTarget, size)
char *str, *strTarget;
{
        int i;
        for (i=0; i < size; i++)
                strTarget[i] = str[i] ^ i;
        strTarget[i] = 0;
}

delay(n) /* n doit etre < 30 */
{
        extern unsigned char ticks;
        unsigned key;

        ticks = 0;
        while (ticks < 18 * n) {
                if (look_kbd()) { /* caractere tap‚ au clavier */
                        read_kbd();
                        break;
                }
                if (mouseEvent)
                        break;

        }
}