//
// pr_strings.c
//

#include "quakedef.h"

// For international stuff
int		*pr_string_index = NULL;
int		pr_string_count = 0;
char		*pr_global_strings = NULL;

char		*puzzle_strings;


void PR_LoadStrings(void)
{
	int		i, count, start;
	char	NewLineChar;

	pr_global_strings = (char *)COM_LoadHunkFile ("strings.txt");
	if (!pr_global_strings)
		Sys_Error ("PR_LoadStrings: couldn't load strings.txt");

	NewLineChar = -1;

	for (i=count=0; pr_global_strings[i] != 0; i++)
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
		else
		{
			//for indexed prints, translate '^' to a newline
			if (pr_global_strings[i]=='^')
			{
				sprintf(pr_global_strings+i,"\n%s",pr_global_strings+i+1);
			}
		}
	}

	pr_string_count = count;
	Con_Printf("Read in %d string lines\n",count);

	puzzle_strings = (char *)COM_LoadHunkFile ("puzzles.txt");
}

