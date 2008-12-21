/*  Copyright (C) 1996-1997  Id Software, Inc.

    This program is free software; you can redistribute it and/or modify
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
	tyrlite.c
	$Id: tyrlite.c,v 1.24 2008-12-21 22:26:53 sezero Exp $

	Modifications by Kevin Shanahan, 1999-2000
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

float		scaledist	= 1.0F;
float		scalecos	= 0.5F;
float		rangescale	= 0.5F;
int		worldminlight	= 0;
vec3_t		minlight_color	= { 255, 255, 255 };	// defaults to white light
int		sunlight	= 0;
vec3_t		sunlight_color	= { 255, 255, 255 };	// defaults to white light
vec3_t		sunmangle	= { 0, 0, 16384 };	// defaults to straight down

//dmodel_t	*bspmodel;
int		bspfileface;	// next surface to dispatch
vec3_t		bsp_origin;

byte		*filebase;
static byte	*file_p, *file_end;

qboolean	nolightface[MAX_MAP_FACES];
vec3_t		faceoffset[MAX_MAP_FACES];
extern int	num_lights;

qboolean	extrasamples;
qboolean	compress_ents;
qboolean	colored;
qboolean	nominlimit;
qboolean	force;
qboolean	makelit;
// js features
qboolean	external;
qboolean	nodefault;
char		extfilename[MAX_OSPATH];


byte *GetFileSpace (int size)
{
	byte	*buf;

	LOCK;
	file_p = (byte *)(((intptr_t)file_p + 3) & ~3);
	buf = file_p;
	file_p += size;
	UNLOCK;
	if (file_p > file_end)
		Error ("%s: overrun", __thisfunc__);
	return buf;
}


static void LightThread (void *junk)
{
	int			i;

	while (1)
	{
		LOCK;
		i = bspfileface++;
		UNLOCK;

		printf("Lighting face %i of %i\r", i, numfaces);
		fflush(stdout);

		if (i >= numfaces)
		{
			printf("\n\nJSH2colour completed.\n\n");
			return;
		}

		if (!makelit)
			LightFace (i, nolightface[i], faceoffset[i]);
		else
			LightFaceLIT (i, nolightface[i], faceoffset[i]);
	}
}


static void LightThread2 (void *junk)
{
	int			i, j;

	j = bspfileface;

	while (1)
	{
		LOCK;
		i = j++;
		UNLOCK;

		printf("Checking face %i of %i\r", i, numfaces);
		fflush(stdout);

		if (i >= numfaces)
		{
			printf ("\n");
			return;
		}

		TestLightFace (i, nolightface[i], faceoffset[i]);
	}
}


extern int	nummodels;
extern dmodel_t	dmodels[MAX_MAP_MODELS];

static void FindFaceOffsets (void)
{
	int		i, j;
	entity_t	*ent;
	char		name[20];
	const char	*classname;
	vec3_t	org;

	memset (nolightface, 0, sizeof(nolightface));

	for (j = dmodels[ 0 ].firstface; j < dmodels[ 0 ].numfaces; j++)
		nolightface[ j ] = 0;

	for (i = 1; i < nummodels; i++)
	{
		sprintf (name, "*%d", i);
		ent = FindEntityWithKeyPair("model", name);
		if ( !ent )
			Error("%s: Couldn't find entity for model %s.\n", __thisfunc__, name);

		classname = ValueForKey (ent, "classname");
		if ( !strncmp(classname, "rotate_", 7) )
		{
			int	start;
			int	end;

			GetVectorForKey(ent, "origin", org);

			start = dmodels[ i ].firstface;
			end = start + dmodels[ i ].numfaces;
			for (j = start; j < end; j++)
			{
				nolightface[j] = 300;
				faceoffset[j][0] = org[0];
				faceoffset[j][1] = org[1];
				faceoffset[j][2] = org[2];
			}
		}
	}
}


extern int	num_clights;

/*
=============
LightWorld
=============
*/
static void LightWorld (void)
{
	int	i;
	int	j;
	int	max;
	int	num_colors;

	CheckTex ();
	printf ("\n");

	filebase = file_p = dlightdata;
	file_end = filebase + MAX_MAP_LIGHTING;

	for (i = 0; i < num_entities; i++)
	{
		if (!strcmp (entities[i].classname, "light"))
		{
			entities[i].lightcolor[0] = 0;
			entities[i].lightcolor[1] = 0;
			entities[i].lightcolor[2] = 0;
		}
	}

	// since we're not running on an alpha box, we can safely do it this way...
	if (numlighttex)
		LightThread2 (NULL);
	else
		printf ("Skipping texture lighting - no faces modify light color in this BSP!\n");

	// normalise the lightcolors to a base max of 255 and set any one with r/g/b of
	// 0 each to an r/g/b value of 255 each
	num_colors = 0;

	printf ("\nChecking all light sources for potential effectiveness...\n");
	for (i = 0; i < num_entities; i++)
	{
		printf ("- Checking entity %i of %i\r", i + 1, num_entities);
		fflush(stdout);

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
			max = 0;

			for (j = 0; j < 3; j++)
				if (entities[i].lightcolor[j] > max)
					max = entities[i].lightcolor[j];

			// this condition will happen for any flame,
			// torch or globe light.
			if (max == 255)
				continue;

			// use 275 here instead of 255 because colored lights
			// can seem darker than white ones
			for (j = 0; j < 3; j++)
				entities[i].lightcolor[j] = (275 * entities[i].lightcolor[j]) / max;
		}
	}

	printf ("- %i extra light sources were colored by texture lighting\n", 
						num_colors - num_clights);
	printf ("- A total of %i light sources out of %i have now been colored\n", 
						num_colors, num_lights);

	if (num_colors < (num_lights / 4))
		DecisionTime ("I suggest you don't continue, especially if you got the first warning too");

	printf ("\n");

	if (!num_colors)
		Error ("This BSP contains no light color modifying data!");

	LightThread (NULL);

	lightdatasize = file_p - filebase;

	//printf ("lightdatasize: %i\n", lightdatasize);
}


/*
==================
main
light modelfile
==================
*/
int main (int argc, char **argv)
{
	int		i;
	double		start, end;
	char		source[1024];
	qboolean	extfile_notfound = false;

	printf ("---------------------------------------------------\n");
	printf ("JSH2Colour %s - %s\n", JSH2COLOR_VER, PLATFORM_STRING);
	printf ("based on Tyrlite 0.8, MHColour 0.5 and JSColour 1.0\n");
	printf ("---------------------------------------------------\n");

	ValidateByteorder ();

	// set the options we always want
	makelit = true;
	nominlimit = false;
	colored = true;
	compress_ents = false;

	// defaults for the user settable options
	force = false;
	external = false;	// js feature
	nodefault = false;	// js feature

	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp(argv[i],"-extra"))
		{
			extrasamples = true;
			printf ("extra sampling enabled\n");
		}
		else if (!strcmp(argv[i],"-dist") && argc > i)
		{
			scaledist = atof (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-range") && argc > i)
		{
			rangescale = atof (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-light") && argc > i)
		{
			worldminlight = atof (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-force"))
		{
			force = true;
			printf ("Forcing coloring regardless of potential effectiveness\n");
		}
		else if (!strcmp (argv[i], "-lit"))
		{
			makelit = true;
		}
		else if (!strcmp (argv[i], "-external") && argc > i)
		{	// js feature
			strcpy(extfilename, argv[i+1]);
			if (access(extfilename, R_OK) == -1)
			{
				printf ("No such file : %s, Ignoring this option\n", extfilename);
				extfile_notfound = true;
				if (nodefault == true)
				{
					printf ("and re-enabling the built-in color definitions\n");
					nodefault = false;
				}
			}
			else
			{
				external = true;
				printf ("Using external definition file : %s\n",extfilename);
			}

			i++;
		}
		else if (!strcmp (argv[i], "-nodefault"))
		{	// js feature
			if (extfile_notfound)
			{
				printf ("Ignoring the -nodefault option, because the external\n");
				printf ("definition file specified can not be accessed\n");
			}
			else
			{
				nodefault = true;
				printf ("Ignoring built-in color definition list\n");
			}
		}
		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
	{
		printf ("Usage: jsh2colour [-light num] [-extra] [-force] [-dist num]\n"
			"\t\t  [-range num] [-nodefault] [-external file] bspfile\n");
		exit(0);
	}

	if (makelit == true)
		printf ("Making a LIT file\n");

	InitDefFile (extfilename);	// js feature

	InitThreads ();

	start = GetTime ();

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

	WriteEntitiesToString ();

//	if (colored)
//		WriteBSPFile (source, BSP_COLORED_VERSION);
//	else
//		WriteBSPFile (source, BSP_OLD_VERSION);
	if (makelit)
		MakeLITFile (source);
	else
		WriteBSPFile (source);

	CloseDefFile ();

	end = GetTime ();
	printf ("%0.1f seconds elapsed\n", end-start);

	return 0;
}

