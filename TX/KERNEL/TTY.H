/* tty.h */
#include  "termio.h"

#define IOCHERR     0x80         /* bit set si une err survient en lec car */
#define OBMINSP       20
#define OACTIF         1
#define OINACTIF       0

/* taille des constantes */
#define EBUFLEN      0x20
#define IBUFLEN      0x80
#define OBUFLEN      0x80
#define ITBUFLEN     0x10
#define BUFMASK      0x7f
#define EBUFMASK     0x1f
#define ITBUFMASK    0x0f

#define WINBUFLEN    0xf0
#define WINBUFMASK   0x7f

/* constantes liees au mode d'utilisation */
#define DEVNAMELEN  10

/* caracteres speciaux par defaut */
#define     ERASEC     '\b'        /* CTRL H */
#define     BELL       '\07'
#define     KILLC      '\x15'      /* CTRL U */
#define     BLANK      '\x20'
#define     NEWLINE    '\n'
#define     RETURN     '\r'
#define     STOPC      '\x13'      /* CTRL S */
#define     STARTC     '\x11'      /* CTRL Q */
#define     EOFC       '\x04'      /* CTRL D */
#define     EOLC       '\x00'      /* CTRL @ */
#define     INTRC      '\x03'      /* CTRL C */
#define     QUITC      '\x1A'      /* CTRL Z */
#define     UPARROW    '^'

/* codes de controle des fonctions tty */

#define     TCSETBRK   1
#define     TCRSTBRK   2
#define     TCNEXTC    3
#define     TCMODER    4
#define     TCMODEC    5
#define     TCMODEK    6
#define     TCFULLC    BELL
#define     TCICHARS   8
/*#define     TCECHO     34
#define     TCNOECHO   35*/
#define     TCINT      11
#define     TCIBUFF    12

struct mouEvent {
       unsigned event;
       int      mouX,
                mouY;
};


struct  tty     {
   /* gestion de la table */
   int            ttyminor;
   char           ttyname[DEVNAMELEN];
   stream_entry  *streamptr;   /* stream entry ou est ouvert le device    */
   int            vsinit;      /* VS deja initialis‚ ?                    */
   unsigned      *vsbuf;       /* buffer de sauvegarde du VS              */

   /* gestion RAM SCREEN */
   int            cursY;       /* ligne courante                          */
   int            cursX;       /* colonne courante                        */
   int            curs_shape;  /* forme du curseur                        */
   unsigned char  cvatt;       /* attribut video courant                  */
   char           echar;       /* caractere suivant ESC                   */
   unsigned       offsetRAM;   /* offset ecriture ds buffer ou RAM screen */

   /* gestion codes ANSI */
   int            oesc;        /* etat echappement                        */
   char           ansiC[10];   /* chaine d'echappement                    */
   int            nansi;       /* nb de caracteres dans la chaine         */
   unsigned char  ansiB;       /* ansi code pour Bcolor                   */
   unsigned char  ansiF;       /* ansi code pour Fcolor                   */

   /* gestion Windows  */
   int           wmode;        /* TRUE : mode window                      */
   int           nbwin;        /* nb de windows ouvertes dans ce EV       */
   struct window *pwin;        /* window attachee a l'EV (stdscr) -       */
                               /* sert egalement de tete de liste         */
                               /* de toutes les windows creees            */
   struct window *curwin;      /* window active dans cet EV               */
   struct window *errwin;      /* window recevant les messages d'ERR      */
   struct window *tailwin;     /* queue de liste des windows creees       */
   int           pidDemon;     /* pid du demon initialis‚ par initScreen  */

   /* buffers */
   unsigned ITbuff[ITBUFLEN];  /* buffer de stockage IT input             */
   unsigned Ibuff[IBUFLEN];    /* buffer canonique d'entree               */
   unsigned Obuff[OBUFLEN];    /* buffer de sortie (en word)              */

   /* gestion modes */
   struct termio ctty;

   /* gestion LOCK session */
   int           tlock;


   /* gestion IT en entree */
   int            ITin;
   int            ITout;

   /* gestion input caractere */
   int            Iin;         /* tete file d'entree                      */
   int            Iout;        /* queue file d'entree                     */
   SEM            isem;        /* semaphore d'entree                      */
   int            icursor;     /* position courante du curseur            */
   int            imaxcnt;     /* longueur maxi de caracteres a l'ecran   */

   /* gestion output caractere */
   int            Oin;         /* tete file de sortie                     */
   int            Oout;        /* queue file de sortie                    */
   int            Ocnt;
   SEM            osem;        /* semaphore de sortie                     */
   int            odsend;      /* gestion WATER MARKS                     */
   int            oetat;       /* etat du circuit serie                   */

   /* gestion des HotKey */

   /* gestion souris */
   int      mouse_state;
   unsigned mask;              /* masque de declenchement evenements */
   unsigned testMask;          /* masque gestion EV (niveau driver)  */
   int      mouActiv;          /* souris activ‚e dans cette session */
   int      mouCpt;            /* cpt de visualisation curseur :
                                * initialis‚ a -1, le cuseur est visible
                                * si sa valeur est >= 0
                                */
   int      mouX, mouY;        /* sauvegarde de la position souris au moment
                                * du SWAP session
                                */
   struct mouEvent MouBuf[16]; /* buffer message de la souris */
   int             Min;
   int             Mout;
   int             MouEvent;   /* # d'Ev dans la queue souris */
   SEM             MouSem;     /* semaphore souris pour cette session */

   /* pointeur sur table de controle du circuit serie */
   struct  csr    *ioaddr;     /* adresse table de controle circuit serie */
   int            user;        /* numero utilisateur lie a ce tty         */

};


extern char *ttyname[];   /* contient les noms des tty */

extern struct tty tty[];

/* macro */
#define ischar(x)      (tty[x].ITin != tty[x].ITout)
#define Vscreen(x)     (x->ttyminor < TTY1)
#define iscanon(x)     (x->ctty.c_lflag & ICANON)
#define israw(x)       (!(x->ctty.c_lflag & (ICANON | ISIG)))
#define isecho(x)      (x->ctty.c_lflag & ECHO)
/*#include  "window.h"*/
#include "serial.h"
