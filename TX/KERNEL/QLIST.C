/* sys.c */

#include "sys.h"
#include "q.h"
#include "io.h"

/*----------------------------------------------------------------------------
 * _enlist_tail - insert an element at the tail of a list
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
 * _enlist_head - insert an element at the head of a list on Sysq only
 *----------------------------------------------------------------------------
 */
_enlist_head(pid , head )
{
    struct node *hp;              /* head  */
    struct node *mp;              /* element */

    hp = &Sysq[head];
    mp = &Sysq[pid];

    mp->next = hp->next;
    mp->prev = head;
    Sysq[hp->next].prev = pid;
    hp->next = pid;
    return(pid);
}

/*----------------------------------------------------------------------------
 * _defect - removes an element from a list and 
 *           returns its offset in system table
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
 * _insertKeyFromTail - insert a task in a list in sorted order (ASC) using
 *                      priority as key, from tail to head
 *----------------------------------------------------------------------------
 */
_insertKFR( pid , tail , key )
int key;          /* key (priority) used           */
{
    int next;
    int prev;

    prev = Sysq[tail].prev;
    while (Sysq[prev].key < key ) {
        prev = Sysq[prev].prev;
    }
    Sysq[pid].prev  = prev ;
    Sysq[pid].next  = next = Sysq[prev].next;
    Sysq[pid].key   = key;
    Sysq[prev].next = pid;
    Sysq[next].prev = pid;
    return(ROK);
}

/*----------------------------------------------------------------------------
 * _getfirst - removes task at the head of list and returns its pid
 *----------------------------------------------------------------------------
 */
_getfirst(ltyp,head)
{
    int task_nr;           /* 1st task in list */

    if (ltyp == SYSQ) {
        task_nr = Sysq[head].next;
    } else {
        task_nr = Clkq[head].next;
    }
    if (task_nr < NTASK) {
        return(_defect(ltyp,task_nr));
    }
    return(EMPTY);
}

/*----------------------------------------------------------------------------
 * _getlast - removes task at the end of list and returns its pid
 *----------------------------------------------------------------------------
 */
_getlast(ltyp,tail)
{
    int task_nr;

    if (ltyp == SYSQ) {
        task_nr = Sysq[tail].next;
    } else {
        task_nr = Clkq[tail].next;
    }
    if (task_nr < NTASK) {
        return(_defect(ltyp,task_nr));
    }
    return(EMPTY);
}

/*----------------------------------------------------------------------------
 * _makeList - initialize a new list in system table
 *----------------------------------------------------------------------------
 */
_makeList(SensInsertion)
{
    struct node *hp;   /* address list head                */
    struct node *tp;   /* address list tail                */
    int    head,tail;  /* offsets of head and tail in Sysq */

    hp = &Sysq[head = nextqueue++]; /*  nextqueue is a global variable 
                                     *  handling head and tail offsets
                                     *  in Sysq
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