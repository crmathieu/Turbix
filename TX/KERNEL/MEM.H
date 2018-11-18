/* mem.h */

/* definition  d'une entete de bloc libre */
struct     mblock    {
           struct    mblock  *mnext ;     /* prochain bloc libre */
           unsigned  mlen ;               /* taille du bloc      */
};

/* definition d'un header de bloc :
 * zone m‚moire d'un PARA situ‚ juste
 * avant le bloc allou‚ lui mˆme
 */
struct     hblk  {
      unsigned     sig;       /* Signatures header de block
                               * 0x1956 = normal block
                               * 0x0412 = special block (buffer Session)
                               */
      unsigned     blen;      /* taille du bloc allou‚ (en click) */
      struct hblk *nextBlk;   /* prochain bloc du process */
      struct hblk *prevBlk;   /* bloc precedent du process */
      struct hblk *dupb;      /* si bloc dupliqu‚ lors d'un fork */
      struct hblk *nextMsg;   /* prochain "message en attente" du process */
};

#define MAXCLICKS    4094      /* maximum de clicks allouables pour 1 seg */
#define CLICK_SHIFT  0020
#define CLICK_SIZE   4
#define HBLK_SIZE    (sizeof(struct hblk)/16)+(sizeof(struct hblk)%16?1:0)
#define NORMALB      0x1956   /* signature block normal  */
#define SPECIALB     0x0412   /* signature special block */
#define STACKB       0x1234   /* bloc de type pile       */

/* macros de gestion memoire */
#define truncew(x)   (int *)( (int)(x) >> 4)
#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

extern     struct mblock memlist;      /* tete de la liste des zones libres */
extern     int  far *maxaddr;        /* adresse maxi de la memoire        */
extern     int  far *debaddr;        /* debut d'adresse libre             */

/* variables du compilateur C */
extern     int end;
extern     int edata;
extern     int etext;

/* fonctions d'allocation memoire ( newmeml.c )  */
extern int  far *_stackAlloc();
extern int  far *mkdMalloc();
extern int  far *_XSmalloc();
extern int  far *_MXmalloc();
extern int  far *Xmalloc64();
