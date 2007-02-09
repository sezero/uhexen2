//
// pr_strng.c
// For international stuff
//

#include "quakedef.h"

int		*pr_string_index = NULL;
int		pr_string_count = 0;
char		*pr_global_strings = NULL;

#if !defined(SERVERONLY)
char		*puzzle_strings;
#endif	/* !SERVERONLY */

#if !defined(H2W)
// Objectives strings of the mission pack
// Not used in HexenWorld
int		*pr_info_string_index = NULL;
int		pr_info_string_count = 0;
char		*pr_global_info_strings = NULL;
#endif	/* !H2W */


void PR_LoadStrings (void)
{
	int		i, count, start;
	signed char	NewLineChar;

	pr_global_strings = (char *)COM_LoadHunkFile ("strings.txt");
	if (!pr_global_strings)
		Sys_Error ("PR_LoadStrings: couldn't load strings.txt");

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
		Sys_Error ("PR_LoadStrings: no string lines found");
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
	puzzle_strings = (char *)COM_LoadHunkFile ("puzzles.txt");
}
#endif	/* !SERVERONLY */


#if !defined(H2W)
// loads the mission pack objectives strings
// not used in HexenWorld
void PR_LoadInfoStrings (void)
{
	int		i, count, start;
	signed char	NewLineChar;

	pr_global_info_strings = (char *)COM_LoadHunkFile ("infolist.txt");
	if (!pr_global_info_strings)
		Sys_Error ("PR_LoadInfoStrings: couldn't load infolist.txt");

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
		Sys_Error ("PR_LoadInfoStrings: no string lines found");
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
#endif	/* !H2W */

