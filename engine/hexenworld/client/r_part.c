/*
 * r_part.c -- particles rendering
 * $Id$
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


#include "quakedef.h"
#include "r_local.h"


#define	MAX_PARTICLES		2048	// default max # of particles at one time
#define	ABSOLUTE_MIN_PARTICLES	512	// no fewer than this no matter what's
					// on the command line

//=============================================================================

static int ramp1[8]  = { 416,   416+2, 416+4, 416+6,  416+8,  416+10, 416+12, 416+14 };
static int ramp2[8]  = { 384+4, 384+6, 384+8, 384+10, 384+12, 384+13, 384+14, 384+15 };
static int ramp3[8]  = { 0x6d, 0x6b, 6, 5, 4, 3 };
static int ramp4[16] = { 416, 416+1, 416+2, 416+3, 416+4, 416+5, 416+6, 416+7, 416+8, 416+9, 416+10, 416+11, 416+12, 416+13, 416+14, 416+15 };
static int ramp5[16] = { 400, 400+1, 400+2, 400+3, 400+4, 400+5, 400+6, 400+7, 400+8, 400+9, 400+10, 400+11, 400+12, 400+13, 400+14, 400+15 };
static int ramp6[16] = { 256, 256+1, 256+2, 256+3, 256+4, 256+5, 256+6, 256+7, 256+8, 256+9, 256+10, 256+11, 256+12, 256+13, 256+14, 256+15 };
static int ramp7[16] = { 384, 384+1, 384+2, 384+3, 384+4, 384+5, 384+6, 384+7, 384+8, 384+9, 384+10, 384+11, 384+12, 384+13, 384+14, 384+15 };
static int ramp8[16] = { 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 13, 14, 15, 16, 17, 18 };
static int ramp9[16] = { 416, 416+1, 416+2, 416+3, 416+4, 416+5, 416+6, 416+7, 416+8, 416+9, 416+10, 416+11, 416+12, 416+13, 416+14, 416+15 };
// MISSION PACK
static int ramp10[16] ={ 432, 432+1, 432+2, 432+3, 432+4, 432+5, 432+6, 432+7, 432+8, 432+9, 432+10, 432+11, 432+12, 432+13, 432+14, 432+15 };
static int ramp11[8] = { 424, 424+1, 424+2, 424+3, 424+4, 424+5, 424+6, 424+7 };
static int ramp12[8] = { 136, 137, 138, 139, 140, 141, 142, 143 };
// HEXENWORLD
static int ramp13[16] ={ 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159 };

//=============================================================================

particle_t	*active_particles, *free_particles;
particle_t	*particles;
static int	r_numparticles;

vec3_t		r_pright, r_pup, r_ppn;
static	vec3_t	rider_origin;

static	cvar_t	leak_color = {"leak_color", "251", CVAR_ARCHIVE};


static particle_t *AllocParticle (void);

void R_RunParticleEffect2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, ptype_t effect, int count);
void R_RunParticleEffect3 (vec3_t org, vec3_t box, int color, ptype_t effect, int count);
void R_RunParticleEffect4 (vec3_t org, float radius, int color, ptype_t effect, int count);

//=============================================================================


/*
===============
R_InitParticles
===============
*/
void R_InitParticles (void)
{
	int		i;

	i = COM_CheckParm ("-particles");

	if (i && i < com_argc-1)
	{
		r_numparticles = atoi(com_argv[i+1]);
		if (r_numparticles < ABSOLUTE_MIN_PARTICLES)
			r_numparticles = ABSOLUTE_MIN_PARTICLES;
	}
	else
	{
		r_numparticles = MAX_PARTICLES;
	}

	particles = (particle_t *) Hunk_AllocName (r_numparticles * sizeof(particle_t), "particles");

	Cvar_RegisterVariable (&leak_color);
}


void R_DarkFieldParticles (entity_t *ent)
{
	int		i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;
	vec3_t		org;

	org[0] = ent->origin[0];
	org[1] = ent->origin[1];
	org[2] = ent->origin[2];

	for (i = -16; i < 16; i += 8)
	{
		for (j = -16; j < 16; j += 8)
		{
			for (k = 0; k < 32; k += 8)
			{
				p = AllocParticle();
				if (!p)
					return;

				p->die = cl.time + 0.2 + (rand() & 7) * 0.02;
				p->color = 150 + (rand() % 6);
				p->type = pt_slowgrav;

				dir[0] = j * 8;
				dir[1] = i * 8;
				dir[2] = k * 8;

				p->org[0] = org[0] + i + (rand() & 3);
				p->org[1] = org[1] + j + (rand() & 3);
				p->org[2] = org[2] + k + (rand() & 3);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}


/*
===============
AllocParticle
===============
*/
static particle_t *AllocParticle (void)
{
	particle_t	*p;

	if (!free_particles)
		return NULL;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	return p;
}

/*
===============
R_ClearParticles
===============
*/
void R_ClearParticles (void)
{
	int		i;

	if (!r_numparticles)
		return;
	free_particles = &particles[0];
	active_particles = NULL;

	for (i = 0; i < r_numparticles-1; i++)
		particles[i].next = &particles[i+1];
	particles[r_numparticles-1].next = NULL;
}


void R_ReadPointFile_f (void)
{
	FILE	*f;
	vec3_t	org;
	int		r;
	int		c;
	particle_t	*p;
	char	name[MAX_QPATH];
	byte	color;

	if (cls.state != ca_active)
		return; // need an active map.

	color = (byte)Cvar_VariableValue("leak_color");
	q_snprintf (name, sizeof(name), "maps/%s.pts", cl.mapname);

	FS_OpenFile (name, &f, NULL);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}

	Con_Printf ("Reading %s...\n", name);
	c = 0;
	VectorClear (org); // silence pesky compiler warnings
	for ( ;; )
	{
		r = fscanf (f,"%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;

		p = AllocParticle();
		if (!p)
		{
			Con_Printf ("Not enough free particles\n");
			break;
		}

		p->die = 99999;
		p->color = color; // (-c)&15;
		p->type = pt_static;
		VectorClear (p->vel);
		VectorCopy (org, p->org);
	}

	fclose (f);
	Con_Printf ("%i points read\n", c);
}


/*
===============
R_EntityParticles
===============
*/

#if defined(GLQUAKE)  /* otherwise from r_alias.c (r_shared.h) */
#define NUMVERTEXNORMALS 162
static const float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};
#endif
static	vec3_t	avelocities[NUMVERTEXNORMALS];
static	float	beamlength = 16;

void R_EntityParticles (entity_t *ent)
{
	int		i;
	particle_t	*p;
	float		angle, dist;
	float		sp, sy, cp, cy;
	vec3_t		forward;

	dist = 64;

	if (!avelocities[0][0])
	{
		for (i = 0; i < NUMVERTEXNORMALS; i++)
		{
			avelocities[i][0] = (rand() & 255) * 0.01;
			avelocities[i][1] = (rand() & 255) * 0.01;
			avelocities[i][2] = (rand() & 255) * 0.01;
		}
	}

	for (i = 0; i < NUMVERTEXNORMALS; i++)
	{
		angle = cl.time * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = cl.time * avelocities[i][2];

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 0.01;
		p->color = 0x6f;
		p->type = pt_explode;

		p->org[0] = ent->origin[0] + r_avertexnormals[i][0]*dist + forward[0]*beamlength;
		p->org[1] = ent->origin[1] + r_avertexnormals[i][1]*dist + forward[1]*beamlength;
		p->org[2] = ent->origin[2] + r_avertexnormals[i][2]*dist + forward[2]*beamlength;
	}
}

void R_SuccubusInvincibleParticles (entity_t *ent)
{
	int		count;
	particle_t	*p;
	vec3_t		ent_angles,forward,org;

	ent_angles[0] = ent_angles[2] = 0;
	ent_angles[1] = cl.time * 12;
	forward[0] = cos(ent_angles[1]) * 32;
	forward[1] = sin(ent_angles[1]) * 32;
	forward[2] = 0;
	VectorCopy(ent->origin, org);
	org[2] += 28;

	count = 140 * host_frametime;
	while (count > 0)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->ramp = 0;
		p->die = cl.time + 2;
		p->color = 416;
		p->type = pt_fireball;

		p->org[0] = org[0] + forward[0] + (rand() % 4) - 2;
		p->org[1] = org[1] + forward[1] + (rand() % 4) - 2;
		p->org[2] = org[2] + forward[2] + (rand() % 4) - 2;
		p->vel[0] = (rand() % 20) - 10;
		p->vel[1] = (rand() % 20) - 10;
		p->vel[2] = (rand() % 25) + 20;
		count--;
	}

	count = 60 * host_frametime;
	while (count > 0)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->ramp = 0;
		p->die = cl.time + 2;
		p->color = 135;
		p->type = pt_redfire;

		p->org[0] = org[0] + forward[0] + (rand() % 4) - 2;
		p->org[1] = org[1] + forward[1] + (rand() % 4) - 2;
		p->org[2] = org[2] + forward[2] + (rand() % 4) - 2;
		p->vel[0] = (rand() % 20) - 10;
		p->vel[1] = (rand() % 20) - 10;
		p->vel[2] = 0;
		count--;
	}
}


/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect (void)
{
	vec3_t		org, dir;
	int		i, count, msgcount, color;

	for (i = 0; i < 3; i++)
		org[i] = MSG_ReadCoord ();
	for (i = 0; i < 3; i++)
		dir[i] = MSG_ReadChar () * (1.0/16);
	msgcount = MSG_ReadByte ();
	color = MSG_ReadByte ();

	if (msgcount == 255)
		count = 1024;
	else
		count = msgcount;

	R_RunParticleEffect (org, dir, color, count);
}

/*
===============
R_ParseParticleEffect2

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect2 (void)
{
	vec3_t		org, dmin, dmax;
	int		i, msgcount, color;
	ptype_t		effect;

	for (i = 0; i < 3; i++)
		org[i] = MSG_ReadCoord ();
	for (i = 0; i < 3; i++)
		dmin[i] = MSG_ReadFloat ();
	for (i = 0; i < 3; i++)
		dmax[i] = MSG_ReadFloat ();
	color = MSG_ReadShort ();
	msgcount = MSG_ReadByte ();
	effect = (ptype_t) MSG_ReadByte ();

	R_RunParticleEffect2 (org, dmin, dmax, color, effect, msgcount);
}

/*
===============
R_ParseParticleEffect3

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect3 (void)
{
	vec3_t		org, box;
	int		i, msgcount, color;
	ptype_t		effect;

	for (i = 0; i < 3; i++)
		org[i] = MSG_ReadCoord ();
	for (i = 0; i < 3; i++)
		box[i] = MSG_ReadByte ();
	color = MSG_ReadShort ();
	msgcount = MSG_ReadByte ();
	effect = (ptype_t) MSG_ReadByte ();

	R_RunParticleEffect3 (org, box, color, effect, msgcount);
}

/*
===============
R_ParseParticleEffect4

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect4 (void)
{
	vec3_t		org;
	int		i, msgcount, color;
	ptype_t		effect;
	float		radius;

	for (i = 0; i < 3; i++)
		org[i] = MSG_ReadCoord ();
	radius = MSG_ReadByte();
	color = MSG_ReadShort ();
	msgcount = MSG_ReadByte ();
	effect = (ptype_t) MSG_ReadByte ();

	R_RunParticleEffect4 (org, radius, color, effect, msgcount);
}

void R_ParseRainEffect(void)
{
	vec3_t		org, e_size;
	short		color, count;
	int		x_dir, y_dir;

	org[0] = MSG_ReadCoord();
	org[1] = MSG_ReadCoord();
	org[2] = MSG_ReadCoord();
	e_size[0] = MSG_ReadCoord();
	e_size[1] = MSG_ReadCoord();
	e_size[2] = MSG_ReadCoord();
	x_dir = MSG_ReadAngle();
	y_dir = MSG_ReadAngle();
	color = MSG_ReadShort();
	count = MSG_ReadShort();

	R_RainEffect (org, e_size, x_dir, y_dir, color, count);
}


/*
===============
R_ParticleExplosion

===============
*/
void R_ParticleExplosion (vec3_t org)
{
	int		i, j;
	particle_t	*p;

	for (i = 0; i < 1024; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 5;
		p->color = ramp1[0];
		p->ramp = rand() & 3;
		if (i & 1)
		{
			p->type = pt_explode;
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() % 32) - 16);
				p->vel[j] = (rand() % 512) - 256;
			}
		}
		else
		{
			p->type = pt_explode2;
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() % 32) - 16);
				p->vel[j] = (rand() % 512) - 256;
			}
		}
	}
}

/*
===============
R_ParticleExplosion2
color mapped explosion
===============
*/
void R_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength)
{
	int		i, j;
	particle_t	*p;
	int		colorMod = 0;

	for (i = 0; i < 512; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 0.3;
		p->color = colorStart + (colorMod % colorLength);
		colorMod++;

		p->type = pt_blob;
		for (j = 0; j < 3; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_BlobExplosion
tarbaby explosion
===============
*/
void R_BlobExplosion (vec3_t org)
{
	int		i, j;
	particle_t	*p;

	for (i = 0; i < 1024; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 1 + (rand() & 8) * 0.05;

		if (i & 1)
		{
			p->type = pt_blob;
			p->color = 66 + (rand() % 6);
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() % 32) - 16);
				p->vel[j] = (rand() % 512) - 256;
			}
		}
		else
		{
			p->type = pt_blob2;
			p->color = 150 + (rand() % 6);
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() % 32) - 16);
				p->vel[j] = (rand() % 512) - 256;
			}
		}
	}
}

/*
===============
R_RunParticleEffect

===============
*/
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, j;
	particle_t	*p;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		if (count == 1024)
		{	// rocket explosion
			p->die = cl.time + 5;
			p->color = ramp1[0];
			p->ramp = rand() & 3;
			if (i & 1)
			{
				p->type = pt_explode;
				for (j = 0; j < 3; j++)
				{
					p->org[j] = org[j] + ((rand() % 32) - 16);
					p->vel[j] = (rand() % 512) - 256;
				}
			}
			else
			{
				p->type = pt_explode2;
				for (j = 0; j < 3; j++)
				{
					p->org[j] = org[j] + ((rand() % 32) - 16);
					p->vel[j] = (rand() % 512) - 256;
				}
			}
		}
		else
		{
			p->die = cl.time + 0.1 * (rand() % 5);
//			p->color = (color & ~7) + (rand() & 7);
//			p->color = 265 + (rand() % 9);
			p->color = 256 + 16 + 12 + (rand() % 4);
			p->type = pt_slowgrav;
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() & 15) - 8);
				p->vel[j] = dir[j] * 15;// + (rand() % 300) - 150;
			}
		}
	}
}

/*
===============
R_RunParticleEffect2

===============
*/
void R_RunParticleEffect2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, ptype_t effect, int count)
{
	int		i, j;
	particle_t	*p;
	float		num;
//	float		num2;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

	//	p->die = cl.time + 0.1 * (rand() % 5);
		p->die = cl.time + 2;
		p->color = color;
		p->type = effect;
		p->ramp = 0;
		for (j = 0; j < 3; j++)
		{
			num = rand() / RAND_MAX;
			p->org[j] = org[j] + ((rand() & 15) - 8);
			p->vel[j] = dmin[j] + ((dmax[j] - dmin[j]) * num);
		}

		/*
		num = rand() / RAND_MAX;
		num2 = (int)(host_time * 20)%25 + 10;
		num2 = (int)(host_time * 20)%10 + 10;
	//	num2 = rand() / RAND_MAX * 5;
		p->org[0] = org[0] + cos(num * 2 * M_PI)*num2;
		p->org[1] = org[1] + sin(num * 2 * M_PI)*num2;
		p->org[2] = org[2];
		*/
	}
}

/*
===============
R_RunParticleEffect3

===============
*/
void R_RunParticleEffect3 (vec3_t org, vec3_t box, int color, ptype_t effect, int count)
{
	int		i, j;
	particle_t	*p;
	float		num;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

	//	p->die = cl.time + 0.1 * (rand() % 5);
		p->die = cl.time + 2;
		p->color = color;
		p->type = effect;
		p->ramp = 0;
		for (j = 0; j < 3; j++)
		{
			num = rand() / RAND_MAX;
			p->org[j] = org[j] + ((rand() & 15) - 8);
			p->vel[j] = (box[j] * num * 2) - box[j];
		}
	}
}

/*
===============
R_RunParticleEffect4

===============
*/
void R_RunParticleEffect4 (vec3_t org, float radius, int color, ptype_t effect, int count)
{
	int		i, j;
	particle_t	*p;
	float		num;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

	//	p->die = cl.time + 0.1 * (rand() % 5);
		p->die = cl.time + 2;
		p->color = color;
		p->type = effect;
		p->ramp = 0;
		for (j = 0; j < 3; j++)
		{
			num = rand() / RAND_MAX;
			p->org[j] = org[j] + ((rand() & 15) - 8);
			p->vel[j] = (radius * num * 2) - radius;
		}
	}
}

void R_SplashParticleEffect (vec3_t org, float radius, int color, ptype_t effect, int count)
{
	int		i, j;
	particle_t	*p;
	float		num;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 2;
		p->color = color;
		p->type = effect;
		p->ramp = 0;
		for (j = 0; j < 3; j++)
		{
			num = rand() / RAND_MAX;
			if (j == 2)
			{
				p->vel[j] = (radius * num * 4) + radius;
				p->org[j] = org[j] + 3;
			}
			else
			{
				p->vel[j] = (radius * num * 2) - radius;
				p->org[j] = org[j] + ((rand() % 64) - 32);
			}
		}
	}
}

/*
===============
R_LavaSplash

===============
*/
void R_LavaSplash (vec3_t org)
{
	int		i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i = -16; i < 16; i++)
	{
		for (j = -16; j < 16; j++)
		{
			for (k = 0; k < 1; k++)
			{
				p = AllocParticle();
				if (!p)
					return;

				p->die = cl.time + 2 + (rand() & 31) * 0.02;
				p->color = 224 + (rand() & 7);
				p->type = pt_slowgrav;

				dir[0] = j*8 + (rand() & 7);
				dir[1] = i*8 + (rand() & 7);
				dir[2] = 256;

				p->org[0] = org[0] + dir[0];
				p->org[1] = org[1] + dir[1];
				p->org[2] = org[2] + (rand() & 63);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}


/*
===============
R_TargetBallEffect

===============
*/
void R_TargetBallEffect (vec3_t org)
{
	int		i;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i = 0; i < 40 * host_frametime; i++)
	{
		p = AllocParticle();
		if (!p)
			return;
		if (v_targDist < 60)
		{
			p->die = cl.time + (rand() & 3) * 0.02 + (.23 * (1.0 - (.23 * (v_targDist - 24.0)/36.0)));
		}
		else
		{
			p->die = cl.time + (.3 * ((256.0 - v_targDist)/256.0)) + (rand() & 7) * 0.02;
		}
		p->color = 7 + (rand() % 24);
		p->type = pt_slowgrav;

		dir[0] = (rand() & 63) - 31;
		dir[1] = (rand() & 63) - 31;
		dir[2] = 256;

		p->org[0] = org[0] + (rand() & 3) - 2;
		p->org[1] = org[1] + (rand() & 3) - 2;
		p->org[2] = org[2] + (rand() & 3);

		VectorNormalize (dir);
		vel = 50 + (rand() & 63);
		VectorScale (dir, vel, p->vel);
	}
}


/*
===============
R_BrightFieldSource

===============
*/
void R_BrightFieldSource (vec3_t org)
{
	int		i;
	particle_t	*p;
	float		vel;
	vec3_t		dir;
	float		height;

	height = cos(cl.time * 4.0) * 25;

	for (i = 0; i < 120 * host_frametime ; i++)
	{
		p = AllocParticle();
		if (!p)
			return;
		p->die = cl.time + .5;
		p->color = 143;
		//p->type = pt_slowgrav;
		p->type = pt_quake;

		dir[0] = (rand() % 256) - 128;
		dir[1] = (rand() % 256) - 128;
		dir[2] = 32;

		p->org[0] = org[0] + (rand() & 15) - 7;
		p->org[1] = org[1] + (rand() & 15) - 7;
	//	p->org[2] = org[2] + (rand() & 3);
		p->org[2] = org[2] + height;

		VectorNormalize (dir);
		vel = 70 + (rand() & 31);
		VectorScale (dir, vel, p->vel);
	}

	for (i = 0; i < 120 * host_frametime; i++)
	{
		p = AllocParticle();
		if (!p)
			return;
		p->die = cl.time + .5;
		p->color = 159;
		//p->type = pt_slowgrav;
		p->type = pt_quake;

		dir[0] = (rand() % 256) - 128;
		dir[1] = (rand() % 256) - 128;
		dir[2] = 32;

		p->org[0] = org[0] + (rand() & 15) - 7;
		p->org[1] = org[1] + (rand() & 15) - 7;
	//	p->org[2] = org[2] + (rand() & 3);
		p->org[2] = org[2] - height;

		VectorNormalize (dir);
		vel = 70 + (rand() & 31);
		VectorScale (dir, vel, p->vel);
	}
}

/*
===============
R_TeleportSplash

===============
*/
void R_TeleportSplash (vec3_t org)
{
	int		i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i = -16; i < 16; i += 4)
	{
		for (j = -16; j < 16; j += 4)
		{
			for (k = -24; k < 32; k += 4)
			{
				p = AllocParticle();
				if (!p)
					return;

				p->die = cl.time + 0.2 + (rand() & 7) * 0.02;
				p->color = 7 + (rand() & 7);
				p->type = pt_slowgrav;

				dir[0] = j * 8;
				dir[1] = i * 8;
				dir[2] = k * 8;

				p->org[0] = org[0] + i + (rand() & 3);
				p->org[1] = org[1] + j + (rand() & 3);
				p->org[2] = org[2] + k + (rand() & 3);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}

/*
===============
R_RunQuakeEffect

===============
*/
void R_RunQuakeEffect (vec3_t org, float distance)
{
	int		i;
	particle_t	*p;
	float		num, num2;
//	int		j;

	for (i = 0; i < 100; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 0.3 * (rand() % 5);
		p->color = (rand() % 4) + ((rand() % 3) * 16) + (13 * 16) + 256 + 11;
		p->type = pt_quake;
		p->ramp = 0;

		num = rand() / RAND_MAX;
		num2 = distance * num;
		num = rand() / RAND_MAX;
		p->org[0] = org[0] + cos(num * 2 * M_PI)*num2;
		p->org[1] = org[1] + sin(num * 2 * M_PI)*num2;
		num = rand() / RAND_MAX;
		p->org[2] = org[2] + 15*num;
		p->org[2] = org[2];

		num = rand() / RAND_MAX;
		p->vel[0] = (num * 40) - 20;
		num = rand() / RAND_MAX;
		p->vel[1] = (num * 40) - 20;
		num = rand() / RAND_MAX;
		p->vel[2] = 65*num + 80;

		/*
		for (j = 0; j < 2; j++) {
			p->vel[j] = dmin[j] + ((dmax[j] - dmin[j]) * num);
		}
		*/
	}
}

/*
===============
R_SunStaffTrail

===============
*/
void R_SunStaffTrail(vec3_t source, vec3_t dest)
{
	int		i;
	particle_t	*p;
	vec3_t		vec, dist;
	float		length, size;

	VectorSubtract(dest, source, vec);
	length = VectorNormalize(vec);
	dist[0] = vec[0];
	dist[1] = vec[1];
	dist[2] = vec[2];

	size = 10;

	while (length > 0)
	{
		length -= size;

		if ((p = AllocParticle()) == NULL)
			return;

		p->die = cl.time + 2;

		p->ramp = rand() & 3;
		p->color = ramp6[(int)(p->ramp)];

		p->type = pt_spit;

		for (i = 0; i < 3; i++)
		{
			p->org[i] = source[i] + ((rand() % 4) - 2);
		}

		p->vel[0] = (rand() % 10) - 5;
		p->vel[1] = (rand() % 10) - 5;
		p->vel[2] = (rand() % 10);

		VectorAdd(source, dist, source);
	}
}

void RiderParticle (int count, vec3_t origin)
{
	int		i;
	particle_t	*p;
//	float	num, num2;
	float	radius, angle;

	VectorCopy(origin, rider_origin);

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 4;
		p->color = 256 + 16 + 15;
		p->type = pt_rd;
		p->ramp = 0;

		VectorCopy(origin, p->org);

		//num = rand() / RAND_MAX;
		angle = (rand() % 360) / (2 * M_PI);
		radius = 300 + (rand() % 256);
		p->org[0] += sin(angle) * radius;
		p->org[1] += cos(angle) * radius;
		p->org[2] += (rand() & 255) - 30;

		p->vel[0] = (rand() & 255) - 127;
		p->vel[1] = (rand() & 255) - 127;
		p->vel[2] = (rand() & 255) - 127;
	}
}

/*
===============
R_RocketTrail

===============
*/
void R_RocketTrail (vec3_t start, vec3_t end, int type)
{
	vec3_t	vec, dist;
	float	len, size, lifetime;
	int			j;
	particle_t	*p;
	static int tracercount;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	dist[0] = vec[0];
	dist[1] = vec[1];
	dist[2] = vec[2];
	size = 1;
	lifetime = 2;
	switch (type)
	{
		case 9:
		// Spit
			break;

		case 8:
		// Ice
			size = 5 * 3;
			break;

		case rt_acidball:
			size = 5;
			lifetime = .8;
			break;

		case rt_grensmoke:
			size = 5;
			break;

		case rt_purify:
			size = 5;
			lifetime = .5;
			break;

		default:
			size = 3;
			break;

	}
	VectorScale(dist, size, dist);

	while (len > 0)
	{
		len -= size;

		p = AllocParticle();
		if (!p)
			return;

		VectorClear (p->vel);
		p->die = cl.time + lifetime;

		switch (type)
		{
			case rt_grensmoke:
			//smoke trail for grenade
				p->color = 283 + (rand() & 3);
				p->type = pt_grensmoke;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 6) - 3);
				break;

			case rt_rocket_trail:
			// rocket trail
				p->ramp = rand() & 3;
				p->color = ramp3[(int)p->ramp];
				p->type = pt_fire;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 6) - 3);
				break;

			case rt_smoke:
			// smoke smoke
				p->ramp = (rand() & 3) + 2;
				p->color = ramp3[(int)p->ramp];
				p->type = pt_fire;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 6) - 3);
				break;

			case rt_blood:
			// blood
				//p->type = pt_slowgrav;
				//p->color = 138 + (rand() & 3);
				//for (j = 0; j < 3; j++)
				//	p->org[j] = start[j] + ((rand() % 6) - 3);

				p->type = pt_darken;
				p->color = 136 + (rand() % 5);
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() & 3) - 2);
				len -= size;
				break;

			case rt_tracer:;
			case rt_tracer2:;
			// tracer
				p->die = cl.time + 0.5;
				p->type = pt_static;
				if (type == 3)
					p->color = 130 + (rand() & 6);	// 243 + (rand() & 3);
				else
					p->color = 230 + ((tracercount & 4) << 1);

				tracercount++;

				VectorCopy (start, p->org);
				if (tracercount & 1)
				{
					p->vel[0] = 30 * vec[1];
					p->vel[1] = 30 * -vec[0];
				}
				else
				{
					p->vel[0] = 30 * -vec[1];
					p->vel[1] = 30 * vec[0];
				}
				break;

			case rt_slight_blood:
			// slight blood
				p->type = pt_slowgrav;
				p->color = 138 + (rand() & 3);
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 6) - 3);
				len -= size;
				break;

			case rt_bloodshot:
			// bloodshot trail
				p->type = pt_darken;
				p->color = 136 + (rand() & 5);
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() & 3) - 2);
				len -= size;
				break;

			case rt_voor_trail:
			// voor trail
				p->color = 9*16 + 8 + (rand() & 3);
				p->type = pt_static;
				p->die = cl.time + 0.3;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() & 15) - 8);
				break;

			case rt_fireball:
			// Fireball
				p->ramp = rand() & 3;
				p->color = ramp4[(int)(p->ramp)];
				p->type = pt_fireball;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 4) - 2);
				p->org[2] += 2;	// compensate for model
				p->vel[0] = (rand() % 200) - 100;
				p->vel[1] = (rand() % 200) - 100;
				p->vel[2] = (rand() % 200) - 100;
				break;

			case rt_acidball:
			// Acid ball
				p->ramp = rand() & 3;
				p->color = ramp10[(int)(p->ramp)];
				p->type = pt_acidball;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() & 3) - 2);
				p->org[2] += 2;	// compensate for model
				p->vel[0] = (rand() % 20) - 10;
				p->vel[1] = (rand() % 20) - 10;
				p->vel[2] = (rand() % 20) - 10;
				break;

			case rt_ice:
			// Ice
				p->ramp = rand() & 3;
				p->color = ramp5[(int)(p->ramp)];
				p->type = pt_ice;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 4) - 2);
				p->org[2] += 2;	// compensate for model
				p->vel[0] = (rand() % 16) - 8;
				p->vel[1] = (rand() % 16) - 8;
				p->vel[2] = (rand() % 20) - 40;
				break;

			case rt_spit:
			// Spit
				p->ramp = rand() & 3;
				p->color = ramp6[(int)(p->ramp)];
				p->type = pt_spit;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 4) - 2);
				p->org[2] += 2;	// compensate for model
				p->vel[0] = (rand() % 10) - 5;
				p->vel[1] = (rand() % 10) - 5;
				p->vel[2] = (rand() % 10);
				break;

			case rt_spell:
			// Spell
				p->ramp = rand() & 3;
				p->color = ramp6[(int)(p->ramp)];
				p->type = pt_spell;
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 4) - 2);
				p->vel[0] = (rand() % 10) - 5;
				p->vel[1] = (rand() % 10) - 5;
				p->vel[2] = (rand() % 10);
				p->vel[0] = vec[0] * -10;
				p->vel[1] = vec[1] * -10;
				p->vel[2] = vec[2] * -10;
				break;

			case rt_vorpal:
			// vorpal missile
				p->type = pt_vorpal;
				p->color = 44 + (rand() & 3) + 256;
				for (j = 0; j < 2; j++)
					p->org[j] = start[j] + ((rand() % 48) - 24);

				p->org[2] = start[2] + ((rand() % 16) - 8);

				break;

			case rt_setstaff:
			// set staff
				p->type = pt_setstaff;
				p->color = ramp9[0];
				p->ramp = rand() & 3;

				for (j = 0; j < 2; j++)
					p->org[j] = start[j] + ((rand() % 6) - 3);

				p->org[2] = start[2] + ((rand() % 10) - 5);

				p->vel[0] = (rand() % 8) - 4;
				p->vel[1] = (rand() % 8) - 4;
				break;

			case rt_purify:
			// purifier
				p->type = pt_setstaff;
				p->color = ramp9[0];
				p->ramp = rand() & 3;

				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + ((rand() % 3) - 1);

			//	p->org[2] = start[2] + ((rand() % 10) - 5);

			//	p->vel[0] = (rand() % 8) - 4;
			//	p->vel[1] = (rand() % 8) - 4;
				p->vel[0] = 0;
				p->vel[1] = 0;

				break;

			case rt_magicmissile:
			// magic missile
				p->type = pt_magicmissile;
				p->color = 148 + (rand() & 11);
				p->ramp = rand() & 3;
				for (j = 0; j < 2; j++)
					p->org[j] = start[j] + ((rand() % 48) - 24);

				p->org[2] = start[2] + ((rand() % 48) - 24);

				p->vel[2] = -((rand() % 16) + 8);
				break;

			case rt_boneshard:
			// bone shard
				p->type = pt_boneshard;
				p->color = 368 + (rand() & 16);
				for (j = 0; j < 2; j++)
					p->org[j] = start[j] + ((rand() % 48) - 24);

				p->org[2] = start[2] + ((rand() % 48) - 24);

				p->vel[2] = -((rand() % 16) + 8);
				break;
			case rt_scarab:
			// scarab staff
				p->type = pt_scarab;
				p->color = 250 + (rand() & 4);
				for (j = 0; j < 3; j++)
					p->org[j] = start[j] + (rand() % 7);

				p->vel[2] = -(rand() % 8);
				break;
		}

		VectorAdd (start, dist, start);
	}
}

/*
===============
R_RainEffect

===============
*/
void R_RainEffect (vec3_t org, vec3_t e_size, int x_dir, int y_dir, int color, int count)
{
	int		i, holdint;
	particle_t	*p;
	float		z_time;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->vel[0] = x_dir;	// X and Y motion
		p->vel[1] = y_dir;
		p->vel[2] = -(rand() % 956);
		if (p->vel[2] > -256)
		{
			p->vel[2] += -256;
		}

		z_time = -(e_size[2]/p->vel[2]);
		p->die = cl.time + z_time;
		p->color = color;
		p->ramp = rand() & 3;

		p->type = pt_rain;

		holdint = e_size[0];
		p->org[0] = org[0] + (rand() % holdint);
		holdint = e_size[1];
		p->org[1] = org[1] + (rand() % holdint);
		p->org[2] = org[2];
	}
}

/*
===============
R_RainEffect2

===============
*/
void R_RainEffect2 (vec3_t org, vec3_t e_size, int x_dir, int y_dir, int color, int count)
{
	int		i, holdint;
	particle_t	*p;
	float		z_time;

	for (i = 0; i < count; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->vel[0] = x_dir;	// X and Y motion
		p->vel[1] = y_dir;
		p->vel[2] = -(rand() % 500);
		if (p->vel[2] > -128)
		{
			p->vel[2] += -128;
		}

		z_time = -(e_size[2]/p->vel[2]);
		p->die = cl.time + z_time;
		p->color = color;
		p->ramp = rand() & 3;

		p->type = pt_rain;

		holdint = e_size[0];
		p->org[0] = org[0] + (rand() % holdint);
		holdint = e_size[1];
		p->org[1] = org[1] + (rand() % holdint);
		p->org[2] = org[2] + (rand() % 16);
	}
}

/*
===============
R_ColoredParticleExplosion

===============
*/
void R_ColoredParticleExplosion (vec3_t org, int color, int radius, int counter)
{
	int		i, j;
	particle_t	*p;

	for (i = 0; i < counter; i++)
	{
		p = AllocParticle();
		if (!p)
			return;

		p->die = cl.time + 3;
		p->color = color;
		p->ramp = rand() & 3;

		if (i & 1)
		{
			p->type = pt_c_explode;
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() % (radius * 2)) - radius);
				p->vel[j] = (rand() % 512) - 256;
			}
		}
		else
		{
			p->type = pt_c_explode2;
			for (j = 0; j < 3; j++)
			{
				p->org[j] = org[j] + ((rand() % (radius * 2)) - radius);
				p->vel[j] = (rand() % 512) - 256;
			}
		}
	}
}


#ifdef GLQUAKE
static qboolean		alphaTestEnabled;
static vec3_t		up, right;
#endif

static void R_RenderParticle (particle_t *p)
{
#ifdef GLQUAKE
	int		color;
	float		scale;
	unsigned char	*at;
	unsigned char	theAlpha;

	/* hexenworld progs seems to have abandoned particleexplosion()
	 * builtin usage, and added the following bounds check instead.
	 * disabling: clamping the color instead (see below.) -- O.S.
	if (p->color < 0 || p->color > 511)
	{
		Con_Printf("Invalid color for particle type %d\n",(int)p->type);
		return;
	}
	*/

	// hack a scale up to keep particles from disapearing
	scale = (p->org[0] - r_origin[0])*vpn[0] +
		(p->org[1] - r_origin[1])*vpn[1] +
		(p->org[2] - r_origin[2])*vpn[2];
	if (scale < 20)
		scale = 1;
	else
		scale = 1 + scale * 0.004;

	/* clamp color to 0-511: particle->type 10 and 11 (pt_c_explode
	 * and pt_c_explode2, e.g. Crusader's ice particles hitting a
	 * wall) lead to negative values, because R_UpdateParticles ()
	 * decrements their color against time.
	 * (Crusader ice particles seem to have changed in HW, but..) */
	color = ((int)p->color) & 0x01ff;
	if (color < 256)
	{
	//	glColor3ubv_fp ((byte *)&d_8to24table[color]);
		at = (byte *)&d_8to24table[color];
		if (p->type == pt_fire)
			theAlpha = 255 * (6 - p->ramp) / 6;
	//		theAlpha = 192;
	//	else if (p->type == pt_explode || p->type == pt_explode2)
	//		theAlpha = 255 * (8 - p->ramp)/8;
		else
			theAlpha = 255;
		glColor4ub_fp (*at, *(at + 1), *(at + 2), theAlpha);
	}
	else
	{
		glColor4ubv_fp ((byte *)&d_8to24TranslucentTable[color-256]);
	}

	glTexCoord2f_fp (0,0);
	glVertex3fv_fp (p->org);
	glTexCoord2f_fp (1,0);
	glVertex3f_fp (p->org[0] + up[0]*scale, p->org[1] + up[1]*scale, p->org[2] + up[2]*scale);
	glTexCoord2f_fp (0,1);
	glVertex3f_fp (p->org[0] + right[0]*scale, p->org[1] + right[1]*scale, p->org[2] + right[2]*scale);

#else
	/*
	if (p->color < 0 || p->color > 511)
	{
		Con_Printf("Invalid color for particle type %d\n",(int)p->type);
		return;
	}
	*/
	D_DrawParticle (p);
#endif
}

/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles (void)
{
	particle_t	*p, *kill, temp_p;
	float		grav, grav2, percent;
	int		i;
	float		time2, time3, time4;
	float		time1;
	float		dvel;
	float		frametime;
	float		vel0, vel1, vel2;
	vec3_t		diff;

#ifdef GLQUAKE
	GL_Bind(particletexture);
	alphaTestEnabled = glIsEnabled_fp(GL_ALPHA_TEST);

	if (alphaTestEnabled)
		glDisable_fp(GL_ALPHA_TEST);
	glEnable_fp (GL_BLEND);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin_fp (GL_TRIANGLES);

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);
#else
	D_StartParticles ();

	VectorScale (vright, xscaleshrink, r_pright);
	VectorScale (vup, yscaleshrink, r_pup);
	VectorCopy (vpn, r_ppn);
#endif

	frametime = host_frametime;
	time4 = frametime * 20;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * movevars.gravity * 0.05;
	grav2 = frametime * movevars.gravity * 0.025;
	dvel = 4 * frametime;
	percent = (frametime / HX_FRAME_TIME);

	/*
	temp_p.org[0] = -1013;
	temp_p.org[1] = -1863;
	temp_p.org[2] = 50;
	temp_p.color = 31;
	R_RenderParticle(&temp_p);
	*/
	for ( ;; )
	{
		kill = active_particles;
		if (kill && kill->die < cl.time)
		{
			active_particles = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			continue;
		}
		break;
	}

	for (p = active_particles ; p ; p = p->next)
	{
		for ( ;; )
		{
			kill = p->next;
			if (kill && kill->die < cl.time)
			{
				p->next = kill->next;
				kill->next = free_particles;
				free_particles = kill;
				continue;
			}
			break;
		}

		if (p->type == pt_rain)
		{
			temp_p = *p;

			vel0 = temp_p.vel[0]*.001;
			vel1 = temp_p.vel[1]*.001;
			vel2 = temp_p.vel[2]*.001;
			for (i = 0; i < 4; i++)
			{
				R_RenderParticle(&temp_p);

				temp_p.org[0] += vel0;
				temp_p.org[1] += vel1;
				temp_p.org[2] += vel2;
 			}
		}

		R_RenderParticle(p);
		p->org[0] += p->vel[0] * frametime;
		p->org[1] += p->vel[1] * frametime;
		p->org[2] += p->vel[2] * frametime;

		switch (p->type)
		{
		case pt_static:
			break;

		case pt_fire:
			p->ramp += time1;
			if ((int)p->ramp >= 6)
				p->die = -1;
			else
				p->color = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			break;

		case pt_explode:
			p->ramp += time2;
			if ((int)p->ramp >= 8)
				p->die = -1;
			else
				p->color = ramp1[(int)p->ramp];
			for (i = 0; i < 3; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_explode2:
			p->ramp += time3;
			if ((int)p->ramp >= 8)
				p->die = -1;
			else
				p->color = ramp2[(int)p->ramp];
			for (i = 0; i < 3; i++)
				p->vel[i] -= p->vel[i] * frametime;
			p->vel[2] -= grav;
			break;

		case pt_c_explode:
			p->ramp += time2;
			if ((int)p->ramp >= 8 || p->color <= 0)
				p->die = -1;
			else if (time2)
				p->color--;
			for (i = 0; i < 3; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_c_explode2:
			p->ramp += time3;
			if ((int)p->ramp >= 8 || p->color <= 1)
				p->die = -1;
			else if (time3)
				p->color -= 2;
			for (i = 0; i < 3; i++)
				p->vel[i] -= p->vel[i] * frametime;
			p->vel[2] -= grav;
			break;

		case pt_blob:
			for (i = 0; i < 3; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_blob2:
			for (i = 0; i < 2; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_grav:
#ifdef QUAKE2
			p->vel[2] -= grav * 20;
			break;
#endif
		case pt_slowgrav:
			p->vel[2] -= grav;
			break;

		case pt_grensmoke:
			p->vel[0] += time3 * ((rand() % 3) - 1);
			p->vel[1] += time3 * ((rand() % 3) - 1);
			p->vel[2] += time3 * ((rand() % 3) - 1);
			break;

		case pt_fastgrav:
			p->vel[2] -= grav * 4;
			break;

		case pt_rain:
			break;

		case pt_fireball:
			p->ramp += time3;
			if ((int)p->ramp >= 16)
				p->die = -1;
			else
				p->color = ramp4[(int)p->ramp];
			break;

		case pt_acidball:
			p->ramp += time4 * 1.4;
			if ((int)p->ramp >= 23)
				p->die = -1;
			else if ((int)p->ramp >= 15)
				p->color = ramp11[(int)p->ramp - 15];
			else
				p->color = ramp10[(int)p->ramp];
			p->vel[2] -= grav;
			break;

		case pt_spit:
			p->ramp += time3;
			if ((int)p->ramp >= 16)
				p->die = -1;
			else
				p->color = ramp6[(int)p->ramp];
		//	p->vel[2] += grav * 2;
			break;

		case pt_ice:
			p->ramp += time4;
			if ((int)p->ramp > 15)
				p->die = -1;
			else
				p->color = ramp5[(int)p->ramp];
			p->vel[2] -= grav;
			break;

		case pt_spell:
			p->ramp += time2;
			if ((int)p->ramp >= 16)
				p->die = -1;
			else
				p->color = ramp7[(int)p->ramp];
		//	p->vel[2] += grav * 2;
			break;

		case pt_test:
			p->vel[2] += 1.3;
			p->ramp += time3;
			if ((int)p->ramp >= 13 || ((int)p->ramp > 10 && p->vel[2] < 20) )
				p->die = -1;
			else
				p->color = ramp8[(int)p->ramp];
			break;

		case pt_quake:
			p->vel[0] *= 1.05;
			p->vel[1] *= 1.05;
			p->vel[2] -= grav * 4;
			if (p->color < 160 && p->color > 143)
				p->color = 152 + 7 * ((p->die - cl.time) * 2.0);
			if (p->color < 144 && p->color > 127)
				p->color = 136 + 7 * ((p->die - cl.time) * 2.0);
			break;

		case pt_rd:
			if (!frametime)
				break;

			p->ramp += percent;
			if ((int)p->ramp > 50)
			{
				p->ramp = 50;
				p->die = -1;
			}
			p->color = 256 + 16 + 16 - (p->ramp / (50/16));

			VectorSubtract(rider_origin, p->org, diff);

			/*
			p->org[0] += diff[0] * p->ramp / 80;
			p->org[1] += diff[1] * p->ramp / 80;
			p->org[2] += diff[2] * p->ramp / 80;
			*/
			vel0 = 1 / (51 - p->ramp);
			p->org[0] += diff[0] * vel0;
			p->org[1] += diff[1] * vel0;
			p->org[2] += diff[2] * vel0;

			break;

		case pt_vorpal:
			--p->color;
			if ((int)p->color <= 37 + 256)
				p->die = -1;
			break;

		case pt_setstaff:
			p->ramp += time1;
			if ((int)p->ramp >= 16)
				p->die = -1;
			else
				p->color = ramp9[(int)p->ramp];

			p->vel[0] *= 1.08 * percent;
			p->vel[1] *= 1.08 * percent;
			p->vel[2] -= grav2;
			break;

		case pt_redfire:
			p->ramp += frametime * 3;
			if ((int)p->ramp >= 8)
				p->die = -1;
			else
				p->color = ramp12[(int)p->ramp] + 256;

			p->vel[0] *= .9;
			p->vel[1] *= .9;
			p->vel[2] += grav/2;
			break;

		case pt_bluestep:
			p->ramp += frametime * 8;
			if ((int)p->ramp >= 16)
				p->die = -1;
			else
				p->color = ramp13[(int)p->ramp] + 256;

			p->vel[0] *= .9;
			p->vel[1] *= .9;
			p->vel[2] += grav;
			break;

		case pt_magicmissile:
			--p->color;
			if ((int)p->color < 149)
				p->color = 149;
			p->ramp += time1;
			if ((int)p->ramp > 16)
				p->die = -1;
			break;

		case pt_boneshard:
			--p->color;
			if ((int)p->color < 368)
				p->die = -1;
			break;

		case pt_scarab:
			--p->color;
			if ((int)p->color < 250)
				p->die = -1;
			break;

		case pt_darken:
		    {
			int	colindex;
			p->vel[2] -= grav * 2;	// Also gravity
			if (rand() & 1)
				--p->color;
			colindex = 0;
			while (colindex < 224)
			{
				if (colindex == 192 || colindex == 200)
					colindex += 8;
				else
					colindex += 16;
				if (p->color == colindex)
					p->die = -1;
			}
		    }
		    break;
		}
	}

#ifdef GLQUAKE
	glEnd_fp ();
	glDisable_fp (GL_BLEND);
	if (alphaTestEnabled)
		glEnable_fp(GL_ALPHA_TEST);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
#else
	D_EndParticles ();
#endif
}

