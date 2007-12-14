/*
	mathlib.h
	math primitives

	$Header: /home/ozzie/Download/0000/uhexen2/utils/common/mathlib.h,v 1.5 2007-12-14 16:41:16 sezero Exp $
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
	(b)[0] = (a)[0];	\
	(b)[1] = (a)[1];	\
	(b)[2] = (a)[2];	\
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

void VectorMA (vec3_t va, double scale, vec3_t vb, vec3_t vc);

vec_t VectorNormalize (vec3_t v);
void VectorScale (vec3_t v, vec_t scale, vec3_t out);

vec_t Q_rint (vec_t in);

#endif	/* __MATHLIB_H */

