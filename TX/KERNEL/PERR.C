/*  TURBIX Error Codes strings */

#include "kernel.h"
#include "error.h"

#define DOS_NONE        0
#define NONE            0

/* translation DOS / UNIX error codes table */
int trErrTable[] = {
         /*DOS_EZERO   , */ EZERO   ,             /* 0  */
         /*DOS_EINVFNC , */ EINVFNC ,             /* 1  Invalid function number      */
         /*DOS_ENOFILE , */ ENOENT  ,             /* 2  File not found               */
         /*DOS_ENOPATH , */ ENOPATH ,             /* 3  Path not found               */
         /*DOS_EMFILE  , */ EMFILE  ,             /* 4  Too many open files          */
         /*DOS_EACCES  , */ EACCES  ,             /* 5  Permission denied            */
         /*DOS_EBADF   , */ EBADF   ,             /* 6  Bad file number              */
         /*DOS_ECONTR  , */ ECONTR  ,             /* 7  Memory blocks destroyed      */
         /*DOS_ENOMEM  , */ ENOMEM  ,             /* 8  Not enough core              */
         /*DOS_EINVMEM , */ EINVMEM ,             /* 9  Invalid memory block address */
         /*DOS_EINVENV , */ EINVENV ,             /* 10 Invalid environment          */
         /*DOS_EINVFMT , */ EINVFMT ,             /* 11 Invalid format               */
         /*DOS_EINVACC , */ EINVACC ,             /* 12 Invalid access code          */
         /*DOS_EINVDAT , */ EINVDAT ,             /* 13 Invalid data                 */
         /*DOS_NONE    , */ NONE    ,             /* 14 None */
         /*DOS_EINVDRV , */ ENODEV  ,             /* 15 Invalid drive specified      */
         /*DOS_ECURDIR , */ ECURDIR ,             /* 16 Attempt to remove CurDir     */
         /*DOS_ENOTSAM , */ ENOTSAM ,             /* 17 Not same device              */
         /*DOS_ENMFILE , */ ENMFILE ,             /* 18 No more files                */
         /*DOS_EINVAL  , */ EINVAL  ,             /* 19 Invalid argument             */
         /*DOS_E2BIG   , */ E2BIG   ,             /* 20 Arg list too long            */
         /*DOS_ENOEXEC , */ ENOEXEC ,             /* 21 Exec format error            */
         /*DOS_EXDEV   , */ EXDEV   ,             /* 22 Cross-device link            */
         /*DOS_NONE    , */ NONE    ,             /* 23 None */
         /*DOS_NONE    , */ NONE    ,             /* 24 None */
         /*DOS_NONE    , */ NONE    ,             /* 25 None */
         /*DOS_NONE    , */ NONE    ,             /* 26 None */
         /*DOS_NONE    , */ NONE    ,             /* 27 None */
         /*DOS_NONE    , */ NONE    ,             /* 28 None */
         /*DOS_NONE    , */ NONE    ,             /* 29 None */
         /*DOS_NONE    , */ NONE    ,             /* 30 None */
         /*DOS_NONE    , */ NONE    ,             /* 31 None */
         /*DOS_NONE    , */ NONE    ,             /* 32 None */
         /*DOS_EDOM    , */ EDOM    ,             /* 33 Math argument                */
         /*DOS_ERANGE  , */ ERANGE  ,             /* 34 Result too large             */
         /*DOS_EEXIST  , */ EFEXIST               /* 35 File already exists          */
};

/*        DOS_ENOENT   2              /* No such file or directory    */
/*        DOS_ENODEV  15              /* No such device               */


#define NSYSERR  52
