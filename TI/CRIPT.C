/* Generation d'un copyright cript‚ */
#include <stdio.h>
#include <alloc.h>

unsigned char *shware[] = {
" ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n",
" ³                        Welcome, TURBIX user!                         ³\n",
" ³                                                                      ³\n",
" ³                    We hope you enjoy the product!                    ³\n",
" ³                                                                      ³\n",
" ³  TURBIX is a shareware. You are allowed to test this product with    ³\n",
" ³  the sample source files available thru this disk before sending     ³\n",
" ³  your $29 contribution. This is a full version of TURBIX, but you    ³\n",
" ³  won't find any API documentation. If you send your contribution,    ³\n",
" ³  you will receive a disk which contains the API functions (provided  ³\n",
" ³  with examples), and how to use make files. More, you'll be able to  ³\n",
" ³  cut and paste examples from the help window to the edit window,     ³\n",
" ³  which will make your multitasking development easier.               ³\n",
" ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n",
" ³  to ORDER your Online TURBIX documentation files, send $29 at:       ³\n",
" ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n",
""};

int size;
unsigned char *cp2;
int *lsize;
int dimension;

main()
{
        FILE *fp;
        int i, j, k, size;
        unsigned char z;

        /* determiner la taille du source a cripter */
        for (i=0, dimension = 0, size = 0, k = strlen(shware[i]);
             k > 0; i++, k = strlen(shware[i])) {
             dimension++;
             size += k;
        }
        /* ajouter les fin de string */
        size += (i+1);
        dimension++;

        /* reserver la memoire necessaire */
        if (((void *)cp2 = malloc(size)) == NULL) {
                printf("\nImpossible d'allouer buffer\n");
                exit(-1);
        }
        if (((void *)lsize = malloc(dimension*2)) == NULL) {
                printf("\nImpossible d'allouer dimension\n");
                exit(-1);
        }

        printf("\nchar shware[] = {\n");
        for (i=0, size = 0, z = 0, k = strlen(shware[i]); k > 0 ; i++, k = strlen(shware[i])) {
                for (j=0; j < k; j++, z++)
                        cp2[j+size] = shware[i][j] ^ z;

/*              cp2[j+size] = 0;*/
                for (j=0; j<k; j++) {
                        printf("0x%02x,",cp2[j+size]);
                        if ((j % 20) == 19)
                                printf("\n");
                }
                printf("\n");
                size += k;
                lsize[i] = k;
        }
        lsize[i] = 0;
        printf("};\n");

        /* generer le source int lsize = {...}; */
        printf("int lsize[] = {\n");
        for (i=0, size = 0, k = strlen(shware[i]); k > 0 ; i++, k = strlen(shware[i])) {
/*              for (j=0; j < k; j++)
                        cp2[j+size] = shware[i][j] ^ i;

                cp2[j+size] = 0;*/
                printf("0x%02x,",k);
                size += k;
                lsize[i] = k;
        }
        printf("0 };\n");

        /* checking */
        for (i=0, z = 0, k = 0;lsize[i] > 0;i++, k += lsize[i])
                for (j=0; j<lsize[i]; z++, j++)
                        putchar(cp2[j + k] ^ z);

}
