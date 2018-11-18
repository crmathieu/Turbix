/* file.h */

#define  NULLFILE         (fslot *)0
#define  F_NSLOT          32         /* # de slots de la filetab */
#define  F_SIZE           sizeof(dir_entry)
#define  F_PER_BLOCK      (DBUFSIZ / F_SIZE)
#define  F_NOSLOT        -2
#define  F_NOFILE        -3
#define  F_FOUND         -4
#define  F_NOTFOUND      -5
#define  F_ISDIR         -6
#define  F_ISFILE        -7
#define  F_NOTDIR        -8
#define  F_LOAD          -9
#define  F_NOLOAD        -10
#define  F_DISK          -11
#define  F_REOPEN        -12
#define  DEL_ENTRY        0xe5       /* 1er caractere sur fichier delet‚ */
#define  BUSY_ENTRY       0x23       /* marqueur d'entree occupee */

extern fslot  Filetab[];
extern fslot *_getfileEntry();