/* clock.c */

#include "sys.h"
#include "q.h"
#include "signal.h"
#include "sem.h"

struct WORDREGS {
        unsigned int    ax, bx, cx, dx, si, di, cflag, flags;
};

struct BYTEREGS {
        unsigned char   al, ah, bl, bh, cl, ch, dl, dh;
};

union   REGS    {
        struct  WORDREGS x;
        struct  BYTEREGS h;
};

struct  SREGS   {
        unsigned int    es;
        unsigned int    cs;
        unsigned int    ss;
        unsigned int    ds;
};

struct  REGPACK {
        unsigned        r_ax, r_bx, r_cx, r_dx;
        unsigned        r_bp, r_si, r_di, r_ds, r_es, r_flags;
};

int    DelayList;       /* index de la file des process en SLEEP          */
                        /* dans la table systeme Clkq                     */
int   *firstDelay;      /* adresse de la 1ere cle dans DelayList          */
int    isDelay;         /* booleen : 1 si DelayList n'est pas vide        */
SEM    bsem;            /* semaphore de mise en file SERVICE              */
int   *clkstate;        /* pointe sur la variable etat de la tache ClockT */

int    dayByMonth[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

LOCAL  struct biosreq *tbreq = BIOSNULL;
LOCAL  struct biosreq *hbreq = BIOSNULL; /* tail & head */
extern int SigMask[];

LOCAL  int defclk;      /* > 0 si les it clock sont differees             */
LOCAL  int ticks;       /* nombre de clicks d'horloge qui sont diff       */

/*----------------------------------------------------------------------------
 * _scheduler - process le plus prioritaire/reveille par la tache horloge
 *----------------------------------------------------------------------------
 */
_scheduler()
{
      while (1) {
        if (isDelay)
          if (--*firstDelay <= 0)
              if (*clkstate == SLEEP) {
                  *clkstate = READY;
                  _insertKFR(CALLOUT, rdytail, 50);
              }
        Tasktab[RUNpid].tstate = SLEEP;
        _swpProc();
     }
}


/*----------------------------------------------------------------------------
 * clock - delay and CPU sharing management
 *----------------------------------------------------------------------------
 */
_clock()
{
        _setrdy(SCHEDULER, SCHEDUL);
}

/*----------------------------------------------------------------------------
 * sdelay - sets a task's state as SLEEP during  n * 1/(18/QUANTUM) sec
 *----------------------------------------------------------------------------
 */
BIBLIO m_Gsleep(n)
int n;
{
    int ps;
    struct taskslot *tp;

    ps = _itDis();
    (tp = &Tasktab[RUNpid])->terrno = 0;
    if (n < 0 ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    if (n == 0) {
        _swpProc();
        _itRes(ps);
        return(ROK);
    }

    if ((tp = &Tasktab[RUNpid])->tflag & F_DELAY) {
         _stopdelay(RUNpid);
    }

    _delayIn( RUNpid , DelayList , n );
    isDelay     = TRUE;
    firstDelay  = (int *)&(Clkq[Clkq[DelayList].next].key);
    tp->tstate  = SLEEP;
    tp->tevent  = EV_CLOCK;
    _swpProc();
    _itRes(ps);
    return(ROK);

}

/*----------------------------------------------------------------------------
 * m_Sleep - set the calling task's state to SLEEP during n seconds
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Sleep(n)
unsigned n;
{
   return(m_Gsleep(n * (18 / QUANTUM)));
}

/*----------------------------------------------------------------------------------
 * m_Alarm - arms a timeout - when timeout expires, a SIGALRM signal is generated
 *----------------------------------------------------------------------------------
 */
SYSTEMCALL m_Alarm(n)
unsigned n;
{
    int ps;
    struct taskslot *tp;

    ps = _itDis();
    Tasktab[RUNpid].terrno = 0;
    if (n < 0 ) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    if (n == 0) {
         /* ignore event */
         Tasktab[RUNpid].tevsig &= (~SigMask[SIGALRM]);

         /* stop delay */
         _stopdelay(RUNpid);
         _swpProc();
         _itRes(ps);
         return(ROK);
    }

    /*  if task is already in delay list, remove previous unterminated delay */
    if ((tp = &Tasktab[RUNpid])->tflag & F_DELAY) {
         _stopdelay(RUNpid);
    }

    _delayIn( RUNpid , DelayList , (18 / QUANTUM) * n);
    tp->tflag |= F_ALRM;
    isDelay    = TRUE;
    firstDelay = (int *)&(Clkq[Clkq[DelayList].next].key);
    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * _timeout - starts a delay and return indicating if delay has expired
 *----------------------------------------------------------------------------
 */
_timeout(pid, n)
unsigned n;
{
    int ps,status;
    struct taskslot *tp;

    if (n == 0) {
        return(TRUE);    /* immediat timeout */
    }

    ps = _itDis();
    tp = &Tasktab[pid];

    _delayIn( pid , DelayList , (18 / QUANTUM) * n );
    isDelay = TRUE;
    firstDelay = (int *)&(Clkq[Clkq[DelayList].next].key);

    /* indicate state as SLEEP with waiting on message with timeout */
    tp->tstate  = SLEEP;
    tp->tevent  = EV_TMESS;
    _swpProc();
    /* coming back only when delay has expired -or- message has arrived */
    status = (tp->tflag & F_TIMOUT ? TRUE : FALSE);
    tp->tflag  &= ~F_TIMOUT;
    _itRes(ps);
    return(status);   /* returns true if delay has expired */
}


/*----------------------------------------------------------------------------
 * _delayOut - reveille la 1ere tache de la DelayList
 *----------------------------------------------------------------------------
 */
_delayOut()
{
    int ps,pid;
    struct taskslot *tp;


    /* prevenir la tache que la tempo est tombee */
    while (1) {
      if (nonemptyD && firstkeyD <= 0)
      {
         tp = &Tasktab[pid = _getfirst(CLKQ,DelayList)];
         tp->tflag &= ~F_DELAY;

         switch(tp->tstate) {
         case READY :
              tp->tflag &= ~(F_ALRM|F_DELAY);
              _sendsig(pid,SIGALRM);
              break;

         case SLEEP :
              tp->tflag &= ~F_DELAY;
              switch(tp->tevent) {
              case EV_CLOCK :    /* attente fin delais */
                   _setrdy(pid,NOSCHEDUL);
                   break;

              case EV_TMESS :    /* attente message avec tempo */
                   tp->tflag |= F_TIMOUT;  /* tempo tombee */
                   _setrdy(pid,NOSCHEDUL);
                   break;

              case EV_PAUSE:
                   tp->tflag &= ~F_ALRM;
                   _sendsig(pid,SIGALRM);
                   break;

              case EV_MESS :
              case EV_SEM  :
              case EV_ZOM  :
                   tp->tflag &= ~F_ALRM;
                   break;

              default      :
                   break;
              }
              break;
         default    :
              break;
         }
      }
      else break;
    }

    /* placer la prochaine tache a reveiller en tete de DelayList */
    if ((isDelay = nonemptyD) != FALSE)
        firstDelay = (int *) &Clkq[Clkq[DelayList].next].key;

}


/*----------------------------------------------------------------------------
 * _delayIn - insert une tache dans la file des delais
 *----------------------------------------------------------------------------
 */
_delayIn (pid, head, key)
int pid;
int head;
int key;
{
       int next;       /* parcours la liste */
       int prev;       /* suit next dans la liste */

       for (prev = head, next = Clkq[head].next;
            Clkq[next].key < key; prev = next, next = Clkq[next].next)
            key -= Clkq[next].key;

       Tasktab[pid].tflag |= F_DELAY;
       Clkq[pid].next   = next;
       Clkq[pid].prev   = prev;
       Clkq[pid].key    = key;
       Clkq[prev].next  = pid;
       Clkq[next].prev  = pid;
       if (next < NTASK)
           Clkq[next].key -= key;

       return(ROK);
}

/*-----------------------------------------------------------------------------
 * _stopdelay - retirer la tache de la file des delais
 *-----------------------------------------------------------------------------
 */
_stopdelay(pid)
int pid;
{
    int ps,next,key;
    struct taskslot *tp;
    ps = _itDis();

    if (!((tp = &Tasktab[pid])->tflag & F_DELAY)) {
/*        kprintf("\nUNSLEEP BAD DELAY pour %s\n",Tasktab[pid].tname);*/
        _itRes(ps);
        return(ROK);
    }
    tp->tflag &= ~F_DELAY;
    key  = Clkq[pid].key;
    next = Clkq[pid].next;
    _defect(CLKQ,pid);

    /*  si file non vide , rajouter son delta t
     *  sur la prochaine tache et repositionner le
     *  pointeur de delais
     */
    if (next < NTASK) {
/*            kprintf("\nUNSLEEP : file non vide : reste %s\n",Tasktab[next].tname);*/
            Clkq[next].key += key;
            firstDelay = (int *) &Clkq[Clkq[DelayList].next].key;
            isDelay = TRUE;
    }
    else    isDelay = FALSE;
    _itRes(ps);
    return(ROK);
}


/*----------------------------------------------------------------------------
 * m_Wakeup - reveiller une tache endormie
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Wakeup(targetid)
int targetid;
{
    int ps;
    struct taskslot *tp;


    Tasktab[RUNpid].terrno = 0;
    ps = _itDis();
    if (isbadpid(targetid) || ((tp = &Tasktab[targetid])->tstate != SLEEP)
        || (!(tp->tevent & (EV_TMESS|EV_PAUSE|EV_CLOCK)))) {
/*        kprintf("\nWAKEUP : BAD PARAMETERS\n");*/
        tp->terrno = EINVAL;
        _itRes(ps);
        return(RERR);
    }
    _stopdelay(targetid);

    /* generer SIGALRM pour annoncer fin watch dog */
    if (tp->tevent & EV_PAUSE) {
        tp->tflag &= ~F_ALRM;
        _sendsig(targetid,SIGALRM);
    }

    _setrdy(targetid,NOSCHEDUL);
    _itRes(ps);
    return(ROK);
}


/*----------------------------------------------------------------------------
 * clkInit - initialisation de l'horloge
 *----------------------------------------------------------------------------
 */
_clkInit()
{
    extern long msdosv_clk;
    extern int _itclock();
    int chead,ctail;

    ctail = 1 + (chead = NTASK);

    isDelay          = FALSE;
    DelayList        = chead;
    defclk           = 0;
    ticks            = 0;
    clkstate         = (int *)&Tasktab[CALLOUT].tstate;
    Clkq[chead].next = ctail;
    Clkq[chead].prev = EMPTY;
    Clkq[ctail].prev = chead;
    Clkq[ctail].next = EMPTY;
    Clkq[chead].key  = MININT;
    Clkq[ctail].key  = MAXINT;
    _sasVector( XENCLOCK, _itclock , &msdosv_clk );

}


/*----------------------------------------------------------------------------
 * m_Time - retourne le nb de secondes ecoulees depuis le 1/1/1970 00:00:00
 *----------------------------------------------------------------------------
 */
BIBLIO long m_Time(tloc)
long *tloc;
{
    int         ps;
    int         i,j,deltaA,date,time,bissx,stat;
    long        day , month , hour , year,loc;
    struct      biosreq hbuff;
    uchar       null;

    ps = _itDis();
    hbuff.b_pid = m_Getpid();
    hbuff.b_func = CLK_SRV;

    _biosenq(&hbuff);
    m_Msgclr();
    m_Sigsem(bsem);
    m_Msgwait(&null, -1, &stat);

  /*  calcul du nb de secondes ecoulees
   *  depuis le 1er Janvier 1970 - le temps calcule differe
   *  - de 8 heures de moins / celui donne par time MICROSOFT
   *  - de 5 heures de moins / celui donne par time BORLAND
   */

  i     = hbuff.h_date[1];
  month = 0;
  for (j = 0;j < i;j++)
        month += dayByMonth[j];

  deltaA = hbuff.h_date[0]-1980;
  bissx = 2 + (deltaA/4);
  if (i > 2)
     bissx += (((hbuff.h_date[0]) % 4) == 0 ? 1 : 0);
  year  = ((deltaA + 10) * 365) + bissx;

  day   = (month + year + (long)(hbuff.h_date[2] - 1)) * 86400;
  hour  = hbuff.h_time[0];
  hour *= 3600;
  loc = (long)(hbuff.h_time[2] + (hbuff.h_time[1]*60)) + hour + day ;
  _itRes(ps);
  if (tloc != (long *)NULL)
     *tloc = loc;
  return(loc);
}

/*----------------------------------------------------------------------------
 * m_DateTime - retourne l'heure et la date courante sous forme d'un tableau
 *            de 6 entiers INT   MM/JJ/AA hh:mm:ss
 *----------------------------------------------------------------------------
 */
BIBLIO m_DateTime(td)
int *td;
{
    struct biosreq hbuff;
    int ps,i,stat;
    uchar null;

    ps = _itDis();
    hbuff.b_pid  = RUNpid;
    hbuff.b_func = CLK_SRV;
    _biosenq(&hbuff);
    m_Msgclr();
    m_Sigsem(bsem);
    m_Msgwait(&null, -1, &stat);
    for (i = 0; i < 3; i++,td++) {
       *td     = hbuff.h_date[i]; /*[2-i];*/
       *(td+3) = hbuff.h_time[i];
    }
    _itRes(ps);
}



/*----------------------------------------------------------------------------
 * service - gere les fonctions  BIOS et souris
 *----------------------------------------------------------------------------
 */
TASK _service()
{
      int pid;
      bsem = m_Creatsem(0,"Service");
      while(1)
      {
          _waitsem(bsem,STAIL);
          while (hbreq != BIOSNULL) {
                 switch(hbreq->b_func) {
                 case CLK_SRV:  /* Service Horloge et Date */
                                _getsystime(hbreq->h_time);
                                _getsysdate(hbreq->h_date);
                                break;

                 case MOU_SRV:  /* Service souris : reveiller la
                                 * tache souris
                                 */
                                m_Msgsync(MOU_TASK, hbreq->m_nofunc);
                                break;
                 }
                 pid = hbreq->b_pid;
                 hbreq = hbreq->b_next;
                 m_Msgsync(pid);
          }
      }
}

/*----------------------------------------------------------------------------
 * biosenq - place une requete Bios ou souris en fin de file
 *----------------------------------------------------------------------------
 */
_biosenq(biosreq)
struct biosreq *biosreq;
{
   struct biosreq *p , *q;

   if (hbreq == BIOSNULL)  {
       hbreq = biosreq;
       biosreq->b_next = BIOSNULL;
       tbreq = biosreq;
       return;
   }

   q = hbreq;
   while ((p = q->b_next) != BIOSNULL)
           q = p;

   q->b_next = biosreq;
   biosreq->b_next = BIOSNULL;
}


/*----------------------------------------------------------------------------
 * clockT - tache horloge (+ haute priorite dans le systeme)
 *----------------------------------------------------------------------------
 */
TASK _clockT()
{
   int next,lost_ticks;

   _itDis();
   while (TRUE) {
          Tasktab[RUNpid].tstate = SLEEP;
          _swpProc();
          lost_ticks = ticks;
          ticks = 0;

          /*  clockT est reveillee uniquement par l'IT clock
           *  pour realiser la fonction WAKEUP
           */
          if (lost_ticks > 0) {
              for (next = firstidD;
                   next < NTASK && Clkq[next].key < lost_ticks ;
                   next = Clkq[next].next) {

                   lost_ticks -= Clkq[next].key;
                   Clkq[next].key = 0;
              }
              if (next < NTASK)  {
                  if (Clkq[next].key >= lost_ticks)
                      Clkq[next].key -= lost_ticks;
                  else
                      Clkq[next].key  = 0;
              }
          }
          if (--*firstDelay <= 0)   _delayOut();
   }
}

/*----------------------------------------------------------------------------
 * clock-tick
 *----------------------------------------------------------------------------
 */
clock_Tick()
{
   int next,lost_ticks;

          lost_ticks = ticks;
          ticks = 0;

          if (lost_ticks > 0) {
              for (next = firstidD;
                   next < NTASK && Clkq[next].key < lost_ticks ;
                   next = Clkq[next].next) {

                   lost_ticks -= Clkq[next].key;
                   Clkq[next].key = 0;
              }
              if (next < NTASK)  {
                  if (Clkq[next].key >= lost_ticks)
                      Clkq[next].key -= lost_ticks;
                  else
                      Clkq[next].key  = 0;
              }
          }
          if (--*firstDelay <= 0)   _delayOut();
}




