/* printf.c */
#include "sys.h"
#include "sem.h"
#include "conf.h"
#include "io.h"
#include "tty.h"
#include "window.h"
#include "dynlink.h"

#define  WOUTPUT  -2   /* sortie de type window */
#define  WSESSION -5   /* sortie de type session */

extern Bool fpanic;
int m_Putc(),m_Getc(),m_Getche(),m_Getch();

/*-----------------------------------------------------------------------------
 * m_Printf - ecrire sur sortie standard
 *-----------------------------------------------------------------------------
 */
m_Printf(strfmt,args)
char *strfmt;
{
/*char buf[128];
va_list marker;

        va_start(marker, strfmt);
    vsprintf(buf, strfmt, marker);

    _Prntf(buf, m_Putc ,&args, stdout);*/
    _Prntf(strfmt, m_Putc ,&args, stdout);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * m_Seprintf - ecrire dans une session
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Seprintf(session, strfmt, args)
char *strfmt;
{
    extern int _Cputc();
    _Prntf(strfmt, _Cputc ,&args, WSESSION, &tty[session], session);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * sysmsg - ecrire sur erreur standard (window ERR si mode initialis‚)
 *-----------------------------------------------------------------------------
 */
/*BIBLIO sysmsg(reponse,pid,strfmt,args)*/
sysmsg(strfmt,args)
char *strfmt;
{
/*    struct tty *ptty;*/
    int session;

/*    if ((ptty = &tty[session = Tasktab[pid].tgrp])->wmode)
         (* winfunc[WIN_PERR])(reponse,session,ptty,strfmt,&args);
    else
         _Prntf(strfmt, m_Putc ,&args, stdout);*/


            /* recuperer le handle de session du process
             * appelant
             */
            session = m_GetProcSessionHandle(m_Getpid());

            /* selectionner la session d'erreur */
            _chgActiveSessionSys(VS9, VS9);

            /* effacer les precedants messages */
            m_Printf("\033[2J\033[0;0H");

            _Prntf(strfmt, m_Putc ,&args, stdout);
            m_Sgetch(0);

            /* replacer la session appelante */
            m_ChgActiveSession(session);

    return(ROK);
}


/*-----------------------------------------------------------------------------
 * m_Fprintf - ecrire sur stream specifie ( device ou fichier )
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Fprintf(fd, strfmt, args)
int fd;
char *strfmt;
{
    _Prntf(strfmt, m_Putc ,&args, fd);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * m_Sprintf - ecrire dans buffer
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Sprintf(str, strfmt, args)
char *str;
char *strfmt;
{

    _Prntf(strfmt, m_Putc ,&args, BADDEV, str);
    return(ROK);
}



/*-----------------------------------------------------------------------------
 * VS0printf - ecrire sur ecran virtuel 0 (VS0)
 *-----------------------------------------------------------------------------
 */
BIBLIO VS0printf(strfmt, args)
char *strfmt;
{
    extern int VS0putc();
    _Prntf(strfmt, VS0putc ,&args, VS0 );
    return(ROK);
}


/*-----------------------------------------------------------------------------
 * _systemHalt - ecrire sur VS0 et stopper immediatement le systeme
 *-----------------------------------------------------------------------------
 */
_systemHalt(str)
char *str;
{
    VS0printf(sysHaltStr, str);
    fpanic = TRUE;
    m_Msgsync(SHTDWN);
}
/*-----------------------------------------------------------------------------
 * m_Perror - envoyer le libell‚ d'erreur sur la standard erreur
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Perror(str)
char *str;
{
    int ps;
    _puts(stderr, str);
    ps = _itDis();
    str = errStr[Tasktab[RUNpid].terrno];
    _itRes(ps);
    _puts(stderr, str);
    return(ROK);
}

/*---------
 * m_Errno
 *---------
 */
BIBLIO m_Errno()
{
    return(Tasktab[RUNpid].terrno);
}



/*-----------------------------------------------------------------------------
 * _Prntf - formatter et ecrire sur l'OUTPUT specifie ou dans une string
 *              (dans le cas de SPRINTF)
 *-----------------------------------------------------------------------------
 */
_Prntf(strfmt, putcfunc, args, output, sprintfBuff, session)
char  *strfmt;               /* string format                              */
int  (* putcfunc)();         /* fonction PUTC employee                     */
int   *args;                 /* @ debut des arguments de la string         */
int    output;               /* file descriptor de l'OUTPUT                */
char  *sprintfBuff;          /* adresse buffer (pour sprintf seulement)    */
                             /* adresse slot session pour seprintf         */
{
    int  c;
    int  i;
    int  f;                  /* format caractere venant derriere "%"       */
    char *str;               /* pointeur courant dans la string de travail */
                             /* ou dans l'argument chaine de caracteres    */
    char string[20];         /* string de travail                          */
    int  length;             /* longueur de la string de travail           */
    char fill;               /* caractere de remplissage ('0' ou ' ')      */
    int  leftjust;           /* indicateur d'alignement : 0 aligner a      */
                             /* droite - sinon a gauche                    */
    int  longflag;           /* ! 0 indique valeur numerique longue        */
    int  fmax, fmin;         /* specification des champs % MIN . MAX s     */
    int  leading;            /* nb de caracteres de remplissage a ecrire   */
    char sign;               /* mis a '-' pour les decimaux negatifs       */
    char digit1;             /* offset a additioner au 1er digit numeric   */
    struct window *pwin;     /* si output de type WINDOW                   */
    struct tty *ptty;        /* si OUTPUT session                          */

    if (output == WOUTPUT)
        pwin = (struct window *)sprintfBuff;
    else
        if (output == WSESSION)
                ptty = (struct tty *)sprintfBuff;
    for(;;)
    {
         /* ecrire caracteres jusqu'a '%' ou '\0'*/
         while ( (c = *strfmt++) != '%')
         {
              if (c == '\0')
              {

                   if (output == BADDEV)
                        *sprintfBuff = '\0';
                   else
                        if (output == WSESSION)
                                _sflush(session);
                        else
                                _pflush(output);
                   return;
              }
             _doWrite(sprintfBuff, &sprintfBuff, putcfunc, c, session, output);
         }

         /* ecrire '%' suite a "%%" */
         if (*strfmt == '%')
         {
             _doWrite(sprintfBuff, &sprintfBuff,putcfunc, *strfmt++, session, output);
             continue;
         }

         /*  on a affaire a un formattage
          *  verifier si alignement a gauche demande ("%-...")
          */

         if (*strfmt == '-') {
                leftjust = TRUE;
                strfmt++;
         }
         else   leftjust = FALSE;

         /*  determiner le caractere de remplissage
          *  "%0..." signifie remplissage par des 0 demande
          */

         fill = (*strfmt == '0') ? *strfmt++ : ' ';

         /*  determiner la largeur minimum du champs specifie par %d,u,x,o,c,s
          *  ainsi que %* pour les largeurs variables (ansi que %0*)
          */

         fmin = 0;
         if (*strfmt == '*')
         {
              fmin = *args++;
              ++strfmt;
         }
         else
              while(*strfmt >= '0' && *strfmt <= '9')
              {
                fmin = fmin * 10 + (*strfmt++ & 0x00ff) - '0';
              };

         /* determiner la largeur MAX de la string (cas de %s) */
         fmax = 0;
         if (*strfmt == '.')
         {
              if ( *(++strfmt) == '*')
              {
                   fmax = *args++;
                   ++strfmt;
              }
              else
                   while (*strfmt >= '0' && *strfmt <= '9')
                   {
                      fmax = fmax * 10 + (*strfmt++ & 0x00ff) - '0';
                   };
         }

         /*  verifier si l'option 'l' forcant en numerique long
          *  est presente
          */

         if (*strfmt == 'l') {
                longflag = TRUE;
                strfmt++;
         }
         else   longflag = FALSE;

         str = string;
         if ( (f = *strfmt++) == '\0')
         {

              if (output >= 0)             (*putcfunc)('%',output);
              else
                  if (output == WOUTPUT)   (*putcfunc)(pwin,'%',session);
                  else
                     if (output == BADDEV)  {
                         *sprintfBuff++ = '%';
                         *sprintfBuff   = '\0';
                     }
                     else
                         (*putcfunc)(ptty, '%');

              return;
         }
         sign = '\0';

         /*  Remplir la string de travail par
          *  l'argument correspondant a cette position
          */

         switch(f)
         {
              case 'c' : string[0] = (char) *args;
                         string[1] = '\0';
                         fmax = 0;
                         fill = ' ';
                         break;

              case 's' :/* str = *(long *)args;*/
                         str = *(char **)args;
                         args++;
                         fill = ' ';
                         break;

              case 'D' : longflag = 1;
              case 'd' : if (longflag)
                         {
                             if (*(long *)args < 0)
                             {
                                  sign = '-';
                                  *(long *)args = -*(long *)args;
                             }
                         }
                         else
                             if (*args < 0)
                             {
                                  sign = '-';
                                  *args = -*args;
                             }
                         longflag--;
              case 'U' : longflag++;
              case 'u' : if (longflag)
                         {
                             digit1 = '\0';

                             /*  les "longs negatifs" a ecrire en format
                              *  non signe ne peuvent etre convertis par division
                              *  moralite : faire la division " a la main"
                              */

                             while (*(long *)args < 0)
                             {
                                  *(long *)args -= 1000000000L;
                                  ++digit1;
                             }
                             _longDecimalString(*(long *)args, str);
                             str[0] += digit1;
                             ++args;
                         }
                         else
                             _decimalString(*args, str);
                         fmax = 0;
                         break;
              case 'O' : longflag++;
              case 'o' : if (longflag)
                         {
                             _longOctalString(*(long *)args, str);
                             ++args;
                         }
                         else
                             _octalString(*args, str);
                         fmax = 0;
                         break;
              case 'X' : longflag++;
              case 'x' : if (longflag)
                         {
                             _longHexaString(*(long *)args, str);
                             ++args;
                         }
                         else
                             _hexaString(*args, str);
                         fmax = 0;
                         break;
              default :
                        _doWrite(sprintfBuff, &sprintfBuff,putcfunc, f, session, output);
                        break;
         }
         args++;

         /* calculer la taille de la string a ecrire */
         for(length = 0; str[length] != '\0'; length++);

         /*  determiner le nb de caracteres de remplissage
          *  a ecrire en entete
          */

         if (fmin > MAXSTR || fmin < 0)
              fmin = 0;
         if (fmax > MAXSTR || fmax < 0)
              fmax = 0;
         leading = 0;
         if (fmax != 0 || fmin != 0)
         {
              if (fmax != 0)
                   if (length > fmax)     length = fmax;
              if (fmin != 0)
              {
                   if (length > fmin)     length = fmin;
                   else
                                          leading = fmin - length;
              }
              if (sign == '-')          --leading;
         }

         /*  Ecriture du signe et des caracteres de remplissage
          *   si fill = '0' , on ecrit le signe immediatement
          *   si fill = ' ' , on ecrit d'abord les carac de remplissage
          *                   puis le signe
          */

         if (sign == '-' && fill == '0') /* ecrire le signe */
                _doWrite(sprintfBuff, &sprintfBuff,putcfunc, sign, session, output);

         /*  si alignement a droite ecrire le nb
          *  de caracteres de remplissage requis
          */

         if (leftjust == 0)              /* ecrire fill ( 0 ou ' ' ) */
              for (i = 0; i < leading; i++)
                   _doWrite(sprintfBuff, &sprintfBuff,putcfunc, fill, session, output);

         /* ecrire le signe '-' si le caractere de remplissage est ' ' */
         if (sign == '-' && fill == ' ')
                   _doWrite(sprintfBuff, &sprintfBuff,putcfunc, sign, session, output);

         /* ecrire string de travail ou argument string */
         for (i = 0; i < length; i++)
                   _doWrite(sprintfBuff, &sprintfBuff,putcfunc, str[i], session, output);

         /* si alignement a gauche , ajouter les caracteres
          * de remplissage si besoin
          */

         if (leftjust != 0)
              for (i = 0; i < leading; i++)
                   _doWrite(sprintfBuff, &sprintfBuff,putcfunc, fill, session, output);

    }
}
_doWrite(objet, string, putcfunc, ch, session, output)
char *objet;
char **string;
int (* putcfunc)();
char ch;
{


                   if (output >= 0)             (*putcfunc)(ch, output);
                   else
                       if (output == WOUTPUT)   (*putcfunc)((struct window *)objet, ch, session);
                       else
                           if (output == BADDEV) {
                                                *objet = ch;
                                                (*string)++;
                           }
                           else
                                                (*putcfunc)((struct tty *)objet, ch);
}
