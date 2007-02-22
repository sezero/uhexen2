/*
	cfgfile.c
	misc reads from the config file

	$Id: cfgfile.c,v 1.1 2007-02-22 07:36:23 sezero Exp $
*/

#include "q_types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "compiler.h"
#include "cvar.h"
#include "quakeio.h"
#include "common.h"


static FILE			*cfg_file;

/*
===================
CFG_ReadCvars

used for doing early reads from config.cfg searching the list
of given cvar names for the user-set values. a temporary
solution until we merge a better cvar system.
the num_vars argument must be the exact number of strings in the
array, otherwise I have nothing against going out of bounds.
===================
*/
void CFG_ReadCvars (const char **vars, const int num_vars)
{
	char	buff[1024], *tmp;
	int			i, j;

	if (!cfg_file || num_vars < 1)
		return;

	j = 0;

	do {
		i = 0;
		memset (buff, 0, sizeof(buff));
		fgets(buff, sizeof(buff), cfg_file);
		if (!feof(cfg_file))
		{
			// we expect a line in the format that Cvar_WriteVariables
			// writes to the config file. although I'm trying to be as
			// much cautious as possible, if the user screws it up by
			// editing it, it's his fault.

			// remove end-of-line characters
			while (buff[i])
			{
				if (buff[i] == '\r' || buff[i] == '\n')
					buff[i] = '\0';
				// while we're here, replace tabs with spaces
				if (buff[i] == '\t')
					buff[i] = ' ';
				i++;
			}
			// go to the last character
			while (buff[i] == 0 && i > 0)
				i--;
			// remove trailing spaces
			while (i > 0)
			{
				if (buff[i] == ' ')
				{
					buff[i] = '\0';
					i--;
				}
				else
					break;
			}

			// the line must end with a quotation mark
			if (buff[i] != '\"')
				continue;
			buff[i] = '\0';

			for (i = 0; i < num_vars && vars[i]; i++)
			{
				// look for the cvar name + one space
				tmp = strstr(buff, va("%s ",vars[i]));
				if (tmp != buff)
					continue;
				// locate the first quotation mark
				tmp = strchr(buff, '\"');
				if (tmp)
				{
					Cvar_Set (vars[i], tmp+1);
					j++;
					break;
				}
			}
		}

		if (j == num_vars)
			break;

	} while (!feof(cfg_file));

	fseek (cfg_file, 0, SEEK_SET);
}

void CFG_CloseConfig (void)
{
	if (cfg_file)
	{
		fclose (cfg_file);
		cfg_file = NULL;
	}
}

int CFG_OpenConfig (const char *cfg_name)
{
	CFG_CloseConfig ();
	QIO_FOpenFile (cfg_name, &cfg_file, true);
	if (!cfg_file)
		return -1;
	return 0;
}

