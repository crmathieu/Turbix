/* fs/fsopen.h */

/*
 * Constantes de gestion des Primitives Open , Creat , Close
 *
 */

/* Codes d'ouvertures - Creations */
/* prochainement utilises */
/* modes d'ouverture USER */
#define  O_RDONLY          0
#define  O_WRONLY          1
#define  O_RDWR            2
#define  O_NDELAY          4
#define  O_APPEND          8
#define  O_CREAT        0x10
#define  O_TRUNC        0x20
#define  O_EXCL         0x40
#define  O_SYNCW        0x80

/* modes d'ouvertures Dos */
#define  D_RDONLY          0
#define  D_WRONLY          1
#define  D_RDWR            2
#define  D_NDELAY          0
#define  D_APPEND          0
#define  D_CREAT           0
#define  D_TRUNC           0
#define  D_EXCLRW       0x10
#define  D_EXCLW        0x20
#define  D_EXCLR        0x30
#define  D_NOINH        0x80
#define  D_SYNCW         128


/* modes de creation */
#define S_IFREG            0
#define S_IREAD            1            /* 0000 0001 */
#define S_IWRITE           2            /* 0000 0010 */
#define S_IDIR             4            /* 0000 0100 */
#define S_IHI              8            /* 0000 1000 */

/* Macro */
#define modeOk(m) ((m<0||m>8)? FALSE : TRUE)

/* Tampon E/S */
#define IOBUFSZ 2048

extern int _errflag;
