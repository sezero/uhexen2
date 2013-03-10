/*
 * sv_model.c -- model loading and caching
 * models are the only shared resource between a client and server
 * running on the same machine.
 * $Id$
 *
 * This version of model.[ch] are based on the quake dedicated server
 * application lhnqserver by Forest 'LordHavoc' Hale, with simplified
 * data structures and loading only brush models without the textures.
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

#include "quakedef.h"

qmodel_t	*loadmodel;
static char	loadname[MAX_QPATH];	// for hunk tags

static void Mod_LoadBrushModel (qmodel_t *mod, void *buffer);
static qmodel_t *Mod_LoadModel (qmodel_t *mod, qboolean crash);

static cvar_t	external_ents = {"external_ents", "1", CVAR_ARCHIVE};

static byte	mod_novis[MAX_MAP_LEAFS/8];
static int	*surfedges;
static medge_t	*edges;

#define	MAX_MOD_KNOWN	2048
static qmodel_t	mod_known[MAX_MOD_KNOWN];
static int	mod_numknown;


/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	Cvar_RegisterVariable (&external_ents);

	memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (vec3_t p, qmodel_t *model)
{
	mnode_t		*node;

	node = model->nodes;
	if (node->contents < 0)
		return (mleaf_t *)node;
	while (1)
	{
		node = node->children[(node->plane->type < 3 ? p[node->plane->type] : DotProduct (p,node->plane->normal)) < node->plane->dist];
		if (node->contents < 0)
			return (mleaf_t *)node;
	}

	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
static byte *Mod_DecompressVis (byte *in, qmodel_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->numleafs+7)>>3;
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);

	return decompressed;
}

byte *Mod_LeafPVS (mleaf_t *leaf, qmodel_t *model)
{
	if (leaf == model->leafs)
		return mod_novis;
	return Mod_DecompressVis (leaf->compressed_vis, model);
}

/*
===================
Mod_ClearAll
===================
*/
void Mod_ClearAll (void)
{
	int		i;
	qmodel_t	*mod;

	for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
			mod->needload = NL_NEEDS_LOADED;
}

/*
==================
Mod_FindName

==================
*/
qmodel_t *Mod_FindName (const char *name)
{
	int		i;
	qmodel_t	*mod;

	if (!name[0])
		Host_Error ("%s: NULL name", __thisfunc__);

//
// search the currently loaded models
//
	for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
	{
		if (!strcmp (mod->name, name) )
			break;
	}

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			Host_Error ("mod_numknown == MAX_MOD_KNOWN");
		q_strlcpy (mod->name, name, MAX_QPATH);
		mod->needload = NL_NEEDS_LOADED;
		mod_numknown++;
	}

	return mod;
}

/*
==================
Mod_LoadModel

Loads a model into the cache
==================
*/
static qmodel_t *Mod_LoadModel (qmodel_t *mod, qboolean crash)
{
	byte	*buf;

	if (mod->needload == NL_PRESENT)
		return mod;

//
// load the file
//
	buf = FS_LoadTempFile (mod->name, & mod->path_id);
	if (!buf)
	{
		if (crash)
			Host_Error ("%s: %s not found", __thisfunc__, mod->name);
		return NULL;
	}

//
// allocate a new model
//
	COM_FileBase (mod->name, loadname, sizeof(loadname));

	loadmodel = mod;

//
// fill it in
//

// call the apropriate loader
	mod->needload = NL_PRESENT;

	Mod_LoadBrushModel (mod, buf);

	return mod;
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
qmodel_t *Mod_ForName (const char *name, qboolean crash)
{
	qmodel_t	*mod;

	mod = Mod_FindName (name);

	return Mod_LoadModel (mod, crash);
}


/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

static byte	*mod_base;


/*
=================
Mod_LoadLighting
=================
*/
static void Mod_LoadLighting (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	loadmodel->lightdata = (byte *) Hunk_AllocName ( l->filelen, "light");
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadVisibility
=================
*/
static void Mod_LoadVisibility (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}
	loadmodel->visdata = (byte *) Hunk_AllocName ( l->filelen, "vis");
	memcpy (loadmodel->visdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadEntities
=================
*/
static void Mod_LoadEntities (lump_t *l)
{
	char	entfilename[MAX_QPATH];
	char		*ents;
	int		mark;
	unsigned int	path_id;

	if (! external_ents.integer)
		goto _load_embedded;

	q_strlcpy(entfilename, loadmodel->name, sizeof(entfilename));
	COM_StripExtension(entfilename, entfilename, sizeof(entfilename));
	q_strlcat(entfilename, ".ent", sizeof(entfilename));
	Con_DPrintf("trying to load %s\n", entfilename);
	mark = Hunk_LowMark();
	ents = (char *) FS_LoadHunkFile (entfilename, &path_id);
	if (ents)
	{
		// use ent file only from the same gamedir as the map
		// itself or from a searchpath with higher priority.
		if (path_id < loadmodel->path_id)
		{
			Hunk_FreeToLowMark(mark);
			Con_DPrintf("ignored %s from a gamedir with lower priority\n", entfilename);
		}
		else
		{
			loadmodel->entities = ents;
			Con_DPrintf("Loaded external entity file %s\n", entfilename);
			return;
		}
	}

_load_embedded:
	if (!l->filelen)
	{
		loadmodel->entities = NULL;
		return;
	}
	loadmodel->entities = (char *) Hunk_AllocName ( l->filelen, "entities");
	memcpy (loadmodel->entities, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadVertexes
=================
*/
static void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (dvertex_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mvertex_t *) Hunk_AllocName (count * sizeof(*out), "vertexes");

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
	}
}

/*
=================
Mod_LoadSubmodels
=================
*/
static void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	dmodel_t	*out;
	int			i, j, count;

	in = (dmodel_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (dmodel_t *) Hunk_AllocName (count * sizeof(*out), "submodels");

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		for (j = 0; j < 3; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		for (j = 0; j < MAX_MAP_HULLS; j++)
			out->headnode[j] = LittleLong (in->headnode[j]);
		out->visleafs = LittleLong (in->visleafs);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
static void Mod_LoadEdges (lump_t *l)
{
	dedge_t *in;
	medge_t *out;
	int	i, count;

	in = (dedge_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (medge_t *) Hunk_AllocName ((count + 1) * sizeof(*out), "edges");

//	loadmodel->edges = out;
//	loadmodel->numedges = count;
	edges = out;

	for (i = 0; i < count; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
static void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out;
	int	i, j, count;

	in = (texinfo_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mtexinfo_t *) Hunk_AllocName (count * sizeof(*out), "texture");

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		for (j = 0; j < 4; j++)
		{
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
			out->vecs[1][j] = LittleFloat (in->vecs[1][j]);
		}
		out->flags = LittleLong (in->flags);
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
static void CalcSurfaceExtents (msurface_t *s, int firstedge, int numedges)
{
	float	mins[2], maxs[2], val;
	int		i, j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;

	for (i = 0; i < numedges; i++)
	{
		e = surfedges[firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[edges[e].v[0]];
		else
			v = &loadmodel->vertexes[edges[-e].v[1]];

		for (j = 0; j < 2; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + 
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i = 0; i < 2; i++)
	{
		bmins[i] = (int) floor(mins[i]/16);
		bmaxs[i] = (int) ceil(maxs[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;
		if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 256)
			Host_Error ("Bad surface extents");
	}
}


/*
=================
Mod_LoadFaces
=================
*/
static void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t	*out;
	int			i, count, surfnum;
	int			planenum, side;
	int			firstedge, numedges;

	in = (dface_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (msurface_t *) Hunk_AllocName (count * sizeof(*out), "faces");

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for (surfnum = 0; surfnum < count; surfnum++, in++, out++)
	{
		firstedge = LittleLong(in->firstedge);
		numedges = LittleShort(in->numedges);
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (out, firstedge, numedges);

	// lighting info

		for (i = 0; i < MAXLIGHTMAPS; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
		{
			out->samples = NULL;
		}
		else
		{
				out->samples = loadmodel->lightdata + i;
		}
	}
}


/*
=================
Mod_SetParent
=================
*/
static void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents < 0)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
static void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t		*out;

	in = (dnode_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mnode_t *) Hunk_AllocName (count * sizeof(*out), "nodes");

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);

		for (j = 0; j < 2; j++)
		{
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
static void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t		*in;
	mleaf_t		*out;
	int			i, count, p;

	in = (dleaf_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mleaf_t *) Hunk_AllocName (count * sizeof(*out), "leafs");

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		p = LittleLong(in->contents);
		out->contents = p;

		p = LittleLong(in->visofs);
		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = loadmodel->visdata + p;
	}
}

/*
=================
Mod_LoadClipnodes
=================
*/
static void Mod_LoadClipnodes (lump_t *l)
{
	dclipnode_t	*in, *out;
	int			i, count;
	hull_t		*hull;

	in = (dclipnode_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (dclipnode_t *) Hunk_AllocName (count * sizeof(*out), "clipnodes");

	loadmodel->clipnodes = out;
	loadmodel->numclipnodes = count;

//player
	hull = &loadmodel->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;

//scorpion
	hull = &loadmodel->hulls[2];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -24;
	hull->clip_mins[1] = -24;
	hull->clip_mins[2] = -20;
	hull->clip_maxs[0] = 24;
	hull->clip_maxs[1] = 24;
	hull->clip_maxs[2] = 20;

//crouch
	hull = &loadmodel->hulls[3];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -12;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 16;

//hydra
	hull = &loadmodel->hulls[4];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -40;
	hull->clip_mins[1] = -40;
	hull->clip_mins[2] = -42;
	hull->clip_maxs[0] = 40;
	hull->clip_maxs[1] = 40;
	hull->clip_maxs[2] = 42;

//golem
	hull = &loadmodel->hulls[5];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -48;
	hull->clip_mins[1] = -48;
	hull->clip_mins[2] = -50;
	hull->clip_maxs[0] = 48;
	hull->clip_maxs[1] = 48;
	hull->clip_maxs[2] = 50;

	for (i = 0; i < count; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
		if (out->children[0] >= count || out->children[1] >= count)
			Host_Error("Corrupt clipping hull (out of range child)\n");
	}
}

/*
=================
Mod_MakeHull0

Duplicate the drawing hull structure as a clipping hull
=================
*/
static void Mod_MakeHull0 (void)
{
	mnode_t		*in, *child;
	dclipnode_t	*out;
	int			i, j, count;
	hull_t		*hull;

	hull = &loadmodel->hulls[0];

	in = loadmodel->nodes;
	count = loadmodel->numnodes;
	out = (dclipnode_t *) Hunk_AllocName (count * sizeof(*out), "hull0");

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;

	for (i = 0; i < count; i++, out++, in++)
	{
		out->planenum = in->plane - loadmodel->planes;
		for (j = 0; j < 2; j++)
		{
			child = in->children[j];
			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - loadmodel->nodes;
		}
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
static void Mod_LoadSurfedges (lump_t *l)
{
	int		i, count;
	int		*in, *out;

	in = (int *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (int *) Hunk_AllocName (count * sizeof(*out), "surfedges");

//	loadmodel->surfedges = out;
//	loadmodel->numsurfedges = count;
	surfedges = out;

	for (i = 0; i < count; i++)
		out[i] = LittleLong (in[i]);
}


/*
=================
Mod_LoadPlanes
=================
*/
static void Mod_LoadPlanes (lump_t *l)
{
	int			i, j;
	mplane_t	*out;
	dplane_t	*in;
	int			count;
	int			bits;

	in = (dplane_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("%s: funny lump size in %s", __thisfunc__, loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mplane_t *) Hunk_AllocName (count * 2 * sizeof(*out), "planes");

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		bits = 0;
		for (j = 0; j < 3; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}

/*
=================
Mod_LoadBrushModel
=================
*/
static void Mod_LoadBrushModel (qmodel_t *mod, void *buffer)
{
	int			i, j;
	dheader_t	*header;
	dmodel_t	*bm;

	loadmodel->type = mod_brush;

	header = (dheader_t *)buffer;

	i = LittleLong (header->version);
	if (i != BSPVERSION)
		Host_Error ("%s: %s has wrong version number (%i should be %i)", __thisfunc__, mod->name, i, BSPVERSION);

// swap all the lumps
	mod_base = (byte *)header;

	for (i = 0; i < (int) sizeof(dheader_t) / 4; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

// load into heap

	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	surfedges = NULL;
	edges = NULL;
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadClipnodes (&header->lumps[LUMP_CLIPNODES]);
	Mod_LoadEntities (&header->lumps[LUMP_ENTITIES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);

	Mod_MakeHull0 ();

	mod->numframes = 2;		// regular and alternate animation

//
// set up the submodels (FIXME: this is confusing)
//
	for (i = 0; i < mod->numsubmodels; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];
		for (j = 1; j < MAX_MAP_HULLS; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes-1;
		}

		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;

		VectorCopy (bm->maxs, mod->maxs);
		VectorCopy (bm->mins, mod->mins);

		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels-1)
		{	// duplicate the basic information
			char	name[10];

			q_snprintf (name, sizeof(name), "*%i", i+1);
			loadmodel = Mod_FindName (name);
			*loadmodel = *mod;
			strcpy (loadmodel->name, name);
			mod = loadmodel;
		}
	}
}

