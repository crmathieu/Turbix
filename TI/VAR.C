/*----------------------------------------------------------------------*
 *       Fichier contenant les variables globales de l'application      *
 *----------------------------------------------------------------------*
 */

#include "xed.h"
#include "ext_var.h"

/************
   COUNTRY
 ************/

int  country = 0;    /* U.S. */


/* reponse YES / OUI */
char  y_o[2][4] = {{'Y','y','N','n'},{'O','o','N','n'}};


/************
   MENU3
 ************/
/* gestion de l'insertion du disque de travail par install.exe */
/* signature de dÇbut de zone : le dernier ÇlÇment est le checksum */
unsigned char debZone[8] = {168, 01, 172, 174, 149, 143, 239, 139};
/*                           ®        ¨    Æ    ï    è    Ô    ã   */
/*                          A8   01   AC   AE   95   8F   EF   8B  */

/* ÇlÇments Ö insÇrer */
char configFile[] = "c:\\tx\\ti\\ti.pro"; /* c par dÇfaut */

   unsigned char         arrLeft   = 0x1b,
                         arrRight  = 0x1a,
                         arrUp     = 0x18,
                         arrDwn    = 0x19;



/* Variable globale */
int      equip_list;       /* equipement HARDWARE de la machine */
int      nflp_phy;         /* nombre de floppy dans la machine */
int      adapter;          /* CGA,EGA,MONO */
unsigned vid_port;         /* adresse registre de controle 6845 */
unsigned videoBuf[2000];   /* buffer sauvegarde pour HELP */

/* pointeur sur la table des drives (qui est allouee dynamiquement) */
struct DriveTable *pdt;

/* type de compilateur */
int toggle = 0;
char CTM[2][30] = {" Product : Microsoft          ",
                   " Product : Borland            "};

/* fichier des erreurs */
char fError[] = "result.msg";

char *CompilerTradeMark[2];
/* init screen */
int xDos, yDos;
unsigned long BiosInt10, BiosInt33, BiosIntclk, DosInt24;
void interrupt int10Dummy();

struct pick  picktab[18];  /* pick table contenant les noms de fic absolus */
char         comptab[3][80];/* string du menu de compilation */

unsigned SEGvideo = 0;
unsigned far *Pvideo;

/* messages de l'interface A VOIR */

char   *fillstr;
struct OBJ mainMenu, fileMenu, bltMenu, pickMenu,
            optMenu,  helpMenu, instMenu,hardMenu,
            kbdMenu,  compMenu, editMenu, runMenu;

struct OBJ edwin,      /* window de l'editeur     */
           rdialwin,   /* window dialogue reduite */
           whelp;      /* fenetre d'aide          */

struct OBJ *pedwin;    /* pointe sur la fenetre courante d'edition */


unsigned char boxtype[7][11] = {{'⁄','ø','¿','Ÿ','≥','ƒ','√','¥','¬','¡','≈'},   /* type 1 */
                                {'’','∏','‘','æ','≥','Õ','∆','µ','—','œ','ÿ'},   /* type 2 */
                                {'÷','∑','”','Ω','∫','ƒ','«','∂','“','–','◊'},   /* type 3 */
                                {'…','ª','»','º','∫','Õ','Ã','π','À',' ','Œ'},   /* type 4 */
                                {'∞','∞','∞','∞','∞','∞','∞','∞','∞','∞','∞'},   /* type 5 */
                                {'«','∂','”','Ω','∫','ƒ','«','∂','“','–','◊'},   /* status */
                                {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},   /* external windows */
};


unsigned char fillpattern[] = {'∞','±','≤','€','‹','›','ﬁ','ﬂ'};


/* INITIALISATION TABLE D'ACCES */
struct ctab Ctab[] = { ALT_F,  NOTERMINAL, {0 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       F3,     TERMINAL,   {0 ,0 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       F4,     TERMINAL,   {2 ,1 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       F7,     TERMINAL,   {2 ,2 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       F8,     TERMINAL,   {2 ,3 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       F5,     TERMINAL,   {0 ,1 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       F2,     TERMINAL,   {0 ,6 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_F3, NOTERMINAL, {0 ,2 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_F10,TERMINAL,   {0 ,10,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_Q,  TERMINAL,   {0 ,11,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_E,  NOTERMINAL, {5 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_C,  NOTERMINAL, {2 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       F6,     TERMINAL,   {5 ,0 ,NP,NP,NP,NP,NP,NP,NP,NP},
/*                       F8,     TERMINAL,   {2 ,0 ,NP,NP,NP,NP,NP,NP,NP,NP},*/
                       ALT_P,  NOTERMINAL, {1 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       F9,     TERMINAL,   {1 ,6 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_X,  NOTERMINAL, {3 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_R,  NOTERMINAL, {3 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_I,  NOTERMINAL, {4 ,0 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_O,  NOTERMINAL, {4 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_H,  NOTERMINAL, {6 ,NP,NP,NP,NP,NP,NP,NP,NP,NP}, /* ver US */
                       ALT_A,  NOTERMINAL, {6 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       F1,     NOTERMINAL, {6 ,NP,NP,NP,NP,NP,NP,NP,NP,NP},
                       ALT_F1, TERMINAL,   {6 ,4 ,NP,NP,NP,NP,NP,NP,NP,NP},
                       NOKEY,  NOKEY,      {NP,NP,NP,NP,NP,NP,NP,NP,NP,NP}
};


/* lorsque la rubrique ne comporte pas de sous menu, la fonction
 * appliquÇe est terminale.
 */

/* MASTER MENU */
struct ITem mainstr[] = {{"File",             'F',ALT_F,5,0,1,&fileMenu,ScanVmenu},
                         {"Project",          'P',ALT_P,6,0,1,&bltMenu,ScanVmenu},
                         {"Compile",          'C',ALT_C,6,0,1,&compMenu,ScanVmenu},
                         {"Run",              'R',ALT_R,6,0,1,&runMenu,ScanVmenu},
                         {"Option",           'O',ALT_O,6,0,1,&optMenu,ScanVmenu},
                         {"Edit",             'E',ALT_E,6,0,1,&editMenu,ScanVmenu},
                         {"Help",             'H',ALT_H,6,0,1,&helpMenu,ScanVmenu},
                         {" ",                ' ',NOEXTC,3,0,0,NULL,NULL},
                         {"",                 ' ',NOEXTC,0,0,0,NULL,NULL}};

/* FILE MENU */
unsigned ReturnFromSaveFile = 0;  /* variable de positionnement sur retour SAVE FILE */
struct ITem filestr[] = {{" Open...            F3 ",'O',F3,     0,1, 0,NULL,doLoad},
                         {" previous File      F5 ",'F',F5,     0,10,0,NULL,doLoadlast},
                         {" Group...       ALT-F3 ",'G',ALT_F3, 0,1, 1,&pickMenu,ScanVmenu},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ",' ',NOEXTC, 0,0, 0,NULL,nullfunc},
                         {" Purge group           ",'P',NOEXTC, 0,1, 0,NULL,doClearPick},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ",' ',NOEXTC, 0,0, 0,NULL,nullfunc},
                         {" Save               F2 ",'S',F2,     0,1, 0,NULL,doSaveFile},
                         {" save As...            ",'A',NOEXTC, 0,6, 0,NULL,doSaveAs},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ",' ',NOEXTC, 0,0, 0,NULL,nullfunc},
                         {" Change dir...         ",'C',NOEXTC, 0,1, 0,NULL,doChgDir},
                         {" Dos           ALT-F10 ",'D',ALT_F10,0,1, 0,NULL,doShell},
                         {" Quit            ALT-Q ",'Q',ALT_Q,  0,1, 0,NULL,doQuit},
                         {"",                            ' ',NOEXTC, 0,0, 0,NULL,NULL}};

struct  MOUSE_FIELD mfDir[5];

/* COMPILE MENU */
/*struct ITem comstr[] = {{ "", 'C',NOEXTC,0,1,0,NULL,doCompile},
                         {"", 'A',NOEXTC,0,1,0,NULL,doAssemble},
                         {"", 'S',NOEXTC,0,1,0,NULL,doShowMess},
                         {"", ' ',NOEXTC,0,0,0,NULL,NULL}};*/

struct ITem comstr[] = {{ " Check API           ", 'h',NOEXTC,0,2,0,NULL,doCheck},
                         {" Compile          F4 ", 'C',NOEXTC,0,1,0,NULL,doCompile},
                         {" Assemble         F7 ", 'A',NOEXTC,0,1,0,NULL,doAssemble},
                         {" Link to TURBIX   F8 ", 'L',NOEXTC,0,1,0,NULL,doLink},
                         {" Show results        ", 'S',NOEXTC,0,1,0,NULL,doShowMess},
                         {"", ' ',NOEXTC,0,0,0,NULL,NULL}};

/* RUN MENU */
struct ITem runstr[] = {{ " run Project     ", 'P',NOEXTC,0,5,0,NULL,doRunPrj},
                         {" run Current exe ", 'C',NOEXTC,0,5,0,NULL,doRunExe},
                         {" command Line... ", 'L',NOEXTC,0,9,0,NULL,doLineCom},
                         {"", ' ',NOEXTC,0,0,0,NULL,NULL}};


/* HELP MENU */
struct ITem helpstr[]  = {{" Help on help            ", 'H',NOEXTC,0,1,  0,NULL,doHelp},
                          {" About TURBIX            ", 'A',NOEXTC,0,1,  0,NULL,doHelp},
                          {" Editor features         ", 'E',NOEXTC,0,1,  0,NULL,doHelp},
                          {" Make syntaxis           ", 'M',NOEXTC,0,1,  0,NULL,doHelp},
                          {" Library index by type...", 'L',ALT_F1,0,1,  0,NULL,doHelp},
                          {" alphabetical Index...   ", 'I',ALT_F1,0,14, 0,NULL,doHelp},
                          {" ASCII characters Set    ", 'S',NOEXTC,0,18, 0,NULL,doHelp},
                          {"",                         ' ',NOEXTC,0,  0,0,NULL,NULL}};



/* EDIT menu */
struct ITem editstr[]  = {{" Editor                  F6 ", 'E',F6    ,0,1,0,NULL,doEdit},
                          {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                          {" Find string...    Ctrl-Q-F ", 'F',NOEXTC,0,1,0,NULL,doFindstr},
                          {" find Next         Ctrl-L   ", 'N',NOEXTC,0,6,0,NULL,doFindnext},
                          {" Replace string... Ctrl-Q-A ", 'R',NOEXTC,0,1,0,NULL,doRepl},
                          {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                          {" Copy block        Ctrl-K-C ", 'C',NOEXTC,0,1,0,NULL,doDuplicate},
                          {" delete Block      Ctrl-K-Y ", 'B',NOEXTC,0,8,0,NULL,doDelete},
                          {" Move block        Ctrl-K-V ", 'M',NOEXTC,0,1,0,NULL,doMove},
                          {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                          {" Write block       Ctrl-K-W ", 'W',NOEXTC,0,1,0,NULL,doCopyToPaste},
                          {" read blOck        Ctrl-K-R ", 'O',NOEXTC,0,8,0,NULL,doReadFromPaste},
                          {" pAste file...              ", 'A',NOEXTC,0,2,0,NULL,doChgPaste},
                          {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                          {" Line length...    Ctrl-Q-L ", 'L',NOEXTC,0,1,0,NULL,doLineLength},
                          {" Goto line...      Ctrl-Q-G ", 'G',NOEXTC,0,1,0,NULL,doGoto},
                          {" Tabulation...              ", 'T',NOEXTC,0,1,0,NULL,doTabulation},
                          {"",                             ' ',NOEXTC,0,0,0,NULL,NULL}};


/* BUILD MENU */
struct ITem bltstr[]  = {{" Project file name...           ", 'P',NOEXTC,0,1,0,NULL,doChgMkfile},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                         {" Update REF database            ", 'U',NOEXTC,0,1,0,NULL,doBuildDB},
                         {" Go to declaration...   Ctrl-F2 ", 'G',NOEXTC,0,1,0,NULL,doGotoDef},
                         {" Function references... Ctrl-F3 ", 'F',NOEXTC,0,1,0,NULL,doDispRef},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                         {" Built project               F9 ", 'B',F9    ,0,1,0,NULL,doMake},
                         {"",                          ' ',NOEXTC,0,0,0,NULL,NULL}};


/* PICK MENU */
char picktitle[] = "Recent files";
char pickload[]  = " Open...        ";
struct pick pckdefault;

struct ITem pickstr[] = {{"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", 'H',NOEXTC,0,1,0,NULL,doPick},
                         {"", ' ',NOEXTC,0,0,0,NULL,NULL}};

/* INSTALLATION MENU */
struct ITem optstr[]  = {{" Installation... ", 'I',ALT_I ,0,1,1,&instMenu,ScanVmenu},
                         {" Parameters...   ", 'P',NOEXTC,0,1,0,NULL,doToolsChoice},

                         {"",                ' ',NOEXTC,0,0,0,NULL,NULL}};


struct ITem confstr[] = {{" Compiler/linker directory...     ", 'C',NOEXTC,0,1,0,NULL,doGetCoLnk},
                         {" Assembler directory...           ", 'A',NOEXTC,0,1,0,NULL,doGetAss},
                         {" C iNclude directory...           ", 'N',NOEXTC,0,4,0,NULL,doGetInc},
                         {" C Librairies directory...        ", 'L',NOEXTC,0,3,0,NULL,doGetLib},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                         {" TURBIX include/User directory... ", 'U',NOEXTC,0,16,0, NULL,doGetUsr},
/*                         {" TURBIX Include directory...     ", 'I',NOEXTC,0,5,0,NULL,doGetIncMkd},*/
                         {" TURBIX Libraries directory...    ", 'T',NOEXTC,0,1,0,NULL,doGetNmx},
                         {" Help files directory...          ", 'H',NOEXTC,0,1,0,NULL,doInstHelp},
                         {"ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ", ' ',NOEXTC,0,0,0,NULL,nullfunc},
                         {" Save configuration               ", 'S',NOEXTC,0,1, 0,NULL,doSconfig},
                         {"",                               ' ',NOEXTC,0, 0,0,NULL,NULL}};


/* YES / NO TOOGLE STRINGS */
char Yes[] = "Yes ";
char No[]  = "No  ";

/* LOGO */
char logoStr[] = " -- TURBIX Development Interface -- ";

int mouse_present, nbuttom;

/*********
  FTERM
 *********/

int nb_car_per_line;

#define HBYTE  0xff00
#define LBYTE  0x00ff


#define XFILE  60
char tabStr[]           = " Enter Tabulation - current is ";
char loadErrStr[]       = " exceeds 64Ko - Troncate ? ";
char expStr1[]          = " Expanding ";
char expStr2[]          = " exceeds 64Ko : Troncate ! ";
char chgDirStr[]        = " Enter New Dir Pathname ";
/*char chgDriveStr[]    = " Incorrect Drive Letter ";*/
char saveAsStr1[]       = " Copy to file "; /*"Save As";*/
char badDrvStr[]        = " Incorrect Drive Letter ";
char chgPaste[]         = " Enter Paste File ";
char confErrStr[]       = " Warning: no Profile file ";

/*********
  DIRECT
 *********/

#define ERROR_DSKEMPTY     0
#define ERROR_BADPATH     -1
#define ERROR_DSKERROR    -2
#define ERROR_DSKOVF      -3
#define ERRNO             -4

#define NOWRITEBOX  FALSE
#define WRITEBOX    TRUE

struct OBJ dirwin,
           dialwin;

/* structure stockant les fichiers par page */
struct dir_lst dir_lst;

/* liste chainee de tri des fichiers */
struct file_lst file_lst;

#define FILE_SIZE sizeof(struct file_lst)

int dir_lst_nfiles,  /* # de fichiers selectionnÇs dans la directory */
    dir_lst_npages;  /* # de pages de directory */

char  file_name[80], fpath[80], work[80];

char defaultMask[80];
char Cmask[] = "*.C";

/* */
char initRubStr[] = "file name : ";

char verifYN[]    = " Yes ( )  No  ( )     <   OK    > ";
char validYN[]    = "  <   OK    >  ";

/* messages gestion saisie du nom de fichier */
char AutoSearch[]  = " Auto Search     (Press space to use subset) ";
char loadDir[]     = " Loading directory . . .                     ";
char NewFile[]     = " New File : CREAT ? (y/n)   ";
char Search[]      = " Search                                      ";
char Enter[]       = " Enter File Name                             ";
char TooMany[]     = " Too Many Files PRESS any key ";
char NoFile[]      = " No File or Subdirectory for this mask. PRESS any key ";
char Verify[]      = " Insert diskette in Drive ( ) and PRESS any key ";
char BadPath[]     = " : Invalid Path. PRESS any key ";
char Savefile[]    = " Save file ";
char Question[]    = " ? ";
char Unkfile[]     = " Drive ( ) : Can't find file - ";
char Creatfile[]   = " - creat ?";

/* messages hardware */
char Dnotrdy[]     = " Drive ( )  not ready : Retry ? ";
char Unkmedia[]    = " Unknown media type ";
char Gnfail[]      = " General failure. PRESS any key ";

/* message erreurs systemes */
char Ens[]         = " Error not supported. PRESS any key ";
char Enoent[]      = " No such file or directory. PRESS any key ";
char E2big[]       = " Agument list too long. PRESS any key ";
char Enoexec[]     = " Exec format error. PRESS any key ";
char Ebadf[]       = " Bad file handle. PRESS any key ";
char Enomem[]      = " Not enought core. PRESS any key ";
char Eacces[]      = " Permission denied. PRESS any key ";
char Eexist[]      = " file already exists. PRESS any key ";
char Exdev[]       = " Cross device link. PRESS any key ";
char Einval[]      = " Invalid argument. PRESS any key ";
char Emfile[]      = " Too many open files. PRESS any key ";
char Enospc[]      = " Not enough SPACE - change support and PRESS any key ";
char Edeadlock[]   = " Resource deadlock would occur. PRESS any key ";

char *errno_tab[]  = {Ens,Ens,Enoent,Ens,Ens,Ens,Ens,E2big,Enoexec,Ebadf,Ens,Ens,
                      Enomem,Eacces,Ens,Ens,Ens,Eexist,Exdev,Ens,Ens,Ens,Einval,
                      Ens,Emfile,Ens,Ens,Ens,Enospc,Ens,Ens,Ens,Ens,Ens,Ens,Ens,
                      Edeadlock};

char *clippbuf;

int curr_floppy = CURRDRIVE_A;

struct OBJ *messwin;
int    Xmess, Ymess;
int Yes_answ, No_answ;
struct file_lst head, tail;

/**********
  DISPLAY
 **********/

char *lineTab;
int tablength = 8; /* valeur tabulation par defaut */

char lineTooLongStr[] = " line too long - Press ESC to insert LF ";

/**********
  EDMAIN2
 **********/

int  activWindow = 0;
/*char status[]    =  "  xL.vvvvxxR.vvvxx%.vvx"*/
char status[]      =  "  Line       Col                                                                ";
char Save[]        =  " Save file ";
char Load[]        =  " Load file ";

char insStr[]      =  "Insert ";

char ltoobig[]     =  " Line is too long ... PRESS any key to troncate ";
char outofmem[]    =  " Not enough MEMORY ... PRESS any key ";
char strNotFound[] =  " String not found ... PRESS any Key ";

char errmalloc[]   =  " Not enough memory to run TI - Exit to DOS ";
char findCs[]      =  " Find:            ( ) Case Sensitive ";
char find[]        =  " Find:                               ";
char replace[]     =  " Replace by :                   ";

char *rplmode[4]   = {"  ( ) Change ALL and VERIFY   ",
                      "  ( ) Change ALL              ",
                      "  ( ) Change and VERIFY       ",
                      "  ( ) Change                  "};
char caseSens[]    =  "                  case        ";
char validRP[]     =  "  <   OK    >   Sensitive ( ) ";
char YesNo[]       =  " Replace String ? (Y/N) ";
char AskYN[]       =  "    <  Yes  >    <  No   >    ";

char lines[6],
     cols[6],
     perc[6];

/* gestion algorithme du FIND / REPLACE */
char findSTR[80], replaceSTR[80];

#define  XLINE  7
#define  XCOL  16
#define  XPER  22
#define  XINS  26

#define  FIND      1
#define  REPLACE   2


unsigned special_key();

/***********
  STARTUP2
 ***********/

int menu_deepth = 0;

/* directorie courante et nom absolu du fichier courant */
char  absfname[80],
      relname[80],
      cdirname[80],
      exename[80],
      mask[80],
      pastefile[80],
      homedir[80],
      nullstr[1];

#define HBYTE  0xff00
#define LBYTE  0x00ff

/* variables definissant les attributs video */
unsigned  char   WINink,         /* avant plan fenetre */
                 WINpaper,       /* arriere plan fenetre */
                 WINborder,      /* cadre fenetre        */
                 WINblock,       /* block dans fenetre   */
                 WINmess,        /* message dans fenetre */
                 WINSYSink,      /* A.P. fenetre systeme */
                 WINSYSpaper,    /* A.R. fenetre systeme */
                 WINSYSborder,
                 WINSYSblock,
                 WINmessInk,
                 WINmessPaper,
                 WINmessBorder,
                 WINDIRink,
                 WINDIRpaper,
                 /* fenetre d'AIDE */
                 WINHLPink,
                 WINHLPpaper,
                 WINHLPborder,
                 WINHLPblock,

                 MENUborder,     /* cadre menu */
                 MENUselected,   /* article selectionnÇ */
                 MENUlcom,       /* lettre de commande */
                 MENUink,        /* avant plan menu */
                 MENUpaper,      /* arriere plan menu */
                 MESS24att,      /* att me ages editeur */
                 MESSatt;        /* att window message */


/* VARIABLES de travail de l'editeur */
int        fdcurr;     /* file descriptor du fichier chargÇ        */
char       *linebuf;   /* tampon ligne             */
char  far *bigbuf;     /* buffer de reception du fichier           */
unsigned   fsize,      /* taille actuelle du fichier en memoire    */
           bottom,     /* offset derniere ligne                    */
           current,    /* offset ligne courante                    */
           topPage,    /* offset ligne en sommet de fenetre        */
           topBlock,   /* offset ligne de debut bloc               */
           bottomBlock;/* offset ligne de fin de block             */
int        fillCh,     /* # Blancs ajoutÇs apres une fin de ligne  */
           fflag,      /* gestion etat du fichier dans l'editeur   */
           blockatt,   /* attribut block defini                    */
           current_line_no, /* no de la ligne courante             */
           topPage_line_no, /* no de ligne du sommet de page       */
           bottom_line_no;  /* no de ligne de le derniere ligne    */

struct varLink *vl;

char errToolsStr[] = " Cannot run development tools ... ";
char errKernelStr[]   = " Cannot run : ";
/*unsigned char cp0412[] = {0x20, 0x42, 0x6D, 0x73, 0x7D,    /* copyright */
/*                          0x77, 0x6F, 0x60,                /* copyright */
/*                          0x60,                            /* copyright */
/*                          0x7D, 0x2A,                      /* copyright */
/*                          0x23, 0x4F, 0x24, 0x2E, 0x5F,    /* copyright */
/*                          0x62, 0x7E, 0x76, 0x66, 0x77,    /* copyright */
/*                          0x61, 0x7F, 0x78, 0x76, 0x39,    /* copyright */
/*                          0x57, 0x6E, 0x70, 0x69, 0x77,    /* copyright */
/*                          0x3F, 0x6D, 0xA3, 0x46, 0x4A,    /* copyright */
/*                          0x45, 0x05, 0x17, 0x1E, 0x11,    /* copyright */
/*                          0x19, 0x0A, 0
/*};*/

unsigned char cp0412[] = {
 0x20, 0x40, 0x77, 0x77, 0x6c, 0x6a, 0x74, 0x3d, 0x28, 0x4a,
 0x62, 0x6a, 0x7e, 0x61, 0x6b, 0x7c, 0x30, 0x5c, 0x53, 0x47,
 0x5c, 0x5c, 0x53, 0x42, 0x38, 0
};

char Corp[] = { /* str =     Multitasking kernel for Dos     */
 0x20, 0x21, 0x22, 0x23, 0x24, 0x48, 0x73, 0x6b, 0x7c, 0x60,
 0x7e, 0x6a, 0x7f, 0x66, 0x67, 0x61, 0x77, 0x31, 0x79, 0x76,
 0x66, 0x7b, 0x73, 0x7b, 0x38, 0x7f, 0x75, 0x69, 0x3c, 0x59,
 0x71, 0x6c, 0x0, 0x1, 0x2, 0x3, 0x4, 0
};

char productName[] = { /* str =    Interface (M.D.I)  */
 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
 0x43, 0x65, 0x78, 0x68, 0x7c, 0x69, 0x71, 0x72, 0x77, 0x33,
 0x3c, 0x58, 0x38, 0x53, 0x36, 0x50, 0x33, 0x3b, 0x3c, 0x3d,
 0x3e, 0x3f, 0x0, 0x1, 0x2, 0x3, 0x4, 0
};
/*char _lg56[] = { /*  Development Interface (DI) */
/*0x20,0x45,0x67,0x75,0x61,0x69,0x69,0x77,0x65,0x6c,0x64,0x7f,
0x2c,0x44,0x60,0x7b,0x75,0x63,0x74,0x72,0x77,0x70,0x36,0x3f,
0x5c,0x50,0x33,0x3b, 0
};*/
char _lg56[] = { /*  TURBIX development Interface  */
0x20,0x55,0x57,0x51,0x46,0x4c,0x5e,0x27,0x6c,0x6c,0x7c,0x6e,
0x60,0x62,0x7e,0x62,0x75,0x7f,0x66,0x33,0x5d,0x7b,0x62,0x72,
0x6a,0x7f,0x7b,0x78,0x79,0x3d,0
};


/**********
  BIGBUF3
 **********/

/*********
  BLOCK5
 *********/

char readblk[]  = " Reading from paste ";
char writeblk[] = " Writing to paste ";
char cpyblk[] = " ";

char cpyPasteStr[] = " can't open paste file ";

/*********
  CONFIG
 *********/
char envIncompletStr[] = " Environment not enough initialized - Press any key ";
char turbocStr[]       = " Cannot create compiler configuration file ";
char builtStr[]        = " Cannot open redirection file ";
#define HBYTE  0xff00
#define LBYTE  0x00ff
#define FIND   1

/* type d'operation : Make, Compile, Link, assemble */
int operation;

#define MKE  0x0100
#define COMP 0x0200
#define LNK  0x0300
#define ASS  0x0400

#define BERR        1
#define MKE_NODIFF  2
#define BEXIT       3

/*  gestion des parametres lus dans le
 *  fichier de configuration "xconfig"
 */

struct configuration conf;


/**********
  ITURBO
 **********/

/* interface entre compilateurs microsoft et Borland :
 * Adaptation des fonctions du turbo Ö la syntaxe
 * Microsoft
 */

#define NDRIVE 3

#ifdef TURBO

#define diskfree_t           dfree
#define total_clusters       df_total
#define avail_clusters       df_avail
#define sectors_per_cluster  df_sclus
#define bytes_per_sector     df_bsec
#endif

/**********
   MOUSE
 **********/

#define  BIOSTAB 0x00400000   /* adresse debut table du BIOS */
#define  HEADptr 0x0040001a   /* adresse pointeur de queue  */
#define  TAILptr 0x0040001c   /* adresse pointeur de tete   */
#define  KEYbuf  0x0040001e   /* adresse du buffer clavier */
#define  LEN     0x10         /* longueur du buffer (en word) */
#define  KSOP    0x00400080   /* key buf start offset pointer */
#define  KEOP    0x00400082   /* key buf end offset pointer */


struct mouse_block_interface ib;

/* gestion des marquages de blocs par la souris */
int XMOUSE, YMOUSE; /* Enregistre la position courante sur pression G */
unsigned CMOUSE;    /* Enregistre l'offset de la ligne dans le
                     * fichier sur pression G
                     */
int LMOUSE;         /* enregistre le # de caracteres non visibles a gauche */
unsigned mouseClic;
int      mouseEvent;

char *ibad;
struct OBJ *currObj;          /* pointe sur l'objet courant */
struct OBJ *headObj;          /* 1er objet de la liste */
struct OBJ *tailObj;          /* dernier objet de la liste */
unsigned mouseBuffer[16];     /* buffer stockage ev souris */
int M_in,M_out;               /* pointeurs du buffer */

/**********
  CVIDEO
 **********/


int mouse_state = 0;
int mouseEmulate = 0;
int posX, posY;         /* position courante de la souris */
unsigned char ticks = 0;/* nb de ticks d'horloge */

/*********
  HELP
 *********/
struct save_edit sedHelp;

char hlpSelectStr[] = "Help Select";
char hlpStr[] = "Help";
char helpErrStr[] = " Help files not found - Run Help installation ";

char *helpFName[] = {"help.hlp","pre@@.hlp","ed.hlp",
                     "make.hlp","index.hlp", "ind@@.hlp", "asc@@.hlp"};

char defhlp[80];
int helpInLine;

char UnknownStr[] = " Unknown or unsupported function ";

char  *hlpFile[] = { /* fonction           -          Fichier */

                    /* Process */
/*                    "m_Disable",                   "h13.hlp",*/
/*                    "m_Enable",                    "h15.hlp",*/
/*                    "m_Execl",                     "h16.hlp",*/
                    "m_Exec",                      "h16.hlp",
                    "m_Fork",                      "h17.hlp",
                    "m_GetPriority",               "h69.hlp",
                    "m_Getpid",                    "h20.hlp",
                    "m_GetProcName",               "h68.hlp",
                    "m_Getppid",                   "h22.hlp",
                    "m_Nice",                      "h39.hlp",
/*                    "m_Restore",                   "h48.hlp",*/
                    "m_Shutdown",                  "h70.hlp",
                    "m_Alarm",                     "h1.hlp",
                    "m_Kill",                      "h27.hlp",
                    "m_Pause",                     "h41.hlp",
                    "m_SetProcName",               "h681.hlp",
                    "m_Signal",                    "h53.hlp",
                    "m_Wait",                      "h61.hlp",
                    "m_Exit",                      "h64.hlp",

                    /* souris */
/*                    "m_OpenMou",                "h80.hlp",*/
/*                    "m_CloseMou",               "h81.hlp",*/
/*                    "m_ShowMou",                "h82.hlp",*/
/*                    "m_HideMou",                "h83.hlp",*/
/*                    "m_ReadEventMou",           "h84.hlp",*/
/*                    "m_EventMaskMou",           "h85.hlp",*/

                    /* SÇmaphore */
                    "m_Countsem",                  "h7.hlp",
                    "m_Creatsem",                  "h9.hlp",
                    "m_Delsem",                    "H12.hlp",
                    "m_Pwaitsem",                  "h45.hlp",
                    "m_Resetsem",                  "h47.hlp",
                    "m_Sigsem",                    "h54.hlp",
                    "m_Waitsem",                   "h62.hlp",

                    /* Message */
                    "m_Msgclr",                    "h31.hlp",
/*                    "m_Msgctl",                    "h32.hlp",
                      "m_Msgget",                    "h33.hlp",
                      "m_Msgrcv",                    "h34.hlp",*/
                    "m_Msgrdv",                    "h35.hlp",
/*                    "m_Msgsnd",                    "h36.hlp",*/
                    "m_Msgsync",                   "h37.hlp",
                    "m_Msgwait",                   "h38.hlp",

                    /* MÇmoire */
                    "m_AdjustPTR",                 "h0.hlp",
                    "m_Free",                   "h65.hlp",
                    "m_Malloc",                 "h66.hlp",

                    /* Time & Delay */
                    "m_DateTime",                  "h11.hlp",
                    "m_Time",                      "h59.hlp",
                    "m_Gsleep",                    "h25.hlp",
                    "m_Sleep",                     "h55.hlp",
                    "m_Wakeup",                    "h63.hlp",

                    /* VidÇo */
                    "m_Beep",                      "h2.hlp",
                    "m_ChgActiveSession",          "h5.hlp",
                    "m_CursorShape",               "h10.hlp",
                    "m_Getpos",                    "h21.hlp",
                    "m_GetSession",                "h23.hlp",
                    "m_GetProcSessionHandle",      "h241.hlp",
                    "m_Gotoxy",                    "h71.hlp",
                    "m_Setcva",                    "h50.hlp",
                    "m_Flush",                     "h501.hlp",

                    /* Device */
                    "m_Setdrv",                    "h3.hlp",
                    "m_Ioctl",                     "h26.hlp",
                    "m_SetErrHandler",             "h261.hlp",

                    /* E/S Disque */
                    "m_Chdir",                     "h4.hlp",
                    "m_Mkdir",                     "h30.hlp",
                    "m_Getcwd",                    "h301.hlp",

                    /* E/S */
                    "m_Close",                     "h6.hlp",
                    "m_Creat",                     "h8.hlp",
                    "m_Dup",                       "h14.hlp",
                    "m_Dup2",                      "h14.hlp",
                    "m_Errno",                     "h141.hlp",
                    "m_Fprintf",                   "h43.hlp",
                    "m_Fscanf",                    "h49.hlp",
                    "m_Getc",                      "h18.hlp",
                    "m_Getch",                     "h18.hlp",
                    "m_SessionGetch",              "h18.hlp",
/*                    "getchar",                   "h18.hlp",*/
                    "m_Getche",                    "h18.hlp",
                    "m_Lock",                      "h28.hlp",
                    "m_Lseek",                     "h29.hlp",
                    "m_Open",                      "h40.hlp",
                    "m_Perror",                    "h401.hlp",
                    "m_Pipe",                      "h42.hlp",
                    "m_Printf",                    "h43.hlp",
                    "m_Putc",                      "h44.hlp",
                    "m_Read",                      "h46.hlp",
                    "m_Remove",                    "h461.hlp",
                    "m_Scanf",                     "h49.hlp",
                    "m_Seprintf",                  "h43.hlp",
                    "m_Sgetc",                     "h51.hlp",
                    "m_Sgetch",                    "h51.hlp",
                    "m_Sgetche",                   "h51.hlp",
                    "m_Sscanf",                    "h49.hlp",
                    "m_Sprintf",                   "h43.hlp",
                    "m_Tell",                      "h58.hlp",
                    "m_Unlock",                    "h60.hlp",
                    "m_Write",                     "h67.hlp",

                    /* Fenetres */
                    "m_box",                       "w0.hlp",
                    "m_endwin",                    "w2.hlp",
                    "m_delwin",                    "w1.hlp",
                    "m_getyx",                     "w3.hlp",
                    "m_initscr",                   "w4.hlp",
                    "m_initStdscrPTR",             "w40.hlp",
                    "m_mvwin",                     "w5.hlp",
                    "m_newwin",                    "w6.hlp",
/*                    "m_subwin",                    "w7.hlp",*/
                    "m_touchwin",                  "w8.hlp",
                    "m_waddch",                    "w9.hlp",
                    "m_waddstr",                   "w90.hlp",
/*                    "m_wattach",                   "w99.hlp",*/
                    "m_wautocrlf",                 "w10.hlp",
                    "m_wclrtobot",                 "w11.hlp",
                    "m_wclrtoeol",                 "w12.hlp",
                    "m_wdelch",                    "w13.hlp",
                    "m_wdeleteln",                 "w14.hlp",
                    "m_wecho",                     "w15.hlp",
                    "m_werase",                    "w16.hlp",
                    "m_wgetch",                    "w161.hlp",
                    "m_winch",                     "w17.hlp",
                    "m_winsch",                    "w18.hlp",
                    "m_winsertln",                 "w19.hlp",
                    "m_wmove",                     "w20.hlp",
                    "m_wpop",                      "w21.hlp",
                    "m_wprintw",                   "w22.hlp",
                    "m_wpush",                     "w23.hlp",
                    "m_wrefresh",                  "w24.hlp",
                    "m_wresize",                   "w95.hlp",
                    "m_wscroll",                   "w27.hlp",
                    "m_wscanw",                    "w25.hlp",
                    "m_wselect",                   "w26.hlp",

                    /* divers */
                    "m_Spy",                     "h52.hlp",

                    /* limite basse */
                    "",                          "h73.hlp"
};

/*******
  WLINK
 *******/


char lnkChoiceStr[] = "Link Parameters";
char lnkChoiceInt[] = "  ( ) Link with SPY interpreter          ";
char lnkChoiceFS[]  = "  ( ) Link with File System              ";
char lnkAutoAsk[]   = "  ( ) Parameters checking before linking ";
char lnkChoiceRep[] = "  <   OK    >    < Cancel  > ";
int mkd_shell, mkd_FS, mkd_AutoAsk;

/*******
  WTOOLS
 *******/

char tooChoiceStr[] = "Environment";
char tooChoiceCmp[] = "  Compiler needed : Borland Turbo C ";
char tooChoiceAs1[] = "  ( ) Microsoft Assembler (MASM) ";
char tooChoiceAs2[] = "  ( ) Borland Assembler (TASM) ";
char tooKbdFastStr[]= "  ( ) Fast Keyboard  ";
char tooAutoSavStr[]= "  ( ) Environment auto save ";

int mkd_masm, mkd_tasm;
int rmode, replaceMode, caseSensitive;

/**********
  FINDFUNC
 **********/

char gotoFuncNameStr[] = " Go to Def  : Enter function name ";
char dispFuncNameStr[] = " References : Enter function name ";
char dbaseOpenErr[] = " DBASE opening failure ";
char notUserStr[] = " Not a user function ";
char rebuildStr[] = " Project has been modified - Press any key ";
char buildDBstr[] = " Run Update Reference DATABASE menu ";
