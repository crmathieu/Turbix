/* gestion des appels de fonctions */

#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include <ctype.h>
#include <process.h>
#include <sys\stat.h>

char *ARGV[5];
char Rbuf[256];
char writeBuf[80];

struct funcdef {
	int type;    /* DEF ou EXTRN */
	int ligne;
	char funcName[64];
};

struct funcdef functab[100];
#define TYPE_NUL 0    	/* rien */
#define TYPE_DEF 1    	/* Definition */
#define TYPE_EXT 2	/* Externe */
#define TYPE_DEC 3      /* Declaration */
char *typeTab[] = {"","DEF","EXT","DECL"};

char *CreservedWord[] = {
	"if", "return","while","for","int","long","sizeof","FP_OFF","FP_SEG",
	"switch"};

#define CRESERVW 10
#define TRUE  1
#define FALSE 0

#define CFLAG  1
#define AFLAG  0

int Lbrack  = 0;   /* # de { */
int Rbrack  = 0;   /* # de } */
int noligne = 0;   /* # de ligne ou se situe l'appel */

int fdW;
char work[80];

main(argc, argv)
int argc;
char *argv[];
{
	int i;
	char *pName;
	char nfname[64];
	i = 0;
	memset(functab, 0, sizeof(struct funcdef)*100);

	if (argc < 2) {
		printf("Use : refcall <file name>\n");
		exit(0);
	}

	/* copier arguments */
        while (argv[i] && i < 5) {
               ARGV[i] = argv[i];
               i++;
        }

	/* determiner le type de fichier */
	strcpy(nfname, argv[1]);
	strupr(nfname);
	if ((pName = strpbrk(nfname, ".")) == NULL) {
		printf("%s : Nom de fichier incorrect\n", nfname);
		exit(-1);
	}
	if (*(pName+1) == 'C') {
		get_function(argv[1], CFLAG);
		updateDECL();
		writeResult(TYPE_DEC);
	}
	else
		get_function(argv[1], AFLAG);


	writeResult(TYPE_DEF);
	writeResult(TYPE_EXT);

}

get_function(fname, Cflag)
char *fname;
{
    int  load, wSize, ret;
    char *pName, nfname[64];

    FILE *fp;

    if ((fp = fopen(fname, "r")) == NULL) {
	printf("\nouverture fichier %s impossible\n", fname);
	exit(-1);
    }

    memset(nfname, '\0', 64);
    strcpy(nfname, fname);

    /* creer le fichier final */
    if ((pName = strpbrk(nfname, ".")) == NULL) {
	printf("%s : Nom de fichier incorrect\n", fname);
	exit(-1);
    }
    *(pName+1) = '\0';
    strcat(pName, "REF");

    if ((fdW = open(nfname,O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) == -1) {
	 printf("impossible de cr‚er fichier de travail\n");
	 exit(-1);
    }


    ret = TRUE;
    while (ret) {
	ret = getline(fp);	/* lire une ligne */
	if (Cflag)
		C_automate();
	else
		A_automate();
    }
    fclose(fp);
}

/*---------------------------------------------------
 * C_automate
 *---------------------------------------------------
 */
C_automate()
{
#define WAIT_LPARENTH 	0
#define WAIT_STATE	1
#define WAIT_STAR1	2
#define WAIT_STAR2	3
#define WAIT_SLASH2	4

	int i, j, k;
	static int etat = 0;

	i = 0;
	j = strlen(Rbuf);

	while (i < j) {
	switch(etat) {

	case WAIT_LPARENTH: if (Rbuf[i] == '(') {
				i--;
				etat = WAIT_STATE;
			    }
			    else {
				if (Rbuf[i] == '{') Lbrack++;
				if (Rbuf[i] == '}') Rbrack++;
				if (Rbuf[i] == '/') etat = WAIT_STAR1;
				i++;
			    }
			    break;

	case WAIT_STATE   : k = i;

			    /* remonter sur la fin du nom de fonction */
			    while (i > 0 && Rbuf[i] == ' ') i--;
			    if (isalpha(Rbuf[i]))
				writefuncName(i);
			    i = k+2;
			    etat = WAIT_LPARENTH;
			    break;
	case WAIT_STAR1   : if (Rbuf[i] == '*') {
				etat = WAIT_STAR2;
				i++;
			    }
			    else
				etat = WAIT_LPARENTH;
			    break;
	case WAIT_STAR2   : if (Rbuf[i] == '*')
				etat = WAIT_SLASH2;
			    i++;
			    break;
	case WAIT_SLASH2  : if (Rbuf[i] == '/') {
				etat = WAIT_LPARENTH;
				i++;
			    }
			    else
				etat = WAIT_STAR2;
			    break;
	default		  : printf("???\n");
	}
	}
}

/*---------------------------------------------------
 * A_automate
 *---------------------------------------------------
 */
A_automate()
{
	int i, j, k;
	char buf[64];
	i = 0;
	j = strlen(Rbuf);

	/* effacer les blancs de d‚but de ligne */
	while ((Rbuf[i] == ' ') && (i < j)) i++;
	if (i == j)
		return;
	j = i;

	/* aller jusqu'au prochain s‚parateur */
	while (!isspace(Rbuf[i])) i++;
	k = i;
	Rbuf[i] = '\0';  /* isoler le mot cl‚ */

	strcpy(buf, "PUBLIC");
	if (strcmp(&Rbuf[j], buf) == 0) {
		getPublic(k+1);
	}
	else {
		strcpy(buf, "EXTRN");
		if (strcmp(&Rbuf[j], buf) == 0) {
			getExtrn(k+1);
		}
	}
}
/*---------------------------------------------------
 * getPublic
 *---------------------------------------------------
 */
getPublic(k)
{
	getAssFunc(k, TYPE_DEF);
}
/*---------------------------------------------------
 * getExtrn
 *---------------------------------------------------
 */
getExtrn(k)
{
	getAssFunc(k, TYPE_EXT);
}
/*---------------------------------------------------
 * getAssFunc
 *---------------------------------------------------
 */
getAssFunc(i, flag)
{
   char buf[64];
   int j, k;

   /* aller en d‚but de nom */
   while (isspace(Rbuf[i])) i++;
   j = i;

   /* aller en fin de nom */
   while (Rbuf[i] != ' ') {
	if ((Rbuf[i] == ';') || (Rbuf[i] == ':'))
		break;
	i++;
   }
   k = i;

   /* recopie du nom */
   memset(buf, '\0', 64);
   for (i = 0; i<k-j; i++)
		buf[i] = Rbuf[j+i];

   /* tester le type */
   if (flag == TYPE_EXT) {
	while (Rbuf[k] != ':') k++;
	if (getExtrnTyp(k+1) == FALSE)
		return;
   }

   /* enregistrer le nom */
   recordName(buf, AFLAG, flag);

   printf("%4d - %s  :  %s\n", noligne, typeTab[flag], buf);
}

/*---------------------------------------------------
 * getExtrnTyp
 *---------------------------------------------------
 */
getExtrnTyp(i)
{
   char buf[64];
   int j, k;

   /* aller en d‚but de nom */
   while (isspace(Rbuf[i])) i++;
   j = i;

   /* aller en fin de nom */
   while (Rbuf[i] != ' ') {
	if (Rbuf[i] == ';')
		break;
	i++;
   }
   k = i;

   /* recopie du nom */
   memset(buf, '\0', 64);
   for (i = 0; i<k-j; i++)
		buf[i] = Rbuf[j+i];
   if (strcmp(buf, "FAR") == 0)
	return(TRUE);
/*   printf("-- %s --\n", buf);*/
   return(FALSE);
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

   noligne++;
   /* saisir ligne en supprimant les CR */
   while ((c = getc(fp)) != '\n'&& c != EOF)
	     if (c != '\r')   Rbuf[input++] = c;

   if (c == '\n') {
	      Rbuf[input++] = '\0';
              return(1);
   }
   else {
	      Rbuf[input] = '\0';
              return(0);
  }
}

/*----------
 * writefuncName
 *----------
 */
writefuncName(i)
{
	char c, buf[64];
	int j, k;

	j = i;

	/* tester si Macro */
	if (Rbuf[0] == '#')
		return;

	/* aller en debut de nom */
	while ((i > 0) && !(isspace((c = Rbuf[i])))) {
		if (!isalnum(c)) {
			if (c == '_')
				i--;
			else {
				i++;
				break;
			}
		}
		else
			i--;
	}

	memset(buf,'\0',64);

	while (!isalnum(Rbuf[i]))
		if (Rbuf[i] == '_')
			break;
		else
			Rbuf[i++] = ' ';
	/* recopie */
	for (k = 0; i<=j; i++, k++)
		buf[k] = Rbuf[i];

	/*strupr(buf);*/

	for (i=0; i<CRESERVW; i++)
		if (strcmp(buf, CreservedWord[i]) == 0)
			return;

	/* enregistrer le nom de la fonction */
	recordName(buf, CFLAG);

	if (Lbrack == Rbrack)
		sprintf(writeBuf, "%4d - DEF  :  %s\n", noligne, buf);
	else
		sprintf(writeBuf, "%4d - CALL :  %s\n", noligne, buf);

	printf("%s", writeBuf);

}

/*----------
 * recordName
 *----------
 */
recordName(buf, cflag, AssTyp)
char *buf;
{
	int i, type;
	if (cflag) {
		if (Lbrack == Rbrack)   {
			if (Lbrack == 0)
				type = TYPE_DEC;
			else
				type = TYPE_DEF;
		}
		else
			type = TYPE_EXT;
	}
	else
		type = AssTyp;

	for (i = 0; functab[i].type != TYPE_NUL; i++)
		if (strcmp(functab[i].funcName, buf) == 0) {
			if (type == TYPE_DEF) {
				functab[i].type  = type;
				functab[i].ligne = noligne;
			}
			return;
		}

	strcpy(functab[i].funcName, buf);
	functab[i].type  = type;
	functab[i].ligne = noligne;
}

/*----------
 * writeResult
 *----------
 */
writeResult(type)
{
	int i;
	for (i=0;functab[i].type != TYPE_NUL; i++) {
		if (functab[i].type == type) {
			sprintf(work, "%4d (%4s) : %s\n", functab[i].ligne, typeTab[type], functab[i].funcName);
			write(fdW, work, strlen(work));
		}
	}
}
/*----------
 * updateDECL
 *----------
 */
updateDECL()
{
	int i, k;
	for (i=0, k=-1;functab[i].type != TYPE_NUL; i++)
		if (functab[i].type == TYPE_DEC)
			k = i;
	if (k >= 0)
		functab[k].type = TYPE_DEF;
}
