/* io.h */

#define    stdin      (int)0
#define    stdout     (int)1
#define    stderr     (int)2

#define    isbaddev(f)   ((f) < 0 || (f) >= NDEVS )

/* macros d'entree / sortie interactives */
#define    getchar()      m_Getc(0)
#define    sgetchar()     m_Sgetc(0)
#define    putchar(ch)    m_Putc(ch,1)
#define    fgetc(fd)      m_Getc(fd)
#define    fputc(ch,fd)   m_Putc(ch,fd)


