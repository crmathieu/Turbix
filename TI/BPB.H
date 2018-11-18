/* bpb.h */

struct bpb {  /* BIOS PARAMETER BLOCK */
       unsigned       nBytePerSec;
       unsigned char  nSecPerCluster;
       unsigned       nReservedSec;
       unsigned char  nFat;
       unsigned       nRootEntries;
       unsigned       nTotalSec;
       unsigned char  mediaDesc;
       unsigned       nSecPerFat;
       unsigned       nSecPerTrack;
       unsigned	      nHead;
       unsigned long  nHiddenSec;
       unsigned long  nTotalSec32; /* a utiliser si la version 16 bits = 0 */
       char	      filler[6];
};


struct pblk {  /* parameter Block */
       unsigned char  flag;
       unsigned char  devTyp;
       unsigned       devAtt;
       unsigned       nCyl;
       unsigned char  mediaTyp;
       struct  bpb    bpbBlock;
};