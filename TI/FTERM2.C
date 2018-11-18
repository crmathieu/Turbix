mlhjmljh fterm.c : fonctions terminales de tous les MENUS */
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doEdit -
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doEdit(mp)
struct OBJ *mp;
{
   extern struct OBJ *whereIsMouse();
   struct OBJ *ob;

   if (fflag & FF_HELP_MOD) { /* mode help edit */
        if (whereIsMouse(tailObj) == &edwin)
                  edHelp(HCLOSE);
        else
                  return(edHelp(HREOPEN));
   }
   return(ed(pedwin));
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doRun -
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doRun(mp)
struct OBJ *mp;
{
   unsigned i, ret;
   struct OBJ errwin;


   save_previous_file(absfname);
   push_pick(absfname);

   /* sauvegarder la configuration */
   memmove(&conf.reload[0], &picktab[0], PICKSIZE * 17);
   doSconfig(mp, 0);

        strcpy(vl->fError, fError);
        strcpy(vl->cdirname, cdirname);
        strcpy(vl->homedir, homedir);
        strcpy(vl->relname, relname);
        strcpy(vl->absfname, absfname);

        m_getPosAndButtonStatus(&vl->mouX, &vl->mouY, &i);

        vl->WINSYSink           = WINSYSink;
        vl->WINSYSpaper         = WINSYSpaper;
        vl->WINSYSborder        = WINSYSborder;
        vl->WINSYSblock         = WINSYSblock;
        vl->WINmessPaper        = WINmessPaper;
        vl->WINmessBorder       = WINmessBorder;
        vl->WINDIRink           = WINDIRink;
        vl->WINDIRpaper         = WINDIRpaper;
        vl->segVideo            = SEGvideo;
        vl->adapter             = adapter;
        vl->retCode             = 0;

        memcpy(&vl->conf, &conf, sizeof(struct configuration));

   /* demarrer le noyau :
    * 5 = code lancement
    * noyau dans resmod.exe
    */
   inhibe_mouse();
/*   showMouse();*/
   exit(5);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doQuit -
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doQuit(mp)
struct OBJ *mp;
{
   unsigned ret;
   exit_ulk(mp);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * exit_ulk -
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
exit_ulk(mp)
struct OBJ *mp;
{
   extern char Question[], Savefile[];
   extern unsigned long DosInt24, BiosIntclk, BiosInt33;
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

   /* replacer gestion Dos de l'int24 */
   _dos_setvect(0x24, DosInt24);

   exit(7);     /* code de sortie dans resmod.exe */
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * getYesNo - reponse  U.S. et FR.
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
getYesNo(pob)
struct OBJ *pob;
{
        int i, ChL;
        int ret;

/*      while (1) {
                ChL = (getkey() & 0x00ff);
                for (i=0; i<4; i++)
                        if (ChL == y_o[country][i])
                                /* retourner le type U.S. */
/*                              return(y_o[0][i]);
                beep(1000);
        }*/
        Yes_answ = TRUE;
        No_answ  = FALSE;
        print_at(pob, FIELD(pob, 0)->rX, FIELD(pob, 0)->rY, "X", OLDATT);
        ret = get_answer(pob, &i);
        if (Yes_answ)
                return((int)'y');
        return((int)'n');
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * inhibe_mouse
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
inhibe_mouse()
{
   /* replacer les anciens vecteurs et cacher la souris */
   _cli();
   if (mouse_present) {
        m_callMaskAndAddress(0, 0L);
        _dos_setvect(0x1c, BiosIntclk);
   }
   else
       _dos_setvect(0x33, BiosInt33);
   _sti();
   hideMouse();
}

/*  >>>>>>>>>>>>>>>>>>>>>>> EDIT  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doFindStr
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doFindstr(mp, num)
struct OBJ *mp;
{
   ctrlqf(pedwin, FIND);
   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doFindnext
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doFindnext(mp, num)
struct OBJ *mp;
{
   ctrll(pedwin);
   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doRepl
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doRepl(mp, num)
struct OBJ *mp;
{
   ctrlqa(pedwin);
   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * doGoto
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doGoto(mp, num)
struct OBJ *mp;
{
   ctrlqg(pedwin, num);
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
   ret = doEnter(work2, work);
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
   /* retourner au help ou … l'editeur */
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
   /* retourner au help ou … l'editeur */
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
   /* retourner au help ou … l'editeur */
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
   /* retourner au help ou … l'editeur */
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
   /* retourner au help ou … l'editeur */
   return(FUNC|F6);
}




/*  >>>>>>>>>>>>>>>>>>>>>>> ENV  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*ÄÄÄÄÄÄÄÄÄÄ
 * doKbSpeed
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doKbSpeed(mp, fast)
struct OBJ *mp;
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

/*ÄÄÄÄÄÄÄÄÄÄ
 * doLoad
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doLoad(mp)
struct OBJ *mp;
{
   int  Otype;
   unsigned ret;

   if ((ret = display_dir(&Otype)) != 0)
        return(ret);

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   if (loadFile(Otype, file_name, absfname, WINMSG, V_CREAT) < 0)
             return(FUNC|F6);

   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * doLoadlast - charge le fichier precedent
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
   else  /* " Load ...  " selectionn‚ */
       return(FUNC|F3);

   if (loadFile(picktab[0].fflag, file_name, absfname, WINMSG, V_CREAT) < 0)
       return(FUNC|F10);
/*   return(ed(pedwin));*/
   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * doSaveFile - sauvegarder le fichier courant
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * loadFile - en entree, file_name contient le nom du fichier selectionn‚
 *            par "F3" et absfname contient le nom du fichier pr‚c‚dent
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
loadFile(mode, nextf, prevf, msgflag, o_mode)
char *nextf, *prevf;
{

    extern int curr_floppy;
    extern char Verify[], Unkfile[], Press[];
    extern char *errno_tab[];
    extern char comptab[3][80], compStr[], assStr[];
    extern char Question[];
    extern char *glta();
    unsigned readfile();
    struct pick pck;
    int        testfflag, retcheck, errflag,
               exterr, i, drive, ChL, Key, err;
    unsigned   cumul, size, load;
    char       *pfile, sdrive[2];
    struct OBJ errwin;
    struct stat sbuf;

    if (save_previous_file(prevf) < 0)
        return(-1);

   /*  controler gestion unit‚s logiques */
   vflopp(nextf);

   /* initialiser les valeurs de l'editeur en valeurs par defaut */
   pck = pckdefault;

   /* initialiser le fichier a compiler */
    strcpy(work, nextf);
    pfile = glta(work); /* nom du fichier dans pfile */
    if (stricmp(cdirname, work) == 0)
        strcpy(relname, pfile);   /* fichier dans directorie courante */
    else
        strcpy(relname, nextf); /* fichier dans une autre directorie */

/*    memset(comptab[0], '\0', 80);
    memset(comptab[1], '\0', 80);
    strcpy(comptab[0], compStr);
    strcat(comptab[0], relname);
    strcpy(comptab[1], assStr);
    strcat(comptab[1], relname);
    update_menu(&compMenu);*/

   /* si fichier deja en PICK TABLE, recuperer sa position */
   if ((retcheck = check_pick(nextf)) != -1) {
        pck = pop_pick(retcheck);
        testfflag = pck.fflag;
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
       init_ed_from_pick(&pck, 0);

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
             /* est on autoris‚ … charger un nouveau fichier ? */
             if (o_mode == V_CREAT) {
                testfflag |= FF_NEWFILE;
                memset(&pck, 0, PICKSIZE);
                pck.current_line_no = 1;
                pck.topPage_line_no = 1;
                goto Newfile;
             }
             else {
                 winPopUp(pedwin, W_CLOSE);       /**/
                 pushPop_MESS(helpErrStr,ERRMSG);
                 return(-1);
             }
        }
        else {
             /* l'utilisateur a tap‚ l'option ABORT
              * si le fichier vient de la PICK TABLE, le reinserer
              */
             if (retcheck != -1)
                               push_pick(nextf);
             if ((retcheck = check_pick(prevf)) != -1)
                 pck = pop_pick(retcheck);
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
       if (ChL == 'N' || ChL == 'n') {
             if ((retcheck = check_pick(prevf)) != -1)
                 pck = pop_pick(retcheck);
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
         /* l'utilisateur a tap‚ l'option ABORT
          * si le fichier vient de la PICK TABLE, le reinserer
          */
         if (retcheck != -1)
             push_pick(nextf);
         if ((retcheck = check_pick(prevf)) != -1)
              pck = pop_pick(retcheck);
         close(fdcurr);
         return(-1);
    }
/*    if (msgflag == NOMSG)
        if ((retcheck = check_pick(prevf)) != -1)
             pck = pop_pick(retcheck);*/

    strcpy(prevf, nextf);

    init_ed_from_pick(&pck, cumul);

    /* decompression (gestion des tabulations) */
/*    decompress();*/

    close(fdcurr);
    return(0);
}

/*  >>>>>>>>>>>>>>>>>>>>>>> PICK  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */

/*ÄÄÄÄÄÄÄÄÄÄ
 * doPick
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doPick(mp)
struct OBJ *mp;
{
   extern char pickload[];
   extern char *glta();
   char work[80];
   int ret_ed;



   if (stricmp(picktab[mp->curY].str, pickload) != 0) {
       strcpy(file_name, &picktab[mp->curY].str[1]); /* shunter le 1er blanc */
       file_name[strlen(file_name) - 1] = '\0';
   }
   else  /* " Load ...  " selectionn‚ */
       return(FUNC|F3);

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   if (loadFile(picktab[mp->curY].fflag, file_name, absfname, 0, V_CREAT) < 0)
       return(FUNC|F10);

   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * init_ed_from_pick - initialiser les variables de travail de l'editeur
 *                     pour un fichier a creer
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
init_ed_from_pick(pck, size)
struct pick *pck;
unsigned size;
{
    unsigned u;

    /* rajouter DELIMITEUR de fin de fichier */
    bigbuf[size] = EOFM;
    fsize        = size + 1;

    /* initialiser le pick defaut */
/*    if (size > 0 && ((u = get_line_no(pck->current)) != pck->current_line_no)) {/* unmatch */
/*        gotoxy(0,10);
        init_edVar(1);
    }*/
    if (fsize < pck->fsize)
        init_edVar(1);
    else {
        current         = pck->current;
        topPage         = pck->topPage;
        topBlock        = pck->topBlock;
        bottomBlock     = pck->bottomBlock;
        current_line_no = pck->current_line_no;
        topPage_line_no = pck->topPage_line_no;
        pedwin->leftCh    = pck->leftCh;
        pedwin->curX      = pck->curX;
        pedwin->curY      = pck->curY;
        fflag          |= pck->fflag;
        write_status(pedwin);
    }
    wgotoxy(pedwin, pedwin->curX, pedwin->curY);
    bottom = getlast_ln(pedwin);
/*    write_wbox(pedwin, TYP2);   /* rendre fenetre editeur active */
    set_win(pedwin);
    bottom_line_no = get_line_no(fsize);  /* LONG !! */
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * save_previous_file - gestion sauvegarde d'un fichier
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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
            init_edVar(1);
   }
   return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * doSave - sauvegarder un fichier (retourne 0 si tout est OK)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
doSave(file)
char *file;
{
   int ret, reset, n;
   struct OBJ errwin;
   int fd;
   extern char *errno_tab[];

   strupr(file);

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
       close(fd);
       pop_MESS();
       pushPop_MESS(errno_tab[errno], ERRMSG);
       return(-1);
   }
   close(fd);
   pop_MESS();
   return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * P_bak - creer le nom de fichier en ".BAK"
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

/*ÄÄÄÄÄÄÄÄ
 * push_SL
 *ÄÄÄÄÄÄÄÄ
 */
push_SL2(str)
char *str;
{
    _wstringDB( 0, 24, 80, NEWATT, MESS24att, str);
}
/*ÄÄÄÄÄÄÄÄ
 * pushPop_MESS
 *ÄÄÄÄÄÄÄÄ
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


/*ÄÄÄÄÄÄÄÄ
 * push_MESS
 *ÄÄÄÄÄÄÄÄ
 */
push_MESS(str, errwin)
char *str;
struct OBJ *errwin;
{
   strcpy(work, str);
   verify_window(errwin, work, nullstr, NO_REP_REQ, NMXMSG);
}

/*ÄÄÄÄÄÄÄÄ
 * pop_MESS
 *ÄÄÄÄÄÄÄÄ
 */
pop_MESS()
{
   winPopUp(messwin, W_CLOSE);
}

/*ÄÄÄÄÄÄÄÄ
 * pop_SL
 *ÄÄÄÄÄÄÄÄ
 */
pop_SL2()
{
    _wstringDB(0, 24, 80, NEWATT, MENUink|MENUpaper, fillstr);
    write_status(pedwin);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * saveEdit - sauvegarde les parametres Globaux de positionnement
 *ÄÄÄÄÄÄÄÄÄÄ
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreEdit - restaure les parametres Globaux de positionnement
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

    formatt_line(linebuf, bigbuf, current, lnlen(pedwin,current), EOS, NB_CAR_PER_LINE * 2);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doChgDir
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doChgDir(mp,num)
struct OBJ *mp;
{
   char   work[80], aux[80];
   unsigned ret;

   strupr(homedir);
   strupr(cdirname);

   strcpy(work, homedir);
   ret = doEnter(chgDirStr, work);
   hide_cursor();
   if (ret)
        return(ret);

   strupr(work);
   if (work[1] != ':') {
        strcpy(aux, work);
        memset(work, '\0', 79);
        work[0] = cdirname[0];
        work[1] = ':';
        if (aux[0] != '\\')
                work[2] = '\\';
        strcat(work, aux);
   }
   /* tester si chgt de drive demand‚ et si oui, changer ! */
   if (work[0] != cdirname[0])
        if (doChdDrive(work))
                return(FUNC|F6);

   pushPop_MESS("CA REPART DANS CHGDIR", ERRMSG);

   if (chdir(work)) {
        if (dosexterr(NULL) != INT24FAIL)
                pushPop_MESS(errno_tab[errno], ERRMSG);
   }
   else {
        strcpy(homedir, work);
        strcpy(cdirname, work);
   }
   return(FUNC|F6);
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doChgDrive
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doChdDrive2(str)
char *str;
{
        int drive, sav_drive, wk_drive, sav_floppy, ret, Chg, test;


                drive = str[0] - 'a';
                _dos_getdrive(&sav_drive);

                /* si floppy, controler gestion unit‚s logiques */
                if (drive <= 1) {
                        sav_floppy  = curr_floppy;
                        curr_floppy = drive;
                        verify_floppy();
                }
                _dos_setdrive(drive + 1, &wk_drive);
                _dos_getdrive(&test);

                /* si abort sur l'operation, revenir au floppy precedent */
                if ((ret = dosexterr(NULL)) == INT24FAIL)
                        set_logic_floppy(sav_floppy);

                if (test != drive + 1) {
                        pushPop_MESS(badDrvStr/*chgDriveStr*/, ERRMSG);
                        return(-1);
                }
                return(0);

}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doChgDrive
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doChdDrive(str)
char *str;
{
        int drive, sav_drive, wk_drive, sav_floppy, ret, Chg, test;


                strupr(str);
                drive = str[0] - 'A';

                /* si floppy, controler gestion unit‚s logiques */
                if (drive <= 1) {
                        sav_floppy  = curr_floppy;
                        curr_floppy = drive;
                        verify_floppy();
                }

                sav_drive = _getdefaultD();
                if (checkIO(drive, sav_drive, sav_floppy))
                        return(-1);

                wk_drive = _setdefaultD(drive);
                if (checkIO(drive, sav_drive, sav_floppy))
                        return(-1);

                /* si abort sur l'operation, revenir au floppy precedent */
/*                if ((ret = dosexterr(NULL)) == INT24FAIL) {
                        if (drive <= 1)
                                set_logic_floppy(sav_floppy);

                        /* replacer l'ancien drive en "courant" */
/*                        pushPop_MESS("ON revient au drive courant", ERRMSG);
                        wk_drive = _setdefaultD(sav_drive);
                        return(-1);
                }*/
                test = _getdefaultD();
                if (test != drive) {
                        pushPop_MESS(badDrvStr/*chgDriveStr*/, ERRMSG);
                        _setdefaultD(sav_drive);
                        return(-1);
                }
                pushPop_MESS("ON sort normalement de chgDRIVE", ERRMSG);
                return(0);

}
checkIO(newDrive, oldDrive, flop )
{

    sprintf(work, " CURR Drive = %d ", oldDrive);
    pushPop_MESS(work, ERRMSG);
    sprintf(work, " NEW Drive = %d ", newDrive);
    pushPop_MESS(work, ERRMSG);

    /* si abort sur l'operation d'E/S */
    if (dosexterr(NULL) == INT24FAIL) {
        if (newDrive <= 1)
                set_logic_floppy(flop);

        /* replacer l'ancien drive en "courant" */
        pushPop_MESS("ON revient au drive courant", ERRMSG);
        _setdefaultD(oldDrive);
        return(-1);
    }
    return(0);
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doSaveAs
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doSaveAs(mp,num)
struct OBJ *mp;
{
   char work[80], w2[80];
   int drive, sav_drive, wk_drive, sav_floppy, Chg, test;
   unsigned ret;

   work[0] = '\0';
   ret = doEnter(saveAsStr1, work);
   if (strlen(work) == 0)
        return(FUNC|F6);

   hide_cursor();
   if (ret)
        return(ret);
   Chg = FALSE;

   strupr(work);
   if (work[1] == ':') { /* changer de drive */
                Chg = TRUE;
                drive = work[0] - 'A';
                sav_drive = _getdefaultD();

                /* si floppy, controler gestion unit‚s logiques */
                if (drive <= 1) {
                        sav_floppy  = curr_floppy;
                        curr_floppy = drive;
                        verify_floppy();
                }
                wk_drive = _setdefaultD(drive);
                test = _getdefaultD();
                if (test != drive + 1) {
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
                wk_drive = _setdefaultD(sav_drive);
        }
        else
                if (doSave(w2) == 0)
                        fflag &= ~FF_DIRTY;
        return(ret ? ret : FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doShell
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doShell(mp,num)
struct OBJ *mp;
{
   int ret;
   _save_screen(0, 0, 79, 24, videoBuf);
   hideMouse();
/*   getcwd(work, 79);*/
   saveCtx(mp);
   exit(4);
/*   if (ret = spawnlp(P_WAIT, "command.com", NULL))
        pushPop_MESS(errno_tab[errno], ERRMSG);
   chdir(work);
   _refresh(0, 0, 79, 24, videoBuf);
   showMouse();
   hide_cursor();
   return(FUNC|F6);*/
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * saveCtx
 *ÄÄÄÄÄÄÄÄÄÄ
 */
saveCtx(mp)
struct OBJ *mp;
{
        save_previous_file(absfname);
        push_pick(absfname);

        /* sauvegarder la configuration */
        memmove(&conf.reload[0], &picktab[0], PICKSIZE * 17);
        doSconfig(mp, 0);
        memcpy(&vl->conf, &conf, sizeof(struct configuration));

        /* liberer BIGBUF */
        free(bigbuf);

        /* inhiber la souris */
        inhibe_mouse();
/*        showMouse();*/
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doChgPaste
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doChgPaste(mp,num)
struct OBJ *mp;
{
   char work[80];
   unsigned ret;

   ret = doEnter(chgPaste, conf.u.pd.c_p);
   hide_cursor();
   if (ret)
        return(ret);

   strcpy(pastefile, conf.u.pd.c_p);
   return(FUNC|F6);
}

/*  >>>>>>>>>>>>>>>>>>>>>>> TOOLS  MENU <<<<<<<<<<<<<<<<<<<<<<<<< */
/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetCoLnk
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetCoLnk(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 0));
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetAss
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetAss(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 1));
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetInc
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetInc(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 2));
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetLib
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetLib(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 3));
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetIncMkd
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetIncMkd(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 4));
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetUsr
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetUsr(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 5));
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doGetNmx
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGetNmx(mp,num)
struct OBJ *mp;
{
   return(doGet(num, 6));
}




/*ÄÄÄÄÄÄÄÄÄÄ
 * doGet
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGet(num, off)
{
   int ret;

       ret = doEnter(confstr[num].str, conf.u.c_p[off]);
       hide_cursor();
       if (strlen(conf.u.c_p[off]))
           if (conf.u.c_p[off][strlen(conf.u.c_p[off])-1] == '\\')
               conf.u.c_p[off][strlen(conf.u.c_p[off])-1] = '\0';
       if (ret)
           return(ret);
       return(ALT|ALT_I);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doChgMkfile
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doChgMkfile(mp,num)
struct OBJ *mp;
{
   unsigned ret;
   int  len;
   char work[80];

   strcpy(work, conf.u.pd.c_m);
   ret = doEnter(bltstr[num].str, work); /*conf.u.pd.c_m);*/
   if ((len = strlen(conf.u.pd.c_x)) &&  ((work[0] != '\\' && work[0] != '.') && work[1] != ':')) {
        strcpy(conf.u.pd.c_m, conf.u.pd.c_x);
        if (conf.u.pd.c_x[len-1] != '\\')
                strcat(conf.u.pd.c_m, "\\");
        strcat(conf.u.pd.c_m, work);
   }
   else
        strcpy(conf.u.pd.c_m, work);
   hide_cursor();
   if (ret)
        return(ret);

   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doInstHelp
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doInstHelp(mp,num)
struct OBJ *mp;
{
   unsigned ret;

   ret = doEnter(confstr[num].str, conf.u.pd.c_h);
   hide_cursor();
       if (strlen(conf.u.pd.c_h))
           if (conf.u.pd.c_h[strlen(conf.u.pd.c_h) - 1] == '\\')
               conf.u.pd.c_h[strlen(conf.u.pd.c_h) - 1] = '\0';

   if (ret)
        return(ret);

   return(ALT|ALT_I);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doLineCom
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doLineCom(mp,num)
struct OBJ *mp;
{
   unsigned ret;

   memset(vl->lineCom, '\0', 80);
   ret = doEnter(optstr[num].str, vl->lineCom);
   hide_cursor();
   if (ret != 0)
       return(ret);
   return(ALT|ALT_R);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doSconfig
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doSconfig(mp,num)
struct OBJ *mp;
{
   int fdconf;
   if ((fdconf = open(configFile,O_RDWR|O_BINARY)) == -1)
        if ((fdconf = open(configFile,O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE)) == -1) {
/*        if ((fdconf = creat(configFile,S_IREAD|S_IWRITE)) == -1)*/
                beep(1000);
                getch();
        }
   write(fdconf, &conf, sizeof(struct configuration));
   return(FUNC|F6);
}


/*ÄÄÄÄÄÄÄÄÄÄ
 * doEnter
 *ÄÄÄÄÄÄÄÄÄÄ
 */
unsigned doEnter(strcomm, str)
char *strcomm, *str;
{
   int rmode;
   unsigned Key;
   struct OBJ *wp, *ws, wsaisie;
   struct save_edit sed;

   wp      = &rdialwin;  /* fenetre principale */
   ws      = &wsaisie;   /* fenetre de saisie  */

   winPopUp(wp, W_OPEN, OBJ_CHAINAGE, TYP2);
   initWin(ws, 0, nullstr, wp->ul_x + 2, wp->ul_y + 3,
           wp->ncol - 4, 1, TYP1, FALSE, FALSE, FALSE, WINSYSink, WINSYSpaper, WINSYSborder, WINSYSblock, W_SWIN);

   winPopUp(ws, W_OPEN, OBJ_FREE, TYP1);


   /* positionner le curseur dans la zone file name */
   saveEdit(&sed);

   /* ecrire commentaire */
   print_at( wp, 1, 0, strcomm, OLDATT);
   print_at( wp, 0, 5, validYN, OLDATT);


   print_nb_at( ws, 0, 0, fillstr, ws->ncol - 1, OLDATT);
   wgotoxy(ws, 0, 0);
   bigbuf    = clippbuf;
   strcpy(bigbuf, str);
   fsize     = strlen(bigbuf)+1;
   bigbuf[fsize - 1] = EOS;
   fflag     = FF_INSERT;
   current   = topPage = 0;
   memset(linebuf, EOS, (NB_CAR_PER_LINE - 1) * 2);
   print_nb_at( ws, 0, 0, str, strlen(str), OLDATT);
   Key = getfilename(ws, CLEAR);
   bigbuf[fsize - 1] = '\0';
   strncpy(str, bigbuf, 79);
   str[79] = '\0';
   restoreEdit(&sed);
   winPopUp(ws, W_CLOSE);
   winPopUp(wp, W_CLOSE);
   wgotoxy(pedwin, pedwin->curX, pedwin->curY);
   return(Key);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doShowMess
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doShowMess(mp)
struct OBJ *mp;
{

   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   strcpy(file_name, cdirname);
   if (get_deepth(file_name) > 0)
           strcat(file_name, "\\");
   strcat(file_name, fError);
   if (loadFile(0, file_name, absfname, 0, V_CREAT) < 0)
       return(FUNC|F10);
   return(FUNC|F6); /*ed(pedwin));*/
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * readfile
 *ÄÄÄÄÄÄÄÄÄÄ
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
    while ((load = ead(fdcurr,readbuf,4090)) > 0) {
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