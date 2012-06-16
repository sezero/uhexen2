/*
 * tyrlite.h
 * $Id: tyrlite.h,v 1.16 2010-02-23 00:07:50 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Modifications by Kevin Shanahan, 1999-2000
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

#ifndef __TYRLITE_H__
#define __TYRLITE_H__

#define JSH2COLOR_VER	"1.2.3"		/* version string */

#define	ON_EPSILON	0.1

#define	MAXLIGHTS	1024

// js features
#define	MAX_ENTRYNUM	32784
#define	MAX_TEX_NAME	64

extern	float		scaledist;
extern	float		scalecos;
extern	float		rangescale;
extern	int		worldminlight;
extern	vec3_t		minlight_color;
extern	int		sunlight;
extern	vec3_t		sunlight_color;
extern	vec3_t		sunmangle;

//extern int		c_culldistplane, c_proper;
extern	byte		*filebase;
extern	vec3_t		bsp_origin;

extern	qboolean	extrasamples;
extern	qboolean	compress_ents;
extern	qboolean	colored;
extern	qboolean	force;
extern	qboolean	nominlimit;
extern	qboolean	makelit;
extern	qboolean	force;
extern	qboolean	external;
extern	qboolean	nodefault;

//void	TransformSample (vec3_t in, vec3_t out);
//void	RotateSample (vec3_t in, vec3_t out);
//void	LoadNodes (char *file);

byte	*GetFileSpace (int size);

qboolean TestLine (vec3_t start, vec3_t stop);
// TYR - added TestSky
qboolean TestSky  (vec3_t start, vec3_t dirn);
void	TestLightFace (int surfnum, qboolean nolight, vec3_t faceoffset);
void	LightFace (int surfnum, qboolean nolight, vec3_t faceoffset);
void	LightFaceLIT (int surfnum, qboolean nolight, vec3_t faceoffset);
void	CheckTex (void);
void	FindTexlightColor (int *surf_r, int *surf_g, int *surf_b, const char *texname);

void	LightLeaf (dleaf_t *leaf);
void	MakeTnodes (dmodel_t *bm);

// js features
void	InitDefFile (const char *fname);
void	CloseDefFile (void);

#endif	/* __TYRLITE_H__ */

