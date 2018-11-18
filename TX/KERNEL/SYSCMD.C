/* MKD: syscmd.c */


#include "sys.h"
#include "conf.h"
#include "signal.h"
#include "shell.h"
#include "cmd.h"
#include "pc.h"
#include "io.h"
#include "sem.h"
#include "tty.h"
#include "window.h"
#include "console.h"
#include "const.h"


#include "dynlink.h"

extern char   interleave[];
extern char  *ttynams[];

LOCAL    char ps1[] =
         " S  UID   PID   PPID   PRI    SLEN   TNAME   TTY   EVENT\n";

LOCAL    char ps2[] = " ";
LOCAL    char *pstnams[] = {"RU","0","RY","SL"};

LOCAL    int  psavsp;
LOCAL    char dv1[] = "Device    minor \n";
LOCAL    char dv2[] = "-------   ----- \n";


LOCAL    char mm1[] = " TOTAL   INITIAL    STACK    HEAP     BUFFERS    SESSIONS\n";
LOCAL    char mm2[] = " ";

LOCAL    char *ev[] = {"sem","zom","msg","tms","pau","clk","sus","pip","wai","lck"};


static position(val)
{
   int i = 0;
   while ((val = val >> 1) > 0)
      i++;
   return(i);
}

/*----------------------------------------------------------------------
 * c_ps - (command ps) format and print process table information
 *----------------------------------------------------------------------
 */
SHELLCOM c_ps(argc,argv)
int argc;
char *argv[];
{
    int  i,window, ps;
    struct taskslot *pptr;
    char *p;
    unsigned currstk;
    unsigned nbproc;

    if (argc == 1) window = NSYSTASK;
    else
        if (*argv[1] == 'a')  window = 0;
        else                  window = NSYSTASK;

    putchar('\n');
    m_Printf("%s",ps1);
    ps = _itDis();
    for (nbproc = 0, i = window; i < NTASK; i++)
    {
       if ((pptr = &Tasktab[i])->tstate == UNUSED)   continue;
       nbproc++;
       m_Printf( "%3s  %2d  %4d   %4d   %3u   %5u   %-8s",
               pstnams[pptr->tstate-1], pptr->tuser,i,
               pptr->tppid,
               pptr->tprio,pptr->tstklen,pptr->tname);

       switch(pptr->tfd[1]->s_streamtyp) {
              case PIPESTREAM :
                   m_Printf(" %-5s ","pipe");break;
              case TTYSTREAM  :
                   m_Printf(" %-5s ",tty[pptr->tfd[stdout]->s_minor].ttyname);
                   break;
              default         :
                   m_Printf(" %-5s ","file");
       }
       if (pptr->tstate == SLEEP)
           p = ev[position(pptr->tevent)];
       else
           p = "...";
       m_Printf("%3s\n", p);
    }
    m_Printf(psStr, nbproc);
    _itRes(ps);
}

/*----------------------------------------------------------------------
 * c_help -
 *----------------------------------------------------------------------
 */
c_help()
{
    int inc;
    int i;
    int j,n;
    n = nb_command();
    if ( (inc = (n + COLUMNS - 1) / COLUMNS) <= 0)
         inc = 1;

    m_Printf(hlpStr);
    for (i = 0; i < inc && i < n; i++) {
         m_Printf("  ");
         for (j = i; j < n; j += inc)
              m_Printf("%-16s", Commandtab[j].cmdName);
         m_Printf("\n");
    }
    return(ROK);
}

/*----------------------------------------------------------------------
 * c_kill -
 *----------------------------------------------------------------------
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
    for ( i = 3;i<NTASK;i++)
        if (i == pid)
        {
           j = Tasktab[i].tppid;
           if ( (status = _sendsig(j,SIGCLD,-1,i)) == RERR)
                m_Printf(killOkStr, pid);
           else
           {
                m_Printf(killNokStr, Tasktab[i].tname);
           }
           _itRes(ps);
           return(status);
        }
    m_Printf(badPidStr);
    _itRes(ps);
    return(RERR);
}




/*----------------------------------------------------------------------
 * c_exit -
 *----------------------------------------------------------------------
 */
SHELLCOM c_exit()
{
    m_Msgsync(SHTDWN);
}


/*----------------------------------------------------------------------
 * c_mem
 *----------------------------------------------------------------------
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

    for (free = 0, mptr = memlist.mnext; mptr != (struct mblock *)NULL;
         mptr = mptr->mnext)         free += mptr->mlen;

    for (stkmem = 0, i = 0; i < NTASK; i++) {
         if (Tasktab[i].tstate != UNUSED)
             stkmem += (unsigned)Tasktab[i].tstklen;
    }
    for (i = 0,k = 0; i < NVS-1 ; i++)
         if (tty[i].vsinit) k++;

    free *= 16;
    avail1 = FP_SEG(maxaddr);
    avail1 = (avail1 * 16) + 16;

    avail2 = FP_SEG(maxaddr) - FP_SEG(debaddr);
    avail2 = (avail2 * 16) + 16;

    m_Printf("%s",mm1);
    m_Sprintf(str,"%lu   %7lu  %7lu  %7lu   %7lu    %7lu\n",
                 avail1,avail2,stkmem,free,
                 (long)0,(long)(k * 4000));
    m_Printf("%s",str);
/*    m_Sprintf(str,"\nfree blocks           Address  |     Size   \n");
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

/*----------------------------------------------------------------------
 * c_resx - deblocage de la liaison serie
 *----------------------------------------------------------------------
 */
c_resx( argc, argv)
int   argc;
char *argv[];
{

    register struct tty *iptr;
    register struct csr *cptr;
    int junk;

    if (argc != 2)
    {
       m_Printf(resxUseStr);
       return;
    }
    iptr = &tty[TTY1+atoi(argv[1])];
    cptr = iptr->ioaddr;

    junk = inp(cptr->cetlgn);

}

/*----------------------------------------------------------------------
 * c_more -
 *----------------------------------------------------------------------
 */
c_more( argc, argv)
int   argc;
char *argv[];
{
     char buff[80];
     char c;
     int i = 0;
     int n,j,fdin,flag,s,nb,savPid;
     struct Window *pwin;

     if ((fdin = m_Open(ttyname[s = _getSessionHandle()], 2/* O_RDWR*/)) == RERR)
          return(RERR);

     if (flag = tty[s].wmode) {
         pwin = tty[s].curwin;
         nb = pwin->_maxy - 2;
         savPid = pwin->_pid;
     }
     else
         nb = 22;
     while ((n = m_Read(0,buff,80)) > 0) {
            j = 0;
            while (n-->0) {
                   putchar(c = buff[j++]);
                   if (c == '\n')
                       if (++i > nb) {
                           i = 0;
                           m_Printf(moreStr);
                           if (flag)
                                 (* winfunc[WIN_GETCH])(pwin);
                           else
                                 m_Sgetch();
                       }
            }
     }
     m_Close(fdin);
     if (flag) {
         pwin->_pid    = savPid;
         pwin->_flags |= W_SUSP;
     }
     return(ROK);
}

