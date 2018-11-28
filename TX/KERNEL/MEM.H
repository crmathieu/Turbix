/* mem.h */

/* free block header definition */
struct     mblock    {
           struct    mblock  *mnext ;     /* next free block */
           unsigned  mlen ;               /* block size      */
};

/* 
 * memory used by parameter located just before the allocated block itself
 */
struct     hblk  {
      unsigned     sig;       /* Signature header block
                               * 0x1956 = normal block
                               * 0x0412 = special block (buffer Session)
                               */
      unsigned     blen;      /* allocated block size (in click) */
      struct hblk *nextBlk;   /* next task block */
      struct hblk *prevBlk;   /* previous task block */
      struct hblk *dupb;      /* used when block was duplicated during a fork call */
      struct hblk *nextMsg;   /* prochain "message en attente" du process */
};

#define MAXCLICKS    4094      /* maximum number of possible clicks for 1 seg */
#define CLICK_SHIFT  0020
#define CLICK_SIZE   4
#define HBLK_SIZE    (sizeof(struct hblk)/16)+(sizeof(struct hblk)%16?1:0)
#define NORMALB      0x1956   /* normal block signature  */
#define SPECIALB 0x0412       /* special block signature */
#define STACKB       0x1234   /* stack type of block     */

/* memory management macros */
#define truncew(x)   (int *)( (int)(x) >> 4)
#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

extern     struct mblock memlist;    /* free zone list head        */
extern     int  far *maxaddr;        /* memory max address         */
extern     int  far *debaddr;        /* beginning of free address  */

/* C compiler variables */
extern     int end;
extern     int edata;
extern     int etext;

/* memory allocation functions ( newmeml.c )  */
extern int  far *_stackAlloc();
extern int  far *mkdMalloc();
extern int  far *_XSmalloc();
extern int  far *_MXmalloc();
extern int  far *Xmalloc64();
