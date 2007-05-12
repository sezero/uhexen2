/*
	jscolor.c
	$Id: jscolor.c,v 1.2 2007-05-12 09:58:57 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#include "util_inc.h"
#include "cmdlib.h"
#include "util_io.h"
#include "q_endian.h"
#include "pathutil.h"
#include "bspfile.h"
#include "tyrlite.h"

miptex_t		miptex[512];
int				numlighttex;

static void ParseTexinfo (void)
{
	int	i;
	miptex_t *out;
	dmiptexlump_t *m;

	m = (dmiptexlump_t *)&dtexdata[0];

	for (i = 0; i < m->nummiptex; i++)
	{
		if (m->dataofs[i] == -1)
			continue;

		out = (miptex_t *)((unsigned char *)m + m->dataofs[i]);

		// all we need is the name
		strcpy (miptex[i].name, out->name);
	}
}


static void FindColorName (char *name, int r, int g, int b)
{
	// return a color name for a given rgb combo
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


extern int	num_clights;
extern int	num_lights;

void CheckTex (void)
{
	int		i, j;
	int		r, g, b;
	int		count;
	int		r2[100], g2[100], b2[100];
	dface_t	*f;
	int		facecolors[100];
	int		bad;
	char	color_name[10];
	int		foundlava;
	int		foundslime;
	int		uniquecolors;

	// there's never gonna be more than 100 unique colors in any map
	// 3 to 4 is the normal.
	for (i = 0; i < 100; i++)
	{
		facecolors[i] = 0;
		r2[i] = 0;
		g2[i] = 0;
		b2[i] = 0;
	}

	// check textures to see if mhcolor will light well
	numlighttex = 0;
	bad = 0;
	count = 0;
	foundlava = 0;
	foundslime = 0;

	printf ("\nPre-checking face lighting for potential effectiveness...\n");

	// find out how many faces will modify light
	for (i = 0; i < numfaces; i++)
	{
		f = dfaces + i;

		FindTexlightColor (&r, &g, &b, miptex[texinfo[f->texinfo].miptex].name);

		if (r == g && g == b)
			continue;

		// slime or lava will naturally dominate any BSP that contains lots of them, so
		// don't make this a warning condition
		if (miptex[texinfo[f->texinfo].miptex].name[0] != '*')
		{
			// see if this color is already used...
			for (j = 0; j < count; j++)
				if (r2[j] == r && g2[j] == g && b2[j] == b)
					break;

			r2[j] = r;
			g2[j] = g;
			b2[j] = b;
			facecolors[j]++;
			if (j >= count)
				count++;	// not already used
		}
		else
		{
			// slime and lava do count as unique colors, so if we find them, we'll
			// make a note of the fact for later.
			if (miptex[texinfo[f->texinfo].miptex].name[1] == 'l' && !foundlava)
				foundlava = 1;
			else if (miptex[texinfo[f->texinfo].miptex].name[1] == 's' && !foundslime)
				foundslime = 1;
		}

		numlighttex++;
	}

	printf ("- %i light sources out of %i have already been colored (torches/flames/etc)\n", 
									num_clights, num_lights);

	uniquecolors = count + foundlava + foundslime;
	if (uniquecolors)
	{
		// we want 3 or more modifying colors, otherwise one may dominate, or we may
		// end up with a two-tone map - which is horrible
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

			if (!strcmp (color_name, "Orange"))
				printf ("  This is normally not a problem - Orange is quite benign.");
			else
				bad = 1;
		}
	}

	// what's a good number to use here? - 50 seems to work out well
	if (numlighttex < (numfaces / 50) || bad)
	{
		if (num_clights > (num_lights / 4))
			DecisionTime ("Entity lights will probably still be effective - I suggest you continue");
		else
			DecisionTime ("Entity lights probably won't help much - I suggest you quit");
	}
}


int	faces_ltoffset[MAX_MAP_FACES];
byte	newdlightdata[MAX_MAP_LIGHTING*3];
int	newlightdatasize;

static void StoreFaceInfo (void)
{
	int		i;
	dface_t	*fa;

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

	if (makelit)
	{
		StoreFaceInfo ();
		MakeNewLightData ();
	}
}

