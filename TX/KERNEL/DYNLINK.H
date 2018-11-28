/* dynamic link with window manager */
#define  WIN_SELEC     0      /* wselect  */
#define  WIN_BOX       1      /* box      */
#define  WIN_TOUCH     2      /* wtouch   */
#define  WIN_PERR      3      /* _werror  */
#define  WIN_GERR      4      /* _geterr  */
#define  WIN_GETCH     5      /* getch    */
#define  WIN_ADDCH     6      /* waddch   */

#define  NWINFUNC      7

extern int (* winfunc[])();