/* termio.h */

#define NCC 8      /* nb de caracteres speciaux */

/* offset des valeurs des caracteres de controle dans c_cc[] */

#define  C_INTR    0
#define  C_QUIT    1
#define  C_ERASE   2
#define  C_KILL    3
#define  C_EOF     4
#define  C_EOL     5
#define  C_UN1     6
#define  C_UN2     7     /* les 2 derniers elements sont inutilises */

struct termio {
       unsigned short c_iflag;    /* modes d'entrees                    */
       unsigned short c_oflag;    /* modes de sorties                   */
       unsigned short c_cflag;    /* modes de controles                 */
       unsigned short c_lflag;    /* modes locaux de comportement       */
       char           c_line;     /* comportement                       */
       char           c_cc[NCC];  /* caracteres de controle ou speciaux */
};

/* modes d'entrees (c_iflag) */

#define ISTRIP   0000040       /* compacter caractere a 7 bits */
#define INLCR    0000100       /* en entree NEWLINE -> RETURN */
#define IGNCR    0000200       /* en entree RETURN ignore */
#define ICRNL    0000400       /* en entree RETURN -> NEWLINE */
#define IUCLC    0001000       /* en entree MAJUSCULE -> MINUSCULE */
#define IXON     0002000       /* autorise le controle de flux */
#define IXANY    0004000       /* reprise du flux par un caractere quelconque*/

/* modes de sorties (c_oflag) */

#define OLCUC    0000002       /* en sortie MINUSCULE -> MAJUSCULE */
#define ONLCR    0000004       /* en sortie NEWLINE -> NEWLINE/RETURN */
#define OCRNL    0000010       /* en sortie RETURN -> NEWLINE */

/* modes de controle (c_cflag) */

#define B110     0000003   /* 110 bauds                                    */
#define B300     0000007   /* 300 bauds                                    */
#define B1200    0000011   /* 1200 bauds                                   */
#define B4800    0000014   /* 4800 bauds                                   */
#define B9600    0000015   /* 9600 bauds                                   */
#define EXTA     0000016   /* 19200 bauds                                  */
#define EXTB     0000017   /* vitesse fonction des valeurs des switches    */
#define CS6      0000020   /* caracteres sur 6 bits                        */
#define CS7      0000040   /* caracteres sur 7 bits                        */
#define CS8      0000060   /* caracteres sur 8 bits                        */
#define CSTOPB   0000100   /* si positionne -> 2 bits STOP, 1 sinon        */
#define CREAD    0000200   /* possibilite de recevoir                      */
#define PARENB   0000400   /* controle de parite                           */
#define PARODD   0001000   /* si positionne -> parite IMPAIRE, paire sinon */
#define CLOCAL   0004000   /* ligne locale ou ligne modem                  */

/* modes locaux de comportement */

#define ISIG     0000001   /* tout caractere tape est compare a ^C et QUIT */
#define ICANON   0000002   /* mode canonique                               */
#define ECHO     0000010   /* echo                                         */
#define ECHOE    0000020   /* l'echo de erase est : <CTRL H><SP><CRTL H>   */
#define ECHOK    0000040   /* l'echo de <KILL> est <NEWLINE>               */
#define ECHONL   0000100   /* echo de <NEWLINE>                            */
#define NOFLSH   0000200   /* pas de vidage des tampons d'E/S si interrupt */

/* parametres utilises par IOCTL */

#define TCGETA   0x0001   /* get parameters dans structure termio */
#define TCSETA   0x0002	  /* set "	"	"	"	  */
#define TCRAWM   0x0004	  /* enter raw mode			  */
#define TCCANM   0x0008	  /* enter canonical mode		  */
#define TCECHO   0x0010	  /* validate echo mode			  */
#define TCNOECHO 0x0012	  /* invalidate echo mode 		  */
#define TCICNT   0x0014	  /* nombre de caracteres en input	  */
#define TCICPY   0x0018	  /* dupliquer les caracteres en input    */

/*#define TCSETAW  0x0003   /* Idem TCSETA, mais la mise a jour est differee
                           * jusqu'a la fin de l'ecoulement des caracteres
                           * en cours de sortie
                           */