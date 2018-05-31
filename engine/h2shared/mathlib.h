/*
 * mathlib.h -- math primitives
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* matches value in gcc v2 math.h */
#endif

#define	nanmask		(255 << 23)	/* 7F800000 */
#if 0	/* macro is violating strict aliasing rules */
#define	IS_NAN(x)	(((*(int *) (char *) &x) & nanmask) == nanmask)
#else
static inline int IS_NAN (float x) {
	union { float f; int i; } num;
	num.f = x;
	return ((num.i & nanmask) == nanmask);
}
#endif
int Q_isnan (float x);	/* For 32 bit floats only. */

extern vec3_t vec3_origin;

#define VectorCompare(v1,v2)	(((v1)[0] == (v2)[0]) && ((v1)[1] == (v2)[1]) && ((v1)[2] == (v2)[2]))
#define DotProduct(x,y)		((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
#define VectorLength(a)		sqrt(DotProduct((a),(a)))
#define DotProductDBL(x,y)	((double)(x)[0] * (double)(y)[0] + \
				 (double)(x)[1] * (double)(y)[1] + \
				 (double)(x)[2] * (double)(y)[2])
#define VectorLengthDBL(a)	sqrt(DotProductDBL((a), (a)))

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

#if 0
#define VectorNegate(a,b)	VectorSubtract(vec3_origin,(a),(b))
#else
#define VectorNegate(a,b)						\
	do {								\
		(b)[0] = -(a)[0];					\
		(b)[1] = -(a)[1];					\
		(b)[2] = -(a)[2];					\
	} while (0)
#endif

#define VectorScale(a,b,c)						\
	do {								\
		(c)[0] = (a)[0] * (b);					\
		(c)[1] = (a)[1] * (b);					\
		(c)[2] = (a)[2] * (b);					\
	} while (0)

#define VectorMA(a,s,b,c)						\
	do {								\
		(c)[0] = (a)[0] + (s) * (b)[0];				\
		(c)[1] = (a)[1] + (s) * (b)[1];				\
		(c)[2] = (a)[2] + (s) * (b)[2];				\
	} while (0)

static inline float _inl_VectorNormalize (vec3_t v);	/* returns vector length */
static inline float _inl_VectorNormalize (vec3_t v)
{
	float	length = VectorLength(v);
	if (length)
	{
		float	ilength = 1.0 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
	return length;
}
#define VectorNormalize(a)	_inl_VectorNormalize((a))

int Q_log2(int val);

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

void FloorDivMod (double numer, double denom, int *quotient, int *rem);
int GreatestCommonDivisor (int i1, int i2);

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
float anglemod (float a);

struct mplane_s;
ASM_LINKAGE_BEGIN
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
ASM_LINKAGE_END

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)				\
	(((p)->type < 3) ?						\
		( ((p)->dist <= (emins)[(p)->type]) ?	1  :		\
			( ((p)->dist >= (emaxs)[(p)->type]) ? 2 : 3 ) )	\
		:							\
			BoxOnPlaneSide((emins), (emaxs), (p)))

ASM_LINKAGE_BEGIN
fixed16_t Invert24To16 (fixed16_t val);
ASM_LINKAGE_END

#endif	/* __MATHLIB_H */

