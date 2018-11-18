/* proc.h */

/*
 * definitions des etats d'un process
 *
 */
#define RUNNING    1  /* process possedant la CPU                           */
#define UNUSED     2  /* process non cr‚e                                   */
#define READY      3  /* process en competition pour la CPU                 */
#define SLEEP      4  /* process en attente d'evenement                     */


/* TYPES d'‚venements si tache en SLEEP */
#define  EV_SEM     1  /* bloquee sur semaphore           */
#define  EV_ZOM     2  /* attente prise en compte du pere */
#define  EV_MESS    4  /* attente reception message       */
#define  EV_TMESS   8  /* attente message avec tempo      */
#define  EV_PAUSE  16  /* attente signal                  */
#define  EV_CLOCK  32  /* attente fin de delais           */
#define  EV_SUSP   64  /* attente d'introduction          */
#define  EV_PIPE  128  /* attente sur pipe plein ou vide  */
#define  EV_WAIT  256  /* attente exit du fils            */
#define  EV_LOCK  512  /* attente sur file locking        */

/* flags de controle des etats transitoires */
#define  F_ALRM     1  /* delais de type watchdog en cours               */
#define  F_TIMOUT   2  /* indique si time-out tomb‚                      */
#define  F_DELAY    4  /* un delais cours sur cette tache                */
#define  F_WAIT     8  /* le pere attend le fils                         */
#define  F_WAITED  16  /* le pere attendait le fils (mais n'attend plus) */
#define  F_LOCK    32  /* la tache a lock‚ un fichier                    */

/*
 * Definitions diverses
 *
 */

#define NTASK     50      /* nb max de taches                 */
#define TNMLEN     8      /* longueur d'un "nom" de process   */
#define ARGLEN   160      /* longueur du buffer argument      */

/* pid des taches systemes */

#define TASK0      0             /* id de la tache racine             */
#define SCHEDULER  TASK0+1
#define CALLOUT    SCHEDULER+1       /* id tache gestion des delais       */
#define TIME       CALLOUT+1     /* id tache gestion du temps         */
#define E_S        TIME+1        /* id tache d'entree / sortie disque */
#define SHTDWN     E_S+1        /* id tache retour vers DOS          */
#define VIO        SHTDWN+1      /* id tache lecture du clavier       */
#define MOU_TASK   VIO+1         /* id demon session erreur           */
#define NSYSTASK   VIO+1    /* nb de taches systemes             */

#define BADPID    -1      /* numero de process ID incorrect   */
#define NFD       20      /* nb de file descriptors           */
#define SIGNR     16      /* nb de signaux (13 reels)         */
#define MAXMESS    8      /* nb de messages NMX possibles     */


/*
 *      entree dans la table des process
 *
 */

struct   taskslot  {
        char          tname[TNMLEN];  /* nom du process                     */
        int           tuser;          /* numero d'utilisateur               */
        int           tgrp;           /* numero d'ecran attach‚ a cette T   */
        int           targc;          /* nb d'arguments au depart           */
        char          targbuf[ARGLEN];/* pointeurs sur arg + strings        */
        stream_entry *tfd[NFD];       /* tableau de pointeurs sur Streamtab */
        int           taddr;          /* adresse de demarrage du process    */
        int           tppid;          /* contient le pid du pere            */
        char          tstate;         /* RUNNING, READY, SLEEP, UNUSED      */
        unsigned      tevent;         /* si SLEEP: ev bloquant la tache     */
        uchar         tflag;          /* flags gestion etats transitoires   */
        int           tprio;          /* priorite du process                */
        int           tkernel;        /* nb d'appels KernelMode emboites    */
        int           tupid;          /* user pid                           */
        int           tITvalid;       /* IT valides  pour cette tache ?     */
        int           ttyp;           /* UMAIN, SYS, USER                   */

        /* gestion des devices */
        int           tcurrdev;       /* device courant pour ce process     */
        char          tcurrdir[64];   /* pt directory courante pour ce process */

        /* gestion des erreurs */
        int           terrno;         /* num‚ro de la derniŠre erreur       */

        /* zone gerant les signaux */
        unsigned      tevsig;         /* bitmap des evenements attendus     */
        unsigned      tevcatch;       /* bitmap des evenements recus        */
        int         (*tevfunc[SIGNR])(); /* pointeurs des fonctions SIGNAL  */
        int           texitstatus;    /* status retourn‚ par le process     */

        /* attente sur semaphore */
        int           tsem;           /* semaphore si process ds l'etat WAIT */

        /* messages */
        uchar          tmsg[MAXMESS],         /* zone de reception message(s) */
                       tmsgnr,        /* nombre de messages sous IT en attente */
                       tmsgIn,        /* index d'entree */
                       tmsgOut;       /* index de sortie */
        int            tnextT;        /* chainage des tƒches bloqu‚es en REC */
/*      struct hblk   *theadMsg;      /* tete de file des messages */
/*      struct hblk   *ttailMsg;      /* queue de file des messages */
/*      unsigned       tmsgFlag;      /* flag de Synchro */

        /* gestion des pipes */
        int           tpipe_nr;       /*  numero du pipe sur lequel la tache
                                       *  est suspendue (  si elle l'est )
                                       */
        int           tpipe_op;       /* suspension sur READ ou WRITE       */

        /* gestion de la pile */
        int far      *tstktop;        /* sommet de la pile d'execution      */
        int           tstklen;        /* longueur Ustack + Kstack           */
        int          *retAddr;        /* fonction fin de tache */

        /*-- PILE mode USER --*/
        int far      *tUstkbase;       /* svg @ Haute Ustack pour FREE (a.dyn)*/
                                       /* *pbase = MAGIC                      */
        int           tUstklen;        /* longueur Ustack                     */
        int far      *tUstklim;        /* @ basse de Ustack ( overflow )      */
        unsigned      tUSP;            /* svg offset Ustack */

        /*-- PILE mode KERNEL --*/
        int far      *tKstkbase;       /* svg @ Haute Kstack pour FREE (a.dyn)*/
                                       /* *pbase = MAGIC                      */
        int           tKstklen;        /* longueur Kstack                     */
        int far      *tKstklim;        /* @ basse de Kstack ( overflow )      */
        unsigned      tKSP;            /* svg offset Kstack */

        /* gestion m‚moire */
        struct hblk  *theadblk;        /* tete de la liste des blocs
                                        * allou‚s dynamiquement
                                        */
        struct hblk  *ttailblk;        /* Queue de liste */

};

/* description de la pile :

Memoire basse                                 Memoire haute
   _________________________________________________
  |                        |                        |
  |    Ustack (1/2)        |    Kstack (1/2)        |
  |________________________|________________________|
  ^                        ^                        ^
  lim (c'est aussi Ulim)   Ubase                    base ( c'est aussi Kbase)
*/

#define  isbadpid(x)      ( x <= 0   || x >= NTASK)
#define  isbadpid2(x)     ( x <= 1   || x >= NTASK)

extern struct taskslot Tasktab[];
extern int      numproc ;         /* numero du process courant              */
extern unsigned nextproc;         /* prochain numero attribu‚ a un process  */
extern int      RUNpid;           /* currently executiv process             */
extern int      nextslot;         /* prochain free slot                     */
extern ushort   glowdir;          /* working directory prise par defaut     */
extern ushort   glowdev;          /* working device pris par defaut         */
extern char    *errStr[];         /* libell‚ des messages d'erreur          */