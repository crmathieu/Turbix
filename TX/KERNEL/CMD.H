/* cmd.h */

/*
 *  entree dans la table des commandes SHELL
 */

struct  cmdslot {
        char   *cmdName;                 /* nom de la commande         */
        int    (*command)();             /* fonction commande          */
        char    justcall;                 /* procedure appelee par call */
};

extern struct cmdslot Commandtab[];

