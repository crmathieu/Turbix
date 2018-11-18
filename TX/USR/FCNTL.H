/* fcntl.h */


/* access mode */
#define  O_RDONLY          0
#define  O_WRONLY          1
#define  O_RDWR            2
#define  O_NDELAY          4
#define  O_APPEND          8
#define  O_CREAT          16
#define  O_TRUNC          32
#define  O_EXCL           64
#define  O_SYNCW         128

/* creation mode */
#define S_IREAD          256
#define S_IWRITE         512
#define S_IDIR          1024
#define S_IHI           2048


/* creation attributs */
#define ATT_RONLY          0x01
#define ATT_HI             0x02
#define ATT_SYS            0x04
#define ATT_VOL            0x08
#define ATT_DIR            0x10
#define ATT_ARCHI          0x20
#define ATT_RES1           0x40
#define ATT_RES2           0x80

/* SEEK origin */
#define SEEK_END               2
#define SEEK_CUR               1
#define SEEK_SET               0



