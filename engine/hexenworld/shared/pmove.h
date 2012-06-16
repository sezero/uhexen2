/*
 * pmove.h -- player movement
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

#ifndef __PLAYERMOVE_H
#define __PLAYERMOVE_H

typedef struct
{
	vec3_t	normal;
	float	dist;
} pmplane_t;

typedef struct
{
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	qboolean	inopen, inwater;
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	pmplane_t		plane;	// surface normal at impact
	int			ent;	// entity the surface is on
} pmtrace_t;


#define	MAX_PHYSENTS	64
typedef struct
{
	vec3_t	origin;
	qmodel_t	*model;	// only for bsp models
	vec3_t	mins, maxs;	// only for non-bsp models
	vec3_t	angles;
	int		info;	// for client or server to identify
} physent_t;


typedef struct
{
	int		sequence;	// just for debugging prints

	// player state
	vec3_t	origin;
	vec3_t	angles;
	vec3_t	velocity;
	int		oldbuttons;
	float		waterjumptime;
	float		teleport_time;
	qboolean	dead;
	int		spectator;
	float	hasted;
	int		movetype;
	qboolean	crouched;

	// world state
	int		numphysent;
	physent_t	physents[MAX_PHYSENTS];	// 0 should be the world

	// input
	usercmd_t	cmd;

	// results
	int		numtouch;
	int		touchindex[MAX_PHYSENTS];
} playermove_t;

typedef struct {
	float	gravity;
	float	stopspeed;
	float	maxspeed;
	float	spectatormaxspeed;
	float	accelerate;
	float	airaccelerate;
	float	wateraccelerate;
	float	friction;
	float	waterfriction;
	float	entgravity;
} movevars_t;


extern	movevars_t	movevars;
extern	playermove_t	pmove;
extern	int		onground;
extern	int		waterlevel;
extern	int		watertype;

void PlayerMove (void);
void Pmove_Init (void);

int PM_PointContents (vec3_t point);
int PM_HullPointContents (hull_t *hull, int num, vec3_t p);
qboolean PM_TestPlayerPosition (vec3_t point);
pmtrace_t PM_PlayerMove (vec3_t start, vec3_t stop);

#endif	/* __PLAYERMOVE_H */

