/*
 * light.h
 * $Id: light.h,v 1.5 2007-03-14 21:04:40 sezero Exp $
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

#ifndef __H2UTILS_LIGHT_H
#define __H2UTILS_LIGHT_H

// structure of the light lump

#define	ON_EPSILON	0.1

#define	MAXLIGHTS	1024

extern	float		scaledist;
extern	float		scalecos;
extern	float		rangescale;

//extern int		c_culldistplane, c_proper;
extern	byte		*filebase;
extern	vec3_t		bsp_origin;
extern	float		minlights[MAX_MAP_FACES];

extern	qboolean	extrasamples;

//void	TransformSample (vec3_t in, vec3_t out);
//void	RotateSample (vec3_t in, vec3_t out);
//void	LoadNodes (char *file);

byte	*GetFileSpace (int size);

qboolean TestLine (const vec3_t start, const vec3_t stop);
void	LightFace (int surfnum);
void	LightLeaf (dleaf_t *leaf);
void	MakeTnodes (dmodel_t *bm);

#endif	/* __H2UTILS_LIGHT_H */

