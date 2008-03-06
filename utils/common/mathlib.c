/*
	mathlib.c
	math primitives

	$Header: /home/ozzie/Download/0000/uhexen2/utils/common/mathlib.c,v 1.4 2008-03-06 18:55:07 sezero Exp $
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

