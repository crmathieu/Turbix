/* country.h : declaration des messages
 *             la definition des messages se
 *             trouve dans msg_us.c pour messages US
 *             et msg_fr.c pour les messages FR
 */

/*  ULK Error Codes strings */
extern char *errStr[];
extern int  trErrTable[];

#define NSYSERR  46

/* disk */
extern char errDriveWp[];
extern char errUnkUnit[];
extern char errDriveNr[];
extern char errUnkComm[];
extern char errBadCRC[];
extern char errBadReq[];
extern char errSeekErr[];
extern char errUnkMtyp[];
extern char errSecNotF[];
extern char errWriteF[];
extern char errReadF[];
extern char errGenFail[];
extern char errHandM[];
extern char floppyM1[];
extern char floppyM2[];

/* io3 */
extern char lockErrStr[];

/* printf */
extern char sysHaltStr[];

/* shell */
extern char   errhd[];
extern char   fmt[];
extern char   shBanStr[];
extern char   shExitStr[];
extern char   shNotFStr[];
extern char   shExitSIGQUITStr[];

/* wind1 */
extern char winIScrStr[];
extern char winSysmsgStr[];
extern char winErrStr[];
extern char winInit1Str[];
extern char winInit2Str[];

/* xenstart */
extern char errFreeStr[];
extern char errIniDirStr[];
extern char shutDwnStr[];
extern char errDiv0Str[];
extern char errOvfStr[];
extern char errIIStr[];

/* xvideo */
extern char errIniSessStr[];
extern char badAnsiStr[];

/* syscmd */
extern char psStr[];
extern char hlpStr[];
extern char killStr[];
extern char killOkStr[];
extern char killNokStr[];
extern char badPidStr[];
extern char moreStr[];
extern char badDrvStr[];

extern char dirFileStr[];
extern char dirDirStr[];
extern char dirTotalStr[];


extern char resxUseStr[];
extern char dlookUseStr[];
extern char dlookBadBlkStr[];

extern char badDrvPStr[];

extern char partUseStr[];

extern char dchkUseStr[];

extern char dchk1_2Str[];
extern char dchk720Str[];
extern char dchk144Str[];
extern char dchk360Str[];
extern char dchkFixStr[];

extern char dchkHeadStr[];
extern char dchkCylStr[];
extern char dchkCluStr[];
extern char dchklSecStr[];
extern char dchkBSecStr[];
extern char dchkSecPerC[];
extern char dchkSecPerT[];
extern char dchkRootEnt[];
extern char dchkHidSec[];
extern char dchkDirBlk[];
extern char dchkFatBlk[];
extern char dchk1DataBlk[];
extern char dchkOverHSec[];

/* fcache */
extern char emptyLstStr[];
extern char errChainStr[];

/* file */
extern char rootFullStr[];

/* fmain */
extern char  errInitPoolStr[];
extern char errInitBigPool[];

/* fsdisk */
extern char dosPartInac[];
extern char activPartNotDos[];
extern char errboundPart[];
extern char diskNotSupp[];
extern char badSig[];
extern char bootBlkNr[];
extern char noDosActivPart[];
extern char noDiskEntry[];

/* fspath */
extern char dirNotFound[];
extern char allocDirEntFail[];
extern char pathSyntaxErr[];

/* fspipe */
extern char errOpenPipe[];
extern char noMorePipe[];
