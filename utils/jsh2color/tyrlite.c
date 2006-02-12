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
 * tyrlite/tyrlite.c
 * Modifications by Kevin Shanahan, 1999-2000
 */

#include "tyrlite.h"

extern int numbounces;

int numhits[MAX_MAP_ENTITIES];

float	scaledist	= 1.0;
float	scalecos	= 0.5;
float	rangescale	= 0.5;
int	worldminlight	= 0;
vec3_t	minlight_color	= { 255, 255, 255 };	/* defaults to white light   */
int	sunlight	= 0;
vec3_t	sunlight_color	= { 255, 255, 255 };	/* defaults to white light   */
vec3_t	sunmangle	= { 0, 0, 16384 };	/* defaults to straight down */

byte	*filebase;
byte	*file_p;
byte	*file_end;

dmodel_t *bspmodel;
int	bspfileface;	/* next surface to dispatch */

vec3_t	bsp_origin;

qboolean extrasamples;
qboolean compress_ents;
qboolean facecounter;
qboolean colored;
qboolean nominlimit;
qboolean force;
qboolean makelit;

qboolean nolightface[MAX_MAP_FACES];
vec3_t   faceoffset[MAX_MAP_FACES];
extern int num_lights;

// js features
qboolean external;
qboolean nodefault;
char extfilename[MAX_OSPATH];
tex_col_list tc_list;

// get number of lines in text
long getNumLines(FILE* file)
{
	long	numlines = 0;
	char	line [1024];
	char	*txt = NULL;

	while (NULL != fgets(line, 1024, file))
	{
		txt = strchr(line,'\n');
		if (txt != NULL)
		{
			numlines++;
		}
		else
			break;
	}
	fseek(file,0,SEEK_SET);
	return numlines;
}

// file parser
void ParseDefFile(char* filename)
{
	long	num = 0;
	int	i = 0;
	int	r, g, b;
	char	name[64];
	char	line [1024];

	FILE* file = fopen(filename,"rt");
	if (file != NULL)
	{
		num = max(0,getNumLines(file));
		num = min(num , MAX_ENTRYNUM);
		tc_list.num = num;
		tc_list.entries = (tex_col*) malloc(sizeof(tex_col) * num);

		while (fgets(line,1024,file) != NULL)
		{
			if (line[strlen(line)-1] == '\n')
				line[strlen(line)-1] = '\0';

			if (strlen(line) > 0)
			{
				sscanf(line, "%s %d %d %d", name, &r, &g, &b);

				if (strlen(name) > 0 )
				{
					strcpy(tc_list.entries[i].name,name);
					tc_list.entries[i].red = min(max(r,1),255);
					tc_list.entries[i].green = min(max(g,1),255);
					tc_list.entries[i].blue = min(max(b,1),255);

					i++;
				}
			}

			if (i >= num)
				break;
		}

		num = i;
		tc_list.num = num;
		printf("Loaded %ld entries from file : %s\n", num, filename);
	}
	else
		printf("Unable to open file named : %s \n", filename);
}
// end of js features

byte *GetFileSpace (int size)
{
	byte	*buf;

	LOCK;
	file_p = (byte *)(((long)file_p + 3)&~3);
	buf = file_p;
	file_p += size;
	UNLOCK;
	if (file_p > file_end)
		Error ("GetFileSpace: overrun");
	return buf;
}


void LightThread (void *junk)
{
	int	i;

	while (1)
	{
		LOCK;
		i = bspfileface++;
		UNLOCK;

		if (!facecounter)
		{
			printf("Lighting face %i of %i\r", i, numfaces);
			fflush(stdout);
		}

		if (i >= numfaces)
		{
			printf("\n\nMHColour Completed.\n\n");
			return;
		}

		if (!makelit)
			LightFace (i, nolightface[i], faceoffset[i]);
		else
			LightFaceLIT (i, nolightface[i], faceoffset[i]);
	}
}


void LightThread2 (void *junk)
{
	int	i, j;

	j = bspfileface;

	while (1)
	{
		LOCK;
		i = j++;
		UNLOCK;

		if (!facecounter)
		{
			printf("Checking face %i of %i\r", i, numfaces);
			fflush(stdout);
		}

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

void FindFaceOffsets( void )
{
	int		i, j;
	entity_t	*ent;
	char	name[ 20 ];
	char	*classname;
	vec3_t	org;

	memset( nolightface, 0, sizeof( nolightface ) );

	for( j = dmodels[ 0 ].firstface; j < dmodels[ 0 ].numfaces; j++ )
		nolightface[ j ] = 0;

	for( i = 1; i < nummodels; i++ )
	{
		sprintf( name, "*%d", i );
		ent = FindEntityWithKeyPair( "model", name );
		if ( !ent )
			Error("FindFaceOffsets: Couldn't find entity for model %s.\n",
 										name );

		classname = ValueForKey ( ent, "classname" );
		if ( !strncmp( classname, "rotate_", 7 ) )
		{
			int	start;
			int	end;

			GetVectorForKey(ent, "origin", org);

			start = dmodels[ i ].firstface;
			end = start + dmodels[ i ].numfaces;
			for( j = start; j < end; j++ )
			{
				nolightface[ j ] = 300;
				faceoffset[ j ][ 0 ] = org[ 0 ];
				faceoffset[ j ][ 1 ] = org[ 1 ];
				faceoffset[ j ][ 2 ] = org[ 2 ];
			}
		}
	}
}


extern int	num_clights;
extern int	numlighttex;

/*
 * =============
 *  LightWorld
 * =============
 */
void LightWorld (void)
{
	int	i;
	int	j;
	int	max;
	int	num_colours;

	CheckTex ();
	printf ("\n");

	filebase = file_p = dlightdata;
	file_end = filebase + MAX_MAP_LIGHTING;

	for (i = 0; i < num_entities; i++)
	{
		if (!strcmp (entities[i].classname, "light"))
		{
			entities[i].lightcolour[0] = 0;
			entities[i].lightcolour[1] = 0;
			entities[i].lightcolour[2] = 0;
		}
	}

	// since we're not running on an alpha box, we can safely do it this way...
	if (numlighttex)
		LightThread2 (NULL);
	else
		printf ("Skipping Texture lighting - no faces modify light colour in this BSP!\n");

	// normalise the lightcolours to a base max of 255 and set any one with r/g/b of
	// 0 each to an r/g/b value of 255 each
	num_colours = 0;

	printf ("\nChecking all light sources for potential effectiveness...\n");
	for (i = 0; i < num_entities; i++)
	{
		printf ("- Checking entity %i of %i\r", i + 1, num_entities);
		fflush(stdout);

		if (entities[i].light)
		{
			// hopefully this should never happen, but just in case...
			if (entities[i].lightcolour[0] == 0 &&
				entities[i].lightcolour[1] == 0 && 
				entities[i].lightcolour[2] == 0)
			{
				// a faint orange tinge keeps white lights from looking dull.
				entities[i].lightcolour[0] = 255;
				entities[i].lightcolour[1] = 225;
				entities[i].lightcolour[2] = 200;

				// the light is white, so we don't need to do anything more
				continue;
			}

			if (entities[i].lightcolour[0] == entities[i].lightcolour[1] &&
				entities[i].lightcolour[1] == entities[i].lightcolour[2])
			{
				// a faint orange tinge keeps white lights from looking dull.
				entities[i].lightcolour[0] = 255;
				entities[i].lightcolour[1] = 225;
				entities[i].lightcolour[2] = 200;

				// the light is white, so we don't need to do anything more
				continue;
			}

			num_colours++;
			max = 0;

			for (j = 0; j < 3; j++)
				if (entities[i].lightcolour[j] > max)
					max = entities[i].lightcolour[j];

			// this condition will happen for any flame, torch or globe light.
			if (max == 255)
				continue;

			// use 275 here instead of 255 because coloured lights can seem darker than white
			// ones
			for (j = 0; j < 3; j++)
				entities[i].lightcolour[j] = (275 * entities[i].lightcolour[j]) / max;
		}
	}

	printf ("- %i extra Light sources were coloured by Texture lighting\n", 
		num_colours - num_clights);
	printf ("- A total of %i Light sources out of %i have now been coloured\n", 
		num_colours, num_lights);

	if (num_colours < (num_lights / 4))
		DecisionTime ("I suggest you don't continue, especially if you got the first warning too");

	printf ("\n");

	if (!num_colours)
		Error ("This BSP contains no light colour modifying data!");

	LightThread (NULL);

	lightdatasize = file_p - filebase;

	//printf ("lightdatasize: %i\n", lightdatasize);
}


/*
 * ==================
 * main
 * light modelfile
 * ==================
 */
int main (int argc, char **argv)
{
	int	i;
	double	start;
	double	end;
	char	source[1024];

	//init_log("tyrlite.log");

	printf ("---------------------------------------------------\n");
	printf ("JSH2Colour " JSH2COLOR_VER "-" PLATFORM_VER "\n");
	printf ("based on Tyrlite 0.8, MHColour 0.5 and JSColour 1.0\n");
	printf ("---------------------------------------------------\n");

	force = false;
	makelit = true;
	external = false;	// js feature
	nodefault = false;	// js feature

	for (i=1 ; i<argc ; i++)
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
			printf ("Forcing colouring regardless of potential effectiveness\n");
		}
		else if (!strcmp (argv[i], "-lit"))
		{
			makelit = true;
			printf ("Making a LIT file\n");
		}
		else if (!strcmp (argv[i], "-external") && argc > i)
		{	// js feature
			FILE* f = NULL;
			strcpy(extfilename, argv[i+1]);
			f = fopen(extfilename,"rt");

			if ( f != NULL)
			{
				external = true;
				printf ("Using external definition file : %s\n",extfilename);
				fclose(f);
			}
			else
				printf ("No such file : %s Ignoring this option\n",extfilename);
				i++;
		}
		else if (!strcmp (argv[i], "-nodefault"))
		{	// js feature
			nodefault = true;
			printf ("Ignoring built-in color definition list\n");
		}
		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	// set the options we always want
	nominlimit = false;
	colored = true;
	compress_ents = false;

	if (external == true)	// js feature
		ParseDefFile(extfilename);

	if (i != argc - 1)
	{
		printf ("Usage: jsh2colour [-light num] [-extra] [-force] [-dist num]\n"
			"\t\t  [-range num] [-nodefault] [-external file] bspfile\n");
		exit(0);
	}

	InitThreads ();

	start = I_FloatTime ();

	numbounces = 0;

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp");

	LoadBSPFile (source);
	printf ("Processing \"%s\"...\n", source);

	LoadEntities ();

	MakeTnodes (&dmodels[0]);

	FindFaceOffsets();
	LightWorld ();

	WriteEntitiesToString ();

	if (colored)
		WriteBSPFile (source, BSP_COLORED_VERSION);
	else
		WriteBSPFile (source, BSP_OLD_VERSION);

	end = I_FloatTime ();
	printf ("%0.1f seconds elapsed\n", end-start);

	free(tc_list.entries);	// js feature

	//close_log();

	return 0;
}

