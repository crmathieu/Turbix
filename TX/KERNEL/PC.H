/* pc.h */

/* Do not modify registers numbers without updating them in cxtswp.asm */

#define  RSP    0
#define  RSS    1
#define  RFLG   2
#define  RCS    3
#define  RIP    4
#define  RES    5
#define  RDS    6
#define  RSI    7
#define  RDI    8
#define  RBP    9
#define  RDX    10
#define  RCX    11
#define  RBX    12
#define  RAX    13


/* MACROS */
#define ADPAIRE(x) (int)x &= 0xfffe

