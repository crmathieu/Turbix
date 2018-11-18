/* msg.h */

#define  IPC_NOKEY   -1
#define  NQID        10         /* 10 queue de messages possible */
#define  NULLMSG     (struct msg *)0

/* definition des permissions */
#define MSG_R   0400    /* permission de lecture */
#define MSG_W   0200    /* permission d'ecriture */

#define MSG_NOERROR 010000 /* pas d'erreur pour des message trop long */

/* definition des ipc_perm Modes */
#define MSG_RWAIT 01000  /* un lecteur attend un message */
#define MSG_WWAIT 02000  /* un ecrivain est en attente d'envoi */

#define MSG_HEADER    (sizeof(struct msg)/16)+(sizeof(struct msg)%16?1:0)

struct msgbuf {
        long mtype;     /* type du message  */
        char mtext[1];  /* texte du message */
};

/* definition d'un header de message */
struct msg {
        struct msg *msg_next;   /* prochain message de cette queue */
        long        msg_type;   /* type de message */
        short       msg_ts;     /* taille du texte du message */
        short       msg_spot;   /* adresse du message dans la table
                                 * d'allocation
                                 */
};

/* definition d'un slot d'une IPC message */
struct msgqid_ds {
        struct ipc_perm msg_perm;       /* permission sur la queue */
        struct msg     *msg_first;      /* tete de file des msg */
        struct msg     *msg_last;       /* queue de file */
        ushort          msg_cbytes;     /* # octets de la queue */
        ushort          msg_qnum;       /* # msg dans la queue */
        ushort          msg_qbytes;     /* # max d'octets de la queue */
        ushort          msg_lspid;      /* pid du dernier msgsnd */
        ushort          msg_lrpid;      /* pid du dernier msgrcv */
        time_t          msg_stime;      /* heure du dernier msgsnd */
        time_t          msg_rtime;      /* heure du dernier msgrcv */
        time_t          msg_ctime;      /* heure du dernier msgctl */
        int             msg_nmx;        /* variable de travail MKD :
                                         * EST en fait le PID de la 1ere
                                         * tƒche en attente sur RECEP
                                         */
};

/* Message information structure */
struct msginfo {
        int     msgmap, /* # entrees dans msg map */
                msgmax, /* taille du plus grand message */
                msgmnb, /* # max de bytes dans la queue */
                msgmni, /* # d'identificateurs de queue */
                msgssz, /* msg segment size */
                msgtql; /* # de header de message systeme */
        ushort  msgseg; /* # de segments de messages ( < 32768) */
};

extern struct msgqid_ds Qtable[];