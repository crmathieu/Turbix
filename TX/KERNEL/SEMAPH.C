/* sem.c */

#include "sys.h"
#include "q.h"
#include "sem.h"


/*----------------------------------------------------------------------------
 * m_Waitsem -   place la tache courante en fin de file si la
 *             ressource n'est pas disponible
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Waitsem(sem)
int sem;
{
   int ret;
   if ((ret = _waitsem(sem,STAIL)) == ROK)
        Tasktab[RUNpid].terrno = 0;
   return(ret);
}

/*----------------------------------------------------------------------------
 * m_Pwaitsem - place la tache courante en tete de file si la
 *            ressource n'est pas disponible
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Pwaitsem(sem)
{
   int ret;
   if ((ret = _waitsem(sem,SHEAD)) == ROK)
        Tasktab[RUNpid].terrno = 0;
   return(ret);
}

/*----------------------------------------------------------------------------
 * _waitsem - realise l'operation P
 *----------------------------------------------------------------------------
 */
_waitsem(sem,where)
{
    int ps;
    struct semslot *sptr;
    struct taskslot *tptr;

    ps = _itDis();
    if (isbadsem(sem) || ( sptr = &Semtab[sem])->sstate == SFREE ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    if (--(sptr->semcnt) < 0 )
    {            /* le semaphore est bloquant */
       (tptr = &Tasktab[RUNpid])->tstate = SLEEP;
        tptr->tevent = EV_SEM;
        tptr->tsem = sem;

        if (where == STAIL)
        /* placer le process en FIN DE FILE */
            _enlist_tail(SYSQ,RUNpid, sptr->sqtail);
        else
        /* placer le nouveau process EN TETE DE FILE */
            _enlist_head(RUNpid,sptr->sqhead);
        _swpProc();

    }
    _itRes(ps);
    return(ROK);             /* la ressource est disponible */
}

/*----------------------------------------------------------------------------
 * m_Sigsem   -  _sigsem avec  scheduling autorise
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Sigsem(sem)
{
   int ret;
   if ((ret = _sigsem(sem, SCHEDUL)) == ROK)
        Tasktab[RUNpid].terrno = 0;
   return(ret);
}

/*----------------------------------------------------------------------------
 * _sigsem   - autorise la disponibilite de la ressource pour la
 *            1ere tache en file d'attente (operation V) avec scheduling
 *            parametrable
 *----------------------------------------------------------------------------
 */
SYSTEMCALL _sigsem(sem, cflag) /* cflag : scheduling flag */
{
    int ps;
    struct semslot *sptr;

    ps = _itDis();
    if (isbadsem(sem) || ( sptr = &Semtab[sem])->sstate == SFREE ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    if (sptr->semcnt++ < 0)  /* d'autres process sont en SWAIT */
        _setrdy(_getfirst(SYSQ,sptr->sqhead) , cflag);

    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * m_Creatsem  - cree et initialise un semaphore
 *             retourne son offset ds la table systeme
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Creatsem(count,semname)
int count;                /* le compteur de sem doit etre >= 0 */
char *semname;
{
    int i,ps;
    int sem;

    ps = _itDis();
    Tasktab[RUNpid].terrno = 0;
    if ( count < 0 || ( sem = _getnewsem()) == RERR ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return (RERR);
    }

    Semtab[sem].semcnt = count ;
    for (i = 0 ; i < 8 ; i++)
        if ((Semtab[sem].sname[i] = semname[i]) == '\0')  break;
    for ( ; i < 8 ; i++)   Semtab[sem].sname[i] = '\x20';
    _itRes(ps);
    return(sem);
}

/*----------------------------------------------------------------------------
 * getnewsem - alloue un semaphore inutilise (head & tail) ds la table systeme
 *             et retourne son offset ( head )
 *----------------------------------------------------------------------------
 */
LOCAL _getnewsem()
{
    int sem;
    int i;

    for ( i=0; i<NSEM ; i++) {
        sem = nextsem--;
        if (nextsem < 0)   nextsem = NSEM - 1;
        if (Semtab[sem].sstate == SFREE ) {
           Semtab[sem].sstate = SUSED;
           return(sem);
        }
    }
    return(RERR);   /* il n'y a plus de semaphore disponible */
}

/*----------------------------------------------------------------------------
 * m_Delsem -  supprime un semaphore de la table des semaphores
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Delsem(sem)
{
    int ps;
    int pid;
    struct semslot *sptr;      /* adresse du semaphore … liberer  */

    ps = _itDis();

    if (isbadsem(sem) || Semtab[sem].sstate == SFREE ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }

    sptr = &Semtab[sem];
    sptr->sstate = SFREE;
    if (nonempty(sptr->sqhead)) { /* liberer les process en attente sur
                                     ce semaphore                       */
         while ((pid = _getfirst(SYSQ,sptr->sqhead)) != EMPTY )
                 _setrdy(pid , NOSCHEDUL );
         _swpProc();
    }
    Tasktab[RUNpid].terrno = 0;
    _itRes(ps);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * signsem   - autorise la disponibilite de la ressource pour les
 *            n 1ere taches en file d'attente
 *-----------------------------------------------------------------------------
 */
_signsem(sem, n)
{
    int ps;
    struct semslot *sptr;
    ps = _itDis();
    if (isbadsem(sem)||(sptr = &Semtab[sem])->sstate == SFREE || n <= 0)
    {
           _itRes(ps);
           return(RERR);
    }
    while(n--)         /* d'autres process sont en SWAIT */
      if (sptr->semcnt++ < 0)
          _setrdy(_getfirst(SYSQ,sptr->sqhead),NOSCHEDUL); /* leur donner la main */
    _swpProc();
    _itRes(ps);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * m_Resetsem - libere les taches en attente du semaphore "sem"
 *            puis remet le compteur a "n"
 *-----------------------------------------------------------------------------
 */
SYSTEMCALL m_Resetsem(sem, n)
{
    int ps;
    int pid;
    struct semslot *sptr;

    ps = _itDis();

    if (isbadsem(sem) || (sptr = &Semtab[sem])->sstate == SFREE) {
           Tasktab[RUNpid].terrno = EINVAL;
           _itRes(ps);
           return(RERR);
    }
    sptr = &Semtab[sem];
    if (nonempty(sptr->sqhead)) { /* liberer les process en attente sur
                                     ce semaphore                       */
         while ((pid = _getfirst(SYSQ,sptr->sqhead)) != EMPTY )
               _setrdy(pid, NOSCHEDUL);
         sptr->semcnt = n;
    }
    else
         sptr->semcnt = n;

    Tasktab[RUNpid].terrno = 0;
    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * m_Countsem - retourne la valeur du compteur associe au semaphore "sem"
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Countsem(sem, cnt)
int *cnt;
{
    int ps,value;
    struct semslot *sptr;
    ps = _itDis();
    if (isbadsem(sem) || ( sptr = &Semtab[sem])->sstate == SFREE ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    *cnt = sptr->semcnt;
    Tasktab[RUNpid].terrno = 0;
    _itRes(ps);
    return(ROK);
}
