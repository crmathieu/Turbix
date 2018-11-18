/* gestion commentaires:
 * Ce programme supprime tous les commentaires d'un prog "C". C'est un
 * bon moyen de rendre la comprehension d'un source difficile !
 * SYNTAXE: gcom.exe  <source.c>
 */
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include <sys\stat.h>
main(argc, argv)
int argc;
char *argv[];
{
	if (argc < 2)
		exit(0);
	suppression(argv[1]);
}

suppression(fname)
char *fname;
{
    char *readbuf, *writebuf;
    int  load, wSize;
    char *pName, nfname[20];

    int fdR, fdW;

    if ((fdR = open(fname, O_RDWR)) == -1) {
	printf("\nouverture fichier %s impossible\n", fname);
	exit(-1);
    }
    if ((fdW = open("ref.ref",O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) == -1) {
	 printf("impossible de cr‚er fichier de travail\n");
	 exit(-1);
    }
    if ((readbuf = malloc(4096)) == NULL) {
	printf("Pas assez de m‚moire pour executer GCOM.EXE\n");
	exit(-1);
    }
    if ((writebuf = malloc(4096)) == NULL) {
	printf("Pas assez de m‚moire pour executer GCOM.EXE\n");
	exit(-1);
    }
    while (1) {
	memset(readbuf,  '\0', 4096);
	memset(writebuf, '\0', 4096);
	if ((load = read(fdR, readbuf, 4090)) <= 0)
		break;
	if ((wSize = automate(readbuf, writebuf, load)) > 0)
		write(fdW, writebuf, wSize);
    }
}

automate(Rbuf, Wbuf, nb)
char *Rbuf, *Wbuf;
int nb;
{
#define WAIT_SLASH1 	0
#define WAIT_CR     	1
#define WAIT_STAR1	2
#define WAIT_STAR2	3
#define WAIT_SLASH2	4

	int i, j;
	static int etat = 0;

	i = j = 0;
	while (i < nb) {
	switch(etat) {
	case WAIT_SLASH1  : if (Rbuf[i] == '/')		etat = WAIT_STAR1;
			    Wbuf[j++] = Rbuf[i];
			    break;
	case WAIT_STAR1   : if (Rbuf[i] == '*') {
				etat = WAIT_STAR2;
				j--;
			    }
			    else
				if (Rbuf[i] == '/') 	etat = WAIT_CR;
				else		    	etat = WAIT_SLASH1;
			    break;
	case WAIT_STAR2   : if (Rbuf[i] == '*') 	etat = WAIT_SLASH2;
			    break;
	case WAIT_SLASH2  : if (Rbuf[i] == '/') 	etat = WAIT_SLASH1;
			    else
							etat = WAIT_STAR2;
			    break;
	case WAIT_CR      : if (Rbuf[i] == '\n') 	etat = WAIT_SLASH1;
			    break;
	default		  : printf("???\n");
	}
	i++;
	}
	return(--j);
}

