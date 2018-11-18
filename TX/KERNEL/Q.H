/* q.h */

#define NSYS        NTASK + (2 * NSEM) + 6   /* 6 = files RDY PIPE LOCK */
#define NCLK        NTASK + 2                /* 2 = file des delais */

/*
 * entree dans la liste de controle des taches
 * une pour chaque tache + deux pour chaque liste
 *
 */

struct node   {
       short  key;         /* cle determinant l'ordre dans la queue       */
       short  next;        /* pointe sur le prochain process ou sur TAIL  */
       short  prev;        /* pointe sur le process precedant ou sur HEAD */
};

extern struct node Sysq[];
extern struct node Clkq[];
extern int    nextqueue;

/* macros de manipulation de Sysq et Clkq */
#define isempty(list)  (Sysq[(list)].next >= NTASK)
#define nonempty(list) (Sysq[(list)].next <  NTASK)
#define firstkey(list) (Sysq[Sysq[(list)].next].key)
#define lastkey(tail)  (Sysq[Sysq[(tail)].prev].key)
#define firstid(list)  (Sysq[(list)].next)
#define nextid(id)     (Sysq[(id)].next)

/* D comme DELAY LIST */
#define isemptyD       (Clkq[NTASK].next >= NTASK)
#define nonemptyD      (Clkq[NTASK].next <  NTASK)
#define firstkeyD      (Clkq[Clkq[NTASK].next].key)
#define lastkeyD       (Clkq[Clkq[NTASK+1].prev].key)
#define firstidD       (Clkq[NTASK].next)
#define nextidD(id)    (Clkq[(id)].next)