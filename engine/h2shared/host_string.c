/*
	host_string.c
	internationalized string resource shared between client and server

	$Id$
*/

#include "quakedef.h"

static char	*host_strings = NULL;
static int	*host_string_index = NULL;
int		host_string_count = 0;


void Host_LoadStrings (void)
{
	int		i, count, start;
	signed char	newline_char;

	host_strings = (char *)FS_LoadHunkFile ("strings.txt", NULL);
	if (!host_strings)
		Host_Error ("%s: couldn't load strings.txt", __thisfunc__);

	newline_char = -1;

	for (i = count = 0; host_strings[i] != 0; i++)
	{
		if (host_strings[i] == '\r' || host_strings[i] == '\n')
		{
			if (newline_char == host_strings[i] || newline_char == -1)
			{
				newline_char = host_strings[i];
				count++;
			}
		}
	}

	if (!count)
	{
		Host_Error ("%s: no string lines found", __thisfunc__);
	}

	host_string_index = (int *)Hunk_AllocName ((count + 1)*sizeof(int), "string_index");

	for (i = count = start = 0; host_strings[i] != 0; i++)
	{
		if (host_strings[i] == '\r' || host_strings[i] == '\n')
		{
			if (newline_char == host_strings[i])
			{
				host_string_index[count] = start;
				start = i + 1;
				count++;
			}
			else
			{
				start++;
			}

			host_strings[i] = 0;
		}
#if defined(H2W)
		/* Hexenworld: translate '^' to
		 * '\n' for indexed prints */
		else if (host_strings[i] == '^')
		{
			host_strings[i] = '\n';
		}
#endif	/* H2W */
	}

	host_string_count = count;
	Con_DPrintf("Read in %d string lines\n", count);
}

const char *Host_GetString (int idx)
{
	return &host_strings[host_string_index[idx]];
}

