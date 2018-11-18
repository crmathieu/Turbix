/* config.c */

#include "xed.h"
#include <fcntl.h>
#include "ext_var.h"

/* version MSC */
#include <stdio.h>
#include <fcntl.h>
#include <process.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

/* version TC */  /*
#include <\tc\include\stdio.h>
#include <\tc\include\fcntl.h>
#include <\tc\include\process.h>
#include <\tc\include\dos.h>
#include <\tc\include\ctype.h>
#include <\tc\include\sys\stat.h>
#include <\tc\include\io.h>
                    */

/*---------
 * doBuild
 *---------
 */
doBuild(mp, num)
struct Menu *mp;
{
   FILE  *fp,*fpred;
   char  *ptr, aux[80];
   int    fdconf, k, i, ret, retL, save_fdcurr;
   struct OBJ errwin, *wmm, msgwin;

   /* changer de dir courante -> dir USER */
   memset(aux, '\0', 80);
   strcpy(aux, conf.u.c_p[5]);
   _performChgDir(aux);

   /*  Voir s'il existe un fichier de configuration .
    *  si oui , reecrire les commandes par leur
    *  chemin absolu
    */

   /* creer le fichier de redirection */
   if ((fpred = fopen(fError,"w+")) == NULL)
   {
      sprintf(work, builtStr);
      pushPop_MESS(work, ERRMSG);
      return(FUNC|F10);
   }
   fclose(fpred);

        /* recuperer la position souris (eventuellement pour le noyau) */
        m_getPosAndButtonStatus(&vl->mouX, &vl->mouY, &i);

        save_previous_file(absfname);
        push_pick(absfname);

        /* sauvegarder la configuration */
        memmove(&conf.reload[0], &picktab[0], PICKSIZE * 17);
        doSconfig(mp, 0);

        strcpy(vl->fError, fError);
        strcpy(vl->cdirname, cdirname);
        strcpy(vl->homedir, homedir);
        strcpy(vl->relname, relname);
        strcpy(vl->absfname, absfname);
        strcpy(vl->exename, exename);
        strcpy(vl->currmask, mask);
        strcpy(vl->findStr, findSTR);
        strcpy(vl->replStr, replaceSTR);

        vl->WINSYSink           = WINSYSink;
        vl->WINSYSpaper         = WINSYSpaper;
        vl->WINSYSborder        = WINSYSborder;
        vl->WINSYSblock         = WINSYSblock;
        vl->WINmessInk          = WINmessInk;
        vl->WINmessPaper        = WINmessPaper;
        vl->WINmessBorder       = WINmessBorder;
        vl->WINDIRink           = WINDIRink;
        vl->WINDIRpaper         = WINDIRpaper;

        vl->segVideo            = SEGvideo;
        vl->adapter             = adapter;
        vl->retCode             = 0;

        vl->mkd_shell           = conf.mkd_shell;
        vl->mkd_FS              = TRUE; /*conf.mkd_FS;*/
        vl->mkd_AutoAsk         = conf.mkd_AutoAsk;
        vl->mkd_masm            = conf.mkd_masm;
        vl->mkd_tasm            = conf.mkd_tasm;

        memcpy(&vl->conf, &conf, sizeof(struct configuration));

        /* liberer BIGBUF */
        free(bigbuf);
        free(fillstr);
        free(clippbuf);
        free(lineTab);
        free(linebuf);

        /* inhiber la souris */
        inhibe_mouse();
        showMouse();

/*        _dos_setvect(0x1c, BiosIntclk);*/

        switch(vl->operation) {
        case COMP : exit(1);
        case ASS  : exit(2);
        case MKE  : exit(3);
        case BRW  : exit(8);
        case LNK  : exit(6);
        case RUN  : hideMouse();
                    exit(5);
        case CHK  : exit(12);
        default   : /* replacer dir courante */ /*Home dir */
                    memset(aux, '\0', 80);
                    strcpy(aux, cdirname); /*homedir);*/
                    _performChgDir(aux);
                    hideMouse();
                    exit(4); /* shell */
        }

}

/*
 * creatTURBOC - creer le fichier TURBOC.CFG utilis- par TCC
 *
 */
creatTURBOC()
{
   int fdconf;
   char aux[80];

   /* se placer dans la directory Utilisateur */
   memset(aux, '\0', 80);
   strcpy(aux, conf.u.c_p[5]);


   /* CREER un fichier TURBOC.CFG dans la user dir */

/*   strcpy(aux, conf.u.pd.c_x);*/
/*   strcpy(aux, homedir); /* modif 92 */
   if (strlen(aux))     /* user dir name       */
        strcat(aux, "\\");

   strcat(aux, "turboc.cfg");
   remove(aux);
   if ((fdconf = open(aux,O_CREAT|O_RDWR|O_TEXT, S_IREAD|S_IWRITE)) != -1) {
             if ((strlen(conf.u.pd.c_i) == 0) || (strlen(conf.u.pd.c_l) == 0)) {
                        strcpy(aux, envIncompletStr);
                        pushPop_MESS(aux, ERRMSG);
                        return(-1);
             }
             strcpy(aux, "-I");
/*             strcat(aux,conf.u.pd.c_i);*/
             strcat(aux,conf.u.pd.c_I);
             strcat(aux, "\n");
             write(fdconf, aux, strlen(aux));
             strcpy(aux, "-L");
             strcat(aux,conf.u.pd.c_l);
             write(fdconf, aux, strlen(aux));
             write(fdconf, "\x1A", 1);
   }
   else {
             strcpy(aux, turbocStr);
             pushPop_MESS(aux, ERRMSG);
             return(-1);
   }
   close(fdconf);
   return(0);
}
/*----------
 * doRun
 *----------
 */
doRun(mp, num)
struct Menu *mp;
{
    vl->operation = RUN;
    return(doBuild(mp, num));
}

/*----------
 * doCheck
 *----------
 */
doCheck(mp, num)
struct Menu *mp;
{
    extern int dummyFunc();

    vl->operation = CHK;
    vl->Bfunc = dummyFunc;
    return(doBuild(mp, num));
}

/*----------
 * doMake
 *----------
 */
doMake(mp, num)
struct Menu *mp;
{
    if (creatTURBOC())
        return(ALT|ALT_I);
    vl->operation = MKE;
    return(doBuild(mp, num));
}

/*----------
 * doBrowse
 *----------
 */
doBrowse(mp, num)
struct Menu *mp;
{
    vl->operation = BRW;
    return(doBuild(mp, num));
}

/*----------
 * doCompile
 *----------
 */
doCompile(mp, num)
struct Menu *mp;
{
    if (creatTURBOC())
        return(ALT|ALT_I);
    vl->operation = COMP;
    return(doBuild(mp, num));
}

/*----------
 * doAssemble
 *----------
 */
doAssemble(mp, num)
struct Menu *mp;
{
    vl->operation = ASS;
    return(doBuild(mp, num));
}

/*----------
 * doLink
 *----------
 */
doLink(mp, num)
struct Menu *mp;
{
/*    if (mkd_AutoAsk)
        doLnkChoice(mp, num);*/

    vl->operation = LNK;
    return(doBuild(mp, num));
}
/*----------
 * doShell
 *----------
 */
doShell(mp,num)
struct OBJ *mp;
{
   int ret;

   _save_screen(0, 0, 79, 24, videoBuf);
    vl->operation = 0412;
    return(doBuild(mp, num));

}

/*----------
 * doPreScan
 *----------
 */
doPreScan(mp, num)
struct Menu *mp;
{
}

