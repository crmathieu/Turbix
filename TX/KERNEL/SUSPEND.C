/* suspend.c */

#include "sys.h"
#include "q.h"

/*----------------------------------------------------------------------------
 * suspend - place un process dans l'etat SUSP
 *----------------------------------------------------------------------------
 */
SYSTEMCALL suspend(pid)
int pid;
{
    int ps;
    struct taskslot *tp;
    int prio;

    ps = _itDis();
    if ((isbadpid(pid)) || ( pid == TASK0 )) {
        Tasktab[RUNpid].terrno = EINVAL;
        _itRes(ps);
        return(RERR);
    }

    if (((tp = &Tasktab[pid])->tstate != RUNNING) && (tp->tstate != READY)) {
          tp->terrno = EINVOP;
          _itRes(ps);
          return(RERR);
    }
    tp->tstate = SLEEP;
    tp->tevent |= EV_SUSP;
    if ( tp->tstate == READY )     dequeue(SYSQ,pid);
    else                           scheduler();

    prio = tp->tprio;
    _itRes(ps);
    return(prio);
}


