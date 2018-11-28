/* kbd.h */

/* positioning keys */
#define ARROWL  75
#define ARROWR  77
#define ARROWD  80
#define ARROWU  72
#define HOME    71
#define END     79
#define PGDN    81
#define PGUP    73

/* constantes relatives au clavier */
/* 1) constantes specific to special keys */
#define F1             0
#define F2             0
#define F3             0
#define F4             0
#define F5             0
#define F6             0
#define F7             0
#define F8             0
#define F9             0
#define F10            0
#define SHIFTD         0
#define SHIFTG         0
#define CAPS           0
#define NUMLOCK        0
#define CTRL           0
#define ALT            0
#define SCROLLLOCK     0
#define SYSREQ         0

/* 2) values used by keyboard state variable */
#define  KBD           1
#define  KBDCTRL       2
#define  KBDCTRALT     3
#define  KBDALT        4
#define  KBDSHIFT      5
#define  KBDALTCTRL    6

/* 3) pressing codes of special keys */
#define CTRLcod       29
#define ALTcod        56
#define SHIFTGcod     42
#define SHIFTDcod     54
#define NLOCKcod      69
#define CAPSLcod      58
#define Ccod          46
#define DELcod        83
#define TABcod	      15
#define F10cod        68
#define F1cod         59
#define SLOCKcod      70
#define RETURNcod     28

/* 4) releasing codes of special keys */
#define CTRLcodR     157
#define ALTcodR      184
#define SHIFTGcodR   170
#define SHIFTDcodR   182

/* variables linked to keyboard driver */
LOCAL shift_key = FALSE;
LOCAL shift_release = TRUE;
LOCAL ctrl_key = FALSE;

/* toggle key */
LOCAL unsigned numlock = 0;
LOCAL unsigned capslock = 0;

LOCAL kbd_state = KBD;

struct KEY {
	char minus;
	char majus;
	char numlck;
	char ctrl;
	char alt;
};

struct KEY keyTab[] = {

/* SCAN    	MINUS	   MAJUS      NUMLCK  CTRL	ALTER	 */

/* 0  */	{0,         0,         0,      0,  	0        },
/* 1  */	{'\x1B',    '\x1B',    0,      0,  	'\x1B'   },
/* 2  */	{'&',       '1',       0,      0,  	0        },
/* 3  */	{'�',       '2',       0,      0,  	'@'      },
/* 4  */	{'"',       '3',       0,      0,  	'#'      },
/* 5  */	{'\x27',    '4',       0,      0,  	0        },
/* 6  */	{'(',       '5',       0,      0,  	0        },
/* 7  */	{'|',       '6',       0,      0,  	'^'      },
/* 8  */	{' ',       '7',       0,      0,  	0        },
/* 9  */	{'!',       '8',       0,      0,  	0        },
/* A  */	{'�',       '9',       0,      0,  	0        },
/* B  */	{'�',       '0',       0,      0,  	0        },
/* C  */	{')',       ')',       0,      0,  	0        },
/* D  */	{'-',       '_',       0,      0,  	0        },

/* SCAN    	MINUS	   MAJUS      NUMLCK  CTRL	ALTER	 */

/* E  */	{'\b',      '\b',      0,      0,  	0        },
/* F  */	{'\t',      '\t',      0,      0,  	0        },
/* 10 */	{'a',       'A',       0,      '\x01', 	0        },
/* 11 */	{'z',       'Z',       0,      '\x1A', 	0        },
/* 12 */	{'e',       'E',       0,      '\x05', 	0        },
/* 13 */	{'r',       'R',       0,      '\x12', 	0        },
/* 14 */	{'t',       'T',       0,      '\x14', 	0        },
/* 15 */	{'y',       'Y',       0,      '\x19', 	0        },
/* 16 */	{'u',       'U',       0,      '\x15', 	0        },
/* 17 */	{'i',       'I',       0,      '\x09', 	0        },
/* 18 */	{'o',       'O',       0,      '\x0f', 	0        },
/* 19 */	{'p',       'P',       0,      '\x10', 	0        },
/* 1A */	{'^',       '{',       0,      0, 	'['      },
/* 1B */	{'$',       '*',       0,      0, 	']'      },

/* SCAN    	MINUS	   MAJUS      NUMLCK  CTRL	ALTER	 */

/* 1C */	{'\x0D',    '\x0D',    0,      0, 	'\x0D'   },
/* 1D */	{CTRL,      CTRL,      0,      0, 	CTRL     },
/* 1E */	{'q',       'Q',       0,      '\x11', 	0        },
/* 1F */	{'s',       'S',       0,      '\x13', 	0        },
/* 20 */	{'d',       'D',       0,      '\x04', 	0        },
/* 21 */	{'f',       'F',       0,      '\x06', 	0        },
/* 22 */	{'g',       'G',       0,      '\x07', 	0        },
/* 23 */	{'h',       'H',       0,      '\x08', 	0        },
/* 24 */	{'j',       'J',       0,      '\x0A', 	0        },
/* 25 */	{'k',       'K',       0,      '\x0B', 	0        },
/* 26 */	{'l',       'L',       0,      '\x0C', 	0        },
/* 27 */	{'m',       'M',       0,      '\x0D', 	0        },
/* 28 */	{'~',       '�',       0,      0, 	0        },
/* 29 */  	{'<',       '>',       0,      0, 	'\\'     },

/* SCAN    	MINUS	   MAJUS      NUMLCK  CTRL	ALTER	 */

/* 2A */	{SHIFTG,    SHIFTG,    0,      0, 	SHIFTG   },
/* 2B */	{'�',       '�',       0,      0, 	0        },
/* 2C */	{'w',       'W',       0,      '\x17', 	0        },
/* 2D */	{'x',       'X',       0,      '\x18', 	0        },
/* 2E */	{'c',       'C',       0,      '\x03', 	0        },
/* 2F */	{'v',       'V',       0,      '\x16', 	0        },
/* 30 */	{'b',       'B',       0,      '\x02', 	0        },
/* 31 */	{'n',       'N',       0,      '\x0E', 	0        },
/* 32 */	{',',       '?',       0,      0, 	0        },
/* 33 */	{';',       '.',       0,      0, 	0        },
/* 34 */	{':',       '/',       0,      0, 	0        },
/* 35 */	{'=',       '+',       0,      0, 	0        },
/* 36 */	{SHIFTD,    SHIFTD,    0,      0, 	SHIFTD   },
/* 37 */	{'*',       '*',       0,      0, 	0        },

/* SCAN    	MINUS	   MAJUS      NUMLCK  CTRL	ALTER	 */

/* 38 */	{ALT,       ALT,       0,      0, 	ALT      },
/* 39 */	{' ',       ' ',       0,      0, 	0        },
/* 3A */	{CAPS,      CAPS,      0,      0, 	CAPS     },
/* 3B */	{F1,        F1,        0,      0, 	F1       },
/* 3C */	{F2,        F2,        0,      0, 	F2       },
/* 3D */	{F3,        F3,        0,      0, 	F3       },
/* 3E */	{F4,        F4,        0,      0, 	F4       },
/* 3F */	{F5,        F5,        0,      0, 	F5       },
/* 40 */	{F6,        F6,        0,      0, 	F6       },
/* 41 */	{F7,        F7,        0,      0, 	F7       },
/* 42 */	{F8,        F8,        0,      0, 	F8       },
/* 43 */	{F9,        F9,        0,      0, 	F9       },
/* 44 */	{F10,       F10,       0,      0, 	F10      },
/* 45 */	{NUMLOCK,   NUMLOCK,   0,      0, 	NUMLOCK  },

/* SCAN    	MINUS	   MAJUS      NUMLCK  CTRL	ALTER	 */

/* 46 */	{SCROLLLOCK,SCROLLLOCK,0,      0, 	SCROLLLOCK},
/* 47 */	{71,        71,        '7',    0, 	0,       },
/* 48 */	{72,        72,        '8',    0, 	0,       },
/* 49 */	{73,        73,        '9',    0, 	0,       },
/* 4A */	{'-',       '-',       '-',    0, 	0,       },
/* 4B */	{75,        75,        '4',    0, 	0,       },
/* 4C */	{ 0,         0,        '5',    0, 	0,       },
/* 4D */	{77,        77,        '6',    0, 	0,       },
/* 4E */	{'+',       '+',       '+',    0, 	0,       },
/* 4F */	{79,        79,        '1',    0, 	0,       },
/* 50 */	{80,        80,        '2',    0, 	0,       },
/* 51 */	{81,        81,        '3',    0, 	0,       },
/* 52 */	{82,        82,        '0',    0, 	0,       },
/* 53 */	{83,	   83,        '.',    0, 	0,       },
/* 54 */	{SYSREQ,    SYSREQ,    0,      0, 	0,       }
};


