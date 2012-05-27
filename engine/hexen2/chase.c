/*
	chase.c
	chase camera code

	$Id: chase.c,v 1.9 2007-04-03 06:19:19 sezero Exp $
*/

#include "quakedef.h"

static	cvar_t	chase_back = {"chase_back", "100", CVAR_NONE};
static	cvar_t	chase_up = {"chase_up", "16", CVAR_NONE};
static	cvar_t	chase_right = {"chase_right", "0", CVAR_NONE};
cvar_t		chase_active = {"chase_active", "0", CVAR_NONE};

static	vec3_t	chase_dest;


void Chase_Init (void)
{
	Cvar_RegisterVariable (&chase_back);
	Cvar_RegisterVariable (&chase_up);
	Cvar_RegisterVariable (&chase_right);
	Cvar_RegisterVariable (&chase_active);
}

void Chase_Reset (void)
{
	// for respawning and teleporting
//	start position 12 units behind head
}

static void TraceLine (vec3_t start, vec3_t end, vec3_t impact)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);
	VectorCopy (trace.endpos, impact);
}

void Chase_Update (void)
{
	float	dist;
	vec3_t	forward, up, right;
	vec3_t	dest, stop;

// if can't see player, reset
	AngleVectors (cl.viewangles, forward, right, up);

// calc exact destination
	chase_dest[0] = r_refdef.vieworg[0] -
			forward[0] * chase_back.value -
			right[0] * chase_right.value;
	chase_dest[1] = r_refdef.vieworg[1] -
			forward[1] * chase_back.value -
			right[1] * chase_right.value;
#if 0
	chase_dest[2] = r_refdef.vieworg[2] -
			forward[2] * chase_back.value -
			right[] * chase_right.value;
	chase_dest[2] += chase_up.value;
#endif
	chase_dest[2] = r_refdef.vieworg[2] + chase_up.value;

// find the spot the player is looking at
	VectorMA (r_refdef.vieworg, 4096, forward, dest);
	TraceLine (r_refdef.vieworg, dest, stop);

// calculate pitch to look at the same spot from camera
	VectorSubtract (stop, r_refdef.vieworg, stop);
	dist = DotProduct (stop, forward);
	if (dist < 1)
		dist = 1;
	r_refdef.viewangles[PITCH] = -atan(stop[2] / dist) / M_PI * 180;

// check for walls between player and camera (from quakeforge)
	TraceLine(r_refdef.vieworg, chase_dest, stop);
	if (VectorLength(stop) != 0)
	{
		chase_dest[0] = stop[0] + forward[0] * 8;
		chase_dest[1] = stop[1] + forward[1] * 8;
		chase_dest[2] = stop[2] + forward[2] * 8;
	}

// move towards destination
	VectorCopy (chase_dest, r_refdef.vieworg);
}

