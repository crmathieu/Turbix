/*  VERSION FRANCAISE */
static char nullstr[] = "";
char *errStr[] = {
        " : Erreur 0\n",                            /* 0  EZERO   */
        " : num‚ro de fonction invalide\n",         /* 1  EINVFNC */
        " : Fichier introuvable\n",                 /* 2  ENOFILE */
        " : Chemin introuvable\n",                  /* 3  ENOPATH */
        nullstr,                                    /* 4          */
        nullstr,                                    /* 5          */
        nullstr,                                    /* 6          */
        " : Blocs M‚moire d‚truits\n",              /* 7  ECONTR  */
        nullstr,                                    /* 8          */
        " : Adresse Bloc m‚moire invalide\n",       /* 9  EINVMEM */
        " : Environment invalide\n",                /* 10 EINVENV */
        " : Format invalide\n",                     /* 11 EINVFMT */
        " : Code d'acc‚s invalide\n",               /* 12 EINVFMT */
        " : donn‚e invalide\n",                     /* 13 EINVDAT */
        nullstr,                                    /* 14         */
        " : drive sp‚cifi‚ invalide\n",             /* 15 EINVDRV */
        " : Tentative de suppression rep courant\n",/* 16 ECURDIR */
        " : p‚riph‚rique non identique\n",          /* 17 ENOTSAM */
        " : Plus de fichier\n",                     /* 18 ENMFILE */
        " : Ni fichier, ni R‚pertoire\n",           /* 19 ENOENT  */
        " : Trop de fichiers ouverts\n",            /* 20 EMFILE  */
        " : Permission refus‚e\n",                  /* 21 EACCES  */
        " : Handle invalide\n",                     /* 22 EBADF   */
        " : Pas assez de m‚moire\n",                /* 23 ENOMEM  */
        " : Pas de device\n",                       /* 24 ENODEV  */
        " : argument Iinvalide\n",                  /* 25 EINVAL  */
        " : liste d'arg. trop longue\n",            /* 26 E2BIG   */
        " : Erreur format Exec\n",                  /* 27 ENOEXEC */
        " : Lien crois‚ entre devices\n",           /* 28 EXDEV   */
        " : Argument Math\n",                       /* 29 EDOM    */
        " : Resultet trop grand\n",                 /* 30 ERANGE  */
        " : Fichier existant d‚j…\n",               /* 31 EFEXIST */
        " : R‚pertoire existant d‚j…\n",            /* 32 EDEXIST */
        " : Ni device, ni adresse\n",               /* 33 ENXIO   */
        " : Erreur d'E/S\n",                        /* 34 EIO     */
        " : n'est pas une tƒche\n",                 /* 35 ERSCH   */
        " : n'est pas un r‚pertoire\n",             /* 36 ENOTDIR */
        " : est un r‚pertoire\n",                   /* 37 EISDIR  */
        " : Plus de place sur le device\n",         /* 38 ENOSPC  */
        " : Erreur inconnue\n",                     /* 39 EUNKNW  */
        " : trop de tƒches\n",                      /* 40 EAGAIN  */
        " : Device occup‚\n",                       /* 41 EBUSY   */
        " : Table des fichiers satur‚e\n",          /* 42 ENFILE  */
        " : Table des flots satur‚e\n",             /* 43 ENSTRE  */
        " : n'est pas un tty\n",                    /* 44 ENOTTY  */
        " : Seek illegal\n",                        /* 45 ESPIPE  */
        " : Table des s‚maphores satur‚e\n",        /* 46 ENSEMA  */
        " : pas de tƒche fille\n",                  /* 47 ECHILD  */
        " : operation invalide\n",                  /* 48 EINVOP  */
        " : trop de sessions ouvertes\n",           /* 49 EMSESS  */
        " : type de stream invalide\n",             /* 50 EINSTR  */
        " : fenˆtre invalide\n",                    /* 51 EINWIN  */
        " : mode window non initialis‚\n"           /* 52 EINWM   */
};

#define NSYSERR  52

/* disk */
char errDriveWp[] = "Drive %c : Disque prot‚g‚ en ‚criture ...\n";
char errUnkUnit[] = "Drive %c : Unit‚ inconnue ...\n";
char errDriveNr[] = "Drive %c non prˆt ...\n";
char errUnkComm[] = "Drive %c : Commande inconnue ...\n";
char errBadCRC[]  = "Drive %c : Erreur de CRC dans les donn‚es\n";
char errBadReq[]  = "Requˆte de longueur incorrecte\n";
char errSeekErr[] = "Drive %c : Erreur de positionnement\n";
char errUnkMtyp[] = "Drive %c : Type de m‚dia inconnu...\n";
char errSecNotF[] = "Drive %c : Secteur introuvable ...\n";
char errWriteF[]  = "Drive %c : Erreur en ‚criture ...\n";
char errReadF[]   = "Drive %c : Erreur en lecture ...\n";
char errGenFail[] = "Drive %c : Echec g‚n‚ral\n";
char errHandM[]   = "R‚essayer, Abandonner ? ";
char floppyM1[]   =  "\nIns‚rer une disquette dans le drive [%c";
char floppyM2[]   =  "] et presser une touche\n";

/* io3 */
char lockErrStr[] = "\nFATAL : erreur dans le chainage des tƒche LOCKEES\n";

/* printf */
char sysHaltStr[] = "%s -- SystŠme suspendu.\n";

/* shell */
char   errhd[]     = "\nErreur de syntaxe\n";
char   fmt[]       = "\nImpossible d'ouvrir %s\n";
char   shBanStr[]  = "MKD shell V1.0 - M‚moire disponible %lu\n";
char   shExitStr[] = "\nFin d'ex‚cution du Shell\n";
char   shNotFStr[] = "\n%s: introuvable\n";
char   shExitSIGQUITStr[] = "\nFin d'ex‚cution de %s\n";

/* wind1 */
char winIScrStr[]   = "\nImpossible d'allouer ressource pour le masque de l'‚cran standard\n";
char winSysmsgStr[] = "fenˆtre messages";
char winErrStr[]    = "\nAttention : La gestion des fenˆtre n'a pas ‚t‚ initialis‚e\n";
char winInit1Str[]  = "\nImpossible d'allouer ressources pour une nouvelle fenˆtre\n";
char winInit2Str[]  = "\nImpossible d'allouer m‚moire pour le tampon fenˆtre\n";

/* xenstart */
char errFreeStr[]   = "\n Erreur en lib‚rant la m‚moire globale - %x -\n";
char errIniDirStr[] = "\n\033[37mImpossible de r‚cup‚rer le num‚ro de bloc du r‚pertoire courant\033[33m\n";
char shutDwnStr[]   = "\n\033[3mSystŠme shutdown\033[0m\n";
char errDiv0Str[]   = "\ntƒche %s : Division par 0  -  tu‚e\n";
char errOvfStr[]    = "\ntƒche %s : d‚bordement de pile  -  tu‚e\n";
char errIIStr[]     = "\ntƒche %s : Instruction illegale -  tu‚e - Sommet de pile = %lX\n";

/* xvideo */
char errIniSessStr[] = "\nImpossible d'initialiser la gestion des SESSIONS : fin anormale\n";
char badAnsiStr[]    = "\nMauvaise s‚quence ANSI - Delim = %c\n";

/* syscmd */
char psStr[]       = "\n%d tƒche(s)\n";
char hlpStr[]      = "\nLes commandes sont :\n";
char killStr[]     = "format : kill [pid]\n";
char killOkStr[]   = "Impossible de tuer : %d\n";
char killNokStr[]  = "* %s * tu‚e\n";
char badPidStr[]   = "\nIdentificateur de tƒche (pid) invalide\n";
char moreStr[]     = "ÄÄ Suite ÄÄ\n";
char badDrvStr[]   = "\nAssignation de drive incorrecte\n";

char dirFileStr[]  = "\n-%c%c%c%c  %8s %3s   %6lu  %2d/%02d/%02d   %2d:%02d";
char dirDirStr[]   = "\nR%c%c%c%c  %8s %3s   <REP>   %2d/%02d/%02d   %2d:%02d";
char dirTotalStr[] = "\n        %d fichier(s)      %d sous r‚pertoire(s)\n";

char resxUseStr[]     = "\nformat :  resx [num‚ro de ligne]\n";
char dlookUseStr[]    = "format : dlook [drive] [bloc]\n";
char dlookBadBlkStr[] = "dlook : (%d) mauvais num‚ro de bloc\n";

char badDrvPStr[]     = "%s : device invalide\n";  /* bad drive avec parametre */

char partUseStr[]     = "format : partition [nom d'unit‚]\n";

char dchkUseStr[]     = "format : dchk [unit‚]\n";

char dchk1_2Str[]     = "\n-- disquette de 1,2  Mo --\n";
char dchk720Str[]     = "\n-- disquette de 720  Ko --\n";
char dchk144Str[]     = "\n-- disquette de 1,44 Ko --\n";
char dchk360Str[]     = "\n-- disquette de 360  Ko --\n";
char dchkFixStr[]     = "\n-- disque dur --\n";

char dchkHeadStr[]    = "\ntˆtes ..................... %u";
char dchkCylStr[]     = "\nCylindres ................. %u";
char dchkCluStr[]     = "\nClusters .................. %u";
char dchklSecStr[]    = "\nSecteurs .................. %u"; /* sec sur 16 bits */
char dchkBSecStr[]    = "\nSecteurs .................. %lu";/* sec sur 32 bits */
char dchkSecPerC[]    = "\nSecteur(s) Par Cluster .... %u";
char dchkSecPerT[]    = "\nSecteurs Par Piste ........ %u";
char dchkRootEnt[]    = "\nEntr‚es en Racine ......... %u";
char dchkHidSec[]     = "\nSecteur(s) cach‚s ......... %u";
char dchkDirBlk[]     = "\nnb de Blocs en racine ..... %u";
char dchkFatBlk[]     = "\nnb de Blocs en FAT ........ %u";
char dchk1DataBlk[]   = "\nPremier Bloc de donn‚es ... %u";
char dchkOverHSec[]   = "\nnb de Secteurs overhead ... %u\n";

/* fcache */
char emptyLstStr[]    = "\nFATAL : Plus de bloc disponible\n";
char errChainStr[]    = "\nFATAL : Erreur dans le chainage des buffers caches\n";

/* file */
char rootFullStr[]    = "\nR‚pertoire racine plein\n";

/* fmain */
char  errInitPoolStr[] = "Impossible d'initialiser la gestion du cache\n";
char errInitBigPool[]  = "Impossible d'initialiser la gestion des blocs FAT\n";

/* fsdisk */
char dosPartInac[]     = "\nPartition MSDOS non active sur le disque %s\n";
char activPartNotDos[] = "\nLa partition active n'est MSDOS : P_SYS flag = %u\n";
char errboundPart[]    = "\nATTENTION : # secteurs avant la partition : %d ne correspond pas \n          avec les paramŠtres du d‚but de partition : %d\n";
char diskNotSupp[]     = "\n            Ce type de disque n'est probablement pas support‚ - Sortir de MKD\n";
char badSig[]          = "\nMauvaise signature de Boot bloc = %x\n";
char bootBlkNr[]       = "Num‚ro du boot bloc = %lx\n";
char noDosActivPart[]  = "\nAucune partition MSDOS active sur le disque %s\n";
char noDiskEntry[]     = "ATTENTION : Impossible de trouver une entr‚e pour le disque - %s -\n";

/* fspath */
char dirNotFound[]     = "R‚pertoire %s introuvable\n";
char allocDirEntFail[] = "\nImpossible d'allouer une entr‚e de r‚pertoire\n";
char pathSyntaxErr[]   = "\nChemin incorrect\n";

/* fspipe */
char errOpenPipe[]     = "\nImpossible d'ouvrir le pipe\n";
char noMorePipe[]      = "\nPlus de pipe disponible\n";
