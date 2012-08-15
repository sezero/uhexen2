/*
 * jscolor.c
 * $Id$
 *
 * based on MHColour v0.5
 * Copyright (C) 2002 Juraj Styk <jurajstyk@host.sk>
 * Copyright (C) 2005-2012 O.Sezer <sezero@users.sourceforge.net>
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "util_io.h"
#include "q_endian.h"
#include "pathutil.h"
#include "bspfile.h"
#include "tyrlite.h"
#include "jscolor.h"


qboolean		external;
qboolean		nodefault;

miptex_t		miptex[512];
int			numlighttex;

typedef struct tex_col
{
	char	name[MAX_TEX_NAME];
	int		red;
	int		green;
	int		blue;
} tex_col;

typedef struct tex_col_list
{
	int	numentries;
	tex_col	*entries;
} tex_col_list;

static tex_col_list	tc_list;


static int getNumLines (FILE* f)
{
	int	numlines = 0;
	char	line[1024];

	while (fgets(line, sizeof(line), f) != NULL)
	{
		if (strchr(line, '\n') == NULL)
			break;
		numlines++;
	}
	return numlines;
}

static void ParseDefFile (const char *filename)
{
	int	i, num;
	int	r, g, b;
	char	name[MAX_TEX_NAME];
	char	line[1024];
	size_t	len;
	FILE	*FH;

	if (!filename || !*filename)
		return;

	FH = fopen(filename, "rt");
	if (FH == NULL)
	{
		COM_Error("%s: unable to open \"%s\"", __thisfunc__, filename);
		return;
	}

	num = getNumLines(FH);
	if (num > MAX_ENTRYNUM)
		num = MAX_ENTRYNUM;
	tc_list.entries = (tex_col*) SafeMalloc(sizeof(tex_col) * num);

	fseek (FH, 0, SEEK_SET);
	i = 0;

	while (fgets(line, sizeof(line), FH) != NULL)
	{
		len = strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';
		if (*line != '\0')
		{
			sscanf(line, "%s %d %d %d", name, &r, &g, &b);
			if (*name != '\0')
			{
				strcpy(tc_list.entries[i].name, name);
				tc_list.entries[i].red  = q_min(q_max(r, 1), 255);
				tc_list.entries[i].green= q_min(q_max(g, 1), 255);
				tc_list.entries[i].blue = q_min(q_max(b, 1), 255);
				i++;
			}
		}

		if (i >= num)
			break;
	}

	fclose (FH);
	num = i;
	tc_list.numentries = num;
	printf ("Loaded %d entries from %s\n", num, filename);
}

static void FindTexlightColorExt (int *surf_r, int *surf_g, int *surf_b, const char *texname, tex_col_list *list)
{
	int	i, num;
	tex_col	*entry;
	size_t	len;

	num = list->numentries;

	/* assign values based on external definition */
	for (i = 0; i < num; i++)
	{
		entry = &(list->entries[i]);
		len = strlen(entry->name);
		if (len > 0 && strncmp(texname, entry->name, len) == 0)
		{
			*surf_r = entry->red;
			*surf_g = entry->green;
			*surf_b = entry->blue;
			return;
		}
	}

	/* use default min values */
	*surf_r = 1;
	*surf_g = 1;
	*surf_b = 1;
}

void FindTexlightColor (int *surf_r, int *surf_g, int *surf_b, const char *texname)
{
	if (nodefault == false)
	{
		if (strncmp(texname, "*lava000", 8) == 0)
		{
			*surf_r = 255;
			*surf_g = 100;
			*surf_b = 10;
		}
		else if (strncmp(texname, "*lava001", 8) == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strncmp(texname, "*lava", 5) == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strncmp(texname, "*lowlight", 9) == 0)
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (strncmp(texname, "*skulls", 7) == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strncmp(texname, "*skullwarp", 10) == 0)
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (strncmp(texname, "*rtex078", 9) == 0)
		{
			*surf_r = 10;
			*surf_g = 64;
			*surf_b = 128;
		}
		else if (strncmp(texname, "*rtex346", 9) == 0)
		{
			*surf_r = 255;
			*surf_g = 255;
			*surf_b = 128;
		}
		else if (strncmp(texname, "*rtex385", 9) == 0)
		{
			*surf_r = 196;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (strncmp(texname, "*rtex396", 9) == 0)
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (strcmp(texname, "+0air") == 0)
		{
			*surf_r = 196;
			*surf_g = 196;
			*surf_b = 255;
		}
		else if (strcmp(texname, "+0fire") == 0)
		{
			*surf_r = 255;
			*surf_g = 196;
			*surf_b = 128;
		}
		else if (strcmp(texname, "+0pyr") == 0)
		{
			*surf_r = 10;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (strcmp(texname, "+0rune1") == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strcmp(texname, "+0rune2") == 0)
		{
			*surf_r = 255;
			*surf_g = 255;
			*surf_b = 128;
		}
		else if (strcmp(texname, "+0steam") == 0)
		{
			*surf_r = 255;
			*surf_g = 255;
			*surf_b = 255;
		}
		else if (strcmp(texname, "+0sun1") == 0)
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (strcmp(texname, "+0tria") == 0)
		{
			*surf_r = 128;
			*surf_g = 255;
			*surf_b = 128;
		}
		else if (strcmp(texname, "+0tri") == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strcmp(texname, "+0wat") == 0)
		{
			*surf_r = 40;
			*surf_g = 40;
			*surf_b = 196;
		}
		else if (strcmp(texname, "+apen") == 0)
		{
			*surf_r = 255;
			*surf_g = 128;
			*surf_b = 10;
		}
		else if (strcmp(texname, "+rtex123") == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strcmp(texname, "celtbrown") == 0)
		{
			*surf_r = 250;
			*surf_g = 150;
			*surf_b = 100;
		}
		else if (strcmp(texname, "marbleseam") == 0)
		{
			*surf_r = 200;
			*surf_g = 100;
			*surf_b = 60;
		}
		else if (strcmp(texname, "marble") == 0)
		{
			*surf_r = 200;
			*surf_g = 100;
			*surf_b = 60;
		}
		else if (strcmp(texname, "mtex402") == 0)
		{
			*surf_r = 10;
			*surf_g = 128;
			*surf_b = 10;
		}
		else if (strcmp(texname, "mtex436") == 0)
		{
			*surf_r = 220;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (strcmp(texname, "mtex460") == 0)
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (strcmp(texname, "mtex462") == 0)
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (strcmp(texname, "mtex463") == 0)
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (strcmp(texname, "mtex464") == 0)
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (strcmp(texname, "mtex465") == 0)
		{
			*surf_r = 250;
			*surf_g = 70;
			*surf_b = 20;
		}
		else if (strcmp(texname, "mtex482") == 0)
		{
			*surf_r = 255;
			*surf_g = 10;
			*surf_b = 10;
		}
		else if (strcmp(texname, "mtex488") == 0)
		{
			*surf_r = 220;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (strcmp(texname, "mtex489") == 0)
		{
			*surf_r = 220;
			*surf_g = 160;
			*surf_b = 80;
		}
		else if (strcmp(texname, "rtex010") == 0)
		{
			*surf_r = 120;
			*surf_g = 200;
			*surf_b = 220;
		}
		else if (strcmp(texname, "rtex028") == 0)
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 80;
		}
		else if (strcmp(texname, "rtex044") == 0)
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (strcmp(texname, "rtex045") == 0)
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (strcmp(texname, "rtex070") == 0)
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (strcmp(texname, "rtex074") == 0)
		{
			*surf_r = 150;
			*surf_g = 200;
			*surf_b = 150;
		}
		else if (strcmp(texname, "rtex088") == 0)
		{
			*surf_r = 150;
			*surf_g = 90;
			*surf_b = 16;
		}
		else if (strcmp(texname, "rtex097") == 0)
		{
			*surf_r = 255;
			*surf_g = 196;
			*surf_b = 196;
		}
		else if (strcmp(texname, "rtex099") == 0)
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 128;
		}
		else if (strcmp(texname, "rtex122") == 0)
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex123") == 0)
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex124") == 0)
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (strcmp(texname, "rtex125") == 0)
		{
			*surf_r = 200;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (strcmp(texname, "rtex126") == 0)
		{
			*surf_r = 200;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (strcmp(texname, "rtex128") == 0)
		{
			*surf_r = 64;
			*surf_g = 196;
			*surf_b = 64;
		}
		else if (strcmp(texname, "rtex129") == 0)
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 196;
		}
		else if (strcmp(texname, "rtex130") == 0)
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 196;
		}
		else if (strcmp(texname, "rtex131") == 0)
		{
			*surf_r = 90;
			*surf_g = 90;
			*surf_b = 196;
		}
		else if (strcmp(texname, "rtex160") == 0)
		{
			*surf_r = 10;
			*surf_g = 128;
			*surf_b = 128;
		}
		else if (strcmp(texname, "rtex165") == 0)
		{
			*surf_r = 200;
			*surf_g = 100;
			*surf_b = 60;
		}
		else if (strcmp(texname, "rtex251") == 0)
		{
			*surf_r = 10;
			*surf_g = 100;
			*surf_b = 200;
		}
		else if (strcmp(texname, "rtex295") == 0)
		{
			*surf_r = 10;
			*surf_g = 128;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex301") == 0)
		{
			*surf_r = 200;
			*surf_g = 120;
			*surf_b = 60;
		}
		else if (strcmp(texname, "rtex321") == 0)
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex322") == 0)
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex332") == 0)
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex333") == 0)
		{
			*surf_r = 10;
			*surf_g = 250;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex334") == 0)
		{
			*surf_r = 10;
			*surf_g = 250;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex345") == 0)
		{
			*surf_r = 220;
			*surf_g = 200;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex349") == 0)
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (strcmp(texname, "rtex350") == 0)
		{
			*surf_r = 128;
			*surf_g = 128;
			*surf_b = 196;
		}
		else if (strcmp(texname, "rtex353") == 0)
		{
			*surf_r = 255;
			*surf_g = 64;
			*surf_b = 64;
		}
		else if (strcmp(texname, "rtex356") == 0)
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex359") == 0)
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex362") == 0)
		{
			*surf_r = 10;
			*surf_g = 100;
			*surf_b = 200;
		}
		else if (strcmp(texname, "rtex363") == 0)
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 128;
		}
		else if (strcmp(texname, "rtex381") == 0)
		{
			*surf_r = 220;
			*surf_g = 200;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex385") == 0)
		{
			*surf_r = 64;
			*surf_g = 64;
			*surf_b = 80;
		}
		else if (strcmp(texname, "rtex386") == 0)
		{
			*surf_r = 255;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex388") == 0)
		{
			*surf_r = 250;
			*surf_g = 220;
			*surf_b = 10;
		}
		else if (strcmp(texname, "rtex451") == 0)
		{
			*surf_r = 255;
			*surf_g = 128;
			*surf_b = 128;
		}
		else if (strcmp(texname, "rtex452") == 0)
		{
			*surf_r = 255;
			*surf_g = 128;
			*surf_b = 128;
		}
		else if (strcmp(texname, "rtex454") == 0)
		{
			*surf_r = 64;
			*surf_g = 128;
			*surf_b = 64;
		}
		else if (external == true)
		{
			FindTexlightColorExt (surf_r, surf_g, surf_b, texname, &tc_list);
		}
		else
		{
			/* return a smaller value */
			*surf_r = 1;
			*surf_g = 1;
			*surf_b = 1;
		}
	}
	else
	{
		if (external == true)
		{
			FindTexlightColorExt (surf_r, surf_g, surf_b, texname, &tc_list);
		}
		else
		{
			/* return a smaller value */
			*surf_r = 1;
			*surf_g = 1;
			*surf_b = 1;
		}
	}
}

static void ParseTexinfo (void)
{
	int		i;
	miptex_t	*out;
	dmiptexlump_t	*m;

	m = (dmiptexlump_t *)&dtexdata[0];

	for (i = 0; i < m->nummiptex; i++)
	{
		if (m->dataofs[i] == -1)
			continue;
		out = (miptex_t *)((unsigned char *)m + m->dataofs[i]);
		/* all we need is the name */
		strcpy (miptex[i].name, out->name);
	}
}

static void FindColorName (char *name, int r, int g, int b)
{
	/* return a color name for a given rgb combo */
	if (r == 255 && g == 10 && b == 10)
		strcpy (name, "Red");
	else if (r == 10 && g == 255 && b == 10)
		strcpy (name, "Green");
	else if (r == 10 && g == 10 && b == 255)
		strcpy (name, "Blue");
	else if (r == 10 && g == 255 && b == 255)
		strcpy (name, "Cyan");
	else if (r == 255 && g == 255 && b == 10)
		strcpy (name, "Yellow");
	else if (r == 255 && g == 10 && b == 255)
		strcpy (name, "Magenta");
	else if (r == 255 && g == 128 && b == 64)
		strcpy (name, "Orange");
	else
		strcpy (name, "Unknown");
}

void CheckTex (void)
{
	int		i, j;
	int		r, g, b;
	int		count;
	int		r2[100], g2[100], b2[100];
	dface_t		*f;
	int		facecolors[100];
	int		bad;
	char		color_name[10];
	int		foundlava;
	int		foundslime;
	int		uniquecolors;

	/* there's never gonna be more than 100 unique colors in any map
	 * 3 to 4 is the normal. */
	for (i = 0; i < 100; i++)
	{
		facecolors[i] = 0;
		r2[i] = 0;
		g2[i] = 0;
		b2[i] = 0;
	}

	/* check textures to see if mhcolor will light well */
	numlighttex = 0;
	bad = 0;
	count = 0;
	foundlava = 0;
	foundslime = 0;

	printf ("\nPre-checking face lighting for potential effectiveness...\n");

	/* find out how many faces will modify light */
	for (i = 0; i < numfaces; i++)
	{
		f = dfaces + i;
		FindTexlightColor (&r, &g, &b, miptex[texinfo[f->texinfo].miptex].name);
		if (r == g && g == b)
			continue;

		/* slime or lava will naturally dominate any BSP that contains lots of them,
		 * so don't make this a warning condition */
		if (miptex[texinfo[f->texinfo].miptex].name[0] != '*')
		{
			/* see if this color is already used */
			for (j = 0; j < count; j++)
			{
				if (r2[j] == r && g2[j] == g && b2[j] == b)
					break;
			}

			r2[j] = r;
			g2[j] = g;
			b2[j] = b;
			facecolors[j]++;
			if (j >= count)
				count++;	/* not already used */
		}
		else
		{
			/* slime and lava do count as unique colors, so if we find them, we'll
			 * make a note of the fact for later. */
			if (miptex[texinfo[f->texinfo].miptex].name[1] == 'l')
				foundlava = 1;
			else if (miptex[texinfo[f->texinfo].miptex].name[1] == 's')
				foundslime = 1;
		}

		numlighttex++;
	}

	printf ("- %i light sources out of %i have already been colored (torches/flames/etc)\n",
		num_clights, num_lights);

	uniquecolors = count + foundlava + foundslime;
	if (uniquecolors)
	{
		/* we want 3 or more modifying colors, otherwise one may dominate, or we may
		 * end up with a two-tone map - which is horrible */
		if (uniquecolors < 3)
			bad = 1;

		printf ("- %i unique color(s) used by texture lighting\n", uniquecolors);
	}
	else
	{
		bad = 1;
	}

	printf ("- %i faces out of %i will modify light color\n", numlighttex, numfaces);

	for (i = 0; i < count; i++)
	{
		FindColorName (color_name, r2[i], g2[i], b2[i]);
		if (facecolors[i] > ((numlighttex * 2) / 3))
		{
			printf ("- %s light may tend to dominate this BSP (%i faces out of %i)\n", 
				color_name, facecolors[i], numlighttex);
			if (strcmp(color_name, "Orange") == 0)
				printf ("  This is normally not a problem - Orange is quite benign.");
			else	bad = 1;
		}
	}

	/* what's a good number to use here? - 50 seems to work out well */
	if (numlighttex < (numfaces / 50) || bad)
	{
		if (num_clights <= (num_lights / 4))
			printf ("num_clights <= num_lights / 4:\nEntity lights probably "
				"won't help much - don't expect a good result\n");
	}
}


int	faces_ltoffset[MAX_MAP_FACES];
byte	newdlightdata[MAX_MAP_LIGHTING*3];
int	newlightdatasize;

static void StoreFaceInfo (void)
{
	int		i;
	dface_t		*fa;

	for (i = 0; i < numfaces; i++)
	{
		fa = dfaces + i;
		faces_ltoffset[i] = fa->lightofs * 3;
	}
}

static void MakeNewLightData (void)
{
	int		i, j;

	for (i = 0, j = 0; i < lightdatasize; i++)
	{
		newdlightdata[j] = dlightdata[i]; j++;
		newdlightdata[j] = dlightdata[i]; j++;
		newdlightdata[j] = dlightdata[i]; j++;
	}

	newlightdatasize = lightdatasize * 3;
}


void Init_JSColor (void)
{
	ParseTexinfo ();
	StoreFaceInfo ();
	MakeNewLightData ();
}

void InitDefFile (const char *fname)
{
	tc_list.numentries = 0;
	tc_list.entries = NULL;
	ParseDefFile (fname);
}

void CloseDefFile (void)
{
	if (tc_list.entries != NULL)
		free(tc_list.entries);
	tc_list.entries = NULL;
}

