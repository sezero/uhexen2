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

#include "cmdlib.h"
//#include "mathlib.h"
#include "bspfile.h"
#include "tyrlite.h"

//=============================================================================

extern	qboolean	makelit;

int			nummodels;
dmodel_t	dmodels[MAX_MAP_MODELS];

miptex_t		miptex[512];
int				nummiptex;

int				numlighttex;

int			visdatasize;
byte		dvisdata[MAX_MAP_VISIBILITY];

int			lightdatasize;
byte		dlightdata[MAX_MAP_LIGHTING];

int			texdatasize;
byte		dtexdata[MAX_MAP_MIPTEX];	// (dmiptexlump_t)

int			entdatasize;
char		dentdata[MAX_MAP_ENTSTRING];

int			numleafs;
dleaf_t		dleafs[MAX_MAP_LEAFS];

int			numplanes;
dplane_t	dplanes[MAX_MAP_PLANES];

int			numvertexes;
dvertex_t	dvertexes[MAX_MAP_VERTS];

int			numnodes;
dnode_t		dnodes[MAX_MAP_NODES];

int			numtexinfo;
texinfo_t	texinfo[MAX_MAP_TEXINFO];

int			numfaces;
dface_t		dfaces[MAX_MAP_FACES];

int			numclipnodes;
dclipnode_t	dclipnodes[MAX_MAP_CLIPNODES];

int			numedges;
dedge_t		dedges[MAX_MAP_EDGES];

int			nummarksurfaces;
unsigned short		dmarksurfaces[MAX_MAP_MARKSURFACES];

int			numsurfedges;
int			dsurfedges[MAX_MAP_SURFEDGES];

//=============================================================================

/*
=============
SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
static void SwapBSPFile (qboolean todisk)
{
	int			i, j, c;
	dmodel_t	*d;
	dmiptexlump_t	*mtl;

// models
	for (i = 0 ; i < nummodels ; i++)
	{
		d = &dmodels[i];

		for (j = 0 ; j < MAX_MAP_HULLS ; j++)
			d->headnode[j] = LittleLong (d->headnode[j]);

		d->visleafs = LittleLong (d->visleafs);
		d->firstface = LittleLong (d->firstface);
		d->numfaces = LittleLong (d->numfaces);

		for (j = 0 ; j < 3 ; j++)
		{
			d->mins[j] = LittleFloat(d->mins[j]);
			d->maxs[j] = LittleFloat(d->maxs[j]);
			d->origin[j] = LittleFloat(d->origin[j]);
		}
	}

//
// vertexes
//
	for (i = 0 ; i < numvertexes ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
			dvertexes[i].point[j] = LittleFloat (dvertexes[i].point[j]);
	}

//
// planes
//
	for (i = 0 ; i < numplanes ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
			dplanes[i].normal[j] = LittleFloat (dplanes[i].normal[j]);
		dplanes[i].dist = LittleFloat (dplanes[i].dist);
		dplanes[i].type = LittleLong (dplanes[i].type);
	}

//
// texinfos
//
	for (i = 0 ; i < numtexinfo ; i++)
	{
		for (j = 0 ; j < 8 ; j++)
			texinfo[i].vecs[0][j] = LittleFloat (texinfo[i].vecs[0][j]);
		texinfo[i].miptex = LittleLong (texinfo[i].miptex);
		texinfo[i].flags = LittleLong (texinfo[i].flags);
	}

//
// faces
//
	for (i = 0 ; i < numfaces ; i++)
	{
		dfaces[i].texinfo = LittleShort (dfaces[i].texinfo);
		dfaces[i].planenum = LittleShort (dfaces[i].planenum);
		dfaces[i].side = LittleShort (dfaces[i].side);
		dfaces[i].lightofs = LittleLong (dfaces[i].lightofs);
		dfaces[i].firstedge = LittleLong (dfaces[i].firstedge);
		dfaces[i].numedges = LittleShort (dfaces[i].numedges);
	}

//
// nodes
//
	for (i = 0 ; i < numnodes ; i++)
	{
		dnodes[i].planenum = LittleLong (dnodes[i].planenum);
		for (j = 0 ; j < 3 ; j++)
		{
			dnodes[i].mins[j] = LittleShort (dnodes[i].mins[j]);
			dnodes[i].maxs[j] = LittleShort (dnodes[i].maxs[j]);
		}
		dnodes[i].children[0] = LittleShort (dnodes[i].children[0]);
		dnodes[i].children[1] = LittleShort (dnodes[i].children[1]);
		dnodes[i].firstface = LittleShort (dnodes[i].firstface);
		dnodes[i].numfaces = LittleShort (dnodes[i].numfaces);
	}

//
// leafs
//
	for (i = 0 ; i < numleafs ; i++)
	{
		dleafs[i].contents = LittleLong (dleafs[i].contents);
		for (j = 0 ; j < 3 ; j++)
		{
			dleafs[i].mins[j] = LittleShort (dleafs[i].mins[j]);
			dleafs[i].maxs[j] = LittleShort (dleafs[i].maxs[j]);
		}

		dleafs[i].firstmarksurface = LittleShort (dleafs[i].firstmarksurface);
		dleafs[i].nummarksurfaces = LittleShort (dleafs[i].nummarksurfaces);
		dleafs[i].visofs = LittleLong (dleafs[i].visofs);
	}

//
// clipnodes
//
	for (i = 0 ; i < numclipnodes ; i++)
	{
		dclipnodes[i].planenum = LittleLong (dclipnodes[i].planenum);
		dclipnodes[i].children[0] = LittleShort (dclipnodes[i].children[0]);
		dclipnodes[i].children[1] = LittleShort (dclipnodes[i].children[1]);
	}

//
// miptex
//
	if (texdatasize)
	{
		mtl = (dmiptexlump_t *)dtexdata;
		if (todisk)
			c = mtl->nummiptex;
		else
			c = LittleLong(mtl->nummiptex);
		mtl->nummiptex = LittleLong (mtl->nummiptex);
		for (i = 0 ; i < c ; i++)
			mtl->dataofs[i] = LittleLong(mtl->dataofs[i]);
	}

//
// marksurfaces
//
	for (i = 0 ; i < nummarksurfaces ; i++)
		dmarksurfaces[i] = LittleShort (dmarksurfaces[i]);

//
// surfedges
//
	for (i = 0 ; i < numsurfedges ; i++)
		dsurfedges[i] = LittleLong (dsurfedges[i]);

//
// edges
//
	for (i = 0 ; i < numedges ; i++)
	{
		dedges[i].v[0] = LittleShort (dedges[i].v[0]);
		dedges[i].v[1] = LittleShort (dedges[i].v[1]);
	}
}


static dheader_t	*header;

static int CopyLump (int lump, void *dest, int size)
{
	int		length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;

	if (length % size)
		Error ("LoadBSPFile: odd lump size");

	memcpy (dest, (byte *)header + ofs, length);

	return length / size;
}

static void ParseTexinfo (void)
{
	int	i;
	miptex_t *out;
	dmiptexlump_t *m;

	m = (dmiptexlump_t *)&dtexdata[0];
	nummiptex = m->nummiptex;

	for (i = 0; i < m->nummiptex; i++)
	{
		if (m->dataofs[i] == -1)
		{
			// one less texture...
			nummiptex--;
			continue;
		}

		out = (miptex_t *)((unsigned char *)m + m->dataofs[i]);

		// all we need is the name
		strcpy (miptex[i].name, out->name);
	}
}


static void FindColourName (char *name, int r, int g, int b)
{
	// return a colour name for a given rgb combo
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
		strcpy (name, "Django"); // should never happen
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
	int		facecolours[100];
	int		bad;
	char	colour_name[10];
	int		foundlava;
	int		foundslime;
	int		uniquecolours;

	// there's never gonna be more than 100 unique colours in any map
	// 3 to 4 is the normal.
	for (i = 0; i < 100; i++)
	{
		facecolours[i] = 0;
		r2[i] = 0;
		g2[i] = 0;
		b2[i] = 0;
	}

	// check textures to see if mh colour will light well
	numlighttex = 0;
	bad = 0;
	count = 0;
	foundlava = 0;
	foundslime = 0;

	printf ("\nPre-Checking Face Lighting for potential effectiveness...\n");

	// find out how many faces will modify light
	for (i = 0; i < numfaces; i++)
	{
		f = dfaces + i;

		FindTexlightColour (&r, &g, &b, miptex[texinfo[f->texinfo].miptex].name);

		if (r == g && g == b)
			continue;

		// slime or lava will naturally dominate any BSP that contains lots of them, so
		// don't make this a warning condition
		if (miptex[texinfo[f->texinfo].miptex].name[0] != '*')
		{
			// see if this colour is already used...
			for (j = 0; j < count; j++)
				if (r2[j] == r && g2[j] == g && b2[j] == b)
					break;

			r2[j] = r;
			g2[j] = g;
			b2[j] = b;
			facecolours[j]++;
			if (j >= count)
				count++;	// not already used
		}
		else
		{
			// slime and lava do count as unique colours, so if we find them, we'll
			// make a note of the fact for later.
			if (miptex[texinfo[f->texinfo].miptex].name[1] == 'l' && !foundlava)
				foundlava = 1;
			else if (miptex[texinfo[f->texinfo].miptex].name[1] == 's' && !foundslime)
				foundslime = 1;
		}

		numlighttex++;
	}

	printf ("- %i Light sources out of %i have already been coloured (torches/flames/etc)\n", 
						num_clights, num_lights);

	uniquecolours = count + foundlava + foundslime;
	if (uniquecolours)
	{
		// we want 3 or more modifying colours, otherwise one may dominate, or we may
		// end up with a two-tone map - which is horrible
		if (uniquecolours < 3)
			bad = 1;

		printf ("- %i Unique Colour(s) used by Texture lighting\n", uniquecolours);
	}
	else
	{
		bad = 1;
	}

	printf ("- %i Faces out of %i will modify Light Colour\n", numlighttex, numfaces);

	for (i = 0; i < count; i++)
	{
		FindColourName (colour_name, r2[i], g2[i], b2[i]);

		if (facecolours[i] > ((numlighttex * 2) / 3))
		{
			printf ("- %s Light may tend to dominate this BSP (%i faces out of %i)\n", 
				colour_name, facecolours[i], numlighttex);

			if (!strcmp (colour_name, "Orange"))
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


int	bsp_ver;

int	faces_ltoffset[MAX_MAP_FACES];
byte	newdlightdata[MAX_MAP_LIGHTING];
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


/*
=============
LoadBSPFile
=============
*/
void LoadBSPFile (char *filename)
{
	int			i;

//
// load the file header
//
	LoadFile (filename, (void **) (char *) &header);

// swap the header
	for (i = 0 ; i < sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

	/* TYR - fixed stupid error (using loop variable as version number??) */
	bsp_ver = (int)header->version;

	//if (header->version != BSPVERSION)
	//	Error ("%s is version %i, not %i", filename, i, BSPVERSION);

	nummodels = CopyLump (LUMP_MODELS, dmodels, sizeof(dmodel_t));
	numvertexes = CopyLump (LUMP_VERTEXES, dvertexes, sizeof(dvertex_t));
	numplanes = CopyLump (LUMP_PLANES, dplanes, sizeof(dplane_t));
	numleafs = CopyLump (LUMP_LEAFS, dleafs, sizeof(dleaf_t));
	numnodes = CopyLump (LUMP_NODES, dnodes, sizeof(dnode_t));
	numtexinfo = CopyLump (LUMP_TEXINFO, texinfo, sizeof(texinfo_t));
	numclipnodes = CopyLump (LUMP_CLIPNODES, dclipnodes, sizeof(dclipnode_t));
	numfaces = CopyLump (LUMP_FACES, dfaces, sizeof(dface_t));
	nummarksurfaces = CopyLump (LUMP_MARKSURFACES, dmarksurfaces, sizeof(dmarksurfaces[0]));
	numsurfedges = CopyLump (LUMP_SURFEDGES, dsurfedges, sizeof(dsurfedges[0]));
	numedges = CopyLump (LUMP_EDGES, dedges, sizeof(dedge_t));

	texdatasize = CopyLump (LUMP_TEXTURES, dtexdata, 1);
	visdatasize = CopyLump (LUMP_VISIBILITY, dvisdata, 1);
	lightdatasize = CopyLump (LUMP_LIGHTING, dlightdata, 1);
	entdatasize = CopyLump (LUMP_ENTITIES, dentdata, 1);

	free (header);		// everything has been copied out

//
// swap everything
//
	SwapBSPFile (false);

	ParseTexinfo ();

	if (makelit) 
	{
		StoreFaceInfo ();
		MakeNewLightData ();
	}
}

//============================================================================

static FILE		*wadfile;
static dheader_t	outheader;

static void AddLump (int lumpnum, void *data, int len)
{
	lump_t	*lump;

	lump = &header->lumps[lumpnum];

	lump->fileofs = LittleLong( ftell(wadfile) );
	lump->filelen = LittleLong(len);
	SafeWrite (wadfile, data, (len+3)&~3);
}

//============================================================================

/*
Make a DarkPlaces format LIT file - we already have
the lightdata in dlightdata so we just need
to write out the appropriate header and we're there!
*/

// litfile header - darkplaces standard
typedef struct litheader_s
{
	char	ident[4];
	int		version;
} litheader_t;


static void MakeLITFile (char *bspname)
{
	char	litname[1024];
	FILE	*litfile;
	litheader_t	litheader;

	if (!makelit)
		return;

	strcpy (litname, bspname);
	StripExtension (litname);
	DefaultExtension (litname, ".lit");
	litfile = fopen (litname, "wb");

	if (!litfile)
	{
		printf ("Unable to create %s\n", litname);
		return;
	}

	litheader.ident[0] = 'Q';
	litheader.ident[1] = 'L';
	litheader.ident[2] = 'I';
	litheader.ident[3] = 'T';
	litheader.version = 1;

	fwrite (&litheader, sizeof (litheader), 1, litfile);
	fwrite (&newdlightdata, newlightdatasize, 1, litfile);

	fclose (litfile);
	printf ("Wrote litfile: %s\n", litname);
}


/*
=============
WriteBSPFile

Swaps the bsp file in place, so it should not be referenced again
=============
*/
void WriteBSPFile (char *filename, int version)
{
	if (makelit)
	{
		MakeLITFile (filename);
		return;
	}

	header = &outheader;
	memset (header, 0, sizeof(dheader_t));

	SwapBSPFile (true);

	//header->version = LittleLong (BSPVERSION);
	header->version = LittleLong (version);
// FIXME !!!
	header->version = 30;
	printf("Writing BSP version %i\n", (int)header->version);

	wadfile = SafeOpenWrite (filename);
	SafeWrite (wadfile, header, sizeof(dheader_t));	// overwritten later

	AddLump (LUMP_PLANES, dplanes, numplanes*sizeof(dplane_t));
	AddLump (LUMP_LEAFS, dleafs, numleafs*sizeof(dleaf_t));
	AddLump (LUMP_VERTEXES, dvertexes, numvertexes*sizeof(dvertex_t));
	AddLump (LUMP_NODES, dnodes, numnodes*sizeof(dnode_t));
	AddLump (LUMP_TEXINFO, texinfo, numtexinfo*sizeof(texinfo_t));
	AddLump (LUMP_FACES, dfaces, numfaces*sizeof(dface_t));
	AddLump (LUMP_CLIPNODES, dclipnodes, numclipnodes*sizeof(dclipnode_t));
	AddLump (LUMP_MARKSURFACES, dmarksurfaces, nummarksurfaces*sizeof(dmarksurfaces[0]));
	AddLump (LUMP_SURFEDGES, dsurfedges, numsurfedges*sizeof(dsurfedges[0]));
	AddLump (LUMP_EDGES, dedges, numedges*sizeof(dedge_t));
	AddLump (LUMP_MODELS, dmodels, nummodels*sizeof(dmodel_t));

	AddLump (LUMP_LIGHTING, dlightdata, lightdatasize);
	AddLump (LUMP_VISIBILITY, dvisdata, visdatasize);
	AddLump (LUMP_ENTITIES, dentdata, entdatasize);
	AddLump (LUMP_TEXTURES, dtexdata, texdatasize);

	fseek (wadfile, 0, SEEK_SET);
	SafeWrite (wadfile, header, sizeof(dheader_t));
	fclose (wadfile);
}

//============================================================================

/*
=============
PrintBSPFileSizes

Dumps info about current file
=============
*/
void PrintBSPFileSizes (void)
{
	printf ("%5i planes       %6i\n",
			numplanes, (int)(numplanes*sizeof(dplane_t)));
	printf ("%5i vertexes     %6i\n",
			numvertexes, (int)(numvertexes*sizeof(dvertex_t)));
	printf ("%5i nodes        %6i\n",
			numnodes, (int)(numnodes*sizeof(dnode_t)));
	printf ("%5i texinfo      %6i\n",
			numtexinfo, (int)(numtexinfo*sizeof(texinfo_t)));
	printf ("%5i faces        %6i\n",
			numfaces, (int)(numfaces*sizeof(dface_t)));
	printf ("%5i clipnodes    %6i\n",
			numclipnodes, (int)(numclipnodes*sizeof(dclipnode_t)));
	printf ("%5i leafs        %6i\n",
			numleafs, (int)(numleafs*sizeof(dleaf_t)));
	printf ("%5i marksurfaces %6i\n",
			nummarksurfaces, (int)(nummarksurfaces*sizeof(dmarksurfaces[0])));
	printf ("%5i surfedges    %6i\n",
			numsurfedges, (int)(numsurfedges*sizeof(dmarksurfaces[0])));
	printf ("%5i edges        %6i\n",
			numedges, (int)(numedges*sizeof(dedge_t)));
	if (!texdatasize)
		printf ("    0 textures          0\n");
	else
		printf ("%5i textures     %6i\n",((dmiptexlump_t*)dtexdata)->nummiptex, texdatasize);
	printf ("      lightdata    %6i\n", lightdatasize);
	printf ("      visdata      %6i\n", visdatasize);
	printf ("      entdata      %6i\n", entdatasize);
}

