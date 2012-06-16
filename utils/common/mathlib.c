/*
 * mathlib.c -- math primitives
 * $Id: mathlib.c,v 1.4 2008-03-06 18:55:07 sezero Exp $
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
#include "mathlib.h"

vec3_t vec3_origin = { 0, 0, 0 };


qboolean VectorCompare (vec3_t v1, vec3_t v2)
{
	int		i;

	for (i = 0; i < 3; i++)
	{
		if (fabs(v1[i]-v2[i]) > EQUAL_EPSILON)
			return false;
	}

	return true;
}

vec_t Q_rint (vec_t in)
{
	return (vec_t)floor(in + 0.49);
}

vec_t VectorNormalize (vec3_t v)
{
	double	length;

	length = VectorLength(v);
	if (length == 0)
		return 0;

	v[0] /= length;
	v[1] /= length;
	v[2] /= length;

	return (vec_t)length;
}

