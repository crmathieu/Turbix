/* io.h */


#define    stdin      (int)0
#define    stdout     (int)1
#define    stderr     (int)2

/* I/O macros */
#define    getchar()      m_Getc(0)
#define    sgetchar()     m_Sgetc(0)
#define    putchar(ch)    m_Putc(ch,1)
#define    fgetc(fd)      m_Getc(fd)
#define    fputc(ch,fd)   m_Putc(ch,fd)

/* I/O functions declarations */
int  cdecl      m_Creat(char *path, int accessmode);
int  cdecl      m_Pipe(int *fd);
long cdecl      m_Tell(int fd);
long cdecl      m_Lseek(int fd, long offset, int origin);
int  cdecl      m_Read (int fd, char *buff, int count);
int  cdecl      m_Getc(int fd);
int  cdecl      m_Sgetc(int fd);
int  cdecl      m_Getch(void);
int  cdecl      m_Getche(void);
int  cdecl      m_Sgetch(int fd);
void cdecl      m_Puts(char *str);
int  cdecl      m_Putc(char ch, int fd);
int  cdecl      m_Write(int fd, char *buff, int count);
int  cdecl      m_Close(int fd);
int  cdecl      m_Open(char *name, int mode);
int  cdecl      m_Dup2(int fd1, int fd2);
int  cdecl      m_Dup(int fd);
int  cdecl      m_Lock(int fd);
int  cdecl      m_Unlock(int fd);
int  cdecl      m_Ioctl(int handle, int cmd, struct termio *t);
int  cdecl      m_Mkdir(char *path);
int  cdecl      m_Setdrv(int dev);
int  cdecl      m_Chdir(char *dirname);
int  cdecl      m_SetErrHandler(int (*userHandler)());
int  cdecl      m_Getcwd(char *path);
