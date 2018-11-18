/* const.h */


#define      FALSE        0
#define      TRUE         1
#define      NULL         (char *)0
#define      NULLPTR      (char *)0
#define      NULLCH       '\0'
#define      NULLSTR      ""

#define      ROK          0
#define      RERR        -1

/* MACROS */
#define      isodd(x)    (01&(int)(x))
#define      min(a, b)   ((a) < (b) ? (a) : (b))
#define      max(a, b)   ((a) > (b) ? (a) : (b))
