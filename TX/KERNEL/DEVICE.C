/* device.c */

#include "sys.h"
#include "conf.h"
#include "error.h"
#include "io.h"
#include "tty.h"
#include "console.h"
#include "const.h"


#include "fspipe.h"

extern int /*activtxtP*/ activtxtP,adapter;
#define SESSION  1

/*----------------------------------------------------------------------
 * _openSession - ouvrir une session
 *----------------------------------------------------------------------
 */
_openSession(pid,dvminor,mode)
int pid, dvminor, mode;
{
   return(_openDev(pid,dvminor,mode,SESSION));
}

/*----------------------------------------------------------------------
 * _openDev - ouvrir un device tty dans la file descriptor table
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

    /* tester si device deja ouvert */
    if (tty[dvminor].streamptr == NULLSTREAM)
    {   /* device ferm‚ : le charger en stream table */

/*          if (dvminor == VSG)
             if (adapter != MONO)  return(RERR);*/

          if ((sp = _getstream()) == (stream_entry *)RERR) {
               _itRes(ps);   /* stream table overflow */
               return(RERR);
          }

          /*  allouer un slot dans la fdesc table de la tache */
          if ( (fd = _getfd(pid)) == RERR)
          {
                sp->s_count = 0;
                _itRes(ps);     /* too many open file */
                return(RERR);
          }

          /* initialiser l'ecran virtuel */
          _init_vs(&tty[dvminor]);

          /* chainer le stream slot a la fdesc table de la tache */
          Tasktab[pid].tfd[fd]     = sp;
          sp->s_minor              = dvminor;
          sp->s_streamtyp          = TTYSTREAM;
          tty[dvminor].streamptr   = sp;

    }
    else           /* le device est deja charg‚ en stream table */
          if ((fd = _getfd(pid)) == RERR)  {
               _itRes(ps);
               return(RERR);
          }
          else
          {
              tty[dvminor].streamptr->s_count++; /* incrementer nb d'ouvertures */
              sp->s_mode           = mode;       /* placer le nouveau mode */
              Tasktab[pid].tfd[fd] = tty[dvminor].streamptr;
          };
    if (special) /* mettre a jour no grp */
        Tasktab[pid].tgrp = dvminor;
    _itRes(ps);
    return(fd);
}


/*----------------------------------------------------------------------
 * _devminor - retourne le minor device du tty name
 *----------------------------------------------------------------------
 */
_devminor(devname)
char *devname;
{
   int i;
   for (i=0;i<_ntty();i++){
       if (strcmp(devname,ttyname[i]) == 0)
           return(i);
   }
   return(RERR);
}

/*----------------------------------------------------------------------
 * fdminor - retourne le minor device du disque name
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
 * _closeDev - fermer un device ou un pipe
 *----------------------------------------------------------------------------
 */
_closeDev(pid,fd)
int pid,fd;
{
  int ps;
  stream_entry **aux, *sp;


  /* aller chercher le stream slot correspondant au fd */
  ps = _itDis();
  aux = &Tasktab[pid].tfd[fd];
  sp = *aux;

     if (sp->s_streamtyp == PIPESTREAM)
         Piptab[sp->s_minor].count--;

     if (--sp->s_count == 0) {
         /* fermer le device  : l'enlever de la stream table */

/*         if (tty[sp->s_minor].ttyminor == VSG && adapter == MONO)
             m_Outp(M_6845+CONF,0);*/

         /*  cas du pipe : si close en READ , reveiller
          *  toutes les taches suspendues en ecriture sur
          *  ce pipe et envoyer signal SIGPIPE
          *                si close en WRITE , reveiller
          *  toutes les taches suspendues en lecture et leur
          *  faire rendre la main avec un nb de caracteres lus a 0
          */

         if (sp->s_streamtyp == PIPESTREAM) {
             switch(sp->s_mode)
             {
               case PREAD  : if (Piptab[sp->s_minor].susptask[PWRITE] > 0) {
/*                                 m_printf("\nCLOSE PIPE : PREAD\n");*/
                                 _restartTask(&Piptab[sp->s_minor],PWRITE,PEXIT);
                             }
                             break;
               case PWRITE : if (Piptab[sp->s_minor].susptask[PREAD] > 0) {
/*                                 m_printf("\nCLOSE PIPE : PWRITE\n");*/
                                 _restartTask(&Piptab[sp->s_minor],PREAD,PEXIT);
                             }
/*                             else m_printf("\nCLOSEP:pas de tache en PREAD\n");*/
                             break;
             }
             Piptab[sp->s_minor].invalid = TRUE;
             if (Piptab[sp->s_minor].count <= 0) {
/*                 m_printf("\nCLOSEPIPE count = %d\n",Piptab[sp->s_minor].count );*/
                 _closepipe(&Piptab[sp->s_minor]);
             }
         }
         else {
             tty[sp->s_minor].streamptr = NULLSTREAM;
             tty[sp->s_minor].vsinit    = FALSE;

             /* liberer le buffer d'ecran si on est pas sur le VS actif */
             if (sp->s_minor != activtxtP)
                 _xfree(tty[sp->s_minor].vsbuf, pid);
         }
         sp->s_streamtyp              = FILESTREAM; /* remettre val/defaut */
     };
     *aux         = NULLSTREAM;        /* liberer slot fd */
     _itRes(ps);
     return(ROK);
}




/*----------------------------------------------------------------------------
 * closeSys - fermer un peripherique ou un fichier (interdit a l'utilisateur)
 *----------------------------------------------------------------------------
 */
_closeSys(pid, fd)
int pid, fd;
{
     stream_entry      *sp;
     int                ps;

     ps = _itDis();
     if (isbadfd(fd) || ((sp = Tasktab[pid].tfd[fd]) == NULLSTREAM))  {
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
 * _openSys - ouvrir un peripherique / fichier (interdit a l'utilisateur)
 *----------------------------------------------------------------------------
 */
_openSys(pid ,name, access, mode)
int pid;
char  *name;
int access, mode;
{
     int dev;

     if ((dev = _devminor(name)) != RERR)
          return(_openDev(pid, dev, mode, 0));
     else
          return(_openFile(pid, name, access, mode));

}


/*---------------------------------------------------------------------------
 * dupSys - duplique un file descriptor (interdit a l'utilisateur)
 *          d'une tache dans une autre tache
 *---------------------------------------------------------------------------
 */
_dupSys(frompid,topid,fd)
int frompid,topid;
int fd;
{
    int fdup,ps;
    stream_entry *sp;

    ps = _itDis();
    if ((fdup = _getfd(topid)) == RERR)  {
         _itRes(ps);
         return(RERR);
    }
    if ((sp = Tasktab[frompid].tfd[fd]) != NULLSTREAM) {
         Tasktab[topid].tfd[fdup] = sp;
         sp->s_count++;

         if (sp->s_streamtyp == FILESTREAM) {
/*                m_Printf("DUP ENTRY fd = %d from = %d to = %d\n", fd, frompid, topid);*/
                _dupfileEntry(sp->s_ft);
         }
         else
                if (sp->s_streamtyp == PIPESTREAM)
                        Piptab[sp->s_minor].count++;
    }
    _itRes(ps);
    return(fdup);
}
