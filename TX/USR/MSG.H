/* msg.h */


#define  MSG_NOMESS  1
#define  MSG_TIMEOUT 2

/* messages functions declarations */
int cdecl m_Msgclr(void);
int cdecl m_Msgrdv(void);
int cdecl m_Msgsync(int pid, unsigned char msg);
int cdecl m_Msgwait(unsigned char *bmsg, int delay, int *status);

