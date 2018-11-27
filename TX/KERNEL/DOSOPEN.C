/* dosopen.c */


#include "sys.h"
#include "conf.h"
#include "error.h"
#include "io.h"
#include "const.h"
#include "file.h"
#include "fstream.h"
#include "fsopen.h"
#include "iodos.h"

#define SHARE    TRUE
#define NOSHARE  FALSE

/* translation du mode de cr-ation MKD -> DOS */
unsigned ioCreatMode[] =       {0x20,   /* 0 S_IFREG (archive) */
                                0x01,   /* 1 S_IREAD */
                                0x20,   /* 2 S_IWRITE */
                                0x20,   /* 3 S_IREAD|S_IWRITE */
                                0x10,   /* 4 S_IDIR */
                                0x10,   /* 5 */
                                0x10,   /* 6 */
                                0x10,   /* 7 */
                                2};     /* 8 S_IHI */

extern fslot *_getfileEntry();
extern int *_MXmalloc();

/*----------------------------------------------------------------------
 * openFile - ouvrir un fichier avec un mode particulier :
 *            O_RDONLY          0
 *            O_WRONLY          1
 *            O_RDWR            2
 *            O_NDELAY          4
 *            O_APPEND          8
 *            O_CREAT          16
 *            O_TRUNC          32
 *            O_EXCL           64
 *            O_SYNCW         128
 *----------------------------------------------------------------------
 */
_openFile(pid, path, access, mode)
char *path;  /* mode ne sert que lorsque O_CREAT est positionn- */
{
    int                  ps ,fd, status, handle, acc;
    fslot               *fp;
    stream_entry        *sp;
    struct  ioDosReq     req;
    char                *makeAbsfname(), ch, att;

    char                 auxPath[80];

    strcpy(auxPath, path);
    acc = (access & (~(O_TRUNC|O_APPEND|O_EXCL|O_CREAT)));
    if (access & O_TRUNC) { /*  RAZ fichier */
        /* supprimer le fichier, puis le recreer */
        att = _getFatt(&req, auxPath);
        if (_removeFile(auxPath))  /* le fichier est d-j- utilis- */
                return(RERR);
        if ((fd = m_Creat(auxPath, 0)) < 0)
                return(RERR);
        m_Close(fd);
        _setFatt(&req, auxPath, att);
    }

    ps = _itDis();

    /* allouer un slot dans la table des fichiers */
    if ((fp = _getfileEntry(auxPath, &status, SHARE)) == (fslot *)RERR) {
         _itRes(ps);
         return(RERR);
    }

    /*  allouer un slot dans la stream table  */
    if ((sp = _getstream()) == (stream_entry *)RERR) {
         fp->fcount--;;
         _itRes(ps);
         return(RERR);
    }

    /*  allouer un slot dans la fdesc table du process */
    if ( (fd = _getfd(pid)) == RERR) {
          fp->fcount--;
          sp->s_count--;
          _itRes(ps);
          return(RERR);
    }

    /* gestion floppies logiques */
    _vflopp(auxPath);

    /* suivant le type d'acc-s, cr-er ou non un tampon d'E/S */
    /* si Read ET Write ou CREAT : PAS de tampon */
    if (!(access & O_RDWR) && !(access & O_CREAT)) {
        /* allouer buffer tampon de type SPECIAL */
/*        if (((int *)sp->s_buf = _MXmalloc(pid, IOBUFSZ, SPECIALB)) == (int *)NULL) {
                fp->fcount--;
                sp->s_count--;
                Tasktab[pid].tfd[fd]  = NULLSTREAM;
                sp->s_buf             = NULLPTR;
                Tasktab[RUNpid].terrno = ENOMEM;
                _itRes(ps);
                return(RERR);
        }*/
    }
/*    else
        sp->s_buf = NULLPTR;*/

        /* allouer buffer tampon de type SPECIAL */
        if (((int *)sp->s_buf = _MXmalloc(pid, IOBUFSZ, SPECIALB)) == (int *)NULL) {
                fp->fcount--;
                sp->s_count--;
                Tasktab[pid].tfd[fd]  = NULLSTREAM;
                sp->s_buf             = NULLPTR;
                Tasktab[RUNpid].terrno = ENOMEM;
                _itRes(ps);
                return(RERR);
        }


    /* chainer la stream table a la fdesc table du process */
    Tasktab[pid].tfd[fd] = sp;
    sp->s_ft             = fp;


    /* initialiser les variables de travail */
    sp->s_pos     = 0L;
    sp->s_mode    = mode;
    sp->s_lastOp  = IO_MOVE;
    sp->s_access  = acc;
    sp->s_limit   = IOBUFSZ;   /* taille du tampon d'E/S */

    if (fp->fcount > 1) { /* fichier d-j- ouvert */
        Tasktab[pid].terrno = 0;
        _itRes(ps);
        return(fd);
    }

    /* construire la requete */
    req.drpid   = pid;
    req.drbuff  = auxPath;
    req.access  = acc;
    req.focus   = FALSE;
    req.drop    = IO_OPEN;
/*    kprintf("OPEN : CONSTRUIRE REQ\n");*/
    if ((handle = _io_op(&req)) < 0) {
/*          kprintf("OPEN : ERRNO = %d\n", Tasktab[pid].terrno);*/
          Tasktab[pid].tfd[fd]  = NULLSTREAM;
          sp->s_streamtyp       = FILESTREAM;
          if (sp->s_buf != NULLPTR)
                _xfree(sp->s_buf, pid);
          fp->fcount--;
          sp->s_count--;
          _itRes(ps);
          return(RERR);
    }

/*    kprintf("OPEN : OK\n");*/
    fp->fRhandle = handle;
    fp->fRpos    = 0L;

    if (access & O_APPEND) { /* se placer en fin de fichier */
/*        m_Printf("O_APPEND DETECTE\n");*/
        m_Lseek(fd, 0L, SEEK_END);
    }

    Tasktab[pid].terrno = 0;
    _itRes(ps);
    return(fd);
}

/*----------------------------------------------------------------------
 * m_Creat - creer un fichier avec un mode particulier :
 *                  S_IREAD      autoriser lecture
 *                  S_IWRITE     autoriser ecriture
 *                  S_IDIR       creer directory
 *                  S_IHI        creer fichier cach-
 *----------------------------------------------------------------------
 */
BIBLIO m_Creat(path, mode)
char *path;
int   mode;
{
    int                 ps , status, pid, handle, fd;
    fslot              *fp;
    stream_entry       *sp;
    struct ioDosReq     req;
    char                auxPath[80];

    if (!modeOk(mode)) {
        Tasktab[RUNpid].terrno = EINVAL;
        return(RERR);
    }

    strcpy(auxPath, path);

    ps = _itDis();

    /* allouer un slot dans la table des fichiers */
    if ((fp = _getfileEntry(auxPath, &status, NOSHARE)) == (fslot *)RERR) {
         _itRes(ps);
         return(RERR);
    }

    if (fp->fcount > 1) { /* fichier d-j- ouvert : ERREUR */
        fp->fcount--;
        _itRes(ps);
        return(RERR);
    }

    /*  allouer un slot dans la stream table  */
    if ((sp = _getstream()) == (stream_entry *)RERR) {
         fp->fcount = 0;
         _itRes(ps);
         return(RERR);
    }

    /*  allouer un slot dans la fdesc table du process */
    if ( (fd = _getfd(pid = m_Getpid())) == RERR) {
          fp->fcount  = 0;
          sp->s_count = 0;
          _itRes(ps);
          return(RERR);
    }

    /* PAS de tampon pour les fichiers en cr-ation */

        /* allouer buffer tampon de type SPECIAL */
        if (((int *)sp->s_buf = _MXmalloc(pid, IOBUFSZ, SPECIALB)) == (int *)NULL) {
                fp->fcount = 0;
                sp->s_count = 0;
                Tasktab[pid].tfd[fd]  = NULLSTREAM;
                sp->s_buf             = NULLPTR;
                Tasktab[RUNpid].terrno = ENOMEM;
                _itRes(ps);
                return(RERR);
        }

/*    sp->s_buf            = NULLPTR;*/

    /* chainer la stream table a la fdesc table du process */
    Tasktab[pid].tfd[fd] = sp;
    sp->s_ft             = fp;

    /* initialiser les variables de travail */
    sp->s_pos     = 0L;
    sp->s_lastOp  = IO_MOVE; /**/
    sp->s_mode    = ioCreatMode[mode];
    sp->s_limit   = IOBUFSZ;   /* taille du tampon d'E/S */


    /* construire la requete */
    req.drpid   = pid;
    req.drbuff  = auxPath;
    req.focus   = FALSE;
    req.mode    = ioCreatMode[mode];
/*    kprintf("CREAT: MODE = %x\n",req.mode);*/
    req.drop    = IO_CREATE;

    if ((handle = _io_op(&req)) < 0) {
          sp->s_streamtyp               = FILESTREAM;
          fp->fcount                    = 0;
          sp->s_count                   = 0;
          Tasktab[RUNpid].tfd[fd]       = NULLSTREAM;
          _itRes(ps);
          return(RERR);
    }
    fp->fRhandle = handle;
    fp->fRpos    = 0L;
    Tasktab[pid].terrno = 0;
    _itRes(ps);
    return(fd);

}

/*----------------------------------------------------------------------
 * _closeFile  - fermer un fichier
 *----------------------------------------------------------------------
 */
_closeFile(sp, pid, fd)
stream_entry *sp;
{
  int                    ps, ret;
  fslot                 *fp;
  struct ioDosReq        req;


        ps = _itDis();
        fp = sp->s_ft;
/*        m_Printf("CLOSE: TASK = %s Name = %s  Count = %d Scount = %d fd = %d\n",Tasktab[pid].tname, fp->fname, fp->fcount, sp->s_count, fd);*/

        /* decrementer le compteur d'instances en STREAM et en FSLOT */
        fp->fcount--;
        if ( --sp->s_count == 0)  {
                if (sp->s_bdirty) {
                        m_Printf("CLOSE: flush buffer\n");
                        _requete(sp, &req, sp->s_buf, IO_WRITE, sp->s_pbuf, TRUE);
                }
/*                m_Printf("CLOSE: Avant requete Count = %d\n", fp->fcount);*/
                if (fp->fcount == 0) { /* aucune tache n'utilise cette entree */
/*                        kprintf("CLOSE : CONSTRUIRE REQ\n");*/
                    /* construire la requete */
                    req.drpid   = pid;
                    req.Rhandle = sp->s_ft->fRhandle;
                    req.drop    = IO_CLOSE;
                    req.focus   = FALSE;
                    ret         = _io_op(&req);
                    if (ret < 0)
                        m_printf("CLOSE : ERRFLAG = %d\n", _errflag);

                }
                /* liberer le tampon */
                if (sp->s_buf != NULLPTR)
                        _xfree(sp->s_buf, pid);
                sp->s_pbuf      = 0;
                sp->s_streamtyp = FILESTREAM;
                sp->s_ft        = NULLFILE;
                sp->s_buf       = NULLPTR;
        }
        Tasktab[pid].tfd[fd]  = NULLSTREAM;   /* liberer slot fd */
        Tasktab[pid].terrno = 0;
        _itRes(ps);
        return(ret);
}

/*----------------------------------------------------------------------
 * _removeFile  - supprimer un fichier
 *                valeurs retourn-es:
 *                      0               OK
 *                Dos
 *                      2               ERROR_FILE_NOT_FOUND
 *                      3               ERROR_PATH_NOT_FOUND
 *                      5               ERROR_ACCESS_DENIED
 *----------------------------------------------------------------------
 */
_removeFile(path)
char *path;
{
    int                    ps, ret, status;
    fslot                 *fp;
    struct ioDosReq        req;
    char                   auxPath[80];

    strcpy(auxPath, path);
    ps = _itDis();

    /* allouer un slot dans la table des fichiers */
    if (((fp = _getfileEntry(auxPath, &status, NOSHARE)) == (fslot *)RERR) ||
         (status == F_LOAD)) { /* le fichier est d-j- utilis- : REFUSER */
             _itRes(ps);
             return(RERR);
    }
/*   kprintf("REMOVE: Name = %s  Count = %d\n",fp->fname, fp->fcount);*/

        /* decrementer le compteur d'instances en Table des fichiers */
        if (--fp->fcount == 0) { /* aucune tache n'utilise cette entree */
/*                    kprintf("REMOVE : CONSTRUIRE REQ\n");*/
                    /* construire la requete */
                    req.drpid   = RUNpid;
                    req.drbuff  = auxPath;
                    req.drop    = IO_REMOVE;
                    req.focus   = FALSE;
                    ret         = _io_op(&req);
/*                    if (ret < 0)
                        kprintf("REMOVE : ERRFLAG = %d\n", _errflag);*/
                    _itRes(ps);
                    return(ret);
        }
        /* Normalement impossible */
        _itRes(ps);
        return(RERR);
}

_setFatt(req, path, att)
struct ioDosReq *req;
char *path;
{
        int ret, ps;

        ps = _itDis();

        /* construire la requete */
        req->drpid   = RUNpid;
        req->drbuff  = path;
        req->count   = att;
        req->drop    = IO_SETFATT;
        req->focus   = FALSE;
        ret          = _io_op(req);
        _itRes(ps);
        return(ret);
}
_getFatt(req, path)
struct ioDosReq *req;
char *path;
{
        int ret, ps;

        ps = _itDis();

        /* construire la requete */
        req->drpid   = RUNpid;
        req->drbuff  = path;
        req->drop    = IO_GETFATT;
        req->focus   = FALSE;
        ret          = _io_op(req);
        if (ret != -1)
                 ret = req->count;
        _itRes(ps);
        return(ret);
}
