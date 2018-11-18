/* ipcmsg.c */

#include "sys.h"
#include "ipc.h"
#include "msg.h"

#define  isbadQid(qid)  ((qid < 0) || (qid >= NQID))

/*
        FORMAT DES MESSAGES

        -------------------
        |  Header BLK     |  x PARA
        |                 |
        -------------------
        |  Header MSG     |  1 PARA
        |             ptr | -----
        -------------------     |
        | Type Message    | <----
        |       &         |  n PARA
        |      MSG        |
        -------------------
*/


/* <<<<<<<<<<<<<<<<<<<<<<<<<<<< Messages IPC >>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*----------------------------------------------------------------------------
 * _init_msg - initialise la table des queue de messages
 *----------------------------------------------------------------------------
 */
_init_msg()
{
        int i;
        for (i=0; i<NQID; i++) {
                /* Niveau IPC */
                Qtable[i].msg_perm.key  = IPC_NOKEY;
                Qtable[i].msg_perm.uid  = BADPID;
                Qtable[i].msg_perm.cuid = BADPID;
                Qtable[i].msg_perm.mode = 0;
                Qtable[i].msg_perm.seq  = 0;

                /* Niveau Message */
                Qtable[i].msg_first     = NULLMSG;
                Qtable[i].msg_last      = NULLMSG;
                Qtable[i].msg_cbytes    = 0;
                Qtable[i].msg_qnum      = 0;
                Qtable[i].msg_qbytes    = 4096; /* # max d'octets sur 1 Q */
                Qtable[i].msg_lspid     = BADPID;
                Qtable[i].msg_lrpid     = BADPID;
                /* la gestion de l'heure n'est pas assuree
                 * pour le moment
                 */

                /* Niveau NMX : Pas de tƒche en Attente en REC */
                Qtable[i].msg_nmx       = BADPID;
        }
}


/*----------------------------------------------------------------------------
 * init_queue - initialise une queue
 *----------------------------------------------------------------------------
 */
init_queue(qid, key)
key_t key;
{
    Qtable[qid].msg_perm.key = key;  /* pour le moment */
}

/*----------------------------------------------------------------------------
 * m_Msgget - recuperer un QID en fonction de la cle fournie
 *----------------------------------------------------------------------------
 *//* INCOMPLET : ajouter setup des permissions */
SYSTEMCALL m_Msgget(key,msgflg)
key_t key;      /* cle correspondant … cette queue */
int msgflg;     /* drapeaux de recuperation du QID */
{
    int ps, i, j, qid;
    struct msgqid_ds *qp;
    struct taskslot *tp;

    ps = _itDis();
    qp = Qtable;
    tp = Tasktab;


    for (i=0,j=-1; i<NQID; i++, qp++) {
        if (qp->msg_perm.key == key) {
                /* la queue existe deja */
                if (key == IPC_PRIVATE)
                        continue;
                if (msgflg & IPC_ALLOC) {
                        qid = i + (qp->msg_perm.seq * NQID);
                        _itRes(ps);
                        return(qid);
                }
                if (msgflg & IPC_CREAT) {
                        if (msgflg & IPC_EXCL) {
                            /*tp->terrno = EEXIST;*/
                            _itRes(ps);
                            return(RERR);
                        }
                        qid = i + (qp->msg_perm.seq * NQID);
                        _itRes(ps);
                        return(qid);
                }
        }
        else
                if (qp->msg_perm.key == IPC_NOKEY)
                        if (key == IPC_PRIVATE) {
                                init_queue(i);
                                qid = i + (Qtable[i].msg_perm.seq * NQID);
                                _itRes(ps);
                                return(qid);
                        }
                        else
                                j = i; /* memoriser le slot */
    }
    /* auncune cl‚ n'est egale a la cle fournie */
    if ((msgflg & IPC_CREAT) && (j >= 0)) {
        init_queue(j);
        qid = j + (Qtable[j].msg_perm.seq * NQID);
        _itRes(ps);
        return(qid);
    }
    if (j >= 0)  /* flag IPC_ALLOC */
        tp->terrno = ENOENT;
    else
        tp->terrno = ENOSPC;
    _itRes(ps);
    return(RERR);
}

/*----------------------------------------------------------------------------
 * m_Msgctl - gestion d'une Queue de message
 *                      cmd est :
 *                              IPC_STAT  pour consulter un descript de queue
 *                              IPC_SET   pour modifier un descript de queue
 *                              IPC_RMID  suppression d'une queue
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgctl(qid, cmd, buf)
int qid, cmd;
struct msgqid_ds *buf;
{
    int ps;
    struct msgqid_ds *qp;

    ps = _itDis();

    /* Check validity */
    if (isbadQid(qid) || (((qp = &Qtable[qid])->msg_perm.key) == IPC_NOKEY)) {
        _itRes(ps);
        return(RERR);
    }
    switch(cmd) {
    case  IPC_STAT : /* recopier la structure */
                     *buf = Qtable[qid];
                     break;

    case  IPC_SET  : /* modifier la structure */
                     qp->msg_perm.uid  = buf->msg_perm.uid;
                     qp->msg_perm.gid  = buf->msg_perm.gid;
                     qp->msg_perm.mode = buf->msg_perm.mode;
                     qp->msg_qbytes    = buf->msg_qbytes;
                     break;

    case  IPC_RMID : /* supprimer la queue */
                     qp->msg_perm.key = IPC_NOKEY;

                     /* relancer las taches bloquees sur cette queue */
                     _resumeRcvTask(qp);
                     break;

    default        : _itRes(ps);
                     return(RERR);
    }
    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------------
 * m_Msgrcv - attendre un message.
 *              FLAG :
 *                      - MSG_NOERROR  permet de tronquer sans d‚clencher
 *                                     d'erreur les messages trop longs
 *                      - IPC_NOWAIT   appel non bloquant s'il n'y a pas
 *                                     de message du type voulu
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgrcv(qid, msgp, msgsz, msgtyp, msgflg)
int             qid;    /* queue identifier */
struct msgbuf   *msgp;  /* pointer to special structure with msg */
int             msgsz;  /* longueur du message */
long            msgtyp; /* type du message */
int             msgflg; /* actions a declencher sous certaines conditions */
{
    struct msg *_getMsgFromQ();
    int ps, ct;
    struct taskslot  *tp;
    struct msgqid_ds *qp;
    struct msg       *MSG;
    char             *pMsgBuf;

    ps = _itDis();

    /* Check validity */
    if (isbadQid(qid) || (((qp = &Qtable[qid])->msg_perm.key) == IPC_NOKEY) ||
        (msgsz <= 0)) {
        _itRes(ps);
        return(RERR);
    }

    for (;;) {
        if (qp->msg_perm.key == IPC_NOKEY) { /* queue supprimee par MSGCTL */
            _itRes(ps);
            return(RERR);
        }
        if ((MSG = _getMsgFromQ(qp, msgtyp)) == (struct msg *)RERR) {
                /* On a pas trouv‚ de message pour le type donn‚ */
                if (msgflg & IPC_NOWAIT) {
                        /*Tasktab[RUNpid].terrno = ENOMSG;*/
                        _itRes(ps);
                        return(RERR);
                }
                else {
                        /* se suspendre apr‚s s'ˆtre Chain‚
                         * … la Queue
                         */
                        tp = &Tasktab[RUNpid];
                        tp->tnextT = qp->msg_nmx;
/*                      m_Printf("RCV: Mise en sommeil pid = %d\n", m_Getpid());*/
                        qp->msg_nmx = RUNpid;

                        tp->tstate = SLEEP;
                        tp->tevent = EV_MESS;
                        _schedul();
                }
        }
        else {
                /* Header de Message trouv‚ : v‚rifier les tailles */
                if (MSG->msg_ts > msgsz) {
                    /* buffer reception trop petit pour contenir le message */
                    if (msgflg & MSG_NOERROR)
                        /* tronquer !! */
                        MSG->msg_ts = msgsz;
                    else {
                        Tasktab[RUNpid].terrno = E2BIG;
                        _itRes(ps);
                        return(RERR);
                    }
                }
                /* copier le message en zone USER */
                FP_SEG(pMsgBuf) = MSG->msg_spot;
                FP_OFF(pMsgBuf) = 0;
                memcpy((char *)msgp, pMsgBuf, MSG->msg_ts + sizeof(long));
                *(char *)(msgp + MSG->msg_ts + sizeof(long) + 1) = '\0';

                /* liberer physiquement le message */
                m_Free(MSG);
                _itRes(ps);
                return(0);
        }
   }
}

/*--------------------------------------
 * _exitGetmsg - sous fonction de getmsg
 *--------------------------------------
 */
struct msg *_exitGetmsg(qp, F, W)
struct msgqid_ds *qp;
struct msg *F, *W;
{
                if (F == qp->msg_first)
                    qp->msg_first = F->msg_next;
                else
                    W->msg_next   = F->msg_next;

                qp->msg_qnum--;
                return(F);
}

/*----------------------------------------------------------------------------
 * _getMsgFromQ - Selectionner un message conforme au type dans la queue
 *              si msgtyp > 0   retourner le 1er message correspondant … ce
 *                              type
 *              si msgtyp < 0   retourner le 1er message de type inferieur
 *                              … la valeur absolue de msgtyp
 *              si msgtyp == 0  retourner le 1er message de la file
 *----------------------------------------------------------------------------
 */
LOCAL struct msg *_getMsgFromQ(qp, msgtyp)
struct msgqid_ds *qp;
long            msgtyp;
{

        struct msg *F, *W, *_exitGetmsg();
        long   valAbs;
        ushort nMess = qp->msg_qnum;

        if (nMess == 0)
                return((struct msg *)RERR);

        if (msgtyp == 0) {
                /* recuperer le 1er message de la queue */
                F = qp->msg_first;
                m_Printf("RCV: message recupere\n");
                return(_exitGetmsg(qp, F, (struct msg *)NULL));
        }
        valAbs = -msgtyp;
        for (F = W = qp->msg_first; nMess > 0; W = F, F = F->msg_next, nMess--) {

                if (msgtyp > 0) {
                    /* recuperer le 1er message correspondant … ce type */
                    if (F->msg_type == msgtyp)
                        return(_exitGetmsg(qp, F, W));
                }
                else {
                    /* le type de message est < 0
                     * recuperer le 1er message dont le type < valAbs
                     */

                    if (F->msg_type < valAbs)
                        return(_exitGetmsg(qp, F, W));
                }
        }

        /* il n'y a pas de message de ce type disponible */
        m_Printf("RCV: pas de message- type = %ld\n", msgtyp);
        return((struct msg *)RERR);
}

/*----------------------------------------------------------------------------
 * m_Msgsnd - envoyer un message
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Msgsnd(qid, msgp, msgsz, msgflg)
int             qid;    /* queue identifier */
struct msgbuf   *msgp;  /* pointer to special structure with msg */
int             msgsz;  /* longueur du message */
int             msgflg; /* actions a declencher sous certaines conditions */
{
    int ps, ct, pid;
    struct taskslot  *tp;
    struct msgqid_ds *qp;
    struct msg       *MSG;
    char             *pMsgBuf;
    extern int *_XSmalloc();

    ps = _itDis();

    /* Check validity */
    if (isbadQid(qid) || ((qp = &Qtable[qid])->msg_perm.key) == IPC_NOKEY ||
        msgsz <= 0) {
        Tasktab[RUNpid].terrno = EINVAL;
        _itRes(ps);
        return(RERR);
    }

    /* reserver un bloc de type SPECIAL
     * integrant le header de msg et le message
     */
    for (;;)
        if ((MSG = (struct msg *)_XSmalloc((MSG_HEADER * 16) + msgsz + sizeof(long))) == (struct msg *)RERR) {
                if (msgflg & IPC_NOWAIT) {
                        Tasktab[RUNpid].terrno = EAGAIN;
                        _itRes(ps);
                        return(RERR);
                }
                else {
                        m_Sleep(1); /* attente liberation de ressource */
                        continue;
                }
        }
        else
                break;

    /* l'@ du message se situe 1 paragraphe au dessus du Header msg */
    MSG->msg_spot = FP_SEG(MSG) + 1;

    /* terminer l'initialisation du header */
    MSG->msg_ts   = msgsz;
    MSG->msg_type = msgp->mtype;
    MSG->msg_next = NULLMSG;

    /* copier le message */
    FP_SEG(pMsgBuf) = MSG->msg_spot;
    FP_OFF(pMsgBuf) = 0;
    memcpy(pMsgBuf, (char *)msgp, msgsz + sizeof(long));

    /* mettre a jour les parametres de gestion de la queue */
    qp->msg_qnum++;

    /* chainer ce message … la queue */
    if (qp->msg_first == NULLMSG) {
        /* pas de message ds cette queue */
        qp->msg_first = qp->msg_last = MSG;
    }
    else {
        qp->msg_last->msg_next = MSG;
        qp->msg_last           = MSG;
    }

    /* Relancer d'‚ventuelles tƒches en attentes de messages */
    _resumeRcvTask(qp);
    _itRes(ps);
    return(ROK);
}

_resumeRcvTask(qp)
struct msgqid_ds *qp;
{
    int pid;

    if ((pid = qp->msg_nmx) > 0) /* il y a des tƒches bloqu‚es */
        while (pid > 0) {
                _setrdy(pid,NOSCHEDUL);
                pid = Tasktab[pid].tnextT;
                Tasktab[pid].tnextT = BADPID;

/*              m_Printf("NEXTPID = %d\n",pid);*/
        }
    qp->msg_nmx = BADPID;
}
