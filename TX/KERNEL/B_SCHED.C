/* schedul.c */

#include "sys.h"
#include "q.h"
#include "pc.h"
#include "conf.h"
#include "tty.h"

int saveCurrTstate;

/*----------------------------------------------------------------------------
 * swpProc - attribue le processeur  a la tache READY la plus prioritaire
 *----------------------------------------------------------------------------
 */
_swpProc()
{
    struct taskslot *old,*new;
    int ps, Y, X;

    /*  tester si la tache courante peut garder la CPU
     *  en comparant sa priorite a celle de la tache
     *  la plus prioritaire ( en tete de la file des READY )
     */

    if ((old = &Tasktab[RUNpid])->tstate == RUNNING) {
        if (firstkey(rdyhead) < old->tprio) {
            return(ROK);
        }
        else {
           /*  la tache en READY liste a une priorite
            *  superieure ou egale a la tache courante :
            *  placer la tache courante dans la file des
            *  READY suivant son niveau de priorite
            */

           old->tstate = READY;
           _insertKFR(RUNpid,rdytail,old->tprio);
       }
    }

    /*  retirer la tache elue de la file des READY et
     *  la placer dans l'etat RUNNING
     */

    new           = &Tasktab[RUNpid = _getfirst(SYSQ,rdyhead)];
    new->tstate   = RUNNING;

    /*  a cet instant , on est toujours sur la pile
     *  de l'ancienne tache . commuter les piles de
     *  l'ancienne et la nouvelle tache
     */
/*    X = tty[0].cursX;
    Y = tty[0].cursY;
    kprintf("\033[24;50H %8s is Running\033[%d;%dH",new->tname,Y,X);*/

    /* swapstk se charge de replacer inhib
     * a FALSE avant d'effectuer le RET
     */
    _swapstk( &old->tstktop , new->tstktop );
    return(ROK);
}

/*----------------------------------------------------------------------------
 * _swpProcC - attribue le processeur  a la tache CALLOUT
 *    BUG : si _swpProcC est appel‚ par l'IT clock alors que la tache courante
 *          vient de changer d'etat, schedulC la replace a l'etat READY
 *----------------------------------------------------------------------------
 */
_swpProcC()
{
    struct taskslot *old,*new;
    int ps, Y, X,pid;

    ps = _disable();
    old           = &Tasktab[pid = RUNpid];
/*    old->tITvalid = itvalidee;*/

    /* placer CALLOUT en etat running */
    new           = &Tasktab[RUNpid = CALLOUT];
    new->tstate   = RUNNING;

    /*  placer la tache courante en TETE de la file des READY */
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
 * setrdy - place un tache dans l'etat READY
 *----------------------------------------------------------------------------
 */
_setrdy(pid, flag_schedul)
int pid;                  /* pid du process … chainer ds la file des READY */
int flag_schedul;         /* booleen indiquant si l'ordonnanceur doit etre */
                          /* invoqu‚ apres l'insertion ou pas              */

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
    if (flag_schedul == SCHEDUL)   _swpProc();
    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * retourne la priorite d'un process donn‚
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_GetPriority(pid)
int pid;              /* pid de la tache dont on souhaite connaitre la prio */
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
 * retourne le pid de la tache active
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Getpid()
{
    return(RUNpid);
}

/*------------------------------------
 * retourne le nom de la tache active
 *------------------------------------
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

/*-------------------------
 * change le nom du process
 *-------------------------
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

        /* recopie le nom du process */
        for (i = 0; i < TNMLEN ; i++)
                if ((Tasktab[pid].tname[i] = name[i]) == 0)
                        break;
        Tasktab[RUNpid].terrno = 0;
        _itRes(ps);
        return(ROK);
}


/*----------------------------------------------------------------------------
 * retourne le pid de la tache parente
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Getppid()
{
    return(Tasktab[RUNpid].tppid);
}

/*----------------------------------------------------------------------------
 * nice - diminuer la priorite d'une tache
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
 * change la priorit‚ d'une tache et retourne l'ancienne priorit‚
 *----------------------------------------------------------------------------
 */
_setprio(pid,newprio)
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
    if (newprio > MAXPRIO) newprio = MAXPRIO;
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
 * _kernelMode - passer en mode KERNEL
 *----------------------------------------------------------------------------
 */
_kernelMode(forcePUSHBP)
int forcePUSHBP;
{
   if (Tasktab[RUNpid].tkernel++ <= 0)
       /* sauvegarder SP USER   et  placer SP KERNEL */
       _UtoK(&Tasktab[RUNpid].tUSP,Tasktab[RUNpid].tKSP);
}

/*----------------------------------------------------------------------------
 * userMode - passer en mode USER
 *----------------------------------------------------------------------------
 */
_userMode(forcePUSHBP)
int forcePUSHBP;
{
   if (--Tasktab[RUNpid].tkernel <= 0)
       /* replacer SP USER */
       _KtoU(Tasktab[RUNpid].tUSP);
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