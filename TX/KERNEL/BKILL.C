/* kill.c */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "signal.h"
#include "sem.h"


#define NOPIPE -1
extern unsigned *deb1;

/*----------------------------------------------------------------------------
 * _doZombi - tue une tache physiquement et la place dans l'etat ZOMBI
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
/*                 kprintf("BAD KILL: pid = %d etat = %d\n",pid,tp->tstate);*/
          tp->terrno = EINVAL;
          _itRes(ps);
          return(RERR);
    }

       /* liberer les blocs allou‚s */
       for (hd = tp->theadblk; hd != (struct hblk *)NULL; ) {
            auxB = hd->nextBlk;
/*               m_Printf("BL=%lx TYP=%x NXT=%lx\n",hd,hd->sig,auxB);*/
            FP_SEG(hd) += HBLK_SIZE; /* s'aligner sur l'adresse USER du bloc */
            _xfree(hd, pid);
            hd = auxB;
       }
       /* fermer les fichiers encore ouverts */
       for (fd = NFD-1 ;fd >= 0 ;fd--)
            if (tp->tfd[fd] != NULLSTREAM)
                _closeSys(pid,fd);

       if (tp->tflag & F_DELAY)   _stopdelay(pid);
       tp->tevsig = 0;   /* RAZ signaux attendus */

       switch(tp->tstate)
       {
          case RUNNING :
               break;
          case READY   :
               _defect(SYSQ,pid) ;         /* retirer la T de sa file et */
               break;
          case SLEEP   :
               if (tp->tevent & EV_SEM) {
                    Semtab[tp->tsem].semcnt++; /* eliminer la tache de la
                                                  file du semaphore    */
                    _defect(SYSQ,pid) ;         /* retirer la T de sa file et */
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
 * _dokill - tuer completement une tache
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

   /* liberer la pile du process */
/*   kprintf("FREE STACK ...\n");*/
   _sfree(tp->tKstkbase,tp->tstklen, pid);

   tp->tstate = UNUSED;
   if (pid == RUNpid)
       _swpProc();
   _itRes(ps);
}

/*----------------------------------------------------------------------------
 * doTerminate - finir de tuer une tache (uniquement pour les taches ZOMBI)
 *----------------------------------------------------------------------------
 */
_doTerminate(pid)
int pid;
{
   int ps;
   struct taskslot *tp;

   ps = _itDis();
   tp = &Tasktab[pid];
/*   kprintf("\nDOTERMINATE ...\n");*/

   /* liberer la pile du process */
/*   kprintf("FREE STACK ...\n");*/
   _sfree(tp->tKstkbase,tp->tstklen, pid);

   numproc--;
   tp->tstate = UNUSED;
   _itRes(ps);
}


/*----------------------------------------------------------------------------
 * m_Kill - envoyer un evenement a une autre tache
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Kill(pid,signum)
int pid,signum;
{
   int ps;

   if (signum == SIGKILL)
       return(_sendsig(Tasktab[pid].tppid,SIGCLD,-2,pid));
   else
       return(_sendsig(pid,signum));
}


/*----------------------------------------------------------------------------
 * m_Exit - fin normale d'un process
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Exit(status)
int status;
{
 /*   kprintf("\nexit %s \n",Tasktab[m_Getpid()].tname);*/


    /* avertir le pere que c'est la fin */
    _sendsig(Tasktab[RUNpid].tppid,SIGCLD,status,RUNpid);
}