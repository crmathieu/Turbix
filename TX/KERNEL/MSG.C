/* msg.c */

#include "sys.h"
#include "ipc.h"
#include "message.h"

/*----------------------------------------------------------------------------
 * msgget - recuperer un QID en fonction de la cle fournie
 *----------------------------------------------------------------------------
 *//* INCOMPLET : ajouter setup des permissions */
SYSTEMCALL msgget(key,msgflg)
key_t key;      /* cle correspondant … cette queue */
int msgflg;     /* drapeaux de recuperation du QID */
{
    int ps, i, j, qid;
    struct msgqid_ds *qp;
    struct taskslot *tp;

    ps = disable();
    qp = &Qtable;
    tp = &Tasktab;
    for (i=0,j=-1; i<NQID; i++, qp++) {
        if (qp->msg_perm.key == key) {
                /* la queue existe deja */
                if (msgflg & IPC_ALLOC) {
                        qid = i + (qp->msg_perm.seq * NQID);
                        restore(ps);
                        return(qid);
                }
                if (msgflg & IPC_CREAT) {
                        if (msgflg & IPC_EXEC) {
                            tp->terrno = EEXIST;
                            restore(ps);
                            return(RERR);
                        }
                        qid = i + (qp->msg_perm.seq * NQID);
                        restore(ps);
                        return(qid);
                }
        }
        else
                if (qp->msg_perm.key == IPC_NOKEY)
                        j = i; /* memoriser le slot */
    }
    /* auncune cl‚ n'est egale a la cle fournie */
    if ((msgflg & IPC_CREAT) && (j >= 0)) {
        init_queue(j);
        qid = j + (Qtable[j].msg_perm.seq * NQID);
        restore(ps);
        return(qid);
    }
    if (j >= 0)  /* flag IPC_ALLOC */
        tp->terrno = ENOENT;
    else
        tp->terrno = ENOSPC;
    restore(ps);
    return(RERR);
}

/*----------------------------------------------------------------------------
 * msgrcv - attendre un message
 *----------------------------------------------------------------------------
 */
SYSTEMCALL msgrcv(qid, msgp, msgsz, msgtyp, msgflg)
int             qid;    /* queue identifier */
struct msgbuf   *msgp;  /* pointer to special structure with msg */
int             msgsz;  /* longueur du message */
long            msgtyp; /* type du message */
int             msgflg; /* actions a declencher sous certaines conditions */
{
    int ps, ct;
    struct taskslot *tp;

    ps = disable();
    if (isbadpid(pid) || ((tp = &Tasktab[pid])->tstate == UNUSED)
                      || (tp->tmsgnr == MAXMESS) ||
        (msgtyp > (MAXMESS - tp->tmsgnr))) {
         tp->terrno = EINVAL;
         restore(ps);
         return(RERR);
    }
    /* deposer message */
    while (msgtyp--) {
           tp->tmsg[tp->tmsgIn] = *(message++);
           tp->tmsgIn = (tp->tmsgIn+1) % MAXMESS;
           tp->tmsgnr++;
    }

    /* activer le process cible */
    if (tp->tstate == SLEEP) {
        if (tp->tevent & EV_MESS) _setrdy(pid,SCHEDUL);
        else
            if (tp->tevent & EV_TMESS)  /* si attente mess avec tempo */
            {
                _stopdelay(pid);
                tp->tflag &= ~F_TIMOUT;    /* indiquer tempo non tomb‚e */
                _setrdy(pid, SCHEDUL);
            }
    }
    restore(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * msgsnd - envoyer un message
 *----------------------------------------------------------------------------
 */
SYSTEMCALL msgsnd(qid, msgp, msgsz, msgflg)
int             qid;    /* queue identifier */
struct msgbuf   *msgp;  /* pointer to special structure with msg */
int             msgsz;  /* longueur du message */
int             msgflg; /* actions a declencher sous certaines conditions */
{
   int ps;
   struct taskslot *tp;
   int msg, ct;

   ps = disable();
   tp = &Tasktab[RUNpid];
   if (((ct = tp->tmsgnr) == 0) || (ct < msgtyp))  /* attendre le message */
   {
       tp->tstate = SLEEP;
       tp->tevent = EV_MESS;
       _schedul();
   }
   /* il y a un message pour la tache : on le prend et on garde la CPU */
   while (msgtyp--) {
          *message++ = tp->tmsg[tp->tmsgOut];
          tp->tmsgOut = (tp->tmsgOut+1) % MAXMESS;
          tp->tmsgnr--;
   }
   restore(ps);
   return(ROK);
}

/*----------------------------------------------------------------------------
 * msgwait() - attendre  message de synchronisation
 *----------------------------------------------------------------------------
 */
SYSTEMCALL msgwait()
{
   int ps;
   struct taskslot *tp;
   int msg;

   ps = disable();
   tp = &Tasktab[RUNpid];
   if (tp->tmsgnr == 0)   /* attendre le message */
   {
       tp->tstate = SLEEP;
       tp->tevent = EV_MESS;
       _schedul();
   }
   tp->tmsgnr--;
   restore(ps);
   return(ROK);

}

/*----------------------------------------------------------------------------
 * tmsgrcv(tempo) - attendre un message avec tempo
 *----------------------------------------------------------------------------
 */
SYSTEMCALL tmsgrcv(tempo, msgtyp, message)
unsigned tempo;
uchar *message;
{
   int ps;
   struct taskslot *tp;
   int msg,status;

   ps = disable();
   tp = &Tasktab[RUNpid];
   if (tempo < 0) {
       tp->terrno = EINVAL;
       restore(ps);
       return(RERR);
   }
   if (tp->tmsgnr < msgtyp)   /* attendre le message */
        if (_timout(RUNpid, tempo)) { /* TIMEOUT tomb‚ */
             restore(ps);
             return(TIMOVER);
        }

   /*  il y a un message pour la tache
    *  le prendre et  garder la CPU
    */
   while (msgtyp--) {
          *message++ = tp->tmsg[tp->tmsgOut];
          tp->tmsgOut = (tp->tmsgOut+1) % MAXMESS;
          tp->tmsgnr--;
   }

   restore(ps);
   return(msg);
}

/*----------------------------------------------------------------------------
 * getmsg  - retourne message si message present sinon OK
 *           elle permet de prendre un message au vol
 *----------------------------------------------------------------------------
 */
SYSTEMCALL getmsg(msgtyp, message)
uchar *message;
{
   int ps, msg, i;
   struct taskslot *tp;

   ps = disable();
   msg = ROK;

   /* existe - t - il un message ? */
   if ((tp = &Tasktab[RUNpid])->tmsgnr < msgtyp)
       while (msgtyp--) {
          *message++ = tp->tmsg[tp->tmsgOut];
          tp->tmsgOut = (tp->tmsgOut+1) % MAXMESS;
          tp->tmsgnr--;
       }
   else
       msg = RERR;
    restore(ps);
    return(msg);
}

/*----------------------------------------------------------------------------
 * msgclr  - RAZ zone message
 *----------------------------------------------------------------------------
 */
SYSTEMCALL msgclr()
{
    int ps;
    int msg;

    ps = disable();
    Tasktab[RUNpid].tmsgnr = 0;
    restore(ps);
    return(ROK);
}


/*----------------------------------------------------------------------------
 * sendI - message envoye sous IT (msg obligatoirement de type BYTE)
 *----------------------------------------------------------------------------
 */
msgsndI(pid,msg,flag)
uchar msg;
{
    extern int itvalidee;

    int ps, it_state, ct;
    struct taskslot *tp;

    it_state = itvalidee;
    ps = disable();
    if (isbadpid(pid) || ((tp = &Tasktab[pid])->tstate == UNUSED)
                      || (tp->tmsgnr == MAXMESS)) {
         tp->terrno = EINVAL;
         restore(ps);
         return(RERR);
    }
    /* deposer message */
    tp->tmsg[tp->tmsgIn] = msg;
    tp->tmsgIn = (tp->tmsgIn+1) % MAXMESS;
    tp->tmsgnr++;

    /* activer le process cible */
    if (tp->tstate == SLEEP)
        if (tp->tevent & EV_MESS)
            _setrdy(pid,(it_state|flag ? SCHEDUL : NOSCHEDUL));

    restore(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * msgsync - allez en rendez vous avec une autre tache
 *----------------------------------------------------------------------------
 */
SYSTEMCALL msgsync(pid, msgbyte)
int   pid;
uchar msgbyte;
{
   return(msgsndI(pid, msgbyte, TRUE));
}

