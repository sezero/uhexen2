/*
 * host_string.c --
 * internationalized string resource shared between client and server
 * $Id$
 *
 * Copyright (C) 1997-1998 Raven Software Corp.
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

