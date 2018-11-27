/* init.c */

#include "sys.h"
#include "conf.h"
#include "signal.h"
#include "sem.h"
#include "q.h"
#include "tty.h"
#include "io.h"
#include "shell.h"
#include "const.h"
#include "fspipe.h"
#include "itbios.h"
#include "dynlink.h"
#include "ipc.h"
#include "msg.h"
#include "dis.h"

/* Dynamic linking with windows manager */
int (* winfunc[NWINFUNC])();

/* task descriptor table with its indexes */
struct taskslot Tasktab[NTASK];
int             nextslot;
unsigned        nextproc;
ushort          glowdir,
                glowdev;

/* Message linked lists table */
struct msgqid_ds Qtable[NQID];

/* Semaphores table */
struct semslot Semtab[NSEM];
int            nextsem;

/* Delays and waiting lists control table */
struct node    Sysq[NSYS];
struct node    Clkq[NCLK];
int            nextqueue;

/* Pipes table */
struct pipslot Piptab[NPIPE];
int            piphead;
int            piptail;

/* System variables */
int            numproc;            /* number of tasks in READY state  */
int            RUNpid;             /* PID of active task (RUNNING state) */

int            rdyhead;            /* READY list head */
int            rdytail;            /* READY list tail */

int            lockhead;           /* LOCK list head */
int            locktail;           /* LOCK list tail */

/* TTYS variables declaration */
struct csr     Serialtab[NSERIAL]; /* one Serial entry per tty serie */

struct session SessionTab[NTTY];
struct tty     tty[NTTY];          /* 1 tty + 10 Sessions */
extern  int    currDrive;
extern  char   currWD[];

/* configuration variables */
unsigned         materialConf;
extern unsigned *deb1;
int              nflp_phy;
struct LOL      *lolp, *getListOfList();

/* Secondary storage management */
struct DriveTable {
        char drive;
        char valide;
        char *path;
};

/* pointer to table of drives (dynamically allocated) */
struct DriveTable *pdt;

/* possible number of drives */
int lastdrive;

/* user error handler */
int (*_uErrHandler)();

/*----------------------------------------------------------------------------
 *  sysinit - system initialization
 *----------------------------------------------------------------------------
 */
_sysinit()
{
    int i,u,j;
    struct semslot *sptr;
    struct pipslot *pp;


    numproc   = 0;
    nextproc  = 1;
    nextslot  = 1;
    nextsem   = NSEM  - 1;
    nextqueue = NTASK;

/*    lolp = getListOfList();*/

    /* read hardware configuration */
    materialConf = _getconf();
    nflp_phy     = ((materialConf & 0x00c0) >> 6) + 1;


    /* init user error handler as NULLPTR */
    _uErrHandler = (int(* )())NULLPTR;

    /* initialize drives table */
    _initDriveTable();

    /* initialize users table */
    currDrive = _GetCurrDrive() + 1;  /* A = 1, B = 2, C = 3 etc...*/

    /* obtain current directory */
    _getcwdir(currWD, currDrive); 

    glowdev = currDrive;         /* initial current device */
    glowdir = 0;                 /* root as defaut */

    /* initialize SESSIONS table */
    for (u = 0;u < NTTY; u++) {
        for (i = 0; i < NDISK ; i++) {
            SessionTab[u].u_drive[i].cwd    = 0;
            SessionTab[u].u_drive[i].deepth = 0;
            for (j=0; j<MAX_PATH_DEEPTH ; j++) {
                SessionTab[u].u_drive[i].pathstr[j][0] = '\0';
            }
        }
        SessionTab[u].u_currDev = currDrive;
    }

    /* initialize tasks table except for slot 0 (initial task) which we are by default running on */
    for ( i = 1; i < NTASK; i++) {
        Tasktab[i].tstate = UNUSED;
    }

    /* initialize READY list */
    rdytail = 1 + ( rdyhead = _makeList(TAIL_TO_HEAD) );

    /* initialize LOCKS management */
    locktail = 1 + ( lockhead = _makeList(HEAD_TO_TAIL));

    /* initialize PIPES management */
    piptail = 1 + ( piphead = _makeList(HEAD_TO_TAIL));
    for (i = 0; i < NPIPE; i++) {
        (pp = &Piptab[i])->invalid = FALSE;
        pp->pipe_nr = i;
        pp->pipzon  = NULLPTR;
    }

    /* initialize semaphores */
    for (i = 0; i < NSEM; i++) {
        (sptr = &Semtab[i])->sstate = SFREE;
        sptr->sqtail = 1 + (sptr->sqhead = _makeList(HEAD_TO_TAIL));
    }

    /* initialize messages queues */
/*    _init_msg();*/


    /* initialize devices */
    _clkInit();                   /* clock */
    _VIOinit();                   /* virtual screen 0, 1,.., 9,    TTY1 */

    /* authorize All interrupts */
    outp(0x21, 0);
}

/*---------------------------------------------------------------------------
 *  sasVector - "save and set interrupt vector"
 *---------------------------------------------------------------------------
 */
_sasVector(vec_nbr, new_vector ,old_vector)
int vec_nbr;
int (far *new_vector)();
ulong *old_vector;
{
    int ps;
    ulong *vector;

    ps = _itDis();
    FP_SEG(vector) = 0;
    FP_OFF(vector) = vec_nbr * 4;

    *old_vector        = *vector;
    *vector            = (ulong)new_vector;

    _itRes(ps);
}

/*---------------------------------------------------------------------------
 *  rVector - "restore interrupt vector"
 *---------------------------------------------------------------------------
 */
_rVector(vec_nbr, addr)
int vec_nbr;
int (far *addr)();
{
    ulong  *vector;

    FP_SEG(vector) = 0;
    FP_OFF(vector) = vec_nbr * 4;

    *vector = (ulong)addr;
}

/*---------------------------------------------------------------------------
 *  getVector - obtain the interrupt vector for a given interrupt number
 *---------------------------------------------------------------------------
 */
int (far *_getvector(vec_nbr))()
int vec_nbr;
{
    int (far *vector)();

    FP_SEG(vector) = 0;
    FP_OFF(vector) = vec_nbr * 4;
    return(*vector);
}

/*--------------------------------------------------------------------------
 * getListOfList - retourne l'@ de LOL - Faire @ - 2 pour inclure le seg du
 *                 1er bloc ARENA
 *--------------------------------------------------------------------------
 */
struct LOL *getListOfList()
{
        struct LOL *lolp; /*, *_getlol();*/
        unsigned offset;

        lolp = _getlol();

        if (FP_OFF(lolp) < 4) {
            FP_SEG(lolp) -= 1;
            FP_OFF(lolp) += 16;
        }
        FP_OFF(lolp) -= 2;
        return(lolp);
}

/*--------------------------------------------------------------------------
 * initDriveTable - recupere les chemins par defaut sur tous les drives
 *                  disponible;
 *--------------------------------------------------------------------------
 */
_initDriveTable()
{
    int i;
    char aux[65];
    struct DriveTable *pdrive;
    lastdrive = 26;

    if ((pdt = (struct DriveTable *)_MXmalloc(0, sizeof(struct DriveTable) * (lastdrive+1), SPECIALB)) == (struct DriveTable *)NULL)
        return(-1);

    pdrive = pdt;
    for (i = 3; i < lastdrive+1; i++) {
        if (_getcwdir(aux, i))  /* erreur */
                (pdrive+i)->valide = FALSE;
        else  {
                (pdrive+i)->valide = TRUE;
                (pdrive+i)->drive  = 'A'+ (i-1);
                (pdrive+i)->path   = (char *)_MXmalloc(0, 64, SPECIALB);
                strcpy((pdrive+i)->path, aux);
        }
    }
    (pdrive+1)->valide = TRUE;
    (pdrive+1)->drive  = 'A';
    (pdrive+1)->path   = (char *)_MXmalloc(0, 64, SPECIALB);
    (pdrive+1)->path[0]= '\0';
    if (nflp_phy < 2)
        (pdrive+2)->valide = FALSE;
    else {
        (pdrive+2)->valide = TRUE;
        (pdrive+2)->drive  = 'B';
        (pdrive+2)->path   = (char *)_MXmalloc(0, 64, SPECIALB);
        (pdrive+2)->path[0]= '\0';
    }
}