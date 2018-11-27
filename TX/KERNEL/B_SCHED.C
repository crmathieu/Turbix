/* schedul.c */

#include "sys.h"
#include "q.h"
#include "pc.h"
#include "conf.h"
#include "tty.h"

int saveCurrTstate;

/*----------------------------------------------------------------------------
 * swpProc - Assigns the CPU to the READY task that have the highest priority
 *----------------------------------------------------------------------------
 */
_swpProc()
{
    struct taskslot *old,*new;
    int ps, Y, X;

    /* check if current task can keep CPU by comparing its
     * priority with the one of the highest priority task 
     * (#1 in READY task list)
     */

    if ((old = &Tasktab[RUNpid])->tstate == RUNNING) {
        if (firstkey(rdyhead) < old->tprio) {
            return(ROK);
        }
        else {
           /* the task in READY list has a higher or eq priority
            * than the current task: Insert current task in READY
            * list based on its priority level
            */

           old->tstate = READY;
           _insertKFR(RUNpid,rdytail,old->tprio);
       }
    }

    /* remove elected task from READY list and its state to RUNNING */

    new           = &Tasktab[RUNpid = _getfirst(SYSQ,rdyhead)];
    new->tstate   = RUNNING;

    /* At this point, we are still running on the previous task's stack. We need
     * to swap the new and previous tasks' stacks
     */

/*    X = tty[0].cursX;
    Y = tty[0].cursY;
    kprintf("\033[24;50H %8s is Running\033[%d;%dH",new->tname,Y,X);*/

    /* The assembler code swapstk is responsible to restore interruption 
     * (inhib = FALSE) before executing the RET instruction
     */
    _swapstk( &old->tstktop , new->tstktop );
    return(ROK);
}

/*----------------------------------------------------------------------------
 * _swpProcC - Assigns the CPU to the CALLOUT task. Note that if _swpProcC is
 *             invoked by the clock interruption routine just as the current 
 *             task just changed its state, schedulC will change it back to
 *             READY
 *----------------------------------------------------------------------------
 */
_swpProcC()
{
    struct taskslot *old,*new;
    int ps, Y, X,pid;

    ps = _disable();
    old           = &Tasktab[pid = RUNpid];
/*    old->tITvalid = itvalidee;*/

    /* set CALLOUT as RUNNING */
    new           = &Tasktab[RUNpid = CALLOUT];
    new->tstate   = RUNNING;

    /* move current task at the head of the READY task list */
    if (old->tstate == RUNNING) {
        old->tstate = READY;
        _enlist_head(pid,rdyhead);
    }
    else
        _systemHalt("SCHEDULING_C error\n");

/*    itvalidee     = new->tITvalid;*/

    _swapstk( &old->tstktop , new->tstktop );
    _restore(ps);
    return(ROK);
}


/*----------------------------------------------------------------------------
 * setrdy - set a task state as READY
 *----------------------------------------------------------------------------
 */
_setrdy(pid, flag_schedul)
int pid;                  /* process id to use in the READY task linked list */
int flag_schedul;         /* boolean specifying if the scheduler must be 
                             invoked after insertion in READY tasks list or not */

{
    int ps;
    struct taskslot *tp;

    ps = _itDis();
    if ( (isbadpid(pid)) || ((tp = &Tasktab[pid])->tstate == READY)) {
         _itRes(ps);
         return(RERR);
    }
    tp->tstate = READY;
    tp->tevent = 0;
    _insertKFR(pid, rdytail, tp->tprio);
    if (flag_schedul == SCHEDUL) {
        _swpProc();
    }

    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * m_GetPriority - returns the task priority given a process id
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_GetPriority(pid)
int pid;              /* task process id */
{
    int ps;
    struct taskslot *tp;

    ps = _itDis();
    if ((isbadpid(pid)) || ( tp = &Tasktab[pid])->tstate == UNUSED ) {
         tp->terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    tp->terrno = 0;
    _itRes(ps);
    return(tp->tprio);
}

/*----------------------------------------------------------------------------
 * m_Getpid - returns the RUNNING task's pid 
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Getpid()
{
    return(RUNpid);
}

/*------------------------------------------------
 * m_GetProcName - returns the RUNNING task's name
 *------------------------------------------------
 */
char * SYSTEMCALL m_GetProcName(pid)
int pid;
{
    int ps;

    ps = _itDis();
    if (isbadpid(pid)) {
        Tasktab[RUNpid].terrno = EINVAL;
        _itRes(ps);
        return(NULLPTR);
    }
    else {
        Tasktab[RUNpid].terrno = 0;
        _itRes(ps);
        return(Tasktab[pid].tname);
    }
}

/*-----------------------------------
 * m_SetProcName - sets a task's name
 *-----------------------------------
 */
SYSTEMCALL m_SetProcName(pid, name)
int pid;
char *name;
{
        int ps, i;

        ps = _itDis();
        if (isbadpid(pid)) {
                Tasktab[RUNpid].terrno = EINVAL;
                _itRes(ps);
                return(RERR);
        }

        /* copy task name */
        for (i = 0; i < TNMLEN ; i++)
                if ((Tasktab[pid].tname[i] = name[i]) == 0)
                        break;
        Tasktab[RUNpid].terrno = 0;
        _itRes(ps);
        return(ROK);
}

/*----------------------------------------------------------------------------
 * m_Getppid - returns the RUNNING task's parent task id
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Getppid()
{
    return(Tasktab[RUNpid].tppid);
}

/*----------------------------------------------------------------------------
 * m_Nice - lowers a task's priority
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Nice(increment)
int increment;
{
    int ps,newprio;
    struct taskslot *tp;

    ps = _itDis();
    if ((increment < 0) || ((newprio = (tp = &Tasktab[RUNpid])->tprio - increment) <= 0)) {
        tp->terrno = EINVAL;
        _itRes(ps);
        return(RERR);
    }
    _setprio(RUNpid,newprio);
    tp->terrno = 0;
    _itRes(ps);
    return(newprio);
}

/*----------------------------------------------------------------------------
 * _setprio - sets a task's priority to "newprio" and returns the previous 
 *            priority value
 *----------------------------------------------------------------------------
 */
_setprio(pid, newprio)
int pid;
int newprio;                  /* newprio > 0 */
{
    int ps;
    int oldprio;
    struct taskslot *tp;

    ps = _itDis();
    if ( pid < 0 || (newprio <= 0) ||
              (tp = &Tasktab[pid])->tstate == UNUSED ) {
        _itRes(ps);
        return(RERR);
    }
    if (newprio > MAXPRIO) {
        newprio = MAXPRIO;
    }
    oldprio     = tp->tprio;
    tp->tprio = newprio;
    if (tp->tstate == READY)  {
        _defect(SYSQ,pid);
        _insertKFR( pid , rdytail , newprio );
    }
    _itRes(ps);
    return(oldprio);
}

/*----------------------------------------------------------------------------
 * _kernelMode - switches to KERNEL mode
 *----------------------------------------------------------------------------
 */
_kernelMode(forcePUSHBP)
int forcePUSHBP;
{
   if (Tasktab[RUNpid].tkernel++ <= 0) {
       /* Saves SP USER and sets SP KERNEL */
       _UtoK(&Tasktab[RUNpid].tUSP,Tasktab[RUNpid].tKSP);
   }
}

/*----------------------------------------------------------------------------
 * _userMode - switches to USER mode
 *----------------------------------------------------------------------------
 */
_userMode(forcePUSHBP)
int forcePUSHBP;
{
   if (--Tasktab[RUNpid].tkernel <= 0) {
       /* replaces SP USER */
       _KtoU(Tasktab[RUNpid].tUSP);
   }
}

/*----------------------------------------------------------------------------
 * _itDis - inhibition logique des interruptions. Retourne le contexte
 *           du FLAG REGISTER logique
 *----------------------------------------------------------------------------
 */
SYSTEMCALL _itDis()
{
    int ps, prev;
    return(_disable());

    ps = _disable();
/*    prev = itvalidee;
    itvalidee = FALSE; */
    _restore(ps);
    return(prev);
}

/*----------------------------------------------------------------------------
 * _itEna - autorise physiquement les interruptions
 *----------------------------------------------------------------------------
 */
SYSTEMCALL _itEna()
{
    int ps,prev;

    _disable();
/*    itvalidee = TRUE;*/
    _enable();
}

/*----------------------------------------------------------------------------
 * _itRes - Restitue un contexte interruptif logique
 *----------------------------------------------------------------------------
 */
SYSTEMCALL _itRes(ps)
int ps;
{
   int pss;
   return(_restore(ps));

   pss = _disable();
/*   itvalidee = ps;*/
   _restore(pss);
/*   if (itvalidee && schedulReq)
       _swpProc();*/
}

