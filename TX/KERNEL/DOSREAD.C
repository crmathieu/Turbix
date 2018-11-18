/* dosread.c */


#include "sys.h"
#include "conf.h"
#include "error.h"
#include "io.h"
#include "const.h"
#include "q.h"
#include "file.h"
#include "fstream.h"
#include "fsopen.h"
#include "iodos.h"

/* IL FAUT GERER LE PASSAGE HANDLE REEL ET HANDLE LOGIQUE */



/*----------------------------------------------------------------------
 * m_Tell - retourne la position courante sur le fichier ( fd )
 *----------------------------------------------------------------------
 */
BIBLIO long m_Tell(fd)
int fd;
{
   int  ret;
   stream_entry *sp;
   struct taskslot *tp;
   struct  ioDosReq  req;


   if ((sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
       tp->terrno = EINVAL;
       return((long)RERR);
   }
   tp->terrno = 0;
   switch(sp->s_lastOp) {
   case IO_READ  : m_printf("lastOP = R\n");return(sp->s_pos - (ulong)(sp->s_limit - sp->s_pbuf));
   case IO_WRITE : m_printf("lastOP = W\n");return(sp->s_pos + (ulong)(sp->s_pbuf));
   case IO_MOVE  : m_printf("lastOP = M\n");
   default:        m_printf("lastOP = D\n");
                   if (sp->s_pos > 0)
                        return(sp->s_pos - (ulong)(sp->s_limit - sp->s_pbuf));
                   else
                        return(sp->s_pos);
   }
}


/*---------------------------------------------------------------------------
 * m_Lseek - positionne le ptr du fichier et retourne f'offset courant.
 *         effets de bord
 *                 1) SEEK_END : le ptr de fichier est ramene a la lg du
 *                            fichier qqsoit la valeur de l'offset
 *                 2) SEEK_CUR : si l'offset fait aller plus loin que la lg du
 *                            fichier ,il est ramene a cette longueur
 *                 3) SEEK_SET
 *         si ERREUR retourne -1
 *---------------------------------------------------------------------------
 */
BIBLIO long m_Lseek(fd, offset, origine)
int         fd;             /* file descriptor */
long        offset;         /* position / a l'origine */
int         origine;        /* 0: debut  1: position courante  2: fin */
{
    stream_entry      *sp;
    struct taskslot   *tp;
    struct ioDosReq    req;
    long   pos;
    int ps;

    ps = _itDis();
    if ((sp = (tp = &Tasktab[RUNpid])->tfd[fd]) == NULLSTREAM) {
        tp->terrno = EINVAL;
        _itRes(ps);
        return((long)RERR);
    }
    tp->terrno = 0;
    if (sp->s_streamtyp == PIPESTREAM) {
        tp->terrno = ESPIPE;
        _itRes(ps);
        return((long)RERR);
    }

    /* construire la requete */

    if (origine == SEEK_CUR) {

        pos = m_tell(fd);
        m_printf("APRES TELL : s_pos = %ld, limit = %d, pbuf = %d, pos = %d\n",
                sp->s_pos, sp->s_limit, sp->s_pbuf, pos);
        if (offset + pos < sp->s_pos) {
                /* on reste dans le tampon */
                sp->s_pbuf  += offset;
                sp->s_lastOp = IO_MOVE;
                _itRes(ps);
                return(sp->s_pos - (ulong)(sp->s_limit - sp->s_pbuf));
        }
        if (sp->s_lastOp == IO_READ)
                offset = offset + (sp->s_pos - pos);
        else
                if (sp->s_bdirty)
                        _requete(sp, &req, sp->s_buf, IO_WRITE, sp->s_pbuf, FALSE); /* flush */
    }
    m_printf("SEEK: offset = %d\n", offset);
    req.drseek   = offset;
    req.focus    = FALSE;
    req.drsp     = sp;
    sp->s_bload  = FALSE;
    sp->s_lastOp = IO_MOVE;
    sp->s_pbuf   = 0;
    sp->s_bdirty = FALSE;
    req.drpid    = m_Getpid();
    req.count    = origine;
    req.Rhandle  = sp->s_ft->fRhandle;
    req.drop     = IO_MOVE;
    _io_op(&req);
    m_printf("apres lseek: pos = %ld\n", sp->s_pos);
    _itRes(ps);
    return(req.drseek);
}

/*----------------------------------------------------------------------
 * _Fread - lire un fichier
 *----------------------------------------------------------------------
 */
_Fread(sp, buff, count)
stream_entry        *sp;      /* pointeur sur stream entry */
uchar        *buff;
int           count;
{
    int                 ps, ret, countI, reste;
    struct  ioDosReq    req;

    ps = _itDis();
    Tasktab[RUNpid].terrno = 0;
    if (!isAccessEnable(sp, IO_READ)) {
        Tasktab[RUNpid].terrno = EACCES;
        _itRes(ps);
        return(RERR);
    }
    if (count == 0) {
        _itRes(ps);
        return(0);
    }
    if (sp->s_lock == FALSE) /* le fichier n'est pas lock‚ par cette tache */
        /* verifier si le fichier est lock‚ par une autre tache */
        _waitfUNLOCK(m_Getpid(), &sp->s_ft->flock);

    if (sp->s_lastOp == IO_WRITE) {
        if (sp->s_pbuf != 0)
                _requete(sp, &req, sp-> s_buf, IO_WRITE, sp->s_pbuf, FALSE); /* flush */
    }
    sp->s_lastOp = IO_READ;
/*
    if (sp->s_buf == NULLPTR)
        return(_requete(sp, &req, buff, IO_READ, count, FALSE));
*/
    countI = count;     /* nb initialement demand‚ */

    /* gestion avec buffer */
    /* au d‚part, sp->s_limit = IOBUFSZ */
    while (TRUE) {
        if (count > (reste = sp->s_limit - sp->s_pbuf)) {
                if (sp->s_bload)  { /* tampon d‚j… charg‚ */
                        if ((sp->s_limit < IOBUFSZ) && reste == 0) /* fin de fichier */
                                return(countI - count);
                        /* copier ce qui reste du tampon dans le buffer */
                        fastcpy(buff, &sp->s_buf[sp->s_pbuf], reste);
                        count -= reste;
                }
                if (count > IOBUFSZ) {
                        /* IO en une passe */
                        _itRes(ps);
                        return(countI - (count -  _requete(sp, &req, &buff[reste], IO_READ, count, FALSE)));
                }
                else
                        /* IO dans le tampon */
                        sp->s_limit = _requete(sp, &req, sp->s_buf, IO_READ, IOBUFSZ, TRUE);
        }
        else {
                if (sp->s_bload) {
                        if ((sp->s_limit < IOBUFSZ) && reste == 0) { /* fin de fichier */
                                _itRes(ps);
                                return(countI - count);
                        }
                        fastcpy(buff, &sp->s_buf[sp->s_pbuf], count);
                        sp->s_pbuf += count;
                        _itRes(ps);
                        return(count);
                }
                sp->s_limit = _requete(sp, &req, sp->s_buf, IO_READ, IOBUFSZ, TRUE);
        }
    }
}

/*----------------------------------------------------------------------
 * _requete : lancer une requete
 *----------------------------------------------------------------------
 */
_requete(sp, req, tampon, op, n, bload)
stream_entry *sp;
struct  ioDosReq    *req;
uchar               *tampon;
int                  op, n, bload;
{
        int ret, ps;

        /* construire la requete */
/*        if (op == IO_READ)
                m_printf("IOREAD\n");
        else    m_printf("IOWRITE\n");*/

        ps = _itDis();
        req->focus   = TRUE;
        req->drsp    = sp;
        sp->s_bload  = bload;
        sp->s_pbuf   = 0;
        sp->s_lastOp = op;
        sp->s_bdirty = FALSE;
        req->drpid   = RUNpid;
        req->drbuff  = tampon;
        req->count   = n;
        req->Rhandle = sp->s_ft->fRhandle;
        req->drop    = op;
        ret          = _io_op(req);
        _itRes(ps);
        return(ret);
}

/*----------------------------------------------------------------------
 * Fwrite - ecrire dans un fichier
 *----------------------------------------------------------------------
 */
_Fwrite(sp, buff, count)
stream_entry        *sp;
uchar        *buff;
unsigned      count;
{
    int                 ps, ret, countI, reste;
    struct  ioDosReq    req;

    ps = _itDis();
    Tasktab[RUNpid].terrno = 0;
    if (!isAccessEnable(sp, IO_WRITE))  {
        Tasktab[RUNpid].terrno = EACCES;
        _itRes(ps);
        return(RERR);
    }
    if (count == 0) {
        _itRes(ps);
        return(0);
    }

    if (sp->s_lock == FALSE) /* le fichier n'est pas lock‚ par cette tache */
       /* verifier si le fichier est lock‚ par une autre tache */
       _waitfUNLOCK(m_Getpid(), &sp->s_ft->flock);

    if (sp->s_lastOp != IO_WRITE) {
        if (sp->s_pbuf != 0)
                sp->s_pos -= sp->s_limit; /* repositionner le pointeur logique en debut de tampon */
        sp->s_lastOp = IO_WRITE;
    }

    m_printf("WRITE : s_Pos = %ld, pbuf = %d\n", sp->s_pos, sp->s_pbuf);

/*    if (sp->s_buf == NULLPTR)
        return(_requete(sp, &req, buff, IO_WRITE, count, FALSE));*/

    /* gestion avec buffer */
    countI = count;     /* nb initialement demand‚ */

    if (count > (reste = IOBUFSZ - sp->s_pbuf)) {
        /* terminer de remplir le tampon E/S */
        fastcpy(&sp->s_buf[sp->s_pbuf], buff, reste);
        count -= reste;
        _requete(sp, &req, sp->s_buf, IO_WRITE, IOBUFSZ, TRUE);
        if (count > IOBUFSZ) {
            /* IO en une passe */
            _itRes(ps);
            return(_requete(sp, &req, &buff[reste], IO_WRITE, count, FALSE));
        }
        else {
            /* copier buffer dans le tampon E/S */
            fastcpy(sp->s_buf, &buff[reste], count);
            sp->s_pbuf += count;
            sp->s_bdirty = TRUE;
       }
    }
    else {
       fastcpy(&sp->s_buf[sp->s_pbuf], buff, count);
       sp->s_pbuf += count;
       sp->s_bdirty = TRUE;
    }
    m_printf("FIN WRITE : s_Pos = %ld, pbuf = %d\n", sp->s_pos, sp->s_pbuf);
    _itRes(ps);
    return(countI);
}

isAccessEnable(sp, op)
stream_entry *sp;
int op;
{
        if (op == IO_READ) {
                if (sp->s_access & O_WRONLY) /* fichier interdit en LECTURE */
                        return(FALSE);
        }
        if (op == IO_WRITE) {
                if (sp->s_access & O_RDONLY) /* fichier interdit en ECRITURE */
                        return(FALSE);
        }
        return(TRUE);
}
