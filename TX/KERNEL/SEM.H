/* sem.h */

/*
 * entree dans la table des semaphores
 *
 */

struct     semslot     {
           char       sstate ;    /* SFREE ou SUSED                      */
           char       sname[8];   /* nom du semaph                       */
           short      semcnt ;    /* compteur du semaphore               */
           short      sqhead ;    /* pointeur sur la tete de liste       */
           short      sqtail ;    /* pointeur sur la queue de liste      */
};

#define    NSEM       100         /* nb de semaphores par defaut         */
#define    SFREE      1           /* le semaphore est libre              */
#define    SUSED      2           /* "      "      "  utilise            */
#define    STAIL      3           /* mise en file en fin de liste        */
#define    SHEAD      4           /* mise en file en tete de liste       */

extern     struct     semslot      Semtab[] ;
extern     int        nextsem ;

#define    isbadsem(s)   (s<0 || s>=NSEM)
