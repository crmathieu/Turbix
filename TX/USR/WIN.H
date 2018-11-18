/* win.h */


/* miscellaneous */
#define  TITLE_SIZE    50
#define  WINBUFLEN     0xf0
#define  NULLWIN       (struct window *)0
#define  WINSIZE       sizeof(struct window)


/* control FLAGS */
#define      W_SCROLLON           1  /* enable scrolling */
#define      W_AUTOCRLF           2  /* CR LF on right border or truncate */
#define      W_FOREGRND           4  /* window that hold the focus */
#define      W_VISIBLE            8  /* the window is visible */
#define      W_GLOBALR           16  /* refresh the whole window buffer */
#define      W_ECHO             128  /* echo on input */
#define      W_SUBWIN           256  /* sub window */

struct window {
       int     _cury,_curx;             /* current relative X,Y window position */
       int     _maxy,_maxx;             /* line , width */
       int     _begy,_begx;             /* absolute window coordonnates (xg,yg) */
       int     _flags;                  /* control flags */
       unsigned  *_wbuff;               /* window buffer */
       int     _firstch;
       int     _lastch;
       unsigned  *_mask;

       unsigned char res1[24];          /* reserved */

       unsigned char paper;             /* background attribut */
       unsigned char ink;               /* foreground attribut */
       unsigned char Hborder;           /* horizontal character type */
       unsigned char Vborder;           /* vertical character type */
       int     ansiS;
       int     wsize;                   /* window size */
       unsigned char res2[WINBUFLEN+6]; /* reserved */
       char    title[TITLE_SIZE];       /* window title */
};

/* functions declarations */
int             cdecl    m_initscr(int ink, int paper);
int             cdecl    m_endwin(void);
struct window * cdecl    m_newwin(int lines, int cols, int begY, int begX,int ink, int paper, char *name);
struct window * cdecl    m_initStdscrPTR(void);
int             cdecl    m_wmove(struct window *pwin, int y, int x);
int             cdecl    m_wrefresh(struct window *pwin);
int             cdecl    m_wresize(struct window *pwin, int nlines, int ncols, int y, int x);
int             cdecl    m_wscroll(struct window *pwin);
void            cdecl    m_getyx(struct window *pwin, int *y, int *x);
int             cdecl    m_wselect(struct window *pwin);
int             cdecl    m_delwin(struct window *pwin);
int             cdecl    m_mvwin(struct window *pwin,int y, int x);
int             cdecl    m_wpush(struct window *pwin);
int             cdecl    m_wpop(struct window *pwin);
int             cdecl    m_wdelch(struct window *pwin);
int             cdecl    m_winsch(struct window *pwin, unsigned char ch);
void            cdecl    m_wdeleteln(struct window *pwin);
int             cdecl    m_winsertln(struct window *pwin);
void            cdecl    m_werase(struct window *pwin);
void            cdecl    m_wclrtobot(struct window *pwin);
void            cdecl    m_wclrtoeol(struct window *pwin);
int             cdecl    m_winch(struct window *pwin);
int             cdecl    m_wprintw(struct window *pwin,char *strfmt, ...);
int             cdecl    m_wscanw(struct window *pwin,char *strfmt, ...);
int             cdecl    m_wgetch(struct window *pwin);
int             cdecl    m_box(struct window *pwin, char Vchar, char Hchar);
void            cdecl    m_wautocrlf(struct window *pwin, int bool);
void            cdecl    m_wecho(struct window *pwin,int bool);
void            cdecl    m_touchwin(struct window *pwin);
