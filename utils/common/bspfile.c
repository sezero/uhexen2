/*
 * bspfile.c
 * $Id: bspfile.c,v 1.6 2009-05-05 16:02:49 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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
#include "mathlib.h"
#include "q_endian.h"
#include "bspfile.h"

//=============================================================================

int			nummodels;
dmodel_t	dmodels[MAX_MAP_MODELS];

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
		for (j = 0 ; j < 4 ; j++)
		{
			texinfo[i].vecs[0][j] = LittleFloat (texinfo[i].vecs[0][j]);
			texinfo[i].vecs[1][j] = LittleFloat (texinfo[i].vecs[1][j]);
		}
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
		COM_Error ("%s: odd lump size", __thisfunc__);

	memcpy (dest, (byte *)header + ofs, length);

	return length / size;
}

/*
=============
LoadBSPFile
=============
*/
void LoadBSPFile (const char *filename)
{
	int			i;
	void		*pbuf;

//
// load the file header
//
	LoadFile (filename, &pbuf);
	header = (dheader_t *)pbuf;

// swap the header
	for (i = 0 ; i < (int)sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

	if (header->version != BSPVERSION)
		COM_Error ("%s is version %i, not %i", filename, i, BSPVERSION);

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

	free (pbuf);	// everything has been copied out

//
// swap everything
//
	SwapBSPFile (false);
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

/*
=============
WriteBSPFile

Swaps the bsp file in place, so it should not be referenced again
=============
*/
void WriteBSPFile (const char *filename)
{
	header = &outheader;
	memset (header, 0, sizeof(dheader_t));

	SwapBSPFile (true);

	header->version = LittleLong (BSPVERSION);

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
	printf ("%5i planes       %6i\n", numplanes, (int)(numplanes*sizeof(dplane_t)));
	printf ("%5i vertexes     %6i\n", numvertexes, (int)(numvertexes*sizeof(dvertex_t)));
	printf ("%5i nodes        %6i\n", numnodes, (int)(numnodes*sizeof(dnode_t)));
	printf ("%5i texinfo      %6i\n", numtexinfo, (int)(numtexinfo*sizeof(texinfo_t)));
	printf ("%5i faces        %6i\n", numfaces, (int)(numfaces*sizeof(dface_t)));
	printf ("%5i clipnodes    %6i\n", numclipnodes, (int)(numclipnodes*sizeof(dclipnode_t)));
	printf ("%5i leafs        %6i\n", numleafs, (int)(numleafs*sizeof(dleaf_t)));
	printf ("%5i marksurfaces %6i\n", nummarksurfaces, (int)(nummarksurfaces*sizeof(dmarksurfaces[0])));
	printf ("%5i surfedges    %6i\n", numsurfedges, (int)(numsurfedges*sizeof(dmarksurfaces[0])));
	printf ("%5i edges        %6i\n", numedges, (int)(numedges*sizeof(dedge_t)));
	if (!texdatasize)
		printf ("    0 textures          0\n");
	else
	{
		dmiptexlump_t	*mtl = (dmiptexlump_t *)dtexdata;
		printf ("%5i textures     %6i\n", mtl->nummiptex, texdatasize);
	}
	printf ("      lightdata    %6i\n", lightdatasize);
	printf ("      visdata      %6i\n", visdatasize);
	printf ("      entdata      %6i\n", entdatasize);
}

