/* stat.h */

#define S_DIR   0x8000  /* directory              */
#define S_READ  0x0100  /* autorisation lecture   */
#define S_WRITE 0x0080  /* autorisation ecriture  */
#define S_EXEC  0x0001  /* autorisation execution */

/* definition  UNIX  de la structure stat */
struct   stat {
    short   st_dev;     /*  numero de drive   */
    short   st_ino;     /*  inutilise         */
    short   st_mode;    /*  attributs fichier */
    short   st_nlink;   /*  inutilise         */
    int     st_uid;     /*  inutilise         */
    int     st_gid;     /*  inutilise         */
    short   st_rdev;    /*  inutilise         */
    long    st_size;    /*  taille du fichier */
    long    st_atime;   /*  comme mtime       */

    long    st_mtime;   /*  date de la derniere
                         *  modification ( en secondes )
                         */

    long    st_ctime;   /*  comme mtime       */
};


