/* sys.c */

#include "sys.h"
#include "q.h"
#include "io.h"

/*----------------------------------------------------------------------------
 * _enlist_tail - insere un element a la fin d'une liste
 *----------------------------------------------------------------------------
 */
_enlist_tail(ltyp,pid, tail)
{

    if (ltyp == SYSQ) {
        Sysq[pid].next = tail;
        Sysq[pid].prev = Sysq[tail].prev;
        Sysq[Sysq[tail].prev].next = pid;
        Sysq[tail].prev = pid;
    }
    else {
        Clkq[pid].next = tail;
        Clkq[pid].prev = Clkq[tail].prev;
        Clkq[Clkq[tail].prev].next = pid;
        Clkq[tail].prev = pid;
    }
    return(pid);

}

/*----------------------------------------------------------------------------
 * _enlist_head - insere un element a la tete d'une liste sur Sysq uniquement
 *----------------------------------------------------------------------------
 */
_enlist_head(pid , head )
{
    struct node *hp;              /* pointeur sur la tete  */
    struct node *mp;              /* pointeur sur l'element */

    hp = &Sysq[head];
    mp = &Sysq[pid];

    mp->next = hp->next;
    mp->prev = head;
    Sysq[hp->next].prev = pid;
    hp->next = pid;
    return(pid);
}

/*----------------------------------------------------------------------------
 * _defect - supprime un element d'une liste et retourne
 *           son offset dans la table systeme
 *----------------------------------------------------------------------------
 */
_defect(ltyp,pid)
{
    if (ltyp == SYSQ) {
        Sysq[Sysq[pid].prev].next = Sysq[pid].next;
        Sysq[Sysq[pid].next].prev = Sysq[pid].prev;
    }
    else {
        Clkq[Clkq[pid].prev].next = Clkq[pid].next;
        Clkq[Clkq[pid].next].prev = Clkq[pid].prev;
    }
    return(pid);
}

/*----------------------------------------------------------------------------
 * _insertKey - insere une tache ds une liste par ordre de cle
 *           ( priorite ) croissante a partir de la tete de liste
 *----------------------------------------------------------------------------
 *//* INUTILISE
_insertKey ( proc , head , key )
int key;  /* la cle est la priorite */
/*{
    int next;
    int prev;

    next = Sysq[head].next;
    while (Sysq[next].key < key )
           next = Sysq[next].next;
    Sysq[proc].next = next ;
    Sysq[proc].prev = prev = Sysq[next].prev;
    Sysq[proc].key  = key;
    Sysq[prev].next = proc;
    Sysq[next].prev = proc;
    return(ROK);

}*/

/*----------------------------------------------------------------------------
 * _insertKeyFromTail - insere un process ds une liste par ordre de cle
 *                      ( priorite ) croissante de la queue vers la tete
 *----------------------------------------------------------------------------
 */
_insertKFR( pid , tail , key )
int key;          /* cle (priorite) utilisee pour l'ordre           */
{
    int next;
    int prev;

    prev = Sysq[tail].prev;
    while (Sysq[prev].key < key )
           prev = Sysq[prev].prev;
    Sysq[pid].prev  = prev ;
    Sysq[pid].next  = next = Sysq[prev].next;
    Sysq[pid].key   = key;
    Sysq[prev].next = pid;
    Sysq[next].prev = pid;
    return(ROK);

}

/*----------------------------------------------------------------------------
 * _getfirst - retire la tache en tete de liste et retourne son PID
 *----------------------------------------------------------------------------
 */
_getfirst(ltyp,head)
{
    int task_nr;           /* 1er process de la liste */

    if (ltyp == SYSQ)   task_nr = Sysq[head].next;
    else                task_nr = Clkq[head].next;

    if (task_nr < NTASK)
             return(_defect(ltyp,task_nr));
        else
             return(EMPTY);
}

/*----------------------------------------------------------------------------
 * _getlast - suprime la tache en queue de liste et retourne son PID
 *----------------------------------------------------------------------------
 */
_getlast(ltyp,tail)
{
    int task_nr;

    if (ltyp == SYSQ)   task_nr = Sysq[tail].next;
    else                task_nr = Clkq[tail].next;

    if (task_nr < NTASK)
          return(_defect(ltyp,task_nr));
    else
          return(EMPTY);
}

/*----------------------------------------------------------------------------
 * _makeList - initialise une nouvelle liste ds la table systeme
 *----------------------------------------------------------------------------
 */
_makeList(SensInsertion)
{
    struct node *hp;   /* adresse tete de liste                       */
    struct node *tp;   /* adresse queue de liste                      */
    int    head,tail;     /* offsets de la tete et la queue ds la Sysq */

    hp = &Sysq[head = nextqueue++];   /*  nextqueue est la variable globale
                                         *  qui gere les offsets des tetes et
                                         *  queues de liste ds la Sysq
                                         */

    tp        = &Sysq[tail = nextqueue++];
    hp->next = tail;
    hp->prev = EMPTY;
    tp->next = EMPTY;
    tp->prev = head;
    if (SensInsertion == HEAD_TO_TAIL) {
       hp->key  = MININT;
       tp->key  = MAXINT;
    }
    else {
       hp->key  = MAXINT;
       tp->key  = MININT;
    }
    return(head);
}
