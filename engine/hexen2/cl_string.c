/*
	cl_string.c: Hexen II internationalized stuff
	strings for the client-side puzzle piece and objectives displays.

	$Id$
*/

#include "quakedef.h"

/* puzzle piece strings for Sbar_PuzzlePieceOverlay() */
char		*puzzle_strings = NULL;
int		*puzzle_string_index = NULL;
int		puzzle_string_count = 0;

/* strings for the mission pack's Objectives display. */
char		*info_strings = NULL;
int		*info_string_index = NULL;
int		info_string_count = 0;

void CL_LoadPuzzleStrings (void)
{
	int		i, j, count = 0;
	char		*start, *end, *space;

	puzzle_string_index = NULL;
	puzzle_string_count = 0;
	puzzle_strings = (char *)FS_LoadHunkFile ("puzzles.txt", NULL);
	if (!puzzle_strings)
		return;

	/*
	 * Format of puzzles.txt:
	 * Line #1 : <number of lines excluding this one>
	 * Line #2+: <one-word short name><one space><full name in multiple words>
	 */

	/* Doing an advanced parsing here in order to overcome any borked files	*/

	j = atoi(puzzle_strings);	/* the intended number of lines		*/
	if (j < 1)
		return;
	if (j > 256)
		j = 256;

	start = puzzle_strings;
	while (*start && *start != '\r' && *start != '\n')
	{	/* find first newline, clear the start	*/
		*start++ = 0;
	}
	if (! *start)
		return;

	while ( *start &&	/* skip and clear all leading space, '\n' and '\r' */
		(*start == '\n' || *start == '\r' || *start == ' ' || *start == '\t') )
	{
		*start++ = 0;
	}
	if (! *start)	/* EOF */
		return;

	while (count <= j)
	{
		i = 0;
		end = start;
		while (*end && *end != '\r' && *end != '\n')
			end++;
		if (! *end)	/* EOF */
			end = NULL;
		else
			*end = 0;

		space = start;
		while (*space && *space != ' ' && *space != '\t')
			space++;
		if (*space)
		{
			/* is there a word after the whitespace? */
			while (space[i] == ' ' || space[i] == '\t')
			{
				space[i] = 0;
				++i;
			}
			if (space[i])	/* we have the full name */
			{
				count++;
				/* clear the trailing space	*/
				while (space[i])
					++i;
				--i;
				while (space[i] == ' ' || space[i] == '\t')
				{
					space[i] = 0;
					--i;
				}
				if (!end)
					break;
				goto forward;
			}
			else	/* .. no full name: we hit the *end = 0 mark we
				 * made, or the EOF. clear until the next entry. */
			{
				if (!end)
					break;
				memset (start, 0, end - start);
				goto forward;
			}
		}
		else	/* no space in the line. clear until the next entry.	*/
		{
			if (!end)
				break;
			memset (start, 0, end - start);
forward:
			start = ++end;
			while ( *start == '\r' || *start == '\n' ||
				*start == ' ' || *start == '\t' )
			{
				*start++ = 0;
			}
			if (*start == 0)	/* EOF	*/
				break;
		}
	}

	if (!count)
		return;

	puzzle_string_count = count * 2;
	puzzle_string_index = (int *)Hunk_AllocName (puzzle_string_count*sizeof(int), "puzzle_string_index");

	i = 0;
	start = puzzle_strings;
	while (i < puzzle_string_count)
	{
		while (*start == 0)
			start++;

		puzzle_string_index[i] = start - puzzle_strings;

		while (*start != 0)
			start++;

		++i;
	}

	Con_Printf("Read in %d puzzle piece names\n", count);
}


void CL_LoadInfoStrings (void)
{
	int		i, count, start;
	signed char	newline_char;

	info_strings = (char *)FS_LoadHunkFile ("infolist.txt", NULL);
	if (!info_strings)
		Host_Error ("%s: couldn't load infolist.txt", __thisfunc__);

	newline_char = -1;

	for (i = count = 0; info_strings[i] != 0; i++)
	{
		if (info_strings[i] == 13 || info_strings[i] == 10)
		{
			if (newline_char == info_strings[i] || newline_char == -1)
			{
				newline_char = info_strings[i];
				count++;
			}
		}
	}

	if (!count)
	{
		Host_Error ("%s: no string lines found", __thisfunc__);
	}

	info_string_index = (int *)Hunk_AllocName ((count + 1)*sizeof(int), "info_string_index");

	for (i = count = start = 0; info_strings[i] != 0; i++)
	{
		if (info_strings[i] == 13 || info_strings[i] == 10)
		{
			if (newline_char == info_strings[i])
			{
				info_string_index[count] = start;
				start = i + 1;
				count++;
			}
			else
			{
				start++;
			}

			info_strings[i] = 0;
		}
	}

	info_string_count = count;
	Con_Printf("Read in %d objectives\n", count);
}

