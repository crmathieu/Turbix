/* NMX_MAKE */

#include "xed.h"
#include "emsg.h"

#define  M_LEFT_PRESS      2
#define  M_LEFT_RELEASE    4
#define  M_RIGHT_PRESS     8
#define  M_RIGHT_RELEASE  16


#ifdef TURBO
#include <fcntl.h>
#include <process.h>
#define _dos_setvect setvect
#define _dos_getvect getvect
#endif


struct OBJ *messwin;
int Xmess, Ymess;
#define nullstr ""
unsigned SEGvideo;
int adapter;

#define ERRMSG 4
#define NMXMSG 3

#define HBYTE  0xff00
#define LBYTE  0x00ff
#define FIND   1


#define MKE  0xf100
#define COMP 0xf200
#define LNK  0xf300
#define ASS  0xf400

#define BERR        1
#define MKE_NODIFF  2
#define BEXIT       3

/*--------------------
 *   Make utility
 *--------------------
 */

#define EOC_MISSING   6
#define FIN_RELATION  8
#define DEB_RELATION  7
#define FIN_LIGNE     9
#define SCAN_LIGNE    5
#define FIN_MAKE     10

#define TRUE          1
#define FALSE         0

#define TC         1
#define MSC        0

#define CTYPE         1         /* type C      */
#define ATYPE         2         /* type ASM    */
#define LTYPE         3         /* type LINK   */
#define HTYPE         4         /* type HEADER */
#define MTYPE         5         /* type MACRO  */
#define ETYPE        -5         /* type ERREUR */
#define MAXMACRO     10
#define SAVE          6


/* gestion des MACROS */
char  MACROname[MAXMACRO][12];
long  MACROdate[MAXMACRO];
int   MACROfree;             /* 1er slot macro libre */
long  mdate;

char *lnkstr1;
char *lnkstr2;

char **spcc;
char **spasm;
char **splnk;

/*  gestion des parametres lus dans le
 *  fichier de configuration "mdi.pro"
 */

struct configuration conf;

/*  zone de reecriture des commandes si le fichier
 *  de configuration existe
 */

char  compilerName[80];
char  linkerName[80];
char  assemblerName[80];

char MSClnkstr1[] = "%s\\xenstart";
char MSClnkstr2[] =
     ",,%s\\ulk %s\\llibfa %s\\llibfp %s\\llibc %s\\libh,/NOD /DOSSEG /m";

char *MSCspcc[]    = {"cl","-AL","-Zp","-Gs","-c","-Od",NULL};
char *MSCspasm[]   = {"masm",NULL,NULL};
char *MSCsplnk[]   = {"link","@_x041256.lnk",NULL};

char TClnkstr1[]           = "%sC0L ";
char TClnkstr1_shell[]     = "%sC0L %scmdtab";
char TClnkstr1_shellNOFS[] = "%sC0L %scmdtabnf";
char TClnkstr2[]           = ",,%stxsml   %stxwin   %stxspy   %sEMU %sMATHL %sCL";
char TClnkstr2NOFS[]       = ",,%stxsmlnf %stxwinnf %stxshxnf %sEMU %sMATHL %sCL";

char *TCspcc[]    = {"tcc","-ml","-O","-r-","-c",NULL,NULL,NULL,NULL};
char *TCspasm[]   = {"masm",NULL,NULL};
char *TCsplnk[]   = {"tlink","@_x041256.lnk",NULL};


/* buffers de travail */

char *bufline;  /* buffer entree */
char *buftoken; /* buffer ou sont stockes les tokens */
char *argf[100];         /* tableau de pointeurs sur tokens   */
char argfA[50][80];      /* tableau de token Absolus */

/* variables globales de travail */
char *tokenptr;
int arglc,narg;
int line,relativline;
int comptyp;

long filarg0,filargi;
int command;

int analyse,errorlevel;
struct stat entry;

struct OBJ *wm, wmake;
int fmsg, filout, makeError, difference;

int tmp_file;

char statObj[] = " Object file(s)  built : ";
char statExe[] = " Executable file built : ";
int stat_obj, stat_exe;
char tempfile[80];
char work[80];
char wkfile[80];


struct varLink *vl;


unsigned char boxtype[7][11] = {{'Ú','¿','À','Ù','³','Ä','Ã','´','Â','Á','Å'},   /* type 1 */
                                {'Õ','¸','Ô','¾','³','Í','Æ','µ','Ñ','Ï','Ø'},   /* type 2 */
                                {'Ö','·','Ó','½','º','Ä','Ç','¶','Ò','Ğ','×'},   /* type 3 */
                                {'É','»','È','¼','º','Í','Ì','¹','Ë','Ê','Î'},   /* type 4 */
                                {'°','°','°','°','°','°','°','°','°','°','°'},   /* type 5 */
                                {'Ç','¶','Ó','½','º','Ä','Ç','¶','Ò','Ğ','×'},   /* status */
                                {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '},   /* external windows */
};
#define TYP1    0
#define TYP2    1
#define TYP3    2
#define TYP4    3
#define TYP5    4
#define TYP6    5
#define TYP7    6

unsigned long BiosInt10;
char *ptr2;

/********* SOURIS *********/
int mouse_present = 0;
unsigned long BiosInt33;
int mouseEvent = 0;


extern int prolog_it_mouse();

char c_cl[80];   /* comp & linker dir   */
char c_a[80];    /* assembleur dir      */
char c_i[80];    /* include dir         */
char c_z[80];    /* MKD include dir     */
char c_l[80];    /* librairies dir      */
char c_u[80];    /* ulk dir             */
char c_m[80];    /* make file name      */
char c_p[80];    /* paste file name     */
char c_h[80];    /* help dir name       */
char c_x[80];    /* user dir name       */

char auxStr[80];

int mouX, mouY;

#define WMAKESIZE  48

/* offset */
#define OFF_CHECK  1 /*1*/
#define OFF_COMP   1 /*2*/
#define OFF_ASS    1 /*3*/
#define OFF_LINK   1 /*4*/

#define OFF_CFILE  3
#define OFF_AFILE  4
int offset;

/* variables de nombres */
int Cfile, Afile;

/* macro */
#define INF(x,y)  (x < y ? x : y)

int nb_car_per_line;

/*-------------------------------------------------------
 *      M       M      MM      M   N     N
 *      M M   M M     M  M     M   N N   N
 *      M   M   M    M MM M    M   N   N N
 *      M       M   M      M   M   N     N
 *-------------------------------------------------------
 */
main()
{
   void interrupt Dummy_interrupt();

   char c, work2[80];
   FILE  *fp,*fpred;
   int    ret, fdconf, difference, moduleBuild,
          checkfile, working,
          k, typ, i, err;

   struct OBJ errwin, *wmm, msgwin;
   long getMDATE();
   unsigned Key;

   /* recuperer l'@ du vecteur de communication */
   vl = (struct varLink *)_dos_getvect(0xf2);

   /* init video mode */
   SEGvideo = vl->segVideo;
   if (SEGvideo == 0xb000)
        adapter = 0;
   else
        adapter = 1;

   /* recuperer le vecteur BIOS 10h */
   BiosInt10 = (unsigned long)_dos_getvect(0x10);

   /* inhiber le ^C */
   _dos_setvect(0x23, Dummy_interrupt);

   i            = Cfile = Afile = 0;
   makeError    = difference = FALSE;
   wm           = &wmake;
   wmm          = &msgwin;

   ptr2 = (vl->operation == MKE ? mkeStr : vl->operation == ASS ? assStr : compStr);


   /* dupliquer l'environnement */
   dupEnviron();

   initBuffer();
   memset(buftoken, '\0', nb_car_per_line);

   /*  Voir s'il existe un fichier de configuration .
    *  si oui , reecrire les commandes par leur
    *  chemin absolu
    */
        comptyp = vl->conf.c_t; /* type de compilateur */
        strcpy(compilerName,c_cl);
        strcpy(linkerName,c_cl);
        strcpy(assemblerName,c_a);
        if (comptyp == TC) {
            strcat(compilerName,"tcc");
            TCspcc[0] = compilerName;
            strcat(linkerName,"tlink");
            TCsplnk[0] = linkerName;
            if (vl->mkd_masm)
                strcat(assemblerName,"masm");
            else
                if (vl->mkd_tasm)
                        strcat(assemblerName,"tasm");

            TCspasm[0] = assemblerName;
        }
        else {
            strcat(compilerName,"cl");
            MSCspcc[0] = compilerName;
            strcat(linkerName,"link");
            MSCsplnk[0] = linkerName;
            if (vl->mkd_masm)
                strcat(assemblerName,"masm");
            else
                if (vl->mkd_tasm)
                        strcat(assemblerName,"tasm");
            MSCspasm[0] = assemblerName;
        }

   /* suivant type de compilateur , initialiser les tableaux */
   if (comptyp == MSC) {

      lnkstr1 = MSClnkstr1;
      lnkstr2 = MSClnkstr2;
      spcc    = MSCspcc;
      spasm   = MSCspasm;
      splnk   = MSCsplnk;
      narg    = 6; /*5;*/
   }
   else {

      /* tester les elements … lier */
      lnkstr1 = TClnkstr1;
      lnkstr2 = TClnkstr2;

/*      if (vl->mkd_FS) {
                lnkstr2 = TClnkstr2;
                if (vl->mkd_shell)
                        lnkstr1 = TClnkstr1_shell;
                else
                        lnkstr1 = TClnkstr1;
      }
      else {
                lnkstr2 = TClnkstr2NOFS;
                if (vl->mkd_shell)
                        lnkstr1 = TClnkstr1_shellNOFS;
                else
                        lnkstr1 = TClnkstr1;
      }*/


      spcc    = TCspcc;
      spasm   = TCspasm;
      splnk   = TCsplnk;
      narg    = 5;
   }


   MACROfree  = 0;
   errorlevel = 0;
   line = 0;

   /* RAZ Nom d'executable */
   memset(vl->conf.projExe, 0, 80);

   if ((c = vl->conf.u.pd.c_m[0]) == BLANK || c == '\0') {
      strcpy(work, noPrjStr);
      pushPop_MESS(work, ERRMSG);
      refExit(wm, MKE|BEXIT, 0);
   }
   /* ouvrir le fichier de projet */
   if ((fp = fopen(vl->conf.u.pd.c_m,"r")) == NULL) {
      strcpy(work, noPrjFileStr);
      pushPop_MESS(work, ERRMSG);
      refExit(wm, MKE|BEXIT, 0);
   }
   /* dupliquer directorie utilisateur */
   if (strlen(c_x))
         strcpy(auxStr, c_x);
   else {
         strcpy(work, noUsrStr);
         affiche(work, ERRMSG);
         enable_screen();
         return(0);
   }

   redirOn();
   flush_kbd();
   /*                x  y   #c  #l */
   initWin(wm, ptr2, (80-WMAKESIZE)/2, 10, WMAKESIZE, 5, TRUE, FALSE,
           vl->WINSYSink, vl->WINSYSpaper, vl->WINSYSborder, vl->WINSYSblock, W_WIN);

   winPopUp(wm, W_OPEN, 0);

   offset = strlen(checkString)+1;
   print_at(wm, 1, OFF_CHECK, checkString, OLDATT);
   print_FA();

   while  (TRUE)  {
           /* gestion arret du MAKE */
           if (_look_kbd()) { /* caractere tap‚ au clavier */
             Key = _read_kbd();
             if ((Key & 0xff) == ESC) {  /* Stop !!! */
                winPopUp(wm, W_CLOSE);
                strcpy(work, breakMake);
                verify_window(wm, work, nullstr, ERRMSG);
                getkey();
                redirOff();
                refExit(wm, MKE|BEXIT, 0);
             }
           }

     arglc    = 0;
     relativline = 0;
     tokenptr = buftoken;
     analyse = DEB_RELATION;
     memset(buftoken, '\0', nb_car_per_line);

     /* saisir une relation */
     while (analyse != FIN_RELATION) {
        line++;
        getline(fp);                        /* lire une ligne */
        switch(analyse = putargf(&err)) {
        case EOC_MISSING : /* accumuler tokens */
                           sprintf(work, syntErr1, line);
                           pushPop_MESS(work, ERRMSG);
                           redirOff();
                           refExit(wm, MKE|BEXIT, 0);

        case FIN_MAKE    : if (err) {
                                sprintf(work, syntErr2, line);
                                pushPop_MESS(work, ERRMSG);
                                redirOff();
                                refExit(wm, MKE|BEXIT, 0);
                           }
                           goto ExitMake;
        }
        memset(bufline, '\0', nb_car_per_line);
     }

     argf[arglc+1] = NULL;    /* marquer la fin des tokens */

     /* verifier que la syntaxe est correcte */
     if (*argf[1] != ':') {      /* separateur ':' obligatoire */
        sprintf(work, syntErr3, line);
        pushPop_MESS(work, ERRMSG);
        redirOff();
        refExit(wm, MKE|BEXIT, 0);
     }

     /* examiner les dates de creation des differents fichiers */
     checkfile = 2;
     MakeAbsName(arglc);
     if ((filarg0 = stat(argfA[0],&entry)) == 0) { /* le fichier existe */
        filarg0 = entry.st_atime;
        command = FALSE;
        while (checkfile < arglc+1) {

            strcpy(work, argfA[checkfile]);
            strcpy(work2, work);
            memset(&work2[strlen(work)], ' ', 80-(strlen(work)+1));
            print_at(wm, 1, OFF_CHECK, checkString, OLDATT); /**/
            print_nb_at(wm, offset, OFF_CHECK, work2, WMAKESIZE-offset, OLDATT);
            if (stat(argfA[checkfile],&entry) == 0) { /* fichier OK */

                 if (entry.st_atime > filarg0) {
                    command = TRUE;
                    break;
                 }

                 checkfile++;
            }
            else {  /* tester si macro */
                 if (gettype(argf[checkfile]) != MTYPE) {
                     sprintf(work, fileNfStr, line, argf[checkfile]);
                     pushPop_MESS(work, ERRMSG);
                     redirOff();
                     refExit(wm, MKE|BEXIT, 0);
                 }
                 else  {  /* c'est une macro  : poursuivre le test */

                     if ((mdate = getMDATE(argf[checkfile])) > filarg0) {
                          command = TRUE;
                          break;
                     }
                     else
                        if (mdate < 0) {
                           sprintf(work, macNfStr, line, argf[checkfile]);
                           pushPop_MESS(work, ERRMSG);
                           redirOff();
                           refExit(wm, MKE|BEXIT, 0);
                        }
                     checkfile++;
                 }
            }
        }

        /* tester si on doit lancer la commande */
        if (command)
            if (execcom((typ = gettype(argf[2])), &difference, &moduleBuild) == -1)
                goto ExitMake;
     }
     else      /* le fichier en arg0 n'existe pas */
     {
       if (gettype(argf[0]) != MTYPE)  {
          if (execcom((typ = gettype(argf[2])), &difference, &moduleBuild) == -1)
              goto ExitMake;
       }
       else     /* c'est une MACRO */
          if (putMACRO() == -1)
              goto ExitMake;
     }
   }

ExitMake:

   winPopUp(wm, W_CLOSE);
   if (makeError) {
       if (ret = CheckErrorLevel(typ)) {
                winPopUp(wm, W_CLOSE);
                strcpy(work, errStr);
                verify_window(wm, work, nullstr, ERRMSG);
                getkey(); /*_read_kbd();*/
                redirOff();
                refExit(wm, MKE|BERR, ret);
       }
       goto EXIT;
   }
   /* recopier le nom de l'EXE */
   strcpy(wkfile, argfA[0]);
   strcpy(vl->conf.projExe, wkfile);

   if (!difference && !vl->rebuild)
        strcpy(work, fileUpDStr);
   else {
        vl->conf.buildDB = TRUE;
        if (moduleBuild)
                strcpy(work, mmodStr);
        else
                strcpy(work, succStr);
   }
   winPopUp(wm, W_CLOSE);

   if (!vl->rebuild) { /* afficher banniere "success" */
        verify_window(wm, work, nullstr, NMXMSG);
        getkey();
        winPopUp(wm, W_CLOSE);
   }

EXIT:

   vl->retCode = (MKE|BEXIT);
   redirOff();
   inhibe_mouse();
   showMouse();
   free(buftoken);
   free(bufline);
   exit(0);  /* relancer l'editeur */

}
/*ÄÄÄÄÄÄÄÄÄÄ
 * MakeAbsName
 *ÄÄÄÄÄÄÄÄÄÄ
 */
MakeAbsName(ntoken)
{
   char checkStr[80];
   int i;

     for (i=0; i <= ntoken; i++) {

        /* Nom absolu */
        strcpy(checkStr, argf[i]);
        strcpy(argfA[i], auxStr);
        strcat(argfA[i], checkStr);
    }
}


/*ÄÄÄÄÄÄÄÄÄÄ
 * CheckErrorLevel
 *ÄÄÄÄÄÄÄÄÄÄ
 */
CheckErrorLevel(typ)
{
        if (errorlevel < 0) {
          if (typ == CTYPE)
                strcpy(work, noCompStr);
          else
                strcpy(work, noAssStr);
          affiche(work, ERRMSG);
          return(0);
        }
        return(errorlevel);
}

/*---------------------------------------------------
 * getline - saisit une ligne de commande en
 *           supprimant CR et remplacant LF par '\0'
 *---------------------------------------------------
 */
getline(fp)
FILE *fp;
{
   char c;
   int input = 0;

   /* saisir ligne en supprimant les CR */
   while ((c = getc(fp)) != '\n'&& (input < nb_car_per_line) && c != EOF)
             if (c != '\r')   bufline[input++] = c;

   if (c == '\n') {
              bufline[input++] = '\0';
              return(1);
   }
   else {
              bufline[input] = EOF;
              return(0);
  }
}

/*--------------------------------------------
 * putargf - place les @ des tokens dans argf
 *--------------------------------------------
 */
putargf(err)
int *err;
{
   char c;
   int i, home, output;
   struct OBJ errwin;

   if (strcmp(vl->cdirname, vl->homedir) != 0)
        home = FALSE;
   else
        home = TRUE;

   output = 0;

   while (TRUE)
   {
      while (bufline[output] == ' ')  output++;    /* sauter les blancs */

      /* verifier si la config est conforme  a l'etat de l'analyse */
      c = bufline[output];

      /* tester si commentaire */
      if (c == '#')  return(DEB_RELATION);

      /* suivant etat du scanning verifier la syntaxe */
      if (analyse == DEB_RELATION) {

           if (c == '\0') {

                relativline = 0;
                return(DEB_RELATION);
           }
           else
               if (c == EOF)  {
                   *err = 0;
                   return(FIN_MAKE);
               }
      }
      else  {

          if (c == '\0') {
              arglc--;
              return(FIN_RELATION);
          }
          if (c == '+')
                return(FIN_LIGNE);

          if (c == EOF)
                return(EOC_MISSING);

          if ((c == ':') && (relativline != 0))  {
               sprintf(work, syntErr2, line);
               pushPop_MESS(work, ERRMSG);
               makeError = TRUE;
               *err = 1;
               return(FIN_MAKE);
          }
      }
      analyse = SCAN_LIGNE;
      i = output;    /* debut de token */

      /* se positionner sur le prochain separateur */
      while (( (c = bufline[output]) != ' ') && (c != '\0') && (c != '+') && (c != EOF))
            output++;

      bufline[output] = '\0';

      if (c == EOF) {  /* fin de fichier */

                strcpy(tokenptr,vl->cdirname);
                if (get_deepth(tokenptr) > 0)
                  strcat(tokenptr, "\\");
                strcat(tokenptr,&bufline[i]);
                argf[arglc] = tokenptr;
                tokenptr += strlen(&bufline[i])+strlen(vl->cdirname)+1;
                return(EOC_MISSING);
      }
      if (c == '\0') { /* fin de relation */

      strcpy(tokenptr,&bufline[i]);
                argf[arglc] = tokenptr;
                tokenptr += strlen(&bufline[i])+1;
                relativline = 0;
                return(FIN_RELATION);
      }
      if (c == '+') {  /* fin de ligne */

                strcpy(tokenptr,&bufline[i]);
                argf[arglc] = tokenptr;
                arglc++;
                tokenptr += strlen(&bufline[i])+1;
                relativline++;
                return(FIN_LIGNE);
      }
      strcpy(tokenptr,&bufline[i]);
      argf[arglc] = tokenptr;
      arglc++;
      tokenptr += strlen(&bufline[i])+1;
      output++;
   }
}

/*---------------------------------------------------
 * gettype - indique le type de commande a effectuer
 *---------------------------------------------------
 */
gettype(arg0)
char *arg0;
{
     char type;

     while (( type = *(arg0++)) != '\0')
        if ( type == '.') {

            type = *arg0;
            switch(type) {
            case 'c' :
            case 'C' :  return(CTYPE);    /* type C */
            case 'a' :
            case 'A' :  return(ATYPE);    /* type assembleur */
            case 'o' :
            case 'O' :  return(LTYPE);    /* type link */
            case 'h' :
            case 'H' :  return(HTYPE);    /* type header */
            case '$' :  return(MTYPE);    /* type macro  */
            default  :  return(ETYPE);    /* erreur      */
            }
        };
     return(ETYPE);
}


/*------------------------------------------------------------------
 * getMDATE - donne la date associee a la MACRO passee en parametre
 *------------------------------------------------------------------
 */
long getMDATE(macro)
char *macro;
{
   int i;
   struct OBJ errwin;
   for (i=0;i<MACROfree;i++)
      if (strncmp(MACROname[i],macro,strlen(argf[0])) == 0)
         return(MACROdate[i]);
   sprintf(work, undefMacStr, line, argf[0], macro);
   pushPop_MESS(work, ERRMSG);
   makeError = TRUE;
   return(-1);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * redirOn
 *ÄÄÄÄÄÄÄÄÄÄ
 */
redirOn()
{
        /* rediriger la sortie vers le fichier de REDIRECTION */
        filout = dup(1);
        fmsg = open(vl->fError,O_RDWR);
        xclose(1);
        dup(fmsg); /* redirection OK */
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * redirOff
 *ÄÄÄÄÄÄÄÄÄÄ
 */
redirOff()
{
        /* revenir */
        xclose(1);
        xclose(fmsg);
        dup(filout);   /* ON revient au stade initial */
        xclose(filout);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * print_FA
 *ÄÄÄÄÄÄÄÄÄÄ
 */
print_FA()
{
        char work[80];

        sprintf(work,"            %2d %s",Cfile, nCfileStr);
        print_at(wm, 1, OFF_CFILE, work, OLDATT);
        sprintf(work,"            %2d %s",Afile, nAfileStr);
        print_at(wm, 1, OFF_AFILE, work, OLDATT);
}

/*-----------------------------------------------------------------
 * execcom - lancer la commande correspondante au type de relation
 *-----------------------------------------------------------------
 */
execcom(relation, diff, modB)
int relation;
int *diff, *modB;
{
      char **combuf;
      char string[80], includeDir[80], outputDir[80], work2[80], *ptr, *pt;
      FILE  *Xtmp;
      int fredirec, noOverWriteOut, noOverWriteInc;
      int i,j,n,insert,filout;

      *modB = FALSE;
      if (relation == ETYPE)  return(ETYPE);

      inhibe_screen();  /* pour eviter les messages compilateur */
      switch(relation) {
        case CTYPE : combuf = spcc  ;insert = narg ;break;
        case ATYPE : combuf = spasm ;insert = 1    ;break;
        case LTYPE :
        case HTYPE :break;
      }

      /* gestion affichage des noms */
      ptr = argfA[2];

      /* effacer les caracteres en zone de verification */
/*      memset(work, ' ', 80);
      print_nb_at(wm, offset, OFF_CHECK, work, INF(WMAKESIZE-offset, strlen(work)), OLDATT);*/

      if (relation == CTYPE || relation == ATYPE) /* COMPILE ou ASSEMBLE */
      {
      if (relation == CTYPE) {
          /* copier l'argument de la commande a lancer dans STRING */
          noOverWriteOut = noOverWriteInc = 0;
          if (strlen(c_x)) {
                strcpy(outputDir, "-n");
                strcat(outputDir, c_x);
                noOverWriteOut++;
          }
          if (strlen(c_z)) {
                strcpy(includeDir, "-I");
                strcat(includeDir, c_z);
                noOverWriteInc++;
          }

          /* le nom absolu du fichier est dans argfA[2] */
          strcpy(string,argfA[2]);
          if (relation == ATYPE)
                string[strlen(argfA[2])] = ';';

          string[strlen(argfA[2])+1]                     = '\0';
          *(combuf+insert)                               = outputDir;
          *(combuf+insert+noOverWriteOut)                = includeDir;
          *(combuf+insert+noOverWriteOut+noOverWriteInc) = string;
      }
      else {
          strcpy(string,argfA[2]);
          string[strlen(argfA[2])] = ';';
          string[strlen(argfA[2])+1] = '\0';
          *(combuf+insert) = string;
      }

          /* ecrire la commande */
          memset(work, '\0', 80);
          if (relation == CTYPE) {
              Cfile++;
              j = 0;
          }
          else {
              Afile++;
              j = 1;
          }


          memset(work, ' ', 79);
          print_nb_at(wm, offset, OFF_COMP, work, INF(WMAKESIZE-offset, strlen(work)-1), OLDATT);

          memset(work, '\0', 80);
          sprintf(work,"%s\n", ptr = strupr(ptr));

          print_at(wm, 1, OFF_COMP, (j ? assString : compString), OLDATT);
          print_nb_at(wm, offset, OFF_COMP, work, INF(WMAKESIZE-offset, strlen(work)-1), OLDATT);
          print_FA();


          *diff = TRUE;
          i  = strlen(work);
          strcpy(vl->ferror_name, ptr);
          write(fmsg, work, i);

          /* lancer la commande en creant un process fils */
          if ((errorlevel = spawnvp(P_WAIT,*combuf,combuf))) {
               if (vl->operation == MKE)
                   makeError = TRUE;
               enable_screen();
               return(-1);  /* traiter messages erreurs Comp & Ass */
          }
          print_at(wm, 1, OFF_CHECK, checkString, OLDATT);
          memset(work, ' ', 79);
          print_nb_at(wm, offset, OFF_COMP, work, INF(WMAKESIZE-offset, strlen(work)-1), OLDATT);
      }
      else {

        if (relation == LTYPE) {  /* LINKING */

               if ((Xtmp = fopen("_x041256.lnk","w+")) == NULL) {
                        strcpy(work, tempStr);
                        affiche(work, ERRMSG);
                        enable_screen();
                        return(0);
               }
               else {
                        /* 1) ecrire le nom du startup et CMDTAB */
                        fprintf(Xtmp,lnkstr1, c_l, c_x); /*c_u);*/

                        /* 2) ecrire l' objet de l'utilisateur */
                        for (i = 2; i < arglc+1 ; i++) {
                                fwrite("+\n",1,2,Xtmp);
                                fwrite(argfA[i],1,strlen(argfA[i]),Xtmp);
                        }

                        /* 3) ecrire le nom de l'executable utilisateur */
                        strcpy(wkfile, argfA[0]);
                        strcpy(vl->conf.projExe, wkfile);

                        strcpy(work, wkfile); /* sauve nom de l'OBJ */
                        fwrite(",",1,1,Xtmp);
                        fwrite(wkfile,1,strlen(wkfile),Xtmp);

                        /* 4) ecrire les noms de librairies */
                        fprintf(Xtmp, lnkstr2, c_u, c_u, c_u, c_l, c_l, c_l);
                }
               fclose(Xtmp);
               fflush(Xtmp);
               ptr = strupr(argfA[0]);
               strcpy(vl->ferror_name, ptr);/**/
               memset(work, '\0', 80);
               sprintf(work,"%s\n", ptr);
               i = strlen(work);


               *diff = TRUE;
               strcpy(argfA[2], ptr);
               write(fmsg, work, i+1);


               memset(work, ' ', 79);
               print_nb_at(wm, offset, OFF_LINK, work, INF(WMAKESIZE-offset, strlen(work)-1), OLDATT);

               memset(work, '\0', 80);
               strcpy(work, ptr);
               print_at(wm, 1, OFF_LINK, linkString, NEWATT, (vl->WINSYSink|vl->WINSYSpaper));
               print_nb_at(wm, offset, OFF_LINK, work, INF(WMAKESIZE-offset, strlen(work)), OLDATT);

               /* lancer la commande en creant un process fils */
               errorlevel = spawnvp(P_WAIT,splnk[0],splnk);

               if (errorlevel) {
                   remove("_x041256.lnk");
                   if (vl->operation == MKE)
                       makeError = TRUE;
                   enable_screen();
                   return(-1);
               }
               else {
                    *modB = TRUE;
                    remove("_x041256.lnk");
                    enable_screen();
                    return(0);
               }
        }
      }
      enable_screen();
      return(0);
}


/*----------------------------------------------------
 * affiche -
 *----------------------------------------------------
 */
affiche(str, msgtyp)
char *str;
{
    struct OBJ errwin;

    pushPop_MESS(str, msgtyp);
    winPopUp(wm, W_CLOSE);  /* supprimer cadre compilation */
}


/*----------------------------------------------------
 * inhibe_screen -
 *----------------------------------------------------
 */
inhibe_screen()
{
   void interrupt Dummy_interrupt();

   _dos_setvect(0x10, Dummy_interrupt);
   /* se placer dans la directory courante
    * ( utilis‚ pour creer les objets dans la mˆme directory
    * que les sources)
    */
   /* chdir(vl->cdirname);*/
}

/*----------------------------------------------------
 * enable_screen -
 *----------------------------------------------------
 */
enable_screen()
{
   _dos_setvect(0x10, BiosInt10);
/*   chdir(vl->homedir);*/
}


/*----------------------------------------------------
 * dummy_interrupt
 *----------------------------------------------------
 */
void interrupt Dummy_interrupt()
{
}


/**********************
 * IMPORTATIONS
 **********************/

/*---------------------------------------------------------------------------
 * verify_window - gestion apparition des messages systemes
 *---------------------------------------------------------------------------
 */
verify_window(wp, str, titre, type)
struct OBJ *wp;
char *str, *titre;         /* message a ecrire et titre */
{
    int size;
    unsigned inkAtt, paperAtt, borderAtt, messAtt;

        if (type == ERRMSG) {
                inkAtt    = vl->WINmessInk;
                paperAtt  = vl->WINmessPaper;
                borderAtt = vl->WINmessBorder;

        }
        else {
                inkAtt    = vl->WINSYSink;
                paperAtt  = vl->WINSYSpaper;
                borderAtt = vl->WINSYSborder;
        }

        messAtt   = inkAtt|paperAtt;
        if ((size = strlen(str)) > 78)
                size = 76;
        initWin(wp, titre, (80 - size)/2, 10, size, 3, TRUE, FALSE, inkAtt, paperAtt, borderAtt, vl->WINSYSblock, W_WIN);
        winPopUp(wp, W_OPEN, type);
        messwin = wp;
        Xmess = 0;
        Ymess = 1;
        print_at(messwin, Xmess, Ymess, str, NEWATT, messAtt);

}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * winPopUp - ouvrir / fermer une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
winPopUp(wp, mode, borderType)
struct OBJ *wp;
{
    if (mode == W_OPEN) {
        /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
        if (wp->save == NULL) /* check presence tampon */
            if ((wp->save = malloc(2 * (wp->ncol + 4) * (wp->nline + 3))) == NULL) {
              pushPop_MESS(errmalloc, ERRMSG);
              return(FUNC|F6);
         }
        wp->pushed = TRUE;
        saveScreen(wp, WINDOW);
        write_wbox(wp, borderType);
        clrwin(wp, BLANK);
    }
    else {
        wp->pushed = FALSE;
        if (wp->save != NULL) {
            restoreScreen(wp, WINDOW);
            free(wp->save);
            wp->save = NULL;
        }
    }
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * clrwin Ä clear window
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION clrwin(wp, Ch)
struct OBJ *wp;
unsigned char Ch;
{
       int i;
       _clrwin(   wp->ul_x+1,
                  wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper,
                  Ch);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_ombrage - dessiner l'ombrage
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
write_ombrage(x,y,l,h)
{
   int offy, i;
   unsigned char hide_att;

   /* ombrage horizontal */
   _brighten(x + 1, y + h + 2, l + 2, (adapter? 0x08: 0x07));

   /* ombrage vertical */
   for (offy = 0; offy <= h + 1; offy++)
        _brighten(x + 2 + l,
                  y + 1 + offy,
                  2,
                  (adapter? 0x08 : 0x07));

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_mbox - dessiner le contour d'un menu
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_mbox(mp)
struct OBJ *mp;
{
  /* type 'Ä' */
  write_box(mp->title, mp->ul_x, mp->ul_y, mp->ncol, mp->nline, mp->border, 0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_box - dessiner le contour d'une BOXE
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_box(title,x,y,l,h,att,typ)
char *title;
unsigned char att;
{
   int ps,i,j,tl;
   unsigned char ulc, llc, urc, lrc, Vchar, Hchar;
   char Ch;

   ulc   = boxtype[typ][0];
   urc   = boxtype[typ][1];
   llc   = boxtype[typ][2];
   lrc   = boxtype[typ][3];
   Vchar = boxtype[typ][4];
   Hchar = boxtype[typ][5];


   /* COINS */
   _wstringDB(x        , y        , 1, NEWATT, att, &ulc);
   _wstringDB(x + l + 1, y        , 1, NEWATT, att, &urc);
   _wstringDB(x        , y + h + 1, 1, NEWATT, att, &llc);
   _wstringDB(x + l + 1, y + h + 1, 1, NEWATT, att, &lrc);


   /* HORIZONTALES */
   for (i = x+1; i <= x+l; i++) {
        _wstringDB(i, y        , 1, NEWATT, att, &Hchar);
        _wstringDB(i, y + h + 1, 1, NEWATT, att, &Hchar);
   }

   /* DETERMINER SI LE TITRE PEUT RENTRER DANS LA LARGEUR */
   tl = strlen(title);
   if ((tl > 0) && (l - 4 >= tl)) {
        i = x + (l - tl)/2;
        Ch = BLANK;
        _wstringDB(i++   , y, 1 , NEWATT, att, &Ch);
        _wstringDB(i     , y, tl, NEWATT, att, title);
        _wstringDB(i + tl, y, 1 , NEWATT, att, &Ch);
   }


   /* VERTICALES */
   for (i = y+1; i <= y+h; i++ ) {
        _wstringDB(x        , i, 1, NEWATT, att, &Vchar);
        _wstringDB(x + l + 1, i, 1, NEWATT, att, &Vchar);
   }
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_wbox - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_wbox(wp, type)
struct OBJ *wp;
{
      write_box(wp->title, wp->ul_x, wp->ul_y, wp->ncol, wp->nline, wp->border, type);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreScreen - RESTITUER la partie de l'ecran ecras‚ par le SOUS MENU
 *                 ou la WINDOW
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
restoreScreen(mp)
struct OBJ *mp;
{
    if (mp->objet == MENU)
        restoreFromMenu(mp,mp->save);
    else
        _refresh( mp->ul_x,
                  mp->ul_y,
                  mp->ul_x + mp->ncol + 3,
                  mp->ul_y + mp->nline + 2,
                  mp->save);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreFromMenu- ecriture PHYSIQUE d'un tampon sur l'ecran
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
restoreFromMenu(mp,buf)
struct OBJ *mp;
int            *buf;
{
   if (mp->mov == VER)
       _refresh(  mp->ul_x,
                  mp->ul_y,
                  mp->ul_x + mp->ncol+3,
                  mp->ul_y + mp->nline+2,
                  buf);
   else
       _wstringDW(mp->ul_x,
                  mp->ul_y,
                  mp->ncol,
                  buf);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * saveScreen - SAUVEGARDER la partie de l'ecran correspondant au SOUS MENU
 *              ou a la WINDOW
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
saveScreen(mp)
struct OBJ *mp;
{
    if (mp->objet == MENU) {
        if (mp->mov == VER)
            _save_screen(mp->ul_x,
                     mp->ul_y,
                     mp->ul_x + mp->ncol+3,
                     mp->ul_y + mp->nline+2,
                     mp->save);
        else
            _rstringDW(  mp->ul_x,
                     mp->ul_y,
                     mp->ncol,
                     mp->save);

    }
    else
        _save_screen( mp->ul_x,
                      mp->ul_y,
                      mp->ul_x + mp->ncol + 3,
                      mp->ul_y + mp->nline + 2,
                      mp->save);
   if (mp->f_ombrage)
       write_ombrage(mp->ul_x, mp->ul_y, mp->ncol, mp->nline);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * initWin : initialiser une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION initWin(wp,title,ul_x,ul_y,ncols,nlines,ombrage,bar,ink,paper,border,batt)
struct OBJ *wp;
char *title;
{
   wp->objet      = WINDOW;
   wp->nline      = nlines;
   wp->leftCh     = 0;
   wp->curX       = wp->curY = 0;
   wp->border    = border;
   wp->ink        = ink;
   wp->paper      = paper;
   wp->ul_x       = ul_x;
   wp->ul_y       = ul_y;
   wp->ncol       = ncols;
   wp->pushed    = FALSE;
   wp->title      = title;
   wp->save       = NULL;
   wp->f_ombrage  = ombrage;
   wp->f_bar      = bar;

/*   fillCh         = 0;*/

}

/*ÄÄÄÄÄÄÄÄ
 * pushPop_MESS
 *ÄÄÄÄÄÄÄÄ
 */
pushPop_MESS(str, msgtyp)
char *str;
{
   struct OBJ errwin;
   char aux[80];
   strcpy(aux, str);
   verify_window(&errwin, aux, nullstr, msgtyp);
   getkey(); /*_read_kbd();*/
   winPopUp(messwin, W_CLOSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * get_deepth - retourne la profondeur du pathName (si 0 <= : dir courante)
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
get_deepth(path)
char *path;
{
   int deepth, i, j;
   if ((i = strlen(path)) == 3)
        return(0);
   for (j = 3, deepth = 1; j < i; j++)
          if (path[j] == '\\')
              deepth++;
   return(deepth);
}

/*----------------------------------------------------
 * putMACRO - place une MACRO dans la table des macros
 *----------------------------------------------------
 */
putMACRO()
{
      char *glta();
      int checkfile,i;
      struct OBJ errwin;
      char buf[80];

        checkfile = 2;
        strcpy(buf, conf.u.pd.c_m);
        for (i=0;i<MACROfree;i++)
            if (strncmp(MACROname[i],argf[0],strlen(argf[0])) == 0)
            {   /* la macro existe deja */
               sprintf(work, redefMacStr, line, argf[0]);
               verify_window(&errwin, work, nullstr, ERRMSG);
               getkey(); /*_read_kbd();*/
               winPopUp(messwin, W_CLOSE);
               makeError = TRUE;
               return(-1);
            };

        /* la macro n'existe pas : la creer */
        strcpy(MACROname[MACROfree],argf[0]);
        if (++MACROfree > MAXMACRO)  {

                sprintf(work, tooMaMacStr, line);
                verify_window(&errwin, work, nullstr, ERRMSG);
                getkey(); /*_read_kbd();*/
                winPopUp(messwin, W_CLOSE);
                makeError = TRUE;
                return(-1);
        }

        /* donner a la MACRO la date du fichier le plus recent */
        filarg0 = 0;
        while (checkfile < arglc+1) {
            if (stat(argfA[checkfile],&entry) == 0)
            {
                 if (entry.st_atime > filarg0)
                     filarg0 = entry.st_atime;
                 checkfile++;
            }
            else
            {
                sprintf(work, cannotCrMac, line, argf[checkfile]);
                verify_window(&errwin, work, nullstr, ERRMSG);
                getkey();
                winPopUp(messwin, W_CLOSE);
                makeError = TRUE;
                return(-1);
            }
        }
        MACROdate[MACROfree - 1] = filarg0;
        return(0);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * glta - get last token address : retire le dernier token de l'arg
 *        et retourne son adresse
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
char *glta(path)
char *path;
{
   int i, j;
   i = j = strlen(path);
   while (i >= 2) {
          if (path[i] == '\\') break;
          i--;
   }
   strcpy(path + j + 1, path + i + 1);
   if (i > 2) path[i]   = '\0';
   else       path[i+1] = '\0';
   return(path + j + 1);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at - ecrire la string str aux coordonnees x, y, dans
 *            la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
print_at(wp, x, y, str, isnewatt, att)
struct OBJ *wp;
char *str;
{
  int i;
  if ((i = strlen(str)) > 0)
       _wstringDB(wp->ul_x + 1 + x,
                  wp->ul_y + 1 + y, i, isnewatt, att, str);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at_nb - ecrire nb carac aux coordonnees x, y, dans
 *               la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
print_nb_at(wp, x, y, str, nb, isnewatt, att)
struct OBJ *wp;
char *str;
{
   if (nb > 0) {
       if (isnewatt)
           _wstringDB(wp->ul_x + 1 + x,
                      wp->ul_y + 1 + y, nb, NEWATT, att, str);
       else
           _wstringDB(wp->ul_x + 1 + x,
                      wp->ul_y + 1 + y, nb, NEWATT, wp->ink|wp->paper, str);
   }
}


/*--------------------------------------------------------------
 * m_installed - teste si le driver souris est present. si oui,
 *               retourne le nombre de boutons
 *--------------------------------------------------------------
 */
m_installed(nbutton)
unsigned *nbutton;
{
 union  REGS  rin,rout;

 rin.x.ax  = 0;
 int86(0x33, &rin, &rout);
 *nbutton = rin.x.bx;
 return(rout.x.ax);
}

/*--------------------------------------------------------------
 * m_getPosAndButtonStatus - retourne la position et le status:
 *                        le bit 0 represente celui de droite
 *                        le bit 1 represente celui de gauche
 *                        Pour Chacun des 2 bits :
 *                        si … 1 -> enfonc‚,   0 -> relach‚
 *--------------------------------------------------------------
 */
m_getPosAndButtonStatus(posX, posY, buttonStatus)
int *posX, *posY;
unsigned   *buttonStatus;
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x03;
        int86(0x33, &rin, &rout);
        *posX = rout.x.cx;
        *posY = rout.x.dx;
        *buttonStatus = rout.x.bx;
}

/*--------------------------------------------------------------
 * m_setCursorPos -
 *
 *--------------------------------------------------------------
 */
m_setPos(newPosX, newPosY)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x04;
        rin.x.cx  = newPosX;
        rin.x.dx  = newPosY;
        int86(0x33, &rin, &rout);
}

/*--------------------------------------------------------------
 * m_callMaskAndAddress - chaine … l'IT souris, le traitement
 *                       utilisateur a appeler si les conditions
 *                       exprim‚es dans le masque sont reunies:
 *                              - bit 0 : changement de position
 *                              - bit 1 : bouton Gauche press‚
 *                              - bit 2 : bouton Gauche relach‚
 *                              - bit 3 : bouton Droit press‚
 *                              - bit 4 : bouton Droit relach‚
 *--------------------------------------------------------------
 */
m_callMaskAndAddress(callMask, subroutine)
int callMask;
int (far * subroutine)();
{
 union  REGS  rin,rout;
 struct SREGS segreg;

        rin.x.ax  = 0x0c;
        rin.x.cx  = callMask;
        rin.x.dx  = FP_OFF(subroutine) ;
        segreg.es = FP_SEG(subroutine);
        int86x(0x33, &rin, &rout,&segreg);
}

it_mouse()
{
    mouseEvent++;
}

/*--------------------------------------------------------------------
 * getkey - lire un caractere et son attribut sur une page inactive
 *          retourne (TYPE | caractere)
 *------------------------------------------------------------------
 */
getkey()
{
   unsigned key, main, auxiliary;

   m_callMaskAndAddress(M_LEFT_PRESS, prolog_it_mouse);

   for (;;) {
        /* examiner le status du clavier */
        if (_look_kbd()) { /* caractere tap‚ au clavier */
             _read_kbd();
             break;
         }

         /* tester si on a utilis‚ la souris */
         if (mouseEvent) {
               mouseEvent = 0;
               break;
         }
   }
   m_callMaskAndAddress(0, 0L);

}

inhibe_mouse()
{
}


/* compatibilite avec la procedure d'installation */
new_int10h()
{
}

DupEnviron()
{
    strcpy(c_cl, vl->conf.u.pd.c_cl);    /* comp & linker dir   */
    if (strlen(c_cl))
        strcat(c_cl, "\\");
    strcpy(c_a, vl->conf.u.pd.c_a);      /* assembleur dir      */
    if (strlen(c_a))
        strcat(c_a, "\\");
    strcpy(c_i, vl->conf.u.pd.c_i);      /* include dir         */
    if (strlen(c_i))
        strcat(c_i, "\\");
    strcpy(c_z, vl->conf.u.pd.c_I);      /* MKD include dir     */
    if (strlen(c_z))
        strcat(c_z, "\\");
    strcpy(c_l,  vl->conf.u.pd.c_l);     /* librairies dir      */
    if (strlen(c_l))
        strcat(c_l, "\\");
    strcpy(c_u,  vl->conf.u.pd.c_u);     /* ulk dir             */
    if (strlen(c_u))
        strcat(c_u, "\\");
    strcpy(c_m,  vl->conf.u.pd.c_m);     /* make file name      */
    if (strlen(c_m))
        strcat(c_m, "\\");
    strcpy(c_p,  vl->conf.u.pd.c_p);     /* paste file name     */
    if (strlen(c_p))
        strcat(c_p, "\\");
    strcpy(c_h,  vl->conf.u.pd.c_h);     /* help dir name       */
    if (strlen(c_h))
        strcat(c_h, "\\");
    strcpy(c_x,  vl->conf.u.pd.c_x);     /* user dir name       */
    if (strlen(c_x))
        strcat(c_x, "\\");

    nb_car_per_line = vl->conf.nb_car_per_line;
}
refExit(wm, code, ret)
struct OBJ *wm;
{
                     winPopUp(wm, W_CLOSE);
                     vl->retCode = code;
                     inhibe_mouse();
                     showMouse();
                     free(buftoken);
                     free(bufline);
                     exit(ret);
}

xclose(fd)
{
        close(fd);
        flush_buffer(fd);
}

initBuffer()
{
    if ((buftoken = malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
    if ((bufline = malloc(nb_car_per_line)) == NULL) {
        pushPop_MESS(" Not enough memory ", ERRMSG);
        exit(0);
    }
}