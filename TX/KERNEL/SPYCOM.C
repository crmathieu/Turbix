/* spycom.c shell commands */


#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "window.h"
#include "signal.h"
#include "console.h"
#include "shell.h"
#include "const.h"
#include "sem.h"
#include "fspipe.h"
#include "stat.h"
#include "setjmp.h"
#include "fsopen.h"
#include "dynlink.h"
#include "cmd.h"
#include "q.h"

extern char   interleave[];
extern char  *ttynams[];

LOCAL    char ps1[] =
         " S   PID   PPID   PRI    SLEN   TNAME   STDOUT  EVENT\n";

LOCAL    char ps2[] = " ";
LOCAL    char *pstnams[] = {"RU","0","RY","SL"};

LOCAL    int  psavsp;
LOCAL    char dv1[] = "Device    minor \n";
LOCAL    char dv2[] = "-------   ----- \n";


LOCAL    char mm1[] = " TOTAL   INITIAL    STACK    HEAP     SESSIONS\n";
LOCAL    char mm2[] = " ";

LOCAL    char *ev[] = {"sem","zom","msg","tms","pau","clk","sus","pip","wai","lck"};

LOCAL    char *hproc[]  = {"dev", "pipe", "file"};

/*-----------------------------------------------
 * c_copy  -  file copy
 *-----------------------------------------------
 */
c_copy( argc, argv)
int   argc;
char *argv[];
{
    unsigned char buff[512];
    int n,i;
    int fdsource,fdtarget;

    if (argc != 3)
    {
        m_Printf("\nuse : copy [sourcefile] [targetfile]\n");
        return;
    }
    putchar('\n');
    if ((fdsource = m_Open(argv[1],O_RDONLY)) < 0) {
        m_Perror("copy");
        m_Exit(-1);
    }
    if ((fdtarget = m_Creat(argv[2],S_IREAD|S_IWRITE)) < -1) {
        m_Perror("copy");
        m_Exit(-1);
    }
    while ((n = m_Read(fdsource,buff,512)) > 0) {
        m_Write(fdtarget,buff,n);
    }
     m_Close(fdsource);
     m_Close(fdtarget);
}


/*-----------------------------------------
 * c_type - display file content
 *-----------------------------------------
 */
c_type( argc, argv)
int   argc;
char *argv[];
{
     unsigned char buff[512];
     int n,i,pid;
     int fd;
     int typeExit();

     if (argc != 2)
     {
         m_Printf("\nuse : type [file_name]\n");
         return;
     }
     m_Signal(SIGINT,typeExit);
     putchar('\n');
     if ((fd = m_Open(argv[1],O_RDONLY)) < 0) {
          m_Perror("type");
          m_Exit(-1);
     }
/*     m_Lock(fd);*/
     while ((n = m_Read(fd,buff,512)) > 0) {
        VS0printf("Type : writing...\n");
        m_Write(1, buff, n);
     }

/*     m_Unlock(fd);*/
     m_Close(fd);
}
typeExit()
{
    m_Printf("\nTerminated by user : exit\n");
    m_Exit(0);
}

/*----------------------
 * c_cls - clear screen
 *----------------------
 */
c_cls()
{
    m_Printf("\033[2J\033[0;0H");
}


/*---------
 * c_delay
 *---------
 */
c_delay( argc, argv)
int   argc;
char *argv[];
{
    int hs[5], i;

    if (argc != 2) {
        m_Printf("\nuse : delay <n sec>\n");
        return;
    }
    m_Sleep(atoi(argv[1]));
}


/*---------
 * c_session
 *---------
 */
c_session()
{
  int m_Spy();

  char *argv[4];

  argv[1] = "-i";
  argv[2] = NULLPTR;
  if (m_Fork() == 0)
  {
     if (m_GetSession()) {
        m_printf("Too many sessions...\n");
        m_exit(0);
     }

     argv[0] = "spy";
     m_Exec(m_Spy,argv);
     m_Printf("\nExec Fails in TTY1\n");
  }
}

/*---------
 * EXIT
 *---------
 */
c_exit()
{
      m_Exit(0);
}

/*--------
 * c_help
 *--------
 */
c_help()
{
    int inc;
    int i;
    int j,n;
    n = nb_command();
    if ( (inc = (n + COLUMNS - 1) / COLUMNS) <= 0) {
        inc = 1;
    }
    m_Printf(hlpStr);
    for (i = 0; i < inc && i < n; i++) {
        m_Printf("  ");
        for (j = i; j < n; j += inc) {
            m_Printf("%-16s", Commandtab[j].cmdName);
        }
        m_Printf("\n");
        m_Sleep(0);
    }
    return(ROK);
}

/*--------
 * c_kill
 *--------
 */
c_kill( argc, argv)
int   argc;
char *argv[];
{
    int i,pid,j;
    int status;
    int ps;

    if (argc != 2) {
         m_Printf(killStr);
         return(RERR);
    }
    pid = atoi(argv[1]);
    ps = _itDis();
    for ( i = 3;i<NTASK;i++) {
        if (i == pid) {
           j = Tasktab[i].tppid;
           if ( (status = _sendsig(j,SIGCLD,-1,i)) == RERR) {
                m_Printf(killOkStr, pid);
           } else {
                m_Printf(killNokStr, Tasktab[i].tname);
           }
           _itRes(ps);
           return(status);
        }
    }
    m_Printf(badPidStr);
    _itRes(ps);
    return(RERR);
}


/*----------------------------------------------------------------------
 * c_ps - (command ps) format and print task table information
 *----------------------------------------------------------------------
 */
SHELLCOM c_ps(argc,argv)
int argc;
char *argv[];
{
    int  ps, i,window;
    struct taskslot *pptr;
    char *p, *psw;
    unsigned currstk;
    unsigned nbproc;

    if (argc == 1) {
        window = NSYSTASK;
    } else {
        if (*argv[1] == 'a') {
            window = 0;
        } else {
            window = NSYSTASK;
        }
    }
    putchar('\n');
    m_Printf("%s",ps1);
    ps = _itDis();
    for (nbproc = 0, i = window; i < NTASK; i++) {
        if ((pptr = &Tasktab[i])->tstate == UNUSED) {
            continue;
        }
        nbproc++;
        m_Printf( "%3s %4d   %4d   %3u   %5u   %-8s",
                pstnams[pptr->tstate-1],
                i,
                pptr->tppid,
                pptr->tprio,
                pptr->tstklen,
                pptr->tname);

        switch(pptr->tfd[1]->s_streamtyp) {
            case PIPESTREAM :
                psw = hproc[1];break;
            case TTYSTREAM  :
                psw = &tty[pptr->tfd[stdout]->s_minor].ttyname[0];
                break;
            default         :
                psw = hproc[2];
        }
        m_Printf(" %-5s ", psw);
        if (pptr->tstate == SLEEP) {
            p = ev[position(pptr->tevent)];
        } else {
            p = "...";
        }
        m_Printf("  %3s\n", p);
    }
    _itRes(ps);
    m_Printf(psStr, nbproc);
}

static position(val)
{
    int i = 0;
    while ((val = val >> 1) > 0) {
        i++;
    }
    return(i);
}


/*-------
 * c_mem
 *-------
 */
c_mem()
{
    struct mblock *mptr;
    char   str[120];
    int    i,k;
    unsigned long free;
    unsigned long avail1,avail2,avail3;
    unsigned long stkmem;

    /* calculate current size of free memory and stack memory */

    for (free = 0, mptr = memlist.mnext; 
            mptr != (struct mblock *)NULL;
                mptr = mptr->mnext) {
        free += mptr->mlen;
    }

    for (stkmem = 0, i = 0; i < NTASK; i++) {
        if (Tasktab[i].tstate != UNUSED) {
            stkmem += (unsigned)Tasktab[i].tstklen;
        }
    }
    for (i = 0,k = 0; i < NVS-1 ; i++) {
        if (tty[i].vsinit) {
            k++;
        }
    }

    free *= 16;
    avail1 = FP_SEG(maxaddr);
    avail1 = (avail1 * 16) + 16;

    avail2 = FP_SEG(maxaddr) - FP_SEG(debaddr);
    avail2 = (avail2 * 16) + 16;

    m_Printf(mm1);
    m_Sprintf(str,"%lu   %7lu  %7lu  %7lu    %7lu\n",
                 avail1, avail2, stkmem, free, (long)(k * 4000));
    m_Printf(str);
/*  m_Sprintf(str,"\nfree blocks           Address  |     Size   \n");
    m_Sprintf(str,"                               | \n");

    for (mptr = memlist.mnext, i = 0; mptr != (struct mblock *)NULL;
              mptr = mptr->mnext, i++)
    {
         avail3 = mptr->mlen;
         m_Sprintf(str,"                  %04x:0000    |  %7lU (%05u paragrafs) \n",
                                    FP_SEG(mptr), (avail3 * 16), avail3);
    }
    m_Sprintf(str,"\nNumber of free blocks : %d\n",i);
    m_Printf("%s",str);*/
    return(ROK);
}
/*--------
 * c_more
 *--------
 */
c_more( argc, argv)
int   argc;
char *argv[];
{
    char c, buff[80];
    int n, i, j, fdin, flag, s, nb, savPid;
    struct window *pwin;

    i = 0;
    if ((fdin = m_Open(ttyname[s = _getSessionHandle()], O_RDWR)) == RERR) {
        return(RERR);
    }

    flag = tty[s].wmode;
    if (flag) {
        pwin = tty[s].curwin;
        nb = pwin->_maxy - 2;
        savPid = pwin->_pid;
    } else {
        nb = 22;
    }

    while ((n = m_Read(0,buff,80)) > 0) {
        j = 0;
        while (n-->0) {
            putchar(c = buff[j++]);
            if (c == '\n') {
                if (++i > nb) {
                    i = 0;
                    m_Printf(moreStr);
                    if (flag) {
                        (* winfunc[WIN_GETCH])(pwin);
                    } else {
                        m_Sessiongetch(s);
                    }
                }
            }
        }
    }
    if (n < 0) {
        m_perror("more");
    }
    m_Close(fdin);
    if (flag) {
        pwin->_pid    = savPid;
        pwin->_flags |= W_SUSP;
    }
    return(ROK);
}

/*--------
 * c_sem
 *--------
 */
c_sem()
{
    int i, j;

    m_Printf("\nSEMAPHORE       COUNT       SID\n");
    for ( i = j = 0; i<NSEM ; i++) {
        if (Semtab[i].sstate != SFREE ) {
            j++;
            m_Printf("%-8s        %3d          %2d\n",
                     Semtab[i].sname, Semtab[i].semcnt, i);
        }
    }
    m_Printf("\n%d semaphore(s) used\n", j);
}

/*--------
 * c_seml - List tasks blocked on a given semaphore
 *--------
 */
c_seml(argc, argv)
int argc;
char *argv[];
{
    int sem;
    int i, j;

    if (argc != 2) {
        m_Printf("\nuse : SEML <semaphore ID>\n");
        return(0);
    }
    if (Semtab[sem = atoi(argv[1])].sstate == SFREE) {
        m_Printf("Bad semaphore ID\n");
        return(0);
    }

    m_Printf("\nTASKS        PID\n");
    for (j = 0, i = Sysq[Semtab[sem].sqhead].next; 
            i != Semtab[sem].sqtail; 
                i = Sysq[i].next) {
        j++;
        m_Printf("%-8s        %2d\n", Tasktab[i].tname, i);
    }
    if (!j) {
        m_Printf("\nno process waiting for semaphore %d\n", sem);
    } else {
        m_Printf("\n%d process(es) wait(s) for this semaphore\n", j);
    }
}

/*--------
 * c_pipe
 *--------
 */
c_pipe()
{
    int i, j;

    m_Printf("\nPIPE ID       AVAIL\n");
    for (i = j = 0; i < NPIPE ; i++) {
        if (Piptab[i].count != 0) {
            j++;
            m_Printf("%4d           %4d\n", i, Piptab[i].avail);
        }
    }
    m_Printf("\n%d pipe(s)\n", j);
}
/*--------
 * c_pipel
 *--------
 */
c_pipel(argc, argv)
int argc;
char *argv[];
{
    static char *_status[] = {"read", "write"};
    int pipe, i, j, next, next2;
    struct taskslot *tp;

    if (argc != 2) {
        m_Printf("\nuse : PIPEL <pipe ID>\n");
        return(0);
    }
    if (Piptab[pipe = atoi(argv[1])].count == 0) {
        m_Printf("Bad pipe ID\n");
        return(0);
    }
    for (i = 0; i < NPIPE; i++) {
        if (Piptab[i].pipe_nr == pipe) {
                break;
        }
    }
    if (i == NPIPE) {
        m_Printf("Bad pipe ID\n");
        return(0);
    }
    m_Printf("\nTASKS          PID            OPERATION\n");
    for (j = 0, next = Sysq[piphead].next; next != piptail;) {
        next2 = Sysq[next].next;
        if ((tp = &Tasktab[next])->tpipe_nr == Piptab[i].pipe_nr) {
            j++;
            m_Printf("%-8s         %3d              %-6s\n", Tasktab[next].tname, next, _status[tp->tpipe_op]);
        }
        next = next2;
    }
    if (!j) {
        m_Printf("\nno process suspended on pipe %d\n", pipe);
    } else {
        m_Printf("\n%d process(es) suspended on this pipe\n", j);
    }
}
/*--------
 * c_pipec - pipe content
 *--------
 */
c_pipec(argc, argv)
int argc;
char *argv[];
{
    int pipe, i, j, next, next2;
    struct taskslot *tp;

    if (argc != 2) {
        m_Printf("\nuse : PIPEC <pipe ID>\n");
        return(0);
    }
    if (Piptab[pipe = atoi(argv[1])].count == 0) {
        m_Printf("Bad pipe ID\n");
        return(0);
    }
    for (i = 0; i < NPIPE; i++) {
        if (Piptab[i].pipe_nr == pipe) {
            break;
        }
    }
    if (i == NPIPE) {
        m_Printf("Bad pipe ID\n");
        return(0);
    }
    m_Printf("\nPipe Contents:\n");
    for (j=0; j < Piptab[i].avail; j++) {
        putchar(Piptab[i].pipzon[(Piptab[i].offR+j) % 4096]);
    }
}

/*--------
 * c_proc
 *--------
 */
c_proc(argc, argv)
int argc;
char *argv[];
{
     int pid, typ, slot, j;
     char *hprocw;
     stream_entry   *sp;
     struct taskslot *tp;

    if (argc != 2) {
        m_Printf("\nuse : TASK <task ID>\n");
        return(0);
    }
    pid = atoi(argv[1]);
    tp = &Tasktab[pid];

    m_Printf("\nTASK %s", tp->tname);
    m_Printf("\nHANDLE     TYPE     NAME\n");
    for (slot = j = 0 ; slot < NFD ; slot++) {
        if ((sp = tp->tfd[slot]) == NULLSTREAM) {
            continue;
        } else {
            switch(sp->s_streamtyp) {
            case TTYSTREAM : typ = 0;hprocw = &tty[tp->tgrp].ttyname[0];break;
            case PIPESTREAM: typ = 1;hprocw = NULLSTR;break;
            case FILESTREAM: typ = 2;hprocw = &sp->s_ft->fname[0];break;
            }
            m_Printf("%3d        %4s     %s\n", slot, hproc[typ], hprocw);
            j++;
        }
        m_Printf("\n%d handle(s) used\n", j);
        switch (tp->tevent) {
        case EV_SEM  :     /* blocked on semaphore                  */
                        m_printf("semID = %d semName = %8s with count = %d\n",
                        tp->tsem, Semtab[tp->tsem].sname, Semtab[tp->tsem].semcnt);
                        break;
        case EV_ZOM  :     /* wait to be handled by parent          */
        case EV_MESS :     /* wait for message to receive           */
        case EV_TMESS:     /* wait for message to recv with timeout */
        case EV_PAUSE:     /* wait for signal                       */
        case EV_CLOCK:     /* wait for end delay                    */
        case EV_SUSP :     /* wait to start                         */
        case EV_PIPE :     /* wait on empty or full pipe            */
        case EV_WAIT :     /* wait for child to exit                */
        case EV_LOCK :     /* wait on file lock                     */
        default      : break;
        }
    }
}
/*--------
 * user_app
 *--------
 */
/*user_app(argc, argv)
char *argv[];
{
     unsigned char buff[512];
     char path[64];
     int n,i,pid;
     int fd;


     m_Printf("CREATION DIR!\n");
     if (m_Mkdir("popo"))
        m_Perror("ERR MKDIR");
     if (m_Chdir("popo"))
        m_Perror("ERR CHDIR");
     m_Getcwd(path);
     m_printf("NEW CWD : %s\n", path);

}*/
