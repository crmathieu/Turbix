/* sem.h */

typedef SEM     int;

/* semaphore functions */
int     cdecl   m_Countsem(SEM semhandle, int *retcnt);
SEM     cdecl   m_Creatsem(int cpt, char *semname);
int     cdecl   m_Delsem(SEM semhandle);
int     cdecl   m_Pwaitsem(SEM semhandle);
int     cdecl   m_Resetsem(SEM semhandle, int cpt);
int     cdecl   m_Sigsem(SEM semhandle);
int     cdecl   m_Waitsem(SEM semhandle);
