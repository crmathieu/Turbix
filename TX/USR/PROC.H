/* proc.h */


/* Process management API */
int      cdecl            m_Exec(int (proc)(),char *argv[]);
int      cdecl            m_Fork(void);
int      cdecl            m_GetPriority(int pid);
int      cdecl            m_Getpid(void);
char *   cdecl            m_GetProcName(int pid);
int      cdecl            m_SetProcName(int pid, char *name);
int      cdecl            m_Getppid(void);
int      cdecl            m_Nice(int prioGranul);
void     cdecl            m_Restore(unsigned regEtat);
void     cdecl            m_Shutdown(void);
int      cdecl            m_Exit(int status);
