// cmdlib.h


#ifndef __CMDLIB_H__
#define __CMDLIB_H__

// HEADER FILES ------------------------------------------------------------

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
// for strcasecmp and strncasecmp
#include <strings.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>


// TYPES -------------------------------------------------------------------

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum { false, true } qboolean;
typedef unsigned char byte;
#endif


// PUBLIC DATA DECLARATIONS ------------------------------------------------

// set these before calling CheckParm
extern int myargc;
extern char **myargv;

extern char com_token[1024];
extern qboolean com_eof;


// MACROS ------------------------------------------------------------------

#ifdef _WIN32
#define Q_strncasecmp	strnicmp
#define Q_strcasecmp	stricmp
#else
#define Q_strncasecmp	strncasecmp
#define Q_strcasecmp	strcasecmp
#endif


// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

int	Q_filelength(FILE *f);

double	GetTime(void);

void	Error (char *error, ...);
int		CheckParm(char *check);

FILE	*SafeOpenWrite(char *filename);
FILE	*SafeOpenRead(char *filename);
void	SafeRead(FILE *f, void *buffer, int count);
void	SafeWrite(FILE *f, void *buffer, int count);

int		LoadFile(char *filename, void **bufferptr);

char	*COM_Parse(char *data);

void	CRC_Init(unsigned short *crcvalue);
void	CRC_ProcessByte(unsigned short *crcvalue, byte data);
unsigned short	CRC_Value(unsigned short crcvalue);

#define	HASH_TABLE_SIZE		9973
int		COM_Hash(char *string);

// endianness stuff: <sys/types.h> is supposed
// to succeed in locating the correct endian.h
// this BSD style may not work everywhere, eg. on WIN32
#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
#undef BYTE_ORDER
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#endif
// assumptions in case we don't have endianness info
#ifndef BYTE_ORDER
#if defined(_WIN32)
#define BYTE_ORDER LITTLE_ENDIAN
#define GUESSED_WIN32_ENDIANNESS
#else
#if defined(SUNOS)
// these bits from darkplaces project
#define GUESSED_SUNOS_ENDIANNESS
#if defined(__i386) || defined(__amd64)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#else
#define ASSUMED_LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif
#endif

short	ShortSwap (short);
int	LongSwap (int);
float	FloatSwap (float);

#if BYTE_ORDER == BIG_ENDIAN
#define BigShort(s) (s)
#define LittleShort(s) ShortSwap((s))
#define BigLong(l) (l)
#define LittleLong(l) LongSwap((l))
#define BigFloat(f) (f)
#define LittleFloat(f) FloatSwap((f))
#else
// BYTE_ORDER == LITTLE_ENDIAN
#define BigShort(s) ShortSwap((s))
#define LittleShort(s) (s)
#define BigLong(l) LongSwap((l))
#define LittleLong(l) (l)
#define BigFloat(f) FloatSwap((f))
#define LittleFloat(f) (f)
#endif

// end of endianness stuff

#endif	/* __CMDLIB_H__	*/

