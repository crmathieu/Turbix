/* proc.h */

/* tasks possible states */
#define RUNNING    1  /* task owns the CPU */
#define UNUSED     2  /* task not created yet */
#define READY      3  /* task competes to own the CPU */
#define SLEEP      4  /* task waits for an event to occur */

/* possible events for a task in SLEEP state */
#define  EV_SEM     1  /* blocked on semaphore */
#define  EV_ZOM     2  /* attente prise en compte du pere */
#define  EV_MESS    4  /* wait on message to receive */
#define  EV_TMESS   8  /* wait on message to receive with tempo */
#define  EV_PAUSE  16  /* wait for a signal */
#define  EV_CLOCK  32  /* wait for end of delay */
#define  EV_SUSP   64  /* wait on jumpstart */
#define  EV_PIPE  128  /* wait on empty pipe or pipe full */
#define  EV_WAIT  256  /* wait on child task to exit */
#define  EV_LOCK  512  /* wait on file locking */

/* Transitory states flag control */
#define  F_ALRM     1  /* watchdog delay is occuring */
#define  F_TIMOUT   2  /* a timeout occured */
#define  F_DELAY    4  /* a delay is currently set on this task */
#define  F_WAIT     8  /* parent task is waiting for child task */
#define  F_WAITED  16  /* parent task was waiting for child task but is not anymore */
#define  F_LOCK    32  /* task has a lock on a file */

/* miscellaneous definitions */

#define NTASK     50      /* max number of tasks in the system */
#define TNMLEN     8      /* max task name length */
#define ARGLEN   160      /* argument buffer length */

/* system tasks pid */

#define TASK0      0             /* id of initial task */
#define SCHEDULER  TASK0+1
#define CALLOUT    SCHEDULER+1   /* id delay handler task */
#define TIME       CALLOUT+1     /* id time handler task */
#define E_S        TIME+1        /* id diskio handler task */
#define SHTDWN     E_S+1         /* id system shutdown task */
#define VIO        SHTDWN+1      /* id keyboard handler task */
#define MOU_TASK   VIO+1         /* id session error handler task */
#define NSYSTASK   VIO+1         /* number of system tasks */

/* system constants */
#define BADPID    -1      /* pid is incorrect */
#define NFD       20      /* number of file descriptors */
#define SIGNR     16      /* number of signals (13 real) */
#define MAXMESS    8      /* number of possible NMX messages */


/* Tasks table */

struct   taskslot  {
        char          tname[TNMLEN];  /* task name */
        int           tuser;          /* user number */
        int           tgrp;           /* terminal id attached to task */
        int           targc;          /* tasks initial number of arguments */
        char          targbuf[ARGLEN];/* Arguments pointers table */
        stream_entry *tfd[NFD];       /* Streamtab pointers table */
        int           taddr;          /* tasks starting address */
        int           tppid;          /* parent task pid */
        char          tstate;         /* tasks state: RUNNING, READY, SLEEP, UNUSED */
        unsigned      tevent;         /* blocking event when state = SLEEP */
        uchar         tflag;          /* transitory states management flags */
        int           tprio;          /* task priority */
        int           tkernel;        /* number of nested KernelMode calls */
        int           tupid;          /* user pid                           */
        int           tITvalid;       /* Interruptions valid for this task? */
        int           ttyp;           /* task type: UMAIN, SYS, USER */

        /* devices management */
        int           tcurrdev;       /* current device for this task */
        char          tcurrdir[64];   /* pointer to current directory for this task */

        /* errors management */
        int           terrno;         /* last error number */

        /* signals magagement */
        unsigned      tevsig;             /* awaiting events bitmap */
        unsigned      tevcatch;           /* received events bitmap */
        int         (*tevfunc[SIGNR])();  /* SIGNAL functions pointers table */
        int           texitstatus;        /* task return status */

        /* wait on semaphore */
        int           tsem;           /* index of semaphore in semaphore table (if task state = WAIT) */

        /* messages */
        uchar          tmsg[MAXMESS], /* messages buffer */
                       tmsgnr,        /* number of awaiting message (under IT) */
                       tmsgIn,        /* input index */
                       tmsgOut;       /* output index */
        int            tnextT;        /* link tasks blocked in REC */
/*      struct hblk   *theadMsg;      /* messages list head pointer */
/*      struct hblk   *ttailMsg;      /* messages list tail */
/*      unsigned       tmsgFlag;      /* synchronization flag */

        /* pipes management */
        int           tpipe_nr;       /*  pipe number of pipe on which task is suspended (if any) */
        int           tpipe_op;       /* suspension type (on READ or WRITE) */

        /* gestion de la pile */
        int far      *tstktop;        /* top of current stack */
        int           tstklen;        /* stack length: Ustack length + Kstack length */
        int          *retAddr;        /* end task fonction pointer */

        /* USER mode stack */
        int far      *tUstkbase;       /* Ustack high @ backup for FREE (a.dyn) */
                                       /* *pbase = MAGIC */
        int           tUstklen;        /* Ustack length */
        int far      *tUstklim;        /* Ustack low @ ( overflow ) */
        unsigned      tUSP;            /* Ustack offset backup */

        /* KERNEL mode stack */
        int far *tKstkbase;            /* Kstack high @ backup for FREE (a.dyn) */
                                       /* *pbase = MAGIC */
        int           tKstklen;        /* Kstack length */
        int far      *tKstklim;        /* Kstack low @ ( overflow ) */
        unsigned      tKSP;            /* Kstack offset backup */

        /* memory management */
        struct hblk  *theadblk;        /* head of dynamically allocated blocks list */
        struct hblk  *ttailblk;        /* list tail */

};

/* stack description:

Lower Memory                                  Higher Memory
   _________________________________________________
  |                        |                        |
  |    Ustack (1/2)        |    Kstack (1/2)        |
  |________________________|________________________|
  ^                        ^                        ^
lim (Ulim)               Ubase                   base (Kbase)
*/

#define  isbadpid(x)      ( x <= 0   || x >= NTASK)
#define  isbadpid2(x)     ( x <= 1   || x >= NTASK)

extern struct taskslot Tasktab[];
extern int      numproc ;         /* number of task (in READY state) */
extern unsigned nextproc;         /* next task in line for CPU assignment */
extern int      RUNpid;           /* current task id */
extern int      nextslot;         /* next free slot in Tasktab */
extern ushort   glowdir;          /* default working directory */
extern ushort   glowdev;          /* default working device */
extern char    *errStr[];         /* Error messages */