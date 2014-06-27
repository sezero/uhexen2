/*
 * tyrlite.h
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Modifications by Kevin Shanahan, 1999-2000
 * MH, 2001,  Juraj Styk, 2002,  O.Sezer 2012
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

#ifndef TYRLITE_H
#define TYRLITE_H

#define	ON_EPSILON	0.1

#define	MAXLIGHTS	1024

extern	float		scaledist;
extern	float		scalecos;
extern	float		rangescale;
extern	int		worldminlight;
extern	vec3_t		minlight_color;
extern	int		sunlight;
extern	vec3_t		sunlight_color;
extern	vec3_t		sunmangle;

//extern int		c_culldistplane, c_proper;
extern	vec3_t		bsp_origin;

extern	qboolean	extrasamples;

//void	TransformSample (vec3_t in, vec3_t out);
//void	RotateSample (vec3_t in, vec3_t out);
//void	LoadNodes (char *file);

qboolean TestLine (const vec3_t start, const vec3_t stop);
void	TestLightFace (int surfnum, const vec3_t faceoffset);
void	LightFaceLIT (int surfnum, const vec3_t faceoffset);

void	LightLeaf (dleaf_t *leaf);
void	MakeTnodes (dmodel_t *bm);

#endif	/* TYRLITE_H */

