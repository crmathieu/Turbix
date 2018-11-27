/* Sample */

#include "io.h"
#include "signal.h"
#include "const.h"
#include "fcntl.h"
#include "video.h"
#include "win.h"



long cxtswitching = 0L;

/*------------------------------------------------------
 * cxtswitch - number of process switching per second
 *------------------------------------------------------
 */
cxtswitch()
{
   long x1, x2;
   int child1, child2;

   if ((child1 = m_Fork()) == 0) {
       m_Nice(1);
       while (1) {
         cxtswitching++;
         /*m_Sleep(0);*/   /* call scheduler */
       }
   }
   if ((child2 = m_Fork()) == 0) {
       m_Nice(1);
       while (1) {
         cxtswitching++;
         /*m_Sleep(0);*/   /* call scheduler */
       }
   }
   x1 = cxtswitching;
   m_Sleep(1);         /* wait for a second */
   x2 = cxtswitching;

   m_Printf("# context switching : %d\n",x2 - x1);
   m_Kill(child1, SIGKILL);
   m_Kill(child2, SIGKILL);
}

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< window >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

/* HANOI variables */
#define  POLE_XCOOR  1
#define  POLE_HIGHT  0
#define  MOVEUP      1
#define  MOVEDWN     0

/* direction variables */
#define S_RIGHT      1
#define S_LEFT       2
#define S_UP         4
#define S_DWN        8

/*--------------
 * creatWindow
 *--------------
 */
struct window *creatWindow(name,ink,paper,x,y,maxx,maxy)
char *name;
{
   struct window *pwin;
   int i,ps;
   pwin = m_newwin(maxy,maxx,y,x,ink,paper,name);

   m_box(pwin,'-','-');
   m_wpush(pwin);
   m_touchwin(pwin);
   return(pwin);
}

/*-----------
 * wdemo
 *-----------
 */
wdemo()
{
int            m_Spy();
int            n, i, X, Y, j;
int            direction[6];
struct window *pswin[6];
char           c, *P[2];
char              *argv[3];
struct window *pwin, *stdscr;

              /* init window mode */
              m_initscr(F_LCYAN,B_BLACK);

              /* init standard screen pointer */
              stdscr = m_initStdscrPTR();

              /* create 4 windows */
              pswin[1] = creatWindow("",F_LRED,B_BLUE,0,0,44,10);
              direction[1]  = S_RIGHT|S_DWN;
              pswin[2] = creatWindow("",F_LYELLOW,B_MAGENTA,38,13,40,10);
              direction[2]  = S_LEFT|S_UP;
              pswin[3] = creatWindow("",F_LWHITE,B_GREEN,0,11,40,9);
              direction[3]  = S_RIGHT|S_UP;
              pswin[4] = creatWindow("",F_LWHITE,B_BLACK,40,0,36,11);
              direction[4]  = S_LEFT|S_DWN;
              pswin[5] = creatWindow("",F_BLACK,B_WHITE,25,8,25,8);
              direction[5]  = 0;

              /*
               * start a program in each window
               */

              /* hanoi code in win1 */
              if (m_Fork() == 0) {
                  while (1)
                        starthanoi(pswin[1]);
              }

              /* hanoi code in win2 */
              if (m_Fork() == 0) {
                   while (1)
                        startHanoi(pswin[2]);
              }

              /* Hanoi code in win3 */
              if (m_Fork() == 0) {
                   while (1)
                        starthanoi(pswin[3]);
              }

              /* Hanoi code in win5 */
              if (m_Fork() == 0) {
                   while (1)
                        starthanoi(pswin[5]);
              }

              /* loop code in win4 */
              if (m_Fork() == 0) {
                   while (1) {
                        m_wautocrlf(pswin[4], TRUE);
                        loop(pswin[4]);
                  }
              }

              /* hanoi code in standard screen */
              if (m_Fork() == 0) {
                   while (1)
                        starthanoi(stdscr);
              }
              if (m_Fork() == 0) {
                        /* child process code : print a
                         * message in a new session
                         */
                        if (m_GetSession())
                                m_Exit(0);
                        argv[0] = "Spy";
                        argv[1] = "-i";
                        argv[2] = NULL;
                        m_Exec(m_Spy, argv);
              }
/*              if (m_fork() == 0) {
                i = 1;
                while (1) {
                        m_Sleep(2);
                        m_wselect(pswin[i]);
                        i = (++i == 6? 1:i);
                }
              }*/
              /* give focus to win5 */
              m_wselect(pswin[5]);

              /* Parent: move windows
               * position every second
               */
              j = i = 0;
              while (1) {
                     m_Sleep(0);
                     if (++j < 2)  m_Sleep(0);
                     else
                     {
                         j = 0;
                         stdscr->_flags |= W_GLOBALR;
                         m_wrefresh(stdscr);

                         /* compute new position according
                          * to window direction
                          */
                         for (i = 1; i < 6; i++) { /* for each window */
                                X = pswin[i]->_begx;
                                m_Sleep(0);
                                if (direction[i] & S_RIGHT) {
                                        if (pswin[i]->_begx+pswin[i]->_maxx+2 < 80)
                                                X = pswin[i]->_begx+1;
                                        else {
                                                direction[i] &= ~S_RIGHT;
                                                direction[i] |=  S_LEFT;
                                        }
                                }
                                m_Sleep(0);
                                if (direction[i] & S_LEFT) {
                                        if (pswin[i]->_begx > 0)
                                                X = pswin[i]->_begx-1;
                                        else {
                                                direction[i] &= ~S_LEFT;
                                                direction[i] |=  S_RIGHT;
                                        }
                                }
                                m_Sleep(0);
                                Y = pswin[i]->_begy;
                                if (direction[i] & S_UP) {
                                        if (pswin[i]->_begy > 0)
                                                Y = pswin[i]->_begy-1;
                                        else {
                                                direction[i] &= ~S_UP;
                                                direction[i] |=  S_DWN;
                                        }
                                }
                                m_Sleep(0);
                                if (direction[i] & S_DWN) {
                                        if (pswin[i]->_begy+pswin[i]->_maxy+2 < 25)
                                                Y = pswin[i]->_begy+1;
                                        else {
                                                direction[i] &= ~S_DWN;
                                                direction[i] |=  S_UP;
                                        }
                                }
                                m_Sleep(0);

                                /* MOVE ! */
                               if (m_mvwin(pswin[i], Y, X) == RERR)
                                        m_Exit(-1);
                         }
                     }
              }

}

/*----------
 * wSpy
 *----------
 */
wSpy()
{
   extern int m_Spy();
   char *argv[3];
   argv[0] = "spy";
   argv[1] = "-i";
   argv[2] = NULL;
   m_Exec(m_Spy, argv);
}


/*-------
 * hanoi
 *-------
 */
hanoi(pwin,n,a,b,c)
struct window *pwin;
int *a,*b,*c;
{
   if (n>0)  {
       hanoi(pwin,n-1,a,c,b);
       moveP(pwin,a,b,n);  /* n : plate size : 2n-1 characters */
       hanoi(pwin,n-1,c,b,a);
   }
}

/*-----------
 * moveP
 *-----------
 */
moveP(pwin,polS,polT,size)
struct window *pwin;
int *polS,*polT; /* source & target pole */
{
    move_vert(pwin,polS,MOVEUP,size);
    move_horizon(pwin,polS,polT,size);
    move_vert(pwin,polT,MOVEDWN,size);
    polS[POLE_HIGHT]--;
    polT[POLE_HIGHT]++;
}

/*------------
 * starthanoi
 *------------
 */
starthanoi(pwin)
struct window *pwin;
{
   int pol[4][2];      /* 3 poles. Each telling current hight
                        * and abscissa (pole 0 is unused)
                        */
   int BasePlate, Nplates, i;

   BasePlate = (pwin->_maxx-4)/3;
   Nplates    = min((BasePlate + 1)/2,pwin->_maxy-3);
   for (i=1;i<4; i++) {
        pol[i][POLE_HIGHT]  = 0;
        pol[i][POLE_XCOOR] = (i-1)*BasePlate + i + BasePlate/2;
   }
   pol[1][POLE_HIGHT] = Nplates;   /* init left plate */
   for (i=0; i<pwin->wsize; i++)
        pwin->_wbuff[i] = (pwin->paper|pwin->ink)<<8;
   for (i=0;i<Nplates; i++)
        drawp(pwin,
              pol[1][POLE_XCOOR]-(Nplates-i-1),
              pwin->_maxy-(2+i),
              2 * (Nplates-i) - 1,
              1);
   /* perform a global refresh */
   m_touchwin(pwin);

   /* start algorithm */
   hanoi(pwin, Nplates, &pol[1], &pol[3], &pol[2]);
}

/*-----------
 * move_vert
 *-----------
 */
move_vert(pwin,pol,direction,size)
struct window *pwin;
int           *pol;
{
   int j,ps;
   if (direction == MOVEUP)
       for (j=pwin->_maxy -(1+pol[POLE_HIGHT]); j>2 ;j--) {
            /*ps = m_Disable();*/
            drawp(pwin,pol[POLE_XCOOR]-(size-1),j,2*size-1,0);
            drawp(pwin,pol[POLE_XCOOR]-(size-1),j-1,2*size-1,1);
            m_Sleep(0);
            /*m_Restore(ps);*/
       }
   else
       for (j=2; j<pwin->_maxy -(2+pol[POLE_HIGHT]); j++) {
            /*ps = m_Disable(); */
            drawp(pwin,pol[POLE_XCOOR]-(size-1),j,2*size-1,0);
            drawp(pwin,pol[POLE_XCOOR]-(size-1),j+1,2*size-1,1);
            m_Sleep(0);
            /*m_Restore(ps);*/
       }
}

/*--------------
 * move_horizon
 *--------------
 */
move_horizon(pwin,polS,polT,size)
struct window *pwin;
int           *polS,*polT;
{
   int i,ps;
   if (polS[POLE_XCOOR] < polT[POLE_XCOOR]) /* left to right */
       for (i=polS[POLE_XCOOR]-(size-1);
            i<polT[POLE_XCOOR]-(size-1) ;i++) {
            /*ps = m_Disable();*/
            drawp(pwin,i,2,2*size-1,0);
            drawp(pwin,i+1,2,2*size-1,1);
            m_Sleep(0);
            /*m_Restore(ps);*/
       }
   else
       for (i=polS[POLE_XCOOR]-(size-1);
            i>polT[POLE_XCOOR]-(size-1) ;i--) {
            /*ps = m_Disable();*/
            drawp(pwin,i,2,2*size-1,0);
            drawp(pwin,i-1,2,2*size-1,1);
            m_Sleep(0);
            /*m_Restore(ps);*/
       }
}

/*-------
 * drawp
 *-------
 */
drawp(pwin,x,y,size,visible)
struct window *pwin;
{
   int i,savink;
   m_wmove(pwin,y,x);
   savink = pwin->ink;
   if (!visible)
       pwin->ink = (pwin->paper>>4)&0x7f;

   for (i=0; i<size; i++)
        if (m_waddch(pwin, 219) == -1)  /* semi graphic character */
                m_Exit(-1);
   pwin->ink = savink;
   m_wrefresh(pwin);
}

/*------
 * loop
 *------
 */
loop(pwin)
struct window *pwin;
{
   pwin->_flags |= W_GLOBALR;
   while(TRUE) {
          m_wprintw(pwin, "Time sharing Demo");
          m_wrefresh(pwin);
          m_Sleep(0);
    }
}

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Message >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

int receiverPID;

/*---------------------------------------------------------------------
 *  Send messages after 5 seconds - the receiver must be started before
 *  the sender
 *---------------------------------------------------------------------
 */
sender()
{

   /* attach new session to this process */
   if (m_GetSession() == -1) {
        m_Printf("Session creation fails...\n");
        m_Exit(-1);
   }

    while(TRUE) { /* send a message after 5 seconds */
      m_Sleep(5);
      m_Printf("SEND message to receiver\n");
      m_Msgsync(receiverPID);      /* wake up receiver */
    }
}

/*------------------------------------
 *  wait for a message with time out
 *------------------------------------
 */
receiver()
{
   int stat;
   unsigned char mess;

   /* attach new session to this process */
   if (m_GetSession() == -1) {
        m_Printf("Erreur Session...\n");
        m_Exit(-1);
   }
   /* init global identification variable */
   receiverPID = m_Getpid();

   while (TRUE)   /* wait for a message during 2 seconds */
      if (m_Msgwait(&mess, 2, &stat)){
          m_Printf("NO MESSAGE received after 2 seconds\n");
      }
      else
          m_Printf("MESSAGE received\n");
}

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Signal >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

int      nbsec;

/*---------------------------
 * sigmain - wait for signals
 *---------------------------
 */
sigmain()
{
   void cntlC(), itsec();

   nbsec = 0;
   m_Printf("Type ^C  at any time...\n");

   /* init signals */
   m_Signal(SIGALRM,itsec);
   m_Signal(SIGINT,cntlC);
   m_Alarm(1);

   /* wait for signals */
   while (TRUE)
        m_Pause();
}

/*---------------------------
 * itsec - SIGALRM action
 *---------------------------
 */
void itsec()
{
  /* restart this action */
  m_Signal(SIGALRM,itsec);
  m_Alarm(1);
  nbsec++;
}

/*---------------------------
 * cntlC - SIGINT action
 *---------------------------
 */
void cntlC()
{
  m_Printf("\n%d seconds before ^C\n",nbsec);
  m_Exit(0);
}

