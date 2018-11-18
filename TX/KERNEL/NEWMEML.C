/*  meml.c */

#include "sys.h"
#include "conf.h"
#include "io.h"

struct mblock memlist;
int  far *maxaddr;
int  far *debaddr;
extern int  *deb1;


/*----------------------------------------------------------------------
 * _link_block - place un bloc allou‚ sur la liste des blocs de la tache
 *----------------------------------------------------------------------
 */
LOCAL _link_block(pid, hb)
struct hblk *hb;
{
      struct taskslot *tp;

      /* se positionner en fin de liste */
      if ((tp = &Tasktab[pid])->theadblk == (struct hblk *)NULL) {
           tp->theadblk = hb;
           tp->ttailblk = hb;
           hb->prevBlk = (struct hblk *)NULL;
      }
      else {
           tp->ttailblk->nextBlk = hb;
           hb->prevBlk = tp->ttailblk;
           tp->ttailblk = hb;
      }
      hb->nextBlk = (struct hblk *)NULL;
}

/*----------------------------------------------------------------------
 * _unlink_block - retire le bloc a liberer de la liste des blocs de la tache
 *----------------------------------------------------------------------
 */
LOCAL _unlink_block(pid, hb)
struct hblk *hb;
{
      struct taskslot *tp;

      tp = &Tasktab[pid];

      /* retirer le bloc */
      if (hb->prevBlk != (struct hblk *)NULL)
          hb->prevBlk->nextBlk = hb->nextBlk;
      else
          tp->theadblk = hb->nextBlk;

      if (hb->nextBlk != (struct hblk *)NULL)
          hb->nextBlk->prevBlk = hb->prevBlk;
      else
          tp->ttailblk = hb->prevBlk;

}

/*----------------------------------------------------------------------
 * MXmalloc - alloue une zone memoire (adresse basse)
 *----------------------------------------------------------------------
 */
int  *_MXmalloc(pid, size, blockType)
unsigned pid, size, blockType;
{
    int ps;
    struct mblock *p, *q, *leftover;
    unsigned  *hb;
    unsigned long bound;
    unsigned nclicks,i;


    /* convertir en para */
    if ((nclicks = (size / 16) + ( size % 16 ?  1 : 0 )) == 0) {
        Tasktab[pid].terrno = EINVAL;
        return((int *)NULL);
    }

    /* reserver la place pour le header de bloc */
    nclicks += HBLK_SIZE;

    ps = _itDis();

    if ((memlist.mnext == (struct mblock *)NULL) || (nclicks > MAXCLICKS)) {
         Tasktab[pid].terrno = EINVAL; /* invalide arg */
         _itRes(ps);
         return((int  *)NULL);
    }

    for (q = &memlist, p = memlist.mnext; p != (struct mblock *)NULL; q = p, p = p->mnext)
         if (p->mlen == nclicks)  { /* matches */
              q->mnext = p->mnext;
              ((struct hblk *)p)->sig  = blockType; /* indiquer type */
              ((struct hblk *)p)->blen = nclicks; /* marquer la taille */
              ((struct hblk *)p)->nextBlk = (struct hblk *)NULL;
              ((struct hblk *)p)->dupb    = (struct hblk *)NULL;
              ((struct hblk *)p)->nextMsg = (struct hblk *)NULL;
              if (blockType == NORMALB)
                  _link_block(pid, p);   /* placer le buffer en liste des alloc Dyn */
              FP_SEG(p) += HBLK_SIZE;    /* augmenter de 1 para pour conserver le header */
              _itRes(ps);
              return( (int *)p );
         }
         else if (p->mlen > nclicks)
              {
                 bound = FP_SEG(p) + nclicks;
                 bound <<= 16;
                 leftover = (struct mblock *)bound;
                 q->mnext = leftover;
                 leftover->mnext = p->mnext;
                 leftover->mlen  = p->mlen - nclicks;
                 ((struct hblk *)p)->sig  = blockType; /* indiquer type */
                 ((struct hblk *)p)->blen = nclicks; /* marquer la taille */
                 ((struct hblk *)p)->nextBlk = (struct hblk *)NULL;
                 ((struct hblk *)p)->dupb    = (struct hblk *)NULL;
                 ((struct hblk *)p)->nextMsg = (struct hblk *)NULL;
                 if (blockType == NORMALB)
                     _link_block(pid, p);   /* placer le buffer en liste des alloc Dyn */
                 FP_SEG(p) += HBLK_SIZE;    /* augmenter de 1 para pour conserver le header */
                 _itRes(ps);
                 return( (int *)p );
              }
    /* pas assez de memoire */
    _itRes(ps);
    return((int  *)NULL);
}

/*----------------------------------------------------------------------
 * m_Malloc - alloue un bloc memoire de type NORMAL (adresse basse)
 *----------------------------------------------------------------------
 */
SYSTEMCALL int  *m_Malloc(size)
unsigned size;
{
   int *p;

   p = _MXmalloc(RUNpid, size, NORMALB);
   if (p != (int *)NULL)
        Tasktab[RUNpid].terrno = 0;
   return(p);
}

/*----------------------------------------------------------------------
 * XSmalloc - alloue un bloc memoire de type SESSION (adresse basse)
 *----------------------------------------------------------------------
 */
int  *_XSmalloc(size)
unsigned size;
{
      return(_MXmalloc(RUNpid, size, SPECIALB));
}

/*----------------------------------------------------------------------
 * m_Free - libere un bloc memoire
 *----------------------------------------------------------------------
 */
SYSTEMCALL m_Free(block)
struct mblock *block;
{
    int p;

    if ((p = _xfree(block, RUNpid)) == ROK)
        Tasktab[RUNpid].terrno = 0;
    return(p);
}

/*----------------------------------------------------------------------
 * _xfree - libere un bloc memoire pour une tache donnee
 *----------------------------------------------------------------------
 */
_xfree(block, pid)
struct mblock *block;
{
    int ps;
    struct mblock *p, *q;
    unsigned long top;
    unsigned sig, nclicks;

    /* se replacer sur le header */
    FP_SEG(block) -= HBLK_SIZE;

    /* verifier le type de bloc */
    if (((sig = ((struct hblk *)block)->sig) != NORMALB) &&
        (sig != SPECIALB) && (sig != STACKB)) {
        Tasktab[RUNpid].terrno = EINVMEM;
/*        kprintf("FREE : BAD BLOCK SIG\n");*/
        return(RERR);
    }

    /* recuperer la taille allouee */
    nclicks = ((struct hblk *)block)->blen;

    if ((FP_SEG(block) > FP_SEG(maxaddr)) ||
         FP_SEG(block) < FP_SEG(debaddr) ) {
         Tasktab[RUNpid].terrno = EINVMEM;
/*         kprintf("\nFREE : ERREUR de COHERENCE\n");*/
         return(RERR);
    }


    ps = _itDis();

    /* dechainer le bloc de sa file s'il est de type "normal" */
    if (((struct hblk *)block)->sig == NORMALB)
        _unlink_block(pid, block);

    /* inserer le bloc dans le liste des blocs libres */
    for (p = memlist.mnext, q = &memlist; (p != (struct mblock *)NULL) && (FP_SEG(p) < FP_SEG(block));
              q = p, p = p->mnext)  ;

    top = q->mlen + FP_SEG(q);
    top <<= 16;
    if ((FP_SEG(top) > FP_SEG(block)) && q != &memlist ||
              p != (struct mblock *)NULL && (nclicks + FP_SEG(block)) > FP_SEG(p) ) {
/*         kprintf("\nFREE : ERREUR 1\n");*/
         Tasktab[RUNpid].terrno = EINVMEM;
         _itRes(ps);
         return(RERR);
    }
    if (q != &memlist && FP_SEG(top) == FP_SEG(block) )
         q->mlen += nclicks;
    else
    {
         block->mlen = nclicks;
         block->mnext = p;
         q->mnext = block;
         q = block;
    }
    if ( (unsigned)(q->mlen + FP_SEG(q) ) == FP_SEG(p) ) {
         q->mlen += p->mlen;
         q->mnext = p->mnext;
    }
    _itRes(ps);
    return(ROK);
}

/*----------------------------------------------------------------------
 * m_AdjustPTR - rectifie la valeur d'un pointeur sur une zone dynamique
 *             apr‚s un FORK
 *----------------------------------------------------------------------
 */
SYSTEMCALL m_AdjustPTR(ptr)
void **ptr;
{
     struct hblk *p;

     p = *ptr;
     FP_SEG(p) -= HBLK_SIZE;
     Tasktab[RUNpid].terrno = 0;
     if ((p->sig != NORMALB) || ((p = p->dupb) == (struct hblk *)NULL)) {
          Tasktab[RUNpid].terrno = EINVAL;
          return(RERR);
     }

     FP_SEG(p) += HBLK_SIZE;
     *ptr = p;
     return(ROK);
}


/*----------------------------------------------------------------------
 * _stackAlloc - alloue un bloc memoire pour la pile (addresse haute)
 *----------------------------------------------------------------------
 */
int  *_stackAlloc(size)
{
     unsigned *p;
     if (((int  *)p = _MXmalloc(RUNpid, size, STACKB)) == (int  *)NULL)
          return((int *)NULL);
     FP_OFF(p) += size;
     return(p);
}


/*----------------------------------------------------------------------
 * sfree - libere une pile
 *----------------------------------------------------------------------
 */
_sfree(block, size, pid)
struct mblock *block;
unsigned size;
{
    unsigned nclicks;

    nclicks = (size / 16) + ( size % 16 ?  1 : 0 );


    /* se replacer sur le header */
    FP_SEG(block) -= HBLK_SIZE;
    FP_OFF(block)  = 0;

    /* verifier si la taille est correcte */
   if ((nclicks + HBLK_SIZE) != ((struct hblk *)block)->blen)
         m_Printf("\nSTACK CORRUPTED");

/*    if (debaddr != deb1)
        kprintf("FFRREE....... %s\n",Tasktab[pid].tname);*/


   FP_SEG(block) += HBLK_SIZE;
   return(_xfree(block, pid));  /* Libere ! */
}


