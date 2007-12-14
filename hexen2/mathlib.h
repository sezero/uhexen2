/*
	mathlib.h
	math primitives

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/mathlib.h,v 1.21 2007-12-14 16:41:10 sezero Exp $
*/

#ifndef __MATHLIB_H
#define __MATHLIB_H

#include <math.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* matches value in gcc v2 math.h */
#endif

#define	nanmask		(255<<23)
#define	IS_NAN(x)	(((*(int *) (char *) &x) & nanmask) == nanmask)

int Q_isnan (float x);	/* don't pass doubles to this */

extern vec3_t vec3_origin;

#define VectorCompare(v1,v2)	( ((v1)[0] == (v2)[0]) && ((v1)[1] == (v2)[1]) && ((v1)[2] == (v2)[2]) )
#define DotProduct(x,y)		( (x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] )
#define VectorLength(a)		sqrt(DotProduct((a),(a)))
#define CrossProduct(v1,v2,cross) { \
	(cross)[0] = (v1)[1]*(v2)[2] - (v1)[2]*(v2)[1]; \
	(cross)[1] = (v1)[2]*(v2)[0] - (v1)[0]*(v2)[2]; \
	(cross)[2] = (v1)[0]*(v2)[1] - (v1)[1]*(v2)[0]; \
}
#define VectorAdd(a,b,c) { \
	(c)[0] = (a)[0] + (b)[0]; \
	(c)[1] = (a)[1] + (b)[1]; \
	(c)[2] = (a)[2] + (b)[2]; \
}
#define VectorSubtract(a,b,c) { \
	(c)[0] = (a)[0] - (b)[0]; \
	(c)[1] = (a)[1] - (b)[1]; \
	(c)[2] = (a)[2] - (b)[2]; \
}
#define VectorInverse(v) { \
	(v)[0] = -(v)[0];	\
	(v)[1] = -(v)[1];	\
	(v)[2] = -(v)[2];	\
}
#define VectorCopy(a,b) { \
	memcpy((b), (a), sizeof(vec3_t)); \
}
#define VectorSet(vec,a,b,c) {	\
	(vec)[0] = (a);		\
	(vec)[1] = (b);		\
	(vec)[2] = (c);		\
}
#define VectorClear(a) {	\
	(a)[0] = (a)[1] = (a)[2] = 0;	\
}
#define VectorNegate(a,b) {	\
	(b)[0] = -(a)[0];	\
	(b)[1] = -(a)[1];	\
	(b)[2] = -(a)[2];	\
}

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);

static inline float VectorNormalize (vec3_t v);		// returns vector length
static inline void VectorScale (vec3_t in, vec_t scale, vec3_t out);

int Q_log2(int val);

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4]);

void FloorDivMod (double numer, double denom, int *quotient, int *rem);
fixed16_t Invert24To16(fixed16_t val);
int GreatestCommonDivisor (int i1, int i2);

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
float	anglemod(float a);

struct mplane_s;
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(								\
		((p)->dist <= (emins)[(p)->type])?	\
			1						\
		:							\
		(							\
			((p)->dist >= (emaxs)[(p)->type])?\
				2					\
			:						\
				3					\
		)							\
	)								\
	:								\
		BoxOnPlaneSide( (emins), (emaxs), (p)))

static inline float VectorNormalize (vec3_t v)
{
	float	length, ilength;

	length = VectorLength(v);

	if (length)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

static inline void VectorScale (vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

#endif	/* __MATHLIB_H */

