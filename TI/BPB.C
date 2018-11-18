#include <stdio.h>
#include <dos.h>
#include "bpb.h"

struct pblk bpb;

main(argc,argv)
int argc;
char *argv[];
{
   if (argc == 1) exit(0);
   vkvkvkvkvkotbpb(atoi(argv[1]), &bpb);
}


vkvkvkvkvkotbpb(dev, pbpb)
struct pblk *pbpb;
{
 union  REGS  rin,rout;
 struct SREGS segreg;

      rin.h.ah  = 0x44;
      rin.h.al  = 0x0d;
      rin.h.bl  = dev; /* 0 = dft, 1 = A, 2 = B, 3 = C, etc ... */
      rin.h.ch  = 8; /* toujours */
      rin.h.cl  = 0x60;
      segreg.ds = FP_SEG(pbpb);
      rin.x.dx  = FP_OFF(pbpb);
      int86x(0x21, &rin, &rout,&segreg);
      if (rout.x.cflag) {
          printf("ERREUR AX = %u\n", rout.x.ax);
          exit(1);
      }
      printf("\
# BYTE / SEC       = %u\n\
# SEC / CLUST      = %u\n\
# RES SEC          = %u\n\
# n FAT            = %d\n\
# n ROOT entrees   = %u\n\
# TOTAL SEC        = %u\n\
# MEDIA            = %u\n\
# SEC / FAT        = %u\n\
# SEC / TRACK      = %u\n\
# HEADS            = %u\n\
# HIDDEN SEC       = %lu\n\
# NCYL             = %u\n\
# TOTAL SEC 32     = %lu\n",
       pbpb->bpbBlock.nBytePerSec,
       pbpb->bpbBlock.nSecPerCluster,
       pbpb->bpbBlock.nReservedSec,
       pbpb->bpbBlock.nFat,
       pbpb->bpbBlock.nRootEntries,
       pbpb->bpbBlock.nTotalSec,
       pbpb->bpbBlock.mediaDesc,
       pbpb->bpbBlock.nSecPerFat,
       pbpb->bpbBlock.nSecPerTrack,
       pbpb->bpbBlock.nHead,
       pbpb->bpbBlock.nHiddenSec,
       pbpb->nCyl,
       pbpb->bpbBlock.nTotalSec32);
}