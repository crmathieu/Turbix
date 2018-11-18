/* kernel.h */


/*
 *          Constantes Noyau
 *
 */

#define      FALSE        0             /* constantes booleennes */
#define      TRUE         1
#define      NULL         (char *)0     /* pointeur nul ( pour les listes ch */
#define      NULLCH       '\0'          /* caractere null      */
#define      NULLSTR      ""            /* pointeur de chaine vide  */
#define      SYSTEMCALL                 /* declaration d'appel systeme */
#define      LOCAL        static        /* declaration de procedure locale */
#define      SHELLCOM                   /* declaration des commandes shell  */
#define      BUILTIN      int           /* commande residente de shell */
#define      INTERRUPT                  /* declaration de procedure d'interr */
#define      TASK                       /* declaration de process */
#define      BIBLIO                     /* declaration de fonction de biblio*/
#define      SCHEDUL      1             /* rescheduler */
#define      NOSCHEDUL    0             /* ne pas rescheduler */
#define      DIFFERED     NOSCHEDUL
#define      IMMEDIAT     SCHEDUL
#define      MININT       0100000       /* + petit entier ( -32768 ) */
#define      MAXINT       0077777       /* + grand entier ( +32767 ) */
#define      EMPTY       -1             /* constante liste vide      */
#define      SYSQ         412           /* table systeme             */
#define      CLKQ         56            /* table delais              */

#define      TASK0STK     512           /* taille de la pile du process 0 */
#define      MAGIC        0125252       /* valeur pour le debut de pile */

#define      ROK          0
#define      RERR        -1
#define      EOF         -1             /* End-Of-File (utilise pour read) */

#define      MAJOR        8
#define      MINOR        0
#define      MAX_P_LONG   2147483647

#define      UMAIN_TASK   0          /* Umain */
#define      USER_TASK    1          /* tache utilisateur */
#define      SYS_TASK     2          /* tache systeme     */

#define      MAX_PATH     128
#define      HEAD_TO_TAIL 0
#define      TAIL_TO_HEAD 1

/* constantes diverses */
#define     CLEAN        0
#define     FNAMLENGTH   8
#define     FEXTLENGTH   3
#define     UNKNWDEV     4
#define     MAXSTR       80
#define     DIRTY        1
#define     NODEV        1956
#define     MAXPRIO      255

/* MACROS utilitaires */

#define      isodd(x)    (01&(int)(x))
#define      min(a, b)   ((a) < (b) ? (a) : (b))
#define      max(a, b)   ((a) > (b) ? (a) : (b))

extern       int         rdyhead , rdytail ;
extern       int         preempt ;
