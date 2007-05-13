/*
	pr_strng.c
	For international stuff

	$Id: pr_strng.c,v 1.10 2007-05-13 11:59:01 sezero Exp $
*/

#include "quakedef.h"

int		*pr_string_index = NULL;
int		pr_string_count = 0;
char		*pr_global_strings = NULL;

#if !defined(SERVERONLY)
char		*puzzle_strings;
int		*puzzle_string_index;
int		puzzle_string_count;
#endif	/* !SERVERONLY */

#if !defined(SERVERONLY) && !defined(H2W)
// Objectives strings of the mission pack
// Not used in HexenWorld
int		*pr_info_string_index = NULL;
int		pr_info_string_count = 0;
char		*pr_global_info_strings = NULL;
#endif	/* !SERVERONLY && !H2W */


void PR_LoadStrings (void)
{
	int		i, count, start;
	signed char	NewLineChar;

	pr_global_strings = (char *)FS_LoadHunkFile ("strings.txt");
	if (!pr_global_strings)
		Sys_Error ("%s: couldn't load strings.txt", __thisfunc__);

	NewLineChar = -1;

	for (i = count = 0; pr_global_strings[i] != 0; i++)
	{
		if (pr_global_strings[i] == 13 || pr_global_strings[i] == 10)
		{
			if (NewLineChar == pr_global_strings[i] || NewLineChar == -1)
			{
				NewLineChar = pr_global_strings[i];
				count++;
			}
		}
	}

	if (!count)
	{
		Sys_Error ("%s: no string lines found", __thisfunc__);
	}

	pr_string_index = (int *)Hunk_AllocName ((count+1)*4, "string_index");

	for (i = count = start = 0; pr_global_strings[i] != 0; i++)
	{
		if (pr_global_strings[i] == 13 || pr_global_strings[i] == 10)
		{
			if (NewLineChar == pr_global_strings[i])
			{
				pr_string_index[count] = start;
				start = i+1;
				count++;
			}
			else
			{
				start++;
			}

			pr_global_strings[i] = 0;
		}
#if defined(H2W)
		else
		{
			// for Hexenworld only:
			// for indexed prints, translate '^' to a newline
			if (pr_global_strings[i] == '^')
				sprintf(pr_global_strings+i,"\n%s",pr_global_strings+i+1);
		}
#endif	/* H2W */
	}

	pr_string_count = count;
	Con_Printf("Read in %d string lines\n",count);
}


#if !defined(SERVERONLY)
void PR_LoadPuzzleStrings (void)
{
	int		i, j, count = 0;
	char		*Start, *End, *Space;

	puzzle_string_index = NULL;
	puzzle_string_count = 0;
	puzzle_strings = (char *)FS_LoadHunkFile ("puzzles.txt");
	if ( !puzzle_strings )
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

	Start = puzzle_strings;
	while (*Start && *Start != '\r' && *Start != '\n')
	{	/* find first newline, clear the start	*/
		*Start = 0;
		Start++;
	}
	if (!*Start)
		return;

	while ( *Start &&	/* skip and clear all leading space, '\n' and '\r' */
		(*Start == '\n' || *Start == '\r' ||
		 *Start == ' ' || *Start == '\t')  )
	{
		*Start = 0;
		Start++;
	}
	if (!*Start)	/* EOF	*/
		return;

	while (count <= j)
	{
		i = 0;
		End = Start;
		while (*End && *End != '\r' && *End != '\n')
			End++;
		if (!*End)	/* EOF	*/
			End = NULL;
		else
			*End = 0;

		Space = Start;
		while (*Space && *Space != ' ' && *Space != '\t')
			Space++;
		if (*Space)
		{
			/* is there a word after the whitespace? */
			while (Space[i] == ' ' || Space[i] == '\t')
			{
				Space[i] = 0;
				++i;
			}
			if (Space[i])	/* we have the full name */
			{
				count++;
				/* clear the trailing space	*/
				while (Space[i])
					++i;
				--i;
				while (Space[i] == ' ' || Space[i] == '\t')
				{
					Space[i] = 0;
					--i;
				}
				if (!End)
					break;
				goto forward;
			}
			else	/* .. no full name: we hit the *End = 0 mark we	*/
			{	/* made or the EOF. clear until the next entry.	*/
				if (!End)
					break;
				memset (Start, 0, End - Start);
				goto forward;
			}
		}
		else	/* no space in the line. clear until the next entry.	*/
		{
			if (!End)
				break;
			memset (Start, 0, End - Start);
forward:
			Start = ++End;
			while ( *Start == '\r' || *Start == '\n' ||
				*Start == ' ' || *Start == '\t' )
			{
				*Start = 0;
				Start++;
			}
			if (*Start == 0)	/* EOF	*/
				break;
		}
	}

	if (!count)
		return;

	puzzle_string_count = count * 2;
	puzzle_string_index = (int *)Hunk_AllocName (puzzle_string_count*4, "puzzle_string_index");

	i = 0;
	Start = puzzle_strings;
	while (i < puzzle_string_count)
	{
		while (*Start == 0)
			Start++;

		puzzle_string_index[i] = Start - puzzle_strings;

		while (*Start != 0)
			Start++;

		++i;
	}

	Con_Printf("Read in %d puzzle piece names\n",count);
}
#endif	/* !SERVERONLY */


#if !defined(SERVERONLY) && !defined(H2W)
// loads the mission pack objectives strings
// not used in HexenWorld
void PR_LoadInfoStrings (void)
{
	int		i, count, start;
	signed char	NewLineChar;

	pr_global_info_strings = (char *)FS_LoadHunkFile ("infolist.txt");
	if (!pr_global_info_strings)
		Sys_Error ("%s: couldn't load infolist.txt", __thisfunc__);

	NewLineChar = -1;

	for (i = count = 0; pr_global_info_strings[i] != 0; i++)
	{
		if (pr_global_info_strings[i] == 13 || pr_global_info_strings[i] == 10)
		{
			if (NewLineChar == pr_global_info_strings[i] || NewLineChar == -1)
			{
				NewLineChar = pr_global_info_strings[i];
				count++;
			}
		}
	}

	if (!count)
	{
		Sys_Error ("%s: no string lines found", __thisfunc__);
	}

	pr_info_string_index = (int *)Hunk_AllocName ((count+1)*4, "info_string_index");

	for ( i = count = start = 0; pr_global_info_strings[i] != 0; i++)
	{
		if (pr_global_info_strings[i] == 13 || pr_global_info_strings[i] == 10)
		{
			if (NewLineChar == pr_global_info_strings[i])
			{
				pr_info_string_index[count] = start;
				start = i+1;
				count++;
			}
			else
			{
				start++;
			}

			pr_global_info_strings[i] = 0;
		}
	}

	pr_info_string_count = count;
	Con_Printf("Read in %d objectives\n",count);
}
#endif	/* !SERVERONLY && !H2W */

