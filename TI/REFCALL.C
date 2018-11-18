/* gestion des appels de fonctions */
#include "xed.h"
#include "emsg.h"

#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include <ctype.h>
#include <process.h>
#include <sys\stat.h>

extern int nb_car_per_line;

char *ARGV[5];
char *Rbuf;
char writeBuf[80];

/* RECORD dans le fichier des DEF */
struct defRef {
         char     ctype;       /* fichier C ou Ass */
         int      position;   /* numero de ligne ou est definie la func */
         char     funcName[32];/* nom de la fonction */
unsigned char     type;       /* type : EXT, DEF, DEC */
};

/* Zone de decomposition en tokens */
#define SHBUFLEN    nb_car_per_line    /* longueur du buffer                */
#define SHARGLEN    82                 /* longueur de la zone des arguments */
#define SHMAXTOK    80                 /* maximum de tokens par ligne       */
#define SHMLEN      12                 /* longueur maximum du nom machine   */

struct  shvars {                       /* shell variables                   */
        long   shlast;                 /* derniere frappe                   */
        int    shncmds;                /* nombre de commandes valides       */
        char   *shbuf;                 /* buffer du shell                   */
        char   *shtok[SHMAXTOK];       /* pointeurs d'entree d'un token     */
        char   shtktyp[SHMAXTOK];      /* type de token dans shtok[i]       */
        char   shargst[SHARGLEN];      /* zone stockage string arg          */
};



#define MAXFUNC 500             /* Nb max de fonctions dans un module */
struct defRef functab[MAXFUNC];

#define TYPE_NUL 0      /* rien */
#define TYPE_DEF 1      /* Definition */
#define TYPE_EXT 2      /* Externe */
#define TYPE_DEC 3      /* Declaration */

char *typeTab[] = {"","DEF","EXT","DECL"};

char *CreservedWord[] = {
        "if", "return","while","for","int","long","sizeof","FP_OFF","FP_SEG",
        "switch"};

#define CRESERVW 10
#define TRUE  1
#define FALSE 0

#define CFLAG  1
#define AFLAG  0

int Bflag;     /* flag de comptage des brackets */
int Lbrack;    /* # de { */
int Rbrack;    /* # de } */
int noligne;   /* # de ligne ou se situe l'appel */
int deja;
int fdW;
char refWork[80];


#define ERRMSG 4
#define NMXMSG 3

static int etat;

refcall(fname)
char *fname;
{
        int  cflag;
        char *pName;
        char nfname[64];

        memset(functab, 0, sizeof(struct defRef)*100);
        etat = deja = noligne = Rbrack = Bflag = Lbrack = 0;

        /* determiner le type de fichier */
        strcpy(nfname, fname);
        strupr(fname);
        if ((pName = strpbrk(nfname, ".")) == NULL) {
                pushPop_MESS(nfname, ERRMSG);
                return(-1);
        }

        cflag = TRUE;
        if ((Rbuf = malloc(nb_car_per_line)) == NULL) {
                pushPop_MESS(" Not enough memory ", ERRMSG);
                return(-1);
        }


        if (*(pName+1) == 'C') {
                if (get_function(fname, CFLAG)) {
                        free(Rbuf);
                        return(-1);
                }
                updateDECL();
        }
        else {
                cflag = FALSE;
                if (get_function(fname, AFLAG)) {
                        free(Rbuf);
                        return(-1);
                }
        }

        writeHeader();
        writeResult(cflag);

        /* fermer le fichier .REF */
        close(fdW);
        free(Rbuf);
        return 0;
}

get_function(fname, Cflag)
char *fname;
{
    int  load, wSize, ret;
    char *pName, nfname[64], str[80];

    FILE *fp;

    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(str,refCallOpenf, fname);
        pushPop_MESS(str, ERRMSG);
        return(-1);
    }

    memset(nfname, '\0', 64);
    strcpy(nfname, fname);

    /* creer le fichier final */
    if ((pName = strpbrk(nfname, ".")) == NULL) {
        sprintf(str,refCallIncorr, fname);
        pushPop_MESS(str, ERRMSG);
        return(-1);
    }
    *(pName+1) = '\0';
    strcat(pName, "REF");

    remove(nfname);
    if ((fdW = open(nfname,O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE)) == -1) {
         pushPop_MESS(refCallwkf, ERRMSG);
         return(-1);
    }


    ret = TRUE;
    while (ret) {
        ret = getSrcLine(fp);      /* lire une ligne */
        if (Cflag)
                C_automate();
        else
                A_automate();
    }
    fclose(fp);
    return(0);
}

/*---------------------------------------------------
 * C_automate - Rbuf contient la ligne de source
 *---------------------------------------------------
 */
C_automate()
{
#define WAIT_LPARENTH   0
#define WAIT_STATE      1
#define WAIT_STAR1      2
#define WAIT_STAR2      3
#define WAIT_SLASH2     4

        int i, j, k;


        i = 0;
        j = strlen(Rbuf);

        while (i < j) {
        switch(etat) {

        case WAIT_LPARENTH: if (Rbuf[i] == '(') {
                                i--;
                                etat = WAIT_STATE;
                            }
                            else {
                                if (Rbuf[i] == '{' && Bflag) Lbrack++;
                                if (Rbuf[i] == '}' && Bflag) Rbrack++;
                                if (Rbuf[i] == '/') etat = WAIT_STAR1;
                                i++;
                            }
                            break;

        case WAIT_STATE   : k = i;

                            /* remonter sur la fin du nom de fonction */
                            while (i > 0 && Rbuf[i] == ' ') i--;
                            if (isalnum(Rbuf[i]))
                                writefuncName(i);
                            i = k+2;
                            Bflag = TRUE;
                            etat = WAIT_LPARENTH;
                            break;

        case WAIT_STAR1   : if (Rbuf[i] == '*') {
                                etat = WAIT_STAR2;
                                i++;
                            }
                            else
                                etat = WAIT_LPARENTH;
                            break;

        case WAIT_STAR2   : if (Rbuf[i] == '*')
                                etat = WAIT_SLASH2;
                            i++;
                            break;

        case WAIT_SLASH2  : if (Rbuf[i] == '/') {
                                etat = WAIT_LPARENTH;
                                i++;
                            }
                            else
                                etat = WAIT_STAR2;
                            break;

        default           : break;
        }
        }
}

/*---------------------------------------------------
 * A_automate - Rbuf contient la ligne de source
 *---------------------------------------------------
 */
A_automate()
{
        struct shvars Shl;   /* decomposition en Token */
        char buf[64];
        int i, ntok;


        ntok = lexan(&Shl);

        /* scanner les tokens */
        for (i = 0; i < ntok; i++) {
/*                pushPop_MESS(Shl.shtok[i], NMXMSG);*/
                if (Shl.shtok[i][0] == ';') /* skip commentaires */
                        break;
                if (stricmp(Shl.shtok[i], "EXTRN") == 0) {
                        strcpy(buf, Shl.shtok[i+1]);
                        recordName(buf, AFLAG, TYPE_EXT);
                        break;
                }

                if (stricmp(Shl.shtok[i], "PROC") == 0) {
                        strcpy(buf, Shl.shtok[i-1]);
                        recordName(buf, AFLAG, TYPE_DEF);
                        break;
                }
        }
}

/*---------------------------------------------------
 * getSrcLine - saisit une ligne de commande en
 *           supprimant CR et remplacant LF par '\0'
 *---------------------------------------------------
 */
getSrcLine(fp)
FILE *fp;
{
   char c;
   int input = 0;

   noligne++;
   /* saisir ligne en supprimant les CR */
   while ((c = getc(fp)) != '\n'&& (input < nb_car_per_line) && c != EOF)
             if (c != '\r')   Rbuf[input++] = c;

   if (c == '\n') {
              Rbuf[input++] = '\0';
              return(1);
   }
   else {
              Rbuf[input] = '\0';
              return(0);
  }
}

/*----------
 * writefuncName
 *----------
 */
writefuncName(i)
{
        char c, buf[64];
        int j, k, z;

        j = i;

        /* tester si Macro */
        z = 0;
        while (Rbuf[z++] == ' ');
        if (Rbuf[z] == '#')
                return;

        /* aller en debut de nom */
        while ((i > 0) && !(isspace((c = Rbuf[i])))) {
                if (!isalnum(c)) {
                        if (c == '_')
                                i--;
                        else {
                                i++;
                                break;
                        }
                }
                else
                        i--;
        }

        memset(buf,'\0',64);

        while (!isalnum(Rbuf[i]))
                if (Rbuf[i] == '_')
                        break;
                else
                        Rbuf[i++] = ' ';
        /* recopie */
        for (k = 0; i<=j; i++, k++)
                buf[k] = Rbuf[i];

        /*strupr(buf);*/

        for (i=0; i<CRESERVW; i++)
                if (strcmp(buf, CreservedWord[i]) == 0)
                        return;

        /* enregistrer le nom de la fonction */
        recordName(buf, CFLAG);

/*        if (Lbrack == Rbrack)
                sprintf(writeBuf, "%4d - DEF  :  %s\n", noligne, buf);
        else
                sprintf(writeBuf, "%4d - CALL :  %s\n", noligne, buf);

        printf("%s", writeBuf);*/

}

/*----------
 * recordName
 *----------
 */
recordName(buf, cflag, assTyp)
char *buf;
{
        int i, type;

        if (cflag) {
                if (Lbrack == Rbrack)   {
                        if (Lbrack == 0)
                                type = TYPE_DEC;
                        else
                                type = TYPE_DEF;
                }
                else {
                        updateDECL();
                        type = TYPE_EXT;
                }
        }
        else
                type = assTyp;

        strupr(buf);
        for (i = 0; (i < MAXFUNC) && (functab[i].type != TYPE_NUL); i++)
                if (strcmp(functab[i].funcName, buf) == 0) {
                        if ((type == TYPE_DEF) ||
                           ((type == TYPE_EXT) && (functab[i].type == TYPE_DEC))) {
                                functab[i].type     = type;
                                functab[i].position = noligne;
                                functab[i].ctype    = cflag;
                        }
                        return;
                }
        if (i == MAXFUNC) {
                pushPop_MESS(refCallOver, ERRMSG);
                return;
        }
        strcpy(functab[i].funcName, buf);
        functab[i].type     = type;
        functab[i].position = noligne;
        functab[i].ctype    = cflag;
}
/*----------
 * writeHeader
 *----------
 */
writeHeader()
{
        int i, NB, type;

        /* compter le nb de fonctions */
        for (i = NB = 0; i < MAXFUNC; i++)
                if ((type = functab[i].type) == TYPE_EXT || type == TYPE_DEF)
                        NB++;

        /* ecrire header */
        write(fdW, &NB, 2);

}
/*----------
 * writeResult
 *----------
 */
writeResult(cflag)
{
        int i;

        for (i=0;(i < MAXFUNC) && (functab[i].type != TYPE_NUL); i++)
                if (functab[i].type != TYPE_DEC) {
                        /* si fichier C, ajouter underscore au nom */
                        if (cflag) {
                                strcpy(refWork, "_");
                                strcat(refWork, functab[i].funcName);
                                strcpy(functab[i].funcName, refWork);
                        }
                        if (functab[i].type == TYPE_DEF)
                                functab[i].ctype = cflag;
                        write(fdW, &functab[i], sizeof(struct defRef));
                }
        /* marquer la fin des fonctions */
        memset(&functab[0], 0xff, sizeof(struct defRef));
        write(fdW, &functab[0], sizeof(struct defRef));
}
/*----------
 * updateDECL
 *----------
 */
updateDECL()
{
        int i, k;

        if (deja)
                return;
        for (i=0, k=-1;functab[i].type != TYPE_NUL; i++)
                if (functab[i].type == TYPE_DEC)
                        k = i;
        if (k >= 0)
                functab[k].type = TYPE_DEF;
        deja = TRUE;
}

/*----------------------------------------------------------------------
 * lexan - ad hoc lexical analyser to divide command line into tokens
 *----------------------------------------------------------------------
 */
lexan( Shl )
struct shvars *Shl;
{
    char *line;
    char **tokptr;
    extern char *TOKbuffer;
    int  ntok;
    char *p;
    char ch;
    char *to;
    char quote;
    Shl->shbuf = TOKbuffer;
    memcpy(Shl->shbuf, Rbuf, SHBUFLEN);
    line = Shl->shbuf;
    to = Shl->shargst;                  /* area to place token strings */
    tokptr = &Shl->shtok[ntok = 0];     /* tableau de ptr sur TOKEN    */
    for (p = line; *p != '\0' && *p != '\n' && *p != '\r' && ntok < SHMAXTOK;)
    {
         while ( (ch = *p) == ' ')     /* sauter les blancs */
              p++;
         if (ch == '\0' || ch == '\n' || ch == '\r')  return(ntok);

         *tokptr++ = to;
         Shl->shtktyp[ntok++] = ch;    /* memoriser 1er carac du token */
         if (ch == '"' || ch == '\'')
         {
              quote = ch;
              for (p++; (ch = *p++) != quote && ch != '\n' && ch != '\0'; )
                                     *to++ = ch;
              if (ch != quote)       return(-1);
         }
         else
         {
              *to++ = *p++;
              if (ch != '>' && ch != '<' && ch != '&' && ch != '|')
                   while ((ch = *p) != '\n' && ch != '\0' && ch != '|' &&
                           ch != '<' && ch != '>' && ch != ' ' &&
                           ch != '"' && ch != '\'' && ch != '&')
                                        *to++ = *p++;   /* copy alphanum */
         }
         *to++ = '\0';
    }
    return(ntok);
}

