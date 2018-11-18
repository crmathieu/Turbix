/* io3.c */

#include "sys.h"
#include "conf.h"
#include "io.h"
#include "q.h"
#include "tty.h"
#include "const.h"
#include "lock.h"
#include "fspipe.h"


/*-----------------------------------------------------------------------------
 * _pflush - flush automatique d'une chaine sortie par Printf
 *-----------------------------------------------------------------------------
 */
_pflush(fd)
int  fd;
{
     stream_entry   *sp;
     ushort typ,r;
     struct taskslot *tp;

     if (isbadfd(fd) || (sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     _flush(&tty[sp->s_minor]);
     return(ROK);
}
/*-----------------------------------------------------------------------------
 * _sflush - flush automatique d'une chaine sortie par sePrintf
 *-----------------------------------------------------------------------------
 */
_sflush(session)
int  session;
{
     _flush(&tty[session]);
}
/*-----------------------------------------------------------------------------
 * m_Flush - vider le buffer de sortie sur le VS du process courant
 *-----------------------------------------------------------------------------
 */
m_Flush()
{
     _flush(&tty[Tasktab[RUNpid].tgrp]);
     Tasktab[RUNpid].terrno = 0;
}


/*----------------------------------------------------------------------------
 * m_Read - lire un ou plusieurs octet(s) d'un peripherique ou d'un fichier
 *----------------------------------------------------------------------------
 */
BIBLIO m_Read ( fd , buff , count )
int  fd ;
char  *buff;
int    count;
{
     unsigned ps;
     stream_entry   *sp;
     struct taskslot *tp;
     ushort typ;

     ps = _itDis();
     if (isbadfd(fd) || (sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         _itRes(ps);
         return (RERR);
     }
     _itRes(ps);
     tp->terrno = 0;
     if (sp->s_streamtyp == FILESTREAM)
        return(_Fread(sp , buff , count));
     else
        if (sp->s_streamtyp == PIPESTREAM){
                return(_Pread(&Piptab[sp->s_minor],buff,count));
        }
        else
                if (sp->s_streamtyp == TTYSTREAM)
                        return(_CSread(&tty[sp->s_minor],buff,count));
                else {
/*                        m_printf("read error: stream = %x\n", sp->s_streamtyp);*/
                        tp->terrno = ENXIO;
                        return(RERR);
                }

/*     switch(sp->s_streamtyp) {
        case FILESTREAM : return(_Fread(sp , buff , count));
        case PIPESTREAM :
                          return(_Pread(&Piptab[sp->s_minor],buff,count));
        case TTYSTREAM  : return(_CSread(&tty[sp->s_minor],buff,count));
        default         :
                          tp->terrno = ENXIO;
                          return(RERR);
     }*/
}

/*----------------------------------------------------------------------------
 * m_Getc - prendre un caractere d'un stream
 *----------------------------------------------------------------------------
 */
BIBLIO m_Getc( fd )
int fd ;
{
     stream_entry   *sp;
     char c;
     struct taskslot *tp;
     ushort typ;

     if (isbadfd(fd) || (sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     tp->terrno = 0;
     switch(sp->s_streamtyp) {
        case FILESTREAM : if (_Fread(sp,&c,1) > 0)   return(c);
                          else                       return(EOF);
        case PIPESTREAM : _Pread(&Piptab[sp->s_minor],&c,1);
                          return(c);
        case TTYSTREAM  : return(_CSgetc(&tty[sp->s_minor]));
        default         : tp->terrno = ENXIO;
                          return(RERR);
     }
}

/*----------------------------------------------------------------------------
 * m_Sgetc - prendre un caractere avec bufferisation en mode prioritaire
 *----------------------------------------------------------------------------
 */
BIBLIO m_Sgetc(fd)
int fd;
{
     stream_entry   *sp;
     char c;
     struct taskslot *tp;

     if (isbadfd(fd) || (sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     tp->terrno = 0;
     if (sp->s_streamtyp == TTYSTREAM)    /* E/S  device */
          return(_S_CSgetc(&tty[sp->s_minor]));
     tp->terrno = ENXIO;
     return(RERR);
}

/*----------------------------------------------------------------------------
 * m_Getch - prendre un caractere sans bufferisation et sans echo
 *----------------------------------------------------------------------------
 */
BIBLIO m_Getch()
{
     stream_entry   *sp;
     char c;
     struct taskslot *tp;

     if ((sp = (tp = &Tasktab[RUNpid])->tfd[stdin]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     tp->terrno = 0;
     if (sp->s_streamtyp == TTYSTREAM)    /* E/S  device */
          return(_CSgetch(&tty[sp->s_minor],FALSE));
     tp->terrno = ENXIO;
     return(RERR);
}

/*----------------------------------------------------------------------------
 * m_Getche - prendre un caractere sans bufferisation et avec echo
 *----------------------------------------------------------------------------
 */
BIBLIO m_Getche()
{
     stream_entry   *sp;
     char c;
     struct taskslot *tp;

     if ((sp = (tp = &Tasktab[RUNpid])->tfd[stdin]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     tp->terrno = 0;
     if (sp->s_streamtyp == TTYSTREAM)    /* E/S  device */
          return(_CSgetch(&tty[sp->s_minor],TRUE));
     tp->terrno = ENXIO;
     return(RERR);
}

/*----------------------------------------------------------------------------
 * m_Sgetch - prendre un caractere sans bufferisation en mode prioritaire
 *----------------------------------------------------------------------------
 */
BIBLIO m_Sgetch()
{
     stream_entry   *sp;
     char c;
     struct taskslot *tp;

     if ((sp = (tp = &Tasktab[RUNpid])->tfd[stdin]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     tp->terrno = 0;
     if (sp->s_streamtyp == TTYSTREAM)    /* E/S  device */
        return(_S_CSgetch(&tty[sp->s_minor]));

     tp->terrno = ENXIO;
     return(RERR);
}
/*----------------------------------------------------------------------------
 * m_Sgetche - prendre un caractere sans bufferisation avec echo en mode prioritaire
 *----------------------------------------------------------------------------
 */
BIBLIO m_Sgetche()
{
     stream_entry   *sp;
     char c;
     struct taskslot *tp;

     if ((sp = (tp = &Tasktab[RUNpid])->tfd[stdin]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }
     tp->terrno = 0;
     if (sp->s_streamtyp == TTYSTREAM)    /* E/S  device */
        return(_S_CSgetche(&tty[sp->s_minor]));

     tp->terrno = ENXIO;
     return(RERR);
}

/*----------------------------------------------------------------------------
 * m_SessionGetch - prendre un caractere sans bufferisation en mode prioritaire
 *----------------------------------------------------------------------------
 */
m_SessionGetch(session)
{
     return(_S_CSgetch(&tty[session]));
}


/*----------------------------------------------------------------------------
 * m_Puts - ecrire une chaine sur la sortie standard
 *----------------------------------------------------------------------------
 */
BIBLIO m_Puts(str)
char *str;
{
      _puts(stdout, str);
}
/*----------------------------------------------------------------------------
 * _puts - ecrire une chaine sur la sortie standard sp‚cifi‚e
 *----------------------------------------------------------------------------
 */
_puts(std, str)
char *str;
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
         m_Putc(str[i], std);
}

/*----------------------------------------------------------------------------
 * m_Putc - ecrire un octet
 *----------------------------------------------------------------------------
 */
BIBLIO m_Putc(ch,fd)
char ch;
int  fd;
{
     stream_entry   *sp;
     ushort typ,r;
     struct taskslot *tp;

     if (isbadfd(fd) || (sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
         tp->terrno = EINVAL;
         return (RERR);
     }

     switch(sp->s_streamtyp) {
        case FILESTREAM : return( _Fwrite(sp,&ch,1));
        case PIPESTREAM : return(_Pwrite(&Piptab[sp->s_minor],&ch,1));
        case TTYSTREAM  : if (sp->s_minor < TTY1)
                              return(_Cputc(&tty[sp->s_minor],ch));
                          else
                              return(_Sputc(&tty[sp->s_minor],ch));
        default         : tp->terrno = ENXIO;
                          return(RERR);
     }
}

/*----------------------------------------------------------------------------
 * VS0putc - ecrire un octet en mode kernel ( shunter la Streamtab )
 *----------------------------------------------------------------------------
 */
VS0putc(ch)
char ch;
{
    return(_Cputc(&tty[VS0],ch));
}

/*----------------------------------------------------------------------------
 * m_Write - ecrire buffer vers stream
 *----------------------------------------------------------------------------
 */
BIBLIO m_Write( fd , buff , count )
int fd ;
char *buff;
unsigned   count;
{
     unsigned ps;
     stream_entry   *sp;
     ushort typ;
     struct taskslot *tp;


     ps = _itDis();
     if (isbadfd(fd) || (sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
/*         VS0printf("write Erreur d'entree...\n");*/
         tp->terrno = EINVAL;
         _itRes(ps);
         return (RERR);
     }
     _itRes(ps);
     tp->terrno = 0;
/*     VS0printf("write...\n");*/
     switch(sp->s_streamtyp) {
        case FILESTREAM : return(_Fwrite(sp , buff , count));
        case PIPESTREAM : return(_Pwrite(&Piptab[sp->s_minor],buff,count));
        case TTYSTREAM  : if (sp->s_minor < TTY1)
                              return(_Cwrite(&tty[sp->s_minor],buff,count));
                          else
                              return(_Swrite(&tty[sp->s_minor],buff,count));
        default         : /*VS0printf("write STREAM INCONNU\n");*/
                          tp->terrno = ENXIO;
                          return(RERR);
     }
}

/*----------------------------------------------------------------------------
 * m_Close - fermer un peripherique ou un fichier
 *----------------------------------------------------------------------------
 */
BIBLIO m_Close( fd )
int fd;
{
     return(_closeSys(RUNpid,fd));
}

/*----------------------------------------------------------------------------
 * m_Remove - supprimer un fichier
 *----------------------------------------------------------------------------
 */
BIBLIO m_Remove(fname)
char *fname;
{
     return(_removeFile(fname));
}

/*----------------------------------------------------------------------------
 * m_Open - ouvrir un peripherique / fichier
 *----------------------------------------------------------------------------
 */
BIBLIO m_Open(name, access, mode)
char  *name;
int mode;
{
     return(_openSys(RUNpid, name, access, mode));
}

/*----------------------------------------------------------------------------
 * m_Dup2 - assigne le fichier d‚sign‚ par fd1 … fd2
 *----------------------------------------------------------------------------
 */
BIBLIO m_Dup2(fd1, fd2)
int fd1, fd2;
{
     int ps;
     stream_entry *sp;
     struct taskslot *tp;

     (tp = &Tasktab[RUNpid])->terrno = 0;
     if (isbadfd(fd1)) {
         tp->terrno = EINVAL;
         return (RERR);
     }
    if (tp->tfd[fd2] != NULLSTREAM)
        m_Close(fd2);

    ps = _itDis();
    if ((sp = tp->tfd[fd1]) != NULLSTREAM) {
         tp->tfd[fd2] = sp;
         sp->s_count++;
         if ((fd2 < 2) && (sp->s_streamtyp == TTYSTREAM))
             tp->tgrp = sp->s_minor; /* MAJ grp */
         if (sp->s_streamtyp == FILESTREAM)
             _dupfileEntry(sp->s_ft);
         if (sp->s_streamtyp == PIPESTREAM)
             Piptab[sp->s_minor].count++;
    }
    _itRes(ps);
    return(ROK);
}

/*---------------------------------------------------------------------------
 * m_Dup - duplique un file descriptor (fait pointer vers la meme stream_entry)
 *---------------------------------------------------------------------------
 */
BIBLIO m_Dup(fd)
int fd;
{
     int fdup,ps;
     stream_entry *sp;
     struct taskslot *tp;

     (tp = &Tasktab[RUNpid])->terrno = 0;
     if (isbadfd(fd)) {
         tp->terrno = EINVAL;
         return (RERR);
     }
    ps = _itDis();
    if ((fdup = _getfd(RUNpid)) == RERR)  {
         _itRes(ps);
         return(RERR);
    }
    if ((sp = tp->tfd[fd]) != NULLSTREAM)
    {
       tp->tfd[fdup] = sp;
       sp->s_count++;
       if ((fdup < 2) && (sp->s_streamtyp == TTYSTREAM))
           tp->tgrp = sp->s_minor; /* MAJ grp */
       if (sp->s_streamtyp == FILESTREAM)   _dupfileEntry(sp->s_ft);
       if (sp->s_streamtyp == PIPESTREAM)   Piptab[sp->s_minor].count++;
    }
    _itRes(ps);
    return(fdup);
}

/*----------------------------------------------------------------------
 * lock - locker un fichier
 *----------------------------------------------------------------------
 */
m_Lock(fd)
{
   return(_lockSys(m_Getpid(),fd));
}

/*----------------------------------------------------------------------
 * _lockSys - locker un fichier en mode systeme
 *----------------------------------------------------------------------
 */
_lockSys(pid,fd)
{
   stream_entry *sp;
   struct taskslot *tp;
   int ps,*lockvar;

   if ((sp = (tp = &Tasktab[pid])->tfd[fd]) == NULLSTREAM) {
       tp->terrno = EINVAL;
       return(RERR);
   }
   tp->terrno = 0;
   if (sp->s_streamtyp == FILESTREAM)
        lockvar = &sp->s_ft->flock;
   else {
        tp->terrno = EINSTR;
        return(RERR);
   }

   ps = _itDis();

   /* attendre disponibilit‚ de l'objet */
   _waitfUNLOCK(pid, lockvar);

   /* l'objet est disponible */
   *lockvar = sp->s_lock = TRUE;
   tp->tflag            |= F_LOCK;
   _itRes(ps);
   return(ROK);
}

/*----------------------------------------------------------------------
 * unlock - unlocker un fichier
 *----------------------------------------------------------------------
 */
m_Unlock(fd)
{
   return(_unlockSys(RUNpid,fd));
}

/*----------------------------------------------------------------------
 * _unlockSys - unlocker un fichier en  mode systeme
 *----------------------------------------------------------------------
 */
_unlockSys(pid,fd)
{
   stream_entry *sp;
   struct taskslot *tp;
   int ps,*lockvar;

   if (((sp = (tp = &Tasktab[pid])->tfd[fd]) == NULLSTREAM)||
        !(tp->tflag & F_LOCK)||(sp->s_lock == FALSE)) {
        tp->terrno = EINVAL;
        return(RERR);
   }
   tp->terrno = 0;
   ps = _itDis();
   tp->tflag &= ~F_LOCK;
   if (sp->s_streamtyp == FILESTREAM)
        lockvar = &sp->s_ft->flock;
   else {
        tp->terrno = EINSTR;
        _itRes(ps);
        return(RERR);
   }

   *lockvar = sp->s_lock = FALSE;

   /* reveiller d'eventuelles taches en attente sur LOCK */
   if (nonempty(lockhead))
       _setrdy(_getfirst(SYSQ,lockhead),NOSCHEDUL);

   _itRes(ps);
   return(ROK);
}

/*----------------------------------------------------------------------
 * _waitfUNLOCK - attente unlocking
 *----------------------------------------------------------------------
 */
_waitfUNLOCK(pid,lockvar)
int          *lockvar;
{
   struct taskslot *tp;
   int ps,next,*val;

   ps = _itDis();
   tp = &Tasktab[pid];

   if (*lockvar) {  /* le fichier est deja lock‚ : se bloquer */
       _enlist_tail(SYSQ, pid, locktail);
       while (1) {
           tp->tstate  = SLEEP;
           tp->tevent |= EV_LOCK;
           _swpProc();
           if (*lockvar) {
                /* il y a eu unlock sur un fichier, mais cette tache n'est
                 * pas concern‚e. On doit propager le reveil … la tache
                 * bloqu‚e suivante
                 */
                if ((next = nextid(pid)) < NTASK) {
                    /* il reste d'autre(s) tache(s) en file :
                     * se remettre en file dans la meme position
                     */
                    _enlist_tail(SYSQ,pid,next);

                    /* faire demarrer la tache suivante */
                    _defect(SYSQ,next);
                    _setrdy(next,NOSCHEDUL);
                }
                else
                    _systemHalt(lockErrStr);
           }
           else
                /* la voie est libre */
                break;
       }
   }
   _itRes(ps);
}

