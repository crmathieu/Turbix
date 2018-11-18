/* signal.h */


/* default actions values */
#define SIG_IGN (int (* )()) 0
#define SIG_DFL (int (* )()) 1

/* signals types */
#define SIGINT       1                  /* CTRL C  */
#define SIGQUIT      2                  /* CTRL D  */
#define SIGILL       3                  /* illegal instruction  (AT) */
#define SIGOVF       4                  /* overflow */
#define SIGKILL      5                  /* kill process */
#define SIGALRM      6                  /* alarm */
#define SIGUSR1      7                  /* user signal 1 */
#define SIGUSR2      8                  /* user signal 2 */
#define SIGUSR3      9                  /* user signal 3 */
#define SIGUSR4     10                  /* user signal 4 */
#define SIGUSR5     11                  /* user signal 5 */
#define SIGDIV      12                  /* divide by 0 */
#define SIGCLD      13                  /* death of child signal */

/* signal functions declarations */
int     cdecl                m_Kill(int pid, int sig);
void (* cdecl                m_Signal(int sig, void (*newAction)()));
int     cdecl                m_Wait(int *status);
int     cdecl                m_Pause(void);
