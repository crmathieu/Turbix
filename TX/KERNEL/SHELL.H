/* shell.h */


/*
 * Declarations pour le shell
 *
 */

#define SHEXIT     -5                  /* shell exit                        */
#define SHNAMLEN    9                  /* longueur du nom user + 1          */
#define SHBUFLEN    80                 /* longueur du buffer                */
#define SHARGLEN    82                 /* longueur de la zone des arguments */
#define SHMAXTOK    16                 /* maximum de tokens par ligne       */
#define SHMLEN      12                 /* longueur maximum du nom machine   */
#define SHCMDSTK    512                /* taille de la pile du proc command */
#define SHCMDPRI    30                 /* priorite du process               */

#define MAX_PATH_DEEPTH	8	/* profondeur Maxi d'un Path */

struct  uwd {                          /* User Working Directories          */
	ushort  cwd;                   /* cwd utilisateur pour ce device    */

        /*  les variables suivantes sont uitlisees
         *  par Shell , s'il existe
         */

	int  deepth;                       /* profondeur du path            */
	char pathstr[MAX_PATH_DEEPTH][13];/* path string : 8 str de 13 carac*/
};

struct  session {
        struct   uwd      u_drive[NDISK];
        int               u_currDev;     /* device courant utilisateur      */
};

struct  shvars {                       /* shell variables                   */
        long   shlast;                 /* derniere frappe                   */
        int    shncmds;                /* nombre de commandes valides       */
        char   shbuf[SHBUFLEN];        /* buffer du shell                   */
        char   *shtok[SHMAXTOK];       /* pointeurs d'entree d'un token     */
        char   shtktyp[SHMAXTOK];      /* type de token dans shtok[i]       */
        char   shargst[SHARGLEN];      /* zone stockage string arg          */
};



extern  struct session SessionTab[];


