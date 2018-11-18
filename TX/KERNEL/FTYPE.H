/* fs/ftype.h */



/*
 * entr‚e dans la table des fichiers ouverts
 *
 */
typedef struct {
   char         fname[64];      /* Nom du fichier */
   int          fRhandle;       /* handle reel (0 -> 255) */
   int          flock;          /* file locking */
   short        fcount;         /* # fois que l'entree est utilisee */
   ushort       fmode;          /* mode invoqu‚ lors de la 1ere ouverture */
   long         fRpos;          /* position r‚elle dans le fichier */
}  fslot;

/*
 * entr‚e dans la table d'indirection des streams
 *
 */

typedef struct {
   int         s_streamtyp;     /* device file ou pipe                     */
   int         s_minor;         /* no device ou pipe                       */
   ushort      s_mode;          /* RW bits, mode de creation du fichier    */
   int         s_lastOp;        /* derniere operation realisee R ou W      */
   int         s_lock;          /* FILE : indique si le fichier est lock‚  */
   int         s_count;         /* si 0 : slot libre                       */
   int         s_access;        /* type d'acc‚s (OPEN)                     */
   uchar      *s_buf;           /* buffer tampon allou‚ … l'OPEN ou CREAT  */
   int         s_pbuf;          /* position courante dans le buffer        */
   Bool        s_bload;         /* booleen de chargement du tampon         */
   int         s_limit;         /* offset de fin de fichier dans le Tampon */
   int         s_bdirty;        /* booleen indiquant de flusher le tampon  */
   fslot      *s_ft;            /* pointeur sur open file slot             */
   ulong       s_pos;           /* position du fichier                      */
}  stream_entry;



#define NSTREAM           64         /* # de slot dans la stream table */
#define NULLFILE          (fslot *)0
#define NULLSTREAM        (stream_entry  *)0
#define FILESTREAM       -1
#define PIPESTREAM       -2
#define TTYSTREAM        -3

extern stream_entry  Streamtab[];
extern stream_entry *_getstream();

