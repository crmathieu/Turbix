/* shell */

#include "sys.h"
#include "conf.h"
#include "signal.h"
#include "shell.h"
#include "const.h"
#include "cmd.h"
#include "tty.h"
#include "io.h"
#include "console.h"
#include "fsopen.h"


LOCAL  char   fmt2[]  = "[%d]\n";
LOCAL  char   promptdev[] = {'A','B','C','D','?'};

#define NUSERS 2
struct  prompt {
        struct shvars *shl;
        struct session *usr;
        struct tty *ptty;
        int     sav_Fcolor;
        int     sav_Bcolor;
        int     uid;
        char    shmode;
};

/*----------------------------------------------------------------------
 * MX shell
 *----------------------------------------------------------------------
 */
m_Spy(argc,argv)
int argc;
char *argv[];
{
    struct shvars *Shl;
    struct shvars shv;
    struct session *usr;
    struct tty *ptty;
    struct prompt prompt;
    int  ps, ntokens, i, j, k ,len,cp,sav_Fcolor,sav_Bcolor;
    char *outnam, *innam ,*frompipe;
    char pipebuf[SHBUFLEN];
    char bufecho[80];
    int  ifrom;
    int  fdin, fdout, fderr, com1 , com2 , status;
    int  fdpipe[2];
    Bool backgnd,ispipe;
    char ch,shmode;
    int  shelldev;
    int  child , father, mode, append, uid, h, session;
    int _spyExit();
    ulong nbytes;

    ps = _itDis();
    uid = Tasktab[m_Getpid()].tgrp;
    ptty = &tty[Tasktab[m_Getpid()].tfd[1]->s_minor];

    Shl = &shv;
    Shl->shncmds  = nb_command();
    usr = &SessionTab[uid];

    if ((usr->u_currDev = Tasktab[RUNpid].tcurrdev) == NODEV)
         usr->u_currDev = UNKNWDEV;

    _setprio(m_Getpid(),15);
    if (argc > 1)
         shmode = (*(argv[1]+1));
    else
         shmode = 'i';

    if (shmode == 'i') {
        /* afficher memoire disponible */
        nbytes   = FP_SEG(maxaddr) - FP_SEG(debaddr);
        nbytes   = (nbytes + 1) * 16;

        m_Printf(shBanStr, nbytes);
        sav_Bcolor = ptty->ansiB;
        sav_Fcolor = ptty->ansiF;

/*      if (adapter != MONO) {
            sav_Bcolor = ptty->ansiB;
            sav_Fcolor = ptty->ansiF;
        }*/
        m_Signal(SIGQUIT,_spyExit);
    }

    /* init prompt */
    prompt.shl        = Shl;
    prompt.usr        = usr;
    prompt.ptty       = ptty;
    prompt.sav_Bcolor = sav_Bcolor;
    prompt.sav_Fcolor = sav_Fcolor;
    prompt.uid        = uid;
    prompt.shmode     = shmode;
    session = _getSessionHandle();

    /* boucle principale du SHELL */
    while (TRUE) {

          /* ecrire prompt */
          _writePrompt(&prompt, session);

          /* faire echo des caracteres tapes pendant la precedante CMD */
          if ((cp = m_Ioctl(0,TCICPY,bufecho)) > 0)
               m_Write(1,bufecho,cp) ;
          m_Ioctl(0,TCECHO);

          /* lire la commande */
          if (shmode == 'i') {
              len = m_Read(stdin, Shl->shbuf, SHBUFLEN);
/*              do {
                ch = m_getche();
                Shl->shbuf[len] = ch;
                len++;
              }
              while (ch != '\n');*/

              /* tester si fin de shell */
              if (Shl->shbuf[len-1] == EOF) {
                  m_Printf(shExitStr);
                  _itRes(ps);
                  return(ROK);
              }
              Shl->shbuf[len - 1] = NULLCH; /* a la place de CR */
          }
          else
              strcpy(Shl->shbuf,argv[argc-1]);

         /* decomposer la ligne de commande en tokens */
         if ( (ntokens = lexan(Shl)) == RERR) {
               m_Printf(errhd);
               continue;
         }
         else
              if (ntokens == 0)      continue;

         outnam  = innam  = frompipe = NULL;
         backgnd = ispipe = FALSE;
         com2    = ifrom  = 0;

         /* gestion caractere background '&' */
         if (Shl->shtktyp[ntokens-1] == '&') {
             Shl->shtktyp[ntokens-1] = ' ';
             ntokens--;
             backgnd = TRUE;
         }

         /* prise en compte des redirections et pipe */
         for (len = 0, i = 0; i < ntokens;)  {

              if ((ch = Shl->shtktyp[i]) == '&') {
                   ntokens = -1;
                   break;
              }
              if (ch == '|') {
                   ispipe   = TRUE;
                   Shl->shtok[i]   = NULL;
                   frompipe        = Shl->shtok[i+1];
                   ifrom           = ++i;
                   continue;
              }
              if (ch == '>') {
                  if (outnam != NULL || i >= --ntokens) {
                      ntokens = -1;
                      break;
                  }
                  if (Shl->shtktyp[i+1] == '>') {
                      mode = O_APPEND;
                      append = 1;
                  }
                  else {
                      mode = O_TRUNC;
                      append = 0;      /*  append gere le shuntage des
                                        *  redirections > (0) ou >> (1)
                                        */
                  }
                  outnam = Shl->shtok[i+1+append]; /* redirection en sortie */
                  for (ntokens-=(1+append), j = i; j < ntokens; j++) {
                       /* shunter les tokens '>' et stdout */
                       Shl->shtktyp[j] = Shl->shtktyp[j+2+append];
                       Shl->shtok  [j] = Shl->shtok [j+2+append];
                  }
                  continue;
              }
              if (ch == '<') {
                  if (innam != NULL || i >= --ntokens) {
                      ntokens = -1;
                      break;
                  }
                  innam = Shl->shtok[i+1];
                  for (ntokens--, j = i; j < ntokens; j++) {
                       Shl->shtktyp[j] = Shl->shtktyp[j+2];
                       Shl->shtok  [j] = Shl->shtok [j+2];
                  }
                  continue;
              }
              else
                  len += strlen(Shl->shtok[i++]);
         }

         /* a ce stade , les tokens sont isoles et les E/S sont connues */
         if (ntokens <= 0) {
             m_Printf(errhd);
             continue;
         }

        /* recherche dans la table de commandes */
        for (com1 = 0; com1 < Shl->shncmds;com1++)
             if (!strcmp(Commandtab[com1].cmdName, Shl->shtok[0]))  break;

        if (frompipe != NULL)
            for (com2 = 0; com2 < Shl->shncmds;com2++)
                 if (!strcmp(Commandtab[com2].cmdName,frompipe))
                      break;

        if (com1 >= Shl->shncmds) {
            m_Printf(shNotFStr, Shl->shtok[0]);
            continue;
        }
        if (com2 >= Shl->shncmds) {
            m_Printf(shNotFStr,frompipe);
            continue;
        }

        /* lancer les commandes de type "simple appel" */
        if (Commandtab[com1].justcall) {
            if (innam != NULL || outnam != NULL || backgnd || ispipe)
                m_Printf(errhd);
            else
                if ((*Commandtab[com1].command)(ntokens , Shl->shtok) == SHEXIT)
                      break;
            continue;
        }

        /* ouvrir les fichiers de redirection si besoin */
        fdin = fdout = fderr = -1;
        if (innam != NULL) {      /* redirection en entree */
            if ((fdin = m_Open(innam,O_RDONLY)) == RERR) {
                 m_Printf(fmt,innam);
                 continue;
            }
        }
        if (outnam != NULL) {     /* redirection en sortie */
            if ((fdout = m_Open(outnam,O_WRONLY)) == RERR)  {
                 if ((fdout = m_Creat(outnam,S_IREAD|S_IWRITE)) == RERR) {
                      m_Printf( fmt, outnam);
                      continue;
                 }
            }
        };

        Shl->shtok[ntokens] = NULLPTR;

        if ((child = m_Fork()) == 0)
        {
             /* redefinir les E/S standards si besoin */
             if (fdin > 0) {
                 m_Close(0);
                 m_Dup(fdin);
                 m_Close(fdin);
             }
             if (fdout > 0) {
                 m_Close(1);
                 m_Dup(fdout);
                 m_Close(fdout);
             }
             /* creer PIPE si besoin */
             if (ispipe)
             {
                 m_Signal(SIGCLD,SIG_IGN);
                 m_Pipe(fdpipe);
                 if (m_Fork() == 0)
                 {
                     /* rediriger stdout dans le PIPE */
                     m_Close(1);
                     m_Dup(fdpipe[1]);
                     m_Close(fdpipe[1]);
                     m_Close(fdpipe[0]);

                     /* lancer la tache "topipe" */
                     m_Exec(Commandtab[com1].command, &Shl->shtok[0]);
/*                   m_Printf("\nExec Fails in SHELL \"TOPIPE\"\n");*/
                 }
                 /* rediriger stdin par le PIPE */
                 m_Close(0);
                 m_Dup(fdpipe[0]);
                 m_Close(fdpipe[0]);
                 m_Close(fdpipe[1]);

                 /* lancer la tache "frompipe" */
                 m_Exec(Commandtab[com2].command,&Shl->shtok[ifrom]);
/*               m_Printf("\nExec Fails in SHELL \"FROMPIPE\"\n");*/
             }
             /* si tache en avant plan, donner possibilit‚ de suppression */
             if (!backgnd) m_Signal(SIGQUIT,_spyExit);
             m_Exec(Commandtab[com1].command,&Shl->shtok[0]);
/*           m_Printf("\nExec Fails in SHELL\n");*/
        }
        if (backgnd)
            m_Printf(fmt2,child);
        else
            m_Wait(&status);            /* attendre Fin Du fils */

        /*  fermer les fichiers de redirection ( s'ils n'ont pas ete
         *  ouverts le CLOSE est inactif)
         */

        m_Close(fdin);
        m_Close(fdout);
        if (shmode != 'i') break;

    }
    _itRes(ps);
    return(ROK);
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
    int  ntok;
    char *p;
    char ch;
    char *to;
    char quote;
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
              if (ch != quote)       return(RERR);
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
         *to++ = NULLCH;
    }
    return(ntok);
}


/*----------------------------------------------------------------------
 * _spyExit - Fin shell sur apparition du signal SIGQUIT
 *----------------------------------------------------------------------
 */
_spyExit()
{
extern char *m_GetProcName();
   m_Printf(shExitSIGQUITStr,m_GetProcName(m_Getpid()));
   m_Exit(0);
}

/*----------------------------------------------------------------------
 * _writePrompt
 *----------------------------------------------------------------------
 */
_writePrompt(pr, session)
struct prompt *pr;
{

     m_Printf("vs%d# ", session);

}
