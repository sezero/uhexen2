// cmdlib.h

#ifndef __CMDLIB__
#define __CMDLIB__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum {false, true} qboolean;
typedef unsigned char byte;
#endif

// the dec offsetof macro doesn't work very well...
#define myoffsetof(type,identifier) ((size_t)&((type *)0)->identifier)


// set these before calling CheckParm
extern int myargc;
extern char **myargv;

#ifndef __GNUC__
#define Q_strncasecmp(s1,s2,n) strnicmp((s1),(s2),(n))
#define Q_strcasecmp(s1,s2) stricmp((s1),(s2))
#else
#define Q_strncasecmp(s1,s2,n) strncasecmp((s1),(s2),(n))
#define Q_strcasecmp(s1,s2) strcasecmp((s1),(s2))
#endif

#if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#define Q_stpcpy stpcpy
#else
char *Q_stpcpy (char *qdest, const char *qsrc);
#endif

int	Q_filelength (FILE *f);

double	I_FloatTime (void);

void	Error (char *error, ...);
int	CheckParm (char *check);

FILE	*SafeOpenWrite (char *filename);
FILE	*SafeOpenRead (char *filename);
void	SafeRead (FILE *f, void *buffer, int count);
void	SafeWrite (FILE *f, void *buffer, int count);
int	LoadFile (char *filename, void **bufferptr);

int 	ParseNum (char *str);

short	BigShort (short l);
short	LittleShort (short l);
int	BigLong (int l);
int	LittleLong (int l);
float	BigFloat (float l);
float	LittleFloat (float l);

char *COM_Parse (char *data);

extern	char		com_token[1024];
extern	qboolean	com_eof;

void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, byte data);
unsigned short CRC_Value(unsigned short crcvalue);

#endif
