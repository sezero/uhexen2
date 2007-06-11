/*
	common.c
	misc utility functions used in client and server

	$Id: common.c,v 1.95 2007-06-11 18:35:59 sezero Exp $
*/

#include "quakedef.h"
#include <ctype.h>


int		safemode;


/*
============================================================================

REPLACEMENT FUNCTIONS

============================================================================
*/

char *Q_strlwr (char *str)
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

char *Q_strupr (char *str)
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
============================================================================

MISC UTILITY FUNCTIONS

============================================================================
*/

/*
============
va

does a varargs printf into a temp buffer. cycles between
4 different static buffers. the number of buffers cycled
is defined in VA_NUM_BUFFS.
============
*/
#define	VA_NUM_BUFFS	4
#define	VA_BUFFERLEN	1024

static char *get_va_buffer(void)
{
	static char va_buffers[VA_NUM_BUFFS][VA_BUFFERLEN];
	static unsigned char	buf_idx;
	return va_buffers[(VA_NUM_BUFFS-1) & ++buf_idx];
}

char *va (const char *format, ...)
{
	va_list		argptr;
	char		*va_buf;

	va_buf = get_va_buffer ();
	va_start (argptr, format);
	if ( vsnprintf(va_buf, VA_BUFFERLEN, format, argptr) >= VA_BUFFERLEN )
		Con_DPrintf("%s: overflow (string truncated)\n", __thisfunc__);
	va_end (argptr);

	return va_buf;
}


/*============================================================================
  quick'n'dirty string comparison function for use with qsort
  ============================================================================*/

int COM_StrCompare (const void *arg1, const void *arg2)
{
	return Q_strcasecmp ( *(char **) arg1, *(char **) arg2);
}


/*============================================================================
  FileName Processing utilities
  ============================================================================*/

/*
============
COM_SkipPath
============
*/
char *COM_SkipPath (char *pathname)
{
	char	*last;

	last = pathname;
	while (*pathname)
	{
		if (*pathname == '/')
			last = pathname+1;
		pathname++;
	}
	return last;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension (const char *in, char *out)
{
	while (*in && *in != '.')
		*out++ = *in++;
	*out = 0;
}

/*
============
COM_FileExtension
============
*/
char *COM_FileExtension (const char *in)
{
	static char exten[8];
	int		i;

	while (*in && *in != '.')
		in++;
	if (!*in)
		return "";
	in++;
	for (i = 0; i < 7 && *in; i++, in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase (const char *in, char *out)
{
	const char	*s, *s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
		s--;

	for (s2 = s; s2 >= in && *s2 && *s2 != '/'; s2--)
		;

	if (s-s2 < 2)
		strcpy (out,"?model?");
	else
	{
		s--;
		strncpy (out, s2+1, s-s2);
		out[s-s2] = 0;
	}
}

/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension (char *path, const char *extension, size_t len)
{
	char	*src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	Q_strlcat_err(path, extension, len);
}


/*
============================================================================

STRING PARSING FUNCTIONS

============================================================================
*/

char		com_token[1024];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	int		c;
	int		len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;			// end of file;
		data++;
	}

// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
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
/*	if (c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}
*/
// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;

//		if (c == '{' || c == '}' || c == ')' || c == '(' || c == '\'' || c == ':')
//			break;
	} while (c > 32);

	com_token[len] = 0;
	return data;
}


/*
============================================================================

COMMAND LINE PROCESSING FUNCTIONS

============================================================================
*/

/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_CheckParm (const char *parm)
{
	int		i;

	for (i = 1; i < com_argc; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP sometimes clears appkit vars.
		if (!strcmp (parm,com_argv[i]))
			return i;
	}

	return 0;
}

static void COM_Cmdline_f (void)
{
	int			i;

	Con_Printf ("cmdline was:");
	for (i = 0; (i < MAX_NUM_ARGVS) && (i < com_argc); i++)
	{
		if (com_argv[i])
			Con_Printf (" %s", com_argv[i]);
	}
	Con_Printf ("\n");
}

/*
================
COM_Init
================
*/
void COM_Init (void)
{
	Cmd_AddCommand ("cmdline", COM_Cmdline_f);

	safemode = COM_CheckParm ("-safe");
}

