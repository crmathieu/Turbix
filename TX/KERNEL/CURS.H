/* curs.h */

/* macros */
#define      addch(ch)          waddch(stdscr,ch)
#define      getch()            wgetch(stdscr)
#define      addstr(str)        waddstr(stdscr,str)
#define      getstr(str)        wgetstr(stdscr,str)
#define      move(y,x)          wmove(stdscr,y,x)
#define      clear()            wclear(stdscr)
#define      erase()            werase(stdscr)
#define      clrtobot()         wclrtobot(stdscr)
#define      clrtoeol()         wclrtoeol(stdscr)
#define      insertln()         winsertln(stdscr)
#define      deleteln()         wdeleteln(stdscr)
#define      refresh()          wrefresh(stdscr)
#define      inch()             winch(stdscr)
#define      insch(ch)          winsch(stdscr,ch)
#define      delch()            wdelch(stdscr)
#define      standout()         wstandout(stdscr)
#define      standend()         wstandend(stdscr)
