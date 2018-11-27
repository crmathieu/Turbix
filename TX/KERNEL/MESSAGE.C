/* message.c */

#include "sys.h"

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<< Messages LMX >>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*--------------------------------------------------------------------------
 * Note : Messaging won't work with the initial task because pid 0 is
 *        considered as invalid!
 * 
 * 2 types of messages : - simple messages: they are used as mean of 
 *                         synchronization between task:
 *                            -> msgsync, msgrdv, _msgsndI, msgclr
 *
 *                       - information messages: they are used to exchange data
 *                         between tasks:
 *                            -> msgsnd, msgrcv, tmsgrcv, getmsg
 *
 *--------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
 * m_Msgwait(msg, timeout, status) - wait for message with timeout 
 *                       si timeout < 0 : wait indefinitely
 *                       si timeout = 0 : no wait
 *                       si timeout > 0 : wait a maximum of timeout seconds
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgwait(msg, timeout, status)
uchar *msg;     /* byte address */
int   tempo;
int *status;    /* contains error type when return = RERR. can be MSG_TIMEOUT or MSG_NOMESS */
{
    return(_msgwait(RUNpid, msg, timeout, status));
}

/*----------------------------------------------------------------------------
 * _msgwait(pid, msg, tempo, status) -  wait for message with timeout 
 *                       si timeout < 0 : wait indefinitely
 *                       si timeout = 0 : no wait
 *                       si timeout > 0 : wait a maximum of timeout seconds
 *----------------------------------------------------------------------------
 */
_msgwait(pid, msg, tempo, status)
int pid;
uchar *msg;     /* byte address */
int   tempo;
int *status;    /* si retour = RERR : MSG_TIMEOUT, MSG_NOMESS */
{
   int ps;
   struct taskslot *tp;

   ps = _itDis();
   tp = &Tasktab[pid];
   if (tp->tmsgnr == 0) { /* wait for message */
        if (tempo < 0) {  /* wait indefinitely */
                tp->tstate = SLEEP;
                tp->tevent = EV_MESS;
                _swpProc();
        }
        else {
                if (tempo == 0) { /* no delay */
                        _itRes(ps);
                        *status = MSG_NOMESS;
                        return(RERR);
                }
                if (_timeout(pid, tempo)) { /* TIMEOUT expired */
                        _itRes(ps);
                        *status = MSG_TIMEOUT;
                        return(RERR);
                }
        }
   }

   /*  there is a message for the task:
    *  take it and keep CPU
    */
   *msg = tp->tmsg[tp->tmsgOut];
   tp->tmsgOut = (tp->tmsgOut+1) % MAXMESS;
   tp->tmsgnr--;

   _itRes(ps);
   return(ROK);
}

/*----------------------------------------------------------------------------
 * m_Msgsync - synchronize with another task
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgsync(pid, msgbyte)
int   pid;
uchar msgbyte;
{
   return(_msgsndI(pid, msgbyte, TRUE));
}

/*----------------------------------------------------------------------------
 * m_Msgrdv -
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgrdv()
{
   int stat;
   uchar null;

   return(m_Msgwait(&null,-1, &stat));
}

/*----------------------------------------------------------------------------
 * m_Msgclr  - Clear message zone
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgclr()
{
   int stat;
   uchar null;

   m_Msgwait(&null, 0, &stat);
   return(ROK);
}

/*----------------------------------------------------------------------------
 * _msgsndI - BYTE message sent with IT disabled
 *----------------------------------------------------------------------------
 */
_msgsndI(pid,msg,flag)
uchar msg;
{

    int ps, it_state, ct;
    struct taskslot *tp;

    ps = _itDis();
    if (isbadpid(pid) || ((tp = &Tasktab[pid])->tstate == UNUSED)
                      || (tp->tmsgnr == MAXMESS)) {
         tp->terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    /* deposit message */
    tp->tmsg[tp->tmsgIn] = msg;
    tp->tmsgIn = (tp->tmsgIn+1) % MAXMESS;
    tp->tmsgnr++;

    /* activate target task */
    if (tp->tstate == SLEEP && (tp->tevent & EV_MESS)) {
            _setrdy(pid,(flag ? SCHEDUL : NOSCHEDUL));
    }

    _itRes(ps);
    return(ROK);
}

