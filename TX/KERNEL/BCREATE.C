/* create.c */

#include "sys.h"
#include "pc.h"
#include "conf.h"
#include "io.h"
#include "sem.h"
#include "signal.h"


#define  ITVALID  0x0200
#define  USERPRIO 10
#define  C_HUGE  (char huge *)

int syspid;
int _user_end(), _system_end(), _umain_end();
extern int SigMask[];

/*----------------------------------------------------------------------------
 * _makeProcess - construire le decripteur d'une tache
 *----------------------------------------------------------------------------
 */
_makeProcess(procaddr,tasktyp,ssize, priority, name, nargs, args)
int   (* procaddr)();/* adresse de demarrage du process                    */
int    tasktyp;      /* tache noyau ou non                                 */
ushort ssize;        /* taille de la pile                                  */
int    priority;     /* priorite du process                                */
char  *name;         /* nom du process                                     */
int    nargs;        /* taille total des arguments ( en WORD )             */
                     /* CHAR = INT = SHORT = 1                             */
                     /* LONG = FLOAT = & = 2                               */
int  args ;          /* debut zone arguments                               */
{
     int ps;
     int fils,pere;
     struct taskslot *tp,*tpere;
     int i;
     int  *stack, *argtask, *argp;

        ps = _itDis();
        if ( ssize < 48 || (fils = _getnewpid()) == RERR || priority < 1 ||
           ( stack = _stackAlloc(ssize)) == (int *)NULL ) {
                 _itRes(ps);
                 return(RERR);
        }
        if (priority > MAXPRIO)
            priority = MAXPRIO;

        numproc++;                 /* un process de plus dans le systeme */
        tpere = &Tasktab[pere = RUNpid];
        tp = &Tasktab[fils];

        /* initialiser le descripteur */
        tp->tstate = SLEEP;
        tp->tevent = EV_SUSP;    /* etat initial = suspendu            */

        /* initialiser le type de process */
        tp->ttyp   = tasktyp;

        /* recuperer drive et directory courante du pere */
        tp->tcurrdev = tpere->tcurrdev;
        memset(tp->tcurrdir, 0, 64);
        fastcpy(tp->tcurrdir, tpere->tcurrdir, strlen(tpere->tcurrdir));

        /* recopie le nom du process */
        for (i = 0; i < TNMLEN ; i++)
                if ((tp->tname[i] = name[i]) == 0)
                        break;

        /*
         *  initialiser la pile USER et la pile KERNEL
         *  ( les 2 piles sont dans le meme segment )
         *  On demarre sur la pile USER
         */

        tp->tprio          = priority;
        tp->tKstkbase      = stack;
        tp->tUstkbase      = stack;

        /* ajuster USER base a la moitie de la pile totale */
        FP_OFF(tp->tUstkbase) -= ssize/2;  /* LOUCHE */

        tp->tstklen        = ssize;
        /* BIG BUG trouv‚ : convertir la taille de la pile en nb integers
         * (car ssize est la taille en bytes)
         */
        tp->tUstklen       = tp->tKstklen = ssize/(2*sizeof(int));
        tp->tUstklim       = stack;
        FP_OFF(tp->tUstklim) = 0;

        tp->tKstklim       = tp->tUstkbase+1;
        tp->tKSP           = FP_OFF(stack);

        tp->tsem           = 0;
        tp->tmsgnr         = 0;
        tp->tmsgIn         = 0;
        tp->tmsgOut        = 0;
        tp->tflag          = 0;
/*        tp->twatchdog      = FALSE;*/
        tp->tevsig         = SigMask[SIGCLD];
        tp->tevcatch       = 0;
        tp->tITvalid       = TRUE;
/*        tp->tdelay         = FALSE;*/
        tp->tpipe_nr       = -1;
        tp->tuser          = tpere->tuser;
        tp->tgrp           = tpere->tgrp;
        tp->tkernel        = 0;      /* on demarre en mode USER */
        tp->theadblk       = (struct hblk *)NULL;
        tp->ttailblk       = (struct hblk *)NULL;

        /* initialiser les file descriptors */
        for ( i = 0 ; i  < NFD ; i++)   tp->tfd[i] = NULLSTREAM;

        /* initialiser les Actions de signal a SIG_DFL */
        for ( i = 0 ; i < SIGNR ; i++)  tp->tevfunc[i] = SIG_DFL;

        /* adopter les E/S standards du pere */
        _dupSys(pere,fils,stdin);
        _dupSys(pere,fils,stdout);
        _dupSys(pere,fils,stderr);

        /* affilier le pere et le fils */
        tp->tppid     = pere;

        /* initialiser la pile USER */
        stack          = tp->tUstkbase;
        argtask        = stack;
        tp->targc      = nargs;
/*        *stack--       = MAGIC;*/

        /* transferer les parametres de la tache sur sa pile */
        argtask = (&args) + (nargs - 1);
        for ( ; nargs > 0 ; nargs--)
                            *stack-- = *argtask--;

        /* mettre en place l'adresse de fin de la tache :
         * tester si adresse sur 32 bits
         */
        if (sizeof(int (*)()) > 2)   stack--;


        /* Mise en place adresse fin de tache */
        switch(tasktyp) {
        case SYS_TASK   : *(int (**)())stack   = _system_end;break;
        case USER_TASK  : /* initialiser D of C valide */
                          tp->tevsig = SigMask[SIGCLD];
                          *(int (**)())stack   = _user_end;break;
        case UMAIN_TASK : /* UMAIN */
                          *(int (**)())stack   = _umain_end;
                          tp->tevsig = SigMask[SIGCLD];break;
        }
        tp->retAddr = stack;

        /* placer l'@ de debut de la tache ( elle est lancee par un RET ) */
        stack -= sizeof(int (*)())/2;
        *(int (**)())(stack) = procaddr;

        /*  mettre en place le contexte de
         *  lancement de la tache : reserver 4 entiers
         *  pour SI DI CR BP ,puis initialiser le CR
         */
        stack -= 4;
        tp->tstktop = stack;
        tp->tUSP           = FP_OFF(tp->tstktop);
        *stack             = ITVALID ;
        _itRes(ps);
        return(fils);
}

/*----------------------------------------------------------------------------
 * m_Execl - exec avec nombre de parametres connu … l'avance
 *         le dernier argument pass‚ doit etre obligatoirement (char *)0
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Execl(func,arg0)
int (* func)();
/*char *arg0;        /* 1er argument pass‚ en parametre  */
{
     return(m_Exec(func, &arg0));
}

/*----------------------------------------------------------------------------
 * m_Exec - exec avec nombre de parametres non connu a l'avance
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Exec(func,argv)
int (*func)();     /* adresse de demarrage du process                     */
char *argv[];      /* debut de la zone des arguments du process           */
                   /* argv[0] par convention pointe sur le nom du process */
{
     int ps, i, *stack;

        ps = _itDis();

        /* tester si TTYP = UMAIN */
        if (Tasktab[RUNpid].ttyp == UMAIN_TASK) {
                m_Printf("EXEC call not allowed in 'umain' process\n");
                _itRes(ps);
                return(-1);
        }

        /* recopier le nom de la tache */
        for (i = 0; i < TNMLEN; i++)
             if ((Tasktab[RUNpid].tname[i] = argv[0][i]) == 0)
                break;

        /* initialiser le descripteur */
        _overwriteDescriptor();

        /* passer sur la pile systeme */
        _kernelMode();
        _stackexec(func,ps,argv);
}

/*----------------------------------------------------------------------
 * putArgv     - place les parametres  Argv0 Argv1 .. Argvn
 *               String0 String1 .. Stringn  dans le buffer argument
 *             retourne l'ajustement
 *----------------------------------------------------------------------
 */
 _putArgv(tp,argc,argv)
struct taskslot *tp;
int  argc;
char *argv[];
{
    char   **topointerArea;
    char    *tostringArea;
    int      i,adjust;

    i = 0;

    /*  taille totale =  sizeof(char *) * (argc + 1(NULL))
     *                +  argc * sizeof(char)
     *                +  Somme strlen(argv[i])
     */

    /* "topointerArea" pointe sur le buffer argument (ajust‚ sur octet paire)*/
    (char *)topointerArea = &tp->targbuf[adjust = isodd(FP_OFF(tp->targbuf))];

    /* "tostringArea" pointe sur la zone des strings */
    tostringArea = (char *)topointerArea + ((argc + 1) * sizeof(char *));

    /* placer les "argvi" et leur chaine associee */
    for (i = 0; argc > 0;i++, argc--)
    {
         *topointerArea++ = tostringArea;
         strcpy(tostringArea,argv[i]);
         tostringArea += strlen(tostringArea) + 1;
    }
    *topointerArea = NULL;
    return(adjust);
}

/*----------------------------------------------------------------------------
 * stackexec - positionne le sommet de pile du process lanc‚ par exec
 *----------------------------------------------------------------------------
 */
_stackexec(func,ps,argv)
int (*func)();
int ps;
char *argv[];
{
        /*  mettre en place le contexte de
         *  lancement de la tache : reserver 4 entiers
         *  pour SI DI CR BP ,puis initialiser le FLAG
         */
        int argc,*stack,*pourien;
        struct taskslot *tp;

        tp = &Tasktab[RUNpid];

        /* determiner le nbre de parametres */
        for (argc = 0 ; argc < 10 ; argc++)
            if (argv[argc] == NULL)   break;
/*            else kprintf("argv%d = %s\n",argc,argv[argc]);*/

        tp->targc    = argc;

        /*  placer les arguments d'appels et l'adresse de
         *  fin de la tache sur la pile USER
         */

/*        (char *)stack = (char *)tp->tUstkbase - putArgv(tp,argc,argv);*/
/*        putargv(tp,argc,argv); /**/
        (char *)stack = (char *)tp->tUstkbase;/**/
        *(--(char **)stack)  = &tp->targbuf[_putargv(tp,argc,argv)]; /**/
        *(--stack)           = argc;
        stack               -= sizeof(int (*)())/2;
        switch(tp->ttyp) {
        case SYS_TASK:   *(int (**)())stack   = _system_end;break;
        case UMAIN_TASK: *(int (**)())stack   = _umain_end;break;
        case USER_TASK:  *(int (**)())stack   = _user_end;break;
        }
        tp->retAddr = stack;
        stack               -= sizeof(int (*)())/2;

        /* placer l'@ de debut de la tache ( elle est lancee par un RET )
         * et reserver 4 int  pour bp si di flag
         */

        *(int (**)())stack  = func;
        tp->tstktop         = stack - 4;
        *tp->tstktop        = ITVALID ;
        tp->tUSP            = FP_OFF(tp->tstktop);

        /* forcer  mode USER */
        tp->tkernel = 0;
        _itRes(ps);

        /* basculer sur pile USER */
        _swapstk(&pourien,tp->tstktop);
}

/*----------------------------------------------------------------------------
 * m_Fork - creation d'un process fils heritant des memes fichiers que le pere
 *----------------------------------------------------------------------------
 */
SYSTEMCALL  m_Fork()
{
   int fils,pere,ps;
   int stackcopy();
   struct taskslot *tp;

   ps = _itDis();
   tp = &Tasktab[pere = RUNpid];
   tp->terrno = 0;

   /* creer le fils */
   if ((fils = _makeProcess((int (*)())0, USER_TASK,tp->tstklen ,
                          USERPRIO , tp->tname, 0)) == RERR)
   {
        tp->terrno = EAGAIN;  /* too many processes */
        _itRes(ps);
        return(RERR);
   }
   /* recopier le contexte du pere sur le fils */
    _kernelMode();
    _heapcopy(pere,fils);
    _stackcopy(pere,fils);

   /*  arrive a ce stade , le code est partage
    *  a la fois par le pere et le fils
    *  si on est dans le pere , se remettre en mode
    *  USER et retourner le pid du fils
    *  sinon retourner 0
    */
   if (pere == RUNpid)  {
       _userMode();
       _itRes(ps);
       return(fils);
   }
   else  {
       _itRes(ps);
       return(0);
   }
}

/*----------------------------------------------------------------------------
 * stackcopy - recopie la pile USER du pere sur la pile USER du fils
 *----------------------------------------------------------------------------
 */
_stackcopy(pere,fils)
int pere,fils;
{
    int _user_end();
    struct taskslot *tpere,*tfils;
    int  *stkfils,*stkpere;
    long *aux;
    int i,ps,z;

    tpere = &Tasktab[pere];
    tfils = &Tasktab[fils];

    /* positionner le sommet de la USER stack du fils */
    tfils->tstktop = tfils->tUstkbase;
    FP_OFF(tfils->tstktop) -= (FP_OFF(tpere->tUstkbase) - tpere->tUSP);

    /* recopie de la USER stack du pere sur celle du fils */
    for ( i=0 , stkfils = tfils->tUstkbase , stkpere = tpere->tUstkbase;
          i < tpere->tUstklen ; i++)

                *stkfils-- = *stkpere--;  /* stack to stack */

    /*  placer les arguments et l'@ retour de stackcopy (qui se trouve
     *  sur la pile KERNEL du pere ) sur la pile USER du fils
     */
     stkfils = tfils->tstktop;
     stkpere = tpere->tKstkbase;

     /* 3 = (pere,fils,bp) */
     for (i = 0 ; i < 3 + (sizeof(int (*)())/2) ; i++)
          *(--stkfils) = *(--stkpere);

     /* forcer le type USER pour le fils, qqs le type du pere */
     tfils->ttyp                 = USER_TASK;
     *(int (**)())tfils->retAddr = _user_end;

    /*  conditionner la pile USER du fils pour demarrage
     *  par swapstk
     */
    tfils->tstktop   = stkfils - 3;
    tfils->tUSP      = FP_OFF(tfils->tstktop);
    *tfils->tstktop  = ITVALID ;
/*    for (i=8;i>=0;i--)
                       kprintf("%x\n",*(tfils->tstktop+i));*/

    /*  le fils partage les fichiers ouverts
     *  par le pere
     */
    for (i=3;i < NFD ;i++)
            _dupSys(pere,fils,i);

    _launch(fils,DIFFERED); /* le fils demarre apres le pere */
}

/*----------------------------------------------------------------------------
 * heapcopy - recopie le tas du pere sur celui du fils
 *----------------------------------------------------------------------------
 */
_heapcopy(pere,fils)
int pere,fils;
{
    struct taskslot *tpere;
    uchar  *heapfils,*heapPere;
    struct hblk *ph, *fh;
    int i,ps;
    int *f, *p;

    tpere = &Tasktab[pere];

    for (ph = tpere->theadblk; ph != (struct hblk *)NULL; ph = ph->nextBlk) {
           if ((f = _MXmalloc(fils, ph->blen - HBLK_SIZE, NORMALB)) == (int *)NULL)
                return(RERR);

           /* copie du bloc */
           p = (int *)ph;
           FP_SEG(p) += HBLK_SIZE;
           fastcpy(f, p, ph->blen - HBLK_SIZE);
           fh = (struct hblk *)f;
           FP_SEG(fh) -= HBLK_SIZE;
           ph->dupb = fh;  /* pointer sur le header du bloc dupliqu‚ */
    }
}
/*----------------------------------------------------------------------------
 * _umain_end - stopper le main
 *----------------------------------------------------------------------------
 */
SYSTEMCALL  _umain_end()
{
    int fd;

    /* fermer les fichiers encore ouverts */
    for (fd = NFD-1 ;fd >= 0 ;fd--)
         if (Tasktab[RUNpid].tfd[fd] != NULLSTREAM)
             _closeSys(RUNpid,fd);

    m_Msgsync(SHTDWN); /* system shutdown */
    _dokill(RUNpid);
}

/*----------------------------------------------------------------------------
 * m_Shutdown - stopper le systeme et revenir sous DOS
 *----------------------------------------------------------------------------
 */
SYSTEMCALL  m_Shutdown()
{
    m_Msgsync(SHTDWN);
}

/*----------------------------------------------------------------------------
 * user_end - fin d'un programme utilisateur
 *----------------------------------------------------------------------------
 */
_user_end()
{
   m_Exit(0);
}

/*----------------------------------------------------------------------------
 * system_end - fin d'un programme systeme
 *----------------------------------------------------------------------------
 */
_system_end()
{
   _dokill(RUNpid);
}

/*----------------------------------------------------------------------------
 * getnewpid - recherche un slot libre ds la Qliste . qd c'est possible ,
 *          retourne l'offset du slot ds la Qliste ( c'est le pid du process )
 *----------------------------------------------------------------------------
 */
LOCAL _getnewpid()
{
    int pid;
    int i;
    for (i = 0; i < NTASK; i++)
    {
        if ((pid = nextslot++) >= NTASK)
             pid = nextslot = NSYSTASK;
        if (Tasktab[pid].tstate == UNUSED )  return(pid);
    }
    return(RERR);
}


/*----------------------------------------------------------------------------
 * overwriteDescriptor - reinitialise le descripteur pour execl , execv
 *----------------------------------------------------------------------------
 */
_overwriteDescriptor()
{
    int _user_end();
    struct taskslot *tp;
    short i;
    struct hblk *hd, *auxB;
    int dev;
    int pid, fd;

    tp = &Tasktab[pid = RUNpid];

    if (tp->tflag & F_DELAY)
        _stopdelay(pid);

    /* liberer les blocs allou‚s */
    for (hd = tp->theadblk; hd != (struct hblk *)NULL; ) {
         auxB = hd->nextBlk;
         FP_SEG(hd) += HBLK_SIZE; /* s'aligner sur l'adresse USER du bloc */
         _xfree(hd, pid);
         hd = auxB;
    }

     tp->tsem           = 0;
     tp->tmsgnr         = 0;
     tp->tmsgIn         = 0;
     tp->tmsgOut        = 0;
     tp->tevent         = 0;
     tp->tflag          = 0;
     tp->tITvalid       = TRUE;
     tp->tevcatch       = 0;
     tp->tpipe_nr       = -1;

     /* forcer le type USER */
     tp->ttyp           = USER_TASK;
     *(int (**)())tp->retAddr = _user_end;

     tp->theadblk       = (struct hblk *)NULL;
     tp->ttailblk       = (struct hblk *)NULL;

     /* initialiser les file descriptors , sauf les E/S standards */
     for ( i = 3 ; i  < NFD ; i++)   m_Close(i);

     /* mettre a jour le numero d'utilisateur */
     if  ((i = tp->tfd[0]->s_minor -(NVS-1)) > 0 )
           tp->tuser = i;
     else
           tp->tuser = 0;
/*     tp->tgrp = tp->tfd[0]->s_minor;    /* no groupe */
}

