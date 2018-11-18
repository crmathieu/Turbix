/* video.h */

/*--------------------- attribut character code -----------------------------------
 *
 *     7        6         5         4          3        2          1       0
 * Blinking  Red-Back green-back blue-back Intensity Red-fore green-fore blue-fore
 *---------------------------------------------------------------------------------
 */

extern int adapter;

#define  BLINK         0x80
#define  INTENSITY     0x08

/* foreground attributs */
#define  F_BLACK        0
#define  F_WHITE        0x07
#define  F_RED          (adapter? 0x04:0x07)
#define  F_GREEN        (adapter? 0x02:0x07)
#define  F_BLUE         (adapter? 0x01:0x07)
#define  F_CYAN         (adapter? 0x03:0x07)
#define  F_MAGENTA      (adapter? 0x05:0x07)
#define  F_YELLOW       (adapter? 0x06:0x07)
#define  F_LBLUE        (INTENSITY|F_BLUE)
#define  F_LRED         (INTENSITY|F_RED)
#define  F_LGREEN       (INTENSITY|F_GREEN)
#define  F_LCYAN        (INTENSITY|F_CYAN)
#define  F_LMAGENTA     (INTENSITY|F_MAGENTA)
#define  F_LYELLOW      (INTENSITY|F_YELLOW)
#define  F_LWHITE       (INTENSITY|F_WHITE)

/* background attributs */
#define  B_BLACK        0
#define  B_WHITE        0x70
#define  B_RED          (adapter? 0x40:0)
#define  B_GREEN        (adapter? 0x20:0)
#define  B_BLUE         (adapter? 0x10:0)
#define  B_CYAN         (adapter? 0x30:0)
#define  B_MAGENTA      (adapter? 0x50:0)
#define  B_YELLOW       (adapter? 0x60:0)


/* ANSI codes attributs */
#define    ANSI_F_BLACK         30
#define    ANSI_F_RED           31
#define    ANSI_F_GREEN         32
#define    ANSI_F_YELLOW        33
#define    ANSI_F_BLUE          34
#define    ANSI_F_MAGENTA       35
#define    ANSI_F_CYAN          36
#define    ANSI_F_WHITE         37

#define    ANSI_B_BLACK         40
#define    ANSI_B_RED           41
#define    ANSI_B_GREEN         42
#define    ANSI_B_YELLOW        43
#define    ANSI_B_BLUE          44
#define    ANSI_B_MAGENTA       45
#define    ANSI_B_CYAN          46
#define    ANSI_B_WHITE         47


#define    ANSI_MONO_REVERSE     7
#define    ANSI_MONO             0

/* cursor shape */
#define CURS_HIDE      32
#define CURS_SMALL      0
#define CURS_LARGE      1

/* functions declarations */
int cdecl       m_Gotoxy(int cdecl x, int cdecl y);
int cdecl       m_Getpos(int  * cdecl x, int  * cdecl y);
int cdecl       m_Setcva(unsigned char newva);
int cdecl       m_CursorShape(int cdecl aspect);
int cdecl       m_Beep(int cdecl freq);
int cdecl       m_GetSession(void);
int cdecl       m_ChgActiveSession(int cdecl sessionNumber);
int cdecl       m_GetProcSessionHandle(int cdecl pid);
int cdecl       m_Flush(void);

