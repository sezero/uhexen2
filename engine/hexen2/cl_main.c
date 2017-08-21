/*
 * cl_main.c -- hexen2 client main loop
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
#include "bgmusic.h"
#include "cdaudio.h"

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t	cl_name = {"_cl_name", "player", CVAR_ARCHIVE};
cvar_t	cl_color = {"_cl_color", "0", CVAR_ARCHIVE};
cvar_t	cl_playerclass = {"_cl_playerclass", "1", CVAR_ARCHIVE};

cvar_t	cl_shownet = {"cl_shownet", "0", CVAR_NONE};	// can be 0, 1, or 2
cvar_t	cl_nolerp = {"cl_nolerp", "0", CVAR_NONE};

cvar_t	cfg_unbindall = {"cfg_unbindall", "1", CVAR_ARCHIVE};

cvar_t	lookspring = {"lookspring", "0", CVAR_ARCHIVE};
cvar_t	lookstrafe = {"lookstrafe", "0", CVAR_ARCHIVE};
cvar_t	sensitivity = {"sensitivity", "3", CVAR_ARCHIVE};
cvar_t	mwheelthreshold = {"mwheelthreshold", "120", CVAR_ARCHIVE};

cvar_t	m_pitch = {"m_pitch", "0.022", CVAR_ARCHIVE};
cvar_t	m_yaw = {"m_yaw", "0.022", CVAR_ARCHIVE};
cvar_t	m_forward = {"m_forward", "1", CVAR_ARCHIVE};
cvar_t	m_side = {"m_side", "0.8", CVAR_ARCHIVE};


client_static_t	cls;
client_state_t	cl;
// FIXME: put these on hunk?
efrag_t		cl_efrags[MAX_EFRAGS];
entity_t	cl_entities[MAX_EDICTS];
entity_t	cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t	cl_dlights[MAX_DLIGHTS];

int		cl_numvisedicts;
entity_t	*cl_visedicts[MAX_VISEDICTS];


/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	int	i;

	if (!sv.active)
		Host_ClearMemory ();

// wipe the entire cl structure
	memset (&cl, 0, sizeof(cl));

	SZ_Clear (&cls.message);

// clear other arrays
	memset (cl_efrags, 0, sizeof(cl_efrags));
	memset (cl_entities, 0, sizeof(cl_entities));
	memset (cl_dlights, 0, sizeof(cl_dlights));
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	CL_ClearTEnts();
	CL_ClearEffects();

// allocate the efrags and chain together into a free list
	cl.free_efrags = cl_efrags;
	for (i = 0; i < MAX_EFRAGS-1; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;

	cl.current_frame = cl.current_sequence = 99;
	cl.reference_frame = cl.last_frame = cl.last_sequence = 199;
	cl.need_build = 2;

	SCR_SetPlaqueMessage("");

	SB_InvReset();
}

/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
// don't get stuck in chat mode
	if (Key_GetDest() == key_message)
		Key_EndChat ();

// reset any active palette shift
	memset (cl.cshifts, 0, sizeof(cl.cshifts));

//jfm: need to clear parts because some now check world
	R_ClearParticles ();

// stop sounds (especially looping!)
	S_StopAllSounds (true);
	BGM_Stop();
	CDAudio_Stop();
	loading_stage = 0;

// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
	}
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);

		Host_RemoveGIPFiles(NULL);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
	cl.intermission = 0;
}

void CL_Disconnect_f (void)
{
	CL_Disconnect ();
	if (sv.active)
		Host_ShutdownServer (false);
}

/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (const char *host)
{
	if (cls.state == ca_dedicated)
		return;

	if (cls.demoplayback)
		return;

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("%s: connect failed", __thisfunc__);
	Con_DPrintf ("%s: connected to %s\n", __thisfunc__, host);

	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing

	MSG_WriteByte (&cls.message, clc_nop);	// fixes connection getting stuck after displaying
								// the "Connection accepted" msg..
}

/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char	str[8192];

	Con_DPrintf ("%s: %i\n", __thisfunc__, cls.signon);

	switch (cls.signon)
	{
	case 1:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn");
		break;

	case 2:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_name.string));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("playerclass %i\n", cl_playerclass.integer));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", cl_color.integer >> 4, cl_color.integer & 15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		q_snprintf (str, sizeof(str), "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);
		break;

	case 3:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
		break;

	case 4:
		SCR_EndLoadingPlaque ();	// allow normal screen updates
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
void CL_NextDemo (void)
{
	char	str[1024];

	if (cls.demonum == -1)
		return;		// don't play demos

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			CL_Disconnect();
			return;
		}
	}

	SCR_BeginLoadingPlaque ();

	q_snprintf (str, sizeof(str),"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
static void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int	i;

	if (cls.state != ca_connected)
		return;

	for (i = 0, ent = cl_entities; i < cl.num_entities; i++, ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n",
				ent->model->name, ent->frame,
				ent->origin[0], ent->origin[1],
				ent->origin[2], ent->angles[0],
				ent->angles[1], ent->angles[2]);
	}
}

/*
=============
CL_Viewpos_f -- display client's position and angles
		from FitzQuake
=============
*/
void CL_Viewpos_f (void)
{
	if (cls.state != ca_connected)
		return;
#if 0
	//camera position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)r_refdef.vieworg[0],
		(int)r_refdef.vieworg[1],
		(int)r_refdef.vieworg[2],
		(int)r_refdef.viewangles[PITCH],
		(int)r_refdef.viewangles[YAW],
		(int)r_refdef.viewangles[ROLL]);
#else
	//player position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)cl_entities[cl.viewentity].origin[0],
		(int)cl_entities[cl.viewentity].origin[1],
		(int)cl_entities[cl.viewentity].origin[2],
		(int)cl.viewangles[PITCH],
		(int)cl.viewangles[YAW],
		(int)cl.viewangles[ROLL]);
#endif
}


/*
===============
SetPal

Debugging tool, just flashes the screen
===============
*/
static void SetPal (int i)
{
#if 0
	static int old;
	byte	pal[768];
	int		c;

	if (i == old)
		return;
	old = i;

	if (i == 0)
		VID_SetPalette (host_basepal);
	else if (i == 1)
	{
		for (c = 0; c < 768; c += 3)
		{
			pal[c] = 0;
			pal[c+1] = 255;
			pal[c+2] = 0;
		}
		VID_SetPalette (pal);
	}
	else
	{
		for (c = 0; c < 768; c += 3)
		{
			pal[c] = 0;
			pal[c+1] = 0;
			pal[c+2] = 255;
		}
		VID_SetPalette (pal);
	}
#endif
}

/*
===============
CL_AllocDlight

===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int		i;
	dlight_t	*dl;

// first look for an exact key match
	dl = cl_dlights;
	if (key)
	{
		for (i = 0; i < MAX_DLIGHTS; i++, dl++)
		{
			if (dl->key == key)
				goto done;
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < cl.time)
			goto done;
	}

	dl = &cl_dlights[0];
done:
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	dl->color[0] = dl->color[1] = dl->color[2] = dl->color[3] = 1.0;
	return dl;
}


/*
===============
CL_DecayLights

===============
*/
void CL_DecayLights (void)
{
	dlight_t	*dl;
	float	time;
	int	i;

	time = cl.time - cl.oldtime;

	dl = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;

		if (dl->radius > 0)
		{
			dl->radius -= time*dl->decay;
			if (dl->radius < 0)
				dl->radius = 0;
		}
		else
		{
			dl->radius += time*dl->decay;
			if (dl->radius > 0)
				dl->radius = 0;
		}
	}
}


/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
static float CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];

	if (!f || cl_nolerp.integer || cls.timedemo || sv.active)
	{
		cl.time = cl.mtime[0];
		return 1;
	}

	if (f > 0.1)
	{	// dropped packet, or start of demo
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = 0.1;
	}
	frac = (cl.time - cl.mtime[1]) / f;
	//Con_Printf ("frac: %f\n",frac);
	if (frac < 0)
	{
		if (frac < -0.01)
		{
			SetPal(1);
			cl.time = cl.mtime[1];
//			Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
		{
			SetPal(2);
			cl.time = cl.mtime[0];
//			Con_Printf ("high frac\n");
		}
		frac = 1;
	}
	else
		SetPal(0);

	return frac;
}


/*
===============
CL_RelinkEntities
===============
*/
static void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int		i, j;
	float		frac, f, d;
	vec3_t		delta;
	vec3_t		oldorg;
	dlight_t	*dl;

// determine partial update time
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;

// interpolate player info
	for (i = 0; i < 3; i++)
		cl.velocity[i] = cl.mvelocity[1][i] + frac * (cl.mvelocity[0][i] - cl.mvelocity[1][i]);

	if (cls.demoplayback && !intro_playing)
	{
	// interpolate the angles
		for (j = 0; j < 3; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}

// start on the entity after the world
	for (i = 1, ent = cl_entities+1; i < cl.num_entities; i++, ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfrags (ent);	// just became empty
			continue;
		}

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0] && !(ent->baseline.flags & BE_ON))
		{
			ent->model = NULL;
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (ent->forcelink || ent->msgtime != cl.mtime[0])
		{	// the entity was not updated in the last message
			// so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j = 0; j < 3; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];

				if (delta[j] > 100 || delta[j] < -100)
					f = 1;		// assume a teleportation, not a motion
			}

		// interpolate the origin and angles
			for (j = 0; j < 3; j++)
			{
				ent->origin[j] = ent->msg_origins[1][j] + f*delta[j];

				d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[j] = ent->msg_angles[1][j] + f*d;
			}

		}

	//	if (ent->effects & EF_BRIGHTFIELD);
	//		R_EntityParticles (ent);

		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticles (ent);

		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, rv, uv);
			VectorMA (dl->origin, 18, fv, dl->origin);
			dl->radius = 200 + (rand() & 31);
			dl->minlight = 32;
			dl->die = cl.time + 0.1;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{	// Make the dynamic light yellow
				dl->color[0] = 1.0;
				dl->color[1] = 1.0;
				dl->color[2] = 0.5;
				dl->color[3] = 0.7;
			}
#		endif
		}
		if (ent->effects & EF_BRIGHTLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			dl->radius = 400 + (rand() & 31);
			dl->die = cl.time + 0.001;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{
				dl->color[0] = 0.8;
				dl->color[1] = 0.8;
				dl->color[2] = 1.0;
				dl->color[3] = 0.7;
			}
#		endif
		}
		if (ent->effects & EF_DIMLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200 + (rand() & 31);
			dl->die = cl.time + 0.001;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{
				dl->color[0] = 0.8;
				dl->color[1] = 0.6;
				dl->color[2] = 0.2;
				dl->color[3] = 0.7;
			}
#		endif
		}

		if (ent->effects & EF_DARKLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200.0 + (rand() & 31);
			dl->die = cl.time + 0.001;
			dl->dark = true;
		}
		if (ent->effects & EF_LIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200;
			dl->die = cl.time + 0.001;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{
				dl->color[0] = 0.8;
				dl->color[1] = 0.4;
				dl->color[2] = 0.2;
				dl->color[3] = 0.7;
			}
#		endif
		}

		if (ent->model->flags & EF_GIB)
			R_RocketTrail (oldorg, ent->origin, 2);
		else if (ent->model->flags & EF_ZOMGIB)
			R_RocketTrail (oldorg, ent->origin, 4);
		else if (ent->model->flags & EF_BLOODSHOT)
			R_RocketTrail (oldorg, ent->origin, 17);
		else if (ent->model->flags & EF_TRACER)
			R_RocketTrail (oldorg, ent->origin, 3);
		else if (ent->model->flags & EF_TRACER2)
			R_RocketTrail (oldorg, ent->origin, 5);
		else if (ent->model->flags & EF_ROCKET)
		{
			R_RocketTrail (oldorg, ent->origin, 0);
			/*
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = cl.time + 0.01;
			*/
		}
		else if (ent->model->flags & EF_FIREBALL)
		{
			R_RocketTrail (oldorg, ent->origin, rt_fireball);

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 120 - (rand() % 20);
			dl->die = cl.time + 0.01;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{
				dl->color[0] = 0.8;
				dl->color[1] = 0.2;
				dl->color[2] = 0.2;
				dl->color[3] = 0.7;
			}
#		endif
		}
		else if (ent->model->flags & EF_ACIDBALL)
		{
			R_RocketTrail (oldorg, ent->origin, rt_acidball);

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 120 - (rand() % 20);
			dl->die = cl.time + 0.01;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{	// Make the dynamic light green
				dl->color[0] = 0.2;
				dl->color[1] = 0.8;
				dl->color[2] = 0.2;
				dl->color[3] = 0.7;
			}
#		endif
		}
		else if (ent->model->flags & EF_ICE)
		{
			R_RocketTrail (oldorg, ent->origin, rt_ice);
		}
		else if (ent->model->flags & EF_SPIT)
		{
			R_RocketTrail (oldorg, ent->origin, rt_spit);

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = -120 - (rand() % 20);
			dl->die = cl.time + 0.05;
#		ifdef GLQUAKE
			if (gl_colored_dynamic_lights.integer)
			{	// Make the dynamic light green
				dl->color[0] = 0.2;
				dl->color[1] = 0.6;
				dl->color[2] = 0.2;
				dl->color[3] = 0.7;
			}
#		endif
		}
		else if (ent->model->flags & EF_SPELL)
		{
			R_RocketTrail (oldorg, ent->origin, rt_spell);
		}
		else if (ent->model->flags & EF_GRENADE)
			R_RocketTrail (oldorg, ent->origin, 1);
		else if (ent->model->flags & EF_TRACER3)
			R_RocketTrail (oldorg, ent->origin, 6);
		else if (ent->model->flags & EF_VORP_MISSILE)
		{
			R_RocketTrail (oldorg, ent->origin, rt_vorpal);

#		ifdef GLQUAKE
			// extra dynamic lights
			if (gl_extra_dynamic_lights.integer)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 240 - (rand() % 20);
				dl->die = cl.time + 0.01;
				if (gl_colored_dynamic_lights.integer)
				{	// Make the dynamic light blue
					dl->color[0] = 0.3;
					dl->color[1] = 0.3;
					dl->color[2] = 0.8;
					dl->color[3] = 0.7;
				}
			}
#		endif
		}
		else if (ent->model->flags & EF_SET_STAFF)
		{
			R_RocketTrail (oldorg, ent->origin,rt_setstaff);
		}
		else if (ent->model->flags & EF_MAGICMISSILE)
		{
			if ((rand() & 3) < 1)
				R_RocketTrail (oldorg, ent->origin, rt_magicmissile);
#		ifdef GLQUAKE
			// extra dynamic lights
			if (gl_extra_dynamic_lights.integer)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 240 - (rand() % 20);
				dl->die = cl.time + 0.01;
				if (gl_colored_dynamic_lights.integer)
				{	// Make the dynamic light blue
					dl->color[0] = 0.1;
					dl->color[1] = 0.1;
					dl->color[2] = 0.8;
					dl->color[3] = 0.7;
				}
			}
#		endif
		}
		else if (ent->model->flags & EF_BONESHARD)
		{
			R_RocketTrail (oldorg, ent->origin, rt_boneshard);
		}
		else if (ent->model->flags & EF_SCARAB)
		{
			R_RocketTrail (oldorg, ent->origin, rt_scarab);
#		ifdef GLQUAKE
			// extra dynamic lights
			if (gl_extra_dynamic_lights.integer)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 240 - (rand() % 20);
				dl->die = cl.time + 0.01;
				if (gl_colored_dynamic_lights.integer)
				{	// Make the dynamic light orange
					dl->color[0] = 0.9;
					dl->color[1] = 0.6;
					dl->color[2] = 0.1;
					dl->color[3] = 0.7;
				}
			}
#		endif
		}

		ent->forcelink = false;

		if (i == cl.viewentity && !chase_active.integer)
			continue;

		if (ent->effects & EF_NODRAW)
			continue;

		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}


/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int	ret;

	cl.oldtime = cl.time;
	cl.time += host_frametime;

	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("%s: lost server connection", __thisfunc__);
		if (!ret)
			break;

		cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && cls.state == ca_connected);

	if (cl_shownet.integer)
		Con_Printf ("\n");

	CL_RelinkEntities ();
	CL_UpdateTEnts ();

// bring the links up to date
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t	cmd;

	if (cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMove (&cmd);

	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);

	// send the unreliable message
		CL_SendMove (&cmd);
	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage (cls.netcon))
	{
		Con_DPrintf ("%s: can't send\n", __thisfunc__);
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("%s: lost server connection", __thisfunc__);

	SZ_Clear (&cls.message);
}

static void CL_Sensitivity_save_f (void)
{
	static float save_sensitivity = 3;
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("sensitivity_save <save/restore>\n");
	}
	else if (q_strcasecmp(Cmd_Argv(1),"save") == 0)
	{
		save_sensitivity = sensitivity.value;
	}
	else if (q_strcasecmp(Cmd_Argv(1),"restore") == 0)
	{
		Cvar_SetValueQuick (&sensitivity, save_sensitivity);
	}
}

/*
=================
CL_Init
=================
*/
void CL_Init (void)
{
	SZ_Init (&cls.message, NULL, 1024);

	CL_InitInput ();
	CL_InitTEnts ();
	CL_InitEffects();

//
// register our commands
//
	Cvar_RegisterVariable (&cl_name);
	Cvar_RegisterVariable (&cl_color);
	Cvar_RegisterVariable (&cl_playerclass);
	Cvar_RegisterVariable (&cl_upspeed);
	Cvar_RegisterVariable (&cl_forwardspeed);
	Cvar_RegisterVariable (&cl_backspeed);
	Cvar_RegisterVariable (&cl_sidespeed);
	Cvar_RegisterVariable (&cl_movespeedkey);
	Cvar_RegisterVariable (&cl_yawspeed);
	Cvar_RegisterVariable (&cl_pitchspeed);
	Cvar_RegisterVariable (&cl_anglespeedkey);
	Cvar_RegisterVariable (&cl_shownet);
	Cvar_RegisterVariable (&cl_nolerp);
	Cvar_RegisterVariable (&lookspring);
	Cvar_RegisterVariable (&lookstrafe);
	Cvar_RegisterVariable (&sensitivity);
	Cvar_RegisterVariable (&mwheelthreshold);

	Cvar_RegisterVariable (&m_pitch);
	Cvar_RegisterVariable (&m_yaw);
	Cvar_RegisterVariable (&m_forward);
	Cvar_RegisterVariable (&m_side);

	Cvar_RegisterVariable (&cfg_unbindall);

	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("sensitivity_save", CL_Sensitivity_save_f);

	Cmd_AddCommand ("viewpos", CL_Viewpos_f);
}

