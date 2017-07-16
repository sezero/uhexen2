/*
 * cl_parse.c -- parse a message received from the server
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
#include "r_shared.h"

static const char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",		// <see code>
	"svc_time",		// [float] server time
	"svc_print",		// [string] null terminated string
	"svc_stufftext",	// [string] stuffed into client's console buffer
				// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value

	"svc_serverinfo",	// [long] version
				// [string] signon string
				// [string]..[0]model cache [string]...[0]sounds cache
				// [string]..[0]item cache
	"svc_lightstyle",	// [byte] [string]
	"svc_updatename",	// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",	// <shortbits + data>
	"svc_stopsound",	// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",		// [byte] impact [byte] blood [vec3] from

	"svc_spawnstatic",
	"svc_raineffect",
	"svc_spawnbaseline",

	"svc_temp_entity",	// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",		// [string] music [string] text
	"svc_cdtrack",		// [byte] track [byte] looptrack
	"svc_sellscreen",
	"svc_particle2",	// [vec3] <variable>
	"svc_cutscene",
	"svc_midi_name",
	"svc_updateclass",	// [byte] client [byte] class
	"svc_particle3",
	"svc_particle4",
	"svc_set_view_flags",
	"svc_clear_view_flags",
	"svc_start_effect",
	"svc_end_effect",
	"svc_plaque",
	"svc_particle_explosion",
	"svc_set_view_tint",
	"svc_reference",
	"svc_clear_edicts",
	"svc_update_inv",
	"svc_setangle_interpolate",
	"svc_update_kingofhill",
	"svc_toggle_statbar",
	"svc_sound_update_pos",
	"svc_mod_name",	// UQE v1.13 by Korax, music file name
	"svc_skybox"	// UQE v1.13 by Korax, skybox name
};
#define	NUM_SVC_STRINGS	(sizeof(svc_strings) / sizeof(svc_strings[0]))

int		cl_protocol;	/* protocol version used by the server */
int		LastServerMessageSize;

qmodel_t	*player_models[MAX_PLAYER_CLASS];

extern	cvar_t	precache;
extern	qboolean menu_disabled_mouse;


//=============================================================================


/*
===============
CL_EntityNum

This error checks and tracks the total number of entities
===============
*/
static entity_t *CL_EntityNum (int num)
{
	if (num >= cl.num_entities)
	{
		if (num >= MAX_EDICTS)
			Host_Error ("%s: %i is an invalid number", __thisfunc__, num);
		while (cl.num_entities <= num)
		{
			cl_entities[cl.num_entities].colormap = vid.colormap;
			cl.num_entities++;
		}
	}

	return &cl_entities[num];
}


/*
==================
CL_ParseStartSoundPacket
==================
*/
static void CL_ParseStartSoundPacket(void)
{
	vec3_t	pos;
	int		channel, ent;
	int		sound_num, volume, field_mask;
	float	attenuation;
	int		i;

	field_mask = MSG_ReadByte();

	if (field_mask & SND_VOLUME)
		volume = MSG_ReadByte ();
	else	volume = DEFAULT_SOUND_PACKET_VOLUME;

	if (field_mask & SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else	attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

	channel = MSG_ReadShort ();
	sound_num = MSG_ReadByte ();
	if (field_mask & SND_OVERFLOW)
		sound_num += MAX_SOUNDS_OLD;
	if (field_mask & SND_OVERFLOW2)
	{
	/* this means the server has MAX_SOUNDS > 512 (== 1024)
	 * and is sending it to us as a byte: Kor Skarn's code.
	 * UQE does this but I haven't seen this in real life yet.
	 * Currently not supported.  TODO: in a newer protocol.
	 */
		sound_num += MAX_SOUNDS_H2MP;	/* +512  */
		Con_DPrintf("%s: field_mask & SND_OVERFLOW2 (%d)\n", __thisfunc__, sound_num);
		return;
	}

	ent = channel >> 3;
	channel &= 7;

	if (ent > MAX_EDICTS)
		Host_Error ("%s: ent = %i", __thisfunc__, ent);

	for (i = 0; i < 3; i++)
		pos[i] = MSG_ReadCoord ();

	S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);
}

/*
==================
CL_KeepaliveMessage

When the client is taking a long time to load stuff, send keepalive messages
so the server doesn't disconnect.
==================
*/
static void CL_KeepaliveMessage (void)
{
	float	time;
	static float lastmsg;
	int		ret;
	sizebuf_t	old;
	byte		olddata[NET_MAXMESSAGE];

	if (sv.active)
		return;		// no need if server is local
	if (cls.demoplayback)
		return;

// read messages from server, should just be nops
	old = net_message;
	memcpy (olddata, net_message.data, net_message.cursize);

	do
	{
		ret = CL_GetMessage ();
		switch (ret)
		{
		default:
			Host_Error ("%s: CL_GetMessage failed", __thisfunc__);
		case 0:
			break;	// nothing waiting
		case 1:
			Host_Error ("%s: received a message", __thisfunc__);
			break;
		case 2:
			if (MSG_ReadByte() != svc_nop)
				Host_Error ("%s: datagram wasn't a nop", __thisfunc__);
			break;
		}
	} while (ret);

	net_message = old;
	memcpy (net_message.data, olddata, net_message.cursize);

// check time
	time = Sys_DoubleTime ();
	if (time - lastmsg < 5)
		return;
	lastmsg = time;

// write out a nop
	Con_Printf ("--> client to server keepalive\n");

	MSG_WriteByte (&cls.message, clc_nop);
	NET_SendMessage (cls.netcon, &cls.message);
	SZ_Clear (&cls.message);
}

/*
==================
CL_ParseServerInfo
==================
*/
static void CL_ParseServerInfo (void)
{
	const char	*str;
	int		i;
	int		nummodels, numsounds;
	char	model_precache[MAX_MODELS][MAX_QPATH];
	char	sound_precache[MAX_SOUNDS][MAX_QPATH];
// rjr	edict_t		*ent;

	Con_DPrintf ("Serverinfo packet received.\n");

// bring up loading plaque for map changes within a demo.
// it will be hidden in CL_SignonReply() -- ericw
	if (cls.demoplayback)
		SCR_BeginLoadingPlaque();

//
// wipe the client_state_t struct
//
	CL_ClearState ();

// parse protocol version number
	cl_protocol = MSG_ReadLong ();
	switch (cl_protocol)
	{
	case PROTOCOL_RAVEN_111:
	case PROTOCOL_RAVEN_112:
	case PROTOCOL_UQE_113:
		Con_DPrintf ("\nServer using protocol %i\n", cl_protocol);
		break;
	default:
		Con_Printf ("\nServer returned version %i, not %i or %i\n",
				cl_protocol, PROTOCOL_RAVEN_112, PROTOCOL_UQE_113);
		return;
	}

// parse maxclients
	cl.maxclients = MSG_ReadByte ();
	if (cl.maxclients < 1 || cl.maxclients > MAX_CLIENTS)
	{
		Con_Printf("Bad maxclients (%d) from server\n", cl.maxclients);
		return;
	}
	cl.scores = (scoreboard_t *) Hunk_AllocName (cl.maxclients*sizeof(*cl.scores), "scores");

// parse gametype
	cl.gametype = MSG_ReadByte ();

	if (cl.gametype == GAME_DEATHMATCH && cl_protocol > PROTOCOL_RAVEN_111)
		sv_kingofhill = MSG_ReadShort ();

// parse signon message
	str = MSG_ReadString ();
	q_strlcpy (cl.levelname, str, sizeof(cl.levelname));

// seperate the printfs so the server message can have a color
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_Printf ("%c%s\n", 2, str);

//
// first we go through and touch all of the precache data that still
// happens to be in the cache, so precaching something else doesn't
// needlessly purge it
//

// precache models
	memset (cl.model_precache, 0, sizeof(cl.model_precache));
	for (nummodels = 1 ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (nummodels == MAX_MODELS)
		{
			Con_Printf ("Server sent too many model precaches\n");
			return;
		}
		q_strlcpy (model_precache[nummodels], str, MAX_QPATH);
		Mod_TouchModel (str);
	}

// precache sounds
	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));
	for (numsounds = 1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (numsounds == MAX_SOUNDS)
		{
			Con_Printf ("Server sent too many sound precaches\n");
			return;
		}
		q_strlcpy (sound_precache[numsounds], str, MAX_QPATH);
		S_TouchSound (str);
	}

//
// now we try to load everything else until a cache allocation fails
//

	if (precache.integer)
	{
		total_loading_size = nummodels + numsounds;
		current_loading_size = 1;
		loading_stage = 2;
	}

	// copy the naked name of the map file to the cl structure
	COM_FileBase (model_precache[1], cl.mapname, sizeof(cl.mapname));

	//always precache the world!!!
	cl.model_precache[1] = Mod_ForName (model_precache[1], false);
	for (i = 2; i < nummodels; i++)
	{
		if (precache.integer)
		{
			cl.model_precache[i] = Mod_ForName (model_precache[i], false);
			current_loading_size++;
			D_ShowLoadingSize();
		}
		else
			cl.model_precache[i] = (qmodel_t *)Mod_FindName (model_precache[i]);

		if (cl.model_precache[i] == NULL)
		{
			Host_Error("Model %s not found", model_precache[i]);
			return;
		}
		CL_KeepaliveMessage ();
	}

	player_models[0] = (qmodel_t *)Mod_FindName ("models/paladin.mdl");
	player_models[1] = !(gameflags & GAME_OLD_DEMO) ? (qmodel_t *)Mod_FindName ("models/crusader.mdl") : NULL;
	player_models[2] = !(gameflags & GAME_OLD_DEMO) ? (qmodel_t *)Mod_FindName ("models/necro.mdl") : NULL;
	player_models[3] = (qmodel_t *)Mod_FindName ("models/assassin.mdl");
	player_models[4] = (gameflags & GAME_PORTALS) ? (qmodel_t *)Mod_FindName ("models/succubus.mdl") : NULL;

	S_BeginPrecaching ();
	for (i = 1; i < numsounds; i++)
	{
		cl.sound_precache[i] = S_PrecacheSound (sound_precache[i]);
		if (precache.integer)
		{
			current_loading_size++;
			D_ShowLoadingSize();
		}

		CL_KeepaliveMessage ();
	}
	S_EndPrecaching ();

	total_loading_size = 0;
	loading_stage = 0;

// local state
	cl_entities[0].model = cl.worldmodel = cl.model_precache[1];

	R_NewMap ();

	if (!sv.active)
		Host_LoadStrings();
	CL_LoadPuzzleStrings();
	// mission pack, objectives strings
	if (gameflags & GAME_PORTALS)
		CL_LoadInfoStrings();

	Hunk_Check ();		// make sure nothing is hurt

// we connected to the server, make sure the mouse is going - S.A.
	menu_disabled_mouse = false;
	IN_ActivateMouse();
}


/*
==================
CL_ParseUpdate

Parse an entity update message from the server
If an entities model or origin changes from frame to frame, it must be
relinked.  Other attributes can change without relinking.
==================
*/
static void CL_ParseUpdate (int bits)
{
	int		i;
	qmodel_t	*model;
	int		modnum;
	qboolean	forcelink;
	entity_t	*ent;
	int		num;
	entity_state2_t *ref_ent, *set_ent, build_ent, dummy;

	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		CL_SignonReply ();
	}

	if (bits & U_MOREBITS)
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}

	if (bits & U_MOREBITS2)
	{
		i = MSG_ReadByte ();
		bits |= (i<<16);
	}

	if (bits & U_LONGENTITY)
		num = MSG_ReadShort ();
	else	num = MSG_ReadByte ();

	ent = CL_EntityNum (num);
	ent->baseline.flags |= BE_ON;
	ref_ent = NULL;

	for (i = 0; i < cl.frames[0].count; i++)
	{
		if (cl.frames[0].states[i].index == num)
		{
			ref_ent = &cl.frames[0].states[i];
			break;
		}
	}
	if (!ref_ent)
	{
		ref_ent = &build_ent;

		build_ent.index = num;
		build_ent.origin[0] = ent->baseline.origin[0];
		build_ent.origin[1] = ent->baseline.origin[1];
		build_ent.origin[2] = ent->baseline.origin[2];
		build_ent.angles[0] = ent->baseline.angles[0];
		build_ent.angles[1] = ent->baseline.angles[1];
		build_ent.angles[2] = ent->baseline.angles[2];
		build_ent.modelindex = ent->baseline.modelindex;
		build_ent.frame = ent->baseline.frame;
		build_ent.colormap = ent->baseline.colormap;
		build_ent.skin = ent->baseline.skin;
		build_ent.effects = ent->baseline.effects;
		build_ent.scale = ent->baseline.scale;
		build_ent.drawflags = ent->baseline.drawflags;
		build_ent.abslight = ent->baseline.abslight;
	}

	if (cl.need_build)
	{	// new sequence, first valid frame
		set_ent = &cl.frames[1].states[cl.frames[1].count];
		cl.frames[1].count++;
	}
	else
		set_ent = &dummy;

	if (bits & U_CLEAR_ENT)
	{
		memset(ent, 0, sizeof(entity_t));
		memset(ref_ent, 0, sizeof(*ref_ent));
		ref_ent->index = num;
	}

	*set_ent = *ref_ent;

	if (ent->msgtime != cl.mtime[1])
		forcelink = true;	// no previous frame to lerp from
	else	forcelink = false;

	ent->msgtime = cl.mtime[0];

	if (bits & U_MODEL)
	{
		modnum = MSG_ReadShort ();
		if (modnum >= MAX_MODELS)
			Host_Error ("%s: bad modnum", __thisfunc__);
	}
	else	modnum = ref_ent->modelindex;

	model = cl.model_precache[modnum];
	set_ent->modelindex = modnum;
	if (model != ent->model)
	{
		ent->model = model;
		// automatic animation (torches, etc) can
		// be either all together or randomized
		if (model)
		{
			if (model->synctype == ST_RAND)
				ent->syncbase = rand() * (1.0 / RAND_MAX);//(float)(rand() & 0x7fff) / 0x7fff;
			else	ent->syncbase = 0.0;
		}
		else	forcelink = true;	// hack to make null model players work
#ifdef GLQUAKE
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
#endif
	}

	if (bits & U_FRAME)
		set_ent->frame = ent->frame = MSG_ReadByte ();
	else	ent->frame = ref_ent->frame;

	if (bits & U_COLORMAP)
		set_ent->colormap = i = MSG_ReadByte();
	else	i = ref_ent->colormap;

	if (num && num <= cl.maxclients)
		ent->colormap = ent->sourcecolormap = cl.scores[num-1].translations;
	else	ent->sourcecolormap = vid.colormap;

#ifdef GLQUAKE
//	ent->colormap = vid.colormap;
#endif

	if (!i)
	{
		ent->colorshade = i;
		ent->colormap = ent->sourcecolormap;
	}
	else
	{
		ent->colorshade = i;
#ifdef GLQUAKE
//		ent->colormap = vid.colormap;
		ent->colormap = globalcolormap;
#else
		ent->colormap = globalcolormap;
#endif
	}

	if (bits & U_SKIN)
	{
		set_ent->skin = ent->skinnum = MSG_ReadByte();
		set_ent->drawflags = ent->drawflags = MSG_ReadByte();
	}
	else
	{
		ent->skinnum = ref_ent->skin;
		ent->drawflags = ref_ent->drawflags;
	}

	if (bits & U_EFFECTS)
		set_ent->effects = ent->effects = MSG_ReadByte();
	else	ent->effects = ref_ent->effects;

// shift the known values for interpolation
	VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
	VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);

	if (bits & U_ORIGIN1)
		set_ent->origin[0] = ent->msg_origins[0][0] = MSG_ReadCoord ();
	else	ent->msg_origins[0][0] = ref_ent->origin[0];

	if (bits & U_ANGLE1)
		set_ent->angles[0] = ent->msg_angles[0][0] = MSG_ReadAngle();
	else	ent->msg_angles[0][0] = ref_ent->angles[0];

	if (bits & U_ORIGIN2)
		set_ent->origin[1] = ent->msg_origins[0][1] = MSG_ReadCoord ();
	else	ent->msg_origins[0][1] = ref_ent->origin[1];

	if (bits & U_ANGLE2)
		set_ent->angles[1] = ent->msg_angles[0][1] = MSG_ReadAngle();
	else	ent->msg_angles[0][1] = ref_ent->angles[1];

	if (bits & U_ORIGIN3)
		set_ent->origin[2] = ent->msg_origins[0][2] = MSG_ReadCoord ();
	else	ent->msg_origins[0][2] = ref_ent->origin[2];

	if (bits & U_ANGLE3)
		set_ent->angles[2] = ent->msg_angles[0][2] = MSG_ReadAngle();
	else	ent->msg_angles[0][2] = ref_ent->angles[2];

	if (bits & U_SCALE)
	{
		set_ent->scale = ent->scale = MSG_ReadByte();
		set_ent->abslight = ent->abslight = MSG_ReadByte();
	}
	else
	{
		ent->scale = ref_ent->scale;
		ent->abslight = ref_ent->abslight;
	}

	if (bits & U_NOLERP)
		ent->forcelink = true;

	if (forcelink)
	{	// didn't have an update last message
		VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
		VectorCopy (ent->msg_origins[0], ent->origin);
		VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);
		VectorCopy (ent->msg_angles[0], ent->angles);
		ent->forcelink = true;
	}
}

static void CL_ParseUpdate2 (int bits)
{
	int	i;

	if (bits & U_MOREBITS)
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}
	if (bits & U_MOREBITS2)
	{
		i = MSG_ReadByte ();
		bits |= (i<<16);
	}
	if (bits & U_LONGENTITY)
		MSG_ReadShort ();
	else	MSG_ReadByte ();
	if (bits & U_MODEL)
		MSG_ReadShort ();
	if (bits & U_FRAME)
		MSG_ReadByte ();
	if (bits & U_COLORMAP)
		MSG_ReadByte();
	if (bits & U_SKIN)
	{
		MSG_ReadByte();
		MSG_ReadByte();
	}
	if (bits & U_EFFECTS)
		MSG_ReadByte();
	if (bits & U_ORIGIN1)
		MSG_ReadCoord ();
	if (bits & U_ANGLE1)
		MSG_ReadAngle();
	if (bits & U_ORIGIN2)
		MSG_ReadCoord ();
	if (bits & U_ANGLE2)
		MSG_ReadAngle();
	if (bits & U_ORIGIN3)
		MSG_ReadCoord ();
	if (bits & U_ANGLE3)
		MSG_ReadAngle();
	if (bits & U_SCALE)
	{
		MSG_ReadByte();
		MSG_ReadByte();
	}
}

/*
==================
CL_ParseBaseline
==================
*/
static void CL_ParseBaseline (entity_t *ent)
{
	int	i;

	ent->baseline.modelindex = MSG_ReadShort ();
	ent->baseline.frame = MSG_ReadByte ();
	ent->baseline.colormap = MSG_ReadByte();
	ent->baseline.skin = MSG_ReadByte();
	ent->baseline.scale = MSG_ReadByte();
	ent->baseline.drawflags = MSG_ReadByte();
	ent->baseline.abslight = MSG_ReadByte();
	for (i = 0; i < 3; i++)
	{
		ent->baseline.origin[i] = MSG_ReadCoord ();
		ent->baseline.angles[i] = MSG_ReadAngle ();
	}
}


/*
==================
CL_ParseClientdata

Server information pertaining to this client only
==================
*/
static void CL_ParseClientdata (int bits)
{
	int	i, j;

	if (bits & SU_VIEWHEIGHT)
		cl.viewheight = MSG_ReadChar ();
//rjr	else	cl.viewheight = DEFAULT_VIEWHEIGHT;

	if (bits & SU_IDEALPITCH)
		cl.idealpitch = MSG_ReadChar ();

	if (bits & SU_IDEALROLL)
		cl.idealroll = MSG_ReadChar ();
//rjr	else	cl.idealroll = 0;

	VectorCopy (cl.mvelocity[0], cl.mvelocity[1]);
	for (i = 0; i < 3; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			cl.punchangle[i] = MSG_ReadChar();
//rjr		else	cl.punchangle[i] = 0;
		if (bits & (SU_VELOCITY1<<i))
			cl.mvelocity[0][i] = MSG_ReadChar()*16;
//rjr		else	cl.mvelocity[0][i] = 0;
	}

	/*
	if (bits & SU_ITEMS)
		i = MSG_ReadLong ();
	*/

	if (cl.items != i)
	{	// set flash times
		Sbar_Changed();
		for (j = 0; j < 32; j++)
			if ((i & (1<<j)) && !(cl.items & (1<<j)))
				cl.item_gettime[j] = cl.time;
		cl.items = i;
	}

	cl.onground = ((bits & SU_ONGROUND) != 0);
	cl.inwater  = ((bits & SU_INWATER)  != 0);

	if (bits & SU_WEAPONFRAME)
		cl.stats[STAT_WEAPONFRAME] = MSG_ReadByte ();
//rjr	else	cl.stats[STAT_WEAPONFRAME] = 0;

	if (bits & SU_ARMOR)
	{
		cl.stats[STAT_ARMOR] = MSG_ReadByte ();
		//Sbar_Changed();
	}

	if (bits & SU_WEAPON)
	{
		cl.stats[STAT_WEAPON] = MSG_ReadShort ();
		//Sbar_Changed();
	}

	/*
	sc1 = sc2 = 0;

	if (bits & SU_SC1)
		sc1 = MSG_ReadLong ();
	if (bits & SU_SC2)
		sc2 = MSG_ReadLong ();

	if (sc1 & SC1_HEALTH)
		cl.v.health = MSG_ReadShort();
	if (sc1 & SC1_LEVEL)
		cl.v.level = MSG_ReadByte();
	if (sc1 & SC1_INTELLIGENCE)
		cl.v.intelligence = MSG_ReadByte();
	if (sc1 & SC1_WISDOM)
		cl.v.wisdom = MSG_ReadByte();
	if (sc1 & SC1_STRENGTH)
		cl.v.strength = MSG_ReadByte();
	if (sc1 & SC1_DEXTERITY)
		cl.v.dexterity = MSG_ReadByte();
	if (sc1 & SC1_WEAPON)
		cl.v.weapon = MSG_ReadByte();
	if (sc1 & SC1_BLUEMANA)
		cl.v.bluemana = MSG_ReadByte();
	if (sc1 & SC1_GREENMANA)
		cl.v.greenmana = MSG_ReadByte();
	if (sc1 & SC1_EXPERIENCE)
		cl.v.experience = MSG_ReadLong();
	if (sc1 & SC1_CNT_TORCH)
		cl.v.cnt_torch = MSG_ReadByte();
	if (sc1 & SC1_CNT_H_BOOST)
		cl.v.cnt_h_boost = MSG_ReadByte();
	if (sc1 & SC1_CNT_SH_BOOST)
		cl.v.cnt_sh_boost = MSG_ReadByte();
	if (sc1 & SC1_CNT_MANA_BOOST)
		cl.v.cnt_mana_boost = MSG_ReadByte();
	if (sc1 & SC1_CNT_TELEPORT)
		cl.v.cnt_teleport = MSG_ReadByte();
	if (sc1 & SC1_CNT_TOME)
		cl.v.cnt_tome = MSG_ReadByte();
	if (sc1 & SC1_CNT_SUMMON)
		cl.v.cnt_summon = MSG_ReadByte();
	if (sc1 & SC1_CNT_INVISIBILITY)
		cl.v.cnt_invisibility = MSG_ReadByte();
	if (sc1 & SC1_CNT_GLYPH)
		cl.v.cnt_glyph = MSG_ReadByte();
	if (sc1 & SC1_CNT_HASTE)
		cl.v.cnt_haste = MSG_ReadByte();
	if (sc1 & SC1_CNT_BLAST)
		cl.v.cnt_blast = MSG_ReadByte();
	if (sc1 & SC1_CNT_POLYMORPH)
		cl.v.cnt_polymorph = MSG_ReadByte();
	if (sc1 & SC1_CNT_FLIGHT)
		cl.v.cnt_flight = MSG_ReadByte();
	if (sc1 & SC1_CNT_CUBEOFFORCE)
		cl.v.cnt_cubeofforce = MSG_ReadByte();
	if (sc1 & SC1_CNT_INVINCIBILITY)
		cl.v.cnt_invincibility = MSG_ReadByte();
	if (sc1 & SC1_ARTIFACT_ACTIVE)
		cl.v.artifact_active = MSG_ReadFloat();
	if (sc1 & SC1_ARTIFACT_LOW)
		cl.v.artifact_low = MSG_ReadFloat();
	if (sc1 & SC1_MOVETYPE)
		cl.v.movetype = MSG_ReadByte();
	if (sc1 & SC1_CAMERAMODE)
		cl.v.cameramode = MSG_ReadByte();
	if (sc1 & SC1_HASTED)
		cl.v.hasted = MSG_ReadFloat();
	if (sc1 & SC1_INVENTORY)
		cl.v.inventory = MSG_ReadByte();
	if (sc1 & SC1_RINGS_ACTIVE)
		cl.v.rings_active = MSG_ReadFloat();

	if (sc2 & SC2_RINGS_LOW)
		cl.v.rings_low = MSG_ReadFloat();
	if (sc2 & SC2_AMULET)
		cl.v.armor_amulet = MSG_ReadByte();
	if (sc2 & SC2_BRACER)
		cl.v.armor_bracer = MSG_ReadByte();
	if (sc2 & SC2_BREASTPLATE)
		cl.v.armor_breastplate = MSG_ReadByte();
	if (sc2 & SC2_HELMET)
		cl.v.armor_helmet = MSG_ReadByte();
	if (sc2 & SC2_FLIGHT_T)
		cl.v.ring_flight = MSG_ReadByte();
	if (sc2 & SC2_WATER_T)
		cl.v.ring_water = MSG_ReadByte();
	if (sc2 & SC2_TURNING_T)
		cl.v.ring_turning = MSG_ReadByte();
	if (sc2 & SC2_REGEN_T)
		cl.v.ring_regeneration = MSG_ReadByte();
	if (sc2 & SC2_HASTE_T)
		cl.v.haste_time = MSG_ReadFloat();
	if (sc2 & SC2_TOME_T)
		cl.v.tome_time = MSG_ReadFloat();
	if (sc2 & SC2_PUZZLE1)
		q_snprintf(cl.puzzle_pieces[0], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE2)
		q_snprintf(cl.puzzle_pieces[1], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE3)
		q_snprintf(cl.puzzle_pieces[2], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE4)
		q_snprintf(cl.puzzle_pieces[3], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE5)
		q_snprintf(cl.puzzle_pieces[4], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE6)
		q_snprintf(cl.puzzle_pieces[5], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE7)
		q_snprintf(cl.puzzle_pieces[6], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE8)
		q_snprintf(cl.puzzle_pieces[7], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
	if (sc2 & SC2_MAXHEALTH)
		cl.v.max_health = MSG_ReadShort();
	if (sc2 & SC2_MAXMANA)
		cl.v.max_mana = MSG_ReadByte();
	if (sc2 & SC2_FLAGS)
		cl.v.flags = MSG_ReadFloat();

	if ((sc1 & SC1_STAT_BAR) || (sc2 & SC2_STAT_BAR))
		Sbar_Changed();

	if ((sc1 & SC1_INV) || (sc2 & SC2_INV))
		SB_InvChanged();
	*/
}

/*
=====================
CL_NewTranslation
=====================
*/
static void CL_NewTranslation (int slot)
{
#ifndef GLQUAKE
	int	i, j;
	int	top, bottom;
	byte	*dest, *source, *sourceA, *sourceB, *colorA, *colorB;
#endif

	if (slot >= cl.maxclients)
		Sys_Error ("%s: slot > cl.maxclients", __thisfunc__);
	if (!cl.scores[slot].playerclass)
		return;

#ifdef GLQUAKE
	R_TranslatePlayerSkin (slot);
	return;
#else

	dest = cl.scores[slot].translations;
	source = vid.colormap;
	memcpy (dest, vid.colormap, sizeof(cl.scores[slot].translations));
	top = (cl.scores[slot].colors & 0xf0) >> 4;
	bottom = (cl.scores[slot].colors & 15);

	if (top > 11 || bottom > 11)
		Con_Printf("Invalid Player Color: %d,%d\n", top, bottom);
	if (top > 10)
		top = 0;
	if (bottom > 10)
		bottom = 0;
	top -= 1;
	bottom -= 1;

//	Con_Printf("Class is %d for slot %d\n",(int)cl.scores[slot].playerclass,slot);
	for (i = 0; i < VID_GRADES; i++, dest += 256, source += 256)
	{
		colorA = playerTranslation + 256 + color_offsets[(int)cl.scores[slot].playerclass-1];
		colorB = colorA + 256;
		sourceA = colorB + 256 + (top * 256);
		sourceB = colorB + 256 + (bottom * 256);
		for (j = 0; j < 256; j++, colorA++, colorB++, sourceA++, sourceB++)
		{
			if (top >= 0 && (*colorA != 255))
				dest[j] = source[*sourceA];
			if (bottom >= 0 && (*colorB != 255))
				dest[j] = source[*sourceB];
		}
	}
#endif
}

/*
=====================
CL_ParseStatic
=====================
*/
static void CL_ParseStatic (void)
{
	entity_t *ent;
	int	i;

	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities");
	ent = &cl_static_entities[i];
	cl.num_statics++;
	CL_ParseBaseline (ent);

// copy it to the current state
	ent->model = cl.model_precache[ent->baseline.modelindex];
	ent->frame = ent->baseline.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = ent->baseline.skin;
	ent->scale = ent->baseline.scale;
	ent->effects = ent->baseline.effects;
	ent->drawflags = ent->baseline.drawflags;
	ent->abslight = ent->baseline.abslight;

	VectorCopy (ent->baseline.origin, ent->origin);
	VectorCopy (ent->baseline.angles, ent->angles);
	R_AddEfrags (ent);
}

/*
===================
CL_ParseStaticSound
===================
*/
static void CL_ParseStaticSound (void)
{
	vec3_t	org;
	int	sound_num, vol, atten;
	int	i;

	for (i = 0; i < 3; i++)
		org[i] = MSG_ReadCoord ();

	if (cl_protocol == PROTOCOL_RAVEN_111)
		sound_num = MSG_ReadByte ();
	else	sound_num = MSG_ReadShort();
	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();

	S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}


static void CL_Plaque(void)
{
	int	idx;

	idx = MSG_ReadShort ();

	if (idx > 0 && idx <= host_string_count)
		SCR_SetPlaqueMessage (Host_GetString(idx - 1));
	else	SCR_SetPlaqueMessage ("");
}

static void CL_ParticleExplosion(void)
{
	vec3_t	org;
	short	color, radius, counter;

	org[0] = MSG_ReadCoord();
	org[1] = MSG_ReadCoord();
	org[2] = MSG_ReadCoord();
	color = MSG_ReadShort();
	radius = MSG_ReadShort();
	counter = MSG_ReadShort();

	R_ColoredParticleExplosion(org,color,radius,counter);
}

static void CL_ParseRainEffect(void)
{
	vec3_t	org, e_size;
	short	color,count;
	int	x_dir, y_dir;

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

	R_RainEffect(org,e_size,x_dir,y_dir,color,count);
}

#if 0	/* for debugging. from fteqw. */
static void CL_DumpPacket (void)
{
	int			i, pos;
	char	*packet = net_message.data;

	Con_Printf("%s, BEGIN:\n", __thisfunc__);
	pos = 0;
	while (pos < net_message.cursize)
	{
		Con_Printf("%5i ", pos);
		for (i = 0; i < 16; i++)
		{
			if (pos >= net_message.cursize)
				Con_Printf(" X ");
			else	Con_Printf("%2x ", (unsigned char)packet[pos]);
			pos++;
		}
		pos -= 16;
		for (i = 0; i < 16; i++)
		{
			if (pos >= net_message.cursize)
				Con_Printf("X");
			else if (packet[pos] == 0)
				Con_Printf(".");
			else	Con_Printf("%c", (unsigned char)packet[pos]);
			pos++;
		}
		Con_Printf("\n");
	}

	Con_Printf("%s, --- END ---\n", __thisfunc__);
}
#endif	/* CL_DumpPacket */

#define SHOWNET(S)						\
do {								\
	if (cl_shownet.integer == 2)				\
		Con_Printf ("%3i:%s\n", msg_readcount-1, (S));	\
} while (0)

/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage (void)
{
	int		cmd;
	int		i, j, k;
	int		EntityCount = 0;
	int		EntitySize = 0;
	int		before;
	static		double lasttime;
	static		qboolean packet_loss = false;
	entity_t	*ent;
	int		sc1, sc2;
	byte		test;
	float		compangles[2][3];
	vec3_t		deltaangles;

// if recording demos, copy the message out
	if (net_message.cursize > LastServerMessageSize)
	{
		LastServerMessageSize = net_message.cursize;
	}
	if (cl_shownet.integer == 1)
	{
		Con_Printf ("Time: %2.2f Pck: %i ", realtime - lasttime, net_message.cursize);
		lasttime = realtime;
	}
	else if (cl_shownet.integer == 2)
	{
		Con_Printf ("------------------\n");
	}

	cl.onground = false;	// unless the server says otherwise

// parse the message
	MSG_BeginReading ();

	while (1)
	{
		if (msg_badread)
			Host_Error ("%s: Bad server message", __thisfunc__);

		cmd = MSG_ReadByte ();
		if (cmd == -1)
		{
			if (cl_shownet.integer == 1)
				Con_Printf ("Ent: %i (%i bytes)", EntityCount, EntitySize);
			SHOWNET("END OF MESSAGE");
			return; // end of message
		}

	// if the high bit of the command byte is set, it is a fast update
		if (cmd & 128)
		{
			before = msg_readcount;
			SHOWNET("fast update");
			if (packet_loss)
				CL_ParseUpdate2 (cmd & 127);
			else	CL_ParseUpdate (cmd & 127);

			EntityCount++;
			EntitySize += msg_readcount - before + 1;
			continue;
		}

		if (cmd < (int)NUM_SVC_STRINGS) {
			SHOWNET(svc_strings[cmd]);
		}

	// other commands
		switch (cmd)
		{
		default:
		//	CL_DumpPacket ();
			Host_Error ("%s: Illegible server message %d", __thisfunc__, cmd);
			break;

		case svc_nop:
		//	Con_Printf ("svc_nop\n");
			break;

		case svc_time:
			cl.mtime[1] = cl.mtime[0];
			cl.mtime[0] = MSG_ReadFloat ();
			break;

		case svc_clientdata:
			i = MSG_ReadShort ();
			CL_ParseClientdata (i);
			break;

		case svc_version:
			cl_protocol = MSG_ReadLong ();
			switch (cl_protocol)
			{
			case PROTOCOL_RAVEN_111:
			case PROTOCOL_RAVEN_112:
			case PROTOCOL_UQE_113:
				Con_Printf ("Server using protocol %i\n", cl_protocol);
				break;
			default:
				Host_Error ("%s: Server is protocol %i instead of %i or %i",
						__thisfunc__, cl_protocol,
						PROTOCOL_RAVEN_112, PROTOCOL_UQE_113);
			}
			break;

		case svc_disconnect:
			Host_EndGame ("Server disconnected\n");
			break;

		case svc_print:
			if (intro_playing) {
				MSG_ReadString ();
				break;
			}
			Con_Printf ("%s", MSG_ReadString ());
			break;

		case svc_centerprint:
			SCR_CenterPrint (MSG_ReadString ());
			break;

		case svc_stufftext:
			Cbuf_AddText (MSG_ReadString ());
			break;

		case svc_damage:
			V_ParseDamage ();
			break;

		case svc_serverinfo:
			CL_ParseServerInfo ();
			vid.recalc_refdef = true;	// leave intermission full screen
			break;

		case svc_setangle:
			for (i = 0; i < 3; i++)
				cl.viewangles[i] = MSG_ReadAngle ();
			break;

		case svc_setangle_interpolate:
			compangles[0][0] = MSG_ReadAngle();
			compangles[0][1] = MSG_ReadAngle();
			compangles[0][2] = MSG_ReadAngle();
			for (i = 0; i < 3; i++)
			{
				compangles[1][i] = cl.viewangles[i];
				for (j = 0; j < 2; j++)
				{//standardize both old and new angles to +-180
					if (compangles[j][i] >= 360)
						compangles[j][i] -= 360*((int)(compangles[j][i]/360));
					else if (compangles[j][i] <= 360)
						compangles[j][i] += 360*(1+(int)(-compangles[j][i]/360));
					if (compangles[j][i] > 180)
						compangles[j][i] = -360 + compangles[j][i];
					else if (compangles[j][i] < -180)
						compangles[j][i] = 360 + compangles[j][i];
				}
				//get delta
				deltaangles[i] = compangles[0][i] - compangles[1][i];
				//cap delta to <=180,>=-180
				if (deltaangles[i] > 180)
					deltaangles[i] += -360;
				else if (deltaangles[i] < -180)
					deltaangles[i] += 360;
				//add the delta
				cl.viewangles[i]+=(deltaangles[i]/8);//8 step interpolation
				//cap newangles to +-180
				if (cl.viewangles[i] >= 360)
					cl.viewangles[i] -= 360*((int)(cl.viewangles[i]/360));
				else if (cl.viewangles[i] <= 360)
					cl.viewangles[i] += 360*(1+(int)(-cl.viewangles[i]/360));
				if (cl.viewangles[i] > 180)
					cl.viewangles[i] += -360;
				else if (cl.viewangles[i] < -180)
					cl.viewangles[i] += 360;
			}
			break;

		case svc_setview:
			cl.viewentity = MSG_ReadShort ();
			break;

		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Sys_Error ("svc_lightstyle > MAX_LIGHTSTYLES");
			q_strlcpy (cl_lightstyle[i].map, MSG_ReadString(), MAX_STYLESTRING);
			cl_lightstyle[i].length = strlen(cl_lightstyle[i].map);
			break;

		case svc_sound:
			CL_ParseStartSoundPacket();
			break;

		case svc_sound_update_pos:
		  {	// FIXME: put a field on the entity that lists the channels
			// it should update when it moves- if a certain flag
			// is on the ent, this update_channels field could
			// be set automatically by each sound and stopSound
			// called for this ent?
			vec3_t	pos;
			int	channel, ent_num;

			channel = MSG_ReadShort ();
			ent_num = channel >> 3;
			channel &= 7;

			if (ent_num > MAX_EDICTS)
				Host_Error ("svc_sound_update_pos: ent = %i", ent_num);

			for (i = 0; i < 3; i++)
				pos[i] = MSG_ReadCoord ();

			S_UpdateSoundPos (ent_num, channel, pos);
		  }	break;

		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;

		case svc_updatename:
			Sbar_Changed();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("%s: svc_updatename > MAX_CLIENTS", __thisfunc__);
			q_strlcpy (cl.scores[i].name, MSG_ReadString(), MAX_SCOREBOARDNAME);
			break;

		case svc_updateclass:
			Sbar_Changed();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("%s: svc_updateclass > MAX_CLIENTS", __thisfunc__);
			cl.scores[i].playerclass = (float)MSG_ReadByte();
			CL_NewTranslation(i); // update the color
			break;

		case svc_updatefrags:
			Sbar_Changed();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("%s: svc_updatefrags > MAX_CLIENTS", __thisfunc__);
			cl.scores[i].frags = MSG_ReadShort ();
			break;

		case svc_update_kingofhill:
			sv_kingofhill = MSG_ReadShort() - 1;
			break;

		case svc_updatecolors:
			Sbar_Changed();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("%s: svc_updatecolors > MAX_CLIENTS", __thisfunc__);
			cl.scores[i].colors = MSG_ReadByte ();
			CL_NewTranslation (i);
			break;

		case svc_particle:
			R_ParseParticleEffect ();
			break;

		case svc_particle2:
			R_ParseParticleEffect2 ();
			break;
		case svc_particle3:
			R_ParseParticleEffect3 ();
			break;
		case svc_particle4:
			R_ParseParticleEffect4 ();
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			CL_ParseBaseline (CL_EntityNum(i));
			break;
		case svc_spawnstatic:
			CL_ParseStatic ();
			break;

		case svc_raineffect:
			CL_ParseRainEffect();
			break;

		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_setpause:
			cl.paused = MSG_ReadByte ();
			if (cl.paused)
			{
				CDAudio_Pause ();
				BGM_Pause ();
				VID_HandlePause (true);
			}
			else
			{
				CDAudio_Resume ();
				BGM_Resume ();
				VID_HandlePause (false);
			}
			break;

		case svc_signonnum:
			i = MSG_ReadByte ();
			if (i <= cls.signon)
				Host_Error ("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
			CL_SignonReply ();
			break;

		case svc_killedmonster:
			cl.stats[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			cl.stats[STAT_SECRETS]++;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();
			if (i < 0 || i >= MAX_CL_STATS)
				Sys_Error ("svc_updatestat: %i is invalid", i);
			cl.stats[i] = MSG_ReadLong ();
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound ();
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			cl.looptrack = MSG_ReadByte ();
			if (q_strcasecmp(bgmtype.string,"cd") != 0)
				CDAudio_Stop ();
			else if ((cls.demoplayback || cls.demorecording) &&
						cls.forcetrack != -1)
				CDAudio_Play ((byte)cls.forcetrack, true);
			else	CDAudio_Play ((byte)cl.cdtrack, true);
			break;

		case svc_midi_name:
			q_strlcpy (cl.midi_name, MSG_ReadString(), sizeof(cl.midi_name));
			if (q_strcasecmp(bgmtype.string,"midi") != 0)
				BGM_Stop();
			else	BGM_PlayMIDIorMusic(cl.midi_name);
			break;

		case svc_toggle_statbar:
			break;

		case svc_intermission:
			CL_SetupIntermission (MSG_ReadByte());
			vid.recalc_refdef = true;	// go to full screen
			break;

		/*
		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());
			break;

		case svc_cutscene:
			cl.intermission = 3;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());
			break;

		case svc_sellscreen:
			Cmd_ExecuteString ("help", src_command);
			break;
		*/

		case svc_set_view_flags:
			cl.viewent.drawflags |= MSG_ReadByte();
			break;

		case svc_clear_view_flags:
			cl.viewent.drawflags &= ~MSG_ReadByte();
			break;

		case svc_start_effect:
			CL_ParseEffect();
			break;
		case svc_end_effect:
			CL_EndEffect();
			break;

		case svc_plaque:
			CL_Plaque();
			break;

		case svc_particle_explosion:
			CL_ParticleExplosion();
			break;

		case svc_set_view_tint:
			i = MSG_ReadByte();
			cl.viewent.colorshade = i;
			break;

		case svc_reference:
		  {	short RemovePlace, OrigPlace, NewPlace, AddedIndex;

			packet_loss = false;
			cl.last_frame = cl.current_frame;
			cl.last_sequence = cl.current_sequence;
			cl.current_frame = MSG_ReadByte();
			cl.current_sequence = MSG_ReadByte();
			if (cl.need_build == 2)
			{
			//	Con_Printf("CL: NB2 CL(%d,%d) R(%d)\n",
			//		cl.current_sequence, cl.current_frame, cl.reference_frame);
				cl.frames[0].count = cl.frames[1].count = cl.frames[2].count = 0;
				cl.need_build = 1;
				cl.reference_frame = cl.current_frame;
			}
			else if (cl.last_sequence != cl.current_sequence)
			{
			//	Con_Printf("CL: Sequence CL(%d,%d) R(%d)\n",
			//		cl.current_sequence, cl.current_frame, cl.reference_frame);
				if (cl.reference_frame >= 1 && cl.reference_frame <= MAX_FRAMES)
				{
					RemovePlace = OrigPlace = NewPlace = AddedIndex = 0;
					for (i = 0; i < cl.num_entities; i++)
					{
						if (RemovePlace >= cl.NumToRemove ||
						    cl.RemoveList[RemovePlace] != i)
						{
							if (NewPlace < cl.frames[1].count &&
							    cl.frames[1].states[NewPlace].index == i)
							{
								cl.frames[2].states[AddedIndex] =
									cl.frames[1].states[NewPlace];
								AddedIndex++;
								cl.frames[2].count++;
							}
							else if (OrigPlace < cl.frames[0].count &&
								 cl.frames[0].states[OrigPlace].index == i)
							{
								cl.frames[2].states[AddedIndex] =
									cl.frames[0].states[OrigPlace];
								AddedIndex++;
								cl.frames[2].count++;
							}
						}
						else	RemovePlace++;

						if (cl.frames[0].states[OrigPlace].index == i)
							OrigPlace++;
						if (cl.frames[1].states[NewPlace].index == i)
							NewPlace++;
					}
					cl.frames[0] = cl.frames[2];
				}
				cl.frames[1].count = cl.frames[2].count = 0;
				cl.need_build = 1;
				cl.reference_frame = cl.current_frame;
			}
			else
			{
			//	Con_Printf("CL: Normal CL(%d,%d) R(%d)\n",
			//		cl.current_sequence, cl.current_frame,cl.reference_frame);
				cl.need_build = 0;
			}

			for (i = 1, ent = cl_entities+1; i < cl.num_entities; i++, ent++)
				ent->baseline.flags &= ~BE_ON;
			for (i = 0; i < cl.frames[0].count; i++)
			{
				ent = CL_EntityNum (cl.frames[0].states[i].index);
				ent->model = cl.model_precache[cl.frames[0].states[i].modelindex];
				ent->baseline.flags |= BE_ON;
			}
		  }	break;

		case svc_clear_edicts:
			j = MSG_ReadByte();
			if (cl.need_build)
				cl.NumToRemove = j;
			for (i = 0; i < j; i++)
			{
				k = MSG_ReadShort();
				if (cl.need_build)
					cl.RemoveList[i] = k;
				ent = CL_EntityNum (k);
				ent->baseline.flags &= ~BE_ON;
			}
			break;

		case svc_update_inv:
			sc1 = sc2 = 0;

			test = MSG_ReadByte();
			if (test & 1)
				sc1 |= ((int)MSG_ReadByte());
			if (test & 2)
				sc1 |= ((int)MSG_ReadByte())<<8;
			if (test & 4)
				sc1 |= ((int)MSG_ReadByte())<<16;
			if (test & 8)
				sc1 |= ((int)MSG_ReadByte())<<24;
			if (test & 16)
				sc2 |= ((int)MSG_ReadByte());
			if (test & 32)
				sc2 |= ((int)MSG_ReadByte())<<8;
			if (test & 64)
				sc2 |= ((int)MSG_ReadByte())<<16;
			if (test & 128)
				sc2 |= ((int)MSG_ReadByte())<<24;

			if (sc1 & SC1_HEALTH)
				cl.v.health = MSG_ReadShort();
			if (sc1 & SC1_LEVEL)
				cl.v.level = MSG_ReadByte();
			if (sc1 & SC1_INTELLIGENCE)
				cl.v.intelligence = MSG_ReadByte();
			if (sc1 & SC1_WISDOM)
				cl.v.wisdom = MSG_ReadByte();
			if (sc1 & SC1_STRENGTH)
				cl.v.strength = MSG_ReadByte();
			if (sc1 & SC1_DEXTERITY)
				cl.v.dexterity = MSG_ReadByte();
			if (sc1 & SC1_WEAPON)
				cl.v.weapon = MSG_ReadByte();
			if (sc1 & SC1_BLUEMANA)
				cl.v.bluemana = MSG_ReadByte();
			if (sc1 & SC1_GREENMANA)
				cl.v.greenmana = MSG_ReadByte();
			if (sc1 & SC1_EXPERIENCE)
				cl.v.experience = MSG_ReadLong();
			if (sc1 & SC1_CNT_TORCH)
				cl.v.cnt_torch = MSG_ReadByte();
			if (sc1 & SC1_CNT_H_BOOST)
				cl.v.cnt_h_boost = MSG_ReadByte();
			if (sc1 & SC1_CNT_SH_BOOST)
				cl.v.cnt_sh_boost = MSG_ReadByte();
			if (sc1 & SC1_CNT_MANA_BOOST)
				cl.v.cnt_mana_boost = MSG_ReadByte();
			if (sc1 & SC1_CNT_TELEPORT)
				cl.v.cnt_teleport = MSG_ReadByte();
			if (sc1 & SC1_CNT_TOME)
				cl.v.cnt_tome = MSG_ReadByte();
			if (sc1 & SC1_CNT_SUMMON)
				cl.v.cnt_summon = MSG_ReadByte();
			if (sc1 & SC1_CNT_INVISIBILITY)
				cl.v.cnt_invisibility = MSG_ReadByte();
			if (sc1 & SC1_CNT_GLYPH)
				cl.v.cnt_glyph = MSG_ReadByte();
			if (sc1 & SC1_CNT_HASTE)
				cl.v.cnt_haste = MSG_ReadByte();
			if (sc1 & SC1_CNT_BLAST)
				cl.v.cnt_blast = MSG_ReadByte();
			if (sc1 & SC1_CNT_POLYMORPH)
				cl.v.cnt_polymorph = MSG_ReadByte();
			if (sc1 & SC1_CNT_FLIGHT)
				cl.v.cnt_flight = MSG_ReadByte();
			if (sc1 & SC1_CNT_CUBEOFFORCE)
				cl.v.cnt_cubeofforce = MSG_ReadByte();
			if (sc1 & SC1_CNT_INVINCIBILITY)
				cl.v.cnt_invincibility = MSG_ReadByte();
			if (sc1 & SC1_ARTIFACT_ACTIVE)
				cl.v.artifact_active = MSG_ReadFloat();
			if (sc1 & SC1_ARTIFACT_LOW)
				cl.v.artifact_low = MSG_ReadFloat();
			if (sc1 & SC1_MOVETYPE)
				cl.v.movetype = MSG_ReadByte();
			if (sc1 & SC1_CAMERAMODE)
				cl.v.cameramode = MSG_ReadByte();
			if (sc1 & SC1_HASTED)
				cl.v.hasted = MSG_ReadFloat();
			if (sc1 & SC1_INVENTORY)
				cl.v.inventory = MSG_ReadByte();
			if (sc1 & SC1_RINGS_ACTIVE)
				cl.v.rings_active = MSG_ReadFloat();

			if (sc2 & SC2_RINGS_LOW)
				cl.v.rings_low = MSG_ReadFloat();
			if (sc2 & SC2_AMULET)
				cl.v.armor_amulet = MSG_ReadByte();
			if (sc2 & SC2_BRACER)
				cl.v.armor_bracer = MSG_ReadByte();
			if (sc2 & SC2_BREASTPLATE)
				cl.v.armor_breastplate = MSG_ReadByte();
			if (sc2 & SC2_HELMET)
				cl.v.armor_helmet = MSG_ReadByte();
			if (sc2 & SC2_FLIGHT_T)
				cl.v.ring_flight = MSG_ReadByte();
			if (sc2 & SC2_WATER_T)
				cl.v.ring_water = MSG_ReadByte();
			if (sc2 & SC2_TURNING_T)
				cl.v.ring_turning = MSG_ReadByte();
			if (sc2 & SC2_REGEN_T)
				cl.v.ring_regeneration = MSG_ReadByte();
			if (sc2 & SC2_HASTE_T)
				cl.v.haste_time = MSG_ReadFloat();
			if (sc2 & SC2_TOME_T)
				cl.v.tome_time = MSG_ReadFloat();
			if (sc2 & SC2_PUZZLE1)
				q_snprintf(cl.puzzle_pieces[0], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE2)
				q_snprintf(cl.puzzle_pieces[1], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE3)
				q_snprintf(cl.puzzle_pieces[2], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE4)
				q_snprintf(cl.puzzle_pieces[3], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE5)
				q_snprintf(cl.puzzle_pieces[4], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE6)
				q_snprintf(cl.puzzle_pieces[5], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE7)
				q_snprintf(cl.puzzle_pieces[6], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_PUZZLE8)
				q_snprintf(cl.puzzle_pieces[7], sizeof(cl.puzzle_pieces[0]), "%.9s", MSG_ReadString());
			if (sc2 & SC2_MAXHEALTH)
				cl.v.max_health = MSG_ReadShort();
			if (sc2 & SC2_MAXMANA)
				cl.v.max_mana = MSG_ReadByte();
			if (sc2 & SC2_FLAGS)
				cl.v.flags = MSG_ReadFloat();

			/* SC2_OBJ, SC2_OBJ2: mission pack objectives
			 * With protocol 18 (PROTOCOL_RAVEN_111), these
			 * bits get set somehow (?!): let's avoid them. */
			if (cl_protocol > PROTOCOL_RAVEN_111)
			{
				if (sc2 & SC2_OBJ)
					cl.info_mask = MSG_ReadLong();
				if (sc2 & SC2_OBJ2)
					cl.info_mask2 = MSG_ReadLong();
			}

			if ((sc1 & SC1_STAT_BAR) || (sc2 & SC2_STAT_BAR))
				Sbar_Changed();

			if ((sc1 & SC1_INV) || (sc2 & SC2_INV))
				SB_InvChanged();
			break;

		case svc_mod_name:
		case svc_skybox:
			MSG_ReadString();
			Con_DPrintf ("Ignored server msg %d (%s)\n", cmd, svc_strings[cmd]);
			break;
		}
	}
}

