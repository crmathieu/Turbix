/* kernel.h */


/*
 *          Kernel constant
 *
 */

#define      FALSE        0             
#define      TRUE         1
#define      NULL         (char *)0     /* Null pointer */
#define      NULLCH       '\0'          /* Null character */
#define      NULLSTR      ""            /* empty string pointer */
#define      SYSTEMCALL                 /* System call declarative */
#define      LOCAL        static        /* Static call declarative */
#define      SHELLCOM                   /* Shell commands declarative */
#define      BUILTIN      int           /* Buil-tin shell command declarative */
#define      INTERRUPT                  /* Interrupt call declarative */
#define      TASK                       /* Task declarative */
#define      BIBLIO                     /* Library declarative */
#define      SCHEDUL      1             /* reschedule */
#define      NOSCHEDUL    0             /* do not reschedule */
#define      DIFFERED     NOSCHEDUL
#define      IMMEDIAT     SCHEDUL
#define      MININT       0100000       /* smallest int ( -32768 ) */
#define      MAXINT       0077777       /* greatest int ( +32767 ) */
#define      EMPTY       -1             /* empty list constant */
#define      SYSQ         412           /* system table */
#define      CLKQ         56            /* delays table */

#define      TASK0STK     512           /* task 0 stack size */
#define      MAGIC        0125252       /* Magic value that must be found at the beginning of stack */

#define      ROK          0
#define      RERR        -1
#define      EOF         -1             /* End-Of-File */

#define      MAJOR        8
#define      MINOR        0
#define      MAX_P_LONG   2147483647

#define      UMAIN_TASK   0          /* User main entry point task */
#define      USER_TASK    1          /* user task */
#define      SYS_TASK     2          /* system task */

#define      MAX_PATH     128
#define      HEAD_TO_TAIL 0
#define      TAIL_TO_HEAD 1

/* miscellaneous */
#define     CLEAN        0
#define     FNAMLENGTH   8
#define     FEXTLENGTH   3
#define     UNKNWDEV     4
#define     MAXSTR       80
#define     DIRTY        1
#define     NODEV        1956
#define     MAXPRIO      255

/* MACROS */

#define      isodd(x)    (01&(int)(x))
#define      min(a, b)   ((a) < (b) ? (a) : (b))
#define      max(a, b)   ((a) > (b) ? (a) : (b))

extern       int         rdyhead , rdytail ;
extern       int         preempt ;
