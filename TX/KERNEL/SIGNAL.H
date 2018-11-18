/* signal.h */

#define SIG_IGN (int (* )()) 0
#define SIG_DFL (int (* )()) 1

/* types de  signaux */
#define SIGINT       1                  /* CTRL C  */
#define SIGQUIT      2                  /* CTRL D  */
#define SIGILL       3                  /* instruction illegale (AT)        */
#define SIGOVF       4                  /* overflow dans calcul             */
#define SIGKILL      5                  /* tuer tache                       */
#define SIGALRM      6                  /* alarme                           */
#define SIGUSR1      7                  /* signal utilisateur  1            */
#define SIGUSR2      8                  /* signal utilisateur  2            */
#define SIGUSR3      9                  /* signal utilisateur  3            */
#define SIGUSR4     10                  /* signal utilisateur  4            */
#define SIGUSR5     11                  /* signal utilisateur  5            */
#define SIGDIV      12                  /* division par 0                   */
#define SIGCLD      13                  /* death of child signal            */


extern int SigMask[];

/* macro */
#define unsignal(pid,sig)  (Tasktab[pid].tevsig & (~SigMask[sig]))
