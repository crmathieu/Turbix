/* scanf.c */
#include "sys.h"
#include "sem.h"
#include "conf.h"
#include "io.h"
#include "tty.h"

#define  WOUTPUT -2   /* sortie de type window */
#define  SPACE  '\x20'

int m_Putc(),m_Getc(),m_Getche(),m_Getch();
extern long atol();

long _stringHexaToLong();
long _stringHexaToLongO();

/*-----------------------------------------------------------------------------
 * m_Scanf - saisie formattee a partir de l'entree standard
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Scanf(strfmt,args)
char *strfmt;
{

    _Scnf(strfmt, m_Getch , m_Putc, &args, stdin, stdout);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * m_Fscanf - saisie format‚e a partir du stream specifie ( device ou fichier )
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Fscanf(fd, strfmt, args)
int fd;
char *strfmt;
{
    _Scnf(strfmt, m_Getch, m_Putc ,&args, fd, stdout);
    return(ROK);
}

/*-----------------------------------------------------------------------------
 * m_Sscanf - Saisie formatt‚e a pratir d'un buffer
 *-----------------------------------------------------------------------------
 */
BIBLIO m_Sscanf(str, strfmt, args)
char *str;
char *strfmt;
{

    _Scnf(strfmt, m_Getch, m_Putc ,&args, BADDEV, stdout, str);
    return(ROK);
}


/*-----------------------------------------------------------------------------
 * _Scnf -
 *-----------------------------------------------------------------------------
 */
_Scnf(strfmt, getcfunc, putcfunc, args, input, output, sprintfBuff)
char  *strfmt;               /* string format                              */
int  (* getcfunc)();         /* fonction GETC employee                     */
int  (* putcfunc)();         /* fonction PUTC employee                     */
int   *args;                 /* @ debut des arguments de la string         */
int    input;                /* fd input                                   */
int    output;               /* file descriptor de l'OUTPUT                */
char  *sprintfBuff;          /* adresse buffer (pour sprintf seulement)    */
{
    int  c;
    int  i;
    int  s, ss, start, smax;
    int  f;                  /* format caractere venant derriere "%"       */
    char *str;               /* pointeur courant dans la string de travail */
                             /* ou dans l'argument chaine de caracteres    */
    char string[20];         /* string de travail                          */
    char strscan[20];
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

    if (output == WOUTPUT)
        pwin = (struct window *)sprintfBuff;

    for(;;)
    {
         /* ecrire caracteres jusqu'a '%' ou '\0'*/
         while ( (c = *strfmt++) != '%')
         {
              if (c == '\0') {
                   _pflush(output);
                   return;
              }
              if (output == WOUTPUT)
                        (*putcfunc)(pwin,c);
              else
                        (*putcfunc)(c,output);
         }

         /* ecrire '%' suite a "%%" */
         if (*strfmt == '%')
         {
                if (output == WOUTPUT)
                        (*putcfunc)(pwin,*strfmt++);
                else
                        (*putcfunc)(*strfmt++,output);
              continue;
         }

         /*  on a affaire a un formattage
          *  verifier si alignement a gauche demande ("%-...")
          */

/*         if (leftjust = ((*strfmt == '-') ? 1 : 0) )
              strfmt++;*/

         /*  determiner le caractere de remplissage
          *  "%0..." signifie remplissage par des 0 demande
          */

/*         fill = (*strfmt == '0') ? *strfmt++ : ' ';*/

         /*  determiner la largeur minimum du champs specifie par %d,u,x,o,c,s
          *  ainsi que %* pour les largeurs variables (ainsi que %0*)
          */

/*         fmin = 0;
         if (*strfmt == '*')
         {
              fmin = *args++;
              ++strfmt;
         }
         else
              while(*strfmt >= '0' && *strfmt <= '9')
              {
                fmin = fmin * 10 + (*strfmt++ & 0x00ff) - '0';
              };*/

         /*  verifier si l'option 'l' forcant en numerique long
          *  est presente
          */

         longflag = 0;
         if (*strfmt == 'l') {
                longflag = 1;
                strfmt++;
         }

         str = string;
         if ( (f = *strfmt++) == '\0')
         {
                if (output == WOUTPUT)
                        (*putcfunc)(pwin,'%');
                else
                        (*putcfunc)('%',output);
              return;
         }
         sign = '\0';

         /*  Remplir la string de travail par
          *  l'argument correspondant a cette position
          */

         switch(f)
         {
              case 'c' : if (output >= 0) {
                             if (input == BADDEV) {
                                        **(char **)args = *sprintfBuff++;
/*                                        if (*sprintfBuff == SPACE)
                                                sprintfBuff++;*/
                             }
                             else
                                        **(char **)args = ( *getcfunc)(input);

                             (*putcfunc)(**(char **)args,output);
                         }
                         else   {
                                        **(char **)args = ( *getcfunc)(pwin);
                                        (*putcfunc)(pwin,**(char **)args);
                         }

                         fmax = 0;
                         fill = ' ';
                         args++;
                         break;

              case 's' :
                         s = 0;
                         if (output >= 0) {
                                if (input == BADDEV)
                                        while ((strscan[s] = *sprintfBuff++) != SPACE)
                                                (*putcfunc)(strscan[s++],output);
                                else
                                        while ((strscan[s] = (*getcfunc)(input)) != SPACE)
                                                (*putcfunc)(strscan[s++],output);
                         }
                         else
                                 while ((strscan[s] = (*getcfunc)(pwin)) != SPACE)
                                        (*putcfunc)(pwin,strscan[s++]);


                         strscan[s] = '\0';
                         str = strscan;
                         memcpy(*(char **)args,strscan,strlen(strscan)+1);
                         args++;         /* because pointeur sur 4 octets */
                         fill = ' ';
                         break;

              case 'D' : /*longflag = 1;*/
              case 'd' :
                         s = 0; smax = 5 * (longflag + 1);
                         if (output >= 0) {
                                if (input == BADDEV)
                                        while ((strscan[s] = *sprintfBuff++) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                                else
                                        while ((strscan[s] = (*getcfunc)(input)) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                         }
                         else
                                 while ((strscan[s] = (*getcfunc)(pwin)) != SPACE && s < smax)
                                        (*putcfunc)(pwin,strscan[s++]);

                         strscan[s] = '\0';
                         start = 0;
                         ss = 1;
                         if (strscan[0] == '-') {
                             ss = -1;
                             start = 1;
                         }
                         else if (strscan[0] == '+') start = 1;

                         if (longflag)
                         {
                             **(long **)args = _stringHexaToLong(&strscan[start]);
                             **(long **)args *= ss;
                             if (**(long **)args < 0)
                             {
                                  sign = '-';
                                  **(long **)args = -**(long **)args;
                             }
                         }
                         else
                         {
                             **(int **)args = _stringHexaToInt(&strscan[start]);
                             **(int **)args *= ss;
                             if (**(int **)args < 0)
                             {
                                  sign = '-';
                                  **(int **)args = -**(int **)args;
                             }
                         }
                         break;
                         longflag--;

              case 'U' : /*longflag++;*/
              case 'u' :
                         if (f == 'u' || f == 'U') { /* saisir */
                         s = 0;smax = 5 * (longflag + 1);
                         if (output >= 0) {
                                if (input == BADDEV)
                                        while ((strscan[s] = *sprintfBuff++) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                                else
                                        while ((strscan[s] = (*getcfunc)(input)) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                         }
                         else
                                 while ((strscan[s] = (*getcfunc)(pwin)) != SPACE && s < smax)
                                        (*putcfunc)(pwin,strscan[s++]);

                         strscan[s] = '\0';
                             if (longflag)  **(long **)args = _stringHexaToLong(strscan);
                             else           **(int **)args = _stringHexaToInt(strscan);
                         }
                         /* commun a D, d, U, u */
                         if (longflag)
                         {
                             digit1 = '\0';

                             /*  les "longs negatifs" a ecrire en format
                              *  non signe ne peuvent etre convertis par division
                              *  moralite : faire la division " a la main"
                              */

                             while (**(long **)args < 0)
                             {
                                  **(long **)args -= 1000000000L;
                                  ++digit1;
                             }
                             _longDecimalString(**(long **)args, str);
                             str[0] += digit1;
                          /*   ++args;*/
                         }
                         else
                             _decimalString(**(int **)args, str);
                         fmax = 0;
                         ++args;/**/
                         break;
              case 'O' : /*longflag++;*/
              case 'o' :
                         s = 0; smax = (5 * (longflag + 1)) + 1;
                         if (output >= 0) {
                                if (input == BADDEV)
                                        while ((strscan[s] = *sprintfBuff++) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                                 else
                                        while ((strscan[s] = (*getcfunc)(input)) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                         }
                         else
                                 while ((strscan[s] = (*getcfunc)(pwin)) != SPACE && s < smax)
                                        (*putcfunc)(pwin,strscan[s++]);

                         strscan[s] = '\0';
                         if (longflag)
                         {
                             **(long **)args = _stringHexaToLongO(strscan);
/*                             _longOctalString(**(long **)args, str);*/
                             ++args;
                         }
                         else {
                             **(int **)args = _stringHexaToO(strscan);
/*                             _octalString(**(int **)args, str);         */
                             args++;
                         }
                         fmax = 0;
                         break;
              case 'X' : /*longflag++;*/
              case 'x' :
                         s = 0; smax = 4 * (longflag + 1);
                         if (output >= 0) {
                                if (input == BADDEV)
                                        while ((strscan[s] = *sprintfBuff++) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                                 else
                                        while ((strscan[s] = (*getcfunc)(input)) != SPACE && s < smax)
                                                (*putcfunc)(strscan[s++],output);
                         }
                         else
                                 while ((strscan[s] = (*getcfunc)(pwin)) != SPACE && s < smax)
                                        (*putcfunc)(pwin,strscan[s++]);

                         strscan[s] = '\0';
                         if (longflag)
                         {
                             **(long **)args = _stringHexaToLong(strscan);
/*                             _longHexaString(**(long **)args, str);*/
                             ++args;
                         }
                         else  {
                             **(int **)args = _stringHexaToInt(strscan);
/*                             _hexaString(**(int **)args, str);*/
                             args++;
                         }
                         fmax = 0;
                         break;
              default :
                         if (output == WOUTPUT)   (*putcfunc)(pwin,f);
                         else
                                (*putcfunc)(f,output);
                        break;
         }
         args++;

    }

}


