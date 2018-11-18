/*  DUNIX Error Codes */

#define EZERO    0              /* Error 0                      */
#define EINVFNC  1              /* Invalid function number      */
#define ENOFILE  2              /* File not found               */
#define ENOPATH  3              /* Path not found               */
#define ECONTR   7              /* Memory blocks destroyed      */
#define EINVMEM  9              /* Invalid memory block address */
#define EINVENV 10              /* Invalid environment          */
#define EINVFMT 11              /* Invalid format               */
#define EINVACC 12              /* Invalid access code          */
#define EINVDAT 13              /* Invalid data                 */
#define EINVDRV 15              /* Invalid drive specified      */
#define ECURDIR 16              /* Attempt to remove CurDir     */
#define ENOTSAM 17              /* Not same device              */
#define ENMFILE 18              /* No more files                */

#define ENOENT   19             /* No such file or directory    */
#define EMFILE   20             /* Too many open files          */
#define EACCES   21             /* Permission denied            */
#define EBADF    22             /* Bad file number              */
#define ENOMEM   23             /* Not enough core              */
#define ENODEV   24             /* No such device               */
#define EINVAL   25             /* Invalid argument             */
#define E2BIG    26             /* Arg list too long            */
#define ENOEXEC  27             /* Exec format error            */
#define EXDEV    28             /* Cross-device link            */
#define EDOM     29             /* Math argument                */
#define ERANGE   30             /* Result too large             */
#define EFEXIST  31             /* File already exists          */
#define EDEXIST  32             /* directory already exists     */
#define ENXIO    33             /* no such device or address    */
#define EIO      34             /* I/O error                    */
#define ESRCH    35             /* no such process              */

#define ENOTDIR  36             /* not a directory              */
#define EISDIR   37             /* is  a directory              */
#define ENOSPC   38             /* no space left on device      */
#define EUNKNW   39             /* unknown error                */
#define EAGAIN   40             /* too many process             */
#define EBUSY    41             /* Device busy                  */
#define ENFILE   42             /* file table overflow          */
#define ENSTRE   43             /* stream table overflow        */
#define ENOTTY   44             /* not a TTY                    */
#define ESPIPE   45             /* illegal seek                 */
#define ENSEMA   46             /* semaphore table overflow     */
#define ECHILD   47             /* no child process             */
#define EINVOP   48             /* invalid operation            */
#define EMSESS   49             /* too many opened sessions     */
#define EINSTR   50             /* invalid stream type          */
#define EINWIN   51             /* invalid window               */
#define EINWM    52             /* window mode disable          */

#define EFAULT  -1              /* bad address                  */
#define EPERM   -1              /* not owner                    */
#define EINTR   -1              /* interrupt system call        */

#define ENOTBLK -1              /* block device required        */

#define ETXTBSY -1              /* UNIX - not MSDOS             */
#define EFBIG   -1              /* file too large               */

#define EROFS   -1              /* read-only file system        */
#define EMLINK  -1              /* UNIX - not MSDOS             */
#define EPIPE   -1              /* broken pipe                  */
#define ENOTNAME -1             /* not a file name              */
#define EUCLEAN -1              /* file system needs cleaning   */

#define NSYSERR  52

/*  Dos Error Codes */

#define DOS_EZERO    0              /* Error 0                      */
#define DOS_EINVFNC  1              /* Invalid function number      */
#define DOS_ENOFILE  2              /* File not found               */
#define DOS_ENOPATH  3              /* Path not found               */
#define DOS_ECONTR   7              /* Memory blocks destroyed      */
#define DOS_EINVMEM  9              /* Invalid memory block address */
#define DOS_EINVENV 10              /* Invalid environment          */
#define DOS_EINVFMT 11              /* Invalid format               */
#define DOS_EINVACC 12              /* Invalid access code          */
#define DOS_EINVDAT 13              /* Invalid data                 */
#define DOS_EINVDRV 15              /* Invalid drive specified      */
#define DOS_ECURDIR 16              /* Attempt to remove CurDir     */
#define DOS_ENOTSAM 17              /* Not same device              */
#define DOS_ENMFILE 18              /* No more files                */

#define DOS_ENOENT   2              /* No such file or directory    */
#define DOS_EMFILE   4              /* Too many open files          */
#define DOS_EACCES   5              /* Permission denied            */
#define DOS_EBADF    6              /* Bad file number              */
#define DOS_ENOMEM   8              /* Not enough core              */
#define DOS_ENODEV  15              /* No such device               */
#define DOS_EINVAL  19              /* Invalid argument             */
#define DOS_E2BIG   20              /* Arg list too long            */
#define DOS_ENOEXEC 21              /* Exec format error            */
#define DOS_EXDEV   22              /* Cross-device link            */
#define DOS_EDOM    33              /* Math argument                */
#define DOS_ERANGE  34              /* Result too large             */
#define DOS_EEXIST  35              /* File already exists          */

