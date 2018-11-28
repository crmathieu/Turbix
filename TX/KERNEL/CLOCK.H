/* clock.h */

#define TIMOVER      -5
#define DOSCLOCK      27     /* int 1Bh : interruption horloge DOS */
#define XENCLOCK      8      /* int 1Ch: dunix clock interrupt */
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

       /* calling task's PID */
       int            b_pid;
       int            b_func;

       struct biosreq *b_next;
};

struct msgtime {
       int          h_op;
       int          h_time[3];
       int          h_date[3];
};

#define BIOSNULL       (struct biosreq *)0

/* various supported services */
#define CLK_SRV        0x80   /* clock service */
#define MOU_SRV        0x90   /* mouse service */

/* -- remarque : dans le message - la tache souris
 * la partie HAUTE du message correspond - MOU_GET
 *                                         MOU_SRV
 * pour un service de type MOU_MASK, la
 * partie basse correspond au masque
 */


extern struct biosreq *hbreq;
extern int dayByMonth[];