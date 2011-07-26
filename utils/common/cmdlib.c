/*
	cmdlib.c
	functions common to all of the utilities

	$Id$
*/


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#ifdef PLATFORM_DOS
#include <time.h>
#endif
#ifdef PLATFORM_UNIX
#include <sys/time.h>
#include <time.h>
#endif
#include <ctype.h>

// MACROS ------------------------------------------------------------------

#define PATHSEPERATOR	'/'

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int myargc;
char **myargv;

char		com_token[1024];
qboolean	com_eof;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// REPLACEMENTS FOR LIBRARY FUNCTIONS --------------------------------------

/*
==============
q_strlwr and q_strupr

==============
*/
char *q_strlwr (char *str)
{
	char	*c;
	c = str;
	while (*c)
	{
		*c = tolower(*c);
		c++;
	}
	return str;
}

char *q_strupr (char *str)
{
	char	*c;
	c = str;
	while (*c)
	{
		*c = toupper(*c);
		c++;
	}
	return str;
}

/*
==============
q_snprintf and q_vsnprintf

==============
*/
#if defined(__DJGPP__) &&	\
  (!defined(__DJGPP_MINOR__) || __DJGPP_MINOR__ < 4)
/* DJGPP < v2.04 doesn't have [v]snprintf().  */
/* to ensure a proper version check, include stdio.h
 * or go32.h which includes sys/version.h since djgpp
 * versions >= 2.02 and defines __DJGPP_MINOR__ */
#include "djlib/vsnprntf.c"
#endif	/* __DJGPP_MINOR__ < 4 */

int q_vsnprintf(char *str, size_t size, const char *format, va_list args)
{
	int		ret;

	ret = vsnprintf_func (str, size, format, args);

	if (ret < 0)
		ret = (int)size;

	if ((size_t)ret >= size)
		str[size - 1] = '\0';

	return ret;
}

int q_snprintf (char *str, size_t size, const char *format, ...)
{
	int		ret;
	va_list		argptr;

	va_start (argptr, format);
	ret = q_vsnprintf (str, size, format, argptr);
	va_end (argptr);

	return ret;
}

size_t qerr_strlcat (const char *caller, int linenum,
		     char *dst, const char *src, size_t size)
{
	size_t	ret = q_strlcat (dst, src, size);
	if (ret >= size)
		Error("%s: %d: string buffer overflow!", caller, linenum);
	return ret;
}

size_t qerr_strlcpy (const char *caller, int linenum,
		     char *dst, const char *src, size_t size)
{
	size_t	ret = q_strlcpy (dst, src, size);
	if (ret >= size)
		Error("%s: %d: string buffer overflow!", caller, linenum);
	return ret;
}

int qerr_snprintf (const char *caller, int linenum,
		   char *str, size_t size, const char *format, ...)
{
	int		ret;
	va_list		argptr;

	va_start (argptr, format);
	ret = q_vsnprintf (str, size, format, argptr);
	va_end (argptr);

	if ((size_t)ret >= size)
		Error("%s: %d: string buffer overflow!", caller, linenum);
	return ret;
}


// CODE --------------------------------------------------------------------

/*
==============
GetTime

==============
*/
#if 0
double GetTime (void)
{
	time_t	t;
	time(&t);
	return t;
}
#endif

#ifdef PLATFORM_WINDOWS
double GetTime (void)
{
/* http://www.codeproject.com/KB/datetime/winapi_datetime_ops.aspx
 * It doesn't matter that the offset is Jan 1, 1601, result
 * is the number of 100 nanosecond units, 100ns * 10,000 = 1ms. */
	SYSTEMTIME st;
	FILETIME ft;
	ULARGE_INTEGER ul1;

	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	ul1.HighPart = ft.dwHighDateTime;
	ul1.LowPart = ft.dwLowDateTime;

	return (double)ul1.QuadPart / 10000000.0;
}
#endif

#ifdef PLATFORM_DOS
double GetTime (void)
{
/* See  DJGPP uclock() man page for its limitations */
	return (double) uclock() / (double) UCLOCKS_PER_SEC;
}
#endif

#ifdef PLATFORM_UNIX
double GetTime (void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}
#endif

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
const char *COM_Parse (const char *data)
{
	int		c;
	int		len;
	qboolean	done = false;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

	do
	{
	// skip whitespace
		while ((c = *data) <= ' ')
		{
			if (c == 0)	// end of file
			{
				com_eof = true;
				return NULL;
			}
			data++;
		}

	// skip C style comments
		if (c == '/' && data[1] == '*')
		{
			data += 2;
			while (!(*data == '*' && data[1] == '/'))
			{
				if (*data == 0)	// end of file
				{
					com_eof = true;
					return NULL;
				}
				data++;
			}
			data += 2;
		}
	// skip C++ style comments
		else
		if (c == '/' && data[1] == '/')
		{
			while (*data && *data != '\n')
			{
				data++;
			}
		}
		else
		{
			done = true;
		}
	} while (done == false);

	// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		do
		{
			c = *data++;
			if (c == '\"')
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		} while (1);
	}

	// parse special characters
	if (c == '{' || c == '}' || c == '(' || c == ')' || c == '\'' || c == ':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

	// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
		if (c == '{' || c == '}' || c == '(' || c == ')' || c == '\'' || c == ':')
		{
			break;
		}
	} while (c > 32);

	com_token[len] = 0;
	return data;
}

/*
==============
Error

For abnormal program terminations.
==============
*/
void Error (const char *error, ...)
{
	va_list argptr;

	printf ("*** ERROR: ***\n");
	va_start (argptr, error);
	vprintf (error, argptr);
	va_end (argptr);
	printf ("\n\n");
	exit (1);
}

/*
==============
SafeMalloc
==============
*/
void *SafeMalloc (size_t size)
{
	void *ptr = calloc(1, size);
	if (!ptr)
		Error ("Malloc failed for %lu bytes.", (unsigned long)size);
	return ptr;
}

/*
==============
CheckParm

Checks for the given parameter in the program's command line arguments.
Returns the argument number (1 to argc-1) or 0 if not present.
==============
*/
int CheckParm (const char *check)
{
	int		i;

	for (i = 1; i < myargc; i++)
	{
		if ( !q_strcasecmp(check, myargv[i]) )
		{
			return i;
		}
	}
	return 0;
}

/*
==============
ParseNum / ParseHex
==============
*/
int ParseHex (const char *hex)
{
	const char	*str;
	int		num;

	num = 0;
	str = hex;

	while (*str)
	{
		num <<= 4;
		if (*str >= '0' && *str <= '9')
			num += *str-'0';
		else if (*str >= 'a' && *str <= 'f')
			num += 10 + *str-'a';
		else if (*str >= 'A' && *str <= 'F')
			num += 10 + *str-'A';
		else
			Error ("Bad hex number: %s",hex);
		str++;
	}

	return num;
}

int ParseNum (const char *str)
{
	if (str[0] == '$')
		return ParseHex (str+1);
	if (str[0] == '0' && str[1] == 'x')
		return ParseHex (str+2);
	return atoi (str);
}

