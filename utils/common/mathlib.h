/*
 * mathlib.h -- math primitives
 * $Id: mathlib.h,v 1.6 2008-03-06 18:55:07 sezero Exp $
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

#ifndef __MATHLIB_H
#define __MATHLIB_H

#include <math.h>

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* matches value in gcc v2 math.h */
#endif
#define	Q_PI		M_PI

#define	EQUAL_EPSILON	0.001

extern vec3_t vec3_origin;

qboolean VectorCompare (vec3_t v1, vec3_t v2);

#define DotProduct(x,y)		((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
#define VectorLength(a)		sqrt(DotProduct((a),(a)))

#define CrossProduct(v1,v2,cross)					\
	do {								\
		(cross)[0] = (v1)[1] * (v2)[2] - (v1)[2] * (v2)[1];	\
		(cross)[1] = (v1)[2] * (v2)[0] - (v1)[0] * (v2)[2];	\
		(cross)[2] = (v1)[0] * (v2)[1] - (v1)[1] * (v2)[0];	\
	} while (0)

#define VectorAdd(a,b,c)						\
	do {								\
		(c)[0] = (a)[0] + (b)[0];				\
		(c)[1] = (a)[1] + (b)[1];				\
		(c)[2] = (a)[2] + (b)[2];				\
	} while (0)

#define VectorSubtract(a,b,c)						\
	do {								\
		(c)[0] = (a)[0] - (b)[0];				\
		(c)[1] = (a)[1] - (b)[1];				\
		(c)[2] = (a)[2] - (b)[2];				\
	} while (0)

#define VectorInverse(v)						\
	do {								\
		(v)[0] = -(v)[0];					\
		(v)[1] = -(v)[1];					\
		(v)[2] = -(v)[2];					\
	} while (0)

#define VectorClear(a)		((a)[2] = (a)[1] = (a)[0] = 0)
#if 0
#define VectorCopy(a,b)		memcpy((b),(a),sizeof(vec3_t))
#else
#define VectorCopy(a,b)							\
	do {								\
		(b)[0] = (a)[0];					\
		(b)[1] = (a)[1];					\
		(b)[2] = (a)[2];					\
	} while (0)
#endif

#define VectorSet(v,a,b,c)						\
	do {								\
		(v)[0] = (a);						\
		(v)[1] = (b);						\
		(v)[2] = (c);						\
	} while (0)

#define VectorNegate(a,b)						\
	do {								\
		(b)[0] = -(a)[0];					\
		(b)[1] = -(a)[1];					\
		(b)[2] = -(a)[2];					\
	} while (0)

#define VectorScale(a,b,c)						\
	do {								\
		(c)[0] = (a)[0] * (b);					\
		(c)[1] = (a)[1] * (b);					\
		(c)[2] = (a)[2] * (b);					\
	} while (0)

/*	VectorMA(vec3_t,double,vec3_t,vec3_t)	*/
#define VectorMA(a,s,b,c)						\
	do {								\
		(c)[0] = (vec_t)((a)[0] + (s) * (b)[0]);		\
		(c)[1] = (vec_t)((a)[1] + (s) * (b)[1]);		\
		(c)[2] = (vec_t)((a)[2] + (s) * (b)[2]);		\
	} while (0)

vec_t VectorNormalize (vec3_t v);

vec_t Q_rint (vec_t in);

#endif	/* __MATHLIB_H */

