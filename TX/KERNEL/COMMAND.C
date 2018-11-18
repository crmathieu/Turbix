/* command.c : Exemples de programmation multitache */


#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "signal.h"
#include "console.h"
#include "shell.h"

#include "const.h"
#include "stat.h"

#include "setjmp.h"
#include "ipc.h"
#include "msg.h"
#include "fsopen.h"

int      pidsig;
int      pidtach;
int      semres;
int      nbsec;
int      tReserv[25];
jmp_buf  jmpstack;
long cxtswitch = 0;



/*-----------------------------------------------
 * c_copy  -  recopier un fichier  dans un autre
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
     while ((n = m_Read(fdsource,buff,512)) > 0)
             m_Write(fdtarget,buff,n);

     m_Close(fdsource);
     m_Close(fdtarget);
}


/*-----------------------------------------
 * c_type - lister le contenu d'un fichier
 *-----------------------------------------
 */
c_type( argc, argv)
int   argc;
char *argv[];
{
     unsigned char buff[512];
     int n,i,pid;
     int fd;
     int voirexit();

     if (argc != 2)
     {
         m_Printf("\nuse : voir [file_name]\n");
         return;
     }
     m_Signal(SIGINT,voirexit);
     putchar('\n');
     if ((fd = m_Open(argv[1],O_RDONLY)) < 0) {
          m_Perror("type");
          m_Exit(-1);
     }
/* m_Lock(fd); */
     while ((n = m_Read(fd,buff,512)) > 0)
        m_Write(1, buff, n);
/*           for(i=0;i<n;)
           {
                putchar(buff[i]);
                buff[i++] = 0;
           }*/
/*     m_Unlock(fd);*/
     m_Close(fd);
}
voirexit()
{
  m_Printf("\nTerminated by user : exit\n");
  m_Exit(0);
}


/*---------------------------------------------
 * utilisation des signaux et des redirections
 *---------------------------------------------
 */
chrono()
{
    int fd,i,n,ps;
    int horl();
    long tloc;
   static int hours = 0;
   static int  minu  = 0;
   static int  sec  = 0;

/*    fd = m_Getvs();
    m_Close(stdout);
    m_Dup(fd);
    m_Close(fd);
    m_CursorShape(CURS_HIDE);*/

    if (m_Fork() == 0) {
        while (1) {
            m_Time(&tloc);
/*            m_Waitsem(tty[Tasktab[m_Getpid()].tfd[1]->s_minor].xsem);*/
/*            m_Lock(1);*/
            ps = m_Disable();
            m_Printf("\033[10;20Htime = %lX",tloc);
/*            m_Unlock(1);*/
            m_Restore(ps);
/*            m_Sigsem(tty[Tasktab[m_Getpid()].tfd[1]->s_minor].xsem);*/
        }
    }
    while (1) {
       if (++sec >= 60) {
           sec = 0;
           if (++minu>=60) {
               minu = 0;
               if (++hours >= 24) hours = 0;
           }
       }
/*       m_Waitsem(tty[Tasktab[m_Getpid()].tfd[1]->s_minor].xsem);*/
/*       m_Lock(1);*/
        ps = m_Disable();
       m_Printf("\033[10;60H%02d:%02d:%02d",hours,minu,sec);
/*       m_Unlock(1);*/
        m_Restore(ps);
       m_Sleep(1);
/*       m_Sigsem(tty[Tasktab[m_Getpid()].tfd[1]->s_minor].xsem);*/
    };
}

horl()
{
   int ps;
   static int hours = 0;
   static int  minu  = 0;
   static int  sec  = 0;
   if (++sec >= 60) {
       sec = 0;
       if (++minu>=60) {
           minu = 0;
           if (++hours >= 24) hours = 0;
       }
   }
/*   m_Gotoxy(40,10);*/
   ps = m_Disable();
   m_Printf("\033[10;60H%02d:%02d:%02d",hours,minu,sec);
   m_Restore(ps);
}

/*-----------------------------------------------------------------------
 *  envoyer un message a une tache toute les 5 secondes
 *-----------------------------------------------------------------------
 */
emett(nargs,args)
int nargs;
char *args[];
{
    int fd;

   if ((fd = m_GetSession()) == -1) {
        m_Printf("Erreur Session...\n");
        m_Exit(-1);
   }

/*    m_Close(stdout);
    m_Dup(fd);
    m_Close(fd);*/

    while(TRUE) {
      m_Sleep(5);
      m_Printf("\nEMETTEUR : j'envoie un message au RECEPTEUR\n");
      m_Msgsync(pidtach);
    }
}

/*------------------------------------
 *  attendre un message avec time out
 *------------------------------------
 */
recep()
{
   int fd, stat;
   uchar mess;

   if ((fd = m_GetSession()) == -1) {
        m_Printf("Erreur Session...\n");
        m_Exit(-1);
   }
/*   m_Close(stdout);
   m_Dup(fd);
   m_Close(fd);*/
   pidtach = m_Getpid();

   while (1)
      if (m_Msgwait(&mess, 2, &stat)){
          m_Printf("RECEPTEUR: timout tombe - grp = %d\n",Tasktab[m_Getpid()].tgrp);
      }
      else
          m_Printf("RECEPTEUR: message recu - grp = %d\n",Tasktab[m_Getpid()].tgrp);
}

/*-------------------------------------------
 * utilisation des signaux SIGINT et SIGALRM
 *-------------------------------------------
 */
sigctrlc(argc,argv)
int argc;
char *argv[];
{
   int cntlC();
   int itsec();

   nbsec = 0;
   m_Printf("Tapez ^C quand vous le voulez ...\n");
   m_Signal(SIGALRM,itsec);
   m_Signal(SIGINT,cntlC);
   m_Alarm(1);
   while (1)       m_Pause();
}

itsec()
{
  m_Signal(SIGALRM,itsec);
  m_Alarm(1);
  nbsec++;
}

cntlC()
{
  m_Printf("\nattente %d secondes\n",nbsec);
  m_Exit(0);
}

/*-----------------------------------------------------------------
 * exemple d'interrogation avec blocage temporaire sur reservation
 *-----------------------------------------------------------------
 */
interro(argc,argv)
int argc;
char *argv[];
{
   int ps;
   char c;
   static int initR = FALSE;

   ps = m_Disable();
   if (initR == FALSE) {
       semres = m_Creatsem(1,"RESERV");
       initR = TRUE;
   }
   m_Restore(ps);
   while (1)
   {
      m_Printf("\nINTERRO : 0  MODIF : 1\n");
      if ((c = m_Getch()) == '0')
           interrogation();
      else
           if (c == '1')   reserv();
           else break;
   }
}


interrogation()
{
   int i = 0;
   for(i=0;i<25;i++)
       if (tReserv[i] == 0)  m_Printf("%d ",i);
   putchar('\n');
}

reserv()
{  int iii;
   int c,pid,ps;
   int sigexit();
   static cign = 0;

   m_Waitsem(semres);
   m_Signal(SIGALRM,sigexit);
   m_Alarm(6);
   interrogation();
   m_Printf("\nvotre choix\n");
   cign = 0;
   if ((pid = m_Fork()) == 0) {
        m_Scanf("%d",&cign);
        m_Exit(0);
   }
   pidsig = pid;
   m_Pause();     /* redemarrer sur SIGCLD ou SIGALRM */
   if (cign != 0)    tReserv[cign - 0x30] = 1;
   m_Sigsem(semres);
}

sigexit()
{
    m_Printf("\nVOUS BLOQUEZ TOUT LE MONDE : COUIK !\n");
    m_Kill(pidsig,SIGKILL);
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

    for (i=0; i<6; i++) {
        if (m_Fork() == 0)
                ecrireDsSession(i+2);
    }
    while (1)
        for (i=0; i<6; i++) {
                m_Sleep(1);
                m_ChgActiveSession(i);
        }
}
ecrireDsSession(delais)
{
        int hs;
        m_GetSession();
        hs = m_GetProcSessionHandle();
        while(1) {
                m_Sleep(delais);
                m_Printf("Session %d\n");
        }
}

/*---------------------------------------------
 * c_video - modifier l'attribut video courant
 *---------------------------------------------
 */
c_video( argc, argv)
int   argc;
char *argv[];
{
    int fd;
    fd = m_GetSession();
/*    m_Close(stdout);
    m_Dup(fd);
    m_Close(fd);*/

    while(TRUE) {
          m_Printf("---------***********++++++++++++\n");
    }

}

/*-------------------------------
 * utilisation setjmp & longjump
 *-------------------------------
 */
setlongjmp()
{
   int z;
   if (m_Setjmp(jmpstack) != 0) {
       m_Printf("\nlongjmp a ete appele\n");
       m_Exit(0);
   }
   m_Printf("\nsetjmp a ete appele\n");
   suite();
}

suite()
{
   int g;
   m_Printf("\navant appel longjmp\n");
   m_Longjmp(jmpstack);
   m_Printf("\npartie du code inaccessible\n");
}


loop()
{/*
  if (m_Fork() == 0)
     while(1);
  else
       if (m_Fork() == 0) while(1) ;
  while (1);*/
    int fd;
    fd = m_GetSession();
/*    m_Close(stdout);
    m_Dup(fd);
    m_Close(fd);*/

    while(TRUE) {
          m_Printf("abcedfghijklmnoppqrstuvwxyz1234567890");
      /*  chgtsk();*/
    }

}

helpexit()
{
/*   char tt[20];
   int i;
   long a;
   char c;
   km_Printf("\nPour sortir, faire CTRL-ALT-DEL ou lancer exit\n");
   m_Scanf("\n entier %d  string %s long %D  char %c\n",&i,tt,&a,&c);
   m_Printf("\nverif : entier = %d - string = %s  long = %ld char = %c\n",
   i,tt,a,c);*/

    int fd;

 /*   fd = m_GetSession();
    m_Close(stdout);
    m_Dup(fd);
    m_Close(fd);
    while (1)  m_Printf("AGAGA ...\n");*/
/*    m_CursorShape(CURS_HIDE);
    m_Getch();
    m_CursorShape(CURS_LARGE);
    m_Getch();
    m_CursorShape(CURS_SMALL);*/
    int i;
    char *p, *m;
    struct hblk *q;

    p = m_Malloc(320);
    m = 0x12345678;
    strcpy(p, "toto est beau");
    m_Getch();
    if (m_Fork() == 0) {
        m_Printf("AVANT P = %lx\n",p);
        if (m_AdjustPTR(&p) == RERR)
            m_Printf("ERROR .........\n");
        if (m_AdjustPTR(&m) == RERR)
            m_Printf("ERROR M.........\n");

        strcat(p, " dans fils\n");
        m_Printf("FILS ptr = %lx  str dup = %s\n",p, p);
        m_Printf("APRES P = %lx\n",p);
        m_Exit(0);
    }
    m_Wait(&i);
    m_Printf("PERE ptr = %lx str = %s\n",p,p);

/*    for (i=0, q = Tasktab[m_Getpid()].theadblk; q != NULL; i++) {
        m_Printf("bloc %d - ADRESSE %lx\n", i, q);
        q = q->nextBlk;
    }*/


}
newsession()
{
  int m_Spy();

  char *argv[4];

  argv[1] = "-i";
  argv[2] = NULLPTR;
  if (m_Fork() == 0)
  {
     m_GetSession();

     argv[0] = "sh";
     m_Execv(m_Spy,argv);
     m_Printf("\nExec Fails in TTY1\n");
  }
}

struct msbuf {
       long mtype;
       char mtext[32];
};

ipc()
{
  int qid,i;
  char c;
  struct msbuf mb;
  char message[] = "message du pere au fils";

  if ((qid = m_Msgget(IPC_PRIVATE, MSG_NOERROR)) == -1) {
       m_Printf("IPC : ERROR\n");
       m_Exit(0);
  }
  if (m_Fork() == 0)
  {
     if (m_Msgrcv(qid, &mb, strlen(message), 12L, MSG_NOERROR) == -1) {
         m_Printf("Fils: RCV ERROR\n");
         m_Exit(-1);
     }
     m_Printf("FILS : Message - %s - recu... PRESS\n", mb.mtext);
     m_Getch();
     if (m_Exit(0)) m_Printf("FILS ERR EXIT ................\n");
  }
  m_Printf("PERE : J'envoie le message des qu'on appuie sur une touche\n");
  m_Read(0, &c, 1);
  mb.mtype = 12L;
  strcpy(mb.mtext, message);
  if (m_Msgsnd(qid, &mb, strlen(message), 0) == -1)
      m_Printf("Pere : SND ERROR\n");
  m_Wait(&i);
  m_Exit(0);
}
ipc2()
{
  int qid,i;
  char c;
  struct msbuf mb;
  char message[] = "message du pere au fils";

  if ((qid = m_Msgget(IPC_PRIVATE, MSG_NOERROR)) == -1) {
       m_Printf("IPC : ERROR\n");
       m_Exit(0);
  }
  if (m_Fork() == 0)
  {
     if (m_Msgrcv(qid, &mb, strlen(message), 12L, MSG_NOERROR) == -1) {
         m_Printf("Fils: RCV ERROR\n");
         m_Exit(-1);
     }
     m_Printf("FILS : Message - %s - recu... PRESS\n", mb.mtext);
     m_Read(0, &c, 1);
     if (m_Exit(0)) m_Printf("FILS ERR EXIT ................\n");
  }
  m_Printf("PERE : Je Supprime la queue ...\n");
  m_Getch();
  if (m_Msgctl(qid, IPC_RMID, 0L) == RERR)
      m_Printf("Pere : CTL ERROR\n");
  m_Wait(&i);
  m_Exit(0);
}
ipc3()
{
  int qid,i;
  struct msbuf mb;
  char c;
  char message[] = "message du pere au fils";

  if ((qid = m_Msgget(IPC_PRIVATE, MSG_NOERROR)) == -1) {
       m_Printf("IPC : ERROR\n");
       m_Exit(0);
  }
  if (m_Fork() == 0)
  {
     if (m_Msgrcv(qid, &mb, strlen(message) - 6, 12L, MSG_NOERROR) == -1) {
         m_Printf("Fils: RCV ERROR\n");
         m_Exit(-1);
     }
     m_Printf("FILS : Message - %s - recu... PRESS\n", mb.mtext);
     m_Getch();
     if (m_Exit(0)) m_Printf("FILS ERR EXIT ................\n");
  }
  m_Printf("PERE : J'envoie le message des qu'on appuie sur une touche\n");
  m_Read(0, &c, 1);
  mb.mtype = 12L;
  strcpy(mb.mtext, message);
  if (m_Msgsnd(qid, &mb, strlen(message), 0) == -1)
      m_Printf("Pere : SND ERROR\n");
  m_Wait(&i);
  m_Exit(0);
}

essai()
{
/*    int c,x,y,i;
    i = 0;
    while (i++<10) {
     m_Getpos(&x,&y);
     switch((c = m_Getch())>>8) {
     case 72 : m_Gotoxy(x,y-1);  /* UP */
/*               break;
     case 80 : m_Gotoxy(x,y+1); /* DWN */
/*               break;
    }
    }
*/
   long x1,x2;
   int fils1, fils2;
   if ((fils1 = m_Fork()) == 0) {
       m_Nice(1);
       while (1) {
         cxtswitch++;
         m_Sleep(0);
       }
   }
   if ((fils2 = m_Fork()) == 0) {
       m_Nice(1);
       while (1) {
         cxtswitch++;
         m_Sleep(0);
       }
   }
   x1 = cxtswitch;
   m_Sleep(1);
   x2 = cxtswitch;
   m_Printf("# context switch : %d\n",x2 - x1);
   m_Kill(fils1, SIGKILL);
   m_Kill(fils2, SIGKILL);
}

/* definition de constantes utilisateur */
#define  LEFT_BUTTON_DOWN   1
#define  RIGHT_BUTTON_DOWN  2
#define  CENTER_BUTTON_DOWN 4
#define  MOUSE_MOVE         8

#define  M_MOVE            1
#define  M_LEFT_PRESS      2
#define  M_LEFT_RELEASE    4
#define  M_RIGHT_PRESS     8
#define  M_RIGHT_RELEASE  16
#define  M_MIDDL_PRESS    32
#define  M_MIDDL_RELEASE  64


mouOn()
{
/*        struct mouEvent Mou;

                   m_OpenMou();
                   m_ShowMou();

                   /* utiliser uniquement le bouton GAUCHE */
/*                   m_EventMaskMou(M_MOVE|M_LEFT_PRESS|M_LEFT_RELEASE);

                   /* Lire les evŠnements associ‚s au bouton GAUCHE */
/*                   while (1) {
                        if (m_ReadEventMou(&Mou, TRUE) < 0) {
                                continue;
                        }
                        m_HideMou();
                        if (Mou.event & MOUSE_MOVE)
                                m_Printf("MOVE...");
                        else
                                if (Mou.event & LEFT_BUTTON_DOWN)
                                        m_Printf("DOWN_L...");
                                else
                                        m_Printf("%x...",Mou.event);
                        m_ShowMou();
                   }*/
}
mouOff()
{
/*        m_HideMou();
        m_CloseMou();*/
}
