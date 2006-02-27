/*
	cmdlib.h

	$Header: /home/ozzie/Download/0000/uhexen2/utils/h2mp_utils/hcc/cmdlib.h,v 1.6 2006-02-27 00:02:59 sezero Exp $
*/

#ifndef __CMDLIB__
#define __CMDLIB__

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <setjmp.h>


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
void	COM_ParseError(char *error, ...);
void	COM_ParseWarning(char *error, ...);
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
int		COM_Hash(char *string);

#endif
