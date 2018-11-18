#include <sys\stat.h>
#include <stdio.h>
#include <dir.h>
main()
{
	struct stat entry;
	int i;
	char cdir[80];

/*	if (stat("c:direct.c",&entry))
                printf("\nNON TROUVE\n");
        else
		printf("\nTROUVE\n");*/
	printf("** %d  **", checkDrive());
}
checkDrive()
{
	int i, j, k;
	char cdir[80];

	k = getdisk();
	j = setdisk(2); /* recuperer le nb total de drives logiques */

	for (i=0; i<=j; i++) {
		setdisk(i);
		if (getcurdir(i+3, cdir) == -1) {
			setdisk(k);
			return(i+2);
		}
	}
	setdisk(k);
	return(-1);
}
