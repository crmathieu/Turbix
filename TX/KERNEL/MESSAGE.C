/* message.c */

#include "sys.h"

/* <<<<<<<<<<<<<<<<<<<<<<<<<<<< Messages LMX >>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*--------------------------------------------------------------------------
 * REM :  le systeme de message est impossible avec la tache initiale
 *        car le pid 0  est considere comme invalide !
 * 2 types de messages : - les messages simples qui ne servent qu'a la
 *                         synchronisation entre taches :
 *                            -> msgsync, msgrdv, _msgsndI, msgclr
 *
 *                       - les messages d'echanges d'informations
 *                         entre taches :
 *                            -> msgsnd, msgrcv, tmsgrcv, getmsg
 *
 *--------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------
 * m_Msgwait(msg, tempo, status) - attendre un message de synchro avec tempo
 *                       si tempo < 0 : attendre indefiniment
 *                       si tempo = 0 : ne pas attendre
 *                       si tempo > 0 : attendre tempo secondes au maxi
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgwait(msg, tempo, status)
uchar *msg;     /* adresse du byte */
int   tempo;
int *status;    /* si retour = RERR : MSG_TIMEOUT, MSG_NOMESS */
{
    return(_msgwait(RUNpid, msg, tempo, status));
}

/*----------------------------------------------------------------------------
 * _msgwait(pid, msg, tempo, status) - mettre en attente un process donn‚
 *                       si tempo < 0 : attendre indefiniment
 *                       si tempo = 0 : ne pas attendre
 *                       si tempo > 0 : attendre tempo secondes au maxi
 *----------------------------------------------------------------------------
 */
_msgwait(pid, msg, tempo, status)
int pid;
uchar *msg;     /* adresse du byte */
int   tempo;
int *status;    /* si retour = RERR : MSG_TIMEOUT, MSG_NOMESS */
{
   int ps;
   struct taskslot *tp;

   ps = _itDis();
   tp = &Tasktab[pid];
   if (tp->tmsgnr == 0) {  /* attendre le message */
        if (tempo < 0) {  /* attendre indefiniment */
                tp->tstate = SLEEP;
                tp->tevent = EV_MESS;
                _swpProc();
        }
        else {
                if (tempo == 0) { /* revenir imm‚diatement */
                        _itRes(ps);
                        *status = MSG_NOMESS;
                        return(RERR);
                }
                if (_timout(pid, tempo)) { /* TIMEOUT tomb‚ */
                        _itRes(ps);
                        *status = MSG_TIMEOUT;
                        return(RERR);
                }
        }
   }

   /*  il y a un message pour la tache
    *  le prendre et  garder la CPU
    */
   *msg = tp->tmsg[tp->tmsgOut];
   tp->tmsgOut = (tp->tmsgOut+1) % MAXMESS;
   tp->tmsgnr--;

   _itRes(ps);
   return(ROK);
}

/*----------------------------------------------------------------------------
 * m_Msgsync - allez en rendez vous avec une autre tache
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
 * m_Msgclr  - RAZ zone message
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgclr()
{
   int stat;
   uchar null;

   m_Msgwait(&null, 0, &stat);
   return(ROK);

/*    ps = _itDis();
    Tasktab[RUNpid].tmsgnr = 0;
    _itRes(ps);*/
}

/*----------------------------------------------------------------------------
 * _msgsndI - message envoye sous IT (msg obligatoirement de type BYTE)
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
    /* deposer message */
    tp->tmsg[tp->tmsgIn] = msg;
    tp->tmsgIn = (tp->tmsgIn+1) % MAXMESS;
    tp->tmsgnr++;

    /* activer le process cible */
    if (tp->tstate == SLEEP && (tp->tevent & EV_MESS))
            _setrdy(pid,(flag ? SCHEDUL : NOSCHEDUL));

    _itRes(ps);
    return(ROK);
}

