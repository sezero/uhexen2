/*
 * sv_model.h -- header for model loading and caching
 * $Id$
 *
 * This version of model.c & model.h are based on a quake dedicated
 * server application, lhnqserver, by LordHavoc.
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

#ifndef __HX2_MODEL_H
#define __HX2_MODEL_H

#include "genmodel.h"
#include "spritegn.h"

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vec3_t		position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// plane_t structure
typedef struct mplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_TRANSLUCENT	0x80	/* r_edge.asm checks this */
#define SURF_DRAWBLACK		0x200

typedef struct
{
	unsigned short	v[2];
} medge_t;

typedef struct
{
	float		vecs[2][4];
	int		flags;
} mtexinfo_t;

typedef struct msurface_s
{
	mplane_t	*plane;
	int		flags;

	short		texturemins[2];
	short		extents[2];

	mtexinfo_t	*texinfo;

// lighting info
	byte		styles[MAXLIGHTMAPS];
	byte		*samples;		// [numstyles*surfsize]
} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int		contents;		// 0, to differentiate from leafs

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];	

	unsigned short	firstsurface;
	unsigned short	numsurfaces;
} mnode_t;


typedef struct mleaf_s
{
// common with node
	int		contents;		// wil be a negative contents number

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
} mleaf_t;

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	dclipnode_t	*clipnodes;
	mplane_t	*planes;
	int		firstclipnode;
	int		lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;


//===================================================================

//
// entity effects
//
#define	EF_BRIGHTFIELD			0x00000001
#define	EF_MUZZLEFLASH			0x00000002
#define	EF_BRIGHTLIGHT			0x00000004
#define	EF_DIMLIGHT			0x00000008
#define	EF_DARKLIGHT			0x00000010
#define	EF_DARKFIELD			0x00000020
#define	EF_LIGHT			0x00000040
#define	EF_NODRAW			0x00000080

//===================================================================

//
// Whole model
//

typedef enum {mod_brush, mod_sprite, mod_alias} modtype_t;

typedef struct qmodel_s
{
	char		name[MAX_QPATH];
	unsigned int	path_id;		// path id of the game directory
							// that this model came from
	int		needload;		// bmodels and sprites don't cache normally

	modtype_t	type;
	int		flags;
	int		numframes;

//
// volume occupied by the model graphics
//
	vec3_t		mins, maxs;
	float		radius;

//
// brush model
//
	int		firstmodelsurface, nummodelsurfaces;

	int		numsubmodels;
	dmodel_t	*submodels;

	int		numplanes;
	mplane_t	*planes;

	int		numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int		numvertexes;
	mvertex_t	*vertexes;

	int		numnodes;
	mnode_t		*nodes;

	int		numtexinfo;
	mtexinfo_t	*texinfo;

	int		numsurfaces;
	msurface_t	*surfaces;

	int		numclipnodes;
	dclipnode_t	*clipnodes;

	hull_t		hulls[MAX_MAP_HULLS];

	byte		*visdata;
	byte		*lightdata;
	char		*entities;
} qmodel_t;

// values for qmodel_t->needload
#define	NL_PRESENT		0
#define	NL_NEEDS_LOADED		1
#define	NL_UNREFERENCED		2

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
qmodel_t *Mod_ForName (const char *name, qboolean crash);
qmodel_t *Mod_FindName (const char *name);

mleaf_t *Mod_PointInLeaf (float *p, qmodel_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, qmodel_t *model);

#endif	/* __HX2_MODEL_H */

