/* start.c */

#include "sys.h"

/*----------------------------------------------------------------------------
 * _launch - updates a task's state from SUSP to READY
 *----------------------------------------------------------------------------
 */
SYSTEMCALL _launch(pid, differed)
int pid , differed;
{
    int ps;
    struct taskslot *tptr;            /* points to task descriptors table */
    int prio;

    ps = _itDis();
    if ((((tptr = &Tasktab[pid])->tstate != SLEEP) && !(tptr->tevent & EV_SUSP)) ||
          (isbadpid(pid))) {
         tptr->terrno = (isbadpid(pid) ? EINVAL : EINVOP);
         _itRes(ps);
         return(RERR);
    }
    prio = tptr->tprio;
    _setrdy( pid , differed );   /* set to READY and immediately (-or- differ its) schedule */
    _itRes(ps);
    return(prio);

}
