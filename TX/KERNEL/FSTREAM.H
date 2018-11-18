/* fs/stream.h */

#define NSTREAM           64         /* # de slot dans la stream table */
#define NULLSTREAM        (stream_entry  *)0
#define FILESTREAM        -1
#define PIPESTREAM        -2
#define TTYSTREAM         -3

extern stream_entry  Streamtab[];
extern stream_entry  *_getstream();

