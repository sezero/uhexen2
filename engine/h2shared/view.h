/*
 * view.h -- player eye positioning
 * $Id$
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

#ifndef __HX2_VIEW_H
#define __HX2_VIEW_H

extern	cvar_t		v_gamma;
extern	cvar_t		crosshair;
extern	cvar_t		cl_crossx, cl_crossy;
extern	cvar_t		crosshaircolor;

#ifdef GLQUAKE
extern	float		v_blend[4];
#endif

extern	byte		gammatable[256];	// palette is sent through this
extern	unsigned short	ramps[3][256];

void V_Init (void);
void V_RenderView (void);
void V_CalcBlend (void);
float V_CalcRoll (vec3_t angles, vec3_t velocity);
void V_UpdatePalette (void);

#endif	/* __HX2_VIEW_H */

