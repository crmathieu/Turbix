/* fspipe.h */

#define  NPIPE        30   /* # de pipes possibles                  */
#define  PIPESIZE  8*512   /* taille d'un pipe                      */
#define  PEXIT        17   /* marquer pipe invalide                 */
#define  PRUN         18   /* le pipe est valide                    */
#define  PREAD         0
#define  PWRITE        1
#define  NOPIPE       -1

struct pipslot {
       int pipe_nr;       /* numero du pipe                        */
       int invalid;       /* pipe invalide si plus de lecteur
                           * ou si plus d'ecrivain
                           */

       int count;         /* # d'ouvertures sur ce pipe            */
       int avail;         /* # caracteres a lire sur le pipe       */
       int susptask[2];   /* cpt # taches bloquees en R ou W (0,1) */
       int offR;          /* offset en lecture                     */
       int offW;          /* offset en ecriture                    */
       char *pipzon;      /* zone d'echange du pipe                */
};

extern struct pipslot Piptab[];
extern int piphead,piptail;