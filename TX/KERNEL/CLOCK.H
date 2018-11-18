/* clock.h */

#define TIMOVER      -5
#define DOSCLOCK      27     /* int 1Bh : interruption horloge DOS */
/*#define XENCLOCK      28     /* int 1Ch : it horloge de Xenius     */
#define XENCLOCK      8 /* MODIFFFFFF */
#define NOCLOCK       FALSE
#define QUANTUM       3

struct biosreq {
       /* Time */
       int            h_op;
       int            h_time[3];
       int            h_date[3];

       /* Mouse */
       int            m_x;
       int            m_y;
       int            m_etatBouton;
       unsigned       m_nofunc;

       /* PID de la tache appelante */
       int            b_pid;
       int            b_func;

       struct biosreq *b_next;
};

/* utilisation future des messages */
struct msgtime {
       int          h_op;
       int          h_time[3];
       int          h_date[3];
};

#define BIOSNULL       (struct biosreq *)0

/* differents services supportes */
#define HOR_SRV        0x80   /* service horloge */
#define MOU_SRV        0x90   /* service souris  */

/* -- remarque : dans le message … la tache souris
 * la partie HAUTE du message correspond … MOU_GET
 *                                         MOU_SRV
 * pour un service de type MOU_MASK, la
 * partie basse correspond au masque
 */


extern struct biosreq *hbreq;
extern int dayByMonth[];