/*
 * lighting.c
 * $Id: light.c,v 1.13 2008-12-27 16:45:08 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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
#include "pathutil.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "threads.h"
#include "light.h"

qboolean	extrasamples;

float		scaledist	= 1.0F;
float		scalecos	= 0.5F;
float		rangescale	= 0.5F;

//dmodel_t	*bspmodel;
int		bspfileface;	// next surface to dispatch
vec3_t		bsp_origin;

byte		*filebase;
static byte	*file_p, *file_end;

float		minlights[MAX_MAP_FACES];


byte *GetFileSpace (int size)
{
	byte	*buf;

	ThreadLock();
	file_p = (byte *)(((intptr_t)file_p + 3) & ~3);
	buf = file_p;
	file_p += size;
	ThreadUnlock();
	if (file_p > file_end)
		COM_Error ("%s: overrun", __thisfunc__);
	return buf;
}


void LightThread (void *junk)
{
	int			i;

	printf("Thread %d started\n", (int)(intptr_t)junk);
	while (1)
	{
		ThreadLock();
		i = bspfileface++;
		ThreadUnlock();
		if (i >= numfaces)
			return;

		LightFace (i);
	}
}

/*
=============
LightWorld
=============
*/
static void LightWorld (void)
{
	filebase = file_p = dlightdata;
	file_end = filebase + MAX_MAP_LIGHTING;

	RunThreadsOn (LightThread);

	lightdatasize = file_p - filebase;

	printf ("lightdatasize: %i\n", lightdatasize);
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
	char		source[1024];

	printf ("----- LightFaces ----\n");

	ValidateByteorder ();

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
			scaledist = (float)atof (argv[++i]);
		}
		else if (!strcmp(argv[i],"-range"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			rangescale = (float)atof (argv[++i]);
		}
		else if (argv[i][0] == '-')
			COM_Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
		COM_Error ("usage: light [-threads num] [-extra] [-dist ?] [-range ?] bspfile");

	InitThreads (wantthreads, 0);

	start = COM_GetTime ();

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp", sizeof(source));

	LoadBSPFile (source);
	LoadEntities ();

	MakeTnodes (&dmodels[0]);

	LightWorld ();

	WriteEntitiesToString ();
	WriteBSPFile (source);

	end = COM_GetTime ();
	printf ("%5.1f seconds elapsed\n", end-start);

	return 0;
}

