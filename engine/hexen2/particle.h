/*
	particle.h
	particle enums and types: note that hexen2 and
	hexenworld versions of these are different !!!

	$Header: /cvsroot/uhexen2/engine/hexen2/particle.h,v 1.1 2007-07-28 12:28:14 sezero Exp $
*/

#ifndef __PARTICLE_H
#define __PARTICLE_H

#define	PARTICLE_Z_CLIP		8.0

typedef enum
{
	pt_static,
	pt_grav,
	pt_fastgrav,
	pt_slowgrav,
	pt_fire,
	pt_explode,
	pt_explode2,
	pt_blob,
	pt_blob2,
	pt_rain,
	pt_c_explode,
	pt_c_explode2,
	pt_spit,
	pt_fireball,
	pt_ice,
	pt_spell,
	pt_test,
	pt_quake,
	pt_rd,			/* rider's death */
	pt_vorpal,
	pt_setstaff,
	pt_magicmissile,
	pt_boneshard,
	pt_scarab,
	pt_acidball,
	pt_darken,
	pt_snow,
	pt_gravwell,
	pt_redfire
} ptype_t;

typedef enum
{
	rt_rocket_trail = 0,
	rt_smoke,
	rt_blood,
	rt_tracer,
	rt_slight_blood,
	rt_tracer2,
	rt_voor_trail,
	rt_fireball,
	rt_ice,
	rt_spit,
	rt_spell,
	rt_vorpal,
	rt_setstaff,
	rt_magicmissile,
	rt_boneshard,
	rt_scarab,
	rt_acidball,
	rt_bloodshot
} rt_type_t;

typedef struct particle_s
{
/* driver-usable fields */
	vec3_t		org;
	float		color;
/* drivers never touch the following fields */
	struct particle_s	*next;
	vec3_t		vel;
	vec3_t		min_org;
	vec3_t		max_org;
	float		ramp;
	float		die;
	byte		type;
	byte		flags;
	byte		count;
} particle_t;

#endif	/* __PARTICLE_H */

