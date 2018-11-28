/* dunix.c */
#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "signal.h"
#include "console.h"
#include "const.h"
#include "fsopen.h"
#include "file.h"

/*  Save current interrupt vectors */
long               msdosv_clk , msdosv_kbd , msdosv_com , msdosv_div0;
long               msdosv_ovf , msdosv_ill, msdosv_int15, msdosv_int61,
                   msdosv_mou,  msdosv_int24;

int               (* msdosv_floppy)(), (* msdosv_disk)();

int                currDrive;
char               currWD[80];
extern   int  far *_allocmem();
Bool               fpanic;
int                stime[3],sdate[3];
long               begintime,endtime;
char   far        *MachineID = (char far *)0xf000fffe;
uchar              Machine;
extern   int       vid_vs;
extern   long      VStab;
int                demarrage, sysrun;
extern   int       iorun;
int               *cronState;
int far *deb1; /**/

extern  int        rdyhead,rdytail,piphead,piptail;
extern  long       m_Time();
int                GlobalArgc;
char              *GlobalArgv[5];

/*---------------------------------------------------------------------------
 * getAllMem - grabs all the memory available
 *---------------------------------------------------------------------------
 */
_getAllMem(nb_paragraphes)
int *nb_paragraphes;
{
#define MAXPARAGRAFS 40000

    *nb_paragraphes = MAXPARAGRAFS;
    debaddr         = _allocmem(*nb_paragraphes);
    *nb_paragraphes = (FP_SEG(debaddr)); 
    debaddr         = _allocmem(*nb_paragraphes);

    if (FP_OFF(debaddr) == 0xffff) {
        return(RERR);
    }
    /* deb1 = debaddr;*/
    return(ROK);
}

/*---------------------------------------------------------------------------
 * dunix entrypoint - called by compiler startup
 *---------------------------------------------------------------------------
 */
main(argc,argv)
char *argv[];
{
    int              fd;
    int             *stack;
    long             elapsed;
    unsigned         paragrafs, ret;
    unsigned         nb_paragraphes;
    unsigned         i,t,hours,minutes,seconds;
    struct mblock   *mp;
    struct taskslot *tp;
    unsigned         amount;


    /* copy arguments */
    for (i = 0;(argv[i] && i < argc); i++) {
           GlobalArgv[i] = argv[i];
    }

    GlobalArgv[0]    = "umain";
    GlobalArgv[argc] = NULLPTR;
    GlobalArgc       = argc;

    _disable();

    /* increase number of possible handles */
    if (_setHandleCount(F_NSLOT)) {
        return;
    }
    if (_getAllMem(&nb_paragraphes)) {
        return;
    }
    maxaddr          = debaddr;
    FP_SEG(maxaddr) += (nb_paragraphes-1);

    /* calculate total # of paragraphs and remove 
     * the initial task0 stack size from it 
     */

    i             = 2048/16;
    paragrafs     = nb_paragraphes - i;

    memlist.mnext = mp = (struct mblock *)debaddr;
    mp->mnext     = (struct mblock *)NULL;
    mp->mlen      =  paragrafs;

    /* ------------------  
     * initialize task0
     * ------------------ */
     
    tp = &Tasktab[TASK0];
    stack = _stackAlloc(i*16);
    tp->tstklen  = i * 16;
    tp->tUstkbase = stack;
    tp->tstate   = RUNNING;
    tp->tevent   = tp->tflag = 0;

    for (i = 0; i < TNMLEN; i++) {
         tp->tname[i] = "idle"[i];
    }

    RUNpid        = TASK0;
/*    *(tp->tUstkbase) = MAGIC;*/
    tp->tprio    = 5;
    tp->tmsgnr   = 0;
    tp->tmsgIn   = 0;
    tp->tmsgOut  = 0;
    tp->tUstklim = tp->tUstkbase;
    FP_OFF(tp->tUstklim) = 0;
    tp->tppid  = 0;
    tp->tevsig   = 0;
    tp->tevcatch = 0;
/*    tp->tdelay   = FALSE;*/
    tp->tupid    = 0;
    tp->tgrp     = VS0;
    tp->tuser    = 0;
    tp->theadblk = (struct hblk *)NULL;
    tp->ttailblk = (struct hblk *)NULL;
    tp->tITvalid = FALSE;
    fpanic       = FALSE;

    /* set standard I/O */
    _init_stream();

    /* initialize file descriptors */
    for ( i = 0 ; i  < NFD ; i++) {
        tp->tfd[i] = NULLSTREAM;
    }

    /* record starting time */
    _getsystime(stime);
    _getsysdate(sdate);

    /* change task0 stack address. We will come back 
     * from _starttask0 when dunix will shut down */

    _starttask0(stack);   

    /* dunix is shutting down */

    tty[VS0].wmode = FALSE; 
    if ((ret = _disallocmem(debaddr)) != 0) { /* give back allocated memory to DOS */
        m_Printf(errFreeStr,ret);
        return;
    }

    /* return to DOS after clearing screen */
    m_Putc('\x0c', 1);
    m_Gotoxy(0, 0);
    return;
}

/*---------------------------------------------------------------------------
 * task0 - IDLE task
 *---------------------------------------------------------------------------
 */
task0()
{
    int           fd,ps,i;
    int           _fsinit(), _ioTsk(), umain(), _shutdown(), _Psync();
    int           _vio(),_service(),_clockT(), _mou_task(), _scheduler();
    int           interrupt _itOverflow();
    extern int    _int_itDiv0(),_int_itIllegal(), mouse_present;
    int           _entryP(),_itdisk(), _int15(), _itcritical();
    int           interrupt _dummy_interrupt();

    /* we can use int 21h until we change the interrupt vectors */
    _disable();
    _sysinit();

    /* set directory and default drive */
    Tasktab[0].tcurrdev = currDrive;
    memset(Tasktab[0].tcurrdir, 0, 64);
    fastcpy(Tasktab[0].tcurrdir, currWD, strlen(currWD));


    /* set interrupt vectors handling signals and secondary memory */
    _sasVector(XENDIV0,_int_itDiv0,    &msdosv_div0);
    _sasVector(XENOVF, _itOverflow,&msdosv_ovf );
    _sasVector(XENILL, _int_itIllegal, &msdosv_ill );
/*    _sasVector(0x15, _int15, &msdosv_int15);
    _sasVector(0x61, msdosv_int15, &msdosv_int61);*/
    _sasVector(0x24, _itcritical, &msdosv_int24);
/*    _sasVector(0x33, _dummy_interrupt, &msdosv_mou);*/

    /* determine the MACHINE type */
    Machine = *MachineID;

    /* ignore all signals */
    for (i=0;i<SIGNR;i++) {
         m_Signal(i,SIG_IGN);
    }

    /* set standard I/O */
    fd = m_Open("/DEV/VS0", O_RDWR);
    m_Dup(fd);
    m_Dup(fd);

    /* start system tasks: WARNING: any change in the order of these tasks
     * must be matched by changes in the values of each task's indentifier
     * in the "proc.h" file */

    _launch(_makeProcess(_scheduler  , SYS_TASK,2048 , 60 ,"kernel",  0 ),DIFFERED);
    _launch(_makeProcess(_clockT     , SYS_TASK,2048 , 50 ,"clock",   0 ),IMMEDIAT);
    _launch(_makeProcess(_service    , SYS_TASK,1024 , 30 ,"request", 0 ),IMMEDIAT);
    _launch(_makeProcess(_ioTsk      , SYS_TASK,2048 , 20 ,"i/o"    , 0 ),IMMEDIAT);
    _launch(_makeProcess(_shutdown   , SYS_TASK,1024 , 55 ,"shtdwn" , 0 ),IMMEDIAT);
    _launch(_makeProcess(_vio        , SYS_TASK,1024 , 25 ,"vio"    , 0 ),IMMEDIAT);

    /*_launch(_makeProcess(_mou_task   , SYS_TASK,1024 , 15 ,"_mst"   , 0 ),IMMEDIAT);*/



    /* start user task after setting the working directory in the USERS table */
    _doCursorShape(CURS_SMALL);
    _launch(_makeProcess(_entryP    , UMAIN_TASK, 4096, 10 ,"umain"   , 0 ),DIFFERED);

    _Enable();

    /* IDLE loop */
    sysrun = 1;
    while (sysrun) {
        _swpProc();
    }

    /* Normal Shutdown */
END:
    _disable();

    /* restore interrupt vectors */
    _rVector(XENCLOCK,msdosv_clk);
    _rVector(XENKBD  ,msdosv_kbd);
    _rVector(XENCOM  ,msdosv_com);
    _rVector(XENDIV0 ,msdosv_div0);
    _rVector(XENOVF  ,msdosv_ovf);
    _rVector(XENILL  ,msdosv_ill);
/*  _rVector(0x15  ,  msdosv_int15);
    _rVector(0x61  ,  msdosv_int61);*/
    _rVector(0x24  ,  msdosv_int24);

    /*if (!mouse_present)
        _rVector(0x33  ,  msdosv_mou);
    else {
        _m_installed(&i);
        _m_callMaskAndAddress(0, 0L);
    }*/

    _enable();
}

/*---------------------------------------------------------------------------
 * _entryP - start user task after setting the 
 *           working directory in the USERS table
 *---------------------------------------------------------------------------
 */
TASK _entryP()
{
   umain(GlobalArgc, GlobalArgv);
}

/*---------------------------------------------------------------------------
 * shutdown - exit dunix - open files will be automatically closed by DOS
 *---------------------------------------------------------------------------
 */
TASK _shutdown()
{
   uchar mess;
   int stat, i, ps;
   extern SEM iosem;

   /* wait for shutdown message */
   m_Msgwait(&mess, -1, &stat);

   ps = _itDis();
   iorun = 0;
   _setprio(E_S, 60);           /* make sure there is no current DOS I/O */
   _sigsem(iosem, NOSCHEDUL);   /* wake up disk I/O handler */
   _itRes(ps);

   /* wait for shutdown message from IOTASK */
   m_Msgwait(&mess, -1, &stat);

   /* kill current tasks */
   for (i=NSYSTASK; i<NTASK; i++) {
        if (Tasktab[i].tstate == UNUSED) {
            continue;
        }
        _doKill(i);
   }

   /* set IDLE task as highest priority to give it the CPU */
   ps = _itDis();
   _setprio(SCHEDULER, 0);
   sysrun = 0;
   _setprio(TASK0,70);
   Tasktab[RUNpid].tstate = SLEEP;
   _swpProc();
   _itRes(ps);
}



/*---------------------------------------------------------------------------
 *  itDiv0 - division by 0 interrupt
 *---------------------------------------------------------------------------
 */
_itDiv0()
{
   int (*sigfunc)();

   _itDis();

   if ((sigfunc = Tasktab[m_Getpid()].tevfunc[SIGDIV]) == SIG_DFL || \
        sigfunc == SIG_IGN) {
           sysmsg(errDiv0Str,Tasktab[m_Getpid()].tname);
           m_Exit(-1);
    } else {
        m_Kill(m_Getpid(),SIGDIV);
    }
}

/*---------------------------------------------------------------------------
 *  itovf - overflow interrupt
 *---------------------------------------------------------------------------
 */
interrupt _itOverflow()
{
   int (*sigfunc)();

   _itDis();
   if ((sigfunc = Tasktab[m_Getpid()].tevfunc[SIGOVF]) == SIG_DFL || \
        sigfunc == SIG_IGN) {
           sysmsg(errOvfStr,Tasktab[m_Getpid()].tname);
           m_Exit(-1);
    } else {
        m_Kill(m_Getpid(),SIGOVF);
    }
}

/*---------------------------------------------------------------------------
 *  itIllegal - illegal instruction interrupt
 *---------------------------------------------------------------------------
 */
_itIllegal()
{
   int (*sigfunc)();
   int i;
   struct taskslot *tp;
   _itDis();

   if ((sigfunc = (tp = &Tasktab[m_Getpid()])->tevfunc[SIGILL]) == SIG_DFL || \
        sigfunc == SIG_IGN) {
        sysmsg(errIIStr,tp->tname,tp->tKstkbase);
        m_Exit(-1);
    } else {
        m_Kill(m_Getpid(),SIGILL);
    }
}

interrupt _dummy_interrupt() { }
