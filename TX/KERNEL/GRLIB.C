/* grlib.c */

/* librairie graphique */

#define limit(x,y)   ((x >= 0) && (x <= 719) && (y >= 0) && (y <= 347))


extern  unsigned char medium[256][10];
/*extern  unsigned char large[256][14];
extern  unsigned char small[95][6];*/

#include "sys.h"
#include "conf.h"
#include "tty.h"
#include "sem.h"
#include "shell.h"
#include "io.h"
#include "console.h"
#include "const.h"

extern int sysdown;
extern unsigned vid_retrace;
extern unsigned vid_base;
extern unsigned vid_mask;
extern unsigned vid_port;
int xgr = 0;
int ygr = 0;

/*---------------------------------------------------------------------------
 * putc_large - ecrire un caractere defini dans le generateur en 8x14
 *---------------------------------------------------------------------------
 *//*
putc_large(iptr)
struct tty *iptr;
{
  int i,j;
  int nrbyte = atoi(iptr->Obuff[iptr->Oout++]);
  xgr = iptr->cursX ;
  ygr = iptr->cursY * 14;

  for( j = 0; j<14 ; j++)
      mwbyte(xgr,ygr+j,large[nrbyte][j]);
}    */

/*---------------------------------------------------------------------------
 * putc_small - ecrire un caractere defini dans le generateur en 4x6
 *---------------------------------------------------------------------------
 *//*
putc_small(iptr)
struct tty *iptr;
{
  int i,j;
  int nrbyte = atoi(iptr->Obuff[iptr->Oout++]);
  xgr = iptr->cursX ;
  ygr = iptr->cursY * 6;

  for( j = 0; j<6 ; j++)
      mwbyte(xgr,ygr+j,small[nrbyte][j]);

}    */

/*---------------------------------------------------------------------------
 * putc_medium - ecrire un caractere defini dans le generateur en  8x8
 *---------------------------------------------------------------------------
 */
putc_medium(iptr)
struct tty *iptr;
{
  int i,j;
  int nrbyte =(iptr->Obuff[iptr->Oout++])&0xff;
  xgr = iptr->cursX++;
  ygr = iptr->cursY * 10;

  for( j = 0; j<10 ; j++)
      mwbyte(xgr,ygr+j,medium[nrbyte][j]);

  if (iptr->cursX >= LINE_WIDTH)
  {
      iptr->cursY++;
      iptr->cursX = 0;
  }

}

/*---------------------------------------------------------------------------
 *  Gflush - transfert char to ramscreen
 *---------------------------------------------------------------------------
 */
Gflush(iptr)
struct tty *iptr;
{
    int ps,count;

    ps = disable();
    if (iptr->Oin == 0)
    {
        restore(ps);
        return;
    }
/*    iptr->cursX += iptr->Oin;
    while(iptr->cursX >= LINE_WIDTH)
    {
         iptr->cursY++;
         iptr->cursX -= LINE_WIDTH;
    } */
    if ((count = iptr->Oout - iptr->Oin) < 0)
    {
/*       switch(iptr->cvatt)
       {
          case 0 : while (count++ < 0)
                         putc_large(iptr);break;
          case 1 : while (count++ < 0)
                         putc_medium(iptr);break;
          case 2 : while (count++ < 0)
                         putc_small(iptr);break;
       }*/
      while (count++ < 0)
            putc_medium(iptr);
    }

    iptr->Oin = 0;
    iptr->Oout = 0;
    restore(ps);
}


/*                           PROCEDURES GRAPHIQUES
                             ---------------------
 */


/*---------------------------------------------------------------------------
 * draw - afficher un segment quelconque
 *---------------------------------------------------------------------------
 */
draw(x1,y1,x2,y2,value)
int x1,y1,x2,y2,value;
{
   int   dx,dy,d,incr1,incr2,x,y,xend;
   if (x1 == x2)     /* segment vertical */
   {
       /* controler les debordements possibles */
       if ((x1 > 719) || (x1 < 0)) return;

       if (y1 > 347)      y1 = 347;
       else  if (y1 < 0)  y1 = 0;

       if (y2 > 347)      y2 = 347;
       else  if (y2 < 0)  y2 = 0;

       if (y1 < y2)  mwlv(x1,y1,y2,value);
       else          mwlv(x1,y2,y1,value);
       return;
   }
   if (y1 == y2)    /* segment horizontal */
   {
       /* controler les debordements possibles */
       if ((y1 > 347) || (x1 < 0)) return;

       if (x1 > 719)      x1 = 719;
       else  if (x1 < 0)  x1 = 0;

       if (x2 > 719)      x2 = 719;
       else  if (x2 < 0)  x2 = 0;

       if (x1 < x2)  mwlh(x1,x2,y1,value);
       else          mwlh(x2,x1,y1,value);
       return;
   }
   /* segment quelconque  : le controle debordement est fait dans mwpixel */
   if ((dx = x2 - x1) < 0)   dx *= -1;
   if ((dy = y2 - y1) < 0)   dy *= -1;

   d = 2*dy - dx;
   incr1 = 2*dy;
   incr2 = 2*(dy - dx);
   if (x1 > x2)
   {
      x    = x2;
      y    = y2;
      xend = x1;
   }
   else
   {
      x    = x1;
      y    = y1;
      xend = x2;
   }
   mwpixel(x,y,value);
   while (x < xend)
   {
      x++;
      if (d < 0)
            d += incr1;
      else
      {
            y++;
            d += incr2;
      }
      mwpixel(x,y,value);
   }
}

/*---------------------------------------------------------------------------
 * SymCircle - affiche les 8 points symetriques du calcul du cercle
 *---------------------------------------------------------------------------
 */
static SymCircle(xc,yc,x,y,value)
int xc,yc,x,y,value;
{
   /* compensez  sur x en multipliant par 1,5 */

   mwpixel(xc + x + (x>>1)  ,yc + y,value);
   mwpixel(xc + y + (y>>1)  ,yc + x,value);
   mwpixel(xc + y + (y>>1)  ,yc - x,value);
   mwpixel(xc + x + (x>>1)  ,yc - y,value);
   mwpixel(xc - (x + (x>>1)),yc - y,value);
   mwpixel(xc - (y + (y>>1)),yc - x,value);
   mwpixel(xc - (y + (y>>1)),yc + x,value);
   mwpixel(xc - (x + (x>>1)),yc + y,value);

}

/*---------------------------------------------------------------------------
 * circle - tracer un cercle
 *---------------------------------------------------------------------------
 */
circle(xc,yc,rayon,value)
int xc,yc,rayon,value;
{
    int x,y,d;

    x = 0;
    y = rayon;
    d = 3 - 2 * rayon;
    while (x < y)
    {
       SymCircle(xc,yc,x,y,value);
       if (d < 0)
            d = d + (4 * x) + 6;
       else
       {
            d = d + 4 * (x - y) + 10;
            y--;
       }
       x++;
    };
    if (x == y)
           SymCircle(xc,yc,x,y,value);
}

/*---------------------------------------------------------------------------
 * boxo - tracer une boxe ordonnee
 *---------------------------------------------------------------------------
 */
boxo ( xG , yG , xD , yD , color)
int xG , yG , xD , yD , color;
{
     draw (xG,yG,xG,yD,color);
     draw (xD,yG,xD,yD,color);
     draw (xG,yG,xD,yG,color);
     draw (xG,yD,xD,yD,color);
}

/*---------------------------------------------------------------------------
 * boxg - tracer une boxe
 *---------------------------------------------------------------------------
 */
boxg ( xA , yA , xB , yB , color)
int xA , yA , xB , yB , color;
{
     if (xA < xB)
                if (yA < yB)                       /* xa , ya pt sup g */
                      boxo ( xA , yA , xB , yB , color);
                else                               /* xa , ya pt inf g */
                      boxo ( xA , yB , xB , yA , color);
     else
               if (yA < yB)                        /* xa , ya pt sup d */
                      boxo ( xB , yA , xA , yB , color);
               else                                /* xa , ya pt inf d */
                      boxo ( xB , yB , xA , yA , color);
}

demobox()
{
     int i;
     for (i = 0;i < 30 ; i++)
         boxg( rand(),rand(),rand(),rand(),1);
}

