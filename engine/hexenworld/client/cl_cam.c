/*
 * cl_cam.c -- Player camera tracking in Spectator mode -- ZOID
 * This takes over player controls for spectator automatic camera.
 * Player moves as a spectator, but the camera tracks an enemy player
 * $Id: cl_cam.c,v 1.9 2007-05-09 18:10:16 sezero Exp $
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

#define	PM_SPECTATORMAXSPEED	500
#define	PM_STOPSPEED		100
#define	PM_MAXSPEED		320

#define BUTTON_JUMP	2
#define BUTTON_ATTACK	1
#define MAX_ANGLE_TURN	10

#define CAM_NONE	0
#define CAM_TRACK	1

static vec3_t desired_position; // where the camera wants to be
static qboolean locked = false;
static int oldbuttons;

// track high fragger
static cvar_t cl_hightrack = {"cl_hightrack", "0", CVAR_NONE};
//static cvar_t cl_camera_maxpitch = {"cl_camera_maxpitch", "10", CVAR_NONE};
//static cvar_t cl_camera_maxyaw = {"cl_camera_maxyaw", "30", CVAR_NONE};

static qboolean cam_forceview;
static double cam_lastviewtime;
#if 0
static vec3_t cam_viewangles;
#endif
static int spec_track = 0; // player# of who we are tracking
static int autocam = CAM_NONE;

static void vectoangles(vec3_t vec, vec3_t ang)
{
	float	forward;
	float	yaw, pitch;

	if (vec[1] == 0 && vec[0] == 0)
	{
		yaw = 0;
		if (vec[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec[1], vec[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);
		pitch = (int) (atan2(vec[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	ang[0] = pitch;
	ang[1] = yaw;
	ang[2] = 0;
}

static void Cam_Unlock(void)
{
	if (autocam)
	{
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "ptrack");
		autocam = CAM_NONE;
		locked = false;
		Sbar_Changed();
	}
}

static void Cam_Lock(int playernum)
{
	char st[40];

	sprintf(st, "ptrack %i", playernum);
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, st);
	spec_track = playernum;
	cam_forceview = true;
	locked = false;
	Sbar_Changed();
}

static pmtrace_t Cam_DoTrace(vec3_t vec1, vec3_t vec2)
{
#if 0
	memset(&pmove, 0, sizeof(pmove));

	pmove.numphysent = 1;
	VectorClear (pmove.physents[0].origin);
	pmove.physents[0].model = cl.worldmodel;
#endif

	VectorCopy (vec1, pmove.origin);
	return PM_PlayerMove(pmove.origin, vec2);
}

// Returns distance or 9999 if invalid for some reason
static float Cam_TryFlyby(player_state_t *self, player_state_t *player, vec3_t vec, qboolean checkvis)
{
	vec3_t	v;
	pmtrace_t	trace;
	float	len;

	vectoangles(vec, v);
//	v[0] = -v[0];
	VectorCopy (v, pmove.angles);
	VectorNormalize(vec);
	VectorMA(player->origin, 800, vec, v);
	// v is endpos
	// fake a player move
	trace = Cam_DoTrace(player->origin, v);
	if (/*trace.inopen ||*/ trace.inwater)
		return 9999;
	VectorCopy(trace.endpos, vec);
	VectorSubtract(trace.endpos, player->origin, v);
	len = VectorLength(v);
	if (len < 32 || len > 800)
		return 9999;
	if (checkvis)
	{
		VectorSubtract(trace.endpos, self->origin, v);
		len = VectorLength(v);

		trace = Cam_DoTrace(self->origin, vec);
		if (trace.fraction != 1 || trace.inwater)
			return 9999;
	}
	return len;
}

// Is player visible?
static qboolean Cam_IsVisible(player_state_t *player, vec3_t vec)
{
	pmtrace_t	trace;
	vec3_t	v;
	float	d;

	trace = Cam_DoTrace(player->origin, vec);
	if (trace.fraction != 1 || /*trace.inopen ||*/ trace.inwater)
		return false;
	// check distance, don't let the player get too far away or too close
	VectorSubtract(player->origin, vec, v);
	d = VectorLength(v);
	if (d < 16)
		return false;
	return true;
}

static qboolean InitFlyby(player_state_t *self, player_state_t *player, int checkvis)
{
	float	f, maxlen;
	vec3_t	vec, vec2;
	vec3_t	forward, right, up;

	VectorCopy(player->viewangles, vec);
	vec[0] = 0;
	AngleVectors (vec, forward, right, up);
//	for (i = 0; i < 3; i++)
//		forward[i] *= 3;

	maxlen = 1000;
	VectorAdd(forward, up, vec2);
	VectorAdd(vec2, right, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorAdd(forward, up, vec2);
	VectorSubtract(vec2, right, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorAdd(forward, right, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorSubtract(forward, right, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorAdd(forward, up, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorSubtract(forward, up, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorAdd(up, right, vec2);
	VectorSubtract(vec2, forward, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorSubtract(up, right, vec2);
	VectorSubtract(vec2, forward, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	// invert
	VectorNegate(forward, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorCopy(forward, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	// invert
	VectorNegate(right, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	VectorCopy(right, vec2);

	if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < maxlen)
	{
		maxlen = f;
		VectorCopy(vec2, vec);
	}

	// ack, can't find him
	if (maxlen >= 1000)
	{
	//	Cam_Unlock();
		return false;
	}

	locked = true;
	VectorCopy(vec, desired_position);
	return true;
}

static void Cam_CheckHighTarget(void)
{
	int	i, j, maxfrags;
	player_info_t	*s;

	j = -1;
	for (i = 0, maxfrags = -9999; i < MAX_CLIENTS; i++)
	{
		s = &cl.players[i];
		if (s->name[0] && !s->spectator && s->frags > maxfrags)
		{
			maxfrags = s->frags;
			j = i;
		}
	}

	if (j >= 0)
	{
		if (!locked || cl.players[j].frags > cl.players[spec_track].frags)
			Cam_Lock(j);
	}
	else
	{
		Cam_Unlock();
	}
}
	
// ZOID
//
// Take over the user controls and track a player.
// We find a nice position to watch the player and move there
void Cam_Track(usercmd_t *cmd)
{
	player_state_t	*player, *self;
	frame_t		*frame;
	vec3_t	vec;
	float	len;

	if (!cl.spectator)
		return;

	if (cl_hightrack.integer && !locked)
		Cam_CheckHighTarget();

	if (!autocam || cls.state != ca_active)
		return;

	if (locked && (!cl.players[spec_track].name[0] || cl.players[spec_track].spectator))
	{
		locked = false;
		if (cl_hightrack.integer)
			Cam_CheckHighTarget();
		else
			Cam_Unlock();
		return;
	}

	frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
	player = frame->playerstate + spec_track;
	self = frame->playerstate + cl.playernum;

	if (!locked || !Cam_IsVisible(player, desired_position))
	{
		if (!locked || realtime - cam_lastviewtime > 0.1)
		{
			if (!InitFlyby(self, player, true))
				InitFlyby(self, player, false);
			cam_lastviewtime = realtime;
		}
	}
	else
	{
		cam_lastviewtime = realtime;
	}

	// couldn't track for some reason
	if (!locked || !autocam)
		return;

	// Ok, move to our desired position and set our angles to view
	// the player
	VectorSubtract(desired_position, self->origin, vec);
	len = VectorLength(vec);
	cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;
	if (len > 16)
	{ // close enough?
		MSG_WriteByte (&cls.netchan.message, clc_tmove);
		MSG_WriteCoord (&cls.netchan.message, desired_position[0]);
		MSG_WriteCoord (&cls.netchan.message, desired_position[1]);
		MSG_WriteCoord (&cls.netchan.message, desired_position[2]);
	}

	// move there locally immediately
	VectorCopy(desired_position, self->origin);
	VectorSubtract(player->origin, desired_position, vec);
	vectoangles(vec, cl.viewangles);
	cl.viewangles[0] = -cl.viewangles[0];
}

#if 0
static float adjustang(float current, float ideal, float speed)
{
	float	move;

	current = anglemod(current);
	ideal = anglemod(ideal);

	if (current == ideal)
		return current;

	move = ideal - current;

	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

//	Con_Printf("c/i: %4.2f/%4.2f move: %4.2f\n", current, ideal, move);
	return anglemod (current + move);
}
#endif

#if 0
void Cam_SetView(void)
{
	player_state_t	*player, *self;
	frame_t		*frame;
	vec3_t	vec, vec2;

	if (cls.state != ca_active || !cl.spectator || !autocam || !locked)
		return;

	frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
	player = frame->playerstate + spec_track;
	self = frame->playerstate + cl.playernum;

	VectorSubtract(player->origin, cl.simorg, vec);
	if (cam_forceview)
	{
		cam_forceview = false;
		vectoangles(vec, cam_viewangles);
		cam_viewangles[0] = -cam_viewangles[0];
	}
	else
	{
		vectoangles(vec, vec2);
		vec2[PITCH] = -vec2[PITCH];

		cam_viewangles[PITCH] = adjustang(cam_viewangles[PITCH], vec2[PITCH], cl_camera_maxpitch.value);
		cam_viewangles[YAW] = adjustang(cam_viewangles[YAW], vec2[YAW], cl_camera_maxyaw.value);
	}
	VectorCopy(cam_viewangles, cl.viewangles);
	VectorCopy(cl.viewangles, cl.simangles);
}
#endif

void Cam_FinishMove(usercmd_t *cmd)
{
	int	i;
	player_info_t	*s;
	int	end;

	if (cls.state != ca_active || server_version < 1.57)
		return;

	if (!cl.spectator) // only in spectator mode
		return;

#if 0
	if (autocam && locked)
	{
		frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
		player = frame->playerstate + spec_track;
		self = frame->playerstate + cl.playernum;

		VectorSubtract(player->origin, self->origin, vec);
		if (cam_forceview)
		{
			cam_forceview = false;
			vectoangles(vec, cam_viewangles);
			cam_viewangles[0] = -cam_viewangles[0];
		}
		else
		{
			vectoangles(vec, vec2);
			vec2[PITCH] = -vec2[PITCH];

			cam_viewangles[PITCH] = adjustang(cam_viewangles[PITCH], vec2[PITCH], cl_camera_maxpitch.value);
			cam_viewangles[YAW] = adjustang(cam_viewangles[YAW], vec2[YAW], cl_camera_maxyaw.value);
		}
		VectorCopy(cam_viewangles, cl.viewangles);
	}
#endif

	if (cmd->buttons & BUTTON_ATTACK)
	{
		if (!(oldbuttons & BUTTON_ATTACK))
		{
			oldbuttons |= BUTTON_ATTACK;
			autocam++;

			if (autocam > CAM_TRACK)
			{
				Cam_Unlock();
				VectorCopy(cl.viewangles, cmd->angles);
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		oldbuttons &= ~BUTTON_ATTACK;
		if (!autocam)
			return;
	}

	if (autocam && cl_hightrack.integer)
	{
		Cam_CheckHighTarget();
		return;
	}

	if (locked)
	{
		if ((cmd->buttons & BUTTON_JUMP) && (oldbuttons & BUTTON_JUMP))
			return;		// don't pogo stick

		if (!(cmd->buttons & BUTTON_JUMP))
		{
			oldbuttons &= ~BUTTON_JUMP;
			return;
		}
		oldbuttons |= BUTTON_JUMP;	// don't jump again until released
	}

//	Con_Printf("Selecting track target...\n");

	if (locked && autocam)
		end = (spec_track + 1) % MAX_CLIENTS;
	else
		end = spec_track;

	i = end;

	do
	{
		s = &cl.players[i];
		if (s->name[0] && !s->spectator)
		{
			Cam_Lock(i);
			return;
		}
		i = (i + 1) % MAX_CLIENTS;
	} while (i != end);

	// stay on same guy?
	i = spec_track;
	s = &cl.players[i];
	if (s->name[0] && !s->spectator)
	{
		Cam_Lock(i);
		return;
	}
	Con_Printf("No target found ...\n");
	autocam = locked = false;
}

void Cam_Reset(void)
{
	autocam = CAM_NONE;
	spec_track = 0;
}

void CL_InitCam(void)
{
	Cvar_RegisterVariable (&cl_hightrack);
//	Cvar_RegisterVariable (&cl_camera_maxpitch);
//	Cvar_RegisterVariable (&cl_camera_maxyaw);
}

