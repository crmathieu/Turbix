/* fdesc.c */

#include "sys.h"
#include "conf.h"
#include "error.h"
#include "io.h"
#include "const.h"
#include "file.h"
#include "fsopen.h"
#include "init.h"
#include "iodos.h"
#include "\tc\include\string.h"

stream_entry       Streamtab[NSTREAM];   /* streams table  */
fslot              Filetab[F_NSLOT];     /* open files     */

/*----------------------------------------------------------------------
 * init_stream - stream table initialization 
 *----------------------------------------------------------------------
 */
_init_stream()
{
   stream_entry *sp;
   for (sp = &Streamtab[0] ; sp < &Streamtab[NSTREAM] ; sp++ ) {
        sp->s_ft        = NULLFILE;
        sp->s_count     = 0;
        sp->s_pos       = 0L;
        sp->s_limit     = IOBUFSZ;
        sp->s_bload     = FALSE;
        sp->s_streamtyp = FILESTREAM;   /* by default */
        sp->s_pbuf      = 0;
        sp->s_buf       = NULLPTR;
        sp->s_bdirty    = FALSE;
   }
}

/*---------------------------------------------------------------------------
 * getstream - link chaine la file entry passee en parametre a
 *             un slot libre de la stream table. retourne l'adresse du slot
 *---------------------------------------------------------------------------
 */
stream_entry *_getstream()
{
    stream_entry *sp;
    struct taskslot *tp;
    int ps,i;

    ps = _itDis();
    tp = &Tasktab[RUNpid];

    for ( sp = &Streamtab[0] ,i=0; sp < &Streamtab[NSTREAM] ; i++,sp++ )
    {
       if (sp->s_count == 0)     /* slot libre */
       {
           sp->s_count  = 1;
           sp->s_pos    = 0L;
           sp->s_limit  = IOBUFSZ;
           sp->s_bload  = FALSE;
           sp->s_bdirty = FALSE;
           sp->s_pbuf   = 0;
           sp->s_bload  = FALSE;
           sp->s_buf    = NULLPTR;
           _itRes(ps);
           return(sp);
       }
    }
    tp->terrno = ENSTRE; /* stream table overflow */
    _itRes(ps);
    return((stream_entry *)RERR);
}

/*----------------------------------------------------------------------------
 * getfd - cherche un slot libre dans la table des descripteurs du process
 *         appelant . retourne l'offset dans cette table
 *----------------------------------------------------------------------------
 */
_getfd(pid)
int pid;
{
    int slot;
    int ps;

    ps = _itDis();

    for (slot = 0 ; slot < NFD ; slot++)
       if (Tasktab[pid].tfd[slot] == NULLSTREAM) {
          _itRes(ps);
          return(slot);
       }
    Tasktab[RUNpid].terrno = EMFILE; /* too many open files */
    _itRes(ps);
    return(RERR);
}
/*----------------------------------------------------------------------
 * fsinit - initialisation du file system
 *----------------------------------------------------------------------
 */
_fsinit()
{
    int ps;
    ps = _itDis();
    _init_file();
    _itRes(ps);
}

/*---------------------------------------------------------------------------
 * init_file - init open file table
 *---------------------------------------------------------------------------
 */
_init_file()
{
   fslot *fp;

   for ( fp = &Filetab[0] ; fp < &Filetab[F_NSLOT] ; fp++ ) {
       memset(fp->fname, 0, 64);
       fp->fcount =  0;
       fp->fmode  = 0;
       fp->flock  = FALSE;
       fp->fcount = 0;
   }
}

/*---------------------------------------------------------------------------
 * getfileEntry   retourne l'@ du slot de la open file table ou est chargee
 *                l'entree correspondant au nom de fichier "file_name"
 *                si dev = NODEV : ne sert qu'a reserver un slot
 *---------------------------------------------------------------------------
 */
fslot *_getfileEntry(file_name, status, share)
char   *file_name;
int    *status;
{
    char *makeAbsfname();
    fslot *fp, *pfslot;
    int          ps,  i;
    uchar        c;

    pfslot = NULLFILE;
    ps = _itDis();
    if ((file_name = makeAbsfname(file_name)) == NULL) {
        Tasktab[RUNpid].terrno = ENODEV;
        _itRes(ps);
        return((fslot *)RERR);
    }

    for (fp = &Filetab[0]; fp < &Filetab[F_NSLOT]; fp++) {
         if (fp->fcount > 0) {
              if (strcmp(fp->fname, file_name) == 0) {
              /*if (_fnamecmp(fp->fname , file_name)) {*/
                   /*   le fichier est deja ouvert par une
                    *   autre tache : augmenter le nb d'ouvertures
                    *   et retourner l'adresse du slot
                    */
                   if (share) {
                        fp->fcount++;
                        *status = F_LOAD;
                        _itRes(ps);
                        return(fp);
                   }
                   Tasktab[RUNpid].terrno = EACCES;
                   _itRes(ps);
                   return((fslot *)RERR);
              }
         }
         else {
                pfslot = fp;
                break;
         }
    }

    if (pfslot == NULLFILE) { /* plus de slot disponible */
        Tasktab[RUNpid].terrno = ENFILE;
        _itRes(ps);
        *status = F_NOSLOT;
        return((fslot *)RERR);
    }

    /*  le fichier n'est pas en table
     *  initialiser cette nelle entr�e
     */

    memset(fp->fname, 0, 64);
    fastcpy(fp->fname, file_name, strlen(file_name));
    pfslot->fRpos  = 0L;
    pfslot->fcount = 1;
    pfslot->fmode  = 0;
    *status        = F_NOLOAD;
    _itRes(ps);
    return(pfslot);
}


/*---------------------------------------------------------------------------
 * dupfileEntry   une tache de plus travaille sur ce fichier
 *---------------------------------------------------------------------------
 */
_dupfileEntry(fp)
fslot *fp;
{
/*    m_Printf("DUP_ENTRY %s\n",fp->fname);*/
    fp->fcount++;
}

/*----------------------------------------------------------------------
 * copy
 *----------------------------------------------------------------------
 */
copy(dest, source, bytes)
uchar *dest;
uchar *source;
int bytes;
{
    if (bytes <= 0)       return;
    while(bytes--)       *(dest++) = *(source++);
}


/*----------------------------------------------------------------------
 * fextcmp - compare 2 extensions  de fichier
 *----------------------------------------------------------------------
 */
_fextcmp(fdname,file_name)
char  *fdname , *file_name;
{
    char c;
    ushort i ;
    for (i = 0; (i < FEXTLENGTH && ( *fdname++ == *file_name++)) ; i++) ;
    if (i >= FEXTLENGTH)   return(TRUE);
    else
       if (*(--fdname) == ' '&& *(--file_name) == '\0')    return(TRUE);
       else                                                return(FALSE);
}


/*----------------------------------------------------------------------
 * _fnamecmp - compare 2 noms  de fichier
 *----------------------------------------------------------------------
 */
_fnamecmp(fdname,file_name)
char  *fdname , *file_name;
{
    char c;
    ushort i ;
/*    m_Printf("\n%s  -  %s\n",fdname,file_name);
    getchar();*/
    for (i = 0; (i < FNAMLENGTH && ( *(fdname+i) == *file_name++)) ; i++) ;
    if (i >= FNAMLENGTH)
    {
          if (*file_name == '.')  file_name++;
          return(_fextcmp(fdname+FNAMLENGTH,file_name));
    }
    if ((*(fdname+i) == ' ') && ((c = *(--file_name)) == '.'|| c =='\0'))
        return(_fextcmp(fdname+FNAMLENGTH,( c == '.'? ++file_name:file_name)));
    else  return(FALSE);
}
/*---------------------------------------------------------------------------
 * strlen - retourne la longueur d'une string
 *---------------------------------------------------------------------------
 */
strlen2(string)
char *string;
{
  int i = 0;
  while(*(string++)) i++;
  return(i);
}
/*---------------------------------------------------------------------------
 * strcat -
 *---------------------------------------------------------------------------
 */
strcat2(strT, strS)
char *strT, *strS;
{
  fastcpy(&strT[strlen(strT)-1], strS, strlen(strS));
}
/*---------------------------------------------------------------------------
 * strcpy -
 *---------------------------------------------------------------------------
 */
strcpy2(strT, strS)
char *strT, *strS;
{

  fastcpy(strT, strS, strlen(strS));
}

/*---------------------------------------------------------------------------
 * strcmp -
 *---------------------------------------------------------------------------
 */
strcmp2(str1, str2)
char *str1, *str2;
{
        int i, lg1;
        if ((lg1 = strlen(str1)) != strlen(str2))
                return(-1);
        i = 0;
        while (*str1++ == *str2++) i++;
        if (i == lg1) return(0);
        return(-1);
}

/*--------------
 * makeAbsfname
 *--------------
 */
char *makeAbsfname(fname)
char *fname;
{
        /* pdt represente le pointeur sur la table des drives
         * DriveTable qui est allouee dynamiquement et contient
         * les informations relatives aux drives (notamment
         * la directory courante
         */
        char work[64], drive;
        struct taskslot *tp;
        int indice;

        tp = &Tasktab[RUNpid];
        fname = strupr(fname);
        memset(work, '\0', 64);
        if (fname[1] == ':') {
                if (!((pdt + fname[0] - 'A' + 1)->valide)) /* drive invalide */
                        return(NULL);

                if (fname[2] == '\\')
                        /* Nom absolu */
                        return(fname);
                 else   {
                        /* construire le nom absolu */
                        drive = work[0] = fname[0];
                        indice = 2;
                 }
         }
         else {
                if (fname[0] == '\\') { /* ajouter le drive */
                        drive = work[0] = 'A' + (tp->tcurrdev - 1);
                        indice = 1;
                }
                else {
                        drive = work[0] = 'A' + (tp->tcurrdev - 1);
                        indice = 0;
                }
        }
        work[1] = ':';
        work[2] = '\\';
        if (indice != 1) {
                if (strlen((pdt+(drive - 'A' + 1))->path)) {
                        strcpy(&work[3], (pdt+(drive - 'A' + 1))->path);
                        strcat(work, "\\");
                }
        }
        strcat(work, &fname[indice]);  /* Done !*/
        strcpy(fname, work);
        return(fname);
}
/*---------------------------------------------------------------
 * m_Setdrv - change current drive
 *---------------------------------------------------------------
 */
m_Setdrv(drive)  /* A = 1, B = 2, C = 3 etc ... */
{

    if ((pdt + drive)->valide == TRUE) {
        Tasktab[RUNpid].tcurrdev = drive;

        /* copy default directory for this drive */
        strcpy(Tasktab[RUNpid].tcurrdir,(pdt + drive)->path);
    }
    else {
        Tasktab[RUNpid].terrno = ENODEV;
        return(RERR);
    }
    Tasktab[RUNpid].terrno = 0;
    return(ROK);
}

/*---------------------------------------------------------------
 * m_Chdir - change current working diectory
 *---------------------------------------------------------------
 */
m_Chdir(dirPath)
char *dirPath;
{
        char work[64], aux[64];
        int fd, i;


        /* forbid name including drive letter */
        if (dirPath[1] == ':') {
                Tasktab[RUNpid].terrno = ENOPATH;
                return(RERR);
        }

        /* 1) make sure directory exists */
        strcpy(work, dirPath);
        if (strlen(work) && work[strlen(work)-1] == '\\') {
                work[strlen(work)-1] = '\0';
        }

        strcpy(aux, work);
        strcat(work, "\\___0412.tmp");
        if ((fd = m_Creat(work, 0)) < 0) {
                return(RERR);
        }
        m_Close(fd);
        _removeFile(work);

        /* 2) update task's default directory */

        /* write directory name */
        strcpy(work, (pdt+Tasktab[RUNpid].tcurrdev)->path);
        if (aux[0] == '\\')
                strcpy(work, aux);
        else {
                strcat(work, "\\");
                strcat(work, aux);
        }

        strupr(work);
        memset(Tasktab[RUNpid].tcurrdir, 0, 64);
        fastcpy(Tasktab[RUNpid].tcurrdir, work, strlen(work));
        Tasktab[RUNpid].terrno = 0;
        return(ROK);
}
/*------------------------------------------
 * m_Getcwd - get current working directory
 *------------------------------------------
 */
m_Getcwd(dirPath)
char *dirPath;
{
        strcpy(dirPath, Tasktab[RUNpid].tcurrdir);
        return(ROK);
}
/*------------------------------------------
 * m_Mkdir - create directory
 *------------------------------------------
 */
m_Mkdir(dirPath)
char *dirPath;
{
    int                    ret;
    struct ioDosReq        req;


        /* construire la requete */
        req.drpid   = RUNpid;
        req.drbuff  = dirPath;
        req.drop    = IO_MKDIR;
        req.focus   = FALSE;
        ret         = _io_op(&req);
        return(ret);
}
/*------------------------------------------------------------
 * m_SetErrHandler - initialize error handler function pointer
 *------------------------------------------------------------
 */
m_SetErrHandler(newhandler)
int (* newhandler)();
{
        _uErrHandler = newhandler;
}
