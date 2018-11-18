/* TX_CHK */

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
                                {'Ö','·','Ó','½','º','Ä','Ç','¶','Ò','Ð','×'},   /* type 3 */
                                {'É','»','È','¼','º','Í','Ì','¹','Ë','Ê','Î'},   /* type 4 */
                                {'°','°','°','°','°','°','°','°','°','°','°'},   /* type 5 */
                                {'Ç','¶','Ó','½','º','Ä','Ç','¶','Ò','Ð','×'},   /* status */
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

char *ARGV[5];
char *Rbuf;
char writeBuf[80];

/* RECORD dans le fichier des DEF */
struct defRef {
         char     ctype;       /* fichier C ou Ass */
         int      position;   /* numero de ligne ou est definie la func */
         char     funcName[32];/* nom de la fonction */
unsigned char     type;       /* type : EXT, DEF, DEC */
};

/* Zone de decomposition en tokens */
#define SHBUFLEN    nb_car_per_line    /* longueur du buffer                */
#define SHARGLEN    82                 /* longueur de la zone des arguments */
#define SHMAXTOK    80                 /* maximum de tokens par ligne       */
#define SHMLEN      12                 /* longueur maximum du nom machine   */

struct  shvars {                       /* shell variables                   */
        long   shlast;                 /* derniere frappe                   */
        int    shncmds;                /* nombre de commandes valides       */
        char   *shbuf;                 /* buffer du shell                   */
        char   *shtok[SHMAXTOK];       /* pointeurs d'entree d'un token     */
        char   shtktyp[SHMAXTOK];      /* type de token dans shtok[i]       */
        char   shargst[SHARGLEN];      /* zone stockage string arg          */
};



#define MAXFUNC 500             /* Nb max de fonctions dans un module */
struct defRef functab[MAXFUNC];

#define TYPE_NUL 0      /* rien */
#define TYPE_DEF 1      /* Definition */
#define TYPE_EXT 2      /* Externe */
#define TYPE_DEC 3      /* Declaration */

char *typeTab[] = {"","DEF","EXT","DECL"};

char *CreservedWord[] = {
        "if", "return","while","for","int","long","sizeof","FP_OFF","FP_SEG",
        "switch"};

#define CRESERVW 10
#define TRUE  1
#define FALSE 0

#define CFLAG  1
#define AFLAG  0

int Bflag;     /* flag de comptage des brackets */
int Lbrack;    /* # de { */
int Rbrack;    /* # de } */
int noligne;   /* # de ligne ou se situe l'appel */
int deja;
int fdW;
char refWork[80];


#define ERRMSG 4
#define NMXMSG 3

static int etat;

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

   FILE  *fp,*fpred;
   int    ret, fdconf, difference, bidon,
          checkfile, working,
          k, typ, i, err;

   struct OBJ errwin, *wmm, msgwin;


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


   i            = 0;
   wm           = &wmake;
   wmm          = &msgwin;

   ptr2 = " Checking API Consistency ";

   /* dupliquer l'environnement */
   dupEnviron();

   initBuffer();
   memset(buftoken, '\0', nb_car_per_line);


   errorlevel = 0;
   line = 0;

   /*                x  y   #c  #l */
   initWin(wm, ptr2, (80-strlen(ptr2))/2, 10, strlen(ptr2)+2, 3, TRUE, FALSE,
           vl->WINSYSink, vl->WINSYSpaper, vl->WINSYSborder, vl->WINSYSblock, W_WIN);

   winPopUp(wm, W_OPEN, TYP1);
   print_at(wm, 1, 1, ptr2, OLDATT);
   /* sauvegarder le fichier courant de maniere temporaire */
   argf[2] = wkfile;
   strcpy(wkfile,vl->absfname);
   refcall(wkfile);
   winPopUp(wm, W_CLOSE);
   vl->retCode = 0;
   vl->conf.buildDB = FALSE;
   inhibe_mouse();
   showMouse();
   free(TOKbuffer);
   free(buftoken);
   free(bufline);
/*   pushPop_MESS("Fin CHK API", ERRMSG);*/
   exit(0);
}
initBuffer()
{
    if ((buftoken = (int *)malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
    if ((bufline = (int *)malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
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

/***********************************************************************/

refcall(fname)
char *fname;
{
        int  cflag;
        char *pName;
        char nfname[64];

        memset(functab, 0, sizeof(struct defRef)*100);
        etat = deja = noligne = Rbrack = Bflag = Lbrack = 0;

        /* determiner le type de fichier */
        strcpy(nfname, fname);
        strupr(fname);
        if ((pName = strpbrk(nfname, ".")) == NULL) {
                pushPop_MESS(nfname, ERRMSG);
                return(-1);
        }

        cflag = TRUE;
        if ((Rbuf = malloc(nb_car_per_line)) == NULL) {
                pushPop_MESS(" Not enough memory ", ERRMSG);
                return(-1);
        }


        if (*(pName+1) == 'C') {
                if (get_function(fname, CFLAG)) {
                        free(Rbuf);
                        return(-1);
                }
                updateDECL();
        }
        else {
                cflag = FALSE;
                if (get_function(fname, AFLAG)) {
                        free(Rbuf);
                        return(-1);
                }
        }

        writeHeader();
        writeResult(cflag);

        /* fermer le fichier .REF */
        close(fdW);
        free(Rbuf);
/*        pushPop_MESS(" FIN REFCALL ", ERRMSG);*/
        return 0;
}

get_function(fname, Cflag)
char *fname;
{
    int  load, wSize, ret;
    char *pName, nfname[64], str[80];

    FILE *fp;

    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(str,refCallOpenf, fname);
        pushPop_MESS(str, ERRMSG);
        return(-1);
    }

    memset(nfname, '\0', 64);
    strcpy(nfname, fname);

    /* creer le fichier final */
    if ((pName = strpbrk(nfname, ".")) == NULL) {
        sprintf(str,refCallIncorr, fname);
        pushPop_MESS(str, ERRMSG);
        return(-1);
    }
    *(pName+1) = '\0';
    strcat(pName, "REF");

    remove(nfname);
    if ((fdW = open(nfname,O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE)) == -1) {
         pushPop_MESS(refCallwkf, ERRMSG);
         return(-1);
    }


    ret = TRUE;
    while (ret) {
        ret = getSrcLine(fp);      /* lire une ligne */
        if (Cflag)
                C_automate();
        else
                A_automate();
    }
    fclose(fp);
    return(0);
}

/*---------------------------------------------------
 * C_automate - Rbuf contient la ligne de source
 *---------------------------------------------------
 */
C_automate()
{
#define WAIT_LPARENTH   0
#define WAIT_STATE      1
#define WAIT_STAR1      2
#define WAIT_STAR2      3
#define WAIT_SLASH2     4

        int i, j, k;


        i = 0;
        j = strlen(Rbuf);

        while (i < j) {
        switch(etat) {

        case WAIT_LPARENTH: if (Rbuf[i] == '(') {
                                i--;
                                etat = WAIT_STATE;
                            }
                            else {
                                if (Rbuf[i] == '{' && Bflag) Lbrack++;
                                if (Rbuf[i] == '}' && Bflag) Rbrack++;
                                if (Rbuf[i] == '/') etat = WAIT_STAR1;
                                i++;
                            }
                            break;

        case WAIT_STATE   : k = i;

                            /* remonter sur la fin du nom de fonction */
                            while (i > 0 && Rbuf[i] == ' ') i--;
                            if (isalnum(Rbuf[i]))
                                writefuncName(i);
                            i = k+2;
                            Bflag = TRUE;
                            etat = WAIT_LPARENTH;
                            break;

        case WAIT_STAR1   : if (Rbuf[i] == '*') {
                                etat = WAIT_STAR2;
                                i++;
                            }
                            else
                                etat = WAIT_LPARENTH;
                            break;

        case WAIT_STAR2   : if (Rbuf[i] == '*')
                                etat = WAIT_SLASH2;
                            i++;
                            break;

        case WAIT_SLASH2  : if (Rbuf[i] == '/') {
                                etat = WAIT_LPARENTH;
                                i++;
                            }
                            else
                                etat = WAIT_STAR2;
                            break;

        default           : break;
        }
        }
}

/*---------------------------------------------------
 * A_automate - Rbuf contient la ligne de source
 *---------------------------------------------------
 */
A_automate()
{
        struct shvars Shl;   /* decomposition en Token */
        char buf[64];
        int i, ntok;


        ntok = lexan(&Shl);

        /* scanner les tokens */
        for (i = 0; i < ntok; i++) {
/*                pushPop_MESS(Shl.shtok[i], NMXMSG);*/
                if (Shl.shtok[i][0] == ';') /* skip commentaires */
                        break;
                if (stricmp(Shl.shtok[i], "EXTRN") == 0) {
                        strcpy(buf, Shl.shtok[i+1]);
                        recordName(buf, AFLAG, TYPE_EXT);
                        break;
                }

                if (stricmp(Shl.shtok[i], "PROC") == 0) {
                        strcpy(buf, Shl.shtok[i-1]);
                        recordName(buf, AFLAG, TYPE_DEF);
                        break;
                }
        }
}

/*---------------------------------------------------
 * getSrcLine - saisit une ligne de commande en
 *           supprimant CR et remplacant LF par '\0'
 *---------------------------------------------------
 */
getSrcLine(fp)
FILE *fp;
{
   char c;
   int input = 0;

   noligne++;
   /* saisir ligne en supprimant les CR */
   while ((c = getc(fp)) != '\n'&& (input < nb_car_per_line) && c != EOF)
             if (c != '\r')   Rbuf[input++] = c;

   if (c == '\n') {
              Rbuf[input++] = '\0';
              return(1);
   }
   else {
              Rbuf[input] = '\0';
              return(0);
  }
}

/*----------
 * writefuncName
 *----------
 */
writefuncName(i)
{
        char c, buf[64];
        int j, k, z;

        j = i;

        /* tester si Macro */
        z = 0;
        while (Rbuf[z++] == ' ');
        if (Rbuf[z] == '#')
                return;

        /* aller en debut de nom */
        while ((i > 0) && !(isspace((c = Rbuf[i])))) {
                if (!isalnum(c)) {
                        if (c == '_')
                                i--;
                        else {
                                i++;
                                break;
                        }
                }
                else
                        i--;
        }

        memset(buf,'\0',64);

        while (!isalnum(Rbuf[i]))
                if (Rbuf[i] == '_')
                        break;
                else
                        Rbuf[i++] = ' ';
        /* recopie */
        for (k = 0; i<=j; i++, k++)
                buf[k] = Rbuf[i];

        /*strupr(buf);*/

        for (i=0; i<CRESERVW; i++)
                if (strcmp(buf, CreservedWord[i]) == 0)
                        return;

        /* enregistrer le nom de la fonction */
        recordName(buf, CFLAG);

/*        if (Lbrack == Rbrack)
                sprintf(writeBuf, "%4d - DEF  :  %s\n", noligne, buf);
        else
                sprintf(writeBuf, "%4d - CALL :  %s\n", noligne, buf);

        printf("%s", writeBuf);*/

}

/*----------
 * recordName
 *----------
 */
recordName(buf, cflag, assTyp)
char *buf;
{
        int i, type;

        if (cflag) {
                if (Lbrack == Rbrack)   {
                        if (Lbrack == 0)
                                type = TYPE_DEC;
                        else
                                type = TYPE_DEF;
                }
                else {
                        updateDECL();
                        type = TYPE_EXT;
                }
        }
        else
                type = assTyp;

        strupr(buf);
        for (i = 0; (i < MAXFUNC) && (functab[i].type != TYPE_NUL); i++)

                if (strcmp(functab[i].funcName, buf) == 0) {
                        if ((type == TYPE_DEF) ||
                           ((type == TYPE_EXT) && (functab[i].type == TYPE_DEC))) {
                                functab[i].type     = type;
                                functab[i].position = noligne;
                                functab[i].ctype    = cflag;
                        }
                        return;
                }
        if (i == MAXFUNC) {
                pushPop_MESS(refCallOver, ERRMSG);
                return;
        }
        strcpy(functab[i].funcName, buf);
        functab[i].type     = type;
        functab[i].position = noligne;
        functab[i].ctype    = cflag;
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * CheckAPI -  verifie si la fonction n'est pas une fct de la lib Borland
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
CheckAPI(func, cflag, no)
char *func;
{
        int i, status, deb;
        char aux[80];
        char aux2[16];
        extern char *itoa();

        /* initialiser l'offset d'underscore */
        if (cflag)
                 deb = 1;
        else
                 deb = 0;
        for (i=0; strlen(_borlfunc[i]); i++) {
/*             pushPop_MESS(func, ERRMSG);
             pushPop_MESS(&_borlfunc[i][deb], ERRMSG);*/
             if ((status = stricmp(func, &_borlfunc[i][deb])) > 0)
                 continue;
             if (status == 0) {
                strcpy(aux, " (Line ");
                strcat(aux, itoa(no, aux2, 10));
                strcat(aux, ") : \'");
                strcat(aux, &_borlfunc[i][1]);
                strcat(aux, "\' is not allowed ");
                pushPop_MESS(aux , ERRMSG);
                return(-1);
             }
        }
        return(0);
}

/*----------
 * writeHeader
 *----------
 */
writeHeader()
{
        int i, NB, type;

        /* compter le nb de fonctions */
        for (i = NB = 0; i < MAXFUNC; i++)
                if ((type = functab[i].type) == TYPE_EXT || type == TYPE_DEF)
                        NB++;

        /* ecrire header */
        write(fdW, &NB, 2);

}
/*----------
 * writeResult
 *----------
 */
writeResult(cflag)
{
        int i;

        for (i=0;(i < MAXFUNC) && (functab[i].type != TYPE_NUL); i++) {
                CheckAPI(functab[i].funcName, cflag, functab[i].position);
                if (functab[i].type != TYPE_DEC) {
                        /* si fichier C, ajouter underscore au nom */
                        if (cflag) {
                                strcpy(refWork, "_");
                                strcat(refWork, functab[i].funcName);
                                strcpy(functab[i].funcName, refWork);
                        }
                        if (functab[i].type == TYPE_DEF)
                                functab[i].ctype = cflag;
                        write(fdW, &functab[i], sizeof(struct defRef));
                }

        }
        /* marquer la fin des fonctions */
        memset(&functab[0], 0xff, sizeof(struct defRef));
        write(fdW, &functab[0], sizeof(struct defRef));
}
/*----------
 * updateDECL
 *----------
 */
updateDECL()
{
        int i, k;

        if (deja)
                return;
        for (i=0, k=-1;functab[i].type != TYPE_NUL; i++)
                if (functab[i].type == TYPE_DEC)
                        k = i;
        if (k >= 0)
                functab[k].type = TYPE_DEF;
        deja = TRUE;
}

/*----------------------------------------------------------------------
 * lexan - ad hoc lexical analyser to divide command line into tokens
 *----------------------------------------------------------------------
 */
lexan( Shl )
struct shvars *Shl;
{
    char *line;
    char **tokptr;
    extern char *TOKbuffer;
    int  ntok;
    char *p;
    char ch;
    char *to;
    char quote;
    Shl->shbuf = TOKbuffer;
    memcpy(Shl->shbuf, Rbuf, SHBUFLEN);
    line = Shl->shbuf;
    to = Shl->shargst;                  /* area to place token strings */
    tokptr = &Shl->shtok[ntok = 0];     /* tableau de ptr sur TOKEN    */
    for (p = line; *p != '\0' && *p != '\n' && *p != '\r' && ntok < SHMAXTOK;)
    {
         while ( (ch = *p) == ' ')     /* sauter les blancs */
              p++;
         if (ch == '\0' || ch == '\n' || ch == '\r')  return(ntok);

         *tokptr++ = to;
         Shl->shtktyp[ntok++] = ch;    /* memoriser 1er carac du token */
         if (ch == '"' || ch == '\'')
         {
              quote = ch;
              for (p++; (ch = *p++) != quote && ch != '\n' && ch != '\0'; )
                                     *to++ = ch;
              if (ch != quote)       return(-1);
         }
         else
         {
              *to++ = *p++;
              if (ch != '>' && ch != '<' && ch != '&' && ch != '|')
                   while ((ch = *p) != '\n' && ch != '\0' && ch != '|' &&
                           ch != '<' && ch != '>' && ch != ' ' &&
                           ch != '"' && ch != '\'' && ch != '&')
                                        *to++ = *p++;   /* copy alphanum */
         }
         *to++ = '\0';
    }
    return(ntok);
}


/***********************************************************************/
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
            if ((wp->save = malloc(2 * (wp->ncol + 4) * (wp->nline + 3))) == NULL)  {
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


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
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

 if (mouse_present) {
        rin.x.ax  = 0x04;
        rin.x.cx  = newPosX;
        rin.x.dx  = newPosY;
        int86(0x33, &rin, &rout);
 }
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

         /* tester si on a utimis‚ la souris */
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
    strcpy(c_z, vl->conf.u.pd.c_I);      /* MKD include dir         */
    if (strlen(c_z))
        strcat(c_z, "\\");
    strcpy(c_l,  vl->conf.u.pd.c_l);     /* librairies dir      */
    if (strlen(c_l))
        strcat(c_l, "\\");
    strcpy(c_u,  vl->conf.u.pd.c_u);     /* ulk dir             */
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
