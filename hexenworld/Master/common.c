/*
	common.c
	misc utility functions

	$Id: common.c,v 1.1 2007-02-12 16:54:48 sezero Exp $
*/

#include "defs.h"

char		com_token[1024];
int		com_argc;
char	**com_argv;

static char	*largv[MAX_NUM_ARGVS + 1];
static char	*argvdummy = " ";

/*
============================================================================

COMMAND LINE PROCESSING FUNCTIONS

============================================================================
*/

void COM_InitArgv (int argc, char **argv)
{
	for (com_argc = 0; (com_argc < MAX_NUM_ARGVS) && (com_argc < argc); com_argc++)
	{
		largv[com_argc] = argv[com_argc];
	}

	largv[com_argc] = argvdummy;
	com_argv = largv;
}

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
============================================================================

STRING PARSING FUNCTIONS

============================================================================
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
			return NULL;	// end of file;
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

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	} while (c > 32);

	com_token[len] = 0;
	return data;
}

