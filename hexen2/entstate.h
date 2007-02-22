/*
	entstate.h
	structures for entity states and client frames

	$Id: entstate.h,v 1.2 2007-02-22 19:26:51 sezero Exp $
*/

#ifndef __ENTSTATE_H
#define __ENTSTATE_H

typedef struct
{
	vec3_t	origin;
	vec3_t	angles;
	short	modelindex;
	byte	frame;
	byte	colormap;
	byte	skin;
	byte	effects;
	byte	scale;
	byte	drawflags;
	byte	abslight;
	byte	ClearCount[32];
} entity_state_t;

typedef struct
{
	byte	flags;
	short	index;

	vec3_t	origin;
	vec3_t	angles;
	short	modelindex;
	byte	frame;
	byte	colormap;
	byte	skin;
	byte	effects;
	byte	scale;
	byte	drawflags;
	byte	abslight;
} entity_state2_t;

typedef struct
{
	byte	flags;

	vec3_t	origin;
	vec3_t	angles;
	short	modelindex;
	byte	frame;
	byte	colormap;
	byte	skin;
	byte	effects;
	byte	scale;
	byte	drawflags;
	byte	abslight;
} entity_state3_t;

#define MAX_CLIENT_STATES	150
#define MAX_FRAMES		5
#define CLEAR_LIMIT		2

#define ENT_STATE_ON		1
#define ENT_CLEARED		2

typedef struct
{
	entity_state2_t	states[MAX_CLIENT_STATES];
//	unsigned long	frame;
//	unsigned long	flags;
	int		count;
} client_frames_t;

typedef struct
{
	entity_state2_t	states[MAX_CLIENT_STATES*2];
	int		count;
} client_frames2_t;

typedef struct
{
	client_frames_t	frames[MAX_FRAMES+2]; // 0 = base, 1-max = proposed, max+1 = too late
} client_state2_t;

#endif	/* __ENTSTATE_H */

