/* wdemo.c */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "signal.h"
#include "console.h"
#include "shell.h"

#include "const.h"


extern unsigned *Pvideo;

/* variables HANOI */
#define  ABSCISSE_POLE 1
#define  HAUTEUR_POLE  0
#define  MOVEUP        1
#define  MOVEDWN       0

#define S_RIGHT   1
#define S_LEFT    2
#define S_UP      4
#define S_DWN     8

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * creatWindow -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
struct window *creatWindow(name,ink,paper,x,y,maxx,maxy)
char *name;
{
   struct window *pwin,*newwin();
   int i,ps;
   pwin = newwin(maxy,maxx,y,x,ink,paper,name);
   wpush(pwin);
   box(pwin,'º','Í');
   touchwin(pwin);
   return(pwin);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wdemo -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
wdemo(argc,argv)
int argc;
char *argv[];
{
int wshell(), boucle();
int  fils[5];
int sens[5];
struct window *pswin[5];
   int no_fils = 0;
   char c;
   char *P[2];
   int pid,n,i,g,X,Y,J;
   struct window *pwin;

              initscr(F_LCYAN,B_BLACK);
              pswin[1] = creatWindow("",F_LRED,B_BLUE,0,0,44,10);
              sens[1]  = S_RIGHT;
              pswin[2] = creatWindow("",F_LYELLOW,B_MAGENTA,38,13,40,10);
              sens[2]  = S_LEFT;
              pswin[3] = creatWindow("",F_LWHITE,B_GREEN,0,11,56,9); /*10);*/
              sens[3]  = S_RIGHT|S_UP;
              pswin[4] = creatWindow("",F_LWHITE,B_BLACK,40,0,36,11);
              sens[4]  = S_DWN;
              if ((pid =fork()) == 0) {
                  while (1) starthanoi(pswin[1]);
              }
              fils[1] = pid;
              if ((pid = fork()) == 0) {
                   while (1) startHanoi(pswin[2]);
              }
              fils[2] = pid;
              if ((pid = fork()) == 0) {
                   wautocrlf(pswin[3],TRUE);
                   wattach(pswin[3], boucle); /*boucle(pswin[3]);*/
              }
              fils[3] = pid;
              if ((pid = fork()) == 0) {
                   while (1) starthanoi(pswin[4]);
              }
              fils[4] = pid;
              if ((pid = fork()) == 0) {
                   while (1) starthanoi(stdscr);
              }
              wselect(pswin[4]);

              /* code du pere */
                  J = i = 0;
                  while (1) {
                     sleep(3);
                     if (++J < 2)  sleep(0);
                     else
                     {
                         J = 0;
                         stdscr->_flags |= W_GLOBALR;
                         wrefresh(stdscr);
                         for (i = 0; i < 5; i++) {
                                X = pswin[i]->_begx;
                                sleep(0);
                                if (sens[i] & S_RIGHT) {
                                        if (pswin[i]->_begx+pswin[i]->_maxx+2 < 80)
                                                X = pswin[i]->_begx+1;
                                        else {
                                                sens[i] &= ~S_RIGHT;
                                                sens[i] |=  S_LEFT;
                                        }
                                }
                                sleep(0);
                                if (sens[i] & S_LEFT) {
                                        if (pswin[i]->_begx > 0)
                                                X = pswin[i]->_begx-1;
                                        else {
                                                sens[i] &= ~S_LEFT;
                                                sens[i] |=  S_RIGHT;
                                        }
                                }
                                sleep(0);
                                Y = pswin[i]->_begy;
                                if (sens[i] & S_UP) {
                                        if (pswin[i]->_begy > 0)
                                                Y = pswin[i]->_begy-1;
                                        else {
                                                sens[i] &= ~S_UP;
                                                sens[i] |=  S_DWN;
                                        }
                                }
                                sleep(0);
                                if (sens[i] & S_DWN) {
                                        if (pswin[i]->_begy+pswin[i]->_maxy+2 < 25)
                                                Y = pswin[i]->_begy+1;
                                        else {
                                                sens[i] &= ~S_DWN;
                                                sens[i] |=  S_UP;
                                        }
                                }
                                sleep(0);
                                if (mvwin(pswin[i],Y,X) == RERR)
                                        m_Exit(-1);
                         }
                     }
                  }

}

wshell()
{
   extern int shm_();
   execl(shm_,"sh","-i",NULLPTR);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * wattach -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
wattach(pwin, func)
struct window *pwin;
int (* func)();
{
   int fd;
   char c,ch;
   int j,k;
   char *ptr;
   int shm_();
   int pp[2],pp1[2],n,i,ps;
   char buff[80];

    pipe(pp);
    if (fork() == 0) {
            pipe(pp1);
            if (fork() == 0) {  /* lire clavier et ecrire ds PIPE PP1 */
                close(pp[0]);
                close(pp[1]);
                close(pp1[0]);
                close(1);
                dup(pp1[1]);
                close(pp1[1]);
                pwin->_flags |= W_ECHO;
                while (1)  {
                      i = 0;
                      do
                          buff[i++] = (char)wgetch(pwin);

                      while (buff[i-1] != '\n');
                      write(1,buff,i);
                }
            }
            /* lire PIPE PP1 et ecrire dans PIPE PP */
            close(pp[0]);
            close(1);
            dup(pp[1]);
            close(pp[1]);

            close(pp1[1]);
            close(0);
            dup(pp1[0]);
            close(pp1[0]);

            execl(func,"wcommand", NULLPTR);
            shm_(2,"shell","-i", NULL);
    }
    /* lecture depuis PP et ecriture dans fenetre */
    close(0);
    dup(pp[0]);
    close(pp[0]);
    close(pp[1]);
    while (1) {
           read(0,buff,1);
           waddch(pwin,buff[0]);
    }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * hanoi -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
hanoi(pwin,n,a,b,c)
struct window *pwin;
int *a,*b,*c;
{
   if (n>0)  {
       hanoi(pwin,n-1,a,c,b);
       deplacer(pwin,a,b,n);  /* n : taille du plateau : 2n-1 caracteres */
       hanoi(pwin,n-1,c,b,a);
   }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * deplacer -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
deplacer(pwin,polS,polD,taille)
struct window *pwin;
int *polS,*polD; /* piquets source & destination */
{
    move_vert(pwin,polS,MOVEUP,taille);
    move_horizon(pwin,polS,polD,taille);
    move_vert(pwin,polD,MOVEDWN,taille);
    polS[HAUTEUR_POLE]--;
    polD[HAUTEUR_POLE]++;
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * starthanoi -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
starthanoi(pwin)
struct window *pwin;
{
   int pol[4][2];   /* 3 piquets ,chacun indiquant hauteur courante
                        * et abscisse (le 0 est inutilise)
                        */
   int BasePlateau,Nplateaux,i;

   BasePlateau = (pwin->_maxx-4)/3; /* taille fenetre doit etre multiple de 4 */
   Nplateaux = min((BasePlateau + 1)/2,pwin->_maxy-3);
   for (i=1;i<4; i++) {
        pol[i][HAUTEUR_POLE]  = 0;
        pol[i][ABSCISSE_POLE] = (i-1)*BasePlateau + i + BasePlateau/2;
   }
   pol[1][HAUTEUR_POLE] = Nplateaux;   /* init plateau de gauche */
   for (i=0; i<pwin->wsize; i++)
        pwin->_wbuff[i] = (pwin->paper|pwin->ink)<<8;
   for (i=0;i<Nplateaux; i++)
        drawp(pwin,
              pol[1][ABSCISSE_POLE]-(Nplateaux-i-1),
              pwin->_maxy-(2+i),
              2 * (Nplateaux-i) - 1,
              1);
   pwin->_flags |= W_GLOBALR;
   wrefresh(pwin);
   hanoi(pwin,Nplateaux,&pol[1],&pol[3],&pol[2]);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * move_vert -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
move_vert(pwin,pol,sens,taille)
struct window *pwin;
int           *pol;
{
   int j,ps;
   if (sens == MOVEUP)
       for (j=pwin->_maxy -(1+pol[HAUTEUR_POLE]); j>2 ;j--) {
            ps = _disable();
            drawp(pwin,pol[ABSCISSE_POLE]-(taille-1),j,2*taille-1,0);
            drawp(pwin,pol[ABSCISSE_POLE]-(taille-1),j-1,2*taille-1,1);
            sleep(0);
            _restore(ps);
/*            sleep(2);*/
       }
   else
       for (j=2; j<pwin->_maxy -(2+pol[HAUTEUR_POLE]); j++) {
            ps = _disable();
            drawp(pwin,pol[ABSCISSE_POLE]-(taille-1),j,2*taille-1,0);
            drawp(pwin,pol[ABSCISSE_POLE]-(taille-1),j+1,2*taille-1,1);
            sleep(0);
            _restore(ps);
/*            sleep(2);*/
       }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * move_horizon -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
move_horizon(pwin,polS,polD,taille)
struct window *pwin;
int           *polS,*polD;
{
   int i,ps;
   if (polS[ABSCISSE_POLE] < polD[ABSCISSE_POLE]) /* gauche vers droite */
       for (i=polS[ABSCISSE_POLE]-(taille-1);
            i<polD[ABSCISSE_POLE]-(taille-1) ;i++) {
            ps = _disable();
            drawp(pwin,i,2,2*taille-1,0);
            drawp(pwin,i+1,2,2*taille-1,1);
            sleep(0);
            _restore(ps);
/*            sleep(2);*/
       }
   else
       for (i=polS[ABSCISSE_POLE]-(taille-1);
            i>polD[ABSCISSE_POLE]-(taille-1) ;i--) {
            ps = _disable();
            drawp(pwin,i,2,2*taille-1,0);
            drawp(pwin,i-1,2,2*taille-1,1);
            sleep(0);
            _restore(ps);
/*            sleep(2);*/
       }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * drawp -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
drawp(pwin,x,y,taille,visible)
struct window *pwin;
{
   int i,savink;
   wmove(pwin,y,x);
   savink = pwin->ink;
   if (!visible)
       pwin->ink = (pwin->paper>>4)&0x7f;

   for (i=0; i<taille; i++)    if (waddch(pwin,219) == -1) m_Exit(-1);
   pwin->ink = savink;
   wrefresh(pwin);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * boucle -
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
boucle(pwin)
struct window *pwin;
{
   pwin->_flags |= W_GLOBALR;
    while(TRUE) {
          printf("Dos Multitasking Kernel ");
          sleep(0);
    }
}
