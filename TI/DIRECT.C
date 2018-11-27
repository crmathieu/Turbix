/* direct.c : gestion affichage directory courante et saisie fichier */

#include "xed.h"
#include "ext_var.h"

/* flag d'erreur d'E/S */
static int ioError = FALSE;
static int int24error = 0;
static int display = 0;

#define ID_TYPE_DR      0x7f
#define DR_LETTER       2

/*-------------------------------------------------------------
 * tri_lst - insere en triant dans la liste FILE_LST
 *-------------------------------------------------------------
 */
tri_lst(fl)
struct file_lst *fl;
{
        struct file_lst *wf;

        for (wf = &head; wf != &tail; wf = wf->next) {
             if (strcmp(fl->file.str, wf->file.str) > 0)
                 continue;
             else
                 break;
        }
        wf->prev->next = fl;
        fl->prev = wf->prev;
        wf->prev = fl;
        fl->next = wf;
}
/*-------------------------------------------------------------
 * add_drive - ajoute la liste des drives
 *-------------------------------------------------------------
 */
add_drive()
{
    struct file_lst *wf;
    int i, n;
    struct DriveTable *pdrive;
    int lastdrive = 26;

    pdrive = pdt;
    for (i = 1, n = 0; i < lastdrive+1; i++) {
        if ((pdrive+i)->valide) {
                if ((wf = malloc(FILE_SIZE)) == NULL)
                        return(-1);
                strcpy(wf->file.str, "[   ]");
                wf->file.str[DR_LETTER] = (pdrive+i)->drive;
                wf->file.att    = ID_TYPE_DR; /* ID type drive */
                tail.prev->next = wf;
                wf->prev = tail.prev;
                tail.prev = wf;
                wf->next = &tail;
                n++;
        }
    }
    return(n);
}

/*-------------------------------------------------------------
 * free_lst - liberer la liste des fichiers
 *-------------------------------------------------------------
 */
free_lst()
{
    struct file_lst *wf, *aux;
    for (wf = head.next; wf != &tail; wf = aux) {
         aux = wf->next;
         free(wf);
    }
}

/*-------------------------------------------------------------
 * get_dir_ls : recuperer le contenu de la directory suivant le
 *              masque par defaut
 *-------------------------------------------------------------
 */
#define NOMORE_FILE  0x12
get_dir_ls()
{
 int    i, dr, ret, insert, first_passage;
 union  REGS  rin,rout;
 struct SREGS segreg;
 struct DTA   dta;
 struct DTA  *currdta, *pdta;
 struct dir_lst *dl;
 struct file_lst *fl;
 char  *dir = work;

      strcpy(dir, cdirname);
      if (get_deepth(dir) > 0)
          strcat(dir, "\\");
      strcat(dir, "*.*");
      vflopp(dir);             /* gestion floppy logique */

      dl = &dir_lst;
      dl->next = dl->prev = NULL;
      i  = 0;
      first_passage = TRUE;
      dir_lst_nfiles = 0;
      dir_lst_npages = 1;
      pdta = &dta;

      /* recuperer la DTA courante */
      rin.h.ah  = 0x2f ;
      int86x(0x21, &rin, &rout,&segreg);
      (unsigned long)currdta = segreg.es;
      (unsigned long)currdta = (unsigned long)currdta << 16;
      (unsigned long)currdta = (unsigned long)currdta | rout.x.bx;

      /* initialiser la DTA */
      rin.h.ah  = 0x1a ;
      rin.x.dx  = FP_OFF(pdta) ;
      segreg.ds = FP_SEG(pdta);
      int86x(0x21, &rin, &rout,&segreg);

      /* init head et tail */
      memset(head.file.str, '\0', 13);
      memset(tail.file.str, 'z', 12);
      tail.file.str[12] = '\0';
      head.next = &tail;
      tail.prev = &head;
      head.prev = tail.next = NULL;

Second:

      /* amorcer la recherche */
      rin.h.ah  = 0x4e;
      rin.x.cx  = 0x30;
      rin.x.dx  = FP_OFF(dir);
      segreg.ds = FP_SEG(dir);
      int86x(0x21, &rin , &rout,&segreg) ;
      if (rout.x.cflag) { /* erreur */
/*          if (rout.x.ax != NOMORE_FILE) { /* path inv */
/*              free_lst();
                     if (dosexterr(NULL) != INT24FAIL)
                         return(ERRNO);

                     return(ERROR_DSKERROR);
          }*/
/***/
          if (dosexterr(NULL) == INT24FAIL) {
                restore_dta(currdta);
                return(ERROR_DSKERROR);
          }
          if (rout.x.ax != NOMORE_FILE) { /* path inv */
                restore_dta(currdta);
                return(ERRNO);
          }

/***/
      }
      else {
          /* poursuivre pour toute la directory */
          while (!rout.x.cflag) {
                 if ((fl = malloc(FILE_SIZE)) == NULL) {
                      restore_dta(currdta);
                      return(ERRNO);
                 }
                 memset((char *)fl,'\0',FILE_SIZE);
                 insert = TRUE;

                 fl->file.att = dta.att;
                 if (dta.att & 0x10) { /* directorie */
                     if (first_passage) {
                         /* sauter la directory ".\" */
                         if (strcmp(dta.fname,".") != 0) {
                             strcpy(fl->file.str, dta.fname);
                             dir_lst_nfiles++;
                         }
                         else {
                             free(fl);
                             insert = FALSE;
                         }
                     }
                     else {
                         free(fl);
                         insert = FALSE;
                     }
                 }
                 else { /* fichier */
                     if (!first_passage) {
                             strcpy(fl->file.str,dta.fname);
                             strlwr(fl->file.str);
                             dir_lst_nfiles++;
                      }
                      else {
                             free(fl);
                             insert = FALSE;
                      }
                 }
                 if (insert)
                     tri_lst(fl);  /* inserer dans la liste chainee en triant */
                 rin.h.ah = 0x4f;
                 int86x(0x21, &rin, &rout, &segreg);
          }
          if (first_passage) {
              first_passage = FALSE;
              strcpy(dir, cdirname);
              if (get_deepth(dir) > 0)
                  strcat(dir, "\\");
              strcat(dir, mask);
              goto Second;
          }
      }

      /* add valide drive to the list */
      if ((dr = add_drive()) < 0) {
        restore_dta(currdta);
        return(ERRNO);
      }

      /* recopier les fichiers dans les pages */
      dir_lst_nfiles += dr;
      for ( i = 0, fl = head.next; fl != &tail; fl = fl->next) {
            if (i >= NB_FILE_PER_PAGE) {
                /* allouer une nouvelle page */
                if ((dl->next = malloc(DIR_LST)) == NULL) {
                     restore_dta(currdta);
                     return(ERRNO);
                }
                dir_lst_npages++;
                memset((char *)dl->next,'\0',DIR_LST);
                dl->nfpp       = NB_FILE_PER_PAGE;
                dl->next->prev = dl;
                dl             = dl->next;
                dl->next       = NULL;
                i              = 0;
             }
             strcpy(dl->dirPage[i].str, fl->file.str);
             dl->dirPage[i].att = fl->file.att;
             i++;
      }

      if ((dir_lst_nfiles / NB_FILE_PER_PAGE) > 0) {
          if ((dl->nfpp = dir_lst_nfiles % NB_FILE_PER_PAGE) == 0)
               dl->nfpp = NB_FILE_PER_PAGE;
      }
      else
          dl->nfpp = dir_lst_nfiles;

      restore_dta(currdta);
      return(dir_lst_nfiles);
}

restore_dta(currdta)
struct DTA *currdta;
{
       union  REGS  rin,rout;
       struct SREGS segreg;

      /* remettre la DTA par defaut */
      rin.h.ah  = 0x1a ;
      rin.x.dx  = FP_OFF(currdta) ;
      segreg.ds = FP_SEG(currdta);
      int86x(0x21, &rin, &rout,&segreg);
      free_lst();
}
/*-----------------------------------------------------------------------------
 * exit_dir - gestion sortie de display_dir
 *-----------------------------------------------------------------------------
 */
exit_dir(sed, ws)
struct save_edit *sed;
struct OBJ *ws;
{
    free_dp();
    display = FALSE;
    winPopUp(&dirwin, W_CLOSE);
    winPopUp(ws, W_CLOSE);
    winPopUp(&dialwin, W_CLOSE);
    restoreEdit(sed);
    hide_cursor();
}

/*-----------------------------------------------------------------------------
 * display_dir - place dans file_name le nom du fichier selectionn-
 *-----------------------------------------------------------------------------
 */
unsigned display_dir(Otype, loadtype)
int *Otype;   /* type d'ouverture (OPEN ou CREAT) */
{
#define ON     on
#define OFF(x) (dl->dirPage[(x)].att & 0x10 ? WINDIRink|dirwin.paper : off)

   struct dir_lst *dl, *dlcurr;
   struct OBJ *wp, *ws, *wd, wsaisie;
   struct save_edit sed;
   unsigned Key, ChL, ChX;
   char work[80], *pt;
   int  ret, deltaF, deltaF_press, file_no, prev_file_no, filecurr,
       page_no, index, find, auto_search, display_state, incr,
       deepth, i, j, x, y, on, off, nb;


   /* memoriser la directory courante */
   strupr(cdirname);
   strcpy(fpath, cdirname);

   /* sauvegarder les parametres de l'editeur */
   saveEdit(&sed);
   ws           = &wsaisie;
   wd           = &dirwin;
   wp           = &dialwin;

   /* initialiser la fenetre de saisie */
   initWin(ws, NORMAL|TAB, nullstr,  22, 5,44, 1, TYP1, FALSE, FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock);

   /* ouvrir les fenetres de dialogue, de saisie et de directorie */
   winPopUp(wp, W_OPEN, OBJ_CHAINAGE, TYP1);
   winPopUp(ws, W_OPEN, OBJ_CHAINAGE, TYP1);
   winPopUp(&dirwin, W_OPEN, OBJ_CHAINAGE, TYP4);

   on           = WINSYSblock; /*blockatt;*/
   off          = dirwin.ink|dirwin.paper;
   display      = TRUE;

Debut:
   write_wbox(wd, TYP4);
   _wstringDB(wd->ul_x + 2           , wd->ul_y                , 1, OLDATT, 0, &arrUp);
   _wstringDB(wd->ul_x + wd->ncol - 1, wd->ul_y                , 1, OLDATT, 0, &arrUp);
   _wstringDB(wd->ul_x + 2           , wd->ul_y + wd->nline + 1, 1, OLDATT, 0, &arrDwn);
   _wstringDB(wd->ul_x + wd->ncol - 1, wd->ul_y + wd->nline + 1, 1, OLDATT, 0, &arrDwn);

   currObj = wd;
   dl   = &dir_lst;
   init_rubrique(ws);


   clrwin(&dirwin, BLANK);
   print_at( wp, 1, 4, loadDir, NEWATT, WINDIRink|dirwin.paper);
   if (loadtype == LOAD_FILE) {
        memset((char *)dl,'\0',DIR_LST);
        if ((ret = get_dir_ls()) <= 0) { /* erreurs */
                if (ret == ERROR_DSKERROR) {
                        exit_dir(&sed, ws);
                        return(FUNC|F6);
                }
                if (error(&dialwin, 1, 4, ret) == 0)
                        strcpy(cdirname, fpath);

                strcpy(mask, defaultMask);
                goto Debut;
        }
   }
   /* sinon la liste est deja creee par
    * la CROSS REFERENCE
    */

Init_Variables:

   print_at( wp, 1, 4, AutoSearch, NEWATT, WINDIRink|dirwin.paper);
   file_no      = index = page_no = 0;
   auto_search  = TRUE;
   find         = TRUE;


   print_dir_pg(dl);
   getPos(dl, file_no, &x, &y);
   _brighten(x, y, 13, ON);
   memset(file_name,'\0',80);
   print_at( ws, 0, 0, mask, OLDATT);

   while (TRUE) {
Get:      Key  = getkey();
Continue: ChL = (Key & 0x00ff);
          ChX = (Key & 0xff00);
          switch (ChX) {
          case NORMAL :
                       switch (ChL) {
                       case RETURN :
                               if (find) {
                                   strcpy(file_name,dl->dirPage[file_no].str);

                                   /* si DIRECTORY, relancer */
                                   if (dl->dirPage[file_no].att == 0x10) {
                                       set_path();
                                       free_dp();
                                       goto Debut;
                                   }
                                   /* si Drive relancer */
                                   if (dl->dirPage[file_no].att == ID_TYPE_DR) {
                                       file_name[0] = dl->dirPage[file_no].str[DR_LETTER];
                                       file_name[1] = ':';
                                       file_name[2] = '\0';
                                       print_at( wp, 1, 4, loadDir, NEWATT, WINDIRink|dirwin.paper);
                                       set_path();
                                       free_dp();
                                       goto Debut;
                                   }
                                   /* nom de fichier, sortir apres avoir
                                    * construit le NOM ABSOLU
                                    */

                                   /* memoriser la directory courante */
                                   strcpy(fpath, cdirname);

                                   if (get_deepth(cdirname) > 0)
                                       strcat(cdirname, "\\");

                                   strcat(cdirname, file_name);
                                   strcpy(file_name,cdirname);
                                   strcpy(cdirname, fpath);

                                   *Otype = 0;/*LOAD;*/
                                   exit_dir(&sed, ws);
                                   return(0);
                               }
                               /* pathname saisi */
                               if (((ret = set_path()) == TOK_DIR) ||
                                    (ret == TOK_WILD)) {
                                   free_dp();
                                   print_nb_at( wp, 1, 4, fillstr, 17, OLDATT);
                                   goto Debut;
                               }
                               else
                                   if (ret == TOK_FILE) {
                                       /* nom de fichier, sortir */
                                       /* replacer HOME directory */
                                       strcpy(fpath, cdirname);
                                       *Otype = 0;/*LOAD;*/
                                       exit_dir(&sed, ws);
                                       return(0);
                                   }
                                   /* fichier inconnu */
/*                                 print_at(wp, 0, 4, NewFile, NEWATT, WINDIRink|dirwin.paper);
                                   ChL = getYesNo(wp);
                                   if (ChL == 'Y' || ChL == 'y') {*/
                                       /* nouveau fichier, sortir */
                                       /* replacer HOME directory */
                                       strcpy(fpath, cdirname);

                                       strcpy(cdirname, fpath);
                                       *Otype = FF_NEWFILE; /*NEW;*/
                                       exit_dir(&sed, ws);
                                       return(0);
/*                                 }*/
                                   /* creation annul-e */
/*                                 print_nb_at(wp, 0, 4,  fillstr, strlen(NewFile), OLDATT);
                                   init_rubrique(ws);
                                   goto Init_Variables;*/

                       case ESC :
                                   exit_dir(&sed, ws);
                                   return(FUNC|F6);


                       case TAB : getPos(dl,file_no,&x, &y);
                                  write_wbox(wd,0);
                                  _brighten(x, y, 13, OFF(file_no));

                                  /* FAIRE a la main le CHAINAGE des objets */
                                  currObj = ws;
                                  write_wbox(ws, 3);
                                  print_at( wp, 1, 4, Enter, NEWATT, WINDIRink|dirwin.paper);
                                  bigbuf = clippbuf;
                                  if (find) {
                                        if (dl->dirPage[file_no].att != 0x10)
                                                strcpy(file_name,dl->dirPage[file_no].str);
                                        else
                                                file_name[0] = '\0';
                                  }

                                  strlwr(file_name);
                                  strcpy(bigbuf,file_name);
                                  fsize = strlen(file_name);
                                  bigbuf[fsize]    = EOS;
                                  fsize++;
                                  if ((nb = strlen(file_name)) < ws->ncol)
                                       print_nb_at(ws, ws->curX,
                                                   ws->curY,
                                                   fillstr,
                                                   ws->ncol - strlen(file_name) - 1,
                                                   OLDATT);
                                  print_nb_at( ws, 0, 0, file_name, INF(nb, ws->ncol), OLDATT);
                                  fflag = FF_INSERT;
                                  current = topPage = 0;

                                  memset(linebuf, EOS, (nb_car_per_line - 1) * 2);
                                  lncpy(wp);
                                  Key = getfilename(ws, NOCLEAR);
                                  bigbuf[fsize - 1] = '\0';
                                  strncpy(file_name, bigbuf, 79);
                                  file_name[79] = '\0';
/*                                  linebuf[fsize - 1] = '\0';
                                  strncpy(file_name,linebuf, 79);

                                  file_name[79] = '\0';*/

                                  if ((Key & 0xff) != TAB) {
/*                                        if ((pt = strpbrk(file_name, ".")) == NULL)
                                                strcat(file_name, ".C");*/

                                        if (Key == 0)
                                                Key = (NORMAL|RETURN);

                                      find = FALSE;
                                      strupr(file_name);
                                      goto Continue;
                                  }
                                  /* replacer la fenetre de saisie
                                   * en arriere plan
                                   */
                                   currObj = wd;

                                  write_wbox(wd,TYP2);
                                  write_wbox(ws, TYP1);
                                  _brighten(x, y, 13, ON);
                                  print_nb_at( wp, 1, 4, fillstr, 17, OLDATT);
                                  print_at( wp, 1, 4, AutoSearch, NEWATT, WINDIRink|dirwin.paper);
                                  auto_search = TRUE;
                                  index = INF(strlen(file_name), ws->ncol);
                                  wgotoxy(ws, index, ws->curY);
                                  incr = 0;
                                  goto Auto_Search;

                       case BACKSP: if (ws->curX > 0) {
                                        file_name[--index] = '\0';
                                        auto_search = TRUE;
                                        print_nb_at( wp, 1, 4, fillstr, 17, OLDATT);
                                        print_at( wp, 1, 4, AutoSearch, NEWATT, WINDIRink|dirwin.paper);
                                        wgotoxy(ws,ws->curX - 1,ws->curY);
                                        print_nb_at(ws, ws->curX,
                                                        ws->curY,
                                                        fillstr,
                                                        ws->ncol - strlen(file_name) - 1,
                                                        OLDATT);
                                        incr = 0;
                                        if (index)
                                            goto Auto_Search;
                                    }
                                    else beep(1000);
                                    break;

                       default  : /* caractere alphanumerique : Auto Search */
                               if (ChL == ' ') {
                                   find     = TRUE;
                                   dlcurr   = dl;
                                   filecurr = file_no;
                                   incr     = 0;
                                   if (++file_no >= dl->nfpp) {
                                       file_no = 0;
                                       if ((dl = dl->next) == NULL)
                                            dl = &dir_lst;
                                   }
                                   goto Auto_Search_Subset;
                               }
                               file_name[index] = ChL;
                               file_name[index+1] = '\0';
                               incr = 1;
Auto_Search:
                               if (auto_search) {
                                   /* demarrer la recherche depuis le debut */
                                   find = TRUE;
                                   dlcurr = dl;
                                   filecurr = file_no;
                                   dl = &dir_lst;
                                   file_no = 0;
Auto_Search_Subset:
                                   while (strnicmp(dl->dirPage[file_no].str,file_name,index + incr) != 0) {
                                          if (++file_no >= dl->nfpp) {
                                              /* changer de page si c'est possible */
                                              if (dl->next != NULL) {
                                                  dl = dl->next;
                                                  file_no = 0;
                                              }
                                              else {
                                                  auto_search = FALSE;
                                                  find         = FALSE;
                                                  dl           = dlcurr;
                                                  file_no      = filecurr;
                                                  break;
                                              }
                                          }
                                   }
                                   if (find) {
                                       _brighten(x, y, 13, OFF(filecurr));
                                       if (dlcurr != dl)
                                           print_dir_pg(dl);
                                       getPos(dl,file_no,&x, &y);
                                       _brighten(x, y, 13, ON);
                                       print_nb_at( ws, 0, 0, fillstr, ws->ncol - 1, OLDATT);
                                       print_at( ws, 0, 0, dl->dirPage[file_no].str, OLDATT);

                                       /* si DIRECTORY, ajouter mask */
                                       if (dl->dirPage[file_no].att & 0x10) {
                                           strcpy(work, "\\");
                                           strcat(work, mask);
                                           print_at( ws, strlen(dl->dirPage[file_no].str),
                                                     0,  work, OLDATT);
                                       }
                                                     print_nb_at( wp, 1, 4, fillstr, 17, OLDATT);
                                       print_at( wp, 1, 4, AutoSearch, NEWATT, WINDIRink|dirwin.paper);
                                   }
                                   print_nb_at( ws, 0, 0, file_name,
                                                INF(strlen(file_name), ws->ncol),
                                                NEWATT, WINDIRink|dirwin.paper);
                                   wgotoxy(ws,ws->curX + incr,ws->curY);
                               }
                               index += incr;
                               if (ChL == ' ') {
                                   if (!find) {
                                       find = TRUE;
                                       auto_search = TRUE;
                                       file_no = 0;
                                       dl = &dir_lst;
                                       goto Auto_Search_Subset;
                                   }
                               }
                               else
                                   if (!find) {
                                       Key = (NORMAL|TAB);
                                       goto Continue;
                                   }
                               break;
                       }
                       break;

          case NUMPAD: deltaF = index  = 0;
                       wgotoxy(ws, 0, ws->curY);
                       print_nb_at( wp, 1, 4, fillstr, 17, OLDATT);
                       print_at( wp, 1, 4, Search, NEWATT, WINDIRink|dirwin.paper);
                       auto_search = TRUE;
                       memset(file_name,'\0',80);
                       print_nb_at( ws, 0, 0, fillstr, ws->ncol - 1, OLDATT);
                       prev_file_no = file_no;
                       switch(ChL) {
                       case ARROWL : deltaF = -1; break;  /* changer colonne */
                       case ARROWR : deltaF = +1; break;  /* changer colonne */
                       case ARROWD : if (dl->nfpp - file_no > 4) deltaF = +4;
                                     else /* on est en derniere ligne */
                                         if ((file_no != dl->nfpp - 1) ||
                                             (file_no != NB_FILE_PER_PAGE - 1)) {
                                              deltaF = 0;
                                              file_no = (file_no + 1) % 4;
                                         }
                                         else
                                             deltaF = +32;
                                     break;
                       case ARROWU : if (file_no >= 4)  deltaF = -4;
                                     else /* on est en premiere ligne */
                                         if (file_no > 0) {
                                             deltaF = 0;
                                             file_no = (dl->nfpp & 0xfffc) - (4 - file_no) - 1;
                                             if ((file_no < dl->nfpp - 4) || (file_no < 0))
                                                  file_no += 4;
                                         }
                                         else
                                             deltaF = -32;
                                     break;
                       case PGUP   : deltaF = -32;break;  /* changer page    */
                       case PGDN   : deltaF = +32;break;  /* changer page    */
                       /*case MOUSE_L: break;*/
                       case MOUSE_LP:
                       case MOUSE_RP:
                                     deltaF_press  = (((posX) - (wd->ul_x + 1))/15) +
                                                     (((posY) - (wd->ul_y + 1)) * 4);
                                     m_setMinMaxVerCursorPos(wd->ul_y * 8, (wd->ul_y + wd->nline + 1) * 8);
                                     m_setMinMaxHorCursorPos(wd->ul_x * 8, (wd->ul_x + wd->ncol  + 1) * 8);

                                     break;
                       case MOUSE_L:
                       case MOUSE_R:
                                     /* si on est sur bord droit ou gauche
                                      * ne rien faire
                                      */
                                     if ((posX == wd->ul_x) || (posX == (wd->ul_x + wd->ncol + 1)))
                                        goto Get;

                                     deltaF  = (((posX) - (wd->ul_x + 1))/15) +
                                               (((posY) - (wd->ul_y + 1)) * 4);
/*                                   if (file_no == deltaF) {
                                        /* item selectionn- */
/*                                      Key = (NORMAL|RETURN);
                                        goto Continue;
                                     }*/
/*                                   ticks = 0;
                                     while (ticks < 1);*/
                                     file_no = 0;
                                     break;
                       case MOUSE_LR:
                       case MOUSE_RR:
                                     deltaF  = (((posX) - (wd->ul_x + 1))/15) +
                                               (((posY) - (wd->ul_y + 1)) * 4);
                                     m_setMinMaxVerCursorPos(0, 192);
                                     m_setMinMaxHorCursorPos(0, 639);

                                     if ((deltaF_press == deltaF) &&
                                         (deltaF == file_no)) {
                                        /* item selectionn- */
                                        Key = (NORMAL|RETURN);
                                        goto Continue;
                                     }
                                     file_no = 0;
                                     break;
                       }
                       _brighten(x, y, 13, OFF(prev_file_no));
                       file_no += deltaF;
                       if (file_no < 0) {
                           /* on remonte d'une page si c'est possible */
                           if (dl->prev != NULL) {
                               file_no = NB_FILE_PER_PAGE - 1;
                               dl = dl->prev;
                               print_dir_pg(dl);
                           }
                           else
                               file_no = 0;
                       }
                       else
                           if (file_no >= dl->nfpp) {
                               /* on descend d'une page si c'est possible */
                               if (dl->next != NULL) {
                                   file_no = 0;
                                   dl = dl->next;
                                   print_dir_pg(dl);
                               }
                               else
                                   file_no = dl->nfpp - 1;
                           }
                       /* placer l'attribut */
                       getPos(dl,file_no,&x, &y);
                       _brighten(x, y, 13, ON);

                       /* ecrire  fichier courant */
                       print_at( ws, 0, 0, dl->dirPage[file_no].str, OLDATT);

                       /* si DIRECTORY, ajouter mask */
                       if (dl->dirPage[file_no].att & 0x10) {
                           strcpy(work, "\\");
                           strcat(work, mask);
                           print_at( ws, strlen(dl->dirPage[file_no].str),
                                     0,  work, OLDATT);
                       }
                       break;
          case FUNC:
          case ALTFUNC:
          case SHIFTFUNC:
          case CTRLFUNC:
          case ALT:
                       exit_dir(&sed, ws);
                       return(Key);
          }
   }
}

/*-----------------------------------------------------------
 * error -
 *-----------------------------------------------------------
 */
error(wp, x, y, errval)
struct OBJ *wp;
{
   char *pt;
   char work[80];
   struct OBJ errwin;

   switch (errval) {
   case ERROR_DSKEMPTY : pt = NoFile;strcpy(mask, "*.*");goto KEY;
   case ERROR_DSKOVF   : pt = TooMany;break;
   case ERROR_BADPATH  : sprintf(work, " %s%s", file_name, BadPath);
                         pt = work;break;
   case ERRNO          : if (errno > 0) {
                                pt = errno_tab[errno];break;
                         }
                         /* changement de drive sans concordance
                          * de directory: revenir en root
                          */
                         strcpy(file_name, cdirname);
                         file_name[3] = '\0';
                         set_path();
                         return(-1);
   default             : return(0);
   }
   strcpy(mask, defaultMask);
KEY:
/*   getkey();
   print_nb_at(wp, x, y, fillstr, strlen(pt), OLDATT);*/

   pushPop_MESS(pt, ERRMSG);
   return(0);
}




/*-----------------------------------------------------------
 * print_dir_pg - writes one page of the dir list in a window
 *-----------------------------------------------------------
 */
print_dir_pg(dl)
struct dir_lst *dl;
{
      int i, j, p, X, Y;
      unsigned  diratt, att;

      X      = dirwin.ul_x;
      Y      = dirwin.ul_y;
      att    = dirwin.ink|dirwin.paper;
      diratt = WINDIRink|dirwin.paper;
      p      = INF(NB_FILE_PER_PAGE,dl->nfpp);

      clrwin(&dirwin, BLANK);
      for (j = 0; j < 8 ; j++)
           for (i = 0; i < 4; i++) {
                _wstringDB( X + 3 + (i * 15),
                            Y + 1 + j,
                            12,
                            NEWATT,
                            (dl->dirPage[(j * 4) + i].att & 0x10 ? diratt : att),
                            dl->dirPage[(j * 4) + i].str);
                if (--p <= 0)  return;
           }
}

/*-----------------------------------------------------------
 * getPos - determine la position du fichier courant dans
 *          la page de directory dp
 *-----------------------------------------------------------
 */
getPos(dp, file_no, x, y)
struct dir_lst *dp;
int *x, *y;
{
   int X, Y, i, j;
      X    = dirwin.ul_x;
      Y    = dirwin.ul_y;

      for (j = 0; j < 8 ; j++)
           for (i = 0; i < 4; i++) {
                  *x = X + 2 + (i * 15);
                  *y = Y + 1 + j;
                  if (--file_no < 0)  return;
           }
}

/*-----------------------------------------------------------
 * free_dp - libere les pages de directory list
 *-----------------------------------------------------------
 */
free_dp()
{
   struct dir_lst *dl, *dlnext;

   dl = dir_lst.next;
   while (dl != NULL) {
          dlnext = dl->next;
          free(dl);
          dl = dlnext;
   }
}


/*-----------------------------------------------------------
 * init_rubrique - initialise la presentation generale
 *-----------------------------------------------------------
 */
init_rubrique(ws)
struct OBJ *ws;
{
   struct OBJ *wp;
   wp = &dialwin;

   /* ecrire zone file name */
   print_at( wp, 1, 1, initRubStr, OLDATT);
   print_at( wp, 1, 4, AutoSearch, NEWATT, WINDIRink|dirwin.paper);

   /* tracer les boxes de directory et de saisie */
/*   write_wbox(&dirwin,3);  /* type 4 */

   write_wbox(ws,0);

   /* positionner le curseur dans la zone file name */
   print_nb_at( ws, 0, 0, fillstr, ws->ncol - 1, OLDATT);
   print_at(ws, 0, 0, mask, OLDATT);
   wgotoxy(ws, 0, 0);
}

/*-----------------------------------------------------------
 * print_at - ecrire la string str aux coordonnees x, y, dans
 *            la window wp
 *-----------------------------------------------------------
 */
print_at(wp, x, y, str, isnewatt, att)
struct OBJ *wp;
char *str;
{
  int i;
  if ((i = strlen(str)) > 0)
       _wstringDB(wp->ul_x + 1 + x,
                  wp->ul_y + 1 + y, i, isnewatt, att, str);
}


/*---------------------------------------------------------
 * print_at_nb - ecrire nb carac aux coordonnees x, y, dans
 *               la window wp
 *-----------------------------------------------------------
 */
print_nb_at(wp, x, y, str, nb, isnewatt, att)
struct OBJ *wp;
char *str;
{
   if (nb > 0) {
       if (isnewatt)
           _wstringDB(wp->ul_x + 1 + x,
                      wp->ul_y + 1 + y, nb, NEWATT, att, str);
       else
           _wstringDB(wp->ul_x + 1 + x,
                      wp->ul_y + 1 + y, nb, NEWATT, wp->ink|wp->paper, str);
   }
}

/*-----------------------------------------------------------
 * getfilename - gestion saisie nom de fichier
 *-----------------------------------------------------------
 */
getfilename(ws, clearFlag)
struct OBJ *ws;
int clearFlag;  /* si pas de BACKSP remettre la chaine - 0 */
{
   unsigned ChX, ChL, Key;
   int val;

   formatt_line(linebuf, bigbuf, current, lnlen(ws,current), EOS, nb_car_per_line * 2);
   while (TRUE) {
         Key  = getkey();
         ChL  = (Key & 0x00ff);
         ChX  = (Key & 0xff00);
         switch (ChX) {
         case NUMPAD :
              clearFlag = FALSE;
              switch (ChL) {
              case ARROWL : arrow_left(ws);break;
              case ARROWR : arrow_right(ws);break;
              case END    : line_end(ws);break;
              case HOME   : line_home(ws);break;
              case DELETE : ctrlg(ws);/*clearFlag = FALSE;*/
                            break;
              case MOUSE_L:
              case MOUSE_LP:
              case MOUSE_LR:
                            wgotoxy(ws,
                                    ((val = (posX)-(ws->ul_x+1)) < 0 ? 0 : val),
                                    ws->curY);
                            break;
              case MOUSE_RP:break;
              case MOUSE_R :
              case MOUSE_RR:if (fflag & FF_LINE_UPDATE)
                                 insert_lnbuf(ws,NO_ERASE);
                            return(0);

              default     : break;
              }
              break;
         case NORMAL :
              switch (ChL) {
              case  RETURN : if (fflag & FF_LINE_UPDATE)
                                 insert_lnbuf(ws,NO_ERASE);
                             return(0);
              case  ESC    : return(FUNC|F6);
              case  TAB    : insert_lnbuf(ws,ERASE);
                             return(Key);
              default      : if (clearFlag && (ChL != BACKSP)) {
                                /* remettre la string a 0 */
                                memset(linebuf, EOS, nb_car_per_line);
                                line_home(ws);
                             }
                             winsch(ws,ChL);
                             clearFlag = FALSE;
                             break;
              }
              break;
         case CTRL :
              if (ChL != 'G' && ChL != 'g')
                  break;
              wdelch(ws);
              clearFlag = FALSE;
              break;
         case FUNC :
         case ALT  :
         case ALTFUNC:
         case CTRLFUNC:
              return(Key);
         default: break;
         }
   }
}

/*-----------------------------------------------------------
 * up_dir - remonter d'une directory dans le pathName
 *-----------------------------------------------------------
 */
up_dir(path)
char *path;
{
   int i = strlen(path) - 1;
   while (path[i] != '\\') i--;
   if (i > 2)  path[i]   = '\0';
   else        path[i+1] = '\0';
}

/*------------------------------------------------------------------------
 * get_deepth - retourne la profondeur du pathName (si 0 <= : dir courante)
 *------------------------------------------------------------------------
 */
get_deepth(path)
char *path;
{
   int deepth, i, j;
   if ((i = strlen(path)) == 3)
        return(0);
   for (j = 3, deepth = 1; j < i; j++)
          if (path[j] == '\\')
              deepth++;
   return(deepth);
}

/*------------------------------------------------------------------------
 * getdcwd - donne la directory courante pour un drive donn-
 *------------------------------------------------------------------------
 */
getdcwd(drive, path)
char *path;
{
   int sav_drive, wk_drive, sav_floppy, ret, test;


   sav_drive = _getdefaultD();

   /* si floppy, controler gestion unit-s logiques */
   if (drive <= 1) {
       sav_floppy  = curr_floppy;
       curr_floppy = drive;
       messwin = &dialwin;
       Xmess   = 1;
       Ymess   = 4;
       verify_floppy();
   }
   wk_drive = _setdefaultD(drive);

   /* si abort sur l'operation, revenir au floppy precedent */
   if ((ret = dosexterr(NULL)) == INT24FAIL) {
        if (drive <= 1)
                set_logic_floppy(sav_floppy);

        _setdefaultD(sav_drive);
        return(INT24FAIL);
   }

   test     = _getdefaultD();
   if (test != drive) {
        pushPop_MESS(badDrvStr, ERRMSG);
        return(0);
   }

   getcwd(path, 79);

   /* si abort sur l'operation, revenir au floppy precedent */
   if ((ret = dosexterr(NULL)) == INT24FAIL) {
        if (drive <= 1)
                set_logic_floppy(sav_floppy);

        _setdefaultD(sav_drive);
        return(INT24FAIL);
   }

   /* revenir au drive initial */
   wk_drive = _setdefaultD(sav_drive);
   return(ret);
}

/*------------------------------------------------------------------------
 * set_path - met a jour cdirname et file_name
 *------------------------------------------------------------------------
 */
set_path()
{
   char *glta();
   struct stat sbuf;
   int Iroot, pointFinal, point;
   char *pf, *pt;

   strupr(file_name);


Update_Path:

   /* gestion du point */
   pointFinal = point = 0;
   if ((pt = strpbrk(file_name, ".")) != NULL) {
        point++;
        if (*(pt+1) == '\0') {
                point = 0;
                pointFinal++;
                *pt = '\0';
        }
   }

   if (file_name[1] == ':')   Iroot = 2;  /* changement DRIVE */
   else                       Iroot = 0;

   strcpy(work, cdirname);

   if (file_name[Iroot] != '\\') { /* nom relatif */
       if (Iroot == 2) {
           if (getdcwd(file_name[0] - 'A', cdirname) == INT24FAIL) {
               strcpy(cdirname, homedir);
               return(TOK_DIR);
           }
       }

       /* naviguer si reference au parent */
       pf = file_name + Iroot;
       while (pf[0] == '.' && pf[1] == '.') {
              if (get_deepth(cdirname) > 0) {
                  up_dir(cdirname);
                  pf = pf + 3;
              }
              else /* root */
                  break;
       }

       /* concatainer le nom debarrass- des ".." au path */
       if (strlen(pf) > 0) {
            if (get_deepth(cdirname) > 0)
                strcat(cdirname, "\\");
            strcat(cdirname, pf);
       }
   }
   else {
       /* nom absolu */
       if (Iroot == 2) {
        if (getdcwd(file_name[0] - 'A', cdirname) == INT24FAIL) {
           strcpy(cdirname, homedir);
           return(TOK_DIR);
        }
       }
       strcpy(cdirname + (Iroot == 2 ? 0 : 2), file_name);
   }

   /* verifier si le fichier (directory) existe */

   if (stat(cdirname, &sbuf) == 0) {
       /* existence */
       if (sbuf.st_mode & S_IFDIR) { /* directorie */
           return(TOK_DIR);
       }
       else {
           /* enlever le dernier token de cdirname apres
            * avoir recopi- cdirname dans file_name
            */
           strcpy(file_name, cdirname);
           glta(cdirname);
           if (pointFinal)
                return(TOK_FILE);

           if (point == FALSE) {
                /* ajouter extension ".C" */
                strcat(file_name, ".C");
                strcpy(work, glta(file_name));
                strcpy(file_name, work);
                goto Update_Path;
           }
           return(TOK_FILE);
       }
   }
   else { /* fichier (directory) inconnu(e) */
           strcpy(file_name, cdirname);

           /* retirer le nom de fichier et verifier le path */
           glta(cdirname);
           if (stat(cdirname, &sbuf) != 0 || !(sbuf.st_mode & S_IFDIR)) {
               /* Chemin invalide */
               error(&dialwin, 1, 3, ERROR_BADPATH);
               strcpy(cdirname, work);
               return(TOK_DIR);
           }
           else { /* le chemin est correct : analyser le fichier */

                if (strpbrk(file_name, "*") != NULL ||
                    strpbrk(file_name, "?") != NULL) {
                        /* wildcar */
                        strcpy(mask, glta(file_name));
                        strcat(file_name, mask);
                        return(TOK_WILD);
                }
                else  { /* new file ? tester si extension ".C" omise */
                        if (!pointFinal && !point) {
                                /* ajouter extension ".C" */
                                strcat(file_name, ".C");
                                strcpy(work, glta(file_name));
                                strcpy(file_name, work);
                                goto Update_Path;
                        }
                }
           }
           return(TOK_UNK); /* new file */
   }
}

/*------------------------------------------------------------------------
 * glta - get last token address : retire le dernier token de l'arg
 *        et retourne son adresse
 *------------------------------------------------------------------------
 */
char *glta(path)
char *path;
{
   int i, j;
   i = j = strlen(path);
   while (i >= 2) {
          if (path[i] == '\\') break;
          i--;
   }
   strcpy(path + j + 1, path + i + 1);
   if (i > 2) path[i]   = '\0';
   else       path[i+1] = '\0';
   return(path + j + 1);
}

/*--------------------------------------------------------------------------
 * set_logic_floppy - modifie le floppy logique courant
 *--------------------------------------------------------------------------
 */
set_logic_floppy(no_floppy)
unsigned char no_floppy;
{
   unsigned char far *ptr = 0x00000504; /* RAM bios location */
   *ptr = no_floppy;
}

/*--------------------------------------------------------------------------
 * get_logic_floppy - retourne le floppy logique courant
 *--------------------------------------------------------------------------
 */
unsigned char get_logic_floppy()
{
   unsigned char  far *ptr = 0x00000504; /* RAM bios location */
   return(*ptr);
}

/*--------------------------------------------------------------------------
 * vflopp - gestion verification du floppy logique
 *--------------------------------------------------------------------------
 */
vflopp(apath)
char *apath; /* path absolu */
{
   int drive;

   /* si floppy, controler gestion unit-s logiques */
   strupr(apath);
   if (apath[1] != ':')
        return;
   drive = apath[0] - 'A';
   if (drive <= 1) {
       curr_floppy = drive;
       verify_floppy();
   }
}


/*--------------------------------------------------------------------------
 * verify_floppy - verifier si un seul floppy physique gere 2 drives logiques
 *--------------------------------------------------------------------------
 */
verify_floppy()
{
   char drive;
   struct OBJ errwin;
   unsigned char val, restore, far *ptr = 0x00000504; /* RAM bios location */

   /* si # de floppy > 1, supprimer verif */
   if (nflp_phy > 1)
       return;

   if ((val = *ptr) != curr_floppy) { /* changer de drive logique */
        verify_window(&errwin, Verify, nullstr, NO_REP_REQ, NMXMSG);
        *ptr = curr_floppy;
        drive = 'A' + curr_floppy;
        print_nb_at(messwin, Xmess + DPOS_VF, Ymess, &drive, 1, OLDATT);

        getkey();
        if (messwin != &dialwin) {
            winPopUp(messwin, W_CLOSE);
            return;
        }
        print_nb_at(messwin, Xmess, Ymess, fillstr, strlen(Verify), OLDATT);
   }
}
/*----------------------------------------------------------------------------
 * _int24 - handler de gestion des erreurs d'E/S
 *----------------------------------------------------------------------------
 */
_int24(si, bp, di, ax)
unsigned si, bp, di, ax;
{
#define RETRY 01
#define ABORT 03
#define INT24FAIL        0x53
#define INT24RETRY       0x54

        char *pt;
        unsigned  Key;
        char *ptr;
        struct OBJ errwin;
        int       c, ChL, ret;

        if (!display)
                winPopUp(pedwin, W_CLOSE);
        switch(di) {
        case 0  :/* ptr = errDriveWp;break;
        case 1  : ptr = errUnkUnit;break;*/
        case 2  :/* ptr = errDriveNr;break;*/
                ptr = Dnotrdy;
                verify_window(&errwin, ptr, nullstr, REP_REQ, ERRMSG);
                ChL = 'A' + curr_floppy; /*(ax & 0x00ff);*/
                print_nb_at(messwin, Xmess + DPOS_INT24, Ymess, &ChL, 1, OLDATT);
                ChL = getYesNo(&errwin);
                winPopUp(messwin, W_CLOSE);
                hide_cursor();
                if (ChL == 'Y' || ChL == 'y')
                        ChL = 'R';
                else
                        ChL = 'A';
                break;
        case 3  : /*ptr = errUnkComm;break;
        case 4  : ptr = errBadCRC;break;
        case 5  : ptr = errBadReq;break;
        case 6  : ptr = errSeekErr;break;
        case 7  : ptr = errUnkMtyp; break;
        case 8  : ptr = errSecNotF;break;
        case 9  : break;
        case 10 : ptr = errWriteF;break;
        case 11 : ptr = errReadF;break;
        case 12 : ptr = errGenFail;break;*/
        default : break;
        }
        winPopUp(messwin, W_CLOSE);

      ioError = TRUE;
      switch(ChL) {
      case 'A': case 'a': ret = ABORT; int24error = INT24FAIL;break;
      case 'R': case 'r': ret = RETRY; int24error = INT24RETRY;break;
      default : ret = ABORT; int24error = INT24FAIL;break;
      }
      if (ret == RETRY && !display)
        winPopUp(pedwin, W_OPEN, OBJ_CHAINAGE, TYP1);

      return(ret);
}
/*---------------------------------------------------------------------------
 *  dosexterr()
 *---------------------------------------------------------------------------
 */
dosexterr(ptr)
char *ptr; /* inutilis- */
{
        int ret;

        if (ioError) ret = int24error;
        else         ret = 0;

        ioError = FALSE;
        return(ret);
}

/*---------------------------------------------------------------------------
 *  disk_err - prend en compte les erreurs disk
 *---------------------------------------------------------------------------
 */
disk_err(ax, di, devhdr)
unsigned ax, di;
unsigned far *devhdr;
{
#define  DRIVENOTRDY  2
#define  UNKMEDIA     7
#define  GNLFAIL     12

#define _HARDERR_IGNORE     0   /* Ignore the error                          */
#define _HARDERR_RETRY      1   /* Retry the operation                       */
#define _HARDERR_ABORT      2   /* Abort program issuing Interrupt 23h       */
#define _HARDERR_FAIL       3   /* Fail the system call in progress          */
                                /* _HARDERR_FAIL is not supported on DOS 2.x */

  unsigned  Key;
  char *ptr;
  struct OBJ errwin;
  int       c, ChL;
  int       restore = FALSE;

  if (ax < 0)  /* device error */
      _hardretn(-1); /* retourner dans l'application */
  else {       /* disk error */
      hide_cursor();
      switch(di) {
      case DRIVENOTRDY :
           ptr = Dnotrdy;
           verify_window(&errwin, ptr, nullstr, REP_REQ, ERRMSG);
           ChL = 'A' + curr_floppy; /*(ax & 0x00ff);*/
           print_nb_at(messwin, Xmess + DPOS_INT24, Ymess, &ChL, 1, OLDATT);
/*           ChL = 'R';
           print_nb_at(messwin, Xmess + LPOS_INT24, Ymess, &ChL, 1, OLDATT);*/
           ChL = getYesNo(&errwin);
           winPopUp(messwin, W_CLOSE);
           hide_cursor();
           if (ChL == 'Y' || ChL == 'y')
                ChL = 'R';
            else
                ChL = 'A';
/*
           while ((c = getkey() & 0x00ff) != RETURN) {
                  if (c != 'A' && c != 'a' && c != 'R' && c != 'r') {
                      beep(1000);
                      continue;
                  }
                  ChL = c;
                  print_nb_at(messwin, Xmess + LPOS_INT24, Ymess, &ChL, 1, OLDATT);
           }*/
           break;
      case UNKMEDIA :
           ptr = Unkmedia;
           verify_window(&errwin, ptr, nullstr, NO_REP_REQ, ERRMSG);
           getkey();
           ChL = 'A';
           break;
      case GNLFAIL :
           ptr = Gnfail;
           verify_window(&errwin, ptr, nullstr, NO_REP_REQ, ERRMSG);
           getkey();
           ChL = 'A';
           break;
       default: _hardresume(_HARDERR_FAIL);
      }
      winPopUp(messwin, W_CLOSE);

      switch(ChL) {
      case 'A': case 'a': _hardresume(_HARDERR_FAIL);
      case 'R': case 'r': _hardresume(_HARDERR_RETRY);
      }
  }
}
/*---------------------------------------------------------------------------
 * verify_window - gestion apparition des messages systemes
 *---------------------------------------------------------------------------
 */
verify_window(wp, str, titre, rep_req, type)
struct OBJ *wp;
char *str, *titre;         /* message a ecrire et titre */
{
    int size, yn, relativ;
    unsigned inkAtt, paperAtt, borderAtt, messAtt;
    static  struct  MOUSE_FIELD mfYN[5];
    int mv0(), mv1(), mv2();

        if (type == ERRMSG) {
                inkAtt    = WINmessInk;
                paperAtt  = WINmessPaper;
                borderAtt = WINmessBorder;
        }
        else {
                inkAtt    = WINSYSink;
                paperAtt  = WINSYSpaper;
                borderAtt = WINSYSborder;
        }
        yn = (rep_req == REP_REQ ?  2 : 0);

        messAtt   = inkAtt|paperAtt;

        if ((size = strlen(str)+2) > 78)
                size = 76;
        if (yn)
                size = SUP(size, strlen(verifYN)+2);

        initWin(wp, 0, titre, (80 - size)/2, 10, size, 3 + yn, TYP1, /*TYP5,*/
                TRUE, FALSE, FALSE, inkAtt, paperAtt, borderAtt,
                vl->WINSYSblock, W_WIN);

        relativ = (size - strlen(verifYN))/2;
        if (yn) {
                /* initialiser la zone Champ */
                wp->mField = mfYN;

                /*            f#  x   y  sz  (* )() */
                initField(wp,  0, relativ+6, 3, 1,  mv0);
                initField(wp,  1, relativ+15, 3, 1,  mv1);
                initField(wp,  2, relativ+23, 3, 9,  mv2);
                FIELD(wp, 3)->size      = 0;    /* marque la fin des champs */

                /* initialiser les actions souris */
                wp->M_Dpr  = m_editDpr;
                wp->M_Gpr  = m_editGpr;
                wp->M_Gre  = m_editGre;
                wp->M_CGpr = m_editCGpr;
                wp->M_CDpr = m_editCDpr;
/*              wp->mouse_ev_flag |= M_MOVMENT;*/
                winPopUp(wp, W_OPEN, OBJ_CHAINAGE, TYP5);
                print_at(wp, relativ, 3, verifYN, OLDATT);
        }
        else
                winPopUp(wp, W_OPEN, OBJ_CHAINAGE, TYP5);
        messwin = wp;
/*        Xmess = 0; */
        Ymess = 1;
        print_at(messwin, (Xmess = (size - strlen(str))/2), Ymess, str, NEWATT, messAtt);
}

mv0(p, no)
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!Yes_answ) {
        Yes_answ = TRUE;
        print_at(p, x,                  y, "X", OLDATT);
        print_at(p, FIELD(p, no+1)->rX, y, " ", OLDATT);
        No_answ = FALSE;
   }
   return(0);
}
mv1(p, no)
struct OBJ *p;
{
   int x, y;
   wgotoxy(p, (x = FIELD(p, no)->rX), (y = FIELD(p, no)->rY));
   if (!No_answ) {
        No_answ = TRUE;
        print_at(p, x,                  y, "X", OLDATT);
        print_at(p, FIELD(p, no-1)->rX, y, " ", OLDATT);
        Yes_answ = FALSE;
   }
   return(0);
}
mv2(p)
struct OBJ *p;
{
   return(1);
}
