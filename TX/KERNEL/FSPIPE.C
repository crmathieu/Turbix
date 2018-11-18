/* fspipe.c */


#include "sys.h"
#include "conf.h"
#include "error.h"
#include "io.h"
#include "const.h"
#include "q.h"

#include "fspipe.h"

/*---------------------------------------------------------------------------
 * _restartTask - libere les taches en attente sur un pipe pour une op donnee
 *---------------------------------------------------------------------------
 */
_restartTask(pp,op,pexit)
struct pipslot *pp;
int op,pexit;
{
       int next,next2;
       struct taskslot *tp;

       for (next = Sysq[piphead].next; next != piptail;) {
            next2 = Sysq[next].next;
            if ((tp = &Tasktab[next])->tpipe_nr == pp->pipe_nr &&
                                   tp->tpipe_op == op) {
                 tp->tpipe_nr = NOPIPE;
                 pp->susptask[op]--;
                 _defect(SYSQ,next);
                 _setrdy(next,NOSCHEDUL);
            }
            next = next2;
       }
       if (pexit == PEXIT)  {
           pp->invalid = TRUE;
/*           m_printf("\nRESTART: PIPE %d INVALIDE\n",pp->invalid);*/
       }
}


/*----------------------------------------------------------------------
 * Pread - lire a partir d'un pipe
 *----------------------------------------------------------------------
 */
_Pread(pp,buff,count)
struct pipslot *pp;      /* pointeur sur pipe entry */
uchar          *buff;
int             count;
{
    struct taskslot *tp;
    int ps,countI;

    /*  tester si caracteres a lire : si oui les
     *  lire et reveiller d'eventuelles taches en attente d'ecriture
     *  sur ce pipe - si non se mettre en attente de lecture
     */

    ps = _itDis();
    for (;;) {
         if (pp->avail != 0) {
/*             m_printf("READING proc\n");*/
             if (count > pp->avail)  count = pp->avail;
             countI     = count;
             pp->avail -= count;
             while (count-- > 0) {
                    *buff++ = *(pp->pipzon + pp->offR++);
                    if (pp->offR >= PIPESIZE) pp->offR = 0;
             }
             if (pp->susptask[PWRITE] > 0) {
/*                 m_printf("\nREAD PIPE : relancer WRITE\n");*/
                 _restartTask(pp,PWRITE,PRUN);
             }
/*             m_printf("READING char %c \n", *(buff-1));*/
             _itRes(ps);
             return(countI);
         }

        /*  pas de caracteres a lire : si pipe invalide
         *  arreter la lecture du PIPE , sinon se suspendre
         */

        if (pp->invalid == TRUE)  {
/*            m_printf("\nREADPIPE : pipe invalide\n");*/

/*            if (--pp->count <= 0) {
                m_Printf("\nclose pipe dans READ\n");
                _closepipe(pp);
            }                  */
            _itRes(ps);
            return(0);
        }

/*        m_printf("\nREAD PIPE %d : SUSP\n",pp->invalid);*/
        (tp = &Tasktab[RUNpid])->tpipe_nr = pp->pipe_nr;
        tp->tpipe_op = PREAD;
        tp->tstate   = SLEEP;
        tp->tevent  |= EV_PIPE;
        pp->susptask[PREAD]++;
        _enlist_tail(SYSQ,RUNpid,piptail);
        _swpProc();
    }
}

/*----------------------------------------------------------------------
 * Pwrite - ecrire dans un pipe
 *----------------------------------------------------------------------
 */
_Pwrite(pp,buff,count)
struct pipslot *pp;      /* pointeur sur pipe entry */
uchar          *buff;
int             count;
{
    struct taskslot *tp;
    int countI,countC,ps;

    /*  tester s'il y a assez de place pour ecrire tous les
     *  caracteres - si oui les ecrire et reveiller d'eventuelles
     *  taches en attente de lecture - si non ecrire dans ce qu'il
     *  reste comme place disponible et se suspendre pour les
     *  caracteres restant a ecrire
     */

    countI = count;
    ps = _itDis();
    for (;;) {
         if (pp->invalid)  {
/*             _sendsig(RUNpid,SIGPIPE);*/
/*             m_printf("\nWRITEP:invalid\n");*/
             getchar();
/*             if (--pp->count <= 0) {
                m_Printf("\nclose pipe dans WRITE\n");
                 _closepipe(pp);
             }*/
             _itRes(ps);
             return(0);
         }
/*         VS0printf("\nWRITE PIPE \n");*/
         if (PIPESIZE > pp->avail) {
             countC = count;
             if (count > PIPESIZE - pp->avail)
                 countC = PIPESIZE - pp->avail;
             pp->avail += countC;
             count     -= countC;
             while (countC-- > 0) {
                    *(pp->pipzon + pp->offW++) = *buff++;
                    if (pp->offW >= PIPESIZE) pp->offW = 0;
             }
             if (pp->susptask[PREAD] > 0) {
/*                VS0printf("\nWRITE PIPE : relancer READ\n");*/
                 _restartTask(pp,PREAD,PRUN);
             }
             if (count == 0)  {
                 _itRes(ps);
                 return(countI);
             }
        }
/*        VS0printf("\nWRITE PIPE : SUSP\n");*/
        (tp = &Tasktab[RUNpid])->tpipe_nr = pp->pipe_nr;
        tp->tpipe_op = PWRITE;
        tp->tstate   = SLEEP;
        tp->tevent  |= EV_PIPE;
        pp->susptask[PWRITE]++;
        _enlist_tail(SYSQ,RUNpid,piptail);
        _swpProc();
    }
}

/*----------------------------------------------------------------------
 * pipe - ouvrir un pipe : retourne 2 file descriptors
 *        * 1 pour la lecture du pipe
 *        * 1 pour l'ecriture dans le pipe
 *----------------------------------------------------------------------
 */
BIBLIO m_Pipe(fd)
int *fd;
{
    int ps;
    stream_entry  *sp0,*sp1;

    ps = _itDis();
    Tasktab[RUNpid].terrno = 0;
    if ((sp0 = _getstream()) == (stream_entry *)RERR)   {
/*         VS0printf("\nPIPE : ERROR 0\n");*/
         _itRes(ps);
         return(RERR);
    }
    if ((sp1 = _getstream()) == (stream_entry *)RERR) {
         sp0->s_count = 0;
/*         VS0printf("\nPIPE ERROR : 1\n");*/
         _itRes(ps);
         return(RERR);
    }

    if ((sp0->s_minor = _getpipe()) == RERR)
        goto PIPERR;

    sp1->s_minor = sp0->s_minor;

    /*  allouer 2 slots dans la fdesc table de la tache
     *  et chainer les 2 stream slot a la fdesc table
     *  de la tache
     */

     if ( (fd[0] = _getfd(RUNpid)) == RERR)
        goto PIPERR;

     Tasktab[RUNpid].tfd[fd[0]] = sp0;
     if ( (fd[1] = _getfd(RUNpid)) == RERR)
        goto PIPERR;

     Tasktab[RUNpid].tfd[fd[1]] = sp1;

     sp0->s_streamtyp           = PIPESTREAM;
     sp0->s_mode                = PREAD;
     sp1->s_streamtyp           = PIPESTREAM;
     sp1->s_mode                = PWRITE;
     _itRes(ps);
     return(ROK);

PIPERR:
     sp0->s_count = sp1->s_count = 0;
/*     sp0->s_count = sp1->s_count = 1;*/
/*     VS0printf("\nPIPE : ERROR 2\n");*/
     _itRes(ps);
     return(RERR);
}

/*----------------------------------------------------------------------
 * _closepipe - fermer proprement un pipe
 *----------------------------------------------------------------------
 */
_closepipe(pp)
struct pipslot *pp;
{
/*  m_printf("\nfunction CLOSEPIPE\n");*/
  pp->invalid = FALSE;
  pp->count   = 0;
  _xfree(pp->pipzon, RUNpid);
}


/*----------------------------------------------------------------------
 * _getpipe - initilaliser un nouveau pipe
 *----------------------------------------------------------------------
 */
_getpipe()
{
   int i;
   struct pipslot *pp;
   for (i = 0;i < NPIPE ; i++) {
        if ((pp = &Piptab[i])->count == 0) {
             if (((int *)pp->pipzon = _XSmalloc(PIPESIZE)) != (int *)NULL) {
                  pp->susptask[0] = pp->susptask[1] = 0;
                  pp->offR        = pp->offW        = 0;
                  pp->invalid     = FALSE;
                  pp->avail       = 0;
                  pp->count       = 2;  /* 1 pour READ et 1 pour WRITE */
                  return(i);
             }
             else {
                  m_printf(errOpenPipe);
                  return(RERR);
             }
        }
   }
   m_printf(noMorePipe);
   return(RERR);
}

/*----------------------------------------------------------------------
 * _pipecount - donne la valeur du pipe compteur
 *----------------------------------------------------------------------
 *//* INUTILISE
_pipeCount(fd)
int fd;
{
    stream_entry *sp;
    sp = Tasktab[RUNpid].tfd[fd];
    if (sp != NULLSTREAM)
        return(Piptab[sp->s_minor].count);
    else
        return(RERR);
}*/
