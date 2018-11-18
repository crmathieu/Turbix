/* iodos.h */

struct ioDosReq {
        Bool            focus;          /* se positionner / au stream Pos */
        struct  ioDosReq *drnext;       /* pointeur sur requete suivante */
        int             drpid;          /* PID de la tache appelante */
        char           *drbuff;         /*      @ du bloc d'E/S
                                         * OU   @ Zstring PATH
                                         * OU   Pointeur de fichier (long)
                                         */
        char            drop;           /* type d'operation */
        int             access;         /* OPEN op seulement */
        int             mode;           /* CREAT op seulement */
        int             Rhandle;        /* handle reel */
        int             count;
        ushort          drstat;         /* status apres operation */
        long            drseek;         /* SEEK op seulement : I/O field */
        stream_entry   *drsp;           /* pointeur sur stream correspondant */
};

#define IONULL       (struct ioDosReq *)0

/* differents op‚rations supportes */
#define IO_READ         0x10
#define IO_WRITE        0x11
#define IO_OPEN         0x12
#define IO_CLOSE        0x13
#define IO_CREATE       0x14
#define IO_REMOVE       0x15
#define IO_MOVE         0x16
#define IO_TELL         0x17

#define IO_GETFATT      0x18
#define IO_SETFATT      0x19
#define IO_CHDIR        0x20
#define IO_MKDIR        0x21

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

