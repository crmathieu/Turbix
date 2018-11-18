/* command.c */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "mem.h"
#include "signal.h"
#include "floppy.h"
#include "console.h"
#include "shell.h"
#include "fs\file.h"
#include "const.h"
#include "stat.h"
#include "fs\fcache.h"
#include "fs\fsopen.h"
#include "setjmp.h"

jmp_buf env;

int pidsig,cign;
int pidtach;
int nbsec = 0;
char string[] = "hello";
int  tReserv[25];
int  initR = FALSE;
int semres;

chrono()
{
    int fd;  int ps,i,n;
    int horl();
    long tloc;

    fd = getvs();
    close(stdout);
    dup(fd);
    close(fd);
    cursorShape(CURS_HIDE);

    if (fork() == 0) {
        while ( 1)
        {
            time(&tloc);
            ps = disable();
            gotoxy(10,10);
            printf("time = %lX\n",tloc);
            restore(ps);
        }
    }
    while (1)
    {
       signal(SIGALRM,horl);
       alarm(1);
       pause();
    };
}

emett(nargs,args)
int nargs;
char *args[];
{
    int fdt,fds,fd;  long i,n;
    Bool bonnard;
    char buff[512];

    fd = getvs();
    close(stdout);
    dup(fd);
    close(fd);

    while(TRUE)
    {
      sleep(5);
      printf("\nEMETTEUR : j'envoie un message au RECEPTEUR\n");
      msgsnd(pidtach,MSGSYNC);
    }

}

recep()
{
   int fd;
   long i = 0;
   fd = getvs();
   close(stdout);
   dup(fd);
   close(fd);
   pidtach = RUNpid;

   while (1)
   {
      if (tmsgrcv(2) == TIMOVER)  printf("\nRECEPTEUR: timout tombe\n");
      else                         printf("\nRECEPTEUR: message recu\n");

   }
}
/*---------------------------------------*/
sigctrlc(argc,argv)
int argc;
char *argv[];
{
   int status,fd,i,j,pid;
   long k;

/*
 *  ESSAI signal SIGINT
 */

   int cntlC();
   int itsec();

   signal(SIGALRM,itsec);
   signal(SIGINT,cntlC);
   alarm(1);
   while (1)       pause();
}
itsec()
{
  signal(SIGALRM,itsec);
  alarm(1);
  nbsec++;
}
cntlC()
{
  printf("\nattente %d secondes\n",nbsec);
  Xexit(0);
}
/*--------------------------------------------*/

horl()
{
   int ps;
   static int hours = 0;
   static int  min  = 0;
   static int  sec  = 0;
   if (++sec >= 60) {
       sec = 0;
       if (++min>=60) {
           min = 0;
           if (++hours >= 24) hours = 0;
       }
   }
   ps = disable();
   gotoxy(40,10);
   printf("%02d:%02d:%02d\n",hours,min,sec);
   restore(ps);
}

divPar0()
{
   printf("\nje me suis trompe dans la division\n");
   Xexit(-1);
}


siguser()
{
   int ps;
   ps = disable();
   c_ps();
   signal(SIGUSR1,siguser);
   restore(ps);
}

ignoredoc(argc,argv)
int argc;
char *argv[];
{
   int ps;
   char c;

   ps = disable();
   if (initR == FALSE) {
       semres = creatsem(1,"RESERV");
       initR = TRUE;
   }
   restore(ps);
   while (1)
   {
      printf("\nINTERRO : 0  MODIF : 1\n");
      if ((c = getch()) == '0')
           interr();
      else
           if (c == '1')   reserv();
           else break;
   }
}

interr()
{
   int i = 0;
   for(i=0;i<25;i++)
       if (tReserv[i] == 0)  printf("%d ",i);
   putchar('\n');
}

reserv()
{  int iii;
   int c,pid,ps;
   int sigexit();

   waitsem(semres);
   signal(SIGALRM,sigexit);
   alarm(6);
   interr();
   printf("\nvotre choix\n");
   cign = 0;
   if ((pid = fork()) == 0) {
       cign = getch();
       Xexit(0);
   }
   pidsig = pid;
   wait(&iii);
   if (cign != 0)    tReserv[cign - 0x30] = 1;
   sigsem(semres);
}

sigexit()
{
    printf("\nVOUS BLOQUEZ TOUT LE MONDE : COUIK !\n");
    kill(pidsig,SIGKILL);
}

tach2(argc,argv)
int argc;
char argv[];
{
  if (setjmp(env) != 0) {
      printf("\nAPRES LONGJMP\n");
      Xexit(0);
  }
  printf("\nAPRES SETJMP\n");
  tach4();
}

tach3()
{
   printf("\nAVANT LONGJMP\n");
   longjmp(env,1);
}

tach4()
{
   tach3();
}

/*----------------------------------------------------------------------
 * c_cls - clear screen
 *----------------------------------------------------------------------
 */
SHELLCOM c_cls()
{
    putchar('\x0c');
}

/*----------------------------------------------------------------------
 * c_voir - lister le contenu d'un fichier
 *----------------------------------------------------------------------
 */
SHELLCOM c_voir( argc, argv)
int   argc;
char *argv[];
{
     uchar buff[512];
     int n,i,pid;
     int fd;
     int voirexit();

     if (argc != 2)
     {
         printf("\nuse : voir [file_name]\n");
         return;
     }
        signal(SIGINT,voirexit);
        putchar('\n');
        fd = open(argv[1],0);
        while ((n = read(fd,buff,512)) > 0)
        {
           for(i=0;i<n;)
           {
                putchar(buff[i]);
                buff[i++] = 0;
           }
        }
        close(fd);
}
voirexit()
{
  kprintf("\nTerminate by user : exit\n");
  Xexit(0);
}

/*----------------------------------------------------------------------
 * c_copy  -  recopier un fichier  dans un autre
 *----------------------------------------------------------------------
 */
SHELLCOM c_copy( argc, argv)
int   argc;
char *argv[];
{
     uchar buff[512];
     int n,i;
     int fdsource,fdtarget;

     if (argc != 3)
     {
         printf("\nuse : copy [sourcefile] [targetfile]\n");
         return;
     }
    putchar('\n');
    if ((fdsource = open(argv[1],0)) == -1) return;
    if ((fdtarget = creat(argv[2],C_WAL)) == -1) return;
    while ((n = read(fdsource,buff,512)) == 512)
    {
/*      for (i=0;i<n;i++)  putchar(buff[i]);*/
      write(fdtarget,buff,512);
    }
/*    for (i=0;i<n;i++)  putchar(buff[i]);      */
    write(fdtarget,buff,n);

    close(fdsource);
    close(fdtarget);
}


/*----------------------------------------------------------------------
 * c_sleep -
 *----------------------------------------------------------------------
 */
SHELLCOM c_sleep( argc, argv)
int   argc;
char *argv[];
{
    int ps;
    if (argc != 2) {
         printf( "usage: sleep delay\n");
         return(RERR);
    }
    sleep(atoi(argv[1]));
    return(ROK);
}

/*----------------------------------------------------------------------
 * c_video - modifier l'attribut video courant
 *----------------------------------------------------------------------
 */
SHELLCOM c_video( argc, argv)
int   argc;
char *argv[];
{
   int i;
     if (argc != 2)
     {
        set_cva(WONB);
        printf("\n 0 ............ normal mode\n");
        set_cva(SUR);
        printf("\n 1 ............ surintensity mode\n");
        set_cva(REVERSE);
        printf("\n 2 ............ reverse video mode\n");
        set_cva(BLINK);
        printf("\n 3 ............ blinking mode\n");
        set_cva(BLINKREV);
        printf("\n 4 ............ blinking in reverse video mode\n");
        set_cva(UWONB);
        printf("\n 5 ............ for normal underligne\n");
        set_cva(USUR);
        printf("\n 6 ............ surintensity underligne\n");
        set_cva(WONB);
        return;
     }
     switch(atoi(argv[1]))
     {
        case 0 : set_cva(WONB);break;
        case 1 : set_cva(SUR);break;
        case 2 : set_cva(REVERSE);break;
        case 3 : set_cva(BLINK);break;
        case 4 : set_cva(BLINKREV);break;
        case 5 : set_cva(UWONB);break;
        case 6 : set_cva(USUR);break;
        default: set_cva(WONB);
     }

}

toto()
{
  int vs1,vs2,vs3;
  char *buff;
  if ((vs1 = getvs("vs1")) != -1)
  if (fork() == 0)
     while(1) write(vs1,buff = "je suis sur vs1\n",strlen(buff));
  else if ((vs2 = getvs("vs2")) != -1)
       if (fork() == 0) while(1) write(vs2,buff = "je suis sur vs2\n",strlen(buff));
  if ((vs3 = getvs("vs3")) != -1) while(1)
                                  write(vs3,buff = "je suis sur vs3\n",strlen(buff));
}

pipex()
{
  char buffr[64],buffw[64];
  int fd2,i,pid,n;
  int fdp[2];
  int fd1;
  pipe(fdp);    /* fdp[0] pour lire , fdp[1] pour ecrire */
  if (fork() == 0)
  {
      if ((fd2 = creat("a:totomaj.c",C_WAL)) == -1) {
           printf("\nERROR CREAT\n");
           Xexit(-1);
      }
      close(fdp[1]);
      while ((n = read(fdp[0],buffr,64)) >0 ) {
              write(fd2,buffr,n);
/*              i = 0;
              while (i<n) putchar(buffr[i++]) ;*/
    }
    close(fd2);
    kprintf("\nFILS : TERMINE\n");
  }
  else   {
         fd1 = open("c:/tc/xenius/command.c",0);
         close(fdp[0]);
         while ((n = read(fd1,buffw,64)) > 0)
         {
           for (i = 0 ; i < 64 ; i++)
               if ((buffw[i] <= 0x7A) && (buffw[i] >= 0x61))
                  buffw[i] -= 0x20;
           write(fdp[1],buffw,n);
         }
         kprintf("\nPERE : TERMINE\n");
         }
}

toto2()
{
}

/*-------------------------------
 * utilisation des PIPES
 *-------------------------------
 */
tutupipe()
{
  char buffr[64],buffw[64];
  int fd2,i,n;
  int fdp[2];
  int fd1 = open("c:/tc/xenius/command.c",O_RAL);
  pipe(fdp);
  if (fork() == 0) {
       if ((fd2 = creat("a:majuscul.c",C_WAL)) == -1) {
            printf("\nERROR CREAT\n");
            Xexit(-1);
       }
       close(fdp[1]);
       while ((n = read(fdp[0],buffr,64)) >0 )
               write(fd2,buffr,n);
       kprintf("\nFILS TERMINE\n");
  }
  else {
       close(fdp[0]);
       while ((n = read(fd1,buffw,64)) > 0) {
               for (i = 0 ; i < 64 ; i++)
                    if ((buffw[i] <= 0x7A) && (buffw[i] >= 0x61))
                         buffw[i] -= 0x20;
               write(fdp[1],buffw,n);
       }
       kprintf("\nPERE TERMINE\n");
  }
}

