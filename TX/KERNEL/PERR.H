/*  ULK Error Codes */

#define EZERO	 0		/* Error 0			*/
#define EINVFNC	 1		/* Invalid function number	*/
#define ENOFILE	 2		/* File not found		*/
#define ENOPATH	 3		/* Path not found		*/
#define ECONTR	 7		/* Memory blocks destroyed	*/
#define EINVMEM	 9		/* Invalid memory block address */
#define EINVENV 10		/* Invalid environment		*/
#define EINVFMT 11		/* Invalid format		*/
#define EINVACC 12		/* Invalid access code		*/
#define EINVDAT 13		/* Invalid data			*/
#define EINVDRV 15		/* Invalid drive specified	*/
#define ECURDIR 16		/* Attempt to remove CurDir	*/
#define ENOTSAM 17		/* Not same device		*/
#define ENMFILE 18		/* No more files		*/

#define ENOENT	 19		/* No such file or directory	*/
#define EMFILE	 20		/* Too many open files		*/
#define EACCES	 21		/* Permission denied		*/
#define EBADF	 22		/* Bad file number		*/
#define ENOMEM	 23		/* Not enough core		*/
#define ENODEV	 24		/* No such device		*/
#define EINVAL	 25		/* Invalid argument		*/
#define E2BIG	 26		/* Arg list too long		*/
#define ENOEXEC  27		/* Exec format error		*/
#define EXDEV	 28		/* Cross-device link		*/
#define EDOM	 29		/* Math argument		*/
#define ERANGE	 30		/* Result too large		*/
#define EFEXIST	 31		/* File already exists		*/
#define EDEXIST	 32		/* directory already exists	*/

#define ENOTDIR  33		/* not a directory		*/
#define EISDIR	 34		/* is  a directory		*/
#define EUNKNW  35              /* unknown error                */
#define EFAULT	-1		/* Unknown error		*/
#define EPERM	-1		/* UNIX - not MSDOS		*/
#define ESRCH	-1		/* UNIX - not MSDOS		*/
#define EINTR	-1		/* UNIX - not MSDOS		*/
#define EIO	-1		/* UNIX - not MSDOS		*/
#define ENXIO	-1		/* UNIX - not MSDOS		*/
#define ECHILD	-1		/* UNIX - not MSDOS		*/
#define EAGAIN	-1		/* UNIX - not MSDOS		*/
#define ENOTBLK -1		/* UNIX - not MSDOS		*/
#define EBUSY	-1		/* UNIX - not MSDOS		*/
#define ENOTDIR -1		/* UNIX - not MSDOS		*/
#define EISDIR	-1		/* UNIX - not MSDOS		*/
#define ENFILE	-1		/* UNIX - not MSDOS		*/
#define ENOTTY	-1		/* UNIX - not MSDOS		*/
#define ETXTBSY -1		/* UNIX - not MSDOS		*/
#define EFBIG	-1		/* UNIX - not MSDOS		*/
#define ENOSPC	-1		/* UNIX - not MSDOS		*/
#define ESPIPE	-1		/* UNIX - not MSDOS		*/
#define EROFS	-1		/* UNIX - not MSDOS		*/
#define EMLINK	-1		/* UNIX - not MSDOS		*/
#define EPIPE	-1		/* UNIX - not MSDOS		*/
#define EUCLEAN -1		/* UNIX - not MSDOS		*/

#define NSYSERR  32