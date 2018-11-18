/* iodos.c */

#include "sys.h"
#include "conf.h"
#include "sem.h"
#include "io.h"
#include "iodos.h"
#include "init.h"
#include "country.h"

SEM    iosem;      /* semaphore de synchronisation E/S */

LOCAL  struct ioDosReq *headR  = IONULL;
LOCAL  struct ioDosReq *tailR = IONULL; /* tail & head */

int _errflag = 0;
int  criticalFlag;
int curr_floppy = 0;
extern int sysrun;
extern int nflp_phy;
int iorun;

#define RETRY 01
#define ABORT 03

extern char *errStr[];

/*----------------------------------------------------------------------------
 * IOTSK - gere les appels systemes IO DOS
 *----------------------------------------------------------------------------
 */
TASK _ioTsk()
{
      int       ps, pid, ret;
      long      lret, pos, Rh, Rl;
      stream_entry *sp;
      extern long __iomove();

      /* ouvrir la session d'erreur */
      if (_forceGetSession(VS9) == RERR)
                m_Shutdown();

      iosem = m_Creatsem(0,"io");
      iorun = TRUE;
      criticalFlag = FALSE;
      ps = _itDis();
      while(iorun) {
          lret = -1L;
          _waitsem(iosem,STAIL);
          if (!iorun) {
                _ioStop();
                continue;
          }

          while (headR != IONULL) {
                sp = headR->drsp;
                _errflag = 0;

                /* On utilise DRSEEK comme ptr sur Stream */
                if (headR->focus && ((pos = sp->s_pos) != sp->s_ft->fRpos)) {
                        /*m_Printf("IOTASK   FOCUS\n");*/
                        ret = _iomove(headR->Rhandle, pos, 0);
                        /*if (ret == -1)
                                kprintf("IOTASK ERR   errflag = %d HANDLE = %d\n", _errflag, headR->Rhandle);*/
                        sp->s_ft->fRpos = pos;
                }
                switch(headR->drop) {
                case IO_OPEN    :ret = _ioopen(  headR->access,  headR->drbuff);break;
                case IO_CREATE  :ret = _iocreat( headR->mode,    headR->drbuff);break;
                case IO_READ    :ret = _ioread(  headR->Rhandle, headR->drbuff, headR->count);
                                 if (ret > 0) {
                                        sp->s_pos      += (long)ret;
                                        sp->s_ft->fRpos = sp->s_pos;
                                 }
                                 break;
                case IO_WRITE   :ret = _iowrite( headR->Rhandle, headR->drbuff, headR->count);
                                 if (ret > 0) {
                                        sp->s_pos      += (long)ret;
                                        sp->s_ft->fRpos = sp->s_pos;
                                 }
                                 break;
                case IO_CLOSE   :ret = _ioclose( headR->Rhandle);break;
                case IO_REMOVE  :ret = _ioremove(headR->drbuff);break;
                case IO_MOVE    :lret = _iomove(  headR->Rhandle, (long)headR->drseek, headR->count);
                                 ret = (int)lret;
                                 if (ret >= 0) {
                                        sp->s_pos       = lret;
                                        sp->s_ft->fRpos = sp->s_pos;
                                 }
                                 break;
                case IO_GETFATT :ret = _iogetfatt(headR->drbuff);
                                 headR->count = ret;break;
                case IO_SETFATT :ret = _iosetfatt(headR->drbuff, headR->count);break;
                case IO_CHDIR   :/* inutilise */break;
                case IO_MKDIR   :ret = _iomkdir(headR->drbuff);break;
                }
                if (!iorun) {
                        _ioStop();
                        continue;
                }
                pid = headR->drpid;
                headR->drstat = ret;
                headR->drseek = lret;
                headR = headR->drnext;

                /* compte rendu d'operation */
                Tasktab[pid].terrno = trErrTable[_errflag];

/*                kprintf("Errflag = %d %s\n", _errflag, errStr[Tasktab[pid].terrno]);*/

                /* reveiller le process appelant */
                m_Msgsync(pid);
          }
      }
      _ioStop();
      _itRes(ps);
}
/*----------------------------------------------------------------------------
 * _stopIO
 *----------------------------------------------------------------------------
 */
_ioStop()
{
      /* stopper les requetes en cours */
      headR = tailR = IONULL;

      /* envoyer message de fin a SHUTDOWN  */
      m_Msgsync(SHTDWN);
}

/*----------------------------------------------------------------------------
 * ioDosEnq - place une requete
 *----------------------------------------------------------------------------
 */
_ioDosEnq(req)
struct ioDosReq *req;
{
   struct ioDosReq *p , *q;

   if (headR == IONULL)  {
       headR       = req;
       req->drnext = IONULL;
       tailR       = req;
       return;
   }

   q = headR;
   while ((p = q->drnext) != IONULL)
           q = p;

   q->drnext = req;
   req->drnext = IONULL;
}



/*----------------------------------------------------------------------------
 * _io_op - realiser une operation E/D disque
 *----------------------------------------------------------------------------
 */
_io_op(req)
struct  ioDosReq  *req;  /* requete d'IO */
{

    int ps,stat;
    uchar null;

    ps = _itDis();

    /* placer la requete dans le file */
    if (iorun)
        _ioDosEnq(req);
    else {
        req->drstat = RERR;
        _itRes(ps);
        return(RERR);
    }

    /* se suspendre jusqu'… la fin de l'op‚ration */
    m_Msgclr();
    _sigsem(iosem, NOSCHEDUL);  /* reveiller le process manager d'E/S disque */
    m_Msgwait(&null,-1,&stat);    /* attendre la fin de l'E/S */

    /* On est r‚veill‚ */
    _itRes(ps);
    return(req->drstat);
}

/*----------------------------------------------------------------------------
 * _checkINT15 - gestion  1) des delais floppy
 *                        2) des suspensions sur E/S
 *                        3) des reveils en fin d'E/S sur IT
 *----------------------------------------------------------------------------
 */
_checkINT15(subfunc, ss)
{
return(0);
}

_3checkINT15(subfunc, ss)
{
    int ps, bios_ret, stat;
    uchar null;
    LOCAL int pid;


    ps = _itDis();
    switch(subfunc & 0xff00) {
    case 0x9000 :
                switch(subfunc & 0x00ff) {
                case 0xFD :
                case 0xC0 :     m_Sleep(1);           /* laisser le moteur demarrer */
                                /*m_Printf("MOTOR DELAY\n");*/
                                bios_ret = 2;
                                break;

                case 0xFC :     m_Gsleep(1);
                                /*m_Printf("MOTOR FC\n");*/
                                bios_ret = 2;
                                break;

                default:        /* se suspendre pendant la duree de l'E/S */
                                /*m_Printf("E/S SUSPEND\n");*/
                                /*while (1) {
                                m_Msgwait(&null, 3, &stat);
                                if (stat == MSG_TIMEOUT) {
                                         if (_int24bis() == ABORT) {
                                                bios_ret = 2;
                                                break;
                                         }
                                         else   continue;
                                }
                                bios_ret = 1;
                                break;
                                }*/
                                _msgwait(E_S, &null, 1, &stat);
                                bios_ret = 1;
                                break;
                }
                break;

    case 0x9100 :
                switch(subfunc & 0x00ff) {
                case 0  :
                case 1  :       /* arrive lors de l'interruption disque ou floppy */
                                 _msgsndI(E_S, 0, TRUE);
                                 bios_ret = 1;
                                 /*m_Printf("E/S REPRISE\n");*/
                                 break;

                default :       /*m_Printf("INT15A\n");*/
                                bios_ret = 0;
                                break;
                }

    default:    /*m_Printf("INT15B\n");*/
                bios_ret = 0;
    }

    /* appeler l'INT15 BIOS Normalement */
    _itRes(ps);
    return(bios_ret);
}
_2checkINT15(subfunc, ss)
unsigned subfunc, ss;
{
    int ps, bios_ret;
    LOCAL finES = FALSE;

    ps = _itDis();
    switch(subfunc & 0xff00) {
    case 0x9000 :
                switch(subfunc & 0x00ff) {
                case 0xFD :
                case 0xC0 :     m_Sleep(1);           /* laisser le moteur demarrer */
                                m_Printf("MOTOR DELAY\n");
                                bios_ret = 2;
                                break;

                case 0xFC :     m_Gsleep(1);
                                m_Printf("MOTOR FC\n");
                                bios_ret = 2;
                                break;

                default:        /* se suspendre pendant la duree de l'E/S */
                                Tasktab[E_S].tstate = SLEEP;
                                Tasktab[E_S].tevent = EV_SUSP;
                                m_Printf("E/S SUSPEND\n");
                                _swpProc();
                                bios_ret = 1;
                                break;
                }
                break;

    case 0x9100 :
                switch(subfunc & 0x00ff) {
                case 0  :
                case 1  :       /* arrive lors de l'interruption disque ou floppy */
                                if ((Tasktab[E_S].tstate == SLEEP) &&
                                    (Tasktab[E_S].tevent == EV_SUSP)) {
                                        _setrdy(E_S, NOSCHEDUL);
                                        bios_ret = 1;
                                     m_Printf("E/S REPRISE\n");
                                }
                                else {
                                        /* normalement impossible */
                                        m_Printf("ERREUR ...\n");
                                        m_Printf("Event = %x - EV susp = %x\n",Tasktab[E_S].tevent,EV_SUSP);
                                        bios_ret = 2;
                                }
                                break;

                default :       m_Printf("INT15A\n");bios_ret = 0;
                                break;
                }

    default:    m_Printf("INT15B\n");bios_ret = 0;
    }

    /* appeler l'INT15 BIOS Normalement */
    _itRes(ps);
    return(bios_ret);
}

/*----------------------------------------------------------------------------
 * _int24 - handler de gestion des erreurs d'E/S
 *----------------------------------------------------------------------------
 */
_int24(si, bp, di, ax)
unsigned si, bp, di, ax;
{
#define RETRY 01
#define ABORT 03

        char c, drive;
        int sessionTarget,  ret;

        /* recuperer le handle de session du process
         * appelant
         */
        sessionTarget = m_GetProcSessionHandle(headR->drpid);

        if (_uErrHandler != (int (*)())NULLPTR) {
                ret = (*_uErrHandler)(ax, di, sessionTarget);
                if (ret != RETRY && ret != ABORT)
                        ret = ABORT;
                return(ret);
        }

        drive = 'A'+(ax & 0xff);


        /* effacer les precedants messages */
        m_Seprintf(VS9, "\033[2J\033[0;0H");

        /* selectionner la session de la tache en erreur */
        _chgActiveSessionSys(VS9, VS9);

        criticalFlag = TRUE;
        switch(di) {
        case 0  : m_Seprintf(VS9, errDriveWp, drive);break;
        case 1  : m_Seprintf(VS9, errUnkUnit, drive);break;
        case 2  : m_Seprintf(VS9, errDriveNr, drive);break;
        case 3  : m_Seprintf(VS9, errUnkComm, drive);break;
        case 4  : m_Seprintf(VS9, errBadCRC, drive);break;
        case 5  : m_Seprintf(VS9, errBadReq);break;
        case 6  : m_Seprintf(VS9, errSeekErr, drive);break;
        case 7  : m_Seprintf(VS9, errUnkMtyp, drive); break;
        case 8  : m_Seprintf(VS9, errSecNotF, drive);break;
        case 9  : break;
        case 10 : m_Seprintf(VS9, errWriteF, drive);break;
        case 11 : m_Seprintf(VS9, errReadF, drive);break;
        case 12 : m_Seprintf(VS9, errGenFail, drive);break;
        default : break;
        }
        m_Seprintf(VS9, errHandM);
        while (1) {
                if ((c = m_SessionGetch(VS9)) == 'R' || c == 'r') {
                        ret = RETRY;
                        break;
                }
                else
                        if (c == 'A' || c == 'a') {
                                ret = ABORT;
                                break;
                        }
                        else
                                m_Beep(1000);
        }

        /* replacer la session appelante */
        m_ChgActiveSession(sessionTarget);

        return(ret);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * _vflopp - gestion verification du floppy logique
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
_vflopp(apath)
char *apath; /* path absolu */
{
   int drive;

   /* si floppy, controler gestion unit‚s logiques */
   strlwr(apath);
   if (apath[1] != ':')
        return;
   drive = apath[0] - 'a';
   if (drive <= 1) {
       curr_floppy = drive;
       _verify_floppy();
   }
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * verify_floppy - verifier si un seul floppy physique gere 2 drives logiques
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
_verify_floppy()
{
   char drive;
   unsigned char restore,
                 far *ptr = 0x00000504; /* RAM bios location */
   int sessionTarget;

   /* si # de floppy > 1, supprimer verif */
   if (nflp_phy > 1)
       return;


   if (*ptr != curr_floppy) { /* changer de drive logique */
        *ptr = curr_floppy;
        drive = 'A' + curr_floppy;
        sessionTarget = m_GetProcSessionHandle(RUNpid);

        /* effacer les precedants messages */
        m_Seprintf(VS9, "\033[2J\033[0;0H");

        _chgActiveSessionSys(VS9, VS9);
        m_Seprintf(VS9, floppyM1, drive);
        m_Seprintf(VS9, floppyM2);
        m_SessionGetch(VS9);
        m_ChgActiveSession(sessionTarget);
   }
}

