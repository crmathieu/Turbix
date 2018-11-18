/* const.h */


/* hardware interrup vectors */
#define CLOCK_VECTOR        8
#define HARDD_VECTOR       13
#define KEYBOARD_VECTOR     9
#define FLOPPY_VECTOR      14
#define PRINTER_VECTOR     15
#define SYS_VECTOR         32

#define XENKBD              9     /* it clavier                  */
#define XENCOM             12     /* it communication            */
#define DOSCLOCK           27     /* it 1Bh geree par DOS        */
/*#define XENCLOCK           28     /* it 1Ch appelee par DOSCLOCK */
#define XENCLOCK            8
#define XENDIV0             0     /* it division par 0           */
#define XENOVF              4     /* it overflow                 */
#define XENILL              6     /* it illegale instruction     */

#define MAJOR               8
#define MINOR               0
#define ONE_BLOCK           1
#define NOTHING             0
#define WORKP              -1
#define GRAFIC1             0xB8000000
#define TEXTE               0
#define GRAFIC              1

#define HOUR                0
#define MINUT               1
#define SECOND              2
#define YEAR                0
#define MONTH               1
#define DAY                 0
#define COLUMNS             4
#define SZBUF               20
#define C8255               0x61       /* port B du 8255 */
#define KEYBD               0x60       /* port de controle du 8048 */


/* flag bits for i_mode in the inode */
#define      EMPTY       -1
#define      LOWBYTE     0377
#define      HIBYTE      0177400
#define      LOW16       0x0000ffff
#define      HI16        0xffff0000

/* MACRO couramment utilisees */
#define isbadfd(x)      (x<0 || x>=NFD ? TRUE : FALSE)
#define isfileptr(x)    (x<&streamtab[NSTREAM] && x>=&streamtab[0] ? TRUE:FALSE)
