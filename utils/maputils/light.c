/*
	lighting.c
	$Id: light.c,v 1.9 2007-12-14 16:41:24 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
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

	LOCK;
	file_p = (byte *)(((long)file_p + 3) & ~3);
	buf = file_p;
	file_p += size;
	UNLOCK;
	if (file_p > file_end)
		Error ("%s: overrun", __thisfunc__);
	return buf;
}


void LightThread (void *junk)
{
	int			i;

	printf("Thread %ld started\n", (long)junk);
	while (1)
	{
		LOCK;
		i = bspfileface++;
		UNLOCK;
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
	double		start, end;
	char		source[1024];

	printf ("----- LightFaces ----\n");
	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp(argv[i],"-threads"))
		{
			numthreads = atoi (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-extra"))
		{
			extrasamples = true;
			printf ("extra sampling enabled\n");
		}
		else if (!strcmp(argv[i],"-dist"))
		{
			scaledist = (float)atof (argv[i+1]);
			i++;
		}
		else if (!strcmp(argv[i],"-range"))
		{
			rangescale = (float)atof (argv[i+1]);
			i++;
		}
		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
		Error ("usage: light [-threads num] [-extra] [-dist ?] [-range ?] bspfile");

	InitThreads ();

	start = GetTime ();

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp");

	LoadBSPFile (source);
	LoadEntities ();

	MakeTnodes (&dmodels[0]);

	LightWorld ();

	WriteEntitiesToString ();
	WriteBSPFile (source);

	end = GetTime ();
	printf ("%5.1f seconds elapsed\n", end-start);

	return 0;
}

