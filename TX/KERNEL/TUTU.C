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



/*  Sauvegarde des vecteurs d'nterruptions
 *  au moment du demarrage
 */

long               msdosv_clk , msdosv_kbd , msdosv_com , msdosv_div0;
long               msdosv_ovf , msdosv_ill, msdosv_int15, msdosv_int61,
                   msdosv_mou,  msdosv_int24;

int               (* msdosv_floppy)(), (* msdosv_disk)();

int                currDrive;
char               currWD[80];
extern   int      *_allocmem();
Bool               fpanic;
int                stime[3],sdate[3];
long               begintime,endtime;
char           *MachineID = 0xf000fffe;
uchar              Machine;
extern   int       vid_vs;
extern   long      VStab;
int                demarrage, sysrun;
extern   int       iorun;
int               *cronState;
int far *deb1; /**/

extern  int        rdyhead,rdytail,piphead,piptail;
extern  long       time();
char              *getcwd();
int                GlobalArgc;
char             **GlobalArgv;

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
     if (FP_OFF(debaddr) == -1)
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

    GlobalArgv     = argv;
    GlobalArgc     = argc;

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
    tp->tprio    = 0;
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
    tp->theadblk = NULL;
    tp->ttailblk = NULL;
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
        printf(errFreeStr,ret);
        return;
    }

    /* revenir au DOS en effa�ant l'�cran */
    putchar('\x0c');
    gotoxy(0, 0);
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
    int           _vio(),_service(),_clockT(), _mou_task();
    int           interrupt _itOverflow();
    extern int    _int_itDiv0(),_int_itIllegal(), mouse_present;
    int           _premain(),_itdisk(), _int15(), _itcritical();
    int           interrupt _dummy_interrupt();

    /* On peut utiliser les int 21h jusqu'� la modification
     * des vecteurs d'interruption
     */
    _disable();
    _sysinit();

    /* placer les vecteurs d'interruptions gerant les
     * signaux et les memoires de masse
     */
    _sasVector(XENDIV0,_int_itDiv0,    &msdosv_div0);
    _sasVector(XENOVF, _itOverflow,&msdosv_ovf );
    _sasVector(XENILL, _int_itIllegal, &msdosv_ill );
    _sasVector(0x15, _int15, &msdosv_int15);
    _sasVector(0x61, msdosv_int15, &msdosv_int61);
    _sasVector(0x24, _itcritical, &msdosv_int24);
/*    _sasVector(0x33, _dummy_interrupt, &msdosv_mou);*/

    /* determiner le type de MACHINE */
    Machine = *MachineID;

    /* ignorer tous les signaux */
    for (i=0;i<SIGNR;i++)
         signal(i,SIG_IGN);

    /* mettre en place les E/S standards */
    fd = open("/dev/vs0", O_RDWR);
    dup(fd);
    dup(fd);

    /*  lancer les taches systemes - ATTENTION  : toute modification
     *  de l'ordre de construction des taches systemes entraine une
     *  mise a jour des constantes d'Identification des taches dans
     *  "proc.h"
     */
    _start(_buildTask(_clockT     , SYS_TASK,2048 , 50 ,"callout", 0 ),IMMEDIAT);
    _start(_buildTask(_service    , SYS_TASK,1024 , 30 ,"service", 0 ),IMMEDIAT);
    _start(_buildTask(_ioTsk      , SYS_TASK,2048 , 20 ,"e/s"    , 0 ),IMMEDIAT);
    _start(_buildTask(_shutdown   , SYS_TASK,1024 , 55 ,"shtdwn" , 0 ),IMMEDIAT);
    _start(_buildTask(_vio        , SYS_TASK,1024 , 25 ,"vio"    , 0 ),IMMEDIAT);

    /*_start(_buildTask(_mou_task   , SYS_TASK,1024 , 15 ,"_mst"   , 0 ),IMMEDIAT);*/


    enable();

    /* lancer la tache utilisateur apres avoir positionn� la directory
     * de travail dans la table des USERS
     */
    _start(_buildTask(_premain    , SYS_TASK, 4096, 10 ,"main"   , 0 ),DIFFERED);

    /* IDLE loop */
    sysrun = 1;
    while (sysrun)
                  _schedul();

    /* Normal Shutdown */
FIN:
    _disable();
    _rVector(XENCLOCK,msdosv_clk);
    _rVector(XENKBD  ,msdosv_kbd);
    _rVector(XENCOM  ,msdosv_com);
    _rVector(XENDIV0 ,msdosv_div0);
    _rVector(XENOVF  ,msdosv_ovf);
    _rVector(XENILL  ,msdosv_ill);
    _rVector(0x15  ,  msdosv_int15);
    _rVector(0x61  ,  msdosv_int61);
    _rVector(0x24  ,  msdosv_int24);

    /*if (!mouse_present)
        _rVector(0x33  ,  msdosv_mou);
    else {
        _m_installed(&i);
        _m_callMaskAndAddress(0, 0L);
    }*/

    enable();
}

/*---------------------------------------------------------------------------
 * _premain - lancer la tache utilisateur apres avoir positionn� la directory
 *           de travail dans la table des USERS
 *---------------------------------------------------------------------------
 */
TASK _premain()
{
    int umain();
    ulong nbytes;
    extern int devGET;


    /* afficher Copyright et memoire disponible */
    nbytes   = FP_SEG(maxaddr) - FP_SEG(debaddr);
    nbytes   = (nbytes + 1) * 16;

    _doCursorShape(CURS_SMALL);
    execv(umain,GlobalArgv);
}

/*---------------------------------------------------------------------------
 * shutdown - sortir proprement de ulk - Les fichiers encore ouverts sont
 *            automatiquement ferm�s par DOS
 *---------------------------------------------------------------------------
 */
TASK _shutdown()
{
   uchar mess;
   int stat, i, ps;
   extern SEM iosem;

   /* attente message de shutdown */
   msgwait(&mess, -1, &stat);

   ps = disable();
   iorun = 0;
   _setprio(E_S, 60);           /* s'assurer qu'une IO dos n'est pas en cours */
   _sigsem(iosem, NOSCHEDUL);   /* reveiller le process manager d'E/S disque */
   restore(ps);

   /* attente message de shutdown depuis IOTASK */
   msgwait(&mess, -1, &stat);

   /* tuer les taches en cours */
   for (i=NSYSTASK; i<NTASK; i++) {
        if (Tasktab[i].tstate == UNUSED)
            continue;
        _doKill(i);
   }

   /* donner la main � la tache IDLE */
   ps = disable();
   sysrun = 0;
   _setprio(TASK0,70);
   restore(ps);
}



/*---------------------------------------------------------------------------
 *  itDiv0 - interruption de division par 0
 *---------------------------------------------------------------------------
 */
_itDiv0()
{
   int (*sigfunc)();

   disable();

   if ((sigfunc = Tasktab[getpid()].tevfunc[SIGDIV]) == SIG_DFL || \
        sigfunc == SIG_IGN)
        {
           sysmsg(0,errDiv0Str,Tasktab[getpid()].tname);
           mkdExit(-1);
        }
        else
           kill(getpid(),SIGDIV);
}

/*---------------------------------------------------------------------------
 *  itovf - interruption overflow
 *---------------------------------------------------------------------------
 */
interrupt _itOverflow()
{
   int (*sigfunc)();

   disable();
   if ((sigfunc = Tasktab[getpid()].tevfunc[SIGOVF]) == SIG_DFL || \
        sigfunc == SIG_IGN)
        {
           sysmsg(0,errOvfStr,Tasktab[getpid()].tname);
           mkdExit(-1);
        }
        else
           kill(getpid(),SIGOVF);
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
   disable();

   if ((sigfunc = (tp = &Tasktab[getpid()])->tevfunc[SIGILL]) == SIG_DFL || \
        sigfunc == SIG_IGN) {
        sysmsg(0,errIIStr,tp->tname,tp->tKstkbase);
        mkdExit(-1);
 /*     for (i=tp->tstklen;i > 0;i--)
               printf(" %lX",*(tp->tKstkbase-i));*/
/*           panic("\nexit from it ILLEGAL\n");
        mkdExit(-1);*/
        }
        else
           kill(getpid(),SIGILL);
}

interrupt _dummy_interrupt()
{
}

