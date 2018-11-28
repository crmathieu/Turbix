/* kill.c */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "signal.h"
#include "sem.h"


#define NOPIPE -1
extern unsigned *deb1;

/*----------------------------------------------------------------------------
 * _doZombi - kills a task and sets its state as ZOMBI
 *----------------------------------------------------------------------------
 */
_doZombi(pid)
int pid;
{
    struct taskslot *tp;
    struct hblk *hd, *auxB;
    int dev;
    int fd, ps;

    ps = _itDis();
    (tp = &Tasktab[pid])->terrno = 0;
    if (( isbadpid2(pid)) || tp->tstate == UNUSED ) {
        /* kprintf("BAD KILL: pid = %d etat = %d\n",pid,tp->tstate);*/
        tp->terrno = EINVAL;
        _itRes(ps);
        return(RERR);
    }

    /* free allocated blocks */
    for (hd = tp->theadblk; hd != (struct hblk *)NULL; ) {
        auxB = hd->nextBlk;
        /* m_Printf("BL=%lx TYP=%x NXT=%lx\n",hd,hd->sig,auxB);*/
        FP_SEG(hd) += HBLK_SIZE; /* self align on block USER address */
        _xfree(hd, pid);
        hd = auxB;
    }

    /* close opened files */
    for (fd = NFD-1 ;fd >= 0 ;fd--) {
        if (tp->tfd[fd] != NULLSTREAM) {
            _closeSys(pid,fd);
        }
    }

    if (tp->tflag & F_DELAY) {
        _stopdelay(pid);
    }
    tp->tevsig = 0;   /* erase set signals */

    switch(tp->tstate) {
        case RUNNING :
            break;
        case READY   :
            _defect(SYSQ,pid) ;         /* remove task from its list */
            break;
        case SLEEP   :
            if (tp->tevent & EV_SEM) {
                Semtab[tp->tsem].semcnt++; /* remove task from semaphore list */
                _defect(SYSQ,pid) ;        /* and remove task from its list */
            }
            break;
        default      :
            break;
    }

    tp->tstate = SLEEP;
    tp->tevent = EV_ZOM;
    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * _dokill - kills a task
 *----------------------------------------------------------------------------
 */
_dokill(pid)
int pid;
{
    int ps;
    struct taskslot *tp;

    ps = _itDis();
    tp = &Tasktab[pid];
    numproc--;
    _doZombi(pid);

    /* free task's stack */
    /* kprintf("FREE STACK ...\n"); */
    _sfree(tp->tKstkbase,tp->tstklen, pid);

    tp->tstate = UNUSED;
    if (pid == RUNpid) {
        _swpProc();
    }
    _itRes(ps);
}

/*----------------------------------------------------------------------------
 * doTerminate - ends a task previously in ZOMBI state
 *----------------------------------------------------------------------------
 */
_doTerminate(pid)
int pid;
{
    int ps;
    struct taskslot *tp;

    ps = _itDis();
    tp = &Tasktab[pid];
    /* kprintf("\nDOTERMINATE ...\n");*/

    /* free task's stack */
    /* kprintf("FREE STACK ...\n");*/
    _sfree(tp->tKstkbase,tp->tstklen, pid);

    numproc--;
    tp->tstate = UNUSED;
    _itRes(ps);
}


/*----------------------------------------------------------------------------
 * m_Kill - send an event to another task
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Kill(pid,signum)
int pid,signum;
{
    int ps;

    if (signum == SIGKILL) {
        return(_sendsig(Tasktab[pid].tppid,SIGCLD,-2,pid));
    } 
    return(_sendsig(pid,signum));
}


/*----------------------------------------------------------------------------
 * m_Exit - normal end task
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Exit(status)
int status;
{
    /* kprintf("\nexit %s \n",Tasktab[m_Getpid()].tname);*/
    /* notify parent that this task is done */
    _sendsig(Tasktab[RUNpid].tppid, SIGCLD, status, RUNpid);
}