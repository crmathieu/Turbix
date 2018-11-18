/* xenstart.c */
#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "signal.h"
#include "console.h"
#include "const.h"
#include "fsopen.h"
#include "file.h"



/*  Sauvegarde des vecteurs d'nterruptions
 *  au moment du demarrage
 */

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

/*---------------------------------------------------------------------------------------------------------------------------------------
 * getAllMem - reserver toute la memoire
 *---------------------------------------------------------------------------
 */
_getAllMem(nb_paragraphes)
int *nb_paragraphes;
{
#define MAXPARAGRAFS 40000

    *nb_paragraphes = MAXPARAGRAFS;
     debaddr        = _allocmem(*nb_paragraphes);
    *nb_paragraphes = (FP_SEG(debaddr)); /* essai : ne prendre que 1/2 */
     debaddr        = _allocmem(*nb_paragraphes);
     if (FP_OFF(debaddr) == 0xffff)
         return(RERR);
/*     deb1 = debaddr;*/
     return(ROK);
}

/*---------------------------------------------------------------------------
 * ulk_startup - appele par le startup du compilateur
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


    /* copier arguments */
    for (i = 0;(argv[i] && i < argc); i++)
           GlobalArgv[i] = argv[i];

    GlobalArgv[0]    = "umain";
    GlobalArgv[argc] = NULLPTR;
    GlobalArgc       = argc;

    _disable();

    /* augmenter le nombre des Handles REELS possibles */
    if (_setHandleCount(F_NSLOT))
        return;

    if (_getAllMem(&nb_paragraphes))
        return;

    maxaddr          = debaddr;
    FP_SEG(maxaddr) += (nb_paragraphes-1);

    /*   calcul le nombre total de paragrafs et retire la taille
     *   de la pile de la tache initiale
     */

    i             = 2048/16;
    paragrafs     = nb_paragraphes - i;

    memlist.mnext = mp = (struct mblock *)debaddr;
    mp->mnext     = (struct mblock *)NULL;
    mp->mlen      =  paragrafs;

    /*   initialise la tache initiale
     *   ----------------------------
     */

    tp = &Tasktab[TASK0];
    stack = _stackAlloc(i*16);
    tp->tstklen  = i * 16;
    tp->tUstkbase = stack;
    tp->tstate   = RUNNING;
    tp->tevent   = tp->tflag = 0;

    for (i = 0; i < TNMLEN; i++)
         tp->tname[i] = "idle"[i];
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


    /* mettre en place les E/S standards */
    _init_stream();

    /* initialiser les  file descriptors */
    for ( i = 0 ; i  < NFD ; i++)
                             tp->tfd[i] = NULLSTREAM;

    /* enregistrer l'instant de demarrage */
    _getsystime(stime);
    _getsysdate(sdate);

    _starttask0(stack);   /*  change l'adresse de pile de la tache task0
                           *  on revient de _starttask0 lors du SHUTDOWN
                           */

    tty[VS0].wmode = FALSE; /* remettre fenetres inoperantes */
    if ((ret = _disallocmem(debaddr)) != 0) { /* redonner toute la memoire au DOS */
        m_Printf(errFreeStr,ret);
        return;
    }

    /* revenir au DOS en effa‡ant l'‚cran */
    m_Putc('\x0c', 1);
    m_Gotoxy(0, 0);
    return;
}

/*---------------------------------------------------------------------------
 * task0 - process IDLE
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

    /* On peut utiliser les int 21h jusqu'… la modification
     * des vecteurs d'interruption
     */
    _disable();
    _sysinit();

    /* placer directorie et drive par defaut */
    Tasktab[0].tcurrdev = currDrive;
    memset(Tasktab[0].tcurrdir, 0, 64);
    fastcpy(Tasktab[0].tcurrdir, currWD, strlen(currWD));


    /* placer les vecteurs d'interruptions gerant les
     * signaux et les memoires de masse
     */
    _sasVector(XENDIV0,_int_itDiv0,    &msdosv_div0);
    _sasVector(XENOVF, _itOverflow,&msdosv_ovf );
    _sasVector(XENILL, _int_itIllegal, &msdosv_ill );
/*    _sasVector(0x15, _int15, &msdosv_int15);
    _sasVector(0x61, msdosv_int15, &msdosv_int61);*/
    _sasVector(0x24, _itcritical, &msdosv_int24);
/*    _sasVector(0x33, _dummy_interrupt, &msdosv_mou);*/

    /* determiner le type de MACHINE */
    Machine = *MachineID;

    /* ignorer tous les signaux */
    for (i=0;i<SIGNR;i++)
         m_Signal(i,SIG_IGN);

    /* mettre en place les E/S standards */
    fd = m_Open("/DEV/VS0", O_RDWR);
    m_Dup(fd);
    m_Dup(fd);

    /*  lancer les taches systemes - ATTENTION  : toute modification
     *  de l'ordre de construction des taches systemes entraine une
     *  mise a jour des constantes d'Identification des taches dans
     *  "proc.h"
     */
    _launch(_makeProcess(_scheduler  , SYS_TASK,2048 , 60 ,"kernel", 0 ),DIFFERED);
    _launch(_makeProcess(_clockT     , SYS_TASK,2048 , 50 ,"clock", 0 ),IMMEDIAT);
    _launch(_makeProcess(_service    , SYS_TASK,1024 , 30 ,"request", 0 ),IMMEDIAT);
    _launch(_makeProcess(_ioTsk      , SYS_TASK,2048 , 20 ,"i/o"    , 0 ),IMMEDIAT);
    _launch(_makeProcess(_shutdown   , SYS_TASK,1024 , 55 ,"shtdwn" , 0 ),IMMEDIAT);
    _launch(_makeProcess(_vio        , SYS_TASK,1024 , 25 ,"vio"    , 0 ),IMMEDIAT);

    /*_launch(_makeProcess(_mou_task   , SYS_TASK,1024 , 15 ,"_mst"   , 0 ),IMMEDIAT);*/



    /* lancer la tache utilisateur apres avoir positionn‚ la directory
     * de travail dans la table des USERS
     */
    _doCursorShape(CURS_SMALL);
    _launch(_makeProcess(_entryP    , UMAIN_TASK, 4096, 10 ,"umain"   , 0 ),DIFFERED);

    _Enable();

    /* IDLE loop */
    sysrun = 1;
    while (sysrun) {
     /*             m_printf(" IDLE ");*/
                  _swpProc();
    }

    /* Normal Shutdown */
FIN:
    _disable();
    _rVector(XENCLOCK,msdosv_clk);
    _rVector(XENKBD  ,msdosv_kbd);
    _rVector(XENCOM  ,msdosv_com);
    _rVector(XENDIV0 ,msdosv_div0);
    _rVector(XENOVF  ,msdosv_ovf);
    _rVector(XENILL  ,msdosv_ill);
/*    _rVector(0x15  ,  msdosv_int15);
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
 * _entryP - lancer la tache utilisateur apres avoir positionn‚ la directory
 *           de travail dans la table des USERS
 *---------------------------------------------------------------------------
 */
TASK _entryP()
{
   umain(GlobalArgc, GlobalArgv);
}

/*---------------------------------------------------------------------------
 * shutdown - sortir proprement de ulk - Les fichiers encore ouverts sont
 *            automatiquement ferm‚s par DOS
 *---------------------------------------------------------------------------
 */
TASK _shutdown()
{
   uchar mess;
   int stat, i, ps;
   extern SEM iosem;

   /* attente message de shutdown */
   m_Msgwait(&mess, -1, &stat);

   ps = _itDis();
   iorun = 0;
   _setprio(E_S, 60);           /* s'assurer qu'une IO dos n'est pas en cours */
   _sigsem(iosem, NOSCHEDUL);   /* reveiller le process manager d'E/S disque */
   _itRes(ps);

   /* attente message de shutdown depuis IOTASK */
   m_Msgwait(&mess, -1, &stat);

   /* tuer les taches en cours */
   for (i=NSYSTASK; i<NTASK; i++) {
        if (Tasktab[i].tstate == UNUSED)
            continue;
        _doKill(i);
   }

   /* donner la main … la tache IDLE */
   ps = _itDis();
   _setprio(SCHEDULER, 0);
   sysrun = 0;
   _setprio(TASK0,70);
   Tasktab[RUNpid].tstate = SLEEP;
   _swpProc();
   _itRes(ps);
}



/*---------------------------------------------------------------------------
 *  itDiv0 - interruption de division par 0
 *---------------------------------------------------------------------------
 */
_itDiv0()
{
   int (*sigfunc)();

   _itDis();

   if ((sigfunc = Tasktab[m_Getpid()].tevfunc[SIGDIV]) == SIG_DFL || \
        sigfunc == SIG_IGN)
        {
           sysmsg(errDiv0Str,Tasktab[m_Getpid()].tname);
           m_Exit(-1);
        }
        else
           m_Kill(m_Getpid(),SIGDIV);
}

/*---------------------------------------------------------------------------
 *  itovf - interruption overflow
 *---------------------------------------------------------------------------
 */
interrupt _itOverflow()
{
   int (*sigfunc)();

   _itDis();
   if ((sigfunc = Tasktab[m_Getpid()].tevfunc[SIGOVF]) == SIG_DFL || \
        sigfunc == SIG_IGN)
        {
           sysmsg(errOvfStr,Tasktab[m_Getpid()].tname);
           m_Exit(-1);
        }
        else
           m_Kill(m_Getpid(),SIGOVF);
}

/*---------------------------------------------------------------------------
 *  itIllegal - interruption illegale instruction
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
 /*     for (i=tp->tstklen;i > 0;i--)
               m_Printf(" %lX",*(tp->tKstkbase-i));*/
/*           _panic("\nexit from it ILLEGAL\n");
        m_Exit(-1);*/
        }
        else
           m_Kill(m_Getpid(),SIGILL);
}

interrupt _dummy_interrupt()
{
}
