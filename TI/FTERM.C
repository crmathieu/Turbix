/* fterm.c : fonctions terminales de tous les MENUS */
#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>
#include <errno.h>

#define  TOO_BIG  1
#define  NOT_OPEN 2

nullfunc()
{
   pushPop_MESS(" Not Implemented ", NMXMSG);
   return(FUNC|F6);
}

/*  >>>>>>>>>>>>>>>>>>>>>>> MASTER  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*------------
 * doEdit -
 *------------
 */
doEdit(mp)
struct OBJ *mp;
{
   extern struct OBJ *whereIsMouse();
   struct OBJ *ob;

/*   pushPop_MESS(" TEST de RETOUR a l'editeur ... ", ERRMSG);*/
   if (fflag & FF_HELP_MOD) { /* mode help edit */
        if (whereIsMouse(tailObj) == &edwin) {
/*                  pushPop_MESS(" RETOUR a l'editeur ... ", ERRMSG);*/
                  edHelp(HCLOSE);
        }
        else  {
/*                  pushPop_MESS(" REOPEN ... ", ERRMSG);*/
                  return(edHelp(HREOPEN));
       }
   }
/*   pushPop_MESS(" EDITEUR !!!! ", ERRMSG);*/
   return(ed(pedwin));
}
/*------------
 * doRunPrj -
 *------------
 */
doRunPrj(mp)
struct OBJ *mp;
{
        if (!vl->rebuild) {
                vl->rebuild = TRUE;
                doMake(mp);
        }
        vl->rebuild = FALSE;
        strcpy(exename, vl->conf.projExe);
        doRun(mp);
}
/*------------
 * doRunExe -
 *------------
 */
doRunExe(mp)
struct OBJ *mp;
{
        struct stat sbuf;
        long date;

        /* sauvegarder le fichier */
        save_previous_file(absfname);

        /* creer le nom de l'OBJ */
        strcpy(exename, file_name);
        makeExtension(exename, ".obj");

        /* verifier les dates */
        if (stat(file_name, &sbuf) == 0)
                date = sbuf.st_atime;

        /* tester OBJ */
        if (stat(exename, &sbuf) == 0) {
                /* existence */
                if (sbuf.st_atime < date)  /* recompiler */
                        vl->recomp = TRUE;
                else    vl->recomp = FALSE;
        }
        else
                vl->recomp = TRUE;

        /* tester EXE */
        makeExtension(exename, ".exe");
        if (stat(exename, &sbuf) == 0) {
                /* existence */
                if (sbuf.st_atime < date)  /* relinker */
                        vl->relink = TRUE;
                else    vl->relink = FALSE;
        }
        else
                vl->relink = TRUE;
        if (vl->recomp)
                doCompile(mp);
        if (vl->relink)
                doLink(mp);
        doRun(mp);

}
/*---------------
 * makeExtension
 *---------------
 */
makeExtension(str, ext)
char *str, *ext;
{
        char *pt;
        if ((pt = strrchr(str, '.')) != NULL)
                *pt = '\0';
        strcat(str, ext);
}


/*------------
 * doQuit -
 *------------
 */
doQuit(mp)
struct OBJ *mp;
{
   unsigned ret;
   exit_txed(mp);
}

/*------------
 * exit_txed -
 *------------
 */
exit_txed(mp)
struct OBJ *mp;
{
   extern char Question[], Savefile[];
   extern unsigned long BiosIntclk, BiosInt33;
   extern int mouse_present;
   struct OBJ errwin;
   char aux[80];
   int ChL, Key, sav_fflag;


   if (fflag & FF_DIRTY) {
         hide_cursor();
        strcpy(work, Savefile);
        strcat(work, absfname);
        strcat(work, Question);
        verify_window(&errwin, work, nullstr, REP_REQ, NMXMSG);
        ChL = getYesNo(&errwin);
        winPopUp(messwin, W_CLOSE);
        hide_cursor();
        if (ChL == 'Y' || ChL == 'y')
            doSave(absfname);
        fflag &= ~FF_DIRTY;
   }
   if (conf.autoSave) {
        push_pick(absfname);

        /* sauvegarder la configuration */
        memmove(&conf.reload[0], &picktab[0], PICKSIZE * 17);
   }


   doSconfig(mp, 0);
/*   popSM(&mainMenu);*/
   strcpy(aux, cdirname);
   strcat(aux,"TURBOC.CFG");
   remove(aux);
   inhibe_mouse();

   /* replacer Home dir */
   memset(aux, '\0', 80);
   strcpy(aux, homedir);
   _performChgDir(aux);

   /* replacer gestion Dos de l'int24 */
   _dos_setvect(0x24, DosInt24);
   free(bigbuf);
   free(fillstr);
   free(clippbuf);
   free(lineTab);
   free(linebuf);

   exit(7);     /* code de sortie dans ti.exe */
}


/*------------
 * getYesNo - reponse  U.S. et FR.
 *------------
 */
getYesNo(pob)
struct OBJ *pob;
{
        int i, ChL;

        Yes_answ = TRUE;
        No_answ  = FALSE;
        print_at(pob, FIELD(pob, 0)->rX, FIELD(pob, 0)->rY, "X", OLDATT);
        get_answer(pob, &i);
        if (Yes_answ)
                return((int)'y');
        return((int)'n');
}

/*------------
 * inhibe_mouse
 *------------
 */
inhibe_mouse()
{
   _cli();
   if (mouse_present)
        m_callMaskAndAddress(0, 0L);
   _dos_setvect(0x1c, BiosIntclk);
   _sti();
    return;
}


/*  >>>>>>>>>>>>>>>>>>>>>>> EDIT  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*------------
 * doFindStr
 *------------
 */
doFindstr(mp, num)
struct OBJ *mp;
{
   ctrlqf(pedwin, FIND);
   return(FUNC|F6);
}

/*------------
 * doFindnext
 *------------
 */
doFindnext(mp, num)
struct OBJ *mp;
{
   ctrll(pedwin);
   return(FUNC|F6);
}

/*------------
 * doRepl
 *------------
 */
doRepl(mp, num)
struct OBJ *mp;
{
   ctrlqa(pedwin);
   return(FUNC|F6);
}

/*------------
 * doGoto
 *------------
 */
doGoto(mp, num)
struct OBJ *mp;
{
   ctrlqg(pedwin, num);
   return(FUNC|F6);
}

/*------------
 * doLineLength
 *------------
 */
doLineLength(wp, num)
struct OBJ *wp;
{
        ctrlql(wp, num);
        return(FUNC|F6);
}

/*-----------------------------------------------------------
 * doTabulation -
 *-----------------------------------------------------------
 */
doTabulation(wp)
struct OBJ *wp;
{
   char work[80], work2[80];
   unsigned ret;
   int i;

   memset(work, '\0', 80);
   strcpy(work2,tabStr);
   strcat(work2, itoa(tablength, NULL, 10));
   ret = doEnter(work2, work, FALSE, FALSE);
   hide_cursor();
   if (ret)
        return(ret);
   if ((i = atoi(work)) > 0)
        tablength = i;
   return(FUNC|F6);
}

/*-----------------------------------------------------------
 * doCopyToPaste -
 *-----------------------------------------------------------
 */
doCopyToPaste(wp)
struct OBJ *wp;
{
   ctrlkw(pedwin);
   /* retourner au help ou - l'editeur */
   return(FUNC|F6);
}

/*-----------------------------------------------------------
 * doDuplicate -
 *-----------------------------------------------------------
 */
doDuplicate(wp)
struct OBJ *wp;
{
   ctrlkc(pedwin);
   /* retourner au help ou - l'editeur */
   return(FUNC|F6);
}

/*-----------------------------------------------------------
 * doDelete -
 *-----------------------------------------------------------
 */
doDelete(wp)
struct OBJ *wp;
{
   ctrlky(pedwin);
   /* retourner au help ou - l'editeur */
   return(FUNC|F6);
}

/*-----------------------------------------------------------
 * doMove -
 *-----------------------------------------------------------
 */
doMove(wp)
struct OBJ *wp;
{
   ctrlkv(pedwin);
   /* retourner au help ou - l'editeur */
   return(FUNC|F6);
}

/*-----------------------------------------------------------
 * doReadFromPaste -
 *-----------------------------------------------------------
 */
doReadFromPaste(wp)
struct OBJ *wp;
{
   ctrlkr(pedwin);
   /* retourner au help ou - l'editeur */
   return(FUNC|F6);
}




/*  >>>>>>>>>>>>>>>>>>>>>>> ENV  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*----------
 * doKbSpeed
 *----------
 */
doKbSpeed(fast)
{
   int delay, rate;


   if (fast) {
       rate = 0;
       delay = 1;
   }
   else {
       rate  = 0x0a;
       delay = 2;
   }
   key_delay(delay, rate);
/*   return(FUNC|F6);*/
}


/*  >>>>>>>>>>>>>>>>>>>>>>> FILE MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*----------
 * doLoad
 *----------
 */
doLoad(mp)
struct OBJ *mp;
{
   int  Otype;
   unsigned ret;

   if ((ret = display_dir(&Otype, LOAD_FILE)) != 0)
        return(ret);

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   if (loadFile(Otype, file_name, absfname, WINMSG, V_CREAT) < 0)
             return(FUNC|F6);

   return(FUNC|F6);
}

/*-----------------------------------------
 * doLoadlast - charge le fichier precedent
 *-----------------------------------------
 */
doLoadlast(mp)
struct OBJ *mp;
{
   extern char pickload[];
   int ret_ed;

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   if (stricmp(picktab[0].str, pickload) != 0) {
       strcpy(file_name, &picktab[0].str[1]); /* shunter le 1er blanc */
       file_name[strlen(file_name) - 1] = '\0';
   }
   else  /* " Open...  " selectionn- */
       return(FUNC|F3);

   if (loadFile(picktab[0].fflag, file_name, absfname, WINMSG, V_CREAT) < 0)
       return(FUNC|F10);
/*   return(ed(pedwin));*/
   return(FUNC|F6);
}

/*-----------------------------------------
 * doSaveFile - sauvegarder le fichier courant
 *-----------------------------------------
 */
doSaveFile(mp)
struct OBJ *mp;
{
   unsigned ret;
   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);
   if (doSave(absfname) == 0) {
        fflag &= ~FF_DIRTY;
        if ((ret = ReturnFromSaveFile) != 0) {
            ReturnFromSaveFile = 0;
            return(ret);
        }
   }
   return(FUNC|F6);
}

/*-------------------------------------------------------------------------
 * loadFile - en entree, file_name contient le nom du fichier selectionn-
 *            par "F3" et absfname contient le nom du fichier pr-c-dent
 *-------------------------------------------------------------------------
 */
loadFile(mode, nextf, prevf, msgflag, o_mode)
char *nextf, *prevf;
{

    extern int curr_floppy;
    extern char Verify[], Unkfile[], Press[];
    extern char *errno_tab[];
    extern char comptab[3][80];
    extern char Question[];
    extern char *glta();
    unsigned readfile();
    struct pick pck;
    int        testfflag, retcheck, errflag, pickf,
               exterr, i, drive, ChL, Key, err;
    unsigned   cumul, size, load;
    char       *pfile, sdrive[2];
    struct OBJ errwin;
    struct stat sbuf;

    if (save_previous_file(prevf) < 0)
        return(-1);

   /*  controler gestion unit-s logiques */
   vflopp(nextf);

   /* initialiser les valeurs de l'editeur en valeurs par defaut */
   pck   = pckdefault;
   pickf = FALSE;

   /* initialiser le fichier a compiler */
    strcpy(work, nextf);
    pfile = glta(work); /* nom du fichier dans pfile */

    if (stricmp(cdirname, work) == 0)
        strcpy(relname, pfile);   /* fichier dans directorie courante */
    else
        strcpy(relname, nextf); /* fichier dans une autre directorie */

   /* si fichier deja en PICK TABLE, recuperer sa position */
   if ((retcheck = check_pick(nextf)) != -1) {
        pck = pop_pick(retcheck);
        pickf = TRUE;
        testfflag = pck.fflag;
/*        strcpy(work, cdirname);             /**/
/*        if (set_path(nextf) == TOK_DIR) {
                strcpy(cdirname, work);
                return(-1);
        }*/
/*        strcpy(cdirname, work);             /**/
   }
   else
        testfflag = mode;

Newfile:

   if (testfflag & FF_NEWFILE) {
       /* si absfname est de taille nulle, il n'y a encore aucun fichier */
       if (strlen(prevf) > 0 && stricmp(prevf, nextf) != 0)
           /* on charge un nouveau fichier */
           push_pick(prevf);

       /* alors ce fichier n'existe pas encore sur disque */
       init_ed_from_pick(&pck, 0, pickf);

       /* ancien emplacement */
       strcpy(prevf, nextf);
       return(0);
   }

   if ((fdcurr = open(nextf, O_RDONLY /*O_RDWR*/)) < 0) {
        if ((exterr = dosexterr(NULL)) != INT24FAIL)  {
             if (errno != ENOENT) {
                 pushPop_MESS(errno_tab[errno],ERRMSG);
                 return(-1);
             }
             /* est on autoris- - charger un nouveau fichier ? */
             if (o_mode == V_CREAT) {
                testfflag |= FF_NEWFILE;
                pck = pckdefault;
                pickf = FALSE;
                goto Newfile;
             }
             else {
                 winPopUp(pedwin, W_CLOSE);       /**/
                 pushPop_MESS(helpErrStr,ERRMSG);
                 return(-1);
             }
        }
        else {
             /* l'utilisateur a tap- l'option ABORT
              * si le fichier vient de la PICK TABLE, le reinserer
              */
             if (retcheck != -1)
                               push_pick(nextf);
             if ((retcheck = check_pick(prevf)) != -1) {
                 pck = pop_pick(retcheck);
                 pickf = TRUE;
             }
             return(-1);
        }
   }
   if (strlen(prevf) > 0 && stricmp(prevf, nextf) != 0)
      /* on charge un nouveau fichier */
      push_pick(prevf);

    /* LOAD */
    errflag = 0;
    fstat(fdcurr, &sbuf);

    if (sbuf.st_size > BIGBUF_SIZE) { /* fichier trop grand */

FILE_TOO_BIG:

        hide_cursor();
        errflag = TRUE;
        strcpy(work, " ");
        strcat(work, nextf);
        strcat(work, loadErrStr);
        verify_window(&errwin, work, nullstr, REP_REQ, NMXMSG);
        ChL = getYesNo(&errwin);
        winPopUp(messwin, W_CLOSE);
        hide_cursor();
       if (ChL == 'N' || ChL == 'n') {
             if ((retcheck = check_pick(prevf)) != -1) {
                 pck = pop_pick(retcheck);
                 pickf = TRUE;
             }
             return(-1);
       }
    }
    if (msgflag == WINMSG) {
        strcpy(work, Load);
        strcat(work, nextf);
        strcat(work, " ");
        push_MESS(work, &errwin);
    }

    i = cumul = 0;
    cumul = readfile(&err, errflag);
    if (msgflag == WINMSG)
        pop_MESS();

    if (err == TOO_BIG)
        goto FILE_TOO_BIG;

    fflag |= FF_FLOADED;
    if ((err == NOT_OPEN) && (exterr = dosexterr(NULL)) == INT24FAIL) {
         /* l'utilisateur a tap- l'option ABORT
          * si le fichier vient de la PICK TABLE, le reinserer
          */
         if (retcheck != -1)
             push_pick(nextf);
         if ((retcheck = check_pick(prevf)) != -1) {
              pck = pop_pick(retcheck);
              pickf = TRUE;
         }
         close(fdcurr);
         return(-1);
    }

    strcpy(prevf, nextf);

    init_ed_from_pick(&pck, cumul, pickf);

    close(fdcurr);
    return(0);
}

/*  >>>>>>>>>>>>>>>>>>>>>>> PICK  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*----------
 * doPick
 *----------
 */
doPick(mp)
struct OBJ *mp;
{
   extern char pickload[];
   extern char *glta();
   char work[80], sauve[80];
   int ret;

   strcpy(work, file_name);
   if (stricmp(picktab[mp->curY].str, pickload) != 0) {
       strcpy(file_name, &picktab[mp->curY].str[1]); /* shunter le 1er blanc */
       file_name[strlen(file_name) - 1] = '\0';
   }
   else  /* " Open...  " selectionn- */
       return(FUNC|F3);

   strcpy(sauve, file_name);
   if (((ret = set_path()) == TOK_DIR) || (ret == TOK_WILD)) {
            pop_pick(mp->curY);
            strcpy(file_name, work);
            return(FUNC|F6);
   }
   strcpy(file_name, sauve); /* retablir le nom de fichier avant set_path
                             * (UN .C peut avoir ete rajoute)
                             */
   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   if (loadFile(picktab[mp->curY].fflag, file_name, absfname, 0, V_CREAT) < 0)
       return(FUNC|F10);

   return(FUNC|F6);
}

/*-----------------------------------------------------------------------
 * init_ed_from_pick - initialiser les variables de travail de l'editeur
 *                     pour un fichier a creer
 *-----------------------------------------------------------------------
 */
init_ed_from_pick(pck, size, pickflag)
struct pick *pck;
unsigned size;
{
    unsigned u, saveflag;

    /* rajouter DELIMITEUR de fin de fichier */
    bigbuf[size] = EOFM;
    fsize        = size + 1;

    /* initialiser le pick defaut */
    if (fsize != pck->fsize) {
        /* sauvegarder le flag de non groupement si il existe */
        saveflag = fflag & FF_NO_GROUP;
        init_edVar(0);
        fflag |= saveflag;
        if (pickflag) {
                if (pck->current > fsize)
                        pck->current = fsize;
                if (pck->current == 0)
                        current = 0;
                else
                        current = lnprev(pedwin, pck->current);
                topPage         = current;
                current_line_no = topPage_line_no = get_line_no(current);
        }
    }
    else {
           /* BUG : si le fichier a ete modifie, mais que la taille
            * reste inchangee, les donnees du pick sont erronnees
            */
        current         = pck->current;
        topPage         = pck->topPage;
        topBlock        = pck->topBlock;
        bottomBlock     = pck->bottomBlock;
        current_line_no = pck->current_line_no;
        topPage_line_no = pck->topPage_line_no;
        pedwin->leftCh  = pck->leftCh;
        pedwin->curX    = pck->curX;
        pedwin->curY    = pck->curY;
        fflag           = pck->fflag;

    }
    write_status(pedwin);
    wgotoxy(pedwin, pedwin->curX, pedwin->curY);
    bottom = getlast_ln(pedwin);
/*    write_wbox(pedwin, TYP2);   /* rendre fenetre editeur active */
    set_win(pedwin);
    bottom_line_no = get_line_no(fsize);  /* LONG !! */
}

/*-----------------------------------------------------------------------
 * save_previous_file - gestion sauvegarde d'un fichier
 *-----------------------------------------------------------------------
 */
save_previous_file(prevf)
char *prevf;
{
   struct OBJ errwin;
   int ChL, Key, sav_fflag;
   extern char Question[], Savefile[];

   if (fflag & FF_DIRTY) {
        strcpy(work, Savefile);
        strcat(work, prevf);
        strcat(work, Question);
        verify_window(&errwin, work, nullstr, REP_REQ, NMXMSG);
        ChL = getYesNo(&errwin);
        winPopUp(messwin, W_CLOSE);
        hide_cursor();
        if (ChL == 'Y' || ChL == 'y') {
                if (doSave(prevf) == 0)
                        fflag &= ~FF_DIRTY;
                else
                        return(-1);
        }
        else
                fflag &= ~(FF_DIRTY|FF_LINE_UPDATE);
/*        else
            init_edVar(1);   Modif fevrier 92 */
   }
   return(0);
}

/*-----------------------------------------------------------------------
 * doSave - sauvegarder un fichier (retourne 0 si tout est OK)
 *-----------------------------------------------------------------------
 */
doSave(file)
char *file;
{
   int ret, reset, n;
   struct OBJ errwin;
   int fd;
   extern char *errno_tab[];

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);
   vflopp(file);

   if (fflag & FF_LINE_UPDATE)
       insert_lnbuf(pedwin, ERASE);

/*   strcpy(work, Save);
   strcat(work, file);
   strcat(work, " ");
   push_MESS(work, &errwin);*/
   /* creer le fichier sur disque */
   reset = FALSE;
   for (;;) {
        /* Verifier s'il y a assez de place */
        if ((ret = chk_devspace(file, reset)) == 0)
                break;
        else
                if (ret == -1) {
                        /* pas assez de memoire sur cette disquette */
                        reset = TRUE;
                        reset_floppy();
/*                      pop_MESS();*/
                        pushPop_MESS(errno_tab[ENOSPC], ERRMSG);
                        return(-1);
                }
                else
                        break;
   }
   /* a ce stade, on est sur d'avoir la place pour sauvegarder */
   if (!(fflag & (FF_NEWFILE|FF_TMPFILE))) {
       P_bak(work, file);
       remove(work);            /* supprimer .BAK */
       rename(file, work);  /* transformer derniere version en .BAK */
       close(open(work, O_RDONLY)); /* flusher les buffers */
   }

   if ((fd = creat(file, S_IREAD|S_IWRITE)) < 0) {
/*      pop_MESS();*/
        pushPop_MESS(errno_tab[errno], ERRMSG);
        return(-1);
   }
   strcpy(work, Save);
   strcat(work, file);
   strcat(work, " ");
   push_MESS(work, &errwin);
   /* ecrire nouvelle version */
   if (write(fd, bigbuf, fsize) == -1) {
       /* Pb d'ecriture */
       /*remove(file);*/
       flush_buffer(fd);
       close(fd);
       pop_MESS();
       pushPop_MESS(errno_tab[errno], ERRMSG);
       return(-1);
   }
   if (flush_buffer(fd))
       pushPop_MESS(" Flush impossible... ", ERRMSG);
   close(fd);
   pop_MESS();
   return(0);
}

/*------------------------------------------
 * P_bak - creer le nom de fichier en ".BAK"
 *------------------------------------------
 */
P_bak(dest, src)
char *dest, *src;
{
    char *ptr;

    strcpy(dest, src);

    /* rechercher la derniere occurence  de "." */
    if ((ptr = strrchr(dest, '.')) != NULL)
         /* une extension existe */
         *ptr = '\0';
    strcat(dest, ".BAK");
}

/*--------
 * push_SL
 *--------
 */
push_SL2(str)
char *str;
{
    _wstringDB( 0, 24, 80, NEWATT, MESS24att, str);
}
/*--------
 * pushPop_MESS
 *--------
 */
pushPop_MESS(str, msgtyp)
char *str;
{
   struct OBJ errwin;
   char aux[80];
   strcpy(aux, str);
   verify_window(&errwin, aux, nullstr, NO_REP_REQ, msgtyp);
   getkey();
   winPopUp(messwin, W_CLOSE);
}


/*--------
 * push_MESS
 *--------
 */
push_MESS(str, errwin)
char *str;
struct OBJ *errwin;
{
   strcpy(work, str);
   verify_window(errwin, work, nullstr, NO_REP_REQ, NMXMSG);
}

/*--------
 * pop_MESS
 *--------
 */
pop_MESS()
{
   winPopUp(messwin, W_CLOSE);
}

/*--------
 * pop_SL
 *--------
 */
pop_SL2()
{
    _wstringDB(0, 24, 80, NEWATT, MENUink|MENUpaper, fillstr);
    write_status(pedwin);
}

/*----------
 * saveEdit - sauvegarde les parametres Globaux de positionnement
 *----------
 */
saveEdit(psed)
struct save_edit *psed;
{
   psed->s_bigbuf               = bigbuf;
   psed->s_flag                 = fflag;
   psed->s_fsize                = fsize;
   psed->s_curr                 = current;
   psed->s_page                 = topPage;
   psed->s_bottom               = bottom;
   psed->s_topBlock             = topBlock;
   psed->s_bottomBlock          = bottomBlock;
   psed->s_current_line_no      = current_line_no;
   psed->s_topPage_line_no      = topPage_line_no;
   psed->s_bottom_line_no       = bottom_line_no;
   psed->s_leftCh               = pedwin->leftCh;
   psed->s_curX                 = pedwin->curX;
   psed->s_curY                 = pedwin->curY;


   fflag |= FF_SAISIE_MOD;
   current      = 0;
   topPage      = 0;
   bottom       = 0;

}

/*-------------
 * restoreEdit - restaure les parametres Globaux de positionnement
 *-------------
 */
restoreEdit(psed)
struct save_edit *psed;
{
    bigbuf              = psed->s_bigbuf;
    fflag               = psed->s_flag;
    fsize               = psed->s_fsize;
    current             = psed->s_curr;
    topPage             = psed->s_page;
    bottom              = psed->s_bottom;
    topBlock            = psed->s_topBlock;
    bottomBlock         = psed->s_bottomBlock;
    current_line_no     = psed->s_current_line_no;
    topPage_line_no     = psed->s_topPage_line_no;
    bottom_line_no      = psed->s_bottom_line_no;
    pedwin->leftCh        = psed->s_leftCh;
    pedwin->curX          = psed->s_curX;
    pedwin->curY          = psed->s_curY;

    formatt_line(linebuf, bigbuf, current, lnlen(pedwin,current), EOS, nb_car_per_line * 2);
}

/*----------
 * doChgDir
 *----------
 */
doChgDir(mp,num)
struct OBJ *mp;
{

   char   work[80];
   unsigned ret;

   strupr(homedir);
   strupr(cdirname);

   strcpy(work, homedir);

   /* saisir le nouveau nom de directory */
   ret = doEnter(chgDirStr, work, TRUE, FALSE);
   hide_cursor();
   if (ret)
        return(ret);

   /* effectuer le changement de directory courante */
   if (_performChgDir(work) == 0) {
        strcpy(homedir, work);
        strcpy(cdirname, work);
   }
   return(FUNC|F6);
}

_performChgDir(str)
char *str;
{
   char aux[80];

   strupr(str);
   if (str[1] != ':') {
        strcpy(aux, str);
        memset(str, '\0', 79);
        str[0] = cdirname[0];
        str[1] = ':';
        if (aux[0] != '\\')
                str[2] = '\\';
        strcat(str, aux);
   }
   if (doChdDrive(str))   /* erreur d'E/S */
        return(-1);

   if (chdir(str)) {
        pushPop_MESS(errno_tab[errno], ERRMSG);
        return(-1);
   }
   return(0);
}

/*----------
 * doChgDrive
 *----------
 */
doChdDrive(str)
char *str;
{
        int drive, sav_drive, wk_drive, sav_floppy, ret, Chg, test;


   strupr(str);

                drive = str[0] - 'A';
                sav_drive = _getdefaultD();

                /* si floppy, controler gestion unit-s logiques */
                if (drive <= 1) {
                        sav_floppy  = curr_floppy;
                        curr_floppy = drive;
                        verify_floppy();
                }
                wk_drive = _setdefaultD(drive);
                test     = _getdefaultD();

                /* si abort sur l'operation, revenir au floppy precedent */
                if ((ret = dosexterr(NULL)) == INT24FAIL)
                        set_logic_floppy(sav_floppy);

                if (test != drive) {
                        pushPop_MESS(badDrvStr/*chgDriveStr*/, ERRMSG);
                        return(-1);
                }
                return(0);

}

/*----------
 * doSaveAs
 *----------
 */
doSaveAs(mp,num)
struct OBJ *mp;
{
   char work[80], w2[80];
   int drive, sav_drive, wk_drive, sav_floppy, Chg, test;
   unsigned ret;

   work[0] = '\0';
   ret = doEnter(saveAsStr1, work, FALSE, FALSE);
   if (strlen(work) == 0)
        return(FUNC|F6);

   strupr(work);
   hide_cursor();
   if (ret)
        return(ret);
   Chg = FALSE;


   if (work[0] == '\\') {
        w2[0] = 'A' + _getdefaultD();
        w2[1] = ':';
        strcat(w2, work);
        strcpy(work, w2);
   }
   if (work[1] == ':') { /* changer de drive */
                Chg = TRUE;
                drive = work[0] - 'A';
                sav_drive = _getdefaultD();

                /* si floppy, controler gestion unit-s logiques */
                if (drive <= 1) {
                        sav_floppy  = curr_floppy;
                        curr_floppy = drive;
                        verify_floppy();
                }
                wk_drive = _setdefaultD(drive);
                test = _getdefaultD();
                if (test != drive ) {
                        pushPop_MESS(badDrvStr, ERRMSG);
                        return(FUNC|F6);
                }

                /* si abort sur l'operation, revenir au floppy precedent */
                if ((dosexterr(NULL)) == INT24FAIL)
                        set_logic_floppy(sav_floppy);

                strcpy(w2, work);
        }
        else {
                if (strlen(work) > 0) {
                        strcpy(w2, cdirname);
                        if (get_deepth(w2) > 0)
                                strcat(w2, "\\");
                        strcat(w2, work);
                }
        }
        if (Chg) {
                if (wk_drive >= sav_drive)
                        if (doSave(w2) == 0)
                                fflag &= ~FF_DIRTY;
                _setdefaultD(sav_drive);
        }
        else
                if (doSave(w2) == 0)
                        fflag &= ~FF_DIRTY;
        return(ret ? ret : FUNC|F6);
}


/*----------
 * doChgPaste
 *----------
 */
doChgPaste(mp,num)
struct OBJ *mp;
{
   char work[80];
   unsigned ret;

   ret = doEnter(chgPaste, conf.u.pd.c_p, FALSE, FALSE);
   hide_cursor();
   if (ret)
        return(ret);

   strcpy(pastefile, conf.u.pd.c_p);
   return(FUNC|F6);
}

/*  >>>>>>>>>>>>>>>>>>>>>>> TOOLS  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */
/*----------
 * doGetCoLnk
 *----------
 */
doGetCoLnk(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 0));
}
/*----------
 * doGetAss
 *----------
 */
doGetAss(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 1));
}
/*----------
 * doGetInc
 *----------
 */
doGetInc(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 2));
}
/*----------
 * doGetLib
 *----------
 */
doGetLib(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 3));
}
/*----------
 * doGetIncMkd
 *----------
 */
doGetIncMkd(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 4));
}

/*----------
 * doGetUsr
 *----------
 */
doGetUsr(mp,num)
struct OBJ *mp;
{
   int ret;
   ret = doGet(num, 5);
   strcpy(conf.u.c_p[4], conf.u.c_p[5]);

   /* la directory USR devient la directory courante */
   strcpy(cdirname, conf.u.c_p[5]);
   return(ret);
}

/*----------
 * doGetNmx
 *----------
 */
doGetNmx(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 6));
}




/*----------
 * doGet
 *----------
 */
doGet(num, off)
{
   int ret;

   ret = doEnter(confstr[num].str, conf.u.c_p[off], TRUE, FALSE);
   hide_cursor();
   if (strlen(conf.u.c_p[off])) {
        if (conf.u.c_p[off][strlen(conf.u.c_p[off])-1] == '\\')
                conf.u.c_p[off][strlen(conf.u.c_p[off])-1] = '\0';
   }
   if (ret)
           return(ret);
   return(ALT|ALT_I);
}

/*----------
 * doChgMkfile
 *----------
 */
doChgMkfile(mp,num)
struct OBJ *mp;
{
   unsigned ret;
   int  len;
   char work[80];

   strcpy(work, conf.u.pd.c_m);
   ret = doEnter(bltstr[num].str, work, FALSE, FALSE); /*conf.u.pd.c_m);*/
   if ((len = strlen(conf.u.pd.c_x)) &&  ((work[0] != '\\' && work[0] != '.') && work[1] != ':')) {
        strcpy(conf.u.pd.c_m, conf.u.pd.c_x);
        if (conf.u.pd.c_x[len-1] != '\\')
                strcat(conf.u.pd.c_m, "\\");
        strcat(conf.u.pd.c_m, work);
   }
   else
        strcpy(conf.u.pd.c_m, work);
   memset(vl->conf.projExe, 0, 80);


   hide_cursor();
   if (ret)
        return(ret);

   /* forcer la reconstruction de la base de references, puisque
    * le projet a ete change
    */
   conf.buildDB = TRUE;

   return(FUNC|F6);
}

/*----------
 * doInstHelp
 *----------
 */
doInstHelp(mp,num)
struct OBJ *mp;
{
   unsigned ret;

   ret = doEnter(confstr[num].str, conf.u.pd.c_h, TRUE, FALSE);
   hide_cursor();
       if (strlen(conf.u.pd.c_h))
           if (conf.u.pd.c_h[strlen(conf.u.pd.c_h) - 1] == '\\')
               conf.u.pd.c_h[strlen(conf.u.pd.c_h) - 1] = '\0';

   if (ret)
        return(ret);

   return(ALT|ALT_I);
}

/*----------
 * doLineCom
 *----------
 */
doLineCom(mp,num)
struct OBJ *mp;
{
   unsigned ret;

   ret = doEnter(runstr[num].str, vl->lineCom, FALSE, FALSE);
   hide_cursor();
   if (ret != 0)
       return(ret);
   return(ALT|ALT_R);
}

/*----------
 * doSconfig
 *----------
 */
doSconfig(mp,num)
struct OBJ *mp;
{
   int fdconf;
   if ((fdconf = open(configFile,O_RDWR|O_BINARY)) == -1)
        if ((fdconf = open(configFile,O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE)) == -1) {
                beep(1000);
                pushPop_MESS(confErrStr,ERRMSG);
                return(FUNC|F6);
        }
   strcpy(conf.projExe, vl->conf.projExe);
   conf.nb_car_per_line = nb_car_per_line;
   write(fdconf, &conf, sizeof(struct configuration));
   return(FUNC|F6);
}


/*----------
 * doEnter
 *----------
 */
unsigned doEnter(strcomm, str, driveLetter)
char *strcomm, *str;
{
   int rmode;
   unsigned Key;
   struct OBJ *wp, *ws, wsaisie;
   struct save_edit sed;

   if (fflag & FF_LINE_UPDATE)
           insert_lnbuf(pedwin, ERASE);

   /* positionner le curseur dans la zone file name */
   saveEdit(&sed);

   wp      = &rdialwin;  /* fenetre principale */
   ws      = &wsaisie;   /* fenetre de saisie  */

   initWin(ws, 0, nullstr, wp->ul_x + 2, wp->ul_y + 3,
           wp->ncol - 4, 1, TYP1, FALSE, FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock, W_SWIN);

   winPopUp(wp, W_OPEN, OBJ_CHAINAGE, TYP2);
   winPopUp(ws, W_OPEN, OBJ_FREE, TYP1);

   while (TRUE) {

        /* ecrire commentaire */
        print_at( wp, 1, 0, strcomm, OLDATT);
        print_at( wp, 0, 5, validYN, OLDATT);
        print_nb_at( ws, 0, 0, fillstr, ws->ncol - 1, OLDATT);
        wgotoxy(ws, strlen(str), 0);
        bigbuf    = clippbuf;
        strcpy(bigbuf, str);
        bigbuf[strlen(str)] = EOS;
        fsize     = strlen(str) + 1;
        fflag     = FF_INSERT;
        current   = topPage = 0;
        memset(linebuf, EOS, (nb_car_per_line - 1) * 2);
        print_nb_at( ws, 0, 0, str, strlen(str), OLDATT);
        Key = getfilename(ws, CLEAR);
        bigbuf[fsize - 1] = '\0';
        strncpy(str, bigbuf, 79);
        str[79] = '\0';
        if ((str[1] != ':') && (driveLetter) && (Key != (FUNC|F6)))
                pushPop_MESS(" You must specify a drive ", ERRMSG);
        else
                break;
   }

   winPopUp(ws, W_CLOSE);
   winPopUp(wp, W_CLOSE);

   restoreEdit(&sed);
   wgotoxy(pedwin, pedwin->curX, pedwin->curY);
   return(Key);
}

/*----------
 * doShowMess
 *----------
 */
doShowMess(mp)
struct OBJ *mp;
{

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   /* le fichier de message est toujours dans
    * le repertoire utilisateur
    */
   strcpy(file_name, conf.u.c_p[5]); /*cdirname);*/
   if (get_deepth(file_name) > 0)
           strcat(file_name, "\\");
   strcat(file_name, fError);
   if (loadFile(0, file_name, absfname, 0, V_CREAT) < 0)
       return(FUNC|F10);
   /* interdire de le replacer dans le pick */
   fflag |= FF_NO_GROUP;
   return(FUNC|F6); /*ed(pedwin));*/
}

/*----------
 * readfile
 *----------
 */
unsigned readfile(err, errflag)
int *err;
{
    char *readbuf;
    int  i, j, k, load;
    unsigned cumul;

    if ((readbuf = malloc(4096)) == NULL) {
        pushPop_MESS(Enomem, NMXMSG);
        exit(7);
    }

    *err = cumul = 0;
    while ((load = read(fdcurr,readbuf,4090)) > 0) {
        for (i = j = 0; j < load; j++, i++) {
                if ((bigbuf[cumul] = readbuf[j]) == TAB) {
                        strncpy(&bigbuf[cumul], fillstr, tablength - (k = i % tablength));
                        cumul += tablength - k - 1;
                        i += tablength - k - 1 ;/* -1 because l'incrementation de boucle */
                }
                else
                        if (bigbuf[cumul] == EOS) {
                                i = -1; /* because inc de boucle */
                                while (bigbuf[--cumul] == BLANK);
                                bigbuf[++cumul] = EOS;
                        }
                if (++cumul >=  BIGBUF_SIZE - 16) {
                        *err = TOO_BIG;
                        goto ENDLOAD;
                }
         }
     }
     if (load < 0)
        *err = NOT_OPEN;

ENDLOAD:
     if (errflag)
        *err = 0;
     free(readbuf);
     return(cumul);
}

