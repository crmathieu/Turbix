#include <stdio.h>
main()
{
	unsigned i;
	printf("{\n");
	for (i=0; i < 256; i++)
		printf("%d,",i);
	printf("}\n");
}