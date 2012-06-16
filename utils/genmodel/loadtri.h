/*
 * loadtri.h
 * $Id: loadtri.h,v 1.5 2007-11-11 18:48:07 sezero Exp $
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

#ifndef __LOADTRI_H
#define __LOADTRI_H

#define MAXTRIANGLES	2048

typedef struct
{
	vec3_t	verts[3];
} triangle_t;

void LoadTriangleList(const char *fileName, triangle_t **triList, int *triangleCount);

extern char	InputFileName[1024];

extern float	FixHTRRotateX;
extern float	FixHTRRotateY;
extern float	FixHTRRotateZ;

extern float	FixHTRTranslateX;
extern float	FixHTRTranslateY;
extern float	FixHTRTranslateZ;

#endif	/* __LOADTRI_H */

