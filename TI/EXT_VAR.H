/*----------------------------------------------------------------------*
 *       Fichier contenant les declarations externes de toutes les      *
 *       variables globales de l'application                            *
 *----------------------------------------------------------------------*
 */

/* type de messages */
#define FNDREP 5  /* find replace */
#define ERRMSG 4
#define NMXMSG 3
#define WINMSG 2
#define NOMSG  0

/************
   COUNTRY
 ************/

extern int  country;


/* reponse YES / OUI */
extern char  y_o[2][4];



/************
   MENU3
 ************/

extern char configFile[];

extern unsigned char     arrLeft, arrRight, arrUp, arrDwn;


#define BARRE_SIZE   10


/* master menu */
#define  ALT_F  33
#define  ALT_E  18
#define  ALT_R  19
#define  ALT_P  25
#define  ALT_X  45
#define  ALT_A  30
#define  ALT_C  46
#define  ALT_B  48
#define  ALT_O  24
#define  ALT_H  35
#define  ALT_Q  16
#define  ALT_I  23


/* INITIALISATION MENUS */
extern nullfunc(),   doFile(),    doEdit(),    doCompile(), doRun(),
       doOption(),   doQuit(),    ScanVmenu(), doLoad(),    doLoadlast(),
       doSaveFile(), doPick(),    doKbSpeed(), doCompile(), doAutoSave(),
       doChgMke(),   doSconfig(), doAssemble(),doMake(),    doLink(),
       doShowMess(), doPreScan(), doShell(),   doChgDir(),  doSaveAs(),
       doChgPaste(), doHelp(),    doFindstr(), doFindnext(),doRepl(),doGetUsr(),
       doGoto(), doChgMkfile(),   doTabulation(), doCopyToPaste(),doClearPick(),
       doReadFromPaste(),doDuplicate(), doMove(), doDelete(), doInstHelp(),
       doToolsChoice(),doGetCoLnk(), doGetAss(), doGetInc(), doGetLib(),
       doGetNmx(), doGetIncMkd(), doBrowse(), doGotoDef(), doDispRef(),
       doBuildDB(), doRunExe(), doRunPrj(), doLineLength(), doCheck();


/* Variable globale */
extern int      equip_list;       /* equipement HARDWARE de la machine */
extern int      nflp_phy;         /* nombre de floppy dans la machine */
extern int      adapter;          /* CGA,EGA,MONO */
extern unsigned vid_port;         /* adresse registre de controle 6845 */
extern unsigned videoBuf[2000];   /* buffer sauvegarde pour HELP */
/* Gestion des drives */
struct DriveTable {
        char drive;
        char valide;
        char *path;
};
/* pointeur sur la table des drives (qui est allouee dynamiquement) */
extern struct DriveTable *pdt;

extern struct OBJ *currObj,       /* pointeur sur l'objet courant */
                  *headObj,
                  *tailObj;
/* type de compilateur */
extern int toggle;
extern char CTM[2][30];

/* fichier des erreurs */
extern char fError[];

extern char *CompilerTradeMark[];

extern int xDos, yDos;
extern unsigned long BiosInt10, BiosInt33, BiosIntclk, DosInt24;
extern void interrupt int10Dummy();

extern struct pick  picktab[];  /* pick table contenant les noms de fic absolus */
extern char         comptab[3][80]; /* string du menu de compilation */

extern unsigned SEGvideo;
extern unsigned far *Pvideo;

/* messages de l'interface A VOIR */

extern char   *fillstr;
extern struct OBJ mainMenu, fileMenu, bltMenu, pickMenu,
            optMenu,  helpMenu, instMenu, runMenu,
            hardMenu, kbdMenu,  compMenu, editMenu;


extern struct OBJ edwin,      /* window de l'editeur     */
                  rdialwin,   /* window dialogue reduite */
                  whelp,      /* fenetre d'aide   */
                  *pedwin;    /* fenetre d'edition courante */


extern unsigned char boxtype[7][11];

extern unsigned char fillpattern[];


/* INITIALISATION TABLE D'ACCES */
extern struct ctab Ctab[];


/* lorsque la rubrique ne comporte pas de sous menu, la fonction
 * appliqu‚e est terminale.
 */

/* MASTER MENU */
extern struct ITem mainstr[];

/* FILE MENU */
extern unsigned ReturnFromSaveFile;
extern struct ITem filestr[];
extern struct  MOUSE_FIELD mfDir[];

/* EDIT MENU */
extern struct ITem editstr[];

/* COMPILE MENU */
extern struct ITem comstr[];

/* HELP MENU */
extern struct ITem helpstr[];

/* BUILD MENU */
extern struct ITem bltstr[];

/* RUN MENU */
extern struct ITem runstr[];

/* PICK MENU */
extern char picktitle[];
extern char pickload[];
extern struct pick pckdefault;

extern struct ITem pickstr[];

/* INSTALLATION MENU */
extern struct ITem optstr[];

extern struct ITem confstr[];

/* YES / NO TOOGLE STRINGS */
extern char Yes[];
extern char No[];
/* LOGO */
extern char logoStr[];

extern int mouse_present, nbuttom;

/*********
  FTERM
 *********/

extern int nb_car_per_line;

#define HBYTE  0xff00
#define LBYTE  0x00ff
#define XFILE  60

/* autoriser ou non la craetion de fichier dans loadFile */
#define V_CREAT    1
#define V_NOCREAT  2

extern unsigned doEnter();
extern int doLineCom();

extern char tabStr[], loadErrStr[], expStr1[], expStr2[],
                      chgDirStr[], chgDriveStr[], saveAsStr1[],
                      badDrvStr[], chgPaste[], confErrStr[];


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

#define FILE_SIZE sizeof(struct file_lst)

/* gestion saisie */
#define NOCLEAR  FALSE
#define CLEAR    TRUE

#define  REP_REQ     1   /* reponse requise */
#define  NO_REP_REQ  2   /* pas de reponse requise */

extern int Yes_answ, No_answ;

extern struct OBJ dirwin,
           dialwin;


extern struct dir_lst dir_lst;

extern struct file_lst file_lst;


extern int dir_lst_nfiles,  /* # de fichiers selectionn‚s dans la directory */
    dir_lst_npages;  /* # de pages de directory */

extern char  file_name[], fpath[], work[];

extern char defaultMask[], Cmask[];

/************/
extern char initRubStr[];


/* messages gestion saisie du nom de fichier */
extern char AutoSearch[];
extern char loadDir[];
extern char NewFile[];
extern char Search[];
extern char Enter[];
extern char TooMany[];
extern char NoFile[];
extern char Verify[];
extern char BadPath[];
extern char Savefile[];
extern char Question[];
extern char Unkfile[];
extern char Creatfile[];

/* messages hardware */
extern char Dnotrdy[];
extern char Unkmedia[];
extern char Gnfail[];

/* message erreurs systemes */
extern char Ens[];
extern char Enoent[];
extern char E2big[];
extern char Enoexec[];
extern char Ebadf[];
extern char Enomem[];
extern char Eacces[];
extern char Eexist[];
extern char Exdev[];
extern char Einval[];
extern char Emfile[];
extern char Enospc[];
extern char Edeadlock[];

extern char *errno_tab[];

extern char *clippbuf;
extern char verifYN[];
extern char validYN[];
extern char caseSens[];
extern char validRP[];

extern int curr_floppy;

extern struct OBJ *messwin;
extern int    Xmess, Ymess;
extern struct file_lst head, tail;

/**********
  DISPLAY
 **********/

#define OBJ_CHAINAGE  1
#define OBJ_FREE      0

extern char *lineTab;
extern int tablength;
extern char lineTooLongStr[];

/**********
  EDMAIN2
 **********/


/* etats d'un fichier */

#define   FF_DIRTY          1   /* fichier modifi‚ */
#define   FF_INSERT         2   /* mode INSERT / OVERWRITE */
#define   FF_FLOADED        4   /* un fichier est deja charg‚ */
#define   FF_EDACTIF        8   /* Editeur Actif */
#define   FF_BLOCKDEF      16   /* block defined */
#define   FF_LINE_UPDATE   32   /* ligne courante en cours de modification */
#define   FF_BOTB_ENDLINE  64   /* marque bottom block en fin de ligne */
#define   FF_TOPB_ENDLINE  128  /* marque top block en fin de ligne */
#define   FF_NEWFILE       256  /* nouveau fichier */
#define   FF_TAB_ON_LINE   512  /* caractere de tabulation sur la ligne */
#define   FF_TMPFILE      1024  /* fichier temporaire */
#define   FF_INHIBE_MOD   2048  /* interdire modifications */
#define   FF_SAISIE_MOD   4096  /* mode saisie sur une ligne */
#define   FF_HELP_MOD     8192  /* help edit mode */
#define   FF_NO_GROUP    16384  /* pas de chargement dans le PICK */

/*
#define  XLINE  7
#define  XCOL  16
#define  XPER  22
#define  XINS  26
*/
#define  XLINE  8
#define  XCOL  17
#define  XPER  22
#define  XINS  26

#define  FIND      1
#define  REPLACE   2




extern int  activWindow;
extern char status[];
extern char Save[];
extern char Load[];

extern char insStr[];

extern char ltoobig[];
extern char outofmem[];
extern char strNotFound[];
extern char YesNo[];
extern char errmalloc[];
extern char find[];
extern char findCs[];
extern char replace[];
extern char *rplmode[];
extern char AskYN[];

extern char lines[6],
     cols[6],
     perc[6];

/* gestion algorithme du FIND / REPLACE */
extern char findSTR[], replaceSTR[];

extern unsigned special_key();

/***********
  STARTUP2
 ***********/

#define HBYTE  0xff00
#define LBYTE  0x00ff



extern int menu_deepth;

/* directorie courante et nom absolu du fichier courant */
extern char  absfname[],
      relname[],
      cdirname[],
      exename[],
      mask[],
      pastefile[],
      homedir[],
      nullstr[];

/* variables definissant les attributs video */
extern unsigned  char   WINink,         /* avant plan fenetre */
                 WINpaper,       /* arriere plan fenetre */
                 WINborder,      /* cadre fenetre        */
                 WINblock,       /* block dans fenetre   */

                 WINSYSink,      /* A.P. fenetre systeme */
                 WINSYSpaper,    /* A.R. fenetre systeme */
                 WINSYSborder,
                 WINSYSblock,

                 WINHLPink,
                 WINHLPpaper,
                 WINHLPborder,
                 WINHLPblock,

                 WINmessInk,
                 WINmessPaper,
                 WINmessBorder,
                 WINDIRink,
                 WINDIRpaper,
                 MENUborder,     /* cadre menu */
                 MENUselected,   /* article selectionn‚ */
                 MENUlcom,       /* lettre de commande */
                 MENUink,        /* avant plan menu */
                 MENUpaper,      /* arriere plan menu */
                 MESS24att,      /* att messages editeur */
                 MESSatt;        /* att window message */


/* VARIABLES de travail de l'editeur */
extern int        fdcurr;     /* file descriptor du fichier charg‚        */
extern char      *linebuf;    /* tampon ligne             */
extern char  far *bigbuf;     /* buffer de reception du fichier           */
extern unsigned   fsize,      /* taille actuelle du fichier en memoire    */
           bottom,     /* offset derniere ligne                    */
           current,    /* offset ligne courante                    */
           topPage,    /* offset ligne en sommet de fenetre        */
           topBlock,   /* offset ligne de debut bloc               */
           bottomBlock;/* offset ligne de fin de block             */
extern int fillCh,     /* # Blancs ajout‚s apres une fin de ligne  */
           fflag,      /* gestion etat du fichier dans l'editeur   */
           blockatt,   /* attribut block defini                    */
           current_line_no, /* no de la ligne courante             */
           topPage_line_no, /* no de ligne du sommet de page       */
           bottom_line_no;  /* no de ligne de le derniere ligne    */

extern struct varLink *vl;


extern char errToolsStr[], errKernelStr[];
extern unsigned char cp0412[];
extern char Corp[];
extern char productName[];
extern char _lg56[];


/**********
  BIGBUF3
 **********/

/*********
  BLOCK5
 *********/

extern char readblk[];
extern char writeblk[];
extern char cpyblk[];

extern char cpyPasteStr[];

/*********
  CONFIG
 *********/
extern char envIncompletStr[];
extern char turbocStr[];

#define HBYTE  0xff00
#define LBYTE  0x00ff
#define FIND   1

#define MKE  0xf100
#define COMP 0xf200
#define LNK  0xf300
#define ASS  0xf400
#define BRW  0xf500
#define RUN  0xf600
#define CHK  0xf700

#define BERR        1
#define MKE_NODIFF  2
#define BEXIT       3

/*--------------------
 *   Make utility
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

#define TC         1
#define MSC        0

#define CTYPE         1         /* type C      */
#define ATYPE         2         /* type ASM    */
#define LTYPE         3         /* type LINK   */
#define HTYPE         4         /* type HEADER */
#define MTYPE         5         /* type MACRO  */
#define ETYPE        -5         /* type ERREUR */
#define MAXMACRO     10
#define SAVE          6




/* type d'operation : Make, Compile, Link, assemble */
extern int operation;

/*--------------------
 *   Make utility
 *--------------------
 */

/* gestion des MACROS */
extern char  *MACROname[];
extern long  MACROdate[];
extern int   MACROfree;             /* 1er slot macro libre */
extern long  mdate;

extern char *lnkstr1;
extern char *lnkstr2;

extern char **spcc;
extern char **spasm;
extern char **splnk;

/*  gestion des parametres lus dans le
 *  fichier de configuration "xconfig"
 */

extern struct configuration conf;

/*  zone de reecriture des commandes si le fichier
 *  de configuration existe
 */

extern char  compilerName[];
extern char  linkerName[];
extern char  assemblerName[];

extern char MSClnkstr1[];
extern char MSClnkstr2[];
extern char *MSCspcc[];
extern char *MSCspasm[];
extern char *MSCsplnk[];

extern char TClnkstr1[];
extern char TClnkstr2[];
extern char TClnkstr2[];

extern char *TCspcc[];
extern char *TCspasm[];
extern char *TCsplnk[];


/* buffers de travail */

extern char *bufline;       /* buffer entree */
extern char *buftoken;      /* buffer ou sont stockes les tokens */
extern char *argf[];         /* tableau de pointeurs sur tokens   */

/* variables globales de travail */
extern char *tokenptr;
extern int arglc,narg;
extern int line,relativline;
extern int comptyp;

extern long filarg0,filargi;
extern int command;

extern int analyse,errorlevel;
extern struct stat entry;

extern struct OBJ *wm, wmake;
extern int fmsg, filout, makeError, difference;

extern int tmp_file;

extern char compStr[];
extern char assStr[];
extern char linkStr[];
extern char statObj[];
extern char statExe[];
extern int stat_obj, stat_exe;
extern char tempfile[];

extern char builtStr[];


/**********
  ITURBO
 **********/
/* interface entre compilateurs microsoft et Borland :
 * Adaptation des fonctions du turbo … la syntaxe
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

extern int XMOUSE, YMOUSE, LMOUSE;
extern unsigned CMOUSE;
extern unsigned mouseClic;
extern struct mouse_block_interface ib;
extern char *ibad;

#define  BIOSTAB 0x00400000   /* adresse debut table du BIOS */
#define  HEADptr 0x0040001a   /* adresse pointeur de queue  */
#define  TAILptr 0x0040001c   /* adresse pointeur de tete   */
#define  KEYbuf  0x0040001e   /* adresse du buffer clavier */
#define  LEN     0x10         /* longueur du buffer (en word) */
#define  KSOP    0x00400080   /* key buf start offset pointer */
#define  KEOP    0x00400082   /* key buf end offset pointer */

extern unsigned mouseBuffer[16];      /* buffer stockage ev souris */
extern int M_in,M_out;               /* pointeurs du buffer */

/**********
  CVIDEO
 **********/


extern int mouse_state;
extern int mouseEvent;
extern int mouseEmulate;
extern int posX, posY;
extern unsigned char ticks;

#define  LOWBYTE  0x0f
#define  M_REPOS           0
#define  M_MOVMENT         1
#define  M_LEFT_PRESS      2
#define  M_LEFT_RELEASE    4
#define  M_RIGHT_PRESS     8
#define  M_RIGHT_RELEASE  16


#define MOUSE_L       01
#define MOUSE_R       02
#define MOUSE_LR      03  /* Action sur release gauche */
#define MOUSE_RR      04  /* Action sur release droite */
#define MOUSE_M       05  /* Action sur mouvement      */
#define MOUSE_LP      06  /* Action sur press gauche   */
#define MOUSE_RP      07  /* Action sur press droit    */
#define MOUSE_DRAG     8  /* balayage menu principal   */

#define REFRESH   TRUE
#define UNREFRESH FALSE

extern  int  m_Gpr(), m_CGpr(), m_Gre(),
             m_Dpr(), m_CDpr(), m_Dre(), m_Mov();
extern  int  m_editDpr(); /* gestion press bouton droit pour l'editeur */
extern  int  m_editGpr(); /* gestion press bouton Gauche pour l'edit   */
extern  int  m_editGre(); /*   "     release bouton Gauche "    "      */
extern  int  m_editCDpr();/*   "     press courante sur B droit        */
extern  int  m_editCGpr();/*   "     press courante sur B gauche       */
extern  int  m_dirGpr();
extern  int  m_saisieGpr(); /* gestion press G pour la fenetre Rdialwin */

/*******
  HELP
 *******/

#define XFILE  60

#define HCLOSE  0    /* fin de l'help edit */
#define HREOPEN 1    /* on revient dans le Help edit */

extern struct save_edit sedHelp;

extern char helpErrStr[], hlpSelectStr[], hlpStr[], UnknownStr[];

extern char *hlpFile[];

extern char *fhlp[];
extern int helpInLine;
extern char defhlp[];

/*******
  WLINK
 *******/

extern int doLnkChoice();
extern char lnkChoiceStr[];
extern char lnkChoiceInt[];
extern char lnkChoiceFS[];
extern char lnkAutoAsk[];

extern char lnkChoiceRep[];
extern int mkd_shell;
extern int mkd_FS;
extern int mkd_AutoAsk;

/*******
  WTOOLS
 *******/

#define FIELD(p,x)         ((p)->mField+x)

extern int doToolsChoice();
extern char tooChoiceStr[];
extern char tooChoiceCmp[];
extern char tooChoiceAs1[];
extern char tooChoiceAs2[];
extern char tooKbdFastStr[];
extern char tooAutoSavStr[];

extern int mkd_masm, mkd_tasm;
extern int rmode, replaceMode, caseSensitive;


/**********
  FINDFUNC
 **********/

extern char gotoFuncNameStr[];
extern char dispFuncNameStr[];
extern char dbaseOpenErr[];
extern char notUserStr[];
extern char rebuildStr[];
extern char buildDBstr[];