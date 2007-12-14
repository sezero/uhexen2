/*
	mathlib.c
	math primitives

	$Header: /home/ozzie/Download/0000/uhexen2/utils/common/mathlib.c,v 1.3 2007-12-14 16:41:16 sezero Exp $
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

void VectorMA (vec3_t va, double scale, vec3_t vb, vec3_t vc)
{
	vc[0] = (vec_t)(va[0] + scale*vb[0]);
	vc[1] = (vec_t)(va[1] + scale*vb[1]);
	vc[2] = (vec_t)(va[2] + scale*vb[2]);
}

vec_t VectorNormalize (vec3_t v)
{
	int		i;
	double	length;

	length = VectorLength(v);
	if (length == 0)
		return 0;

	for (i = 0; i < 3; i++)
		v[i] /= length;

	return (vec_t)length;
}

void VectorScale (vec3_t v, vec_t scale, vec3_t out)
{
	out[0] = v[0] * scale;
	out[1] = v[1] * scale;
	out[2] = v[2] * scale;
}

