/* signal.c */
#include "sys.h"
#include "xsignal.h"
#include "io.h"
#include "q.h"

/*
 * REM :  le systeme de message est impossible avec le process null,
 *        car le pid 0  est considere comme invalide !
 */


/*----------------------------------------------------------------------------
 * m_Signal - armer le mecanisme de signalisation avec l'action a appliquer
 *          lorsque l'evenement apparaitra - retourne la precedante action
 *----------------------------------------------------------------------------
 */
SYSTEMCALL int (* m_Signal(signum,action))()
int signum;                      /* numero de l'evenement       */
int (*action)();                 /* fonction gerant l'evenement */
{
    int ps;
    struct taskslot *tp;
    int (* prevAction)();

    /*  si Action = SIG_IGN  mettre bit a 0 dans evt attendu
     *  (ne pas modifier le contenu du champ action correspondant)
     */

    ps = _itDis();
    tp = &Tasktab[RUNpid];
    tp->terrno = 0;
    if (isbadsig(signum)) {
        tp->terrno = EINVAL;
        _itRes(ps);
        return((int (* )())RERR);
    }
    prevAction = tp->tevfunc[signum];
    if (action == SIG_IGN) {
        tp->tevsig &= (~SigMask[signum]);
        _itRes(ps);
        return(prevAction);
    }
    tp->tevfunc[signum]  = action;
    tp->tevsig          |= SigMask[signum];

    _itRes(ps);
    return(prevAction);
}


/*----------------------------------------------------------------------------
 * _sendsig - envoyer un signal a un process
 *----------------------------------------------------------------------------
 */
_sendsig(targetpid,signum,status,childpid)
int targetpid;      /*  pid de la tache a qui l'evenement est destine */
int signum;
int status;
int childpid;   /*  status et childpid sont utilises uniquement
                 *  pour l'evenement SIGCLD
                 */
{

    struct taskslot *tp;
    int ps,i,_anormal_end();
    int (* function)();

    ps = _itDis();

    if (((tp = &Tasktab[targetpid])->tstate == UNUSED) ||
         (targetpid == BADPID) || isbadsig(signum)) {
/*       m_printf("\nSENDSIG : pid = %d  state = %d  signum = %d\n",targetpid,tp->tstate,signum);
         m_printf("\npere = %s\n",tp->tname);*/
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return(RERR);
    }
    Tasktab[RUNpid].terrno = 0;
    if (signum == SIGCLD) {
        /*  accomplir DEATH OF CHILD si l'action du
         *  pere est SIG_DFL ou si le pere est la
         *  tache IDLE (qui ignore tous les signaux)
         */

         if ((tp->tevfunc[SIGCLD] == SIG_DFL) || (targetpid == TASK0))
             _death_of_child(childpid,targetpid,status);
         else {
/*            m_printf("\nAVANT PUTACTION\n");*/
              _putAction(targetpid,SIGCLD,tp->tevfunc[SIGCLD]);
         }
         _itRes(ps);
         return(ROK);
    }
    else {
         /* verifier que l'evenement n'est pas ignore, sauf si on a
          * invoqu‚ PAUSE()
          */
         if (tp->tstate == SLEEP && (tp->tevent & EV_PAUSE))
                ;
        /* {
                _setrdy(targetpid, NOSCHEDUL);
                _itRes(ps);
                return(ROK);
         }*/
         else /**/
         if ((tp->tevsig & (SigMask[signum])) == 0) { /* ignore ! */
/*              m_printf("\nSIGNAL IGNORE par tache %s signum = %d\n",tp->tname,signum);*/
                _itRes(ps);
                return(ROK);  /* ignorer l'evt */
         }

         /*  l'evenement n'est pas ignore :
          *  positionner flag dans bitmap d'ev arrives et
          *  desarmer flag d'attente d'ev
          */

         tp->tevcatch |=  SigMask[signum];
         tp->tevsig   &= ~SigMask[signum];

         /*  si DFL : TUER le process cible */

         if ((function = tp->tevfunc[signum]) == SIG_DFL) {
              function = _anormal_end;
/*          m_printf("\nDFL fonction pour %s\n",Tasktab[targetpid].tname);*/
         }

         /*  enrichir le contexte empil‚ avec la fonction
          *  a executer - verifier si la tache cible n'est pas
          *  la tache courante (cas des signaux envoyes sous it
          *  a la tache courante)
          */

          _putAction(targetpid,signum,function);
          _itRes(ps);
          return(ROK);
    }
}

_anormal_end()
{
   m_Exit(-1);
}

/*----------------------------------------------------------------------------
 * _putAction - mettre en place le contexte de l'action a executer sur la pile
 *----------------------------------------------------------------------------
 */
_putAction(target,signum,action)
int target;
int signum;
int (* action)();
{
    struct taskslot *tp;
    int  i;
    int *to , *from;
    tp = &Tasktab[target];

    if (tp->tstate == SLEEP && (tp->tevent & EV_PAUSE))
        _setrdy(target,NOSCHEDUL);

    if (target == RUNpid) {
/*      m_printf("\nlancer ACTION\n");*/
        (*action)();   /* lancer l'action */
    }
    else {
/*      m_printf("\npreparer ACTION : RUNpid = %d TARGET = %d\n",m_Getpid(),target);*/
        from = to = tp->tstktop;
        to -= sizeof(int (*)())/2;
        for (i=0 ; i<4 ; i++)
             *to++ = *from++;

        *(int (**)())to = action;
        tp->tstktop  -= sizeof(int (*)())/2; /*4;*/
        tp->tevfunc[signum] = SIG_DFL;
    }
}


/*----------------------------------------------------------------------------
 * _death_of_child - gestion du signal SIGCLD
 *----------------------------------------------------------------------------
 */
_death_of_child(pid,pere,status)
int pid,pere,status;
{
    struct taskslot *tp, *any;
    int              ps, i, d, mypid;
    int             *to, *from;

           /*  A) faire adopter les fils par IDLE
            *     qui ignore tous les signaux
            */

/*         m_printf("\nDEBUT DOC\n");*/
           for (i = 0 ; i < NTASK ; i++) {
                if ((any = &Tasktab[i])->tstate != UNUSED  &&
                     any->tppid == pid) {  /* fils detect‚ */
                     /* eliminer les zombi */
/*                               m_Printf("Fils DETECTE ...\n");*/
                                   if ((any->tstate == SLEEP) && (any->tevent & EV_ZOM)) {
/*                                           m_Printf("ZOMBI\n");*/
/*                        any->tstate = UNUSED;*/
                                 _doTerminate(i);
                                   }
                                   else {
/*                                           m_Printf("ADOPTE PAR IDLE\n");*/
                                 any->tppid = TASK0;    /* adopte par IDLE !   */
                                   }
                }
           }

           /*  B) prevenir le pere que la mort
            *     est proche si celui ci n'ignore
            *     pas l'evenement
            */

           Tasktab[pid].texitstatus = status;
           if (((tp = &Tasktab[pere])->tstate == SLEEP) &&
                (tp->tevent & EV_PAUSE)) {
                _setrdy(pere,NOSCHEDUL);
                _dokill(pid);
                return;
           }
/*         m_printf("\nDOC : avant test SIGCLD IGN\n");*/
           d = ((tp = &Tasktab[pere])->tevsig & SigMask[SIGCLD]);
           if (d == 0) {
                /* l'evenement est ignore */
/*              m_printf("DOC : ev IGN -- pere = %s\n",tp->tname);*/
                Tasktab[pid].texitstatus = status;
                if (tp->tflag & F_WAIT) {
                    /* le pere a fait un WAIT et est en attente de la mort
                     *  de tous ses fils
                     */
/*                  m_printf("le pere etait en attente sur EV IGN\n");*/
                    _doZombi(pid);
                    _setrdy(pere,SCHEDUL); /* relancer pere dans sa boucle
                                            * de WAIT
                                            */
                }
                else {
                    /* se tuer sans attendre WAIT du pere */
/*                  m_printf("se flinguer car le pere n'attend pas\n");*/
                    _dokill(pid);
                }
           }
           else {
                /* l'evenement n'est pas ignore */
/*                   m_printf("SIGCLD non IGN -- pere = %s -- bitmap = %d\n",tp->tname,d);*/
                if (tp->tflag & F_WAIT) {
                    /* le pere a fait un WAIT et redemarre sur
                     * l'apparition du 1er ZOMBI venu
                     */
/*                              m_printf("pere en attente sur EV NON IGN\n");*/
                           _doZombi(pid);
                    _setrdy(pere, SCHEDUL);
                }
                else {
                    if (tp->tflag & F_WAITED) {
                        /* le pere est deja sorti de son WAIT
                         *  (un ZOMBI l'a deja reveille) : se tuer
                         */
/*                                  m_printf("le pere n'est PLUS en attente\n");*/
/*                      _swpProc();*/
/*                      m_printf("Apres attente du FILS\n");*/
                        /* ATTENTION COUILLE */
                        if (tp->tflag & F_WAIT) { /* cas du wait en boucle */
/*                                             m_printf("pere apres attente du fils\n");*/
                            _doZombi(pid);
                            _setrdy(pere,SCHEDUL);
                        }
                        else
                            _dokill(pid);
                    }
                    else {
                        /* le pere ne s'est pas encore mis en WAIT :
                         * se mettre en ZOMBI en attendant que
                         * le pere se mette en WAIT
                         */
/*                      m_printf("\nle pere n'est PAS en attente\n");*/
                        _doZombi(pid);
                        _swpProc();
                    }
                }
           }
}


/*----------------------------------------------------------------------------
 * m_Wait - synchronisation entre tache mere et fille(s)
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Wait(status)
int *status;
{
   /*
    *  WAIT elimine dans un 1er temps tous les fils zombi, puis
    *  verifie si le signal SIGCLD est ignore : si OUI , WAIT attend
    *  la mort de tous les autres fils avant de redonner la main au pere
    *  si NON , WAIT redonne la main au pere si au moins un zombi a ete
    *  trouve ( si aucun zombi detecte : attendre zombi - si pas de fils
    *  alors ERREUR)
    */

   int ps,children,first_time,zombi,i,pere,firstZ,lastZ;
   struct taskslot *tp;
      ps = _itDis();
      tp = &Tasktab[pere = RUNpid];

      tp->terrno = 0;

      tp->tflag |= F_WAIT;
      tp->tflag &= ~F_WAITED;
      for (;;)
      {
/*        m_printf("WAIT\n");*/
          children = zombi = 0;

          for (i = NSYSTASK; i < NTASK ; i++)  {

             if (Tasktab[i].tstate == UNUSED)  continue;
             if (Tasktab[i].tppid == pere)    /* un fils detecte */
             {
/*                m_printf("\nfils = %s\n",Tasktab[i].tname);*/
                /* tuer tous les fils ZOMBI */
                if (Tasktab[i].tstate == SLEEP && (Tasktab[i].tevent & EV_ZOM)) {
                    if (zombi == 0) {
                       *status = Tasktab[i].texitstatus;
                        firstZ = i;
                    }
                    lastZ = i;
                    _doTerminate(i);
                    /*Tasktab[i].tstate = UNUSED;*/
                    zombi++;
                }
                else
                    children++;
             }
          }
/*          m_printf("\nWAIT : apres genocide\n");*/
          if (zombi != 0) {

             if ((tp->tevsig & SigMask[SIGCLD]) == 0) {

                 /*  l'evenement est ignore : attendre la
                  *  mort de tous les fils s'il en reste
                  */
/*                 m_printf("\nWAIT : ZOMBI + SIGCLD IGNORE\n");*/
                 if (children == 0) {
                     tp->tflag &= ~F_WAIT;
                     tp->tflag |=  F_WAITED;
                     _itRes(ps);
                     return(lastZ);
                 }
                 else {

                     tp->tstate = SLEEP;
                     tp->tevent = EV_WAIT;
                     _swpProc();
                     m_printf("\nle pere redemarre en ZOMBI\n");
                 }
             }
             else {

                 /*  l'evenement n'est pas ignore : retourner
                  *  pid du 1er Zombi tue
                  */

/*               m_printf("\nWAIT : SIGCLD non IGN\n");*/
                 tp->tflag &= ~F_WAIT;
                 tp->tflag |=  F_WAITED;
                 _itRes(ps);
                 return(firstZ);
             }
          }
          else {   /* pas de zombi */

             if (children == 0) {
                 tp->tflag &= ~F_WAIT;
                 tp->tflag |=  F_WAITED;
                 tp->terrno = ECHILD;
                 _itRes(ps);
                 return(RERR);
             }
             else {

                 tp->tstate = SLEEP;
                 tp->tevent = EV_WAIT;
                 _swpProc();
/*               m_printf("\nle pere redemarre en NON ZOMBI\n");*/
             }
          }
      }
}


/*----------------------------------------------------------------------------
 * m_Pause - se placer en SLEEP et repartir sur l'arrivee d'un ev quelconque
 *----------------------------------------------------------------------------
 */
SYSTEMCALL m_Pause()
{
  int ps;
  struct taskslot *pptr;

  ps = _itDis();
  pptr = &Tasktab[RUNpid];
  pptr->tstate  = SLEEP;
  pptr->tevent |= EV_PAUSE;
  _swpProc();
  _itRes(ps);
}




