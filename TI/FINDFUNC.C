/* FINDFUNC.C : gestion des declarations et des references */
#include "xed.h"
#include "ext_var.h"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

/* RECORD dans le fichier des modules */
struct module {
        char modname[64];
};

/* RECORD dans le fichier des DEF */
struct def {
         char     cflag;       /* type de fonct: "C" ou "A"ssembleur */
         int      position;    /* numero de ligne ou est definie la func */
         char     function[32];/* nom de la fonction */
unsigned char     type;        /* EXT ou DEF */
         int      module;      /* numero du module dans le module file */
         int      cpt;         /* # de REF externes dans la zone externe */
         int     *offRef;      /* offset dans le fichier des REF pour les
                                * modules qui utilisent cette fonction
                                */
};

/* handles des fichiers de la base */
int Hext, Hmod, Hdec;

char dBaseSaisie[80];

/*ÄÄÄÄÄÄÄÄÄÄ
 * dummyFunc
 *ÄÄÄÄÄÄÄÄÄÄ
 */
dummyFunc()
{
        return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doBuildDB
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doBuildDB(mp,num)
struct OBJ *mp;
{
        int dummyFunc();
        vl->Bfunc = dummyFunc;
        doBrowse(mp);
}
/*ÄÄÄÄÄÄÄÄÄÄ
 * doGotoDef
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doGotoDef(mp,num)
struct OBJ *mp;
{

   char   work[80], aux[80], sauve[80], *p;
   int pos, cflag, status;
   unsigned ret;

   if (conf.buildDB) {
        pushPop_MESS(rebuildStr, NMXMSG);
        vl->Bfunc = doGotoDef;
        vl->conf.buildDB = FALSE;
        doBrowse(mp);
   }

   memset(work, 0, 80);
   ret = doEnter(gotoFuncNameStr, dBaseSaisie, FALSE, FALSE);
   strcpy(work, dBaseSaisie);
   strupr(work);
   hide_cursor();
   if (ret)
        return(ret);

   return(_doGotoDef(mp, num, work));
}

_doGotoDef(mp, num, str)
struct OBJ *mp;
char *str;
{
   char   aux[80], sauve[80], *p;
   int pos, cflag, status;
   unsigned ret;

   strcpy(file_name, absfname);
   p = strpbrk(file_name, ".");

   /* tout fichier != d'un fichier Ass est considere comme de type C */
   cflag = TRUE;
   if (*(p+1) == 'A')
        cflag = FALSE;

   /* si fichier C, ajouter underscore au nom */
   if (cflag) {
        strcpy(sauve, str);
        strcpy(aux, "_");
        strcat(aux, str);
        strcpy(str, aux);
   }

   /* str contient le nom de la fonction */
   if ((status = findFuncFromDB(str, aux, &pos)) != 0) {
        if (status == -2) { /* PB DATABASE */
                pushPop_MESS(buildDBstr, NMXMSG);
                return(FUNC|F6);
        }
        if (cflag)
                status = findFuncFromDB(sauve, aux, &pos);
   }

   if (status == -1) { /* Fonction Non trouvee */
       pushPop_MESS(notUserStr, ERRMSG);
       return(FUNC|F6);
   }

   /* si on est en mode HELP, en sortir */
   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   /* construire le NOM ABSOLU du module */
   strcpy(file_name, aux);

   if (loadFile(0, file_name, absfname, NOMSG, V_CREAT) < 0)
                return(FUNC|F6);

   doGotoLine(pedwin, pos);
   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * doDispRef
 *ÄÄÄÄÄÄÄÄÄÄ
 */
doDispRef(mp,num)
struct OBJ *mp;
{
   char   work[80], aux[80], sauve[80], *p, *glta();
   int pos, cflag, Otype, status;
   unsigned ret;

   if (conf.buildDB) {
        pushPop_MESS(rebuildStr, NMXMSG);
        vl->Bfunc = doDispRef;
        vl->conf.buildDB = FALSE;
        doBrowse(mp);
   }

   memset(work, 0, 79);
   ret = doEnter(dispFuncNameStr, dBaseSaisie, FALSE, FALSE);
   strcpy(work, dBaseSaisie);
   strupr(work);
   hide_cursor();
   if (ret)
        return(ret);

   return(_doDispRef(mp, num, work));
}

_doDispRef(mp, num, str)
struct OBJ *mp;
char *str;
{
   char   aux[80], sauve[80], *p, *glta();
   int pos, cflag, Otype, status;
   unsigned ret;

   strcpy(file_name, absfname);
   p = strpbrk(file_name, ".");

   /* tout fichier != d'un fichier Ass est considere comme de type C */
   cflag = TRUE;
   if (*(p+1) == 'A')
        cflag = FALSE;

   /* copier le nom de la fonction dans le tampon de recherche */
   if (str[0] == '_')
        strcpy(findSTR, &str[1]);
   else
        strcpy(findSTR, str);


   /* si fichier C, ajouter underscore au nom */
   if (cflag) {
        strcpy(sauve, str);
        strcpy(aux, "_");
        strcat(aux, str);
        strcpy(str, aux);
   }

   /* STR contient le nom de la fonction : placer tous les
    * modules referants cette fonction dans la liste DIR_LST
    */

   if ((status = findExtFromDB(str, aux)) != 0) {
        if (status == -2) { /* PB DATABASE */
                pushPop_MESS(buildDBstr, NMXMSG);
                return(FUNC|F6);
        }
        if (cflag)
                status = findExtFromDB(sauve, aux);
   }
   if (status == -1) { /* Fonction Non trouvee */
       pushPop_MESS(notUserStr, ERRMSG);
       return(FUNC|F6);
   }


   strcpy(str, file_name);
   strcpy(sauve, cdirname);
   strcpy(cdirname, aux);
   strcpy(mask,"");

   /* lancer l'affichage des fichiers */
   if ((ret = display_dir(&Otype, LOOK_FILE)) != 0) {
        strcpy(mask, "*.C");
        strcpy(file_name, str);
        strcpy(cdirname, sauve);
        return(ret);
   }
   strcpy(cdirname, sauve);
   strcpy(mask, "*.C");

   /* si on est en mode HELP, en sortir */
   if (fflag & FF_HELP_MOD) /* mode help edit */
          edHelp(HCLOSE);

   if (loadFile(0, file_name, absfname, NOMSG, V_CREAT) < 0)
        strcpy(cdirname, sauve);

   return(FUNC|F6);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * findFuncFromDB
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
findFuncFromDB(fname, aux, pos)
char *fname, *aux;
unsigned  *pos;
{
        struct def def;
        int n, status;


        /* ouvrir la base */
        if (openDB())
                return(-2);

        /* initialiser status a -2 */
        status = -2;

        /* comparer les enregistrements */
        while ((n = read(Hdec, &def, sizeof(struct def))) > 0) {

                if ((status = strcmp(fname, def.function)) > 0)
                        continue;
                else  {
                        if (status == 0) {
                                /* trouv‚ : recuperer le nom
                                 * du module et la position
                                 */
                                lseek(Hmod, (long)(def.module * sizeof(struct module)), SEEK_SET);
                                *pos = def.position;
                                read(Hmod, aux, sizeof(struct module));
                                status = 0;
                        }
                        else
                                status = -1;
                        break;
                }
        }
        if (status || n <= 0)
                status = -1;

        closeDB();
        return(status);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * findExtFromDB
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
findExtFromDB(fname, path)
char *fname, *path;
{
        struct dir_lst *dl;
        struct file_lst *fl;
        struct def def;
        int i, n, status;
        char *p;
        char aux[80], *glta();

        /* ouvrir la base */
        if (openDB())
                return(-2);

        /* initialiser status a -2 */
        status = -2;

        /* comparer les enregistrements */
        while ((n = read(Hdec, &def, sizeof(struct def))) > 0) {

                if ((status = strcmp(fname, def.function)) > 0)
                        continue;
                else  {
                        if (status == 0) { /* trouv‚ */
                                /* recuperer tous les numeros de
                                 * module ou cette fonction est appelee
                                 */
                                if (((int *)p = malloc(def.cpt+1)) == (int *)0) {
                                      pushPop_MESS(outofmem, ERRMSG);
                                      return(-1);
                                }
                                *p = def.module;
                                lseek(Hext, (long)def.offRef, SEEK_SET);

                                /* lire les numeros de modules dans depuis EXTRAL.DB */
                                read(Hext, p+1, def.cpt);

                                /* preparer les variables DIR_LST
                                 * afin de visualiser les fichiers
                                 */
                                dl = &dir_lst;
                                dl->next = dl->prev = NULL;
                                dir_lst_nfiles = 0;
                                dir_lst_npages = 1;

                                /* init head et tail */
                                memset(head.file.str, '\0', 13);
                                memset(tail.file.str, 'z', 12);
                                tail.file.str[12] = '\0';
                                head.next = &tail;
                                tail.prev = &head;
                                head.prev = tail.next = NULL;

                                /* lire les noms de modules */
                                for (i=0; i<=def.cpt; i++) {
                                        memset(path, 0, sizeof(struct module));
                                        memset(aux, 0, sizeof(struct module));

                                        lseek(Hmod, (long)((*p++) * sizeof(struct module)), SEEK_SET);
                                        read(Hmod, path, sizeof(struct module));

                                        /* recuperer le PATH */
                                        strcpy(aux, glta(path));

                                        /* inserer dans la liste */
                                        if ((fl = malloc(FILE_SIZE)) == NULL) {
                                                pushPop_MESS(outofmem, ERRMSG);
                                                return(-1);
                                        }
                                        memset((char *)fl,'\0',FILE_SIZE);
                                        strcpy(fl->file.str, aux);
                                        fl->file.att = 0x20; /* ARchive */
                                        tri_lst(fl);  /* insrerer en triant */
                                        dir_lst_nfiles++;

                                }
                                /* placer les fonctions dans la DIR_LST */
                                put_ext_lst(dl);


                                status = 0;
                        }
                        else
                                status = -1;
                        break;
                }
        }
        if (status || n <= 0)
                status = -1;

        closeDB();
        return(status);
}


/*ÄÄÄÄÄÄÄÄÄÄ
 * openDB
 *ÄÄÄÄÄÄÄÄÄÄ
 */
openDB()
{
        if ((Hmod  = openDBfile("module.db")) < 0) {
                beep(1000);
                pushPop_MESS(dbaseOpenErr, ERRMSG);
                return(-1);
        }
        if ((Hdec  = openDBfile("declar.db")) < 0) {
                beep(1000);
                pushPop_MESS(dbaseOpenErr, ERRMSG);
                return(-1);
        }
        if ((Hext  = openDBfile("external.db")) < 0) {
                beep(1000);
                pushPop_MESS(dbaseOpenErr, ERRMSG);
                return(-1);
        }
        return(0);
}

/*ÄÄÄÄÄÄÄÄÄÄ
 * openDBfile
 *ÄÄÄÄÄÄÄÄÄÄ
 */
openDBfile(fname)
char *fname;
{
        strcpy(work, conf.u.pd.c_x);  /* user dir */
        if (strlen(work))
                strcat(work, "\\");
        strcat(work, fname);
        return(open(work, O_RDWR|O_BINARY));
}

closeDB()
{
        close(Hdec);
        close(Hext);
        close(Hmod);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * put_ext_lst : recuperer les references
 *              externes
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
put_ext_lst(dl)
struct dir_lst *dl;
{
 int    i, ret;
 struct file_lst *fl;
 char  *dir = work;

      /* rincer le 1er element de la liste */
      memset((char *)dl,'\0',DIR_LST);

      /* recopier les fichiers dans les pages */
      for ( i = 0, fl = head.next; fl != &tail; fl = fl->next) {
            if (i >= NB_FILE_PER_PAGE) {
                /* allouer une nouvelle page */
                if ((dl->next = malloc(DIR_LST)) == NULL) {
                        pushPop_MESS(outofmem, ERRMSG);
                        return(-1);
                }
                dir_lst_npages++;
                memset((char *)dl->next,'\0',DIR_LST);
                dl->nfpp       = NB_FILE_PER_PAGE;
                dl->next->prev = dl;
                dl             = dl->next;
                dl->next       = NULL;
                i              = 0;
            }
            strcpy(dl->dirPage[i].str, fl->file.str);
            dl->dirPage[i].att = fl->file.att;
            i++;
      }
      if ((dir_lst_nfiles / NB_FILE_PER_PAGE) > 0) {
          if ((dl->nfpp = dir_lst_nfiles % NB_FILE_PER_PAGE) == 0)
               dl->nfpp = NB_FILE_PER_PAGE;
      }
      else
          dl->nfpp = dir_lst_nfiles;

      return(dir_lst_nfiles);
}
