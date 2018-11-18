/* window.h */


/* miscellenaous */
#define  TITLE_SIZE   50
#define  WOUTPUT      -2          /* sortie de type window */

/* definition de l'ecran standard */
/*#define  stdscr         tty[Tasktab[getpid()].tfd[1]->s_minor].pwin
#define  wcur           tty[Tasktab[getpid()].tfd[1]->s_minor].curwin
#define  werr           tty[Tasktab[getpid()].tfd[1]->s_minor].errwin
#define  wtail          tty[Tasktab[getpid()].tfd[1]->s_minor].tailwin
#define  wnb            tty[Tasktab[getpid()].tfd[1]->s_minor].nbwin
#define  windowMode     tty[Tasktab[getpid()].tfd[1]->s_minor].wmode
#define  minorSCR       Tasktab[getpid()].tfd[1]->s_minor
*/

/*#define  stdscr        tty[Tasktab[RUNpid].tgrp].pwin*/

#define  stdscr2(s)    tty[s].pwin
#define  wcur(s)       tty[s].curwin
#define  werr(s)       tty[s].errwin
#define  wtail(s)      tty[s].tailwin
#define  wnb(s)        tty[s].nbwin
#define  windowMode(s) tty[s].wmode
#define  minorSCR(pid) Tasktab[pid].tgrp

#define  NULLWIN        (struct window *)0
#define  WINSIZE        sizeof(struct window)

/* inhibition controle de session pour les fonctions
 * appelees par la tache _VIO:
 *      - box()
 *      - touchwin()
 *      - wrefresh()
 */
#define  NO_CHECK          0x1956
#define  CHECK             0x0412

/* definition des FLAGS de controle */
#define      W_SCROLLON           1  /* scrolling autoris‚                 */
#define      W_AUTOCRLF           2  /* CR LF sur bord droit ou troncature */
#define      W_FOREGRND           4  /* c'est la window active             */
#define      W_VISIBLE            8  /* la window est visualisable         */
#define      W_GLOBALR           16  /* rafraichir entierement la window   */
#define      W_EXCLUSIV          32  /* mode exclusif                      */
#define      W_SUSP              64  /* suspension sur INput               */
#define      W_ECHO             128  /* echo sur input                     */
#define      W_SUBWIN           256  /* sous window                        */

struct window {
       int     _cury,_curx;   /* position courante dans la fenetre en Y,X  */
       int     _maxy,_maxx;   /* hauteur , largeur                         */
       int     _begy,_begx;   /* coordonnees (xg,yg)                       */
       int     _flags;
       unsigned *_wbuff;      /* tampon                                    */
       int     _firstch;
       int     _lastch;
       uchar  *_mask;    /* masque d'ecriture : toujours 250 octets       */

       struct window *_WLOWER;  /* window visualisable inferieure     */
       struct window *_WUPPER;  /* window visualisable superieure     */

       /* chainage subwindows - Une SUBWINDOW doit etre entierement inscrite
        * dans sa window mere . les subwindows partagent le meme masque,
        * c'est … dire celui de la window mere la plus basse; par contre,
        * elles possedent leur propre tampon. Une subwindow n'a pas besoin
        * d'etre "pushee"; elle a la meme visibilite que la window mere
        */

       struct window *_SWUPPER; /* Souswindow superieure */
       struct window *_SWLOWER; /* Souswindow inferieure */

       struct window *_QNEXT; /* prochaine window dans la liste */
       struct window *_QPREV; /* precedente window dans la liste */

       unsigned char paper;    /* attribut fond                            */
       unsigned char ink;      /* attribut lettre                          */
       uchar   Hborder;
       uchar   Vborder;
       int     ansiS;
       int     wsize;             /* taille en word                        */
       uchar   INPI,INPO;
       uchar   INPbuf[WINBUFLEN];  /* buffer saisie                         */
       int     _pid;              /* pid tache en suspension               */
       int     minor;             /* no de la window (historique)          */
       char    title[TITLE_SIZE]; /* nom de la fenetre                     */
};

extern unsigned char maskbit[];
extern int             activtxtP;
