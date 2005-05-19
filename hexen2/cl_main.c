/*
	cl_main.c
	client main loop

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/cl_main.c,v 1.11 2005-05-19 16:41:50 sezero Exp $
*/

#include "quakedef.h"

#ifndef PLATFORM_UNIX
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <fnmatch.h>
#include <unistd.h>
#endif

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t	cl_name = {"_cl_name", "player", true};
cvar_t	cl_color = {"_cl_color", "0", true};
cvar_t	cl_playerclass = {"_cl_playerclass", "5", true};

cvar_t	cl_shownet = {"cl_shownet","0"};	// can be 0, 1, or 2
cvar_t	cl_nolerp = {"cl_nolerp","0"};

cvar_t	lookspring = {"lookspring","0", true};
cvar_t	lookstrafe = {"lookstrafe","0", true};
cvar_t	sensitivity = {"sensitivity","3", true};
static float save_sensitivity;

cvar_t	m_pitch = {"m_pitch","0.022", true};
cvar_t	m_yaw = {"m_yaw","0.022", true};
cvar_t	m_forward = {"m_forward","1", true};
cvar_t	m_side = {"m_side","0.8", true};


client_static_t	cls;
client_state_t	cl;
// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_entities[MAX_EDICTS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];
/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	int			i;

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

//
// allocate the efrags and chain together into a free list
//
	cl.free_efrags = cl_efrags;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;

	cl.current_frame = cl.current_sequence = 99;
	cl.reference_frame = cl.last_frame = cl.last_sequence = 199;
	cl.need_build = 2;

	plaquemessage = "";

	SB_InvReset();
}

void CL_RemoveGIPFiles (char *path)
{
	char	name[MAX_OSPATH],tempdir[MAX_OSPATH];
#ifndef PLATFORM_UNIX
	int i;
	HANDLE handle;
	WIN32_FIND_DATA filedata;
	BOOL retval;

	if (path)
	{
		sprintf(tempdir,"%s\\",path);
	}
	else
	{
		i = GetTempPath(sizeof(tempdir),tempdir);
		if (!i) 
		{
			sprintf(tempdir,"%s\\",com_gamedir);
		}
	}

	sprintf (name, "%s*.gip", tempdir);

	handle = FindFirstFile(name,&filedata);
	retval = TRUE;

	while (handle != INVALID_HANDLE_VALUE && retval)
	{
		sprintf(name,"%s%s", tempdir,filedata.cFileName);
		DeleteFile(name);

		retval = FindNextFile(handle,&filedata);
	}

	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);
#else
	DIR	*dir;
	struct dirent	*dent;

	if (path)
	{
		snprintf(tempdir, MAX_OSPATH, "%s/",path);
	
	}
	else
	{
		snprintf(tempdir, MAX_OSPATH, "%s/",com_userdir);

	}

	dir = opendir (tempdir);
	if (dir == NULL) {
		return;
	}
	do {
		dent = readdir(dir);
		if (dent != NULL) {
			if (!fnmatch ("*.gip", dent->d_name,FNM_PATHNAME)) {
				snprintf (name, MAX_OSPATH, "%s%s", tempdir,dent->d_name);
			//	printf("unlinking %s\n",name);
				unlink (name);
			}
		}
	} while (dent != NULL);	
	closedir (dir);
#endif
}

qboolean CL_CopyFiles(char *source, char *pat, char *dest)
{
#ifndef PLATFORM_UNIX
	char	name[MAX_OSPATH],tempdir[MAX_OSPATH];
	HANDLE handle;
	WIN32_FIND_DATA filedata;
	BOOL retval,error;

	handle = FindFirstFile(pat,&filedata);
	retval = TRUE;
	error = false;

	while (handle != INVALID_HANDLE_VALUE && retval)
	{
		sprintf(name,"%s%s", source, filedata.cFileName);
		sprintf(tempdir,"%s%s", dest, filedata.cFileName);
		if (!CopyFile(name,tempdir,FALSE))
			error = true;

		retval = FindNextFile(handle,&filedata);
	}

	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);

	return error;
#else
	char	name[MAX_OSPATH],tempdir[MAX_OSPATH], *fpat;
	DIR	*dir;
	struct dirent	*dent;

/*
pat is assumed to be <source> / *.gip
as this function is currently only used for saving/loading games.
plus, COM_CopyFile doesn't return anything, so there is no way to tell if it
failed.
*/
	dir = opendir (source);
	if (dir == NULL) {
		return true;
	}
	
	fpat = strrchr(pat, '/');
	if (fpat == NULL)
		fpat = pat;
	else
		fpat++; /* skip the '/' */
			
	do {
		dent = readdir(dir);
		if (dent != NULL) {
			if (!fnmatch (fpat, dent->d_name,FNM_PATHNAME)) {
				sprintf(name,"%s%s", source, dent->d_name);
				sprintf(tempdir,"%s%s", dest, dent->d_name);
				COM_CopyFile(name,tempdir);
			}
		}
	} while (dent != NULL);	
	closedir (dir);
	
	return false;	// assuming no error
#endif
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
	R_ClearParticles ();	//jfm: need to clear parts because some now check world
	S_StopAllSounds (true);// stop sounds (especially looping!)
	loading_stage = 0;

// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
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

		CL_RemoveGIPFiles(NULL);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
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
void CL_EstablishConnection (char *host)
{
	if (cls.state == ca_dedicated)
		return;

	if (cls.demoplayback)
		return;

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("CL_Connect: connect failed\n");
	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);
	
	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing
}

/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char 	str[8192];

Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

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
		MSG_WriteString (&cls.message, va("playerclass %i\n", (int)cl_playerclass.value));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", ((int)cl_color.value)>>4, ((int)cl_color.value)&15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		sprintf (str, "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);
		break;
		
	case 3:	
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
		break;
		
	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
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

	sprintf (str,"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int			i;
	
	for (i=0,ent=cl_entities ; i<cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}


/*
===============
SetPal

Debugging tool, just flashes the screen
===============
*/
void SetPal (int i)
{
#if 0
	static int old;
	byte	pal[768];
	int		c;
	
	if (i == old)
		return;
	old = i;

	if (i==0)
		VID_SetPalette (host_basepal);
	else if (i==1)
	{
		for (c=0 ; c<768 ; c+=3)
		{
			pal[c] = 0;
			pal[c+1] = 255;
			pal[c+2] = 0;
		}
		VID_SetPalette (pal);
	}
	else
	{
		for (c=0 ; c<768 ; c+=3)
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
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}


/*
===============
CL_DecayLights

===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	float		time;
	
	time = cl.time - cl.oldtime;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
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
float	CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];
	
	if (!f || cl_nolerp.value || cls.timedemo || sv.active)
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
//				Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
		{
SetPal(2);
			cl.time = cl.mtime[0];
//				Con_Printf ("high frac\n");
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
void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, f, d;
	vec3_t		delta;
//	float		bobjrotate;
	vec3_t		oldorg;
	dlight_t	*dl;
	int c;
//	static int lastc = 0;

	c = 0;
// determine partial update time	
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;

//
// interpolate player info
//
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity[1][i] + 
			frac * (cl.mvelocity[0][i] - cl.mvelocity[1][i]);

	if (cls.demoplayback && !intro_playing)
	{
	// interpolate the angles	
		for (j=0 ; j<3 ; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}
	
	//bobjrotate = anglemod(100*(cl.time+ent->origin[0]+ent->origin[1]));
	
// start on the entity after the world
	for (i=1,ent=cl_entities+1 ; i<cl.num_entities ; i++,ent++)
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
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];

				if (delta[j] > 100 || delta[j] < -100)
					f = 1;		// assume a teleportation, not a motion
			}

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
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

		c++;

		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticles (ent);

		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->origin[2] += 16;
				AngleVectors (ent->angles, fv, rv, uv);
				 
				VectorMA (dl->origin, 18, fv, dl->origin);
				dl->radius = 200 + (rand()&31);
				dl->minlight = 32;
				dl->die = cl.time + 0.1;
			}
		}
		if (ent->effects & EF_BRIGHTLIGHT)
		{			
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->origin[2] += 16;
				dl->radius = 400 + (rand()&31);
				dl->die = cl.time + 0.001;
			}
		}
		if (ent->effects & EF_DIMLIGHT)
		{			
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->radius = 200 + (rand()&31);
				dl->die = cl.time + 0.001;
			}
		}

		if (ent->effects & EF_DARKLIGHT)
		{			
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->radius = 200.0 + (rand()&31);
				dl->die = cl.time + 0.001;
				dl->dark = true;
			}
		}
		if (ent->effects & EF_LIGHT)
		{			
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->radius = 200;
				dl->die = cl.time + 0.001;
			}
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
/*			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = cl.time + 0.01;*/
		}
		else if (ent->model->flags & EF_FIREBALL)
		{
			R_RocketTrail (oldorg, ent->origin, rt_fireball);
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 120 - (rand() % 20);
				dl->die = cl.time + 0.01;
			}
		}
		else if (ent->model->flags & EF_ACIDBALL)
		{
			R_RocketTrail (oldorg, ent->origin, rt_acidball);
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 120 - (rand() % 20);
				dl->die = cl.time + 0.01;
			}
		}
		else if (ent->model->flags & EF_ICE)
		{
			R_RocketTrail (oldorg, ent->origin, rt_ice);
		}
		else if (ent->model->flags & EF_SPIT)
		{
			R_RocketTrail (oldorg, ent->origin, rt_spit);
			if (cl_prettylights.value)
			{
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = -120 - (rand() % 20);
				dl->die = cl.time + 0.05;
			}
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
		}
		else if (ent->model->flags & EF_SET_STAFF)
		{
			R_RocketTrail (oldorg, ent->origin,rt_setstaff);
		}
		else if (ent->model->flags & EF_MAGICMISSILE)
		{
			if ((rand() & 3) < 1)
				R_RocketTrail (oldorg, ent->origin, rt_magicmissile);
		}
		else if (ent->model->flags & EF_BONESHARD)
			R_RocketTrail (oldorg, ent->origin, rt_boneshard);
		else if (ent->model->flags & EF_SCARAB)
			R_RocketTrail (oldorg, ent->origin, rt_scarab);

		ent->forcelink = false;

		if (i == cl.viewentity && !chase_active.value)
			continue;

		if ( ent->effects & EF_NODRAW )
			continue;

		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}

/*	if (c != lastc)
	{
		Con_Printf("Number of entities: %d\n",c);
		lastc = c;
	}*/
}


/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int		ret;

	cl.oldtime = cl.time;
	cl.time += host_frametime;
	
	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;
		
		cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && cls.state == ca_connected);
	
	if (cl_shownet.value)
		Con_Printf ("\n");

	CL_RelinkEntities ();
	CL_UpdateTEnts ();

//
// bring the links up to date
//
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t		cmd;

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
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	SZ_Clear (&cls.message);
}

void CL_Sensitivity_save_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("sensitivity_save <save/restore>\n");
		return;
	}

	if (Q_strcasecmp(Cmd_Argv(1),"save") == 0)
		save_sensitivity = sensitivity.value;
	else if (Q_strcasecmp(Cmd_Argv(1),"restore") == 0)
		Cvar_SetValue ("sensitivity", save_sensitivity);
}
/*
=================
CL_Init
=================
*/
void CL_Init (void)
{	
	SZ_Alloc (&cls.message, 1024);

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

	Cvar_RegisterVariable (&m_pitch);
	Cvar_RegisterVariable (&m_yaw);
	Cvar_RegisterVariable (&m_forward);
	Cvar_RegisterVariable (&m_side);
	Cvar_RegisterVariable (&cl_prettylights);

//	Cvar_RegisterVariable (&cl_autofire);
	
	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("sensitivity_save", CL_Sensitivity_save_f);
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2005/05/17 22:56:19  sezero
 * cleanup the "stricmp, strcmpi, strnicmp, Q_strcasecmp, Q_strncasecmp" mess:
 * Q_strXcasecmp will now be used throughout the code which are implementation
 * dependant defines for __GNUC__ (strXcasecmp) and _WIN32 (strXicmp)
 *
 * Revision 1.9  2005/04/30 08:07:21  sezero
 * CL_NextDemo calls SCR_BeginLoadingPlaque (which triggers scr_drawloading = 1)
 * before checking if an actual nextdemo exists, so scr_drawloading still remained
 * as true.
 *
 * Revision 1.8  2005/04/15 20:21:49  sezero
 * Kill warning: cl_main.c:207: warning: `return' with no value, in function returning non-void
 *
 * Revision 1.7  2004/12/18 14:15:34  sezero
 * Clean-up and kill warnings 10:
 * Remove some already commented-out functions and code fragments.
 * They seem to be of no-future use. Also remove some unused functions.
 *
 * Revision 1.6  2004/12/18 14:08:07  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.5  2004/12/18 13:54:43  sezero
 * Clean-up and kill warnings 6:
 * Include proper stock headers to avoid some "implicity declared" warnings
 *
 * Revision 1.4  2004/12/18 13:52:54  sezero
 * Clean-up and kill warnings 5:
 * Misc irritants..
 *
 * Revision 1.3  2004/12/14 21:49:37  sezero
 * don't spam the console while exiting the game
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:01:04  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.3  2001/12/13 22:18:25  phneutre
 * changed game_dir by user_dir in CL_RemoveGIPFiles()
 *
 * Revision 1.2  2001/12/08 06:35:35  relnev
 * Implemented CL_CopyFiles(), for savegames.
 *
 * 12    3/16/98 5:32p Jweier
 * 
 * 11    3/16/98 12:02a Jweier
 * 
 * 10    3/13/98 7:30p Jweier
 * 
 * 9     3/12/98 6:30p Mgummelt
 * 
 * 8     3/09/98 12:30p Mgummelt
 * 
 * 7     3/06/98 4:55p Mgummelt
 * 
 * 6     3/02/98 11:04p Jmonroe
 * changed start sound back to byte, added stopsound, put in a hack fix
 * for touchtriggers area getting removed
 * 
 * 5     3/02/98 3:52p Jweier
 * 
 * 34    10/03/97 4:07p Rjohnson
 * Copy file updates
 * 
 * 33    9/25/97 11:56p Rjohnson
 * CL_CopyFiles() not returns error if wasn't successful
 * 
 * 32    9/19/97 8:47a Rlove
 * 
 * 31    9/18/97 2:34p Rlove
 * 
 * 30    9/17/97 1:27p Rlove
 * 
 * 29    9/17/97 11:11a Rlove
 * 
 * 28    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 27    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 26    9/01/97 4:29a Rjohnson
 * Save mouse sensitivity
 * 
 * 25    8/31/97 9:27p Rjohnson
 * GL Updates
 * 
 * 24    8/29/97 2:49p Rjohnson
 * Network updates
 * 
 * 23    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 22    8/14/97 2:37p Rjohnson
 * Fix for save games
 * 
 * 21    7/21/97 9:25p Rjohnson
 * Reduces the amount of memory used by RJNET
 * 
 * 20    7/15/97 4:09p Rjohnson
 * New particle effect
 * 
 * 19    6/12/97 9:02a Rlove
 * New vorpal particle effect
 * 
 * 18    5/23/97 3:04p Rjohnson
 * Included some more quake2 things
 * 
 * 17    5/20/97 11:18a Bgokey
 * 
 * 16    5/19/97 2:54p Rjohnson
 * Added new client effects
 * 
 * 15    5/10/97 1:08p Bgokey
 * 
 * 14    4/30/97 11:20p Bgokey
 * 
 * 13    4/20/97 5:05p Rjohnson
 * Networking Update
 * 
 * 12    4/16/97 12:16p Rjohnson
 * Made the GIP files be stored in the temp directory
 * 
 * 11    4/16/97 12:02p Bgokey
 * 
 * 10    4/09/97 2:34p Rjohnson
 * Revised inventory
 * 
 * 9     3/31/97 7:24p Rjohnson
 * Added a playerclass field and made sure the server/clients handle it
 * properly
 * 
 * 8     3/07/97 12:23p Rjohnson
 * Id Updates
 * 
 * 7     3/07/97 12:06p Rjohnson
 * Added new spell particle effect
 * 
 * 6     2/20/97 12:13p Rjohnson
 * Code fixes for id update
 * 
 * 5     2/17/97 2:20p Rjohnson
 * Id Update
 * 
 * 4     1/02/97 11:16a Rjohnson
 * Christmas work - added adaptive time, game delays, negative light,
 * particle effects, etc
 * 
 * 3     12/11/96 10:45a Rjohnson
 * Added the new ice effect
 * 
 * 2     12/06/96 2:00p Rjohnson
 * New particle type for the fireball
 */
