/* start.c */

#include "sys.h"

/*----------------------------------------------------------------------------
 * _launch - place un process de l'‚tat SUSP … l'‚tat READY
 *----------------------------------------------------------------------------
 */
SYSTEMCALL _launch(pid,differed)
int pid , differed;
{
    int ps;
    struct taskslot *tptr;            /* pointe sur la table des descripteurs */
    int prio;

    ps = _itDis();
    if ((((tptr = &Tasktab[pid])->tstate != SLEEP) && !(tptr->tevent & EV_SUSP)) ||
          (isbadpid(pid))) {
         tptr->terrno = (isbadpid(pid) ? EINVAL : EINVOP);
         _itRes(ps);
         return(RERR);
    }
    prio = tptr->tprio;
    _setrdy( pid , differed );   /* lancer en differ‚ ou immediatement */
    _itRes(ps);
    return(prio);

}
