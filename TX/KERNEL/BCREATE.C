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
 * _makeProcess - build a task descriptor
 *----------------------------------------------------------------------------
 */
_makeProcess(procaddr,tasktyp,ssize, priority, name, nargs, args)
int   (* procaddr)();/* task starting address                              */
int    tasktyp;      /* user / system                                      */
ushort ssize;        /* stack size                                         */
int    priority;     /* task priority                                      */
char  *name;         /* task name                                          */
int    nargs;        /* total size of task parameters (in WORD)            */
                     /* CHAR = INT = SHORT = 1                             */
                     /* LONG = FLOAT = & = 2                               */
int  args ;          /* start parameters location                          */
{
     int ps;
     int child,parent;
     struct taskslot *tp,*tparent;
     int i;
     int  *stack, *argtask, *argp;

        ps = _itDis();
        if ( ssize < 48 || (child = _getnewpid()) == RERR || priority < 1 ||
           ( stack = _stackAlloc(ssize)) == (int *)NULL ) {
                 _itRes(ps);
                 return(RERR);
        }
        if (priority > MAXPRIO)
            priority = MAXPRIO;

        numproc++;                 /* add number of tasks in system */
        tparent = &Tasktab[parent = RUNpid];
        tp = &Tasktab[child];

        /* initialize descriptor */
        tp->tstate = SLEEP;
        tp->tevent = EV_SUSP;    /* initial state = SUSPENDED            */

        /* initialize task type */
        tp->ttyp   = tasktyp;

        /* retrieve parent's drive and and directory */
        tp->tcurrdev = tparent->tcurrdev;
        memset(tp->tcurrdir, 0, 64);
        fastcpy(tp->tcurrdir, tparent->tcurrdir, strlen(tparent->tcurrdir));

        /* copy task's name */
        for (i = 0; i < TNMLEN ; i++)
                if ((tp->tname[i] = name[i]) == 0)
                        break;

        /*
         * Initialize USER and SYSTEM stacks
         * (both stacks are in the same segment)
         * We start on the USER stack
         */

        tp->tprio          = priority;
        tp->tKstkbase      = stack;
        tp->tUstkbase      = stack;

        /* adjust USER stack to half the total stack size */
        FP_OFF(tp->tUstkbase) -= ssize/2;  

        tp->tstklen        = ssize;
        /* Convert stack size in # of integers
         * (ssize is the size in bytes) */
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
        tp->tuser          = tparent->tuser;
        tp->tgrp           = tparent->tgrp;
        tp->tkernel        = 0;      /* we start in user mode */
        tp->theadblk       = (struct hblk *)NULL;
        tp->ttailblk       = (struct hblk *)NULL;

        /* init file descriptors */
        for ( i = 0 ; i  < NFD ; i++)   tp->tfd[i] = NULLSTREAM;

        /* init signal actions to SIG_DFL */
        for ( i = 0 ; i < SIGNR ; i++)  tp->tevfunc[i] = SIG_DFL;

        /* use standards IO from parent */
        _dupSys(parent,child,stdin);
        _dupSys(parent,child,stdout);
        _dupSys(parent,child,stderr);

        /* establish affiliation between parent and child tasks */
        tp->tppid     = parent;

        /* initialize USER stack */
        stack          = tp->tUstkbase;
        argtask        = stack;
        tp->targc      = nargs;
/*        *stack--       = MAGIC;*/

        /* transfer task parameters onto its stack */
        argtask = (&args) + (nargs - 1);
        for ( ; nargs > 0 ; nargs--)
                            *stack-- = *argtask--;

        /* set the end task address:
         * test if 32 bits address
         */
        if (sizeof(int (*)()) > 2)   stack--;


        /* set end task address */
        switch(tasktyp) {
        case SYS_TASK   : *(int (**)())stack   = _system_end;break;
        case USER_TASK  : /* initialize D of C valid */
                          tp->tevsig = SigMask[SIGCLD];
                          *(int (**)())stack   = _user_end;break;
        case UMAIN_TASK : /* UMAIN */
                          *(int (**)())stack   = _umain_end;
                          tp->tevsig = SigMask[SIGCLD];break;
        }
        tp->retAddr = stack;

        /* place task's starting address (will be launched by a RET instruction) */
        stack -= sizeof(int (*)())/2;
        *(int (**)())(stack) = procaddr;

        /*  set the task launch context: reserve 4 integers
         *  for registers SI DI CR BP, then initialize CR
         */
        stack -= 4;
        tp->tstktop = stack;
        tp->tUSP           = FP_OFF(tp->tstktop);
        *stack             = ITVALID ;
        _itRes(ps);
        return(child);
}

/*----------------------------------------------------------------------------
 * m_Execl - exec with known number of parameters. The last parameter must be
 *           a null pointer (char *)0
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Execl(func, arg0)
int (* func)();
char *arg0;        /* 1st parameter pass� en parametre  */
{
     return(m_Exec(func, &arg0));
}

/*----------------------------------------------------------------------------
 * m_Exec - exec with unknown number of parameters
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Exec(func,argv)
int (*func)();     /* task starting address                               */
char *argv[];      /* begin of debut de la zone des arguments du process           */
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

    /* "topointerArea" pointe sur le buffer argument (ajust� sur octet paire)*/
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
 * stackexec - positionne le sommet de pile du process lanc� par exec
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
 * m_Fork - creation d'un process child heritant des memes fichiers que le parent
 *----------------------------------------------------------------------------
 */
SYSTEMCALL  m_Fork()
{
   int child,parent,ps;
   int stackcopy();
   struct taskslot *tp;

   ps = _itDis();
   tp = &Tasktab[parent = RUNpid];
   tp->terrno = 0;

   /* creer le child */
   if ((child = _makeProcess((int (*)())0, USER_TASK,tp->tstklen ,
                          USERPRIO , tp->tname, 0)) == RERR)
   {
        tp->terrno = EAGAIN;  /* too many processes */
        _itRes(ps);
        return(RERR);
   }
   /* recopier le contexte du parent sur le child */
    _kernelMode();
    _heapcopy(parent,child);
    _stackcopy(parent,child);

   /*  arrive a ce stade , le code est partage
    *  a la fois par le parent et le child
    *  si on est dans le parent , se remettre en mode
    *  USER et retourner le pid du child
    *  sinon retourner 0
    */
   if (parent == RUNpid)  {
       _userMode();
       _itRes(ps);
       return(child);
   }
   else  {
       _itRes(ps);
       return(0);
   }
}

/*----------------------------------------------------------------------------
 * stackcopy - copy the parent's USER stack onto the child's USER stack
 *----------------------------------------------------------------------------
 */
_stackcopy(parent,child)
int parent,child;
{
    int _user_end();
    struct taskslot *tparent,*tchild;
    int  *stkchild,*stkparent;
    long *aux;
    int i,ps,z;

    tparent = &Tasktab[parent];
    tchild = &Tasktab[child];

    /* position the child's USER stack top */
    tchild->tstktop = tchild->tUstkbase;
    FP_OFF(tchild->tstktop) -= (FP_OFF(tparent->tUstkbase) - tparent->tUSP);

    /* copy parent's USER stack onto child's */
    for ( i=0 , stkchild = tchild->tUstkbase , stkparent = tparent->tUstkbase;
          i < tparent->tUstklen ; i++)

                *stkchild-- = *stkparent--;  /* stack to stack */

    /*  place _stackcopy return @ and arguments (which happen to be on 
     *  parent's KERNEL stack) onto child's USER stack
     */
     stkchild = tchild->tstktop;
     stkparent = tparent->tKstkbase;

     /* 3 = (parent, child, bp) */
     for (i = 0 ; i < 3 + (sizeof(int (*)())/2) ; i++)
          *(--stkchild) = *(--stkparent);

     /* force type USER for child, regardless of parent's type */
     tchild->ttyp                 = USER_TASK;
     *(int (**)())tchild->retAddr = _user_end;

    /*  
     * condition child's USER stack to start through swapstk 
     */
    tchild->tstktop   = stkchild - 3;
    tchild->tUSP      = FP_OFF(tchild->tstktop);
    *tchild->tstktop  = ITVALID ;
/*    for (i=8;i>=0;i--)
                       kprintf("%x\n",*(tchild->tstktop+i));*/

    /*  
     * The child shares the parent's open files
     */
    for (i=3;i < NFD ;i++)
            _dupSys(parent, child, i);

    _launch(child, DIFFERED); /* the child starts after parent resumes its execution */
}

/*----------------------------------------------------------------------------
 * heapcopy - copy the parent's heap onto the child's heap
 *----------------------------------------------------------------------------
 */
_heapcopy(parent, child)
int parent, child;
{
    struct taskslot *tparent;
    uchar  *heapfils, *heapPere;
    struct hblk *ph, *fh;
    int i, ps;
    int *f, *p;

    tparent = &Tasktab[parent];

    for (ph = tparent->theadblk; ph != (struct hblk *)NULL; ph = ph->nextBlk) {
           if ((f = _MXmalloc(child, ph->blen - HBLK_SIZE, NORMALB)) == (int *)NULL)
                return(RERR);

           /* copy block */
           p = (int *)ph;
           FP_SEG(p) += HBLK_SIZE;
           fastcpy(f, p, ph->blen - HBLK_SIZE);
           fh = (struct hblk *)f;
           FP_SEG(fh) -= HBLK_SIZE;
           ph->dupb = fh;  /* points to header of duplicated block */
    }
}
/*----------------------------------------------------------------------------
 * _umain_end - stops user main
 *----------------------------------------------------------------------------
 */
SYSTEMCALL  _umain_end()
{
    int fd;

    /* close open files */
    for (fd = NFD-1 ;fd >= 0 ;fd--)
         if (Tasktab[RUNpid].tfd[fd] != NULLSTREAM)
             _closeSys(RUNpid,fd);

    m_Msgsync(SHTDWN); /* system shutdown */
    _dokill(RUNpid);
}

/*----------------------------------------------------------------------------
 * m_Shutdown - shutdown dunix and return to dos
 *----------------------------------------------------------------------------
 */
SYSTEMCALL  m_Shutdown()
{
    m_Msgsync(SHTDWN);
}

/*----------------------------------------------------------------------------
 * user_end - end of user task
 *----------------------------------------------------------------------------
 */
_user_end()
{
   m_Exit(0);
}

/*----------------------------------------------------------------------------
 * system_end - end of system task.
 *----------------------------------------------------------------------------
 */
_system_end()
{
   _dokill(RUNpid);
}

/*----------------------------------------------------------------------------
 * getnewpid - Looks for a free slot in Qliste. When possible, returns the 
 *             slot offset in Qliste (the task pid)
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
 * overwriteDescriptor - reinitialize descriptor for execl , execv
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

    /* free allocated blocks */
    for (hd = tp->theadblk; hd != (struct hblk *)NULL; ) {
         auxB = hd->nextBlk;
         FP_SEG(hd) += HBLK_SIZE; /* self align on block USER address */
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

     /* force USER task type */
     tp->ttyp           = USER_TASK;
     *(int (**)())tp->retAddr = _user_end;

     tp->theadblk       = (struct hblk *)NULL;
     tp->ttailblk       = (struct hblk *)NULL;

     /* initialize file descriptors, except standard IO */
     for ( i = 3 ; i  < NFD ; i++)   m_Close(i);

     /* set up the user number */
     if  ((i = tp->tfd[0]->s_minor -(NVS-1)) > 0 )
           tp->tuser = i;
     else
           tp->tuser = 0;
/*     tp->tgrp = tp->tfd[0]->s_minor;    /* no groupe */
}

