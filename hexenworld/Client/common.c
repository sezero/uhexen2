/*
	common.c
	misc utility functions used in client and server

	$Id: common.c,v 1.91 2007-02-23 15:16:17 sezero Exp $
*/

#include "quakedef.h"
#include <ctype.h>


static char	*largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static char	*argvdummy = " ";

static char	*safeargvs[NUM_SAFE_ARGVS] =
	{"-nomidi", "-nolan", "-nosound", "-nocdaudio", "-nojoy", "-nomouse"};

#define CMDLINE_LENGTH	256
static char	com_cmdline[CMDLINE_LENGTH];

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
==============================================================================

Q_MALLOC / Q_FREE

malloc and free system memory. LordHavoc.
==============================================================================
*/

static unsigned int	qmalloctotal_alloc,
			qmalloctotal_alloccount,
			qmalloctotal_free,
			qmalloctotal_freecount;

void *Q_malloc(unsigned int size)
{
	unsigned int	*mem;

	qmalloctotal_alloc += size;
	qmalloctotal_alloccount++;
	mem = malloc(size+sizeof(unsigned int));
	if (!mem)
		return mem;
	*mem = size;

	return (void *)(mem + 1);
}

void Q_free(void *mem)
{
	unsigned int	*m;

	if (!mem)
		return;
	m = mem;
	m--;	// back up to size
	qmalloctotal_free += *m;	// size
	qmalloctotal_freecount++;
	free(m);
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
		Con_DPrintf("%s: overflow (string truncated)\n", __FUNCTION__);
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

	/* Pa3PyX: no range checking -- used to trash the stack and crash the
	   game randomly upon loading progs, for instance (or in any other
	   instance where one would supply a filename without a path	*/
//	for (s2 = s; *s2 && *s2 != '/'; s2--);
	for (s2 = s; *s2 && *s2 != '/' && s2 >= in; s2--)
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
int		com_argc;
char		**com_argv;

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

/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, char **argv)
{
	qboolean	safe;
	int		i, j, n;

// reconstitute the command line for the cmdline console command
	n = 0;

	for (j = 0; (j < MAX_NUM_ARGVS) && (j < argc); j++)
	{
		i = 0;

		while ((n < (CMDLINE_LENGTH - 1)) && argv[j][i])
		{
			com_cmdline[n++] = argv[j][i++];
		}

		if (n < (CMDLINE_LENGTH - 1))
		{
			if (n)
				com_cmdline[n++] = ' ';
		}
		else
		{
			break;
		}
	}

	com_cmdline[n] = 0;
	if (n && com_cmdline[n-1] == ' ')
		com_cmdline[n-1] = 0;

	safe = false;

	for (com_argc = 0; (com_argc < MAX_NUM_ARGVS) && (com_argc < argc); com_argc++)
	{
		largv[com_argc] = argv[com_argc];
		if (!strcmp ("-safe", argv[com_argc]))
			safe = true;
	}

	if (safe)
	{
	// force all the safe-mode switches. Note that we reserved extra space in
	// case we need to add these, so we don't need an overflow check
		for (i = 0; i < NUM_SAFE_ARGVS; i++)
		{
			largv[com_argc] = safeargvs[i];
			com_argc++;
		}
	}

	largv[com_argc] = argvdummy;
	com_argv = largv;
}

#if 0
/*
================
COM_AddParm

Adds the given string at the end of the current argument list
================
*/
void COM_AddParm (const char *parm)
{
	largv[com_argc++] = parm;
}
#endif

static void COM_Cmdline_f (void)
{
	Con_Printf ("cmdline is: \"%s\"\n", com_cmdline);
}

/*
================
COM_Init
================
*/
void COM_Init (void)
{
	Cmd_AddCommand ("cmdline", COM_Cmdline_f);
}

