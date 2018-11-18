
/* utask.c */
#include "setjmp.h"
#include "sys.h"
#include "conf.h"
#include "sem.h"
#include "q.h"
/*#include "tty.h"*/
#include "io.h"
#include "serial.h"
#include "shell.h"
#include "const.h"
#include "fsopen.h"
#include "dis.h"
#include "signal.h"

#include "ipc.h"
#include "msg.h"
#include "termio.h"

extern int currDrive;
extern char currWD[];
extern int _errflag;

struct string {
        int lg;
        char str[80];
} toto;
/* Gestion des drives */
struct DriveTable {
        char drive;
        char valide;
        char *path;
};
extern int nflp_phy;

        char buf[] = "abcd12341777 Athis_is_a_string";
        umain2()
        {
                int   octalVar;
                long  longHexaVar;
                char  charVar, stringVar[30];

                m_Scanf("Long hexa: %lx - Octal: %o - String %s\n",
                         &longHexaVar,
                         &octalVar,
                         stringVar);
                m_Printf("Long hexa  stored: %lx\n", longHexaVar);
                m_Printf("     Octal stored: %o\n",  octalVar);
                m_Printf("String     stored: %s\n",  stringVar);
                m_Getch();
                m_Sscanf(buf, "%lx %o %c %s",
                         &longHexaVar,
                         &octalVar,
                         stringVar,
                         &charVar);
                m_Printf("Long hexa  stored: %lx\n", longHexaVar);
                m_Printf("     Octal stored: %o\n",  octalVar);
                m_Printf("Character  stored: %c\n",  charVar);
                m_Printf("String     stored: %s\n",  stringVar);
                m_Getch();

        }


umain(c,p)                 /* essai tache utilisateur */
char *p[];
{
  int myHandler();
  extern struct DriveTable *pdt;
  int fd,fdup,n,i;
  int m_Spy();
  long l,k;
  char ch;

  char *argv[4];

  for (i=0;i<c;i++)  if (p[i] == NULLPTR) break;
                     else m_Printf("ARG%d = %s\n",i,p[i]);

  for (i=1; i<27; i++)
        if ((pdt+i)->valide)
                m_Printf("Drive %c  Dir = %s\n",(pdt+i)->drive,(pdt+i)->path);
  m_Printf("NOMBRE de floppies PHY = %d\n", nflp_phy);

/*  if ((fd = m_Creat("bidon", S_IREAD|S_IWRITE)) == -1)
        return;
  memset(&toto.str[0], 'A', 79);
  toto.str[79] = '\0';
  m_Write(fd, &toto, sizeof(struct string));

  memset(&toto.str[0], 'B', 79);
  toto.str[79] = '\0';
  m_Write(fd, &toto, sizeof(struct string));

  memset(&toto.str[0], 'C', 79);
  toto.str[79] = '\0';
  m_Write(fd, &toto, sizeof(struct string));

  memset(&toto.str[0], 'D', 79);
  toto.str[79] = '\0';
  m_Write(fd, &toto, sizeof(struct string));

  m_Lseek(fd, (long)(sizeof(struct string)), 0);
  m_Read(fd, &toto, sizeof(struct string));

  m_Printf(" 2eme ENr theorique : %s", toto.str);
  m_Close("bidon");
  return;*/
/*  if (Tasktab[RUNpid].tfd[4] != NULLSTREAM)
        m_Printf("DIFFERENCE 1\n");
  if ((fd = m_Open("toto.c", O_APPEND|O_RDWR)) < 0)
        m_Exit(-1);
  if (Tasktab[RUNpid].tfd[4] != NULLSTREAM)
        m_Printf("DIFFERENCE 2\n");

  m_Fprintf(fd, "TROU DU CUL\n");*/

/*  while ((n=m_Read(fd, buf, 512)) > 0)
        for (i=0; i<n; i++)
                putchar(buf[i]);*/
/*  m_Close(fd);
  if (Tasktab[RUNpid].tfd[4] != NULLSTREAM)
        m_Printf("DIFFERENCE 3\n");*/

  argv[1] = "-i";
  argv[2] = NULLPTR;
  m_Printf("DRIVE = %c  errflag = %d DIR = %s\n", 'A'+currDrive-1, _errflag, currWD);


                        if (m_Fork() == 0) { /* fille */
                                m_Printf("APRES FORK DANS UTASK\n");
                                m_Gsleep(1); /* laissez la mŠre diminuer
                                           * sa priorit‚
                                           */
                                m_Printf("Prio TACHE Mere : %d\n", m_GetPriority(m_Getppid()));
                                m_Printf("Prio TACHE Fille: %d\n", m_GetPriority(m_Getpid()));
                                m_Getch();
                                m_Exit(0);
                        }
                        m_Nice(2);
                        m_Pause();


/*  if (m_Fork() == 0)
  {
     if ((fd = m_Open("/dev/tty1",O_RDWR)) == -1) {
          m_Printf("\nerreur OPEN dans fils\n");
          m_Exit(-1);
     }*/
/*     m_GetSession();*/
     argv[0] = "sht";
/*     fdup = m_Dup(fd);
     m_Chgstdio(0, fd);
     m_Chgstdio(1, fdup);*/
/*     m_Dup2(fd, 0);
     m_Dup2(fd, 1);*/
/*     m_Execv(shell,argv);
     m_Printf("\nExec Fails in TTY1\n");
  }
  else {*/
    /* pere */
  m_Printf("AVANT SHELL\n");
  m_Printf("NAME = %s\n", Tasktab[RUNpid].tname);
  m_Execl(m_Spy,"sh","-i",NULLPTR);
  m_Printf("\nExec Fails in CON\n");
/*  }*/
}


umain4()
{        /* Start the hanoi tower program (defined in sample.c) */
        m_Printf("Hello world\n");
        m_Getch(); /*wdemo();*/

}
/*<<<<<<<<<<<<<<<<<<<<<<<<<<< Setjmp & longjmp >>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

jmp_buf  jmpstack;

        setlongjmp()
        {
                int z;
                if ((z = m_Setjmp(jmpstack))) {
                    m_Printf("\nlongjmp call ret value = %d\n", z);
                    return(0);
                }
                m_Printf("\nsetjmp has been called\n");
                longjmpCall(10);
        }

        longjmpCall(n)
        {
                m_Printf("\nBefore longjmp call\n");
                m_Longjmp(jmpstack, n);
                m_Printf("\n - unreachable code -\n");
        }

user_app()
{        setlongjmp();
        m_Printf("Avant GETCH\n");
        m_Getch();
}