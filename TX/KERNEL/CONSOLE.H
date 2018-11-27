/* console.h */

#define C8255           0x61               /* 8255 B port */

/* Definitions used by the video driver */
#define COLOR_BASE      0xb800    /* color screen base memory */
#define MONO_BASE       0xb000    /* monochrome screen base memory */
#define C_VID_MASK      0x3fff    /* RAM 16k video mask */
#define M_VID_MASK 0x01ff         /* RAM 4k video mask */
#define C_RETRACE       0x0300
#define M_RETRACE       0x7000
#define WORD_MASK       0xffff    /* 16 bits mask */
#define OFF_MASK        0x000f    /* 4 bits mask */
#define BEEP_FREQ       0x0533
#define B_TIME          0x2000
#define LINE_WIDTH          80
#define SCR_LINES           25
#define CTRL_S              31
#define MONOCHROME           1
#define TIMER2            0x42
#define TIMER3            0x43

/* keyboard */
#define KEYBD             0x60  /* Kbd data IN */
#define KBD_IN		      0x60  /* "    "   "  */
#define PORT_B            0x61
#define KBD_CTL		      0x61  /* control bits */
#define KBIT              0x80
#define KBD_ERR		      0x80  /* Kbd transmit error flag */

#define KBD_LED		      0x0ED /* write LED */

#define PAGE_SIZE         4000

/* definitions specific to the 6845 controller */
#define M_6845           0x3b0
#define C_6845           0x3d0
#define MODE                 8
#define CONF                15
#define INDEX                4
#define DATA                 5
#define CUR_SIZE            10
#define VID_ORG             12
#define CURSOR              14

/*--------------------- character attribut codes ----------------------------------
 *
 *     7        6         5         4          3        2          1       0
 * Blinking  Red-Back green-back blue-back Intensity Red-fore green-fore blue-fore
 *---------------------------------------------------------------------------------
 */

#define  BLINK         0x80
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
#define  F_CYAN         (adapter? 0x03:0x07)      /* green    blue   */
#define  F_MAGENTA      (adapter? 0x05:0x07)
#define  F_YELLOW       (adapter? 0x06:0x07)      /* yellow   dark   */
#define  F_LBLUE        (INTENSITY|F_BLUE)        /* blue     light  */
#define  F_LRED         (INTENSITY|F_RED)         /* red      light  */
#define  F_LGREEN       (INTENSITY|F_GREEN)       /* green    light  */
#define  F_LCYAN        (INTENSITY|F_CYAN)        /* cyan     light  */
#define  F_LMAGENTA     (INTENSITY|F_MAGENTA)     /* magenta  light  */
#define  F_LYELLOW      (INTENSITY|F_YELLOW)      /* yellow   light  */
#define  F_LWHITE       (INTENSITY|F_WHITE)       /* white    bright */

/* Background */
#define  B_WHITE        0x70
#define  B_CYAN         (adapter ? 0x30:0)
#define  B_MAGENTA      (adapter ? 0x50:0)
#define  B_YELLOW       (adapter ? 0x60:0)

/* Foreground & Background attributs ANSI codes */
#define    ANSI_F_BLACK		30
#define    ANSI_F_RED       31
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

#define	   ANSI_MONO_REVERSE 7
#define	   ANSI_MONO		 0

/* cursor */

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

/* Global Variables */
extern int adapter;
extern unsigned vid_retrace;
extern unsigned vid_base;
extern unsigned vid_mask;
extern unsigned vid_port;

