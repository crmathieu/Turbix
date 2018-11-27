/* device.c */

#include "sys.h"
#include "conf.h"
#include "error.h"
#include "io.h"
#include "tty.h"
#include "console.h"
#include "const.h"


#include "fspipe.h"

extern int activtxtP, adapter;
#define SESSION  1

/*----------------------------------------------------------------------
 * _openSession - 
 *----------------------------------------------------------------------
 */
_openSession(pid,dvminor,mode)
int pid, dvminor, mode;
{
   return(_openDev(pid,dvminor,mode,SESSION));
}

/*----------------------------------------------------------------------
 * _openDev - opens a tty device ouvrir un device tty dans la file descriptor table
 *           de la tache pid
 *----------------------------------------------------------------------
 */
_openDev(pid,dvminor,mode,special)
int pid, dvminor, mode;
{
    int ps  ;
    stream_entry  *sp;
    int fd;

    ps = _itDis();

    /* test if device is already open */
    if (tty[dvminor].streamptr == NULLSTREAM) {   
        /* device is closed: load it in stream table */
        if ((sp = _getstream()) == (stream_entry *)RERR) {
            _itRes(ps);   /* stream table overflow */
            return(RERR);
        }

        /*  allocate a slot in the task's fdesc table */
        if ( (fd = _getfd(pid)) == RERR) {
            sp->s_count = 0;
            _itRes(ps);     /* too many open files */
            return(RERR);
        }

        /* initialize virtual screen */
        _init_vs(&tty[dvminor]);

        /* link stream slot to task's fdesc table */
        Tasktab[pid].tfd[fd]     = sp;
        sp->s_minor              = dvminor;
        sp->s_streamtyp          = TTYSTREAM;
        tty[dvminor].streamptr   = sp;

    } else  {
        /* device already loaded in stream table */
        if ((fd = _getfd(pid)) == RERR) {
            _itRes(ps);
            return (RERR);
        } else {
            tty[dvminor].streamptr->s_count++; /* increments open counter */
            sp->s_mode = mode;                 /* set new mode */
            Tasktab[pid].tfd[fd] = tty[dvminor].streamptr;
        }
    }
    
    if (special) {
        /* update group number */
        Tasktab[pid].tgrp = dvminor;
    }
    _itRes(ps);
    return(fd);
}


/*----------------------------------------------------------------------
 * _devminor - returns tty name's minor device
 *----------------------------------------------------------------------
 */
_devminor(devname)
char *devname;
{
    int i;
    for (i=0;i<_ntty();i++) {
        if (strcmp(devname,ttyname[i]) == 0) {
            return(i);
        }
    }
    return(RERR);
}

/*----------------------------------------------------------------------
 * fdminor - returns disk name minor device
 *----------------------------------------------------------------------
 */
fdminor(fdletter)
char fdletter;
{
   int i;
   switch(fdletter)
   {
     case 'a':
     case 'A': return(FD0);
     case 'b':
     case 'B': return(FD1);
     case 'c':
     case 'C': return(HD0);
     case 'd':
     case 'D': return(HD1);
   }
   Tasktab[RUNpid].terrno = ENXIO;
   return(RERR);
}


/*----------------------------------------------------------------------------
 * _closeDev - close a device or a pipe
 *----------------------------------------------------------------------------
 */
_closeDev(pid,fd)
int pid,fd;
{
    int ps;
    stream_entry **aux, *sp;


    /* fetch the stream slot corresponding to the file descriptor fd */
    ps = _itDis();
    aux = &Tasktab[pid].tfd[fd];
    sp = *aux;

    if (sp->s_streamtyp == PIPESTREAM) {
        Piptab[sp->s_minor].count--;
    }

    if (--sp->s_count == 0) {
        /* close device: remove it fromstream table */

        /*  when closing a pipe: 
               - for a READ operation, wake up all tasks suspended on a
                 WRITE operation on this pipe and send a SIGPIPE signal
               - for a WRITE operation, wake up all tasks suspended on a
                 READ operation on this pipe and make them return a number
                 of read characters = 0
        */

        if (sp->s_streamtyp == PIPESTREAM) {
            switch(sp->s_mode) {
            case PREAD  : 
                if (Piptab[sp->s_minor].susptask[PWRITE] > 0) {
                    /* CLOSE PIPE : PREAD */
                    _restartTask(&Piptab[sp->s_minor],PWRITE,PEXIT);
                }
                break;
            case PWRITE : 
                if (Piptab[sp->s_minor].susptask[PREAD] > 0) {
                    /* CLOSE PIPE : PWRITE */
                    _restartTask(&Piptab[sp->s_minor],PREAD,PEXIT);
                }
            default:
                break;
            }

            Piptab[sp->s_minor].invalid = TRUE;
            if (Piptab[sp->s_minor].count <= 0) {
                _closepipe(&Piptab[sp->s_minor]);
            }
        } else {
            /* close device */
            tty[sp->s_minor].streamptr = NULLSTREAM;
            tty[sp->s_minor].vsinit    = FALSE;

            /* free screen buffer if we are not on the active VS */
            if (sp->s_minor != activtxtP) {
                _xfree(tty[sp->s_minor].vsbuf, pid);
            }
        }
        sp->s_streamtyp = FILESTREAM; /* reset defaut value */
    };
    *aux = NULLSTREAM;        /* free fd slot */
    _itRes(ps);
    return(ROK);
}


/*----------------------------------------------------------------------------
 * _closeSys - closes a file / peripheral (system use only)
 *----------------------------------------------------------------------------
 */
_closeSys(pid, fd)
int pid, fd;
{
     stream_entry      *sp;
     int                ps;

     ps = _itDis();
     if (isbadfd(fd) || ((sp = Tasktab[pid].tfd[fd]) == NULLSTREAM)) {
         Tasktab[RUNpid].terrno = EINVAL;
         _itRes(ps);
         return (RERR);
     }
     _itRes(ps);

     switch(sp->s_streamtyp) {
            case FILESTREAM : return(_closeFile(sp, pid, fd));
            case PIPESTREAM :
            case TTYSTREAM  : return(_closeDev(pid,fd));
            default         : Tasktab[RUNpid].terrno = ENXIO;
                              return(RERR);
     }
}

/*----------------------------------------------------------------------------
 * _openSys - opens a file / peripheral (system use only)
 *----------------------------------------------------------------------------
 */
_openSys(pid ,name, access, mode)
int pid;
char  *name;
int access, mode;
{
    int dev;

    if ((dev = _devminor(name)) != RERR) {
        return(_openDev(pid, dev, mode, 0));
    } else {
        return(_openFile(pid, name, access, mode));
    }
}

/*---------------------------------------------------------------------------
 * _dupSys - duplicates a task's file descriptor to another task
 *           (system use only)
 *---------------------------------------------------------------------------
 */
_dupSys(frompid,topid,fd)
int frompid,topid;
int fd;
{
    int fdup,ps;
    stream_entry *sp;

    ps = _itDis();
    if ((fdup = _getfd(topid)) == RERR) {
         _itRes(ps);
         return(RERR);
    }
    if ((sp = Tasktab[frompid].tfd[fd]) != NULLSTREAM) {
         Tasktab[topid].tfd[fdup] = sp;
         sp->s_count++;

         if (sp->s_streamtyp == FILESTREAM) {
            _dupfileEntry(sp->s_ft);
         } else { 
            if (sp->s_streamtyp == PIPESTREAM) {
                    Piptab[sp->s_minor].count++;
            }
         }
    }
    _itRes(ps);
    return(fdup);
}
