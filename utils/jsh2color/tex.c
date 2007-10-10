/*  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    See file, 'COPYING', for details.
*/

/*
	tex.c
	returns a color for a given texture name.  currently
	only works with standard quake textures.
	expand as you see fit - you'll get the idea :-)

	$Id: tex.c,v 1.12 2007-10-10 14:38:27 sezero Exp $
*/

// V0.4 modifications
// - made some of the textures that were only used in medieval/metal maps orange
//   instead of yellow. this may give spurious 'domination' warnings, but orange
//   at least looks better - yellow is really only suited to base maps
// - added rogue, hipnotic and zerstorer textures

#include "util_inc.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "threads.h"
#include "tyrlite.h"

typedef struct tex_col
{
	char	name[MAX_TEX_NAME];
	int		red;
	int		green;
	int		blue;
} tex_col;

typedef struct tex_col_list
{
	int		num;
	tex_col	*entries;
} tex_col_list;

static tex_col_list		tc_list;


static int getNumLines (FILE* f)
{
	int	numlines = 0;
	char	line[1024];
	char	*txt = NULL;

	while (NULL != fgets(line, sizeof(line), f))
	{
		txt = strchr(line,'\n');
		if (txt != NULL)
		{
			numlines++;
		}
		else
			break;
	}
	fseek (f, 0, SEEK_SET);
	return numlines;
}

static void ParseDefFile (const char *filename)
{
	int	i = 0, num = 0;
	int	r, g, b;
	char	name[MAX_TEX_NAME];
	char	line[1024];

	FILE*	FH = fopen(filename,"rt");

	if (FH == NULL)
	{
		printf("Unable to open file named : %s \n", filename);
		return;
	}

	num = q_max( 0, getNumLines(FH) );
	num = q_min( num , MAX_ENTRYNUM );
	tc_list.num = num;
	tc_list.entries = (tex_col*) malloc(sizeof(tex_col) * num);

	while (fgets(line, sizeof(line), FH) != NULL)
	{
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';

		if (strlen(line) > 0)
		{
			sscanf(line, "%s %d %d %d", name, &r, &g, &b);

			if (strlen(name) > 0 )
			{
				strcpy(tc_list.entries[i].name, name);
				tc_list.entries[i].red	= q_min( q_max(r,1), 255 );
				tc_list.entries[i].green= q_min( q_max(g,1), 255 );
				tc_list.entries[i].blue	= q_min( q_max(b,1), 255 );

				i++;
			}
		}

		if (i >= num)
			break;
	}

	fclose (FH);
	num = i;
	tc_list.num = num;
	printf ("Loaded %d entries from file : %s\n", num, filename);
}

void InitDefFile (const char *fname)
{
	tc_list.entries = NULL;
	if (external == true)
		ParseDefFile (fname);
}

void CloseDefFile (void)
{
	if (tc_list.entries != NULL)
		free(tc_list.entries);
	tc_list.entries = NULL;
}

static void FindTexlightColorExt (int *surf_r, int *surf_g, int *surf_b, const char *texname, tex_col_list list)
{
	int		i, len, num;
	tex_col	*entry;

	entry = NULL;
	num = list.num;

	// use default min values
	*surf_r = 1;
	*surf_g = 1;
	*surf_b = 1;

	// assign values based on external definition
	for (i = 1; i < num; i++)
	{
		entry = &(list.entries[i]);
		len = strlen(entry->name);
		if ((len > 0) && (strncmp (texname, entry->name, len) == 0))
		{
			*surf_r = entry->red;
			*surf_g = entry->green;
			*surf_b = entry->blue;

			break;
		}
	}

#if 0
// sanity check
	if (*surf_r < 1)
		*surf_r = 1;
	if (*surf_g < 1)
		*surf_g = 1;
	if (*surf_b < 1)
		*surf_b = 1;
#endif
}

void FindTexlightColor (int *surf_r, int *surf_g, int *surf_b, const char *texname)
{
	if (nodefault == false)	// js feature
	{
		if (!strncmp (texname, "*lava000", 8))
		{
			*surf_r = 255;
			*surf_g = 100;
			*surf_b = 10;
		}
		else if (!strncmp (texname, "*lava001", 8))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strncmp (texname, "*lava", 5))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strncmp (texname, "*lowlight", 9))
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (!strncmp (texname, "*skulls", 7))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strncmp (texname, "*skullwarp", 10))
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (!strncmp (texname, "*rtex078", 9))
		{
			*surf_r = 10;
			*surf_g = 64;
			*surf_b = 128;
		}
		else if (!strncmp (texname, "*rtex346", 9))
		{
			*surf_r = 255;
			*surf_g = 255;
			*surf_b = 128;
		}
		else if (!strncmp (texname, "*rtex385", 9))
		{
			*surf_r = 196;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (!strncmp (texname, "*rtex396", 9))
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "+0air"))
		{
			*surf_r = 196;
			*surf_g = 196;
			*surf_b = 255;
		}
		else if (!strcmp (texname, "+0fire"))
		{
			*surf_r = 255;
			*surf_g = 196;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "+0pyr"))
		{
			*surf_r = 10;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "+0rune1"))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "+0rune2"))
		{
			*surf_r = 255;
			*surf_g = 255;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "+0steam"))
		{
			*surf_r = 255;
			*surf_g = 255;
			*surf_b = 255;
		}
		else if (!strcmp (texname, "+0sun1"))
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "+0tria"))
		{
			*surf_r = 128;
			*surf_g = 255;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "+0tri"))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "+0wat"))
		{
			*surf_r = 40;
			*surf_g = 40;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "+apen"))
		{
			*surf_r = 255;
			*surf_g = 128;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "+rtex123"))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "celtbrown"))
		{
			*surf_r = 250;
			*surf_g = 150;
			*surf_b = 100;
		}
		else if (!strcmp (texname, "marbleseam"))
		{
			*surf_r = 200;
			*surf_g = 100;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "marble"))
		{
			*surf_r = 200;
			*surf_g = 100;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "mtex402"))
		{
			*surf_r = 10;
			*surf_g = 128;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "mtex436"))
		{
			*surf_r = 220;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "mtex460"))
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (!strcmp (texname, "mtex462"))
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (!strcmp (texname, "mtex463"))
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (!strcmp (texname, "mtex464"))
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (!strcmp (texname, "mtex465"))
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (!strcmp (texname, "mtex482"))
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "mtex488"))
		{
			*surf_r = 220;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "mtex489"))
		{
			*surf_r = 220;
			*surf_g = 160;
			*surf_b = 80;
		}
		else if (!strcmp (texname, "rtex010"))
		{
			*surf_r = 120;
			*surf_g = 200;
			*surf_b = 220;
		}
		else if (!strcmp (texname, "rtex028"))
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 80;
		}
		else if (!strcmp (texname, "rtex044"))
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "rtex045"))
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "rtex070"))
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "rtex074"))
		{
			*surf_r = 150;
			*surf_g = 200;
			*surf_b = 150;
		}
		else if (!strcmp (texname, "rtex088"))
		{
			*surf_r = 150;
			*surf_g = 90;
			*surf_b = 16;
		}
		else if (!strcmp (texname, "rtex097"))
		{
			*surf_r = 255;
			*surf_g = 196;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "rtex099"))
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "rtex122"))
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex123"))
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex124"))
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (!strcmp (texname, "rtex125"))
		{
			*surf_r = 200;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (!strcmp (texname, "rtex126"))
		{
			*surf_r = 200;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (!strcmp (texname, "rtex128"))
		{
			*surf_r = 64;
			*surf_g = 196;
			*surf_b = 64;
		}
		else if (!strcmp (texname, "rtex129"))
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "rtex130"))
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "rtex131"))
		{
			*surf_r = 90;
			*surf_g = 90;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "rtex160"))
		{
			*surf_r = 10;
			*surf_g = 128;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "rtex165"))
		{
			*surf_r = 200;
			*surf_g = 100;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "rtex251"))
		{
			*surf_r = 10;
			*surf_g = 100;
			*surf_b = 200;
		}
		else if (!strcmp (texname, "rtex295"))
		{
			*surf_r = 10;
			*surf_g = 128;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex301"))
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (!strcmp (texname, "rtex321"))
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex322"))
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex332"))
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex333"))
		{
			*surf_r = 10;
			*surf_g = 250;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex334"))
		{
			*surf_r = 10;
			*surf_g = 250;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex345"))
		{
			*surf_r = 220;
			*surf_g = 200;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex349"))
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "rtex350"))
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (!strcmp (texname, "rtex353"))
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (!strcmp (texname, "rtex356"))
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex359"))
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex362"))
		{
			*surf_r = 10;
			*surf_g = 100;
			*surf_b = 200;
		}
		else if (!strcmp (texname, "rtex363"))
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "rtex381"))
		{
			*surf_r = 220;
			*surf_g = 200;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex385"))
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 80;
		}
		else if (!strcmp (texname, "rtex386"))
		{
			*surf_r = 255;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex388"))
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (!strcmp (texname, "rtex451"))
		{
			*surf_r = 255;
			*surf_g = 128;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "rtex452"))
		{
			*surf_r = 255;
			*surf_g = 128;
			*surf_b = 128;
		}
		else if (!strcmp (texname, "rtex454"))
		{
			*surf_r = 64;
			*surf_g = 128;
			*surf_b = 64;
		}
		else if (external == true)
		{	// js feature
			FindTexlightColorExt (surf_r, surf_g, surf_b, texname, tc_list);
		}
		else
		{
			// return a smaller value
			*surf_r = 1;
			*surf_g = 1;
			*surf_b = 1;
		}
	}
	else
	{
		if (external == true)
		{	// js feature
			FindTexlightColorExt (surf_r, surf_g, surf_b, texname, tc_list);
		}
		else
		{
			// return a smaller value
			*surf_r = 1;
			*surf_g = 1;
			*surf_b = 1;
		}
	}
}

