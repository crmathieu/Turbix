
/* tty.c */

#include "sys.h"
#include "conf.h"
#include "tty.h"
#include "io.h"
#include "shell.h"
#include "sem.h"

#define TTYH 7

extern char *ttynams[];
LOCAL Bool speed12 = TRUE;
extern int *cronState;
int scrolltty = 0;


/*----------------------------------------------------------------------------
 * ttydisp - point d'entree commun aux IT :
 *                 reception            ( it_in )
 *                 transmission         ( it_out )
 *                 erreur sur reception ( it_err )
 *----------------------------------------------------------------------------
 */
_ttydisp()
{
       unsigned int ps,c;
       int numchip;

       c = (inp(Serialtab[numchip = _whatchip()].ctypit) >> 1) & 0x03;

       /* type d'it ? */
       switch(c)
       {
          case 0 :  break;
          case 1 :  _it_out(numchip);break;    /* emission  */
          case 2 :  _it_in(numchip);break;    /* reception */
          case 3 :  _it_err(numchip);break;    /* erreur    */
          default : m_printf("COM Error\n");break;
       }
}

/*----------------------------------------------------------------------------
 *  whatchip - determine quel est le circuit qui a declench‚ l'interruption
 *----------------------------------------------------------------------------
 */
LOCAL int _whatchip()
{
    return(0);

}
/*----------------------------------------------------------------------------
 * _it_err - traite l'erreur de reception sur une ligne
 *----------------------------------------------------------------------------
 */
_it_err(chipnumber)
int chipnumber;
{
    struct tty *ptty;
    struct csr *cptr;
    int ps;

    ptty = &tty[chipnumber+TTY1];
    cptr = ptty->ioaddr;

    /* debloquer le circuit */
    inp(cptr->cetlgn);

    /* changer vitesse */
    ps = _itDis();
    if (speed12)
    {
      _delay();       outp( cptr->ccntrl , 0x80);
      _delay();       outp( cptr->crtbuf , 0x18);
      _delay();       outp( cptr->csetit , 0x00);
      _delay();       outp( cptr->ccntrl , 0x1a);
      _delay();       outp( cptr->ccntrm , 0x0b);
    }
    else
    {
      _delay();       outp( cptr->ccntrl , 0x80);
      _delay();       outp( cptr->crtbuf , 0x60);
      _delay();       outp( cptr->csetit , 0x00);
      _delay();       outp( cptr->ccntrl , 0x1a);
      _delay();       outp( cptr->ccntrm , 0x0b);
    }
    speed12 = !speed12;
    _itRes(ps);

}


/*---------------------------------------------------------------------------
 *  _it_in - filtrage sur interruption reception d'un caractere
 *---------------------------------------------------------------------------
 */
INTERRUPT _it_in(chipnumber)
int chipnumber;
{
    struct tty *ptty;
    struct csr *cptr;
    int ch;
    int ct;
    int ps;

    ps = _itDis();
/*    kputc("#");*/
    ptty = &tty[chipnumber+NVS];
    cptr = ptty->ioaddr;
    ch   = inp(cptr->crtbuf);
/*    m_printf("%c\n",ch);*/
    ptty->ITbuff[ptty->ITin++] = ch;
    ptty->ITin &= ITBUFMASK;

/*    msgsndI(VIO, 0, FALSE);*/
    _itRes(ps);
}

/*---------------------------------------------------------------------------
 *  _it_out - routine d'interruption tty en sortie
 *---------------------------------------------------------------------------
 */
INTERRUPT _it_out(chipnumber)
register int chipnumber;
{
    struct csr *cptr;
    struct tty *ptty;
    int ct,i,ps;
    unsigned char ch;

    ptty = &tty[chipnumber+NVS];
    cptr = ptty->ioaddr;

    /* vider la file de sortie */
    ps = _itDis();
    m_Countsem(ptty->osem, &ct);
    if (ct < OBUFLEN)
    {
/*        ch = ptty->Obuff[ptty->Oout++]&0xff;*/
          outp(cptr->crtbuf , ptty->Obuff[ptty->Oout++]&0xff);
          ptty->Oout &= BUFMASK;
          if ( ct > OBMINSP )     m_Sigsem(ptty->osem);
          else
          {
               if (++(ptty->odsend) == OBMINSP)
               {
                   ptty->odsend = 0;
                   _signsem(ptty->osem, OBMINSP);
               }
          }
    }
    else
    {
         ptty->oetat = OINACTIF;
         outp( cptr->csetit , S_DISABLE );
    }
    _itRes(ps);
/*    kputc(ch);*/
}


/*---------------------------------------------------------------------------
 *  _Sputc - ecrire un caractere
 *---------------------------------------------------------------------------
 */
_Sputc(ptty, ch)
struct tty *ptty;
char ch;
{
     int ps;
     int debut;

     ps = _itDis();
     if (ptty->oesc != 0)
         _ansiSequence(ptty,ch);
     else
         switch(ch) {
                 case '\033' : _flush(ptty);
                               ptty->oesc = 1;
                               break;
                 case '\n'   :          /* line feed */
                               if (ptty->ctty.c_oflag & ONLCR) { /* NL -> CR+NL */
                                   ptty->cursX = 0;
                                   _waitsem(ptty->osem,STAIL);
                                   ptty->Obuff[ptty->Oin++] = RETURN;
                                   ptty->Oin &= BUFMASK;
                               }
                               break;
                 case '\r'   :          /* carriage return */
                               if (ptty->ctty.c_oflag & OCRNL) /* CR -> NL */
                                   ch = NEWLINE;
                               ptty->cursX = 0;
                               break;
                 case '\b'   : ptty->cursX--;
                               break;
                 default     :
                               if (ptty->cursX++ >= 80)
                                   _Sputc(ptty,NEWLINE);
                               break;
         };
     _waitsem(ptty->osem,STAIL);
     ptty->Obuff[ptty->Oin++] = ch;
     ptty->Oin &= BUFMASK;
     _flush(ptty);
     _itRes(ps);
     return(ROK);
}

/*---------------------------------------------------------------------------
 *  _Swrite - ecrire un buffer sur la ligne
 *---------------------------------------------------------------------------
 */
_Swrite ( ptty , buff , count )
struct tty *ptty;
char *buff;
int count;
{
      int avail;
      int ps,ps2;

      if ( count < 0 )  return(RERR);
      if ( count == 0 ) return(ROK);
      ps = _itDis();
      m_Countsem(ptty->osem, &avail);
      if (avail >= count)
      {
         _writecopy(buff,ptty,count);
         if (ptty->oetat == OINACTIF)
         {
             ps2 = _disable();
             ptty->oetat = OACTIF;
             outp( (ptty->ioaddr)->csetit , S_ENABLE);
             _restore(ps);
         }
      }
      else
      {
         if (avail > 0)
         {
            _writecopy(buff,ptty,avail);
            if (ptty->oetat == OINACTIF)
            {
               ps2 = _disable();
               ptty->oetat = OACTIF;
               outp( (ptty->ioaddr)->csetit , S_ENABLE);
               _restore(ps2);
            }
            buff  += avail;
            count -= avail;
         }
         /* envoyer les caracteres supplementaires par des Sputc */
         for ( ; count > 0 ; count--)     _Sputc(ptty,*buff++);
      }
      _itRes(ps);
      return(ROK);
}

/*---------------------------------------------------------------------------
 *  _writecopy - copie rapide du buffer utilisateur dans le buffer de sortie
 *---------------------------------------------------------------------------
 */
LOCAL _writecopy(buff,ptty,count)
char       *buff;
struct tty *ptty;
int         count;
{
   int  *qhead ,*qend, ct ;
   char *uend;

   (unsigned *)qhead = &ptty->Obuff[ptty->Oin];
   (unsigned *)qend  = &ptty->Obuff[OBUFLEN];
   uend  = buff + count;
   while (buff < uend)
   {
      *(char *)qhead++ = *buff++;
      if (qhead >= qend)  (unsigned *)qhead = ptty->Obuff;
   }
   ptty->Oin = (unsigned *)qhead - ptty->Obuff;
   m_Countsem(ptty->osem, &ct);
   m_Resetsem(ptty->osem , ct - count);
}
_delay()
{
        long i;
        for(i=0x0000ff; i>0; i--) ;
}

/*----------------------------------------------------------------------------
 * initialisation des circuits 8250
 *----------------------------------------------------------------------------
 */
_initcomx(chipnumber)
int chipnumber;
{
  int ps;
  struct csr *cptr;
  extern long msdosv_com;
  extern int _itcom1();

  cptr = &Serialtab[chipnumber];
  cptr->csetit = S_1SETIT;
  cptr->ctypit = S_1TYPIT;
  cptr->crtbuf = S_1IOREG;
  cptr->ccntrl = S_1CNTRL;
  cptr->ccntrm = S_1CNTRM;
  cptr->cetlgn = S_1ETLGN;


      _delay();       outp( cptr->ccntrl , 0x80);
      _delay();       outp( cptr->crtbuf , 0x18);
      _delay();       outp( cptr->csetit , 0x00);
      _delay();       outp( cptr->ccntrl , 0x1a);
      _delay();       outp( cptr->ccntrm , 0x0b);

  /* demarrer a 4800 bauds */
/*  _delay();  outp( cptr->ccntrl , 0x80);
  _delay();  outp( cptr->crtbuf , 0x18);
  _delay();  outp( cptr->csetit , 0x00);
  _delay();  outp( cptr->ccntrl , 0x1a);
  _delay();  outp( cptr->ccntrm , 0x0b);*/

  tty[chipnumber+NVS].ioaddr = cptr;

  _sasVector(XENCOM , _itcom1 ,&msdosv_com);

    _delay(); inp(cptr->crtbuf);  /* decoince le circuit au cas ou */

    _delay();outp(cptr->csetit,S_ENABLE);
    _delay();outp(cptr->csetit,S_DISABLE);

}

/*----------------------------------------------------------------------------
 * tty_handler - gestion des drivers sur les lignes serie et le clavier
 *----------------------------------------------------------------------------
 */
TASK _tty_handler()
{ /*
     int i, ps, again;
     struct taskslot *tp;

     tp = &Tasktab[m_Getpid()];
     cronState = &tp->tstate;
     while (TRUE) {
            again = 0;
            for (i=0; i<NTTY; i++)
                 if (ischar(i))  {
                     _Input_ch(&tty[i]);
                     again++;
                 };
            if (!again) {
                ps = _itDis();
                tp->tstate = SLEEP;
                _schedul();
                _itRes(ps);
            }
     }
 */
}


/*---------------------------------------------------------------------------
 *  Cwrite - ecrire un caractere ou plus d'un tty device
 *---------------------------------------------------------------------------
 *//* INUTILISE
Swrite2(ptty, buff, count)
struct tty *ptty;
char   *buff;
int    count;
{
    int ps;
    int i, j;
    char *buff2;

    if (count < 0)       return(RERR);
    if (count == 0)      return(ROK);

    while(count--)
    {
         ps = _itDis();
         /* filtrer si caractere de controle ou sequence d'echappement */
/*         if (*buff < ' ' || ptty->oesc > 0)   _Sputc(ptty, *buff++);
         else
         {
             ptty->Obuff[ptty->Oin++] = (ptty->cvatt << 8) | *buff++;
             if (ptty->Oin >= ptty->Oout)     _flush(ptty);
         }
         _itRes(ps);
    }
    return(ROK);
}*/

