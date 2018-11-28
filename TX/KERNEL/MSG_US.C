/*  dunix Error Codes strings */
static char nullstr[] = "";
char *errStr[] = {
        " : Error 0\n",                           /* 0  EZERO   */
        " : Invalid function number\n",           /* 1  EINVFNC */
        " : File not found\n",                    /* 2  ENOFILE */
        " : Path not found\n",                    /* 3  ENOPATH */
        nullstr,                                  /* 4          */
        nullstr,                                  /* 5          */
        nullstr,                                  /* 6          */
        " : Memory blocks destroyed\n",           /* 7  ECONTR  */
        nullstr,                                  /* 8          */
        " : Invalid memory block address\n",      /* 9  EINVMEM */
        " : Invalid environment\n",               /* 10 EINVENV */
        " : Invalid format\n",                    /* 11 EINVFMT */
        " : Invalid access code\n",               /* 12 EINVFMT */
        " : Invalid data\n",                      /* 13 EINVDAT */
        nullstr,                                  /* 14         */
        " : Invalid drive specified\n",           /* 15 EINVDRV */
        " : Attempt to remove CurDir\n",          /* 16 ECURDIR */
        " : Not same device\n",                   /* 17 ENOTSAM */
        " : No more files\n",                     /* 18 ENMFILE */
        " : No such file or directory\n",         /* 19 ENOENT  */
        " : Too many open files\n",               /* 20 EMFILE  */
        " : Permission denied\n",                 /* 21 EACCES  */
        " : Bad file number\n",                   /* 22 EBADF   */
        " : Not enough core\n",                   /* 23 ENOMEM  */
        " : No such device\n",                    /* 24 ENODEV  */
        " : Invalid argument\n",                  /* 25 EINVAL  */
        " : Arg list too long\n",                 /* 26 E2BIG   */
        " : Exec format error\n",                 /* 27 ENOEXEC */
        " : Cross-device link\n",                 /* 28 EXDEV   */
        " : Math argument\n",                     /* 29 EDOM    */
        " : Result too large\n",                  /* 30 ERANGE  */
        " : File already exists\n",               /* 31 EFEXIST */
        " : directory already exists\n",          /* 32 EDEXIST */
        " : no such device or address\n",         /* 33 ENXIO   */
        " : I/O error\n",                         /* 34 EIO     */
        " : no such process\n",                   /* 35 ERSCH   */
        " : not a directory\n",                   /* 36 ENOTDIR */
        " : is  a directory\n",                   /* 37 EISDIR  */
        " : no space on device\n",                /* 38 ENOSPC  */
        " : unknown error\n",                     /* 39 EUNKNW  */
        " : too many processes\n",                /* 40 EAGAIN  */
        " : Device busy\n",                       /* 41 EBUSY   */
        " : file table overflow\n",               /* 42 ENFILE  */
        " : stream table overflow\n",             /* 43 ENSTRE  */
        " : not a tty\n",                         /* 44 ENOTTY  */
        " : illegal seek\n",                      /* 45 ESPIPE  */
        " : semaphore table overflow\n",          /* 46 ENSEMA  */
        " : no child process\n",                  /* 47 ECHILD  */
        " : invalid operation\n",                 /* 48 EINVOP  */
        " : too many opened sessions\n",          /* 49 EMSESS  */
        " : invalid stream type\n",               /* 50 EINSTR  */
        " : invalid window\n",                    /* 51 EINWIN  */
        " : window mode disable\n"                /* 52 EINWM   */
};

#define NSYSERR  52

/* disk */
char errDriveWp[] = "Drive %c : Disk write Protected ...\n";
char errUnkUnit[] = "Drive %c : Unknown unit ...\n";
char errDriveNr[] = "Drive %c not ready ...\n";
char errUnkComm[] = "Drive %c :Unknown command ...\n";
char errBadCRC[]  = "Drive %c : CRC error in data\n";
char errBadReq[]  = "Incorrect lenght in request structure\n";
char errSeekErr[] = "Drive %c : Seek error\n";
char errUnkMtyp[] = "Drive %c : Unknown Media type ...\n";
char errSecNotF[] = "Drive %c : sector not found ...\n";
char errWriteF[]  = "Drive %c : Write fault ...\n";
char errReadF[]   = "Drive %c : Read  fault ...\n";
char errGenFail[] = "Drive %c : General failure\n";
char errHandM[]   = "Retry, Abort ? ";
char floppyM1[]   =  "\nInsert disk in drive [%c";
char floppyM2[]   =  "] and press any key\n";


/* io3 */
char lockErrStr[] = "\nFATAL : locking chain error\n";

/* printf */
char sysHaltStr[] = "%s -- System halted.\n";

/* shell */
char   errhd[]     = "\nSyntax error\n";
char   fmt[]       = "\nCannot open %s\n";
char   shBanStr[]  = "DUNIX Spy utility\n";
char   shExitStr[] = "\nexit from Spy\n";
char   shNotFStr[] = "\n%s: not found\n";
char   shExitSIGQUITStr[] = "\nexit %s\n";

/* wind1 */
char winIScrStr[]   = "\nCannot allocate space for standard screen mask\n";
char winSysmsgStr[] = "system message";
char winErrStr[]    = "\nWarning : window processing has not been initialized\n";
char winInit1Str[]  = "\nCannot allocate space for a new window\n";
char winInit2Str[]  = "\nCannot allocate space for window buffer\n";

/* dunix */
char errFreeStr[]   = "\n Error freeing global Memory - %x -\n";
char errIniDirStr[] = "\n\033[37mCannot initialize working directory block number\033[33m\n";
char shutDwnStr[]   = "\n\033[3mSystem shutdown\033[0m\n";
char errDiv0Str[]   = "\ntask %s : Divide by 0  -  killed\n";
char errOvfStr[]    = "\ntask %s : overflow  -  killed\n";
char errIIStr[]     = "\ntask %s : illegal Instruction -  killed - TopStack = %lX\n";

/* xvideo */
char errIniSessStr[] = "\ncan't initialize Session array : exit\n";
char badAnsiStr[]    = "\nBad ANSI Sequence - Delim = %c\n";

/* syscmd */
char psStr[]       = "\n%d task(s)\n";
char hlpStr[]      = "\nCommands are :\n";
char killStr[]     = "use: kill [pid]\n";
char killOkStr[]   = "Cannot kill: %d\n";
char killNokStr[]  = "* %s * killed\n";
char badPidStr[]   = "\nBad Process Id\n";
char moreStr[]     = "�� more ��\n";
char badDrvStr[]   = "\nincorrect drive assign\n";

char dirFileStr[]  = "\n-%c%c%c%c  %8s %3s   %6lu  %2d/%02d/%02d   %2d:%02d";
char dirDirStr[]   = "\nD%c%c%c%c  %8s %3s   <DIR>   %2d/%02d/%02d   %2d:%02d";
char dirTotalStr[] = "\n        %d file(s)      %d subdirectory(ies)\n";

char resxUseStr[]     = "\nuse  resx [serial line number]\n";
char dlookUseStr[]    = "use: dlook [drive] [block]\n";
char dlookBadBlkStr[] = "dlook : (%d) bad block number\n";

char badDrvPStr[]     = "%s : invalid device\n";  /* bad drive avec parametre */

char partUseStr[]     = "use : partition [device unit]\n";

char dchkUseStr[]     = "use: dchk [drive]\n";

char dchk1_2Str[]     = "\n-- 1,2  Mo diskette --\n";
char dchk720Str[]     = "\n-- 720  Ko diskette --\n";
char dchk144Str[]     = "\n-- 1,44 Mo diskette --\n";
char dchk360Str[]     = "\n-- 360  Ko diskette --\n";
char dchkFixStr[]     = "\n-- fix disk --\n";

char dchkHeadStr[]    = "\nHeads ..................... %u";
char dchkCylStr[]     = "\nCylinders ................. %u";
char dchkCluStr[]     = "\nClusters .................. %u";
char dchklSecStr[]    = "\nSectors ................... %u"; /* sec sur 16 bits */
char dchkBSecStr[]    = "\nSectors ................... %lu";/* sec sur 32 bits */
char dchkSecPerC[]    = "\nSector(s) Per Cluster ..... %u";
char dchkSecPerT[]    = "\nSectors Per Track ......... %u";
char dchkRootEnt[]    = "\nRoot Entries .............. %u";
char dchkHidSec[]     = "\nHiden Sector(s) ........... %u";
char dchkDirBlk[]     = "\nDirectory Blocks .......... %u";
char dchkFatBlk[]     = "\nFat Blocks ................ %u";
char dchk1DataBlk[]   = "\nFirst Data Block .......... %u";
char dchkOverHSec[]   = "\nOverhead Sectors .......... %u\n";

/* fcache */
char emptyLstStr[]    = "\nFATAL : empty free list\n";
char errChainStr[]    = "\nFATAL : cache buffer chain error\n";

/* file */
char rootFullStr[]    = "\nRoot directory full\n";

/* fmain */
char  errInitPoolStr[] = "can't initialize buffers pool\n";
char errInitBigPool[]  = "can't initialize bigbloc pool\n";

/* fsdisk */
char dosPartInac[]     = "\nMSDOS Partition not activ on disk %s\n";
char activPartNotDos[] = "\nactiv Partition is not MSDOS : P_SYS flag = %u\n";
char errboundPart[]    = "\nWARNING : # sectors before partition : %d does not match \n          with Begining partition's parameters : %d\n";
char diskNotSupp[]     = "\n          disk type probably not supported - Exit from DUNIX\n";
char badSig[]          = "\nBad boot bloc signature = %x\n";
char bootBlkNr[]       = "boot bloc number = %lx\n";
char noDosActivPart[]  = "\nNo MSDOS activ Partition on disk %s\n";
char noDiskEntry[]     = "WARNING : can't find disk entry for disk - %s -\n";

/* fspath */
char dirNotFound[]     = "can't find directory %s\n";
char allocDirEntFail[] = "\ndirectory entry cannot be allocated\n";
char pathSyntaxErr[]   = "\nincorrect pathname\n";

/* fspipe */
char errOpenPipe[]     = "\nCan't open pipe\n";
char noMorePipe[]      = "\nNo more Pipe\n";