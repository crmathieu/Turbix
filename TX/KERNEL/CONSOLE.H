/* console.h */


#define  C8255    0x61               /* port B du 8255  */

/* Definition utilise pour le controleur driver */
#define COLOR_BASE      0xb800    /* base memoire ecran couleur */
#define MONO_BASE       0xb000    /* base memoire ecran monochrome */
#define C_VID_MASK      0x3fff    /* masque video RAM 16k */
#define M_VID_MASK      0x01ff    /* masque video RAM 4k */
#define C_RETRACE       0x0300
#define M_RETRACE       0x7000
#define WORD_MASK       0xffff    /* masque pour 16 bits */
#define OFF_MASK        0x000f    /* masque pour 4 bits */
#define BEEP_FREQ       0x0533
#define B_TIME          0x2000
#define LINE_WIDTH          80
#define SCR_LINES           25
#define CTRL_S              31
#define MONOCHROME           1
/*#define GO_FORWARD           0
#define GO_BACKWARD          1*/
#define TIMER2            0x42
#define TIMER3            0x43

/* keyboard */
#define KEYBD             0x60  /* Kbd data IN */
#define KBD_IN		  0x60  /* "    "   "  */
#define PORT_B            0x61
#define KBD_CTL		  0x61  /* control bits */
#define KBIT              0x80
#define KBD_ERR		  0x80  /* Kbd transmit error flag */

#define KBD_LED		  0x0ED /* commande d'ecriture LED clavier */


#define PAGE_SIZE         4000

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

/*--------------------- codes des attributs caracteres ----------------------------
 *
 *     7        6         5         4          3        2          1       0
 * Blinking  Red-Back green-back blue-back Intensity Red-fore green-fore blue-fore
 *---------------------------------------------------------------------------------
 */

#define  BLINK         0x80
/*#define  B_RED         0x40
#define  B_GREEN       0x20
#define  B_BLUE        0x10
#define  B_BLACK       0
#define  INTENSITY     0x08
#define  F_RED         0x04
#define  F_GREEN       0x02
#define  F_BLUE        0x01*/
#define  F_BLACK       0

#define  B_RED         (adapter? 0x40:0)
#define  B_GREEN       (adapter? 0x20:0)
#define  B_BLUE        (adapter? 0x10:0)
#define  B_BLACK       0
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

/*#define  B_CYAN         (B_GREEN|B_BLUE)          /* vert     bleu   */
/*#define  B_MAGENTA      (B_RED|B_BLUE)
/*#define  B_YELLOW       (B_RED|B_GREEN)           /* jaune    fonc‚  */

/* valeur des codes ANSI pour les attributs F & B */
#define    ANSI_F_BLACK		30
#define    ANSI_F_RED           31
#define    ANSI_F_GREEN		32
#define    ANSI_F_YELLOW	33
#define    ANSI_F_BLUE		34
#define    ANSI_F_MAGENTA	35
#define    ANSI_F_CYAN		36
#define    ANSI_F_WHITE		37

#define    ANSI_B_BLACK		40
#define    ANSI_B_RED		41
#define    ANSI_B_GREEN		42
#define    ANSI_B_YELLOW	43
#define    ANSI_B_BLUE		44
#define    ANSI_B_MAGENTA	45
#define    ANSI_B_CYAN		46
#define    ANSI_B_WHITE		47

#define	   ANSI_MONO_REVERSE	 7
#define	   ANSI_MONO		 0




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

#define NPAGES  5
#define WORKP  -1

#define MONO    0
#define CGA     1
#define EGA     2

/* Variables globales */
extern int adapter;
extern unsigned vid_retrace;
extern unsigned vid_base;
extern unsigned vid_mask;
extern unsigned vid_port;

