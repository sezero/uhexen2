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
	
	"svc_serverdata",	// [long] version ...
	"svc_lightstyle",	// [byte] [string]
	"svc_updatename",	// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",	// <shortbits + data>
	"svc_stopsound",	// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",		// [byte] impact [byte] blood [vec3] from

	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",

	"svc_temp_entity",	// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",

	"svc_cdtrack",
	"svc_sellscreen",

	"svc_smallkick",
	"svc_bigkick",

	"svc_updateping",
	"svc_updateentertime",

	"svc_updatestatlong",
	"svc_muzzleflash",
	"svc_updateuserinfo",
	"svc_download",
	"svc_playerinfo",
	"svc_nails",
	"svc_choke",
	"svc_modellist",
	"svc_soundlist",
	"svc_packetentities",
 	"svc_deltapacketentities",
	"svc_maxspeed",
	"svc_entgravity",

	"svc_plaque",
	"svc_particle_explosion",
	"svc_set_view_tint",
	"svc_start_effect",
	"svc_end_effect",
	"svc_set_view_flags",
	"svc_clear_view_flags",
	"svc_update_inv",
	"svc_particle2",
	"svc_particle3",
	"svc_particle4",
	"svc_turn_effect",
	"svc_update_effect",
	"svc_multieffect",
	"svc_midi_name",
	"svc_raineffect",
	"svc_packmissile",

	"svc_indexed_print",
	"svc_targetupdate",
	"svc_name_print",
	"svc_sound_update_pos",
	"svc_update_piv",
	"svc_player_sound",
	"svc_updatepclass",
	"svc_updatedminfo",	// [byte] [short] [byte]
	"svc_updatesiegeinfo",	// [byte] [byte]
	"svc_updatesiegeteam",	// [byte] [byte]
	"svc_updatesiegelosses",// [byte] [byte]
	"svc_haskey",		// [byte] [byte]
	"svc_nonehaskey",	// [byte]
	"svc_isdoc",		// [byte] [byte]
	"svc_nodoc",		// [byte]
	"svc_playerskipped"	// [byte]
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL"
};
#define	NUM_SVC_STRINGS	(sizeof(svc_strings) / sizeof(svc_strings[0]))

int		parsecountmod;
double		parsecounttime;

qmodel_t	*player_models[MAX_PLAYER_CLASS];

int		cl_spikeindex, cl_playerindex[MAX_PLAYER_CLASS], cl_flagindex;
int		cl_ballindex, cl_missilestarindex, cl_ravenindex, cl_raven2index;


//=============================================================================


/*
===============
CL_CheckOrDownloadFile

Returns true if the file exists, otherwise it attempts
to start a download from the server.
===============
*/
qboolean CL_CheckOrDownloadFile (const char *filename)
{
	if (strstr (filename, ".."))
	{
		Con_Printf ("Refusing to download a path with ..\n");
		return true;
	}

	if (FS_FileExists(filename, NULL))
	{	// it exists, no need to download
		return true;
	}

	//ZOID - can't download when recording
	if (cls.demorecording)
	{
		Con_Printf("Unable to download %s in record mode.\n", cls.downloadname);
		return true;
	}
	//ZOID - can't download when playback
	if (cls.demoplayback)
		return true;

	q_strlcpy (cls.downloadname, filename, MAX_OSPATH);
	Con_Printf ("Downloading %s...\n", cls.downloadname);

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension (cls.downloadname, cls.downloadtempname, sizeof(cls.downloadtempname));
	q_strlcat (cls.downloadtempname, ".tmp", sizeof(cls.downloadtempname));

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, va("download %s", cls.downloadname));

	cls.downloadnumber++;

	return false;
}

/*
=================
Model_NextDownload
=================
*/
static void Model_NextDownload (void)
{
	const char	*s;
	int	i;

	if (cls.downloadnumber == 0)
	{
		Con_Printf ("Checking models...\n");
		cls.downloadnumber = 1;
	}

	cls.downloadtype = dl_model;
	for ( ; cl.model_name[cls.downloadnumber][0] ; cls.downloadnumber++)
	{
		s = cl.model_name[cls.downloadnumber];
		if (s[0] == '*')
			continue;	// inline brush model
		if (!CL_CheckOrDownloadFile(s))
			return;		// started a download
	}

	for (i = 1; i < MAX_MODELS; i++)
	{
		if (!cl.model_name[i][0])
			break;
		cl.model_precache[i] = Mod_ForName (cl.model_name[i], false);
		if (!cl.model_precache[i])
		{
			Con_Printf ("\nThe required model file '%s' could not be found or downloaded.\n\n", cl.model_name[i]);
			Con_Printf ("You may need to download or purchase a %s client "
					"pack in order to play on this server.\n\n", fs_gamedir_nopath);
			CL_Disconnect ();
			return;
		}
	}

	// copy the naked name of the map file to the cl structure
	COM_FileBase (cl.model_name[1], cl.mapname, sizeof(cl.mapname));

	// all done
	cl.worldmodel = cl.model_precache[1];
	R_NewMap ();

	Host_LoadStrings();

	Hunk_Check ();		// make sure nothing is hurt

	// done with modellist, request first of static signon messages
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, va("prespawn %i", cl.servercount));
}

/*
=================
Sound_NextDownload
=================
*/
static void Sound_NextDownload (void)
{
	const char	*s;
	int	i;

	if (cls.downloadnumber == 0)
	{
		Con_Printf ("Checking sounds...\n");
		cls.downloadnumber = 1;
	}

	cls.downloadtype = dl_sound;
	for ( ; cl.sound_name[cls.downloadnumber][0] ; cls.downloadnumber++)
	{
		s = cl.sound_name[cls.downloadnumber];
		if (!CL_CheckOrDownloadFile(va("sound/%s",s)))
			return;		// started a download
	}

	for (i = 1; i < MAX_SOUNDS; i++)
	{
		if (!cl.sound_name[i][0])
			break;
		cl.sound_precache[i] = S_PrecacheSound (cl.sound_name[i]);
	}

	// done with sounds, request models now
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, va("modellist %i", cl.servercount));
}


/*
======================
CL_RequestNextDownload
======================
*/
static void CL_RequestNextDownload (void)
{
	switch (cls.downloadtype)
	{
	case dl_single:
		break;
	case dl_skin:
		Skin_NextDownload ();
		break;
	case dl_model:
		Model_NextDownload ();
		break;
	case dl_sound:
		Sound_NextDownload ();
		break;
	case dl_none:
	default:
		Con_DPrintf("Unknown download type.\n");
	}
}

/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
static void CL_ParseDownload (void)
{
	int	size, percent;
	char	name[MAX_OSPATH];

	// read the data
	size = MSG_ReadShort ();
	percent = MSG_ReadByte ();

	if (cls.demoplayback)
	{
		if (size > 0)
			msg_readcount += size;
		return; // not in demo playback
	}

	if (size == -1)
	{
		Con_Printf ("File not found.\n");
		if (cls.download)
		{
			Con_Printf ("cls.download shouldn't have been set\n");
			fclose (cls.download);
			cls.download = NULL;
		}
		CL_RequestNextDownload ();
		return;
	}

	// open the file if not opened yet
	if (!cls.download)
	{
		FS_MakePath_BUF (FS_USERDIR, NULL, name, sizeof(name), cls.downloadtempname);
		if ( FS_CreatePath(name) )
		{
			msg_readcount += size;
			Con_Printf ("Unable to create directory for downloading %s\n", cls.downloadtempname);
			CL_RequestNextDownload ();
			return;
		}

		cls.download = fopen (name, "wb");
		if (!cls.download)
		{
			msg_readcount += size;
			Con_Printf ("Failed to open %s\n", cls.downloadtempname);
			CL_RequestNextDownload ();
			return;
		}
	}

	fwrite (net_message.data + msg_readcount, 1, size, cls.download);
	msg_readcount += size;

	if (percent != 100)
	{
		// request next block
		cls.downloadpercent = percent;

		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, "nextdl");
	}
	else
	{
		// rename the temp file to it's final name
		char	oldn[MAX_OSPATH];
		char	newn[MAX_OSPATH];

		fclose (cls.download);
		cls.download = NULL;
		cls.downloadpercent = 0;

		FS_MakePath_BUF (FS_USERDIR, NULL, oldn, sizeof(oldn), cls.downloadtempname);
		FS_MakePath_BUF (FS_USERDIR, NULL, newn, sizeof(newn), cls.downloadname);
		if (Sys_rename(oldn, newn) != 0)
			Con_Printf ("failed to rename.\n");

		// get another file if needed
		CL_RequestNextDownload ();
	}
}


/*
=====================================================================

SERVER CONNECTING MESSAGES

=====================================================================
*/

/*
==================
CL_ParseServerData
==================
*/
static void CL_ParseServerData (void)
{
	const char	*str;

	Con_DPrintf ("Serverdata packet received.\n");

//
// wipe the client_state_t struct
//
	CL_ClearState ();

	// CL_ClearState() clears the cl structure already,
	// so no need zero'ing the sound/model list arrays.
	//memset (cl.sound_precache, 0, sizeof(cl.sound_precache));
	//memset (cl.model_precache, 0, sizeof(cl.model_precache));
	cl_playerindex[0] = -1;
	cl_playerindex[1] = -1;
	cl_playerindex[2] = -1;
	cl_playerindex[3] = -1;
	cl_playerindex[4] = -1;
	cl_playerindex[5] = -1;//mg-siege
	cl_spikeindex = -1;
	cl_flagindex = -1;
	cl_ballindex = -1;
	cl_missilestarindex = -1;
	cl_ravenindex = -1;
	cl_raven2index = -1;

// parse protocol version number
	cl.protocol = MSG_ReadLong ();
	switch (cl.protocol) {
	case OLD_PROTOCOL_VERSION:
	case PROTOCOL_VERSION:
	case PROTOCOL_VERSION_EXT:
		Con_Printf ("Server using protocol %i\n", cl.protocol);
		break;
	default:
		Host_EndGame ("Server returned unsupported protocol %i", cl.protocol);
	}

	cl.servercount = MSG_ReadLong ();

	// game directory
	str = MSG_ReadString ();

	if (q_strcasecmp(fs_gamedir_nopath, str))
	{
		Con_Printf("Server set the gamedir to %s\n", str);

		// save current config
		Host_WriteConfiguration ("config.cfg");

		// set the new gamedir and userdir
		FS_Gamedir(str);

		// ZOID - run autoexec.cfg in the gamedir if it exists
		if (FS_FileInGamedir("config.cfg"))
		{
		// remove any weird mod specific key bindings / aliases
			Cbuf_AddText("unbindall\n");
			Cbuf_AddText("unaliasall\n");
			Cbuf_AddText("exec autoexec.cfg\n");
			Cbuf_AddText("exec config.cfg\n");
		}
		// gamespy crap
		if (FS_FileInGamedir("frontend.cfg"))
			Cbuf_AddText("exec frontend.cfg\n");

		Cbuf_Execute ();

		// re-init draw
		Draw_ReInit ();
	}

	// parse player slot, high bit means spectator
	cl.playernum = MSG_ReadByte ();
	if (cl.playernum & 128)
	{
		cl.spectator = true;
		cl.playernum &= ~128;
	}

	// get the full level name
	str = MSG_ReadString ();
	q_strlcpy (cl.levelname, str, sizeof(cl.levelname));

	// get the movevars
	if (cl.protocol >= PROTOCOL_VERSION)
	{
		movevars.gravity		= MSG_ReadFloat();
		movevars.stopspeed		= MSG_ReadFloat();
		movevars.maxspeed		= MSG_ReadFloat();
		movevars.spectatormaxspeed	= MSG_ReadFloat();
		movevars.accelerate		= MSG_ReadFloat();
		movevars.airaccelerate		= MSG_ReadFloat();
		movevars.wateraccelerate	= MSG_ReadFloat();
		movevars.friction		= MSG_ReadFloat();
		movevars.waterfriction		= MSG_ReadFloat();
		movevars.entgravity		= MSG_ReadFloat();
	}
	else
	{
		movevars.gravity		= 800;
		movevars.stopspeed		= 100;
		movevars.maxspeed		= 320;
		movevars.spectatormaxspeed	= 500;
		movevars.accelerate		= 10;
		movevars.airaccelerate		= 0.7;
		movevars.wateraccelerate	= 10;
		movevars.friction		= 6;
		movevars.waterfriction		= 1;
		movevars.entgravity		= 1.0;
	}

	// seperate the printfs so the server message can have a color
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_Printf ("%c%s\n", 2, str);

	// ask for the sound list next
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, va("soundlist %i", cl.servercount));

	// now waiting for downloads, etc
	cls.state = ca_onserver;
	cl_keyholder = -1;
	cl_doc = -1;
}

static void CL_ParseSoundlistChunks (void) /* from QW */
{
	int	numsounds, n;
	const char	*str;

	// precache sounds
	numsounds = MSG_ReadLong ();

	for (;;) {
		str = MSG_ReadString ();
		if (!str[0])
			break;
		numsounds++;
		if (numsounds >= MAX_SOUNDS)
			Host_Error ("Server sent too many sound_precache");
		strcpy (cl.sound_name[numsounds], str);
	}

	n = MSG_ReadLong ();
	if (n)
	{
		if (!cls.demoplayback) {
			MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
			MSG_WriteString (&cls.netchan.message,
						 va ("soundlist %i %i", cl.servercount, n));
		}
		return;
	}

	cls.downloadnumber = 0;
	cls.downloadtype = dl_sound;
	Sound_NextDownload ();
}

/*
==================
CL_ParseSoundlist
==================
*/
static void CL_ParseSoundlist (void)
{
	int	numsounds;
	const char	*str;

// precache sounds
	for (numsounds = 1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (numsounds == MAX_SOUNDS)
			Host_EndGame ("Server sent too many sound_precache");
		q_strlcpy (cl.sound_name[numsounds], str, MAX_QPATH);
	}

	cls.downloadnumber = 0;
	cls.downloadtype = dl_sound;
	Sound_NextDownload ();
}

static void CL_ParseModellistChunks (void) /* from QW */
{
	int	nummodels, n;
	const char	*str;

	// precache models and note certain default indexes
	nummodels = MSG_ReadLong ();

	for (;;)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;

		nummodels++;
		if (nummodels >= MAX_MODELS)
			Host_EndGame ("Server sent too many model_precache");

		q_strlcpy (cl.model_name[nummodels], str, MAX_QPATH);

		if (!strcmp(cl.model_name[nummodels],"progs/spike.mdl"))
			cl_spikeindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/paladin.mdl"))
			cl_playerindex[0] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/crusader.mdl"))
			cl_playerindex[1] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/necro.mdl"))
			cl_playerindex[2] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/assassin.mdl"))
			cl_playerindex[3] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/succubus.mdl"))
			cl_playerindex[4] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/hank.mdl"))
			cl_playerindex[5] = nummodels;//mg-siege
		if (!strcmp(cl.model_name[nummodels],"progs/flag.mdl"))
			cl_flagindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/ball.mdl"))
			cl_ballindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/newmmis.mdl"))
			cl_missilestarindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/ravproj.mdl"))
			cl_ravenindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/vindsht1.mdl"))
			cl_raven2index = nummodels;
	}

	player_models[0] = (qmodel_t *)Mod_FindName ("models/paladin.mdl");
	player_models[1] = !(gameflags & GAME_OLD_DEMO) ? (qmodel_t *)Mod_FindName ("models/crusader.mdl") : NULL;
	player_models[2] = !(gameflags & GAME_OLD_DEMO) ? (qmodel_t *)Mod_FindName ("models/necro.mdl") : NULL;
	player_models[3] = (qmodel_t *)Mod_FindName ("models/assassin.mdl");
	player_models[4] = (qmodel_t *)Mod_FindName ("models/succubus.mdl");
	player_models[5] = (qmodel_t *)Mod_FindName ("models/hank.mdl");//siege

	n = MSG_ReadLong ();
	if (n)
	{
		if (!cls.demoplayback) {
			MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
			MSG_WriteString (&cls.netchan.message,
							 va ("modellist %i %i", cl.servercount, n));
		}
		return;
	}

	cls.downloadnumber = 0;
	cls.downloadtype = dl_model;
	Model_NextDownload ();
}

/*
==================
CL_ParseModellist
==================
*/
static void CL_ParseModellist (void)
{
	int	nummodels;
	const char	*str;

// precache models and note certain default indexes
	for (nummodels = 1 ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (nummodels == MAX_MODELS)
			Host_EndGame ("Server sent too many model_precache");
		q_strlcpy (cl.model_name[nummodels], str, MAX_QPATH);

		if (!strcmp(cl.model_name[nummodels],"progs/spike.mdl"))
			cl_spikeindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/paladin.mdl"))
			cl_playerindex[0] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/crusader.mdl"))
			cl_playerindex[1] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/necro.mdl"))
			cl_playerindex[2] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/assassin.mdl"))
			cl_playerindex[3] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/succubus.mdl"))
			cl_playerindex[4] = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/hank.mdl"))
			cl_playerindex[5] = nummodels;//mg-siege
		if (!strcmp(cl.model_name[nummodels],"progs/flag.mdl"))
			cl_flagindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/ball.mdl"))
			cl_ballindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/newmmis.mdl"))
			cl_missilestarindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/ravproj.mdl"))
			cl_ravenindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"models/vindsht1.mdl"))
			cl_raven2index = nummodels;
	}

	player_models[0] = (qmodel_t *)Mod_FindName ("models/paladin.mdl");
	player_models[1] = !(gameflags & GAME_OLD_DEMO) ? (qmodel_t *)Mod_FindName ("models/crusader.mdl") : NULL;
	player_models[2] = !(gameflags & GAME_OLD_DEMO) ? (qmodel_t *)Mod_FindName ("models/necro.mdl") : NULL;
	player_models[3] = (qmodel_t *)Mod_FindName ("models/assassin.mdl");
	player_models[4] = (qmodel_t *)Mod_FindName ("models/succubus.mdl");
	player_models[5] = (qmodel_t *)Mod_FindName ("models/hank.mdl");//siege

	cls.downloadnumber = 0;
	cls.downloadtype = dl_model;
	Model_NextDownload ();
}

/*
==================
CL_ParseBaseline
==================
*/
static void CL_ParseBaseline (entity_state_t *es)
{
	int	i;

	es->modelindex = MSG_ReadShort ();
	es->frame = MSG_ReadByte ();
	es->colormap = MSG_ReadByte();
	es->skinnum = MSG_ReadByte();
	es->scale = MSG_ReadByte();
	es->drawflags = MSG_ReadByte();
	es->abslight = MSG_ReadByte();

	for (i = 0; i < 3; i++)
	{
		es->origin[i] = MSG_ReadCoord ();
		es->angles[i] = MSG_ReadAngle ();
	}
}


/*
=====================
CL_ParseStatic

Static entities are non-interactive world objects
like torches
=====================
*/
static void CL_ParseStatic (void)
{
	entity_t *ent;
	int	i;
	entity_state_t	es;

	CL_ParseBaseline (&es);

	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_EndGame ("Too many static entities");
	ent = &cl_static_entities[i];
	cl.num_statics++;

// copy it to the current state
	ent->model = cl.model_precache[es.modelindex];
	ent->frame = es.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = es.skinnum;
	ent->scale = es.scale;
	ent->drawflags = es.drawflags;
	ent->abslight = es.abslight;

	VectorCopy (es.origin, ent->origin);
	VectorCopy (es.angles, ent->angles);

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
	sound_num = MSG_ReadByte ();
	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();

	S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}


/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
==================
CL_ParseStartSoundPacket
==================
*/
static void CL_ParseStartSoundPacket(void)
{
	vec3_t	pos;
	int	channel, ent;
	int	sound_num, volume;
	float	attenuation;
	int	i;

	channel = MSG_ReadShort();

	if (channel & SND_VOLUME)
		volume = MSG_ReadByte ();
	else	volume = DEFAULT_SOUND_PACKET_VOLUME;

	if (channel & SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 32.0;
	else	attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

	sound_num = MSG_ReadByte ();

	for (i = 0; i < 3; i++)
		pos[i] = MSG_ReadCoord ();

	ent = (channel>>3)&1023;
	channel &= 7;

	if (ent > MAX_EDICTS)
		Host_EndGame ("%s: ent = %i", __thisfunc__, ent);

	S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);
}


/*
==================
CL_ParseClientdata

Server information pertaining to this client only, sent every frame
==================
*/
static void CL_ParseClientdata (void)
{
	int		i;
	float		latency;
	frame_t		*frame;

// calculate simulated time of message

	i = cls.netchan.incoming_acknowledged;
	cl.parsecount = i;
	i &= UPDATE_MASK;
	parsecountmod = i;
	frame = &cl.frames[i];
	parsecounttime = cl.frames[i].senttime;

	frame->receivedtime = realtime;

// calculate latency
	latency = frame->receivedtime - frame->senttime;

	if (latency < 0 || latency > 1.0)
	{
	//	Con_Printf ("Odd latency: %5.2f\n", latency);
	}
	else
	{
	// drift the average latency towards the observed latency
		if (latency < cls.latency)
			cls.latency = latency;
		else	cls.latency += 0.001;	// drift up, so correction are needed
	}
}

/*
=====================
CL_NewTranslation
=====================
*/
static void CL_NewTranslation (int slot)
{
#ifdef GLQUAKE
	if (slot >= MAX_CLIENTS)
		Sys_Error ("%s: slot > MAX_CLIENTS", __thisfunc__);

	R_TranslatePlayerSkin(slot);
#else

	int	i, j;
	int	top, bottom;
	byte	*dest, *source, *sourceA, *sourceB, *colorA, *colorB;
	player_info_t	*player;

	if (slot >= MAX_CLIENTS)
		Sys_Error ("%s: slot > MAX_CLIENTS", __thisfunc__);

	player = &cl.players[slot];
	if (!player->playerclass)
		return;

	dest = player->translations;
	source = vid.colormap;
	memcpy (dest, vid.colormap, sizeof(player->translations));

	top = player->topcolor;
	if (top > 10 || top < 0)
		top = 10;
	bottom = player->bottomcolor;
	if (bottom > 10 || bottom < 0)
		bottom = 10;
	top -= 1;
	bottom -= 1;

	for (i = 0; i < VID_GRADES; i++, dest += 256, source += 256)
	{
		colorA = playerTranslation + 256 + color_offsets[(int)player->playerclass-1];
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
==============
CL_UpdateUserinfo
==============
*/
static void CL_UpdateUserinfo (void)
{
	int		slot;
	player_info_t	*player;

	slot = MSG_ReadByte ();
	if (slot >= MAX_CLIENTS)
		Host_EndGame ("%s: svc_updateuserinfo > MAX_CLIENTS", __thisfunc__);

	player = &cl.players[slot];
	player->userid = MSG_ReadLong ();
	q_strlcpy (player->userinfo, MSG_ReadString(), sizeof(player->userinfo));

	q_strlcpy (player->name, Info_ValueForKey (player->userinfo, "name"), sizeof(player->name));
	player->topcolor = atoi(Info_ValueForKey (player->userinfo, "topcolor"));
	player->bottomcolor = atoi(Info_ValueForKey (player->userinfo, "bottomcolor"));
	if (Info_ValueForKey (player->userinfo, "*spectator")[0])
		player->spectator = true;
	else	player->spectator = false;

	if (cls.state == ca_active)
		Skin_Find (player);

	player->playerclass = atoi(Info_ValueForKey (player->userinfo, "playerclass"));
	/*
	if (cl.playernum == slot && player->playerclass != playerclass.integer)
		Cvar_SetValue ("playerclass",player->playerclass);
	*/
	Sbar_Changed ();
	player->Translated = false;
	CL_NewTranslation (slot);
}


/*
=====================
CL_SetStat
=====================
*/
static void CL_SetStat (int idx, int value)
{
	int	j;

	if (idx < 0 || idx >= MAX_CL_STATS)
		Sys_Error ("%s: %i is invalid", __thisfunc__, idx);

	Sbar_Changed ();

	if (idx == STAT_ITEMS)
	{	// set flash times
		Sbar_Changed ();
		for (j = 0; j < 32; j++)
			if ((value & (1<<j)) && !(cl.stats[idx] & (1<<j)))
				cl.item_gettime[j] = cl.time;
	}

	cl.stats[idx] = value;
}

/*
==============
CL_MuzzleFlash
==============
*/
static void CL_MuzzleFlash (void)
{
	vec3_t	fv, rv, uv;
	dlight_t	*dl;
	int		i;
	player_state_t	*pl;

	i = MSG_ReadShort ();

	if ((unsigned int)(i - 1) >= MAX_CLIENTS)
		return;
#ifdef GLQUAKE
	// don't draw our own muzzle flash in gl if flashblending
	if (i - 1 == cl.playernum && gl_flashblend.integer)
		return;
#endif
	pl = &cl.frames[parsecountmod].playerstate[i-1];

	dl = CL_AllocDlight (i);
	VectorCopy (pl->origin,  dl->origin);
	AngleVectors (pl->viewangles, fv, rv, uv);

	VectorMA (dl->origin, 18, fv, dl->origin);
	dl->radius = 200 + (rand() & 31);
	dl->minlight = 32;
	dl->die = cl.time + 0.1;
	dl->color[0] = 0.2;
	dl->color[1] = 0.1;
	dl->color[2] = 0.05;
	dl->color[3] = 0.7;
}

static void CL_Plaque(void)
{
	int	idx;

	idx = MSG_ReadShort ();

	if (idx > 0 && idx <= host_string_count)
		SCR_SetPlaqueMessage (Host_GetString(idx - 1));
	else	SCR_SetPlaqueMessage ("");
}

static void CL_IndexedPrint(void)
{
	int	idx, i;

	i = MSG_ReadByte ();
	if (i == PRINT_CHAT)
	{
		S_LocalSound ("misc/talk.wav");
		con_ormask = 256;
	}

	idx = MSG_ReadShort ();
	if (idx > 0 && idx <= host_string_count)
		Con_Printf ("%s", Host_GetString(idx - 1));

	con_ormask = 0;
}

static void CL_NamePrint(void)
{
	int idx, i;

	i = MSG_ReadByte ();
	if (i == PRINT_CHAT)
	{
		S_LocalSound ("misc/talk.wav");
		con_ormask = 256;
	}

	idx = MSG_ReadByte ();
	if (idx >= 0 && idx < MAX_CLIENTS)
		Con_Printf ("%s", cl.players[idx].name);

	con_ormask = 0;
}

static void CL_ParticleExplosion(void)
{
	vec3_t org;
	short color, radius, counter;

	org[0] = MSG_ReadCoord();
	org[1] = MSG_ReadCoord();
	org[2] = MSG_ReadCoord();
	color = MSG_ReadShort();
	radius = MSG_ReadShort();
	counter = MSG_ReadShort();

	R_ColoredParticleExplosion(org,color,radius,counter);
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
//static int	received_framecount;
int LastServerMessageSize = 0;

void CL_ParseServerMessage (void)
{
	int		cmd;
	const char	*s;
	int		i, j;
	unsigned int	sc1, sc2;
	byte		test;
	char		temp[100];
	vec3_t		pos;

	LastServerMessageSize += net_message.cursize;

//	received_framecount = host_framecount;
	cl.last_servermessage = realtime;
	CL_ClearProjectiles ();
	CL_ClearMissiles ();
	v_targDist = 0;	// This clears out the target field on each netupdate;
			// it won't be drawn unless another update comes...

// if recording demos, copy the message out
	if (cl_shownet.integer == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.integer == 2)
		Con_Printf ("------------------\n");

	CL_ParseClientdata ();

// parse the message
	while (1)
	{
		if (msg_badread)
		{
			Host_EndGame ("%s: Bad server message", __thisfunc__);
			break;
		}

		cmd = MSG_ReadByte ();
		if (cmd == -1)
		{
			msg_readcount++;	// so the EOM showner has the right value
			SHOWNET("END OF MESSAGE");
			break;
		}
		if (cmd < (int)NUM_SVC_STRINGS) {
			SHOWNET(svc_strings[cmd]);
		}

	// other commands
		switch (cmd)
		{
		default:
		//	CL_DumpPacket ();
			Host_EndGame ("%s: Illegible server message %d", __thisfunc__, cmd);
			break;

		case svc_nop:
		//	Con_Printf ("svc_nop\n");
			break;

		case svc_disconnect:
			Host_EndGame ("Server disconnected\n");
			break;

		case svc_print:
			i = MSG_ReadByte ();
			if (i == PRINT_CHAT)
			{
				S_LocalSound ("misc/talk.wav");
				con_ormask = 256;
			}
			else if (i >= PRINT_SOUND)
			{
				if (talksounds.integer)
				{
					q_snprintf (temp, sizeof(temp), "taunt/taunt%.3d.wav", i - PRINT_SOUND + 1);
					S_LocalSound (temp);
					con_ormask = 256;
				}
				else
				{
					MSG_ReadString();
					break;
				}
			}
			Con_Printf ("%s", MSG_ReadString ());
			con_ormask = 0;
			break;

		case svc_centerprint:
			SCR_CenterPrint (MSG_ReadString ());
			break;

		case svc_stufftext:
			s = MSG_ReadString ();
			Con_DPrintf ("stufftext: %s\n", s);
			Cbuf_AddText (s);
			break;

		case svc_damage:
			V_ParseDamage ();
			break;

		case svc_serverdata:
			Cbuf_Execute ();		// make sure any stuffed commands are done
			CL_ParseServerData ();
			vid.recalc_refdef = true;	// leave full screen intermission
			break;

		case svc_setangle:
			for (i = 0; i < 3; i++)
				cl.viewangles[i] = MSG_ReadAngle ();
		//	cl.viewangles[PITCH] = cl.viewangles[ROLL] = 0;
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

		case svc_updatefrags:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("%s: svc_updatefrags > MAX_CLIENTS", __thisfunc__);
			cl.players[i].frags = MSG_ReadShort ();
			break;

		case svc_updateping:
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("%s: svc_updateping > MAX_CLIENTS", __thisfunc__);
			cl.players[i].ping = MSG_ReadShort ();
			break;

		case svc_updateentertime:
		// time is sent over as seconds ago
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("%s: svc_updateentertime > MAX_CLIENTS", __thisfunc__);
			cl.players[i].entertime = realtime - MSG_ReadFloat ();
			break;

		case svc_updatepclass:
		// playerclass has changed for this dude
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("%s: svc_updatepclass > MAX_CLIENTS", __thisfunc__);
			cl.players[i].playerclass = MSG_ReadByte ();
			cl.players[i].level = cl.players[i].playerclass&31;
			cl.players[i].playerclass = cl.players[i].playerclass>>5;
			break;

		case svc_updatedminfo:
		// This dude killed someone, update his frags and level
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("%s: svc_updatedminfo > MAX_CLIENTS", __thisfunc__);
			cl.players[i].frags = MSG_ReadShort ();
			cl.players[i].playerclass = MSG_ReadByte ();
			cl.players[i].level = cl.players[i].playerclass&31;
			cl.players[i].playerclass = cl.players[i].playerclass>>5;
			break;

		case svc_updatesiegelosses:
		// This dude killed someone, update his frags and level
			defLosses = MSG_ReadByte ();
			attLosses = MSG_ReadByte ();
			break;

		case svc_updatesiegeteam:
		// This dude killed someone, update his frags and level
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("%s: svc_updatesiegeteam > MAX_CLIENTS", __thisfunc__);
			cl.players[i].siege_team = MSG_ReadByte ();
			break;

		case svc_updatesiegeinfo:
		// We are on a siege server, set cl_siege
			cl_siege = true;
			cl_timelimit = MSG_ReadByte () * 60;
			cl_fraglimit = MSG_ReadByte ();
			break;

		case svc_haskey:
			cl_keyholder = MSG_ReadShort() - 1;
			break;

		case svc_isdoc:
			cl_doc = MSG_ReadShort() - 1;
			break;

		case svc_nonehaskey:
			cl_keyholder = -1;
			break;

		case svc_nodoc:
			cl_doc = -1;
			break;

		case svc_time:
			cl_server_time_offset = ((int)MSG_ReadFloat()) - cl.time;
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			CL_ParseBaseline (&cl_baselines[i]);
			break;
		case svc_spawnstatic:
			CL_ParseStatic ();
			break;
		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_killedmonster:
			cl.stats[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			cl.stats[STAT_SECRETS]++;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();
			j = MSG_ReadByte ();
			CL_SetStat (i, j);
			break;
		case svc_updatestatlong:
			i = MSG_ReadByte ();
			j = MSG_ReadLong ();
			CL_SetStat (i, j);
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound ();
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			if (q_strcasecmp(bgmtype.string,"cd") != 0)
				CDAudio_Stop ();
			else	CDAudio_Play ((byte)cl.cdtrack, true);
			break;

		case svc_midi_name:
			q_strlcpy (cl.midi_name, MSG_ReadString(), sizeof(cl.midi_name));
			if (q_strcasecmp(bgmtype.string,"midi") != 0)
				BGM_Stop();
			else	BGM_PlayMIDIorMusic(cl.midi_name);
			break;

		case svc_intermission:
			/*
			if (cl_siege) {//MG
			*/
				CL_SetupIntermission (MSG_ReadByte());
				vid.recalc_refdef = true;	// go to full screen
				break;
			/*
			} else { // Old Quake way- won't work
				cl.intermission = 1;
				cl.completed_time = realtime;
				vid.recalc_refdef = true;	// go to full screen
				for (i = 0; i < 3; i++)
					cl.simorg[i] = MSG_ReadCoord ();
				for (i = 0; i < 3; i++)
					cl.simangles[i] = MSG_ReadAngle ();
				VectorClear (cl.simvel);
				break;
			}
			*/
		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = realtime;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());
			break;

		case svc_sellscreen:
			Cmd_ExecuteString ("help", src_command);
			break;

		case svc_smallkick:
			cl.punchangle = -2;
			break;
		case svc_bigkick:
			cl.punchangle = -4;
			break;

		case svc_muzzleflash:
			CL_MuzzleFlash ();
			break;

		case svc_updateuserinfo:
			CL_UpdateUserinfo ();
			break;

		case svc_download:
			CL_ParseDownload ();
			break;

		case svc_playerinfo:
			CL_ParsePlayerinfo ();
			break;

		case svc_playerskipped:
			CL_SavePlayer ();
			break;

		case svc_nails:
			CL_ParseProjectiles ();
			break;

		case svc_packmissile:
			CL_ParsePackMissiles ();
			break;

		case svc_chokecount:		// some preceding packets were choked
			i = MSG_ReadByte ();
			for (j = 0; j < i; j++)
				cl.frames[ (cls.netchan.incoming_acknowledged-1-j)&UPDATE_MASK ].receivedtime = -2;
			break;

		case svc_modellist:
			if (cl.protocol >= PROTOCOL_VERSION_EXT) /* from QW */
			{
				CL_ParseModellistChunks ();
				break;
			}
			CL_ParseModellist ();
			break;

		case svc_soundlist:
			if (cl.protocol >= PROTOCOL_VERSION_EXT) /* from QW */
			{
				CL_ParseSoundlistChunks ();
				break;
			}
			CL_ParseSoundlist ();
			break;

		case svc_packetentities:
			CL_ParsePacketEntities (false);
			break;

		case svc_deltapacketentities:
			CL_ParsePacketEntities (true);
			break;

		case svc_maxspeed :
			movevars.maxspeed = MSG_ReadFloat();
			break;

		case svc_entgravity :
			movevars.entgravity = MSG_ReadFloat();
			break;

		case svc_plaque:
			CL_Plaque();
			break;

		case svc_indexed_print:
			CL_IndexedPrint();
			break;

		case svc_name_print:
			CL_NamePrint();
			break;

		case svc_particle_explosion:
			CL_ParticleExplosion();
			break;

		case svc_set_view_tint:
			i = MSG_ReadByte();
		//	cl.viewent.colorshade = i;
			break;

		case svc_start_effect:
			CL_ParseEffect();
			break;

		case svc_end_effect:
			CL_EndEffect();
			break;

		case svc_set_view_flags:
			cl.viewent.drawflags |= MSG_ReadByte();
			break;

		case svc_clear_view_flags:
			cl.viewent.drawflags &= ~MSG_ReadByte();
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
		//	if (sc1 & SC1_WEAPON)
		//		cl.v.weapon = MSG_ReadByte();
			if (sc1 & SC1_TELEPORT_TIME)
			{
		//		Con_Printf("Teleport_time>time, got bit\n");
				cl.v.teleport_time = realtime + 2; // can't airmove for 2s
			}

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
				cl.v.artifact_active = MSG_ReadByte();
			if (sc1 & SC1_ARTIFACT_LOW)
				cl.v.artifact_low = MSG_ReadByte();
			if (sc1 & SC1_MOVETYPE)
				cl.v.movetype = MSG_ReadByte();
			if (sc1 & SC1_CAMERAMODE)
				cl.v.cameramode = MSG_ReadByte();
			if (sc1 & SC1_HASTED)
				cl.v.hasted = MSG_ReadFloat();
			if (sc1 & SC1_INVENTORY)
				cl.v.inventory = MSG_ReadByte();
			if (sc1 & SC1_RINGS_ACTIVE)
				cl.v.rings_active = MSG_ReadByte();

			if (sc2 & SC2_RINGS_LOW)
				cl.v.rings_low = MSG_ReadByte();
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
		//	if (sc2 & SC2_HASTE_T)
		//		cl.v.haste_time = MSG_ReadFloat();
		//	if (sc2 & SC2_TOME_T)
		//		cl.v.tome_time = MSG_ReadFloat();
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
		case svc_turn_effect:
			CL_TurnEffect ();
			break;
		case svc_update_effect:
			CL_ReviseEffect();
			break;

		case svc_multieffect:
			CL_ParseMultiEffect();
			break;

		case svc_raineffect:
			R_ParseRainEffect();
			break;

		case svc_targetupdate:
			V_ParseTarget();
			break;

		case svc_update_piv:
			cl.PIV = MSG_ReadLong();
			break;

		case svc_player_sound:
			test = MSG_ReadByte();
			pos[0] = MSG_ReadCoord();
			pos[1] = MSG_ReadCoord();
			pos[2] = MSG_ReadCoord();
			i = MSG_ReadShort ();
			S_StartSound(test, 1, cl.sound_precache[i], pos, 1.0, 1.0);
			break;
		}
	}

	CL_SetSolidEntities ();
}

