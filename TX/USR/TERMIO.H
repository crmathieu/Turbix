/* termio.h */

#define NCC 8      /* # special characters */

/* c_cc[] control characters offset */
#define  C_INTR    0
#define  C_QUIT    1
#define  C_ERASE   2
#define  C_KILL    3
#define  C_EOF     4
#define  C_EOL     5
#define  C_UN1     6
#define  C_UN2     7

struct termio {
       unsigned short c_iflag;    /* input mode */
       unsigned short c_oflag;    /* output mode */
       unsigned short c_cflag;    /* control mode */
       unsigned short c_lflag;    /* local behavior */
       char           c_line;     /* behavior */
       char           c_cc[NCC];  /* control character or special char */
};

/* input modes (c_iflag) */
#define ISTRIP   0000040       /* compacter caractere a 7 bits */
#define INLCR    0000100       /* input NEWLINE -> RETURN */
#define IGNCR    0000200       /* ignore input RETURN */
#define ICRNL    0000400       /* input RETURN -> NEWLINE */
#define IUCLC    0001000       /* input MAJUSCULE -> MINUSCULE */
#define IXON     0002000       /* enable stream control */
#define IXANY    0004000       /* resume stream with character */

/* output modes (c_oflag) */
#define OLCUC    0000002       /* output MINUSCULE -> MAJUSCULE */
#define ONLCR    0000004       /* output NEWLINE -> NEWLINE/RETURN */
#define OCRNL    0000010       /* output RETURN -> NEWLINE */

/* control modes (c_cflag) */
#define B110     0000003   /* 110 bauds */
#define B300     0000007   /* 300 bauds */
#define B1200    0000011   /* 1200 bauds */
#define B4800    0000014   /* 4800 bauds */
#define B9600    0000015   /* 9600 bauds */
#define EXTA     0000016   /* 19200 bauds */
#define EXTB     0000017   /* speed rate according to switches value */
#define CS6      0000020   /* 6 bits characters */
#define CS7      0000040   /* 7 bits characters */
#define CS8      0000060   /* 8 bits characters */
#define CSTOPB   0000100   /* ON : 2 STOP bits , OFF : 1 STOP bit */
#define CREAD    0000200   /* receipt enable */
#define PARENB   0000400   /* parity checking */
#define PARODD   0001000   /* ON : odd parity, OFF : even parity */
#define CLOCAL   0004000   /* ON : local line, OFF : modem line */

/* local behavior */
#define ISIG     0000001   /* check ^C and QUIT characters */
#define ICANON   0000002   /* canonical mode */
#define ECHO     0000010   /* echo */
#define ECHOE    0000020   /* erase echo : <CTRL H><SP><CRTL H> */
#define ECHOK    0000040   /* kill echo :  <NEWLINE> */
#define ECHONL   0000100   /* <NEWLINE> echo */
#define NOFLSH   0000200   /* disable buffer flushing if interrupt */

/* IOCTL const */
#define TCGETA   0x0001   /* get parameters from termio structure */
#define TCSETA   0x0002   /* set "      "    to         "         */
#define TCRAWM   0x0004   /* enter raw mode */
#define TCCANM   0x0008   /* enter canonical mode */
#define TCECHO   0x0010   /* validate echo mode */
#define TCNOECHO 0x0012   /* invalidate echo mode */
#define TCICNT   0x0014   /* # input characters */
#define TCICPY   0x0018   /* duplicate input characters */

