extern  int    currDrive;
extern  char   currWD[];

/* variable de configuration */
extern unsigned         materialConf;
extern int              nflp_phy;
extern struct LOL      *lolp, *getListOfList();

/* Gestion des drives */
struct DriveTable {
        char drive;
        char valide;
        char *path;
};

/* pointeur sur la table des drives (qui est allouee dynamiquement) */
extern struct DriveTable *pdt;

/* nombre de drives possibles */
extern int lastdrive;

/* pointeur sur handler utilisateur */
extern int (* _uErrHandler)();
