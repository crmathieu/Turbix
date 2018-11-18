/* MKD_REF */

#include "xed.h"
#include "emsg.h"

#define  M_LEFT_PRESS      2
#define  M_LEFT_RELEASE    4
#define  M_RIGHT_PRESS     8
#define  M_RIGHT_RELEASE  16

/* type de fonction dans les .REF */
#define TYPE_NUL 0      /* rien */
#define TYPE_DEF 1      /* Definition */
#define TYPE_EXT 2      /* Externe */
#define TYPE_DEC 3      /* Declaration */


#ifdef TURBO
#include <fcntl.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#define _dos_setvect setvect
#define _dos_getvect getvect
#endif

/* RECORD dans le fichier des REFDEF */
struct refDef {
         char     cflag;       /* C ou Ass */
         int      position;    /* numero de ligne ou est definie la func */
         char     funcName[32];/* nom de la fonction */
unsigned char     type;        /* type : EXT, DEF, DEC */
};

/* RECORD dans le fichier des modules */
struct module {
        char modname[64];
};

/* RECORD dans le fichier des DEF */
struct def {
         char     cflag;       /* type de fonct: "C" ou "A"ssembleur */
         int      position;    /* numero de ligne ou est definie la func */
         char     function[32];/* nom de la fonction */
unsigned char     type;        /* EXT ou DEF */
         int      module;      /* numero du module dans le module file */
         int      cpt;         /* # de REF externes dans la zone externe */
unsigned char    *offRef;      /* offset dans le fichier des REF pour les
                                * modules qui utilisent cette fonction
                                */
};

/* LISTE CHAINEE de travail */
struct LLIST {
        struct def    def;
        struct LLIST *next,
                     *prev;
};

struct LLIST lhead, ltail;
#define NULLLIST (struct LLIST *)0


/* handles des fichiers de la base
 * -------------------------------
 */
int Hmod;       /* handle fichier des nom de module */
int Hdecl;      /* handle fichier des declarations */
int Hext;       /* handle fichier des ref externes */

struct OBJ *messwin;
int Xmess, Ymess;
#define nullstr ""
unsigned SEGvideo;
int adapter;

#define ERRMSG 4
#define NMXMSG 3

#define HBYTE  0xff00
#define LBYTE  0x00ff
#define FIND   1


#define MKE  0xf100
#define COMP 0xf200
#define LNK  0xf300
#define ASS  0xf400
#define BRW  0xf500

#define BERR        1
#define MKE_NODIFF  2
#define BEXIT       3

/*--------------------
 *   Browser utility
 *--------------------
 */

#define EOC_MISSING   6
#define FIN_RELATION  8
#define DEB_RELATION  7
#define FIN_LIGNE     9
#define SCAN_LIGNE    5
#define FIN_MAKE     10

#define TRUE          1
#define FALSE         0

#define CTYPE         1         /* type C      */
#define ATYPE         2         /* type ASM    */
#define LTYPE         3         /* type LINK   */
#define HTYPE         4         /* type HEADER */
#define MTYPE         5         /* type MACRO  */
#define DTYPE         6         /* type REFERENCE */
#define ETYPE        -5         /* type ERREUR */

#define MAXMACRO     10
#define SAVE          6


/* gestion des MACROS */
char  MACROname[MAXMACRO][12];
long  MACROdate[MAXMACRO];
int   MACROfree;             /* 1er slot macro libre */
long  mdate;

/*  gestion des parametres lus dans le
 *  fichier de configuration "mdi.pro"
 */

struct configuration conf;

/* buffers de travail */

char *bufline;       /* buffer entree */
char *buftoken;      /* buffer ou sont stockes les tokens */
char *argf[100];         /* tableau de pointeurs sur tokens   */
char argfA[50][80];      /* tableau de token Absolus */
char refFile[100][14];   /* tableau des fichiers de reference */
char OrefFile[100][64];  /* tableau parallele des fichiers d'origine */
int  iref;

/* variables globales de travail */
char *tokenptr;
int arglc,narg;
int line,relativline;
int comptyp;

long filarg0,filargi;
int command;

int analyse,errorlevel;
struct stat entry;

struct OBJ *wm, wmake;
int fmsg, filout, browserError, difference;

int tmp_file;

char work[80];
char wkfile[80];


struct varLink *vl;


unsigned char boxtype[7][11] = {{'Ú','¿','À','Ù','³','Ä','Ã','´','Â','Á','Å'},   /* type 1 */
                                {'Õ','¸','Ô','¾','³','Í','Æ','µ','Ñ','Ï','Ø'},   /* type 2 */
                                {'Ö','·','Ó','½','º','Ä','Ç','¶','Ò','Ğ','×'},   /* type 3 */
                                {'É','»','È','¼','º','Í','Ì','¹','Ë','Ê','Î'},   /* type 4 */
                                {'°','°','°','°','°','°','°','°','°','°','°'},   /* type 5 */
                                {'Ç','¶','Ó','½','º','Ä','Ç','¶','Ò','Ğ','×'},   /* status */
                                {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},   /* external windows */
};

#define TYP1    0
#define TYP2    1
#define TYP3    2
#define TYP4    3
#define TYP5    4
#define TYP6    5
#define TYP7    6

unsigned long BiosInt10;
char *ptr2;

/********* SOURIS *********/
int mouse_present = 0;
unsigned long BiosInt33;
int mouseEvent = 0;


extern int prolog_it_mouse();

char c_cl[80];   /* comp & linker dir   */
char c_a[80];    /* assembleur dir      */
char c_i[80];    /* include dir         */
char c_z[80];    /* MKD include dir     */
char c_l[80];    /* librairies dir      */
char c_u[80];    /* ulk dir             */
char c_m[80];    /* make file name      */
char c_p[80];    /* paste file name     */
char c_h[80];    /* help dir name       */
char c_x[80];    /* user dir name       */
char auxStr[80];
int mouX, mouY;

#define WBRWSIZE1  32   /* largeur */
#define WBRWSIZE2  48
#define WBRWH       7   /* hauteur */
int wbrwsize;

/* offset */
#define OFF_CHECK  1 /*1*/
#define OFF_COMP   1 /*2*/
#define OFF_ASS    1 /*3*/
#define OFF_LINK   1 /*4*/

#define OFF_DEF  3
#define OFF_EXT  4
int offset;

/* variables de nombres */
int nDef, nExt;

/* macro */
#define INF(x,y)  (x < y ? x : y)

int nb_car_per_line;
char *TOKbuffer;

char *_borlfunc[] = {
"__chmod","__close","__creat","__exit","__fgetc","__fputc","__open","__read",
"__strerror","__write","_abort","_absread","_abswrite","_access","_allocmem",
"_asctime","_atexit","_atexit_t","_bdos","_bdosptr","_bioscom","_biosdisk",
"_biosequip","_bioskey","_biosmemory","_biosprint","_biostime","_brk","_calloc",
"_cgets","_chdir","_chmod","_chsize","_clearerr","_clock","_close","_clreol",
"_clrscr","_coreleft","_country","_cprintf","_cputs","_creat","_creatnew",
"_creattemp","_cscanf","_ctrlbrk","_delay","_delline","_disable","_dosexterr",
"_dostounix","_dup","_dup2","_emit__","_enable","_eof","_execl","_execle","_execlp",
"_execlpe","_execv","_execve","_execvp","_execvpe","_exit","_farcalloc","_farcoreleft",
"_farfree","_farmalloc","_farrealloc","_fclose","_fcloseall","_fdopen","_fflush",
"_fgetc","_fgetchar","_fgetpos","_fgets","_filelength","_findfirst","_findnext",
"_flushall","_fnmerge","_fnsplit","_fopen","_fprintf","_fputc","_fputchar","_fputs",
"_fread","_free","_freemem","_freopen","_fscanf","_fseek","_fsetpos","_ftell","_fwrite",
"_getcbrk","_getch","_getche","_getcurdir","_getcwd","_getdate","_getdfree","_getdisk",
"_getenv","_getfat","_getfatd","_getftime","_getpass","_getpsp","_gets","_getswitchar",
"_gettext","_gettextinfo","_gettime","_getvect","_getverify","_getw","_gmtime",
"_gotoxy","_harderr","_hardresume","_hardretn","_inport","_inportb","_insline",
"_int86","_int86x","_intdos","_intdosx","_intr","_ioctl","_isatty","_kbhit","_keep",
"_localtime","_lock","_longjmp","_lowvideo","_lseek","_malloc","_mkdir","_mktemp",
"_normvideo","_nosound","_open","_outport","_outportb","_parsfnm","_peek","_peekb",
"_perror","_poke","_pokeb","_printf","_putch","_puts","_puttext","_putw","_raise",
"_randbrd","_randbwr","_read","_realloc","_rename","_rewind","_rmdir","_sbrk","_scanf",
"_searchpath","_segread","_setblock","_setbuf","_setcbrk","_setdate","_setdisk",
"_setftime","_setjmp","_setmode","_setswitchar","_settime","_setvbuf","_setvect",
"_setverify","_signal","_sleep","_sound","_spawnl","_spawnle","_spawnlp","_spawnlpe",
"_spawnv","_spawnve","_spawnvp","_spawnvpe","_sprintf","_sscanf","_stime","_strerror",
"_system","_tell","_textattr","_textbackground","_textcolor","_time","_tmpfile",
"_tmpnam","_tzset","_umask","_ungetc","_ungetch","_unixtodos","_unlink","_unlock",
"_vfprintf","_vfscanf","_vprintf","_vscanf","_vsprintf","_vsscanf","_wherex",
"_wherey","_write",""
};
/*-------------------------------------------------------
 *      M       M      MM      M   N     N
 *      M M   M M     M  M     M   N N   N
 *      M   M   M    M MM M    M   N   N N
 *      M       M   M      M   M   N     N
 *-------------------------------------------------------
 */
main()
{
   void interrupt Dummy_interrupt();

   char c, work2[80];
   FILE  *fp,*fpred;
   int    ret, fdconf, difference, moduleBuild,
          checkfile, working,
          k, typ, i, err;

   struct OBJ errwin, *wmm, msgwin;
   long getMDATE();
   unsigned Key;

   /* init index de stockage dans le tableau des fichiers de references */
   iref = 0;

   /* recuperer l'@ du vecteur de communication */
   vl = (struct varLink *)_dos_getvect(0xf2);

   /* init video mode */
   SEGvideo = vl->segVideo;
   if (SEGvideo == 0xb000)
        adapter = 0;
   else
        adapter = 1;


   /* recuperer le vecteur BIOS 10h */
   BiosInt10 = (unsigned long)_dos_getvect(0x10);

   /* inhiber le ^C */
   _dos_setvect(0x23, Dummy_interrupt);

   i            = nDef = nExt = 0;
   browserError    = difference = FALSE;
   wm           = &wmake;
   wmm          = &msgwin;

   ptr2 = refStr;


   /* dupliquer l'environnement */
   dupEnviron();

   initBuffer();
   memset(buftoken, '\0', nb_car_per_line);

   MACROfree  = 0;
   errorlevel = 0;
   line = 0;

   if ((c = vl->conf.u.pd.c_m[0]) == BLANK || c == '\0') {
      strcpy(work, noPrjStr);
      pushPop_MESS(work, ERRMSG);
      refExit(wm, BRW|BEXIT, 0);
   }
   /* ouvrir le fichier de projet */
   if ((fp = fopen(vl->conf.u.pd.c_m,"r")) == NULL) {
      strcpy(work, noPrjFileStr);
      pushPop_MESS(work, ERRMSG);
      refExit(wm, BRW|BEXIT, 0);
   }
   /* dupliquer directorie utilisateur */
   if (strlen(vl->conf.u.pd.c_x)) {
        strcpy(auxStr, vl->conf.u.pd.c_x);
        if (auxStr[strlen(auxStr)-1] != '\\')
            strcat(auxStr, "\\");
   }
   else {
         strcpy(work, noUsrStr);
         affiche(work, ERRMSG);
         enable_screen();
         return(0);
   }

   flush_kbd();
   /*                x  y   #c  #l */
   initWin(wm, ptr2, (80-WBRWSIZE1)/2, 10, wbrwsize = WBRWSIZE1, 3, TRUE, FALSE,
           vl->WINSYSink, vl->WINSYSpaper, vl->WINSYSborder, vl->WINSYSblock, W_WIN);

   winPopUp(wm, W_OPEN, TYP1);

   offset = strlen(checkString)+1;
   print_at(wm, 1, OFF_CHECK, checkString, OLDATT);
   while  (TRUE)  {
           /* gestion arret du MAKE */
           if (_look_kbd()) { /* caractere tap‚ au clavier */
             Key = _read_kbd();
             if ((Key & 0xff) == ESC) {  /* Stop !!! */
                winPopUp(wm, W_CLOSE);
                strcpy(work, breakRef);
                verify_window(wm, work, nullstr, ERRMSG);
                getkey();
                refExit(wm, BRW|BEXIT, 0);
             }
           }

     arglc    = 0;
     relativline = 0;
     tokenptr = buftoken;
     analyse = DEB_RELATION;
     memset(buftoken, '\0', nb_car_per_line);

     /* saisir une relation */
     while (analyse != FIN_RELATION) {
        line++;
        getline(fp);                        /* lire une ligne */
        switch(analyse = putargf(&err)) {
        case EOC_MISSING : /* accumuler tokens */
                           sprintf(work, syntErr1, line);
                           pushPop_MESS(work, ERRMSG);
                           refExit(wm, BRW|BEXIT, 0);

        case FIN_MAKE    : if (err) {
                                sprintf(work, syntErr2, line);
                                pushPop_MESS(work, ERRMSG);
                                refExit(wm, BRW|BEXIT, 0);
                           }
                           goto ExitBrowser;
        }
        memset(bufline, '\0', nb_car_per_line);
     }

     argf[arglc+1] = NULL;    /* marquer la fin des tokens */

     /* verifier que la syntaxe est correcte */
     if (*argf[1] != ':') {      /* separateur ':' obligatoire */
        sprintf(work, syntErr3, line);
        pushPop_MESS(work, ERRMSG);
        refExit(wm, BRW|BEXIT, 0);
     }

     /* examiner les dates de creation des differents fichiers */
     checkfile = 2;
     if (MakeAbsName(arglc))
        continue;
     while ((typ = gettype(argf[checkfile])) != CTYPE &&
             typ != ATYPE && (checkfile < arglc+1))
                   checkfile++ ;

     /* enregistrer le nom du source */
     if (checkfile < arglc+1)
            strcpy(OrefFile[iref-1], argfA[checkfile]);

     if ((filarg0 = stat(argfA[0],&entry)) == 0) { /* le fichier existe */
        filarg0 = entry.st_atime;
        command = FALSE;

            if (checkfile == arglc+1) { /* il n'y a pas de source */
                     sprintf(work, fileNsrcStr, line);
                     pushPop_MESS(work, ERRMSG);
                     refExit(wm, BRW|BEXIT, 0);
            }


            print_at(wm, 1, OFF_CHECK, checkString, OLDATT); /**/
            printW(argf[checkfile], offset, OFF_CHECK, TRUE);

            if (stat(argfA[checkfile],&entry) == 0) { /* fichier OK */

                 if (entry.st_atime > filarg0)
                    command = TRUE;
            }
            else {  /* tester si macro */
                 if (gettype(argf[checkfile]) != MTYPE) {
                     sprintf(work, fileNfStr, line, argf[checkfile]);
                     pushPop_MESS(work, ERRMSG);
                     refExit(wm, BRW|BEXIT, 0);
                 }
                 else  {  /* c'est une macro  : poursuivre le test */

                     if ((mdate = getMDATE(argf[checkfile])) > filarg0) {
                          command = TRUE;
                          break;
                     }
                     else
                        if (mdate < 0) {
                           sprintf(work, macNfStr, line, argf[checkfile]);
                           pushPop_MESS(work, ERRMSG);
                           refExit(wm, BRW|BEXIT, 0);
                        }
                 }
            }

        /* tester si on doit lancer la commande */
        if (command)
            if (execcom((typ = gettype(argf[checkfile])), &difference, &moduleBuild) == -1)
                goto ExitBrowser;
     }
     else      /* le fichier en arg0 n'existe pas */
     {
       if (gettype(argf[0]) != MTYPE)  {
          if (execcom((typ = gettype(argf[checkfile])), &difference, &moduleBuild) == -1)
              goto ExitBrowser;
       }
       else     /* c'est une MACRO */
          if (putMACRO() == -1)
              goto ExitBrowser;
     }
   }

ExitBrowser:

   if (browserError) {
       if (ret = CheckErrorLevel(typ)) {
                winPopUp(wm, W_CLOSE);
                strcpy(work, errStr);
                verify_window(wm, work, nullstr, ERRMSG);
                getkey(); /*_read_kbd();*/
                refExit(wm, BRW|BERR, ret);
       }
       goto EXIT;
   }

   if (!difference && !vl->conf.buildDB)
        strcpy(work, DbaseUpd);
   else {
        /* Linker la base */
        if (dbaseLink())  /* erreur */
                strcpy(work, dbaseErrStr);
        else
                strcpy(work, dbaseStr);
   }
   vl->conf.buildDB = FALSE;
   winPopUp(wm, W_CLOSE);
   verify_window(wm, work, nullstr, NMXMSG);
   getkey();
   winPopUp(wm, W_CLOSE);

EXIT:

   vl->retCode = (BRW|BEXIT);
   inhibe_mouse();
   showMouse();
   free(TOKbuffer);
   free(buftoken);
   free(bufline);

   exit(0);  /* relancer l'editeur */

}

/*-----------
 * updateWtitle
 *-----------
 */
updateWtitle(newTitle)
char *newTitle;
{
        wm->title = newTitle;
        write_wbox(wm, TYP1);
}

/*-----------
 * dbaseLink
 *-----------
 */
dbaseLink()
{
   /* redefinir la fenetre */
   winPopUp(wm, W_CLOSE);
   initWin(wm, ptr2, (80-WBRWSIZE2)/2, (25 - WBRWH)/2, wbrwsize = WBRWSIZE2, WBRWH, TRUE, FALSE,
           vl->WINSYSink, vl->WINSYSpaper, vl->WINSYSborder, vl->WINSYSblock, W_WIN);

   winPopUp(wm, W_OPEN, TYP1);

   /* creer les fichiers de la base */
   if (makeDBfiles())
        return(-1);

   /* interconnecter les fichiers */
   if (linkDB())
        return(-1);

   /* fermer la base */
   closeDB();
   return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * makeDBfiles
 *ÄÄÄÄÄÄÄÄÄÄ
 */
makeDBfiles()
{
        updateWtitle(lnkDBstr);
        printW(creaDBstr, (wbrwsize - strlen(creaDBstr))/2, (WBRWH - 1)/2, FALSE);

        if ((Hmod  = createDB("module.db")) < 0) {
                sound(1000);
                pushPop_MESS(dbaseFerrStr,ERRMSG);
                return(-1);
        }
        if ((Hdecl = createDB("declar.db")) < 0) {
                sound(1000);
                pushPop_MESS(dbaseFerrStr,ERRMSG);
                return(-1);
        }
        if ((Hext  = createDB("external.db")) < 0) {
                sound(1000);
                pushPop_MESS(dbaseFerrStr,ERRMSG);
                return(-1);
        }
        return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * createDB
 *ÄÄÄÄÄÄÄÄÄÄ
 */
createDB(str)
char *str;
{
        strcpy(work, c_x);  /* user dir */
        strcat(work, str);
        remove(work);
        return(open(work, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE));
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * closeDB
 *ÄÄÄÄÄÄÄÄÄÄ
 */
closeDB()
{
   xclose(Hmod);
   xclose(Hdecl);
   xclose(Hext);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * linkDB : ONE pass db linker
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
linkDB()
{
   int i;
   char aux[80];

   /* ETAPE 1 : placer les informations dans les fichiers de la base:
    * les nom absolus de modules sont stock‚s dans MODULE.DB, les
    * declarations sont plac‚es dans le fichier DECLAR.DB et les
    * references dans une liste chain‚e de references.
    */

   /* init head et tail de la LLIST */
   memset(lhead.def.function, '\0', 32);
   memset(ltail.def.function, 'z', 32);
   ltail.def.function[31] = '\0';
   lhead.next = &ltail;
   ltail.prev = &lhead;
   lhead.prev = ltail.next = NULLLIST;

   /* rincer la fenetre */
   clrwin(wm, BLANK);
   printW(dbaseLnkStr, 1, OFF_COMP, FALSE);
/*   printW(dbaseFunc, 1, OFF_COMP+1, FALSE);*/
   /* boucle generale de traitement */
   for (i=0; i<iref; i++) {

        memset(aux, 0, 64);
        strcpy(aux, OrefFile[i]);
        insertMod(aux);      /* placer le nom dans MODULE.DB */
        memset(aux, 0, 64);
        strcpy(aux, c_x);
        strcat(aux, refFile[i]);     /* nom absolu des .REF */
        printW(refFile[i], offset, OFF_COMP, TRUE);
        if (insertRefToList(aux, i)) /* placer les references dans la LLIST */
                return(-1);
   }

   /* ETAPE 2 : Placer les references externes dans EXTERNAL.DB et
    * etablir les liens entre DECLAR.DB et EXTERNAL.DB (les liens entre
    * DECLAR.DB et MODULE.DB sont deja etablis)
    */

   makeDeclExtrn();
   return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * insertMod - record .REF name
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
insertMod(fname)
char *fname;
{
        write(Hmod, fname, 64);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * printW - ecrire dans fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
printW(fname, offset, y, upCase)
char *fname;
{
        char work[80];

            memset(work, ' ', 80);
            print_nb_at(wm, offset, y, work, wbrwsize-offset, OLDATT);
            strcpy(work, fname);
            if (upCase)
                strupr(work);
            memset(&work[strlen(fname)], ' ', 80-(strlen(fname)+1));
            print_nb_at(wm, offset, y, work, wbrwsize-(offset+1), OLDATT);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * insertRefToList - placer les references dans la LLIST
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
insertRefToList(fname, no)
char *fname;
unsigned char no;
{
        struct LLIST *lref;
        struct refDef ref;
        int fd, NB, i;

        /* ouvrir fichier .REF */
        if ((fd = open(fname, O_RDWR|O_BINARY)) < 0) {
                sprintf(work, dbaseOpenErr, fname);
                pushPop_MESS(work, ERRMSG);
                return(-1);
        }

        /* recuperer le nb d'enregistrements en debut de fichier */
        read(fd, &NB, 2);

        /* Placer les elements dans la liste */
        nDef = nExt = 0;
        print_refDef();
        for (i = 0; i < NB; i++) {
                /* Lire fonction dans .REF */
                read(fd, &ref, sizeof(struct refDef));
                if (strlen(ref.funcName) > 0) {

                        /* creer un nouvel element */
                        if (((int *)lref = malloc(sizeof(struct LLIST))) == (int *)0) {
                                pushPop_MESS(outofmem, ERRMSG);
                                return(-1);
                        }

                        memset(lref, 0, sizeof(struct LLIST));
                        memcpy(lref, &ref, sizeof(struct refDef));

                }
                else
                        continue;

             /*   printW(lref->def.function, offset, OFF_COMP+1, FALSE);*/
                if (insertToList(lref, no))
                        return(-1);
                print_refDef();
        }
        /* verifier si on est bien en fin de fichier */
        read(fd, &ref, sizeof(struct refDef));
        if (ref.type != 0xff) {
                strcat(work, dbaseIntegrityErr);
                pushPop_MESS(work, ERRMSG);
                getkey();
                return(-1);
        }
        xclose(fd);
        return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * showBase
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
showBase()
{
        struct LLIST *wf;
        int i;

        for (wf = &lhead; wf != &ltail; wf = wf->next) {
            sprintf(work," %s : %s ", wf->def.function, (wf->def.type == TYPE_DEF?"DEF":"EXT"));
            pushPop_MESS(work, ERRMSG);
        }
        return(0);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * insertToList - insere en triant dans la liste LLIST
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
insertToList(lref, noMod)
struct LLIST *lref;
unsigned char noMod;
{
        struct LLIST *wf;
        int status;

        for (wf = &lhead; wf != &ltail; wf = wf->next) {
             if ((status = strcmp(lref->def.function, wf->def.function)) > 0)
                 continue;
             else
                 if (status == 0) { /* nom deja dans la liste */
                        if (wf->def.module < 0) {
                                if (lref->def.type == TYPE_DEF) {
                                        /* on a affaire a la declaration */
                                        wf->def.module   = noMod;
                                        wf->def.type     = TYPE_DEF;
                                        wf->def.position = lref->def.position;
                                        nDef++;
                                }
                                else {
                                        *(wf->def.offRef + wf->def.cpt++) = noMod;
                                        nExt++;
                                }
                        }
                        else {
                                *(wf->def.offRef + wf->def.cpt++) = noMod;
                                nExt++;
                        }
                        /* liberer la zone memoire */
                        /*free(lref);*/
                        return(0);
                 }
                 else

                        break;
        }

        /* nouvelle fonction dans la liste */
        wf->prev->next   = lref;
        lref->prev       = wf->prev;
        wf->prev         = lref;
        lref->next       = wf;

        /* init EXT zone */
        if (((int *)(lref->def.offRef) = malloc(iref+1)) == (int *)0) {
                pushPop_MESS(outofmem, ERRMSG);
                return(-1);
        }
        memset(lref->def.offRef, 0, iref+1);

        /* init DEF field */
        if (lref->def.type == TYPE_DEF) {
                lref->def.module = noMod;
                nDef++;
        }
        else {
                /* verifier si ce n'est pas une fct de Borland */
                CheckAPI(lref, noMod);
                lref->def.module = -1;
                nExt++;
                *(lref->def.offRef + lref->def.cpt++) = noMod;
        }
        return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * makeDeclExtrn() - Mise a jour de EXTERNAL.DB et DECLAR.DB
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
makeDeclExtrn()
{
        struct LLIST *wf;
        long pos, tell();
        int i;

        /* rincer la fenetre */
        clrwin(wm, BLANK);
        updateWtitle(mkeDBstr);
        printW(recDBfiles, (wbrwsize - strlen(recDBfiles))/2, (WBRWH - 1)/2, FALSE);

        for (wf = &lhead; wf != &ltail; wf = wf->next) {
                if (wf->def.type == TYPE_DEF) {

                        /* enregistrer EXT informations */
                        pos = tell(Hext);
                        write(Hext,  wf->def.offRef, wf->def.cpt);

                        /* liberer zone EXT */
                        free(wf->def.offRef);

                        /* etablir Lien entre DECLAR.DB et EXTERNAL.DB */
                        (long)wf->def.offRef = pos;

                        /* enregistrer DECL information */
                        write(Hdecl, &wf->def, sizeof(struct def));
                }
        }

        /* liberer les elements de la liste */
        free_lst();
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * CheckAPI -  verifie si la fonction n'est pas une fct de la lib Borland
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
CheckAPI(lref, noMod)
struct LLIST *lref;
{
        int i, status;
        char aux[80];
  /*      pushPop_MESS(lref->def.function , ERRMSG);*/
        for (i=0; strlen(_borlfunc[i]); i++) {
/*        pushPop_MESS(lref->def.function , ERRMSG);
        pushPop_MESS(_borlfunc[i] , ERRMSG);*/

             if ((status = stricmp(lref->def.function, _borlfunc[i])) > 0)
                 continue;
             if (status == 0) {
                strcpy(aux, " ");
                strcat(aux, OrefFile[noMod]);
                strcat(aux, ": \'");
                strcat(aux, &_borlfunc[i][1]);
                strcat(aux, "\' is not allowed ");
                pushPop_MESS(aux , ERRMSG);
                return(-1);
             }
        }
        return(0);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * free_lst - liberer la liste des fichiers
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
free_lst()
{
    struct LLIST *wf, *aux;
    for (wf = lhead.next; wf != &ltail; wf = aux) {
         aux = wf->next;
         free(wf);
    }
}


/*ÄÄÄÄÄÄÄÄÄÄ
 * MakeAbsName
 *ÄÄÄÄÄÄÄÄÄÄ
 */
MakeAbsName(ntoken, type)
{
   char *glta();
   char checkStr[80], *p;
   int i;


     for (i=0; i <= ntoken; i++) {

        /* Nom absolu */
        strcpy(checkStr, argf[i]);
        if (i == 0) {  /* modifier les extensions de fichier en 0 */
            if (gettype(checkStr) == ETYPE) /* fichier EXE */
                return(-1);

            if ((p = strpbrk(checkStr, ".")) == NULL)
                        p = checkStr;
                else {
                        if (*(p+1) == '$') { /* Macro */
                                strcpy(argf[i], checkStr);
                                continue;
                        }
                        *p = '\0';
                }

            /* ajouter extension ".ref" */
            strcat(p, ".REF");
            strcpy(argf[0], checkStr);  /* possible puisque le taille du
                                         * nom reste identique (OBJ -> REF)
                                         */
            strcpy(refFile[iref++], argf[0]);

        }
        strcpy(argfA[i], auxStr);
        strcat(argfA[i], checkStr);
    }
    return(0);
}


/*ÄÄÄÄÄÄÄÄÄÄ
 * CheckErrorLevel
 *ÄÄÄÄÄÄÄÄÄÄ
 */
CheckErrorLevel(typ)
{
        if (errorlevel < 0) {
          if (typ == CTYPE)
                strcpy(work, noCompStr);
          else
                strcpy(work, noAssStr);
          affiche(work, ERRMSG);
          return(0);
        }
        return(errorlevel);
}

/*---------------------------------------------------
 * getline - saisit une ligne de commande en
 *           supprimant CR et remplacant LF par '\0'
 *---------------------------------------------------
 */
getline(fp)
FILE *fp;
{
   char c;
   int input = 0;

   /* saisir ligne en supprimant les CR */
   while ((c = getc(fp)) != '\n'&& (input < nb_car_per_line) && c != EOF)
             if (c != '\r')   bufline[input++] = c;

   if (c == '\n') {
              bufline[input++] = '\0';
              return(1);
   }
   else {
              bufline[input] = EOF;
              return(0);
  }
}

/*--------------------------------------------
 * putargf - place les @ des tokens dans argf
 *--------------------------------------------
 */
putargf(err)
int *err;
{
   char c;
   int i, home, output;
   struct OBJ errwin;

   if (strcmp(vl->cdirname, vl->homedir) != 0)
        home = FALSE;
   else
        home = TRUE;

   output = 0;

   while (TRUE)
   {
      while (bufline[output] == ' ')  output++;    /* sauter les blancs */

      /* verifier si la config est conforme  a l'etat de l'analyse */
      c = bufline[output];

      /* tester si commentaire */
      if (c == '#')  return(DEB_RELATION);

      /* suivant etat du scanning verifier la syntaxe */
      if (analyse == DEB_RELATION) {

           if (c == '\0') {

                relativline = 0;
                return(DEB_RELATION);
           }
           else
               if (c == EOF)  {
                   *err = 0;
                   return(FIN_MAKE);
               }
      }
      else  {

          if (c == '\0') {
              arglc--;
              return(FIN_RELATION);
          }
          if (c == '+')
                return(FIN_LIGNE);

          if (c == EOF)
                return(EOC_MISSING);

          if ((c == ':') && (relativline != 0))  {
               sprintf(work, syntErr2, line);
               pushPop_MESS(work, ERRMSG);
               browserError = TRUE;
               *err = 1;
               return(FIN_MAKE);
          }
      }
      analyse = SCAN_LIGNE;
      i = output;    /* debut de token */

      /* se positionner sur le prochain separateur */
      while (( (c = bufline[output]) != ' ') && (c != '\0') && (c != '+') && (c != EOF))
            output++;

      bufline[output] = '\0';

      if (c == EOF) {  /* fin de fichier */

                strcpy(tokenptr,vl->cdirname);
                if (get_deepth(tokenptr) > 0)
                  strcat(tokenptr, "\\");
                strcat(tokenptr,&bufline[i]);
                argf[arglc] = tokenptr;
                tokenptr += strlen(&bufline[i])+strlen(vl->cdirname)+1;
                return(EOC_MISSING);
      }
      if (c == '\0') { /* fin de relation */

      strcpy(tokenptr,&bufline[i]);
                argf[arglc] = tokenptr;
                tokenptr += strlen(&bufline[i])+1;
                relativline = 0;
                return(FIN_RELATION);
      }
      if (c == '+') {  /* fin de ligne */

                strcpy(tokenptr,&bufline[i]);
                argf[arglc] = tokenptr;
                arglc++;
                tokenptr += strlen(&bufline[i])+1;
                relativline++;
                return(FIN_LIGNE);
      }
      strcpy(tokenptr,&bufline[i]);
      argf[arglc] = tokenptr;
      arglc++;
      tokenptr += strlen(&bufline[i])+1;
      output++;
   }
}

/*---------------------------------------------------
 * gettype - indique le type de commande a effectuer
 *---------------------------------------------------
 */
gettype(arg0)
char *arg0;
{
     char type;

     while (( type = *(arg0++)) != '\0')
        if ( type == '.') {

            type = *arg0;
            switch(type) {
            case 'c' :
            case 'C' :  return(CTYPE);    /* type C */
            case 'a' :
            case 'A' :  return(ATYPE);    /* type assembleur */
            case 'o' :
            case 'O' :  return(LTYPE);    /* type link */
            case 'h' :
            case 'H' :  return(HTYPE);    /* type header */
            case '$' :  return(MTYPE);    /* type macro  */
            case 'D' :  return(DTYPE);    /* type REF    */
            default  :  return(ETYPE);    /* erreur      */
            }
        };
     return(ETYPE);
}


/*------------------------------------------------------------------
 * getMDATE - donne la date associee a la MACRO passee en parametre
 *------------------------------------------------------------------
 */
long getMDATE(macro)
char *macro;
{
   int i;
   struct OBJ errwin;
   for (i=0;i<MACROfree;i++)
      if (strncmp(MACROname[i],macro,strlen(argf[0])) == 0)
         return(MACROdate[i]);
   sprintf(work, undefMacStr, line, argf[0], macro);
   pushPop_MESS(work, ERRMSG);
   browserError = TRUE;
   return(-1);
}


/*ÄÄÄÄÄÄÄÄÄÄ
 * print_refDef
 *ÄÄÄÄÄÄÄÄÄÄ
 */
print_refDef()
{
        char work[80];

        sprintf(work,"          %4d  %s ",nDef, nDefStr);
        print_at(wm, 1, OFF_COMP+3, work, OLDATT);
        sprintf(work,"          %4d  %s ", nExt, nExtStr);
        print_at(wm, 1, OFF_COMP+4, work, OLDATT);
}

/*-----------------------------------------------------------------
 * execcom - lancer la commande correspondante au type de relation
 *-----------------------------------------------------------------
 */
execcom(relation, diff, modB)
int relation;
int *diff, *modB;
{
      char **combuf;
      char string[80], includeDir[80], outputDir[80], work2[80], *ptr, *pt;
      FILE  *Xtmp;
      int fredirec, noOverWriteOut, noOverWriteInc;
      int i,j,n,insert,filout;

      *modB = FALSE;
      if (relation == ETYPE)  return(ETYPE);

      switch(relation) {
        case CTYPE : break;
        case ATYPE : break;
        case LTYPE :
        case HTYPE :break;
      }
      inhibe_screen();  /* pour eviter les messages compilateur */

      /* gestion affichage des noms */
      ptr = argf[0];

      /* effacer les caracteres en zone de verification */

      if (strlen(c_x)) {
          strcpy(outputDir, "-n");
          strcat(outputDir, c_x);
          noOverWriteOut++;
      }

      /* le nom absolu du fichier est dans argfA[2] */
      strcpy(string,argfA[2]);

      /* ecrire la commande */
      memset(work, '\0', 80);
      if (relation == CTYPE)
              j = 0;
      else
              j = 1;


/*       memset(work, ' ', 79);
       print_nb_at(wm, offset, OFF_COMP, work, INF(wbrwsize-offset, strlen(work)-1), OLDATT);

       memset(work, '\0', 80);
       sprintf(work,"%s\n", ptr);
       strupr(work);*/


       print_at(wm, 1, OFF_COMP, generateStr, OLDATT);
       printW(ptr, offset, OFF_COMP, TRUE);
/*       print_nb_at(wm, offset, OFF_COMP, work, INF(wbrwsize-offset, strlen(work)-1), OLDATT);*/

       *diff = TRUE;
       i  = strlen(work);
       strcpy(vl->ferror_name, ptr);
       write(fmsg, work, i);

       /* lancer la creation des references */
       strupr(string);
       if (refcall(string) < 0) {
            browserError = TRUE;
            enable_screen();
            return(-1);  /* traiter messages erreurs Comp & Ass */
       }
       print_at(wm, 1, OFF_CHECK, checkString, OLDATT);
       memset(work, ' ', 79);
       print_nb_at(wm, offset, OFF_COMP, work, INF(wbrwsize-offset, strlen(work)-1), OLDATT);
       enable_screen();
       return(0);
}


/*----------------------------------------------------
 * affiche -
 *----------------------------------------------------
 */
affiche(str, msgtyp)
char *str;
{
    struct OBJ errwin;

    pushPop_MESS(str, msgtyp);
    winPopUp(wm, W_CLOSE);  /* supprimer cadre compilation */
}


/*----------------------------------------------------
 * inhibe_screen -
 *----------------------------------------------------
 */
inhibe_screen()
{
   void interrupt Dummy_interrupt();

   _dos_setvect(0x10, Dummy_interrupt);
   /* se placer dans la directory courante
    * ( utilis‚ pour creer les objets dans la mˆme directory
    * que les sources)
    */
/*   chdir(vl->cdirname);*/
}

/*----------------------------------------------------
 * enable_screen -
 *----------------------------------------------------
 */
enable_screen()
{
   _dos_setvect(0x10, BiosInt10);
/*   chdir(vl->homedir);*/
}


/*----------------------------------------------------
 * dummy_interrupt
 *----------------------------------------------------
 */
void interrupt Dummy_interrupt()
{
}


/**********************
 * IMPORTATIONS
 **********************/

/*---------------------------------------------------------------------------
 * verify_window - gestion apparition des messages systemes
 *---------------------------------------------------------------------------
 */
verify_window(wp, str, titre, type)
struct OBJ *wp;
char *str, *titre;         /* message a ecrire et titre */
{
    int size;
    unsigned inkAtt, paperAtt, borderAtt, messAtt;

        if (type == ERRMSG) {
                inkAtt    = vl->WINmessInk;
                paperAtt  = vl->WINmessPaper;
                borderAtt = vl->WINmessBorder;

        }
        else {
                inkAtt    = vl->WINSYSink;
                paperAtt  = vl->WINSYSpaper;
                borderAtt = vl->WINSYSborder;
        }

        messAtt   = inkAtt|paperAtt;
        if ((size = strlen(str)) > 78)
                size = 76;
        initWin(wp, titre, (80 - size)/2, 10, size, 3, TRUE, FALSE, inkAtt, paperAtt, borderAtt, vl->WINSYSblock, W_WIN);
        winPopUp(wp, W_OPEN, type);
        messwin = wp;
        Xmess = 0;
        Ymess = 1;
        print_at(messwin, Xmess, Ymess, str, NEWATT, messAtt);

}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * winPopUp - ouvrir / fermer une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
winPopUp(wp, mode, borderType)
struct OBJ *wp;
{
    if (mode == W_OPEN) {
        /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
        if (wp->save == NULL) /* check presence tampon */
            if ((wp->save = malloc(2 * (wp->ncol + 4) * (wp->nline + 3))) == NULL) {
                 pushPop_MESS(errmalloc, ERRMSG);
                 return(FUNC|F6);
            }
        wp->pushed = TRUE;
        saveScreen(wp, WINDOW);
        write_wbox(wp, borderType);
        clrwin(wp, BLANK);
    }
    else {
        wp->pushed = FALSE;
        if (wp->save != NULL) {
            restoreScreen(wp, WINDOW);
            free(wp->save);
            wp->save = NULL;
        }
    }
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * clrwin Ä clear window
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_ombrage - dessiner l'ombrage
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_mbox - dessiner le contour d'un menu
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_mbox(mp)
struct OBJ *mp;
{
  /* type 'Ä' */
  write_box(mp->title, mp->ul_x, mp->ul_y, mp->ncol, mp->nline, mp->border, 0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_box - dessiner le contour d'une BOXE
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_box(title,x,y,l,h,att,typ)
char *title;
unsigned char att;
{
   int ps,i,j,tl;
   unsigned char ulc, llc, urc, lrc, Vchar, Hchar;
   char Ch;

   ulc   = boxtype[typ][0];
   urc   = boxtype[typ][1];
   llc   = boxtype[typ][2];
   lrc   = boxtype[typ][3];
   Vchar = boxtype[typ][4];
   Hchar = boxtype[typ][5];


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
        Ch = BLANK;
        _wstringDB(i++   , y, 1 , NEWATT, att, &Ch);
        _wstringDB(i     , y, tl, NEWATT, att, title);
        _wstringDB(i + tl, y, 1 , NEWATT, att, &Ch);
   }


   /* VERTICALES */
   for (i = y+1; i <= y+h; i++ ) {
        _wstringDB(x        , i, 1, NEWATT, att, &Vchar);
        _wstringDB(x + l + 1, i, 1, NEWATT, att, &Vchar);
   }
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_wbox - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_wbox(wp, type)
struct OBJ *wp;
{
      write_box(wp->title, wp->ul_x, wp->ul_y, wp->ncol, wp->nline, wp->border, type);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreScreen - RESTITUER la partie de l'ecran ecras‚ par le SOUS MENU
 *                 ou la WINDOW
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
restoreScreen(mp)
struct OBJ *mp;
{
    if (mp->objet == MENU)
        restoreFromMenu(mp,mp->save);
    else
        _refresh( mp->ul_x,
                  mp->ul_y,
                  mp->ul_x + mp->ncol + 3,
                  mp->ul_y + mp->nline + 2,
                  mp->save);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreFromMenu- ecriture PHYSIQUE d'un tampon sur l'ecran
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
restoreFromMenu(mp,buf)
struct OBJ *mp;
int            *buf;
{
   if (mp->mov == VER)
       _refresh(  mp->ul_x,
                  mp->ul_y,
                  mp->ul_x + mp->ncol+3,
                  mp->ul_y + mp->nline+2,
                  buf);
   else
       _wstringDW(mp->ul_x,
                  mp->ul_y,
                  mp->ncol,
                  buf);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * saveScreen - SAUVEGARDER la partie de l'ecran correspondant au SOUS MENU
 *              ou a la WINDOW
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
saveScreen(mp)
struct OBJ *mp;
{
    if (mp->objet == MENU) {
        if (mp->mov == VER)
            _save_screen(mp->ul_x,
                     mp->ul_y,
                     mp->ul_x + mp->ncol+3,
                     mp->ul_y + mp->nline+2,
                     mp->save);
        else
            _rstringDW(  mp->ul_x,
                     mp->ul_y,
                     mp->ncol,
                     mp->save);

    }
    else
        _save_screen( mp->ul_x,
                      mp->ul_y,
                      mp->ul_x + mp->ncol + 3,
                      mp->ul_y + mp->nline + 2,
                      mp->save);
   if (mp->f_ombrage)
       write_ombrage(mp->ul_x, mp->ul_y, mp->ncol, mp->nline);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * initWin : initialiser une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION initWin(wp,title,ul_x,ul_y,ncols,nlines,ombrage,bar,ink,paper,border,batt)
struct OBJ *wp;
char *title;
{
   wp->objet      = WINDOW;
   wp->nline      = nlines;
   wp->leftCh     = 0;
   wp->curX       = wp->curY = 0;
   wp->border    = border;
   wp->ink        = ink;
   wp->paper      = paper;
   wp->ul_x       = ul_x;
   wp->ul_y       = ul_y;
   wp->ncol       = ncols;
   wp->pushed    = FALSE;
   wp->title      = title;
   wp->save       = NULL;
   wp->f_ombrage  = ombrage;
   wp->f_bar      = bar;

/*   fillCh         = 0;*/

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
   verify_window(&errwin, aux, nullstr, msgtyp);
   getkey(); /*_read_kbd();*/
   winPopUp(messwin, W_CLOSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * get_deepth - retourne la profondeur du pathName (si 0 <= : dir courante)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

/*----------------------------------------------------
 * putMACRO - place une MACRO dans la table des macros
 *----------------------------------------------------
 */
putMACRO()
{
      char *glta();
      int checkfile,i;
      struct OBJ errwin;
      char buf[80];

        checkfile = 2;
        strcpy(buf, conf.u.pd.c_m);
        for (i=0;i<MACROfree;i++)
            if (strncmp(MACROname[i],argf[0],strlen(argf[0])) == 0)
            {   /* la macro existe deja */
               sprintf(work, redefMacStr, line, argf[0]);
               verify_window(&errwin, work, nullstr, ERRMSG);
               getkey(); /*_read_kbd();*/
               winPopUp(messwin, W_CLOSE);
               browserError = TRUE;
               return(-1);
            };

        /* la macro n'existe pas : la creer */
        strcpy(MACROname[MACROfree],argf[0]);
        if (++MACROfree > MAXMACRO)  {

                sprintf(work, tooMaMacStr, line);
                verify_window(&errwin, work, nullstr, ERRMSG);
                getkey(); /*_read_kbd();*/
                winPopUp(messwin, W_CLOSE);
                browserError = TRUE;
                return(-1);
        }

        /* donner a la MACRO la date du fichier le plus recent */
        filarg0 = 0;
        while (checkfile < arglc+1) {
            if (stat(argfA[checkfile],&entry) == 0)
            {
                 if (entry.st_atime > filarg0)
                     filarg0 = entry.st_atime;
                 checkfile++;
            }
            else
            {
                sprintf(work, cannotCrMac, line, argf[checkfile]);
                verify_window(&errwin, work, nullstr, ERRMSG);
                getkey();
                winPopUp(messwin, W_CLOSE);
                browserError = TRUE;
                return(-1);
            }
        }
        MACROdate[MACROfree - 1] = filarg0;
        return(0);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * glta - get last token address : retire le dernier token de l'arg
 *        et retourne son adresse
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at - ecrire la string str aux coordonnees x, y, dans
 *            la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
print_at(wp, x, y, str, isnewatt, att)
struct OBJ *wp;
char *str;
{
  int i;
  if ((i = strlen(str)) > 0)
       _wstringDB(wp->ul_x + 1 + x,
                  wp->ul_y + 1 + y, i, isnewatt, att, str);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at_nb - ecrire nb carac aux coordonnees x, y, dans
 *               la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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


/*--------------------------------------------------------------
 * m_installed - teste si le driver souris est present. si oui,
 *               retourne le nombre de boutons
 *--------------------------------------------------------------
 */
m_installed(nbutton)
unsigned *nbutton;
{
 union  REGS  rin,rout;

 rin.x.ax  = 0;
 int86(0x33, &rin, &rout);
 *nbutton = rin.x.bx;
 return(rout.x.ax);
}

/*--------------------------------------------------------------
 * m_getPosAndButtonStatus - retourne la position et le status:
 *                        le bit 0 represente celui de droite
 *                        le bit 1 represente celui de gauche
 *                        Pour Chacun des 2 bits :
 *                        si … 1 -> enfonc‚,   0 -> relach‚
 *--------------------------------------------------------------
 */
m_getPosAndButtonStatus(posX, posY, buttonStatus)
int *posX, *posY;
unsigned   *buttonStatus;
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x03;
        int86(0x33, &rin, &rout);
        *posX = rout.x.cx;
        *posY = rout.x.dx;
        *buttonStatus = rout.x.bx;
}

/*--------------------------------------------------------------
 * m_setCursorPos -
 *
 *--------------------------------------------------------------
 */
m_setPos(newPosX, newPosY)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x04;
        rin.x.cx  = newPosX;
        rin.x.dx  = newPosY;
        int86(0x33, &rin, &rout);
}

/*--------------------------------------------------------------
 * m_callMaskAndAddress - chaine … l'IT souris, le traitement
 *                       utilisateur a appeler si les conditions
 *                       exprim‚es dans le masque sont reunies:
 *                              - bit 0 : changement de position
 *                              - bit 1 : bouton Gauche press‚
 *                              - bit 2 : bouton Gauche relach‚
 *                              - bit 3 : bouton Droit press‚
 *                              - bit 4 : bouton Droit relach‚
 *--------------------------------------------------------------
 */
m_callMaskAndAddress(callMask, subroutine)
int callMask;
int (far * subroutine)();
{
 union  REGS  rin,rout;
 struct SREGS segreg;

        rin.x.ax  = 0x0c;
        rin.x.cx  = callMask;
        rin.x.dx  = FP_OFF(subroutine) ;
        segreg.es = FP_SEG(subroutine);
        int86x(0x33, &rin, &rout,&segreg);
}

it_mouse()
{
    mouseEvent++;
}

/*--------------------------------------------------------------------
 * getkey - lire un caractere et son attribut sur une page inactive
 *          retourne (TYPE | caractere)
 *------------------------------------------------------------------
 */
getkey()
{
   unsigned key, main, auxiliary;

   m_callMaskAndAddress(M_LEFT_PRESS, prolog_it_mouse);

   for (;;) {
        /* examiner le status du clavier */
        if (_look_kbd()) { /* caractere tap‚ au clavier */
             _read_kbd();
             break;
         }

         /* tester si on a utilis‚ la souris */
         if (mouseEvent) {
               mouseEvent = 0;
               break;
         }
   }
   m_callMaskAndAddress(0, 0L);

}

inhibe_mouse()
{
}


/* compatibilite avec la procedure d'installation */
new_int10h()
{
}

DupEnviron()
{
    strcpy(c_cl, vl->conf.u.pd.c_cl);    /* comp & linker dir   */
    if (strlen(c_cl))
        strcat(c_cl, "\\");
    strcpy(c_a, vl->conf.u.pd.c_a);      /* assembleur dir      */
    if (strlen(c_a))
        strcat(c_a, "\\");
    strcpy(c_i, vl->conf.u.pd.c_i);      /* include dir         */
    if (strlen(c_i))
        strcat(c_i, "\\");
    strcpy(c_z, vl->conf.u.pd.c_I);      /* MKD include dir     */
    if (strlen(c_z))
        strcat(c_z, "\\");
    strcpy(c_l,  vl->conf.u.pd.c_l);     /* TC librairies dir   */
    if (strlen(c_l))
        strcat(c_l, "\\");
    strcpy(c_u,  vl->conf.u.pd.c_u);     /* turbix LIB dir         */
    if (strlen(c_u))
        strcat(c_u, "\\");
    strcpy(c_m,  vl->conf.u.pd.c_m);     /* make file name      */
    if (strlen(c_m))
        strcat(c_m, "\\");
    strcpy(c_p,  vl->conf.u.pd.c_p);     /* paste file name     */
    if (strlen(c_p))
        strcat(c_p, "\\");
    strcpy(c_h,  vl->conf.u.pd.c_h);     /* help dir name       */
    if (strlen(c_h))
        strcat(c_h, "\\");
    strcpy(c_x,  vl->conf.u.pd.c_x);     /* user dir name       */
    if (strlen(c_x))
        strcat(c_x, "\\");
    nb_car_per_line = vl->conf.nb_car_per_line;
    if ((TOKbuffer = malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
}
xclose(fd)
{
        close(fd);
        flush_buffer(fd);
}
refExit(wm, code, ret)
struct OBJ *wm;
{
                     winPopUp(wm, W_CLOSE);
                     vl->retCode = code;
                     inhibe_mouse();
                     showMouse();
                     free(TOKbuffer);
                     free(buftoken);
                     free(bufline);
                     exit(ret);
}
initBuffer()
{
    if ((buftoken = malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
    if ((bufline = malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
}

