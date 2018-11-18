/* ipc.h */

/* parametres d'obtention d'un qid dans msgget */
#define  IPC_CREAT    0001000  /* allouer une entree en Qtable */
#define  IPC_ALLOC    0100000  /* demander le qid d'une entree */
#define  IPC_EXCL     0002000  /* en association avec IPC_CREAT,
				* permet de savoir si l'entree existait
				* deja avant le msgget
				*/
#define  IPC_PRIVATE        0
#define  IPC_NOWAIT   0004000

/* paramŠtres de commande dans msgctl */
#define  IPC_STAT      2
#define  IPC_SET       1
#define  IPC_RMID      0

typedef  int        	 key_t;
typedef  unsigned long   time_t;
typedef  unsigned short  ushort;

/* Aspect communs des IPC : definition des droits d'une IPC */
struct ipc_perm {
	ushort uid;	/* propri‚taire de l'IPC */
	ushort gid;	/* inutilis‚ */
	ushort cuid;	/* cr‚ateur de l'IPC */
	ushort cgid;	/* inutilis‚ */
	ushort mode;	/* droits sur l'IPC */
	ushort seq;	/* compteur d'utilisation du poste */
	key_t key;	/* cl‚ de l'IPC */
};

/* la valeur du qid retournee par Msgget se decompose comme suit:
 *
 *		QID = rang du poste + (seq * dimension de Qtable)
 *
 * Algo de verification:
 *
 *		si (QID != (QID % dimension de Qtable) +
 *			   (seq * dimension de Qtable))
 *		alors
 *			le poste a ete supprim‚, puis recr‚‚. Ce qid
 *			fait reference … l'ancien poste : ERREUR
 *		sinon
 *			rang du poste = QID % dimension de Qtable
 */
