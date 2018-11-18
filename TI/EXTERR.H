/* exterr.h : definitions des valeurs possibles de l'EXTENDED ERROR VALUE
 *            donn‚e par DOSEXTERR()
 */

#define    FCINV           0x01     /* function number invalid */
#define    FNFOUND         0x02     /* file not found          */
#define    PNFOUND         0x03     /* path not found          */
#define    TOOMANY         0x04     /* too many open files     */
#define    ACCDEN          0x05     /* access denied           */
#define    HDINV           0x06     /* handle invalid          */
#define    MEMCBHS         0x07     /* memory control block destroy */
#define    NOTEMEM         0x08     /* not enought memory */
#define    MEMBAINV        0x09     /* memory block address invalid */
#define    ENVINV          0x0A     /* environment invalid */
#define    FORMINV         0x0B     /* format invalid */
#define    ACCCINV         0x0C     /* access code invalid */
#define    INVDATA         0x0D     /* invalid data */
#define    RESERVED        0x0E     /* reserved value */
#define    DRIVEINV        0x0F     /* disk drive invalid */
#define    ATTREMDIR       0x10     /* attempt to remove not empty directory */
#define    NOTSAMEDEV      0x11     /* not same device */
#define    NOMOREFILES     0x12     /* no more files */
#define    DISKWP          0x13     /* disk write protected */
#define    UNKUNIT         0x14     /* unknown unit */
#define    DRIVENOTRDY     0x15     /* drive not ready */
#define    UNKCOMMAND      0x16     /* unknown command */
#define    CRCERROR        0x17     /* data error (CRC) */
#define    BADREQSL        0x18     /* bad request struct lenght */
#define    SEEKERR         0x19     /* seek error */
#define    UNKMEDIUM       0x1A     /* unknown medium type */
#define    SECTORNF        0x1B     /* sector not found */
#define    PRINTOFP        0x1C     /* printer out of paper */
#define    WRITEFAULT      0x1D     /* write fault */
#define    READFAULT       0x1E     /* read fault */
#define    GLOBALFAIL      0x1F     /* general failure */
#define    SHAREVIOL       0x20     /* sharing violation */
#define    LOCKVIOL        0x21     /* lock violation */
#define    DISKCHGINV      0x22     /* disk change invalid */
#define    FCBUNAVAILABLE  0x23     /* FCB unavailable */
#define    FILEEXIST       0x50     /* file already exists */
#define    MKDIRERR        0x52     /* cannot make directory */
#define    INT24FAIL       0x53     /* fail on INT24 (critical error interrupt) */
