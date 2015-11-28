/*
 * tyrlite.c
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Modifications by Kevin Shanahan, 1999-2000
 * MH, 2001,  Juraj Styk, 2002,  O.Sezer 2012
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
#include "q_endian.h"
#include "byteordr.h"
#include "util_io.h"
#include "pathutil.h"
#include "mathlib.h"
#include "bspfile.h"
#include "litfile.h"
#include "entities.h"
#include "threads.h"
#include "tyrlite.h"
#include "jscolor.h"

qboolean	extrasamples;

float		scaledist	= 1.0F;
float		scalecos	= 0.5F;
float		rangescale	= 0.5F;
int		worldminlight	= 0;
vec3_t		minlight_color	= { 255, 255, 255 };	// defaults to white light
int		sunlight	= 0;
vec3_t		sunlight_color	= { 255, 255, 255 };	// defaults to white light
vec3_t		sunmangle	= { 0, 0, 16384 };	// defaults to straight down

int		bspfileface;	// next surface to dispatch
vec3_t		bsp_origin;


static vec3_t	faceoffset[MAX_MAP_FACES];

static void ColorLightThread (void *junk)
{
	int			i;

	printf ("Begining %s: %i\n", __thisfunc__, (int)(intptr_t)junk);
	while (1)
	{
		ThreadLock();
		i = bspfileface++;
		ThreadUnlock();
		if (i >= numfaces)
			return;

		LightFaceLIT (i, faceoffset[i]);
	}
}


static void TestLightThread (void *junk)
{
	int			i;

	printf ("Begining %s: %i\n", __thisfunc__, (int)(intptr_t)junk);
	while (1)
	{
		ThreadLock();
		i = bspfileface++;
		ThreadUnlock();
		if (i >= numfaces)
			return;

		TestLightFace (i, faceoffset[i]);
	}
}


static void FindFaceOffsets (void)
{
	int		i, j;
	entity_t	*ent;
	char		name[20];
	const char	*classname;
	vec3_t	org;

	for (i = 1; i < nummodels; i++)
	{
		sprintf (name, "*%d", i);
		ent = FindEntityWithKeyPair("model", name);
		if (!ent)
			COM_Error("%s: Couldn't find entity for model %s.\n", __thisfunc__, name);

		classname = ValueForKey (ent, "classname");
		if (!strncmp(classname, "rotate_", 7))
		{
			int	start, end;

			GetVectorForKey(ent, "origin", org);

			start = dmodels[i].firstface;
			end = start + dmodels[i].numfaces;
			for (j = start; j < end; j++)
			{
				faceoffset[j][0] = org[0];
				faceoffset[j][1] = org[1];
				faceoffset[j][2] = org[2];
			}
		}
	}
}

/*
=============
LightWorld
=============
*/
static void LightWorld (void)
{
	int	i, j;
	int	num_colors, colormax;

	CheckTex ();
	printf ("\n");

	for (i = 0; i < num_entities; i++)
	{
		if (!strcmp (entities[i].classname, "light"))
		{
			entities[i].lightcolor[0] = 0;
			entities[i].lightcolor[1] = 0;
			entities[i].lightcolor[2] = 0;
		}
	}

	if (numlighttex)
		RunThreadsOn (TestLightThread);
	else
		printf ("Skipping texture lighting - no faces modify light color in this BSP!\n");

	// normalise the lightcolors to a base max of 255 and set any one with r/g/b of
	// 0 each to an r/g/b value of 255 each
	num_colors = 0;

	printf ("\nChecking all light sources for potential effectiveness...\n");
	for (i = 0; i < num_entities; i++)
	{
		if (entities[i].light)
		{
			// hopefully this should never happen,
			// but just in case...
			if (entities[i].lightcolor[0] == 0 &&
				entities[i].lightcolor[1] == 0 &&
				entities[i].lightcolor[2] == 0)
			{
				// a faint orange tinge keeps white
				// lights from looking dull.
				entities[i].lightcolor[0] = 255;
				entities[i].lightcolor[1] = 225;
				entities[i].lightcolor[2] = 200;

				// the light is white, so we don't need to
				// do anything more
				continue;
			}

			if (entities[i].lightcolor[0] == entities[i].lightcolor[1] &&
				entities[i].lightcolor[1] == entities[i].lightcolor[2])
			{
				// a faint orange tinge keeps white
				// lights from looking dull.
				entities[i].lightcolor[0] = 255;
				entities[i].lightcolor[1] = 225;
				entities[i].lightcolor[2] = 200;

				// the light is white, so we don't need
				// to do anything more
				continue;
			}

			num_colors++;
			colormax = 0;

			for (j = 0; j < 3; j++)
				if (entities[i].lightcolor[j] > colormax)
					colormax = entities[i].lightcolor[j];

			// this condition will happen for any flame,
			// torch or globe light.
			if (colormax == 255)
				continue;
			if (colormax == 0) // just in case
				continue;

			// use 275 here instead of 255 because colored lights
			// can seem darker than white ones
			for (j = 0; j < 3; j++)
				entities[i].lightcolor[j] = (275 * entities[i].lightcolor[j]) / colormax;
		}
	}

	printf ("- %i extra light sources were colored by texture lighting\n", 
						num_colors - num_clights);
	printf ("- A total of %i light sources out of %i have now been colored\n", 
						num_colors, num_lights);

	if (num_colors < (num_lights / 4))
		printf ("num_colors < num_lights / 4, don't expect a good result\n");

	if (!num_colors)
		COM_Error ("This BSP contains no light color modifying data!");

	bspfileface = 0;	// reset
	RunThreadsOn (ColorLightThread);
}


/*
========
main

light modelfile
========
*/
int main (int argc, char **argv)
{
	int		i;
	int		wantthreads;
	double		start, end;
	const char	*extfilename;
	char		source[1024];

	printf ("---------------------------------------------------\n");
	printf ("JSH2Colour %s - %s\n", JSH2COLOR_VER, PLATFORM_STRING);
	printf ("based on Tyrlite 0.8, MHColour 0.5 and JSColour 1.0\n");
	printf ("---------------------------------------------------\n");

	ValidateByteorder ();

	external = false;
	extfilename = NULL;
	nodefault = false;

	wantthreads = 1;	// default to single threaded

	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp(argv[i],"-threads"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			wantthreads = atoi (argv[++i]);
		}
		else if (!strcmp(argv[i],"-extra"))
		{
			extrasamples = true;
			printf ("extra sampling enabled\n");
		}
		else if (!strcmp(argv[i],"-dist"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			scaledist = atof (argv[++i]);
		}
		else if (!strcmp(argv[i],"-range"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			rangescale = atof (argv[++i]);
		}
		else if (!strcmp(argv[i],"-light"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			//worldminlight = atof (argv[++i]);
			++i;	// ignore: does unusual things with eidolon map.
		}
		else if (!strcmp(argv[i],"-force"))
		{
			continue; // compat -- ignore
		}
		else if (!strcmp (argv[i], "-lit"))
		{
			continue; // compat -- ignore
		}
		else if (!strcmp (argv[i], "-external"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			extfilename = argv[++i];
			external = true;
			printf ("Using external definition file: %s\n", extfilename);
		}
		else if (!strcmp (argv[i], "-nodefault"))
		{
			nodefault = true;
			printf ("Ignoring built-in color definition list\n");
		}
		else if (argv[i][0] == '-')
			COM_Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
	{
		printf ("Usage: jsh2colour [-threads #] [-light num] [-extra] [-dist num]\n"
			"\t\t  [-range num] [-nodefault] [-external file] bspfile\n");
		exit(0);
	}

	InitDefFile (extfilename);
	InitThreads (wantthreads, 0x300000);	// FIXME: crazy stacksize: see ltface.c.

	start = COM_GetTime ();

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp", sizeof(source));

	LoadBSPFile (source);
	Init_JSColor();
	printf ("Map file : %s\n", source);

	LoadEntities ();

	MakeTnodes (&dmodels[0]);

	FindFaceOffsets();
	LightWorld ();

	CloseDefFile ();
	MakeLITFile (source);

	end = COM_GetTime ();
	printf ("%0.1f seconds elapsed\n", end-start);

	return 0;
}

