/*
 * cmdlib.c -- functions common to all of the utilities
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "q_ctype.h"
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
#ifdef PLATFORM_AMIGA
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <time.h>
#endif

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int myargc;
char **myargv;

char		com_token[1024];

// PRIVATE DATA DEFINITIONS ------------------------------------------------

#ifdef PLATFORM_AMIGA
struct timerequest	*timerio;
struct MsgPort		*timerport;
#ifdef __MORPHOS__
struct Library		*TimerBase;
#else
struct Device		*TimerBase;
#endif
#endif /*  _AMIGA */

// REPLACEMENTS FOR LIBRARY FUNCTIONS --------------------------------------

int q_strcasecmp(const char * s1, const char * s2)
{
	const char * p1 = s1;
	const char * p2 = s2;
	char c1, c2;

	if (p1 == p2)
		return 0;

	do
	{
		c1 = q_tolower (*p1++);
		c2 = q_tolower (*p2++);
		if (c1 == '\0')
			break;
	} while (c1 == c2);

	return (int)(c1 - c2);
}

int q_strncasecmp(const char *s1, const char *s2, size_t n)
{
	const char * p1 = s1;
	const char * p2 = s2;
	char c1, c2;

	if (p1 == p2 || n == 0)
		return 0;

	do
	{
		c1 = q_tolower (*p1++);
		c2 = q_tolower (*p2++);
		if (c1 == '\0' || c1 != c2)
			break;
	} while (--n > 0);

	return (int)(c1 - c2);
}

char *q_strlwr (char *str)
{
	char	*c;
	c = str;
	while (*c)
	{
		*c = q_tolower(*c);
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
		*c = q_toupper(*c);
		c++;
	}
	return str;
}

size_t qerr_strlcat (const char *caller, int linenum,
		     char *dst, const char *src, size_t size)
{
	size_t	ret = q_strlcat (dst, src, size);
	if (ret >= size)
		COM_Error("%s: %d: string buffer overflow!", caller, linenum);
	return ret;
}

size_t qerr_strlcpy (const char *caller, int linenum,
		     char *dst, const char *src, size_t size)
{
	size_t	ret = q_strlcpy (dst, src, size);
	if (ret >= size)
		COM_Error("%s: %d: string buffer overflow!", caller, linenum);
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
		COM_Error("%s: %d: string buffer overflow!", caller, linenum);
	return ret;
}


/*
==============
COM_GetTime

==============
*/
#if defined(PLATFORM_WINDOWS)
double COM_GetTime (void)
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

#elif defined(PLATFORM_DOS)
double COM_GetTime (void)
{
/* See  DJGPP uclock() man page for its limitations */
	return (double) uclock() / (double) UCLOCKS_PER_SEC;
}

#elif defined(PLATFORM_UNIX)
double COM_GetTime (void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}

#elif defined(PLATFORM_AMIGA)
static void AMIGA_TimerCleanup (void)
{
	if (TimerBase)
	{
		/*
		if (!CheckIO((struct IORequest *) timerio)
		{
			AbortIO((struct IORequest *) timerio);
			WaitIO((struct IORequest *) timerio);
		}
		*/
		WaitIO((struct IORequest *) timerio);
		CloseDevice((struct IORequest *) timerio);
		DeleteIORequest((struct IORequest *) timerio);
		DeleteMsgPort(timerport);
		TimerBase = NULL;
	}
}

static void AMIGA_TimerInit (void)
{
	if ((timerport = CreateMsgPort()))
	{
		if ((timerio = (struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest))))
		{
			if (OpenDevice((STRPTR) TIMERNAME, UNIT_MICROHZ,
					(struct IORequest *) timerio, 0) == 0)
			{
#ifdef __MORPHOS__
				TimerBase = (struct Library *)timerio->tr_node.io_Device;
#else
				TimerBase = timerio->tr_node.io_Device;
#endif
			}
			else
			{
				DeleteIORequest((struct IORequest *)timerio);
				DeleteMsgPort(timerport);
			}
		}
		else
		{
			DeleteMsgPort(timerport);
		}
	}

	if (!TimerBase)
		COM_Error("Can't open timer.device");

	atexit (AMIGA_TimerCleanup);
	/* 1us wait, for timer cleanup success */
	timerio->tr_node.io_Command = TR_ADDREQUEST;
	timerio->tr_time.tv_secs = 0;
	timerio->tr_time.tv_micro = 1;
	SendIO((struct IORequest *) timerio);
	WaitIO((struct IORequest *) timerio);
}

double COM_GetTime (void)
{
	struct timeval tv;
	if (!TimerBase)
		AMIGA_TimerInit();
	GetSysTime(&tv);
	return tv.tv_secs + tv.tv_micro / 1000000.0;
}

#else /* GENERIC CASE: */
double COM_GetTime (void)
{
	time_t	t;
	time(&t);
	return t;
}
#endif	/* COM_GetTime () */

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

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;	// end of file
		data++;
	}

// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

// skip /*..*/ comments
	if (c == '/' && data[1] == '*')
	{
		data += 2;
		while (*data && !(*data == '*' && data[1] == '/'))
			data++;
		if (*data)
			data += 2;
		goto skipwhite;
	}

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			if ((c = *data) != 0)
				++data;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}

// parse single characters
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
			break;
	} while (c > 32);

	com_token[len] = 0;
	return data;
}

/*
==============
COM_Error

For abnormal program terminations.
==============
*/
void COM_Error (const char *error, ...)
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
		COM_Error ("%s failed for %lu bytes.", __thisfunc__, (unsigned long)size);
	return ptr;
}

/*
==============
SafeStrdup
==============
*/
char *SafeStrdup (const char *str)
{
	size_t sz = strlen(str) + 1;
	char *ptr = (char *) malloc(sz);
	if (!ptr)
		COM_Error ("%s failed for %lu bytes.", __thisfunc__, (unsigned long)sz);
	memcpy (ptr, str, sz);
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
		if (!q_strcasecmp(check, myargv[i]))
			return i;
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
			COM_Error ("Bad hex number: %s",hex);
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

