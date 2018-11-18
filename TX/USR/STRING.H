/* string.h */

void           *cdecl memccpy (void *dest, void *src, int c, unsigned n);
void           *cdecl memchr  (void *s, int c, unsigned n);
int             cdecl memcmp  (void *s1, void *s2, unsigned n);
void           *cdecl memcpy  (void *dest, void *src, unsigned n);
int             cdecl memicmp (void *s1, void *s2, unsigned n);
void           *cdecl memmove (void *dest, void *src, unsigned n);
void           *cdecl memset  (void *s, int c, unsigned n);
void            cdecl movedata(unsigned srcseg, unsigned srcoff, unsigned dstseg,
                               unsigned dstoff, unsigned n);
char           *cdecl stpcpy  (char *dest, char *src);
char           *cdecl strcat  (char *dest, char *src);
char           *cdecl strchr  (char *s, int c);
int             cdecl strcmp  (char *s1, char *s2);
char           *cdecl strcpy  (char *dest, char *src);
unsigned        cdecl strcspn (char *s1, char *s2);
char           *cdecl strdup  (char *s);
char           *cdecl strerror(int errnum);
int             cdecl stricmp (char *s1, char *s2);
unsigned        cdecl strlen  (char *s);
char           *cdecl strlwr  (char *s);
char           *cdecl strncat (char *dest, char *src, unsigned maxlen);
int             cdecl strncmp (char *s1, char *s2, unsigned maxlen);
char           *cdecl strncpy (char *dest, char *src, unsigned maxlen);
int             cdecl strnicmp(char *s1, char *s2, unsigned maxlen);
char           *cdecl strnset (char *s, int ch, unsigned n);
char           *cdecl strpbrk (char *s1, char *s2);
char           *cdecl strrchr (char *s, int c);
char           *cdecl strrev  (char *s);
char           *cdecl strset  (char *s, int ch);
unsigned        cdecl strspn  (char *s1, char *s2);
char           *cdecl strstr  (char *s1, char *s2);
char           *cdecl strtok  (char *s1, char *s2);
char           *cdecl strupr  (char *s);

