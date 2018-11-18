/* sconf.h */

struct  configuration {
        char c_eof;
        char c_t;        /* type de compilateur */
        union {
        char c_p[10][80];
        struct pd {
           char c_cl[80];        /* comp & linker dir   */
           char c_a[80];         /* assembleur dir      */
           char c_i[80];         /* turbo include dir   */
           char c_l[80];         /* turbo lib dir       */
           char c_I[80];         /* MKD include dir     */
           char c_x[80];         /* user dir name       */
           char c_u[80];         /* MKD lib dir         */
           char c_m[80];         /* make file name      */
           char c_p[80];         /* paste file name     */
           char c_h[80];         /* help dir name       */
        } pd;
        } u;
        int reload_flag;         /* reloading ou non ?  */
        int  mkd_shell,
             mkd_FS,
             mkd_AutoAsk,
             mkd_masm,
             mkd_tasm;
        int    autoSave;         /* flag autosauvegarde */
        int    kbdFast;          /* flag vitesse clavier */
        int    buildDB;          /* flag de reconstruction de la base */
        int nb_car_per_line;     /* # de caracteres / ligne */
        char   projExe[80];      /* nom de l'executable du projet */
        struct pick reload[19];  /* si oui, config du fichier */
};



