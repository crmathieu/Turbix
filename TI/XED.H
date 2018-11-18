/* definition des differents menus */


#ifdef TURBO
#include <stdio.h>
#include <alloc.h>
#include <dir.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <io.h>
#include <ctype.h>
#include <mem.h>
#include <sys\types.h>
#include <sys\stat.h>

#define  _harderr          harderr
#define  _bios_equiplist   biosequip
#define  _hardretn         hardretn
#define  _hardresume       hardresume

#else
#include <stdio.h>
#include <malloc.h>
#include <direct.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <io.h>
#include <ctype.h>
#include <memory.h>
#include <process.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif

#define FUNCTION
#define DEBUG           41256   /* signature debug */
#define EOFM             0x1A   /* marque fin de fichier */
#define BIGBUF_SIZE     65500   /* taille du buffer de travail */
#define NIL_LN          0xffff  /* gestion blocs */
#define ERASE             1     /* erase trailing blanks */
#define NO_ERASE          0     /* no erase trailing blanks */
#define DELETE_BLOCK      1     /* se positionner au debut du block supprim‚ */
#define MOVE_BLOCK        2     /* ne pas se positionner */
#define COPY_BLOCK        3     /* */
#define EOS             '\n'
#define BLANK           ' '

/* constantes de gestion insertion ligne */
#define   ON_MODE      0    /* insertion suivant le mode en cours */
#define   INSERT_MODE  1    /* force le mode en cours en mode INSERT */


/* Definitions utilisees pour le controleur driver */
#define BEEP_FREQ       0x0533
#define B_TIME          0x2000
#define TIMER2            0x42
#define TIMER3            0x43
#define PORT_B            0x61

/* constantes relatives au 6845 */
#define M_6845           0x3b0
#define C_6845           0x3d0
#define MODE                 8
#define CONF                15
#define INDEX                4
#define DATA                 5
#define CUR_SIZE            10
#define VID_ORG             12
#define CURSOR              14

/* constantes curseurs */
#define CURS_HIDE      32
#define CURS_SMALL      0
#define CURS_LARGE      1

#define CURSOR_START  10
#define CURSOR_END    11

#define S_MONO_SMALL  12
#define S_MONO_LARGE   0
#define S_COLOR_SMALL  6
#define S_COLOR_LARGE  0

#define E_MONO        13
#define E_COLOR        7


/*#define HIDE      32
#define SMALL      0
#define LARGE      1
#define CURSOR_START  10   /* R10 of 6845 */
/*#define CURSOR_END    11   /* R11 of 6845 */
/*#define S_MONO_SMALL  12
#define S_MONO_LARGE   0
#define S_COLOR_SMALL  6
#define S_COLOR_LARGE  0
#define E_MONO        13
#define E_COLOR        7*/


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * position des cl‚s
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */

/* num pade */
#define MOUSE_L       01
#define MOUSE_R       02
#define ARROWL        75
#define ARROWR        77
#define ARROWD        80
#define ARROWU        72
#define HOME          71
#define END           79
#define PGUP          73
#define PGDN          81
#define INSERT        82
#define DELETE        83

/* ctrl num pade */
#define CTRL_ARRL    115
#define CTRL_ARRR    116
#define CTRL_END     117
#define CTRL_PGDN    118
#define CTRL_HOME    119
#define CTRL_PGUP    132

/* fonctions */
#define F1            59
#define F2            60
#define F3            61
#define F4            62
#define F5            63
#define F6            64
#define F7            65
#define F8            66
#define F9            67
#define F10           68

/* shift fonction */
#define SHIFT_F1       84
#define SHIFT_F2       85
#define SHIFT_F3       86
#define SHIFT_F4       87
#define SHIFT_F5       88
#define SHIFT_F6       89
#define SHIFT_F7       90
#define SHIFT_F8       91
#define SHIFT_F9       92
#define SHIFT_F10      93

/* ctrl fonction */
#define CTRL_F1       94
#define CTRL_F2       95
#define CTRL_F3       96
#define CTRL_F4       97
#define CTRL_F5       98
#define CTRL_F6       99
#define CTRL_F7      100
#define CTRL_F8      101
#define CTRL_F9      102
#define CTRL_F10     103

/* alt fonction */
#define  ALT_F1      104
#define  ALT_F2      105
#define  ALT_F3      106
#define  ALT_F4      107
#define  ALT_F5      108
#define  ALT_F6      109
#define  ALT_F7      110
#define  ALT_F8      111
#define  ALT_F9      112
#define  ALT_F10     113


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * types de touches
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
#define NUMPAD      0x100   /* type NUM PADE                                */
#define CTRL        0x200   /* type CTRL i                                  */
#define ALT         0x400   /* type ALT i                                   */
#define FUNC        0x800   /* type Fi                                      */
#define CTRLFUNC    0x1000  /* type ctrl Fi                                 */
#define CTRLNUMPAD  0x2000  /* type ctrl home, end, etc ..                  */
#define SHIFTFUNC   0x4000  /* type shift Fi                                */
#define ALTFUNC     0x8000  /* type alt Fi                                  */
#define NORMAL      0       /* touche normale                               */

#define BACKSP      0x08    /* back space                                   */
#define RETURN      0x0d    /* retour chariot                               */
#define ESC         0x1b    /* touche ESC                                   */
#define TAB         0x09    /* touche TAB                                   */
#define SHIFTTAB    0x0f    /* SHIFT TAB (conditionn‚e en ALT|SHIFTAB)      */


#define CMDKEY      0x1A    /* seuil au dela duquel c'est une touche normal */
#define NOEXTC      0       /* pas de commande de declenchement autre que par lettre de commande */

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * lettres de commande
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */

/* master menu */
#define  ALT_F  33
#define  ALT_E  18
#define  ALT_R  19
#define  ALT_C  46
#define  ALT_B  48
#define  ALT_O  24
#define  ALT_H  35
#define  ALT_Q  16

#define  ALT_I  23

/*ÄÄÄÄÄÄÄÄÄÄÄÄ
 * constantes
 *ÄÄÄÄÄÄÄÄÄÄÄÄ
 */

/* divers */
#define  NEWATT                  1
#define  OLDATT                  0
#define  OK                      1
#define  TRUE                    1
#define  FALSE                   0
#define  NIL                    -1
#define  YES                     1
#define  NO                      0
#define  NOTHING                 0
#define  BOOL        unsigned char
#define  VER                     1   /* menu de type VERTICAL */
#define  HOR                     2   /* menu de type HORIZONTAL */
#define  FIXM                    3   /* menu de taille FIXE */
#define  VARM                    4   /* menu de taille variable */

#define DEFAULT_NB_CAR_PER_LINE 256
#define MAX_COLS 80
#define MAX_LINES 24
#define BUFSIZE 1024
#define ASCII     1
#define OBJECT    0

#define FIL       0
#define DIR       1
#define DMSK      2
#define BNAME     3
#define EMPTY     4
#define DISK      5

/* video */
#define MONOBASE  0xb000
#define COLORBASE 0xb800
#define VIDMONO   0xb0000000
#define VIDCOLOR  0xb8000000


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*
 *   7        6         5         4          3        2       1       0     *
 * Blink     b_Red    b_green   b_blue   Intensity   f_Red   f_green f_blue *
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*
 */

#define  BLINK         0x80
#define  F_BLACK       0
#define  B_BLACK       0

#define  B_RED         (adapter? 0x40:0)
#define  B_GREEN       (adapter? 0x20:0)
#define  B_BLUE        (adapter? 0x10:0)
#define  INTENSITY     0x08
#define  F_RED         (adapter? 0x04:0x07)
#define  F_GREEN       (adapter? 0x02:0x07)
#define  F_BLUE        (adapter? 0x01:0x07)

/* COMPLEX  ATTRIBUTES */

/* Foreground */
#define  F_WHITE        0x07
#define  F_CYAN         (adapter? 0x03:0x07)        /* vert     bleu   */
#define  F_MAGENTA      (adapter? 0x05:0x07)
#define  F_YELLOW       (adapter? 0x06:0x07)        /* jaune    fonc‚  */
#define  F_LBLUE        (INTENSITY|F_BLUE)        /* bleu     clair  */
#define  F_LRED         (INTENSITY|F_RED)         /* rouge    clair  */
#define  F_LGREEN       (INTENSITY|F_GREEN)       /* vert     clair  */
#define  F_LCYAN        (INTENSITY|F_CYAN)        /* cyan     clair  */
#define  F_LMAGENTA     (INTENSITY|F_MAGENTA)     /* magenta  clair  */
#define  F_LYELLOW      (INTENSITY|F_YELLOW)      /* jaune    clair  */
#define  F_LWHITE       (INTENSITY|F_WHITE)       /* blanc    vif    */

/* Background */
#define  B_WHITE        0x70
#define  B_CYAN         (adapter ? 0x30:0)
#define  B_MAGENTA      (adapter ? 0x50:0)
#define  B_YELLOW       (adapter ? 0x60:0)

#define MONO    0
#define CGA     1
#define EGA     2

/* color by type */
#define ATT_WBORDER    F_WHITE              /* encadrement window         */
#define ATT_WNOSELECT (adapter ? F_LWHITE:F_WHITE)    /* option window non selectee */
#define ATT_WSELECT   (adapter ? F_LWHITE|B_MAGENTA:B_WHITE)     /* option window selectee     */
#define ATT_MNOSELECT   (F_LWHITE|B_MAGENTA)       /* option menu non selectee   */
#define ATT_MSELECT   (adapter ? F_LWHITE|B_BLUE:B_WHITE)      /* option menu selectee       */
#define ATT_ED_NORMAL (adapter ? F_LYELLOW : F_WHITE)              /* mode ed : att normal       */
#define ATT_ED_SELECT  (F_LWHITE|B_MAGENTA)       /* mode ed : att selection b/p*/
#define ATT_ED_INVERSE (adapter ? F_LWHITE|B_MAGENTA:B_WHITE)    /* mode ed : att inverse      */
#define ATT_MESSAGE   (F_LWHITE|B_MAGENTA)       /* messages ed                */
#define ATT_CTRL      (F_BLACK|B_WHITE)    /* controle                   */
#define ATT_RESETCTRL  F_WHITE               /* reset controle             */



/* constants for box types */
#define LUC 0        /* left upper corner */
#define RUC 1        /* right upper corner */
#define LLC 2        /* left lower corner */
#define RLC 3        /* right lower corner */
/*#define VER 4        /* vertical character */
/*#define HOR 5        /* horizontal character */

#define   MENU     1
#define   WINDOW   2

/* types de contour de boxe */
#define TYP1    0
#define TYP2    1
#define TYP3    2
#define TYP4    3
#define TYP5    4
#define TYP6    5
#define TYP7    6

#define W_OPEN  1
#define W_CLOSE 2
#define W_SWIN  1     /* fenetre de saisie */
#define W_WIN   0     /* fenetre normale */

#define LOAD_FILE  1
#define LOOK_FILE  2

#define LOADED  1
#define NEWFILE -1
#define FXLARGE  0

/* gestion saisie du nom de fichier */
#define  NB_FILE_PER_PAGE  32
#define  DIR_LST           sizeof(struct dir_lst)

#define  AUTO_MODE         1
#define  MANUAL_MODE       2

#define TOK_FILE   1    /* type fichier */
#define TOK_DIR    2    /* type directory */
#define TOK_UNK    3    /* type inconnu : NEW file */
#define TOK_WILD   4    /* type wild car */

#define LOAD       4
#define NEW        5
#define GET_LST_DSKEMPTY  0
#define GET_LST_DSKOVF   -1
#define GET_LST_DSKERROR -2

#define INT24FAIL        0x53

#define CURRDRIVE_A       0
#define CURRDRIVE_B       1

#define LPOS_INT24        41  /* position reponse dans error handler */
#define DPOS_INT24         8  /* position drive letter dans error handler */
#define DPOS_VF           27  /* position drive letter dans verify floppy */

/* MACROS */
#define INF(x,y)          ((x)<(y) ?(x):(y))
#define SUP(x,y)          ((x)>(y) ?(x):(y))
#define ITEM(p,x)         ((p)->items+x)
#define ISINBLK(wp,pt)    (((pt) > (wp)->topBlock)&&((pt) < (wp)->bottomBlock))
#define ISINBLOCK(wp,pt,len)  ((((pt+len) > (wp)->topBlock)&&((pt) < (wp)->topBlock))) ||\
                               (((pt+len) > (wp)->bottomBlock)&&((pt) < (wp)->bottomBlock))))

/* definition d'un menu */
struct Menu {
      unsigned *save;      /* tampon de sauvegarde                   */
      char *title;         /* nom du menu                            */
      int   ul_x,          /* left upon corner X                     */
            ul_y,          /* left upon corner Y                     */
            ncol,          /* NCOL  :menu  width                            */
            nline,         /* NLINE :number of items                        */
            curY,          /* CURY  :current item                           */
            border,        /* attribut contour fenetre               */
            ink,           /* foregrnd  att                          */
            paper,         /* backgrnd  att                          */
            pushed,        /* WPUSHED ce menu est deja PUSHE                 */
            f_ombrage;     /* ombrage valide                         */
unsigned    blockAtt;      /* BOCKATT attributs(F&B) de la rubrique courante */

      /* partie propre aux menus */
      int   mov,           /* type de selection (depl VER ou HOR)    */
            typ,           /* MFIX ou MVAR                           */
            smenu,         /* sous menus visibles                    */
            flcom,         /* flag de validation attribut lettre com */
            lcom_att;      /* attributs lettre de commande           */



      struct ITem *items;  /* pointeur sur les elements du  menu     */
      struct Menu *UPm;    /* menu superieur                         */
};

/* Element d'un menu */
struct ITem {
      char  *str;          /* nom de la rubrique */
      char   lcom;         /* lettre de commande */
      int    extra_command;/* autre commande de declenchement */
      int    ecart;        /* pour type HOR : offset / rubrique precedente */
      int    offlcom;      /* offset lettre de commande dans la string */
      BOOL   sm;           /* indique si la rubrique declenche un sous menu */
      struct OBJ *SM;     /* sous menu a ouvrir (si sm = TRUE) */
      int (* SMaction)();  /* fonction de gestion du sous menu ou fct terminale */
};

/* table d'acc‚s a un menu / … une commande */
struct ctab {
       unsigned char extra_com;  /* commande d'acces */
       char          type;       /* terminale ou non */
       int           path[10];   /* chemin d'acces   */
};
#define  TERMINAL    1     /* action a declencher de type terminale */
#define  NOTERMINAL  2     /* action a declencher de type non terminale */
#define  NP         -1     /* valeur path non positionn‚e */
#define  NOKEY       0

/* definition de la PICK structure (donne la position dans un fichier) */
struct pick {
       char       str[80];
       unsigned   current,
                  topPage,
                  topBlock,
                  bottomBlock,
                  fsize;
       int        current_line_no,
                  topPage_line_no,
                  leftCh,
                  curX,
                  curY,
                  fflag;
};
#define  PICKSIZE  sizeof(struct pick)

/* definition d'une fenetre */
struct Win {
      unsigned    *save;       /* tampon de sauvegarde                     */
      char        *title;      /* titre eventuel de la fenetre             */
      int          ul_x,       /* left upon corner X                       */
                   ul_y,       /* left upon corner Y                       */
                   ncol,       /* number of columns                        */
                   nline,      /* number of lines                          */
                   curY,       /* current line in window                   */
                   border,     /* attribut contour win                     */
                   ink,        /* foregrnd  att                            */
                   paper,      /* backgrnd  att                            */
                   pushed,    /* la fenetre est visible                   */
                   f_ombrage;  /* ombrage valide                           */
        unsigned   blockAtt;   /* attribut de block defini                 */


        /* partie propre aux fenetres */
        int        leftCh,     /* nombre de carac non visibles a gauche    */
                   curX,       /* current col in window                    */
                   f_bar,      /* barre de position valide                 */
                   borderTyp;  /* type de contour                          */

};

/* definition d'un champ d'action souris */
struct  MOUSE_FIELD {
        int     rX, rY,         /* position relative du champs / … la fenetre */
                size;           /* taille du champs */
        int (* MouAction)();    /* fonction de gestion du champs */
};

/* definition d'un OBJET */
struct OBJ {
      int          objet;      /* type de l'objet : MENU ou WINDOW */
      unsigned     m_key;      /* cl‚ d'ouverture de l'objet               */
      unsigned    *save;       /* tampon de sauvegarde                     */
      char        *title;      /* titre eventuel de l'OBJET                */
      int          ul_x,       /* coin sup gauche  X                       */
                   ul_y,       /* coin sup gauche  Y                       */
                   ncol,       /* nb de colonnes                           */
                   nline,      /* nb de lignes                             */
                   curY,       /* ligne courante                           */
                   border,     /* attribut du contour de  l'OBJET          */
                   ink,        /* attribut avant plan                      */
                   paper,      /* attribut arriere plan                    */
                   pushed,     /* l'OBJET est visible                      */
                   f_ombrage;  /* ombrage valide                           */
        unsigned   blockAtt;   /* attribut de block defini                 */


        /* chainage des OBJ */
        struct OBJ *prev;      /* objet precedant */
        struct OBJ *next;      /* objet suivant   */

        /* action souris dans l'automate general */
        int (* M_Gpr)();   /* sur pression bouton gauche   */
        int (* M_CGpr)();  /* sur pression gauche en cours */
        int (* M_Gre)();   /* sur release bouton gauche    */

        int (* M_Dpr)();   /* sur pression bouton droit    */
        int (* M_CDpr)();  /* sur pression droite en cours */
        int (* M_Dre)();   /* sur release bouton droit     */

        int (* M_Mov)();   /* sur deplacement de la souris */

        /* evenements acceptes par cet OBJ */
        unsigned  mouse_ev_flag;

        /* Champs g‚r‚s par la souris (utilises si besoin
         * pour les fenetres)
         */
        struct MOUSE_FIELD *mField;

        /* Action souris par defaut */
        int (* DefaultMouAction)();

        /* partie propre aux fenetres */
        int        leftCh,     /* nombre de carac non visibles a gauche    */
                   curX,       /* current col in window                    */
                   borderTyp,  /* type de contour                          */
                   f_arrow,    /* fleche de direction valide               */
                   f_bar;      /* barre de position valide                 */


      /* partie propre aux menus */
      int   mov,           /* type de selection (depl VER ou HOR)    */
            typ,           /* MFIX ou MVAR                           */
            smenu,         /* sous menus visibles                    */
            flcom,         /* flag de validation attribut lettre com */
            lcom_att;      /* attributs lettre de commande           */



      struct ITem *items;  /* pointeur sur les elements du  menu     */
      struct OBJ *UPm;    /* menu superieur                         */

};

/* Partie Propre au Fonctionnement de l'EDITEUR */

struct ed {
      char         fname[13];  /* nom de fichier                           */
      unsigned     fsize,      /* taille actuelle du fichier en memoire    */
                   bottom,     /* offset derniere ligne                    */
                   current,    /* offset ligne courante                    */
                   topPage,    /* offset ligne en sommet de fenetre        */
                   topBlock,   /* offset ligne de debut bloc               */
                   bottomBlock;/* offset ligne de fin de block             */
      int          fillCh,     /* # Blancs ajout‚s apres une fin de ligne  */
                   fflag,      /* gestion etat du fichier dans l'editeur   */
                   current_line_no, /* no de la ligne courante             */
                   topPage_line_no, /* no de ligne du sommet de page       */
                   bottom_line_no,  /* no de ligne de le derniere ligne    */
                   firstCh,    /* 1er ch sur 1ere ligne du bloc / paste    */
                   lastCh;     /* dernier ch sur dern ligne du bloc/paste  */
};

/* VARIABLES de travail de l'editeur */
extern int        fdcurr;     /* file descriptor du fichier charg‚        */
extern char       *linebuf;   /* tampon ligne             */
extern char  far *bigbuf;     /* buffer de reception du fichier           */
extern unsigned   fsize,      /* taille actuelle du fichier en memoire    */
                  bottom,     /* offset derniere ligne                    */
                  current,    /* offset ligne courante                    */
                  topPage,    /* offset ligne en sommet de fenetre        */
                  topBlock,   /* offset ligne de debut bloc               */
                  bottomBlock;/* offset ligne de fin de block             */
extern int        fillCh,     /* # Blancs ajout‚s apres une fin de ligne  */
                  fflag,      /* gestion etat du fichier dans l'editeur   */
                  blockatt,   /* attribut block defini                    */
                  current_line_no, /* no de la ligne courante             */
                  topPage_line_no, /* no de ligne du sommet de page       */
                  bottom_line_no;  /* no de ligne de le derniere ligne    */


/* variables definissant les attributs video */
extern unsigned  char   WINink,         /* avant plan fenetre */
                        WINpaper,       /* arriere plan fenetre */
                        WINborder,      /* cadre fenetre        */
                        WINblock,       /* block dans fenetre   */
                        WINSYSink,         /* avant plan fenetre */
                        WINSYSpaper,       /* arriere plan fenetre */
                        WINSYSborder,      /* cadre fenetre        */
                        WINSYSblock,       /* block dans fenetre   */
                        WINmess,        /* message dans fenetre */
                        MENUborder,     /* cadre menu */
                        MENUselected,   /* article selectionn‚ */
                        MENUlcom,       /* lettre de commande */
                        MENUink,        /* avant plan menu */
                        MENUpaper,      /* arriere plan menu */
                        MESS24att,      /* att message derniere ligne */
                        MESSatt;        /* att window message */




/* string gestion fichier */
extern char     *fillstr,     /* string de "bourrage" */
                absfname[],    /* nom absolu du fichier courant */
                cdirname[],    /* nom directorie courante */
                file_name[],   /* chaine de travail */
                mask[],        /* masque courant */
                pastefile[],   /* paste file courant */
                homedir[],     /* nom directorie sous laquelle on a demarr‚ */
                defaultMask[], /* masque par defaut */
                nullstr[];     /* string nulle */

extern struct ctab Ctab[];

/* definition de toutes les fenetres */
extern struct OBJ  edwin,      /* fenetre de l'editeur */
                   syswin,     /* fenetre des messages systemes */
                   dirwin,     /* fenetre affichage de la directory */
                   dialwin,    /* fenetre de dialogue */
                   rdialwin;   /* fenetre reduite de dialogue */

/* REM: le vecteur MESSWIN sert a afficher les messages d'erreur systemes.
 *      Si l'erreur se produit dans la fenetre de dialogue, messwin
 *      doit etre initialis‚e a la valeur de dialwin au point (1,4).
 *      Si l'erreur se produit dans la fenetre d'edition, une fenetre de
 *      travail doit etre cr‚ee et messwin doit etre alors etre initialis‚e
 *      a la valeur de cette fenetre au point (0,0)
 *      (exemple dans la fonction <loadFile> du fichier "FTERM.C")
 */
/* vecteur MESSWIN */
extern struct OBJ *messwin;    /* pointe sur dialwin ou une W locale */
extern int Xmess, Ymess;       /* coordonnees d'affichage ds messwin */

/* variables de sauvegarde parametres Globaux de l'editeur */
struct save_edit {
       unsigned s_curr,
                s_page,
                s_bottom,
                s_topBlock,
                s_bottomBlock,
                s_flag,
                s_fsize;
       int      s_current_line_no,
                s_topPage_line_no,
                s_bottom_line_no,
                s_leftCh,
                s_curX,
                s_curY;
       char    *s_bigbuf;
};

extern unsigned getnextline(),
                getprevline(),
                lnprev(),
                lnnext(),
                getlast_ln(),
                getdebln(),
                display_dir();

extern struct pick pop_pick();
extern unsigned ReturnFromSaveFile; /* gestion retour de doSaveFile() */

struct DTA {
       char reserved[21]; /* reserv‚ pour DOS */
       char att;          /* attribut */
       char time[2];
       char date[2];
       long size;
       char fname[13];    /* nom fichier sous forme de ZSTRING */
};

struct entry {
       unsigned char att;
       char          str[13];
};

/* structure stockant les fichiers par page */
struct dir_lst {
       unsigned char   nfpp;  /* # de fichiers dans la page */
       struct entry    dirPage[NB_FILE_PER_PAGE];
       struct dir_lst *next;  /* page suivante   */
       struct dir_lst *prev;  /* page precedente */
};

/* liste chainee de tri des fichiers */
struct file_lst {
       struct entry file;
       struct file_lst *next;
       struct file_lst *prev;
};

#define FILE_SIZE sizeof(struct file_lst)


struct mouse_block_interface {
       unsigned cmask;
       unsigned buttonState;
       int      hCursor,
                vCursor,
                hMickey,
                vMickey;
};

#include "sconf.h"

struct varLink {
        int     signature;              /* indique si le vecteur est charg‚ */
        int     retCode;                /* code retour du Spawn */
        int     rebuild;                /* forcer la compile de tout le proj */
        int     recomp;                 /* forcer le compile du fichier courant */
        int     relink;                 /* forcer le link du fichier courant */
        int     result;                 /* resultat du make */
        int     spawnRet;               /* code retour logique de l'exe */
        unsigned segVideo;              /* segment video */
        int     adapter;                /* type de carte video */
        int     operation;              /* type d'op (COMP, MKE, ASS) */
        struct configuration conf;      /* configuration : PICK */
        char   fError[80];              /* nom fichier d'erreur */
        char   ferror_name[80];         /* nom fichier causant l'erreur */
        char   cdirname[80];            /* nom directorie courante */
        char   homedir[80];             /* directorie de demarrage */
        char   relname[80];             /* nom relatif */
        char   absfname[80];            /* nom de fic absolu */
        char   exename[80];             /* nom de l'exe … generer */
        char   currExe[80];             /* nom de l'EXE courant dans la session */
        char   currmask[80];            /* masque courant 5/92 */
        char   findStr[80];             /* string de recherche courante 5/92 */
        char   replStr[80];             /* string de replace courante 5/92 */

        /* ligne de Commande Noyau */
        char    lineCom[80];

        /* Variable Make , Comp , Ass, Browser */
        char   *lnkstr1;
        char   *lnkstr2;
        char   **spcc;
        char   **spasm;
        char   **splnk;
        int   (*Bfunc)(); /* fonction browser a lancer apres le build DB */
        int    narg;

        /* variables LINK */
        int    mkd_shell;
        int    mkd_FS;
        int    mkd_AutoAsk;

        /* variables ASS */
        int    mkd_tasm;
        int    mkd_masm;

        /* position souris */
        int     mouse;       /* souris presente ou non */
        int     mouX,
                mouY;

        /* attributs */
        int     WINSYSink;
        int     WINSYSpaper;
        int     WINSYSborder;
        int     WINSYSblock;
        int     WINmessInk;
        int     WINmessPaper;
        int     WINmessBorder;
        int     WINDIRink;
        int     WINDIRpaper;

};


