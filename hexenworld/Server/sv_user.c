/*
	sv_user.c
	server code for moving users

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Server/sv_user.c,v 1.26 2007-09-29 18:10:26 sezero Exp $
*/

#include "quakedef.h"
#include <ctype.h>

edict_t	*sv_player;

static	usercmd_t	cmd;

static	cvar_t	cl_rollspeed = {"cl_rollspeed", "200", CVAR_NONE};
static	cvar_t	cl_rollangle = {"cl_rollangle", "2.0", CVAR_NONE};
static	cvar_t	sv_spectalk = {"sv_spectalk", "1", CVAR_NONE};
static	cvar_t	sv_allowtaunts = {"sv_allowtaunts", "1", CVAR_NONE};

extern	cvar_t	allow_download;
extern	cvar_t	allow_download_skins;
extern	cvar_t	allow_download_models;
extern	cvar_t	allow_download_sounds;
extern	cvar_t	allow_download_maps;
extern	vec3_t	player_mins;

extern	int	fp_messages, fp_persecond, fp_secondsdead;
extern	char	fp_msg[];


/*
============================================================

USER STRINGCMD EXECUTION

host_client and sv_player will be valid.
============================================================
*/

/*
================
SV_New_f

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
static void SV_New_f (void)
{
	char		*gamedir;
	int			playernum;

	if (host_client->state == cs_spawned)
		return;

	host_client->state = cs_connected;
	host_client->connection_started = realtime;

	// send the info about the new client to all connected clients
//	SV_FullClientUpdate (host_client, &sv.reliable_datagram);
	host_client->sendinfo = true;

	gamedir = Info_ValueForKey (svs.info, "*gamedir");
	if (!gamedir[0])
		gamedir = "hw";

	// send the serverdata
	MSG_WriteByte (&host_client->netchan.message, svc_serverdata);
	MSG_WriteLong (&host_client->netchan.message, PROTOCOL_VERSION);
	MSG_WriteLong (&host_client->netchan.message, svs.spawncount);
	MSG_WriteString (&host_client->netchan.message, gamedir);

	playernum = NUM_FOR_EDICT(host_client->edict)-1;
	if (host_client->spectator)
		playernum |= 128;
	MSG_WriteByte (&host_client->netchan.message, playernum);

	// send full levelname
	if (sv.edicts->v.message > 0 && sv.edicts->v.message <= pr_string_count)
	{
		MSG_WriteString (&host_client->netchan.message,&pr_global_strings[pr_string_index[(int)sv.edicts->v.message-1]]);
	}
	else 
	{
	// Use netname on map if there is one, so they don't have to edit strings.txt
	//	MSG_WriteString(&host_client->netchan.message,"");
		MSG_WriteString(&host_client->netchan.message, PR_GetString(sv.edicts->v.netname));
	}

	// send the movevars
	MSG_WriteFloat(&host_client->netchan.message, movevars.gravity);
	MSG_WriteFloat(&host_client->netchan.message, movevars.stopspeed);
	MSG_WriteFloat(&host_client->netchan.message, movevars.maxspeed);
	MSG_WriteFloat(&host_client->netchan.message, movevars.spectatormaxspeed);
	MSG_WriteFloat(&host_client->netchan.message, movevars.accelerate);
	MSG_WriteFloat(&host_client->netchan.message, movevars.airaccelerate);
	MSG_WriteFloat(&host_client->netchan.message, movevars.wateraccelerate);
	MSG_WriteFloat(&host_client->netchan.message, movevars.friction);
	MSG_WriteFloat(&host_client->netchan.message, movevars.waterfriction);
	MSG_WriteFloat(&host_client->netchan.message, movevars.entgravity);

	// send music
	MSG_WriteByte (&host_client->netchan.message, svc_cdtrack);
//	MSG_WriteByte (&host_client->netchan.message, sv.edicts->v.soundtype);
	MSG_WriteByte (&host_client->netchan.message, sv.cd_track);

	MSG_WriteByte (&host_client->netchan.message, svc_midi_name);
	MSG_WriteString (&host_client->netchan.message, sv.midi_name);

	// send server info string
	MSG_WriteByte (&host_client->netchan.message, svc_stufftext);
	MSG_WriteString (&host_client->netchan.message, va("fullserverinfo \"%s\"\n", svs.info) );
}

/*
==================
SV_Soundlist_f
==================
*/
static void SV_Soundlist_f (void)
{
	int		i;

	if (host_client->state != cs_connected)
	{
		Con_Printf ("soundlist not valid -- already spawned\n");
		return;
	}

	// handle the case of a level changing while a client was connecting
	if ( atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Con_Printf ("%s from different level\n", __thisfunc__);
		SV_New_f ();
		return;
	}

	MSG_WriteByte (&host_client->netchan.message, svc_soundlist);
	for (i = 1; i < MAX_SOUNDS && sv.sound_precache[i][0]; i++)
		MSG_WriteString (&host_client->netchan.message, sv.sound_precache[i]);
	MSG_WriteByte (&host_client->netchan.message, 0);
}

/*
==================
SV_Modellist_f
==================
*/
static void SV_Modellist_f (void)
{
	int		i;

	if (host_client->state != cs_connected)
	{
		Con_Printf ("modellist not valid -- already spawned\n");
		return;
	}

	// handle the case of a level changing while a client was connecting
	if ( atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Con_Printf ("%s from different level\n", __thisfunc__);
		SV_New_f ();
		return;
	}

	MSG_WriteByte (&host_client->netchan.message, svc_modellist);
	for (i = 1; i < MAX_MODELS && sv.model_precache[i][0]; i++)
		MSG_WriteString (&host_client->netchan.message, sv.model_precache[i]);
	MSG_WriteByte (&host_client->netchan.message, 0);
}

/*
==================
SV_PreSpawn_f
==================
*/
static void SV_PreSpawn_f (void)
{
	int		buf;

	if (host_client->state != cs_connected)
	{
		Con_Printf ("prespawn not valid -- already spawned\n");
		return;
	}

	// handle the case of a level changing while a client was connecting
	if ( atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Con_Printf ("%s from different level\n", __thisfunc__);
		SV_New_f ();
		return;
	}

	buf = atoi(Cmd_Argv(2));
	if (buf >= sv.num_signon_buffers)
		buf = 0;

	SZ_Write (&host_client->netchan.message, sv.signon_buffers[buf], sv.signon_buffer_size[buf]);

	buf++;
	if (buf == sv.num_signon_buffers)
	{	// all done prespawning
		MSG_WriteByte (&host_client->netchan.message, svc_stufftext);
		MSG_WriteString (&host_client->netchan.message, va("cmd spawn %i\n",svs.spawncount) );
	}
	else
	{	// need to prespawn more
		MSG_WriteByte (&host_client->netchan.message, svc_stufftext);
		MSG_WriteString (&host_client->netchan.message, va("cmd prespawn %i %i\n", svs.spawncount, buf) );
	}
}

/*
==================
SV_Spawn_f
==================
*/
static void SV_Spawn_f (void)
{
	int		i;
	client_t	*client;
	edict_t	*ent;
	eval_t	*val;

//	Con_Printf("%s\n", __thisfunc__);

	if (host_client->state != cs_connected)
	{
		Con_Printf ("Spawn not valid -- already spawned\n");
		return;
	}

	// handle the case of a level changing while a client was connecting
	if ( atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Con_Printf ("%s from different level\n", __thisfunc__);
		SV_New_f ();
		return;
	}

	// set up the edict
	ent = host_client->edict;

	memset (&ent->v, 0, progs->entityfields * 4);
	ent->v.colormap = NUM_FOR_EDICT(ent);
	if (dmMode.integer == DM_SIEGE)
		ent->v.team = ent->v.siege_team;	// FIXME
	else
		ent->v.team = 0;	// FIXME

	ent->v.netname = PR_SetEngineString(host_client->name);
	//ent->v.playerclass = host_client->playerclass = 
	ent->v.next_playerclass = host_client->next_playerclass;
	ent->v.has_portals = host_client->portals;

	host_client->entgravity = 1.0;
	val = GetEdictFieldValue(ent, "gravity");
	if (val)
		val->_float = 1.0;
	host_client->maxspeed = sv_maxspeed.value;
	val = GetEdictFieldValue(ent, "maxspeed");
	if (val)
		val->_float = sv_maxspeed.value;

	// send all current names, colors, and frag counts
	// FIXME: is this a good thing?
	SZ_Clear (&host_client->netchan.message);

	// send current status of all other players
	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
		SV_FullClientUpdate (client, &host_client->netchan.message);

	// send all current light styles
	for (i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		MSG_WriteByte (&host_client->netchan.message, svc_lightstyle);
		MSG_WriteByte (&host_client->netchan.message, (char)i);
		MSG_WriteString (&host_client->netchan.message, sv.lightstyles[i]);
	}

//
// force stats to be updated
//
	memset (host_client->stats, 0, sizeof(host_client->stats));

	MSG_WriteByte (&host_client->netchan.message, svc_updatestatlong);
	MSG_WriteByte (&host_client->netchan.message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->netchan.message, PR_GLOBAL_STRUCT(total_secrets));

	MSG_WriteByte (&host_client->netchan.message, svc_updatestatlong);
	MSG_WriteByte (&host_client->netchan.message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->netchan.message, PR_GLOBAL_STRUCT(total_monsters));

	MSG_WriteByte (&host_client->netchan.message, svc_updatestatlong);
	MSG_WriteByte (&host_client->netchan.message, STAT_SECRETS);
	MSG_WriteLong (&host_client->netchan.message, PR_GLOBAL_STRUCT(found_secrets));

	MSG_WriteByte (&host_client->netchan.message, svc_updatestatlong);
	MSG_WriteByte (&host_client->netchan.message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->netchan.message, PR_GLOBAL_STRUCT(killed_monsters));

	// get the client to check and download skins
	// when that is completed, a begin command will be issued
	MSG_WriteByte (&host_client->netchan.message, svc_stufftext);
	MSG_WriteString (&host_client->netchan.message, va("skins\n") );
}

/*
==================
SV_SpawnSpectator
==================
*/
static void SV_SpawnSpectator (void)
{
	int		i;
	edict_t	*e;

	VectorClear (sv_player->v.origin);
	VectorClear (sv_player->v.view_ofs);
	sv_player->v.view_ofs[2] = 22;

	// search for an info_playerstart to spawn the spectator at
	for (i = MAX_CLIENTS-1; i < sv.num_edicts; i++)
	{
		e = EDICT_NUM(i);
		if (!strcmp(PR_GetString(e->v.classname), "info_player_start"))
		{
			VectorCopy (e->v.origin, sv_player->v.origin);
			return;
		}
	}
}

/*
==================
SV_Begin_f
==================
*/
static void SV_Begin_f (void)
{
	int		i;

	host_client->state = cs_spawned;

	// handle the case of a level changing while a client was connecting
	if ( atoi(Cmd_Argv(1)) != svs.spawncount )
	{
		Con_Printf ("%s from different level\n", __thisfunc__);
		SV_New_f ();
		return;
	}

	if (host_client->spectator)
	{
		SV_SpawnSpectator ();

		if (SpectatorConnect)
		{
			// copy spawn parms out of the client_t
			for (i = 0; i < NUM_SPAWN_PARMS; i++)
				(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

			// call the spawn function
			pr_global_struct->time = sv.time;
			pr_global_struct->self = EDICT_TO_PROG(sv_player);
			PR_ExecuteProgram (SpectatorConnect);
		}
	}
	else
	{
		// copy spawn parms out of the client_t
		for (i = 0; i < NUM_SPAWN_PARMS; i++)
			(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

		host_client->send_all_v = true;

		// call the spawn function
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		// actually spawn the player
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->PutClientInServer);
	}

	// clear the net statistics, because connecting gives a bogus picture
	host_client->netchan.frame_latency = 0;
	host_client->netchan.frame_rate = 0;
	host_client->netchan.drop_count = 0;
	host_client->netchan.good_count = 0;
#if 0
//
// send a fixangle over the reliable channel to make sure it gets there
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	MSG_WriteByte (&host_client->netchan.message, svc_setangle);
	for (i = 0; i < 2; i++)
		MSG_WriteAngle (&host_client->netchan.message, ent->v.angles[i] );
	MSG_WriteAngle (&host_client->netchan.message, 0 );
#endif
}

//=============================================================================

/*
==================
SV_NextDownload_f
==================
*/
static void SV_NextDownload_f (void)
{
	byte	buffer[1024];
	int		r;
	int		percent;
	int		size;

	if (!host_client->download)
		return;

	r = host_client->downloadsize - host_client->downloadcount;
	if (r > 1024)
		r = 1024;
	r = fread (buffer, 1, r, host_client->download);
	MSG_WriteByte (&host_client->netchan.message, svc_download);
	MSG_WriteShort (&host_client->netchan.message, r);

	host_client->downloadcount += r;
	size = host_client->downloadsize;
	if (!size)
		size = 1;
	percent = host_client->downloadcount*100/size;
	MSG_WriteByte (&host_client->netchan.message, percent);
	SZ_Write (&host_client->netchan.message, buffer, r);

	if (host_client->downloadcount != host_client->downloadsize)
		return;

	fclose (host_client->download);
	host_client->download = NULL;
}

/*
==================
SV_BeginDownload_f
==================
*/
static void SV_BeginDownload_f(void)
{
	char	*name, *p;

	name = Cmd_Argv(1);

	// hacked by zoid to allow more conrol over download
	// first off, no .. or global allow check
	if (strstr (name, "..") || !allow_download.integer
		// leading dot is no good
		|| *name == '.'
		// leading slash bad as well, must be in subdir
		|| *name == '/'
		// next up, skin check
		|| (strncmp(name, "skins/", 6) == 0 && !allow_download_skins.integer)
		// now models
		|| (strncmp(name, "models/", 6) == 0 && !allow_download_models.integer)
		// now sounds
		|| (strncmp(name, "sound/", 6) == 0 && !allow_download_sounds.integer)
		// now maps (note special case for maps, must not be in pak)
		|| (strncmp(name, "maps/", 6) == 0 && !allow_download_maps.integer)
		// MUST be in a subdirectory
		|| !strstr (name, "/") )
	{	// don't allow anything with .. path
		MSG_WriteByte (&host_client->netchan.message, svc_download);
		MSG_WriteShort (&host_client->netchan.message, -1);
		MSG_WriteByte (&host_client->netchan.message, 0);
		return;
	}

	if (host_client->download)
	{
		fclose (host_client->download);
		host_client->download = NULL;
	}

	// lowercase the name (needed for case sensitive file systems)
	for (p = name; *p; p++)
		*p = (char)tolower(*p);

	host_client->downloadsize = FS_OpenFile (name, &host_client->download, false);
	host_client->downloadcount = 0;

	if (!host_client->download
		// special check for maps, if it came from a pak file,
		// don't allow download  ZOID
		|| (strncmp(name, "maps/", 5) == 0 && file_from_pak))
	{
		if (host_client->download)
		{
			fclose(host_client->download);
			host_client->download = NULL;
		}

		Sys_Printf ("Couldn't download %s to %s\n", name, host_client->name);
		MSG_WriteByte (&host_client->netchan.message, svc_download);
		MSG_WriteShort (&host_client->netchan.message, -1);
		MSG_WriteByte (&host_client->netchan.message, 0);
		return;
	}

	SV_NextDownload_f ();
	Sys_Printf ("Downloading %s to %s\n", name, host_client->name);
}


//=============================================================================

/*
==================
SV_Say
==================
*/
static void SV_Say (qboolean team)
{
	client_t	*client;
	int			j, tmp;
	char		*p;
	char		text[2048];
	char		t1[32], *t2;
	int			speaknum = -1;

	if (Cmd_Argc () < 2)
		return;

	if (team)
		q_strlcpy (t1, Info_ValueForKey(host_client->userinfo, "team"), sizeof(t1));

	if (host_client->spectator && (!sv_spectalk.integer || team))
		q_snprintf (text, sizeof(text), "[SPEC] %s: ", host_client->name);
	else if (team)
		q_snprintf (text, sizeof(text), "(%s): ", host_client->name);
	else
		q_snprintf (text, sizeof(text), "%s: ", host_client->name);

	if (fp_messages)
	{
		if (realtime<host_client->lockedtill)
		{
			SV_ClientPrintf(host_client, PRINT_CHAT,
					"You can't talk for %d more seconds\n", 
					(int) (host_client->lockedtill - realtime));
			return;
		}
		tmp = host_client->whensaidhead - fp_messages + 1;
		if (tmp < 0)
			tmp = 10+tmp;
		if (host_client->whensaid[tmp] && (realtime-host_client->whensaid[tmp] < fp_persecond))
		{
			host_client->lockedtill = realtime + fp_secondsdead;
			if (fp_msg[0])
				SV_ClientPrintf(host_client, PRINT_CHAT, "FloodProt: %s\n", fp_msg);
			else
				SV_ClientPrintf(host_client, PRINT_CHAT, "FloodProt: You can't talk for %d seconds.\n", fp_secondsdead);
			return;
		}
		host_client->whensaidhead++;
		if (host_client->whensaidhead > 9)
			host_client->whensaidhead = 0;
		host_client->whensaid[host_client->whensaidhead] = realtime;
	}

	p = Cmd_Args();

	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

	if (p[0] == '`' && (!host_client->spectator && sv_allowtaunts.integer) )
	{
		speaknum = atoi(&p[1]);
		if (speaknum <= 0 || speaknum > 255-PRINT_SOUND)
		{
			speaknum = -1;
		}
		else
		{
			text[strlen(text)-2] = 0;
			q_strlcat(text," speaks!\n", sizeof(text));
		}
	}

	if (speaknum == -1)
	{
		q_strlcat(text, p, sizeof(text));
		q_strlcat(text, "\n", sizeof(text));
	}

	Sys_Printf ("%s", text);

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;
		if (host_client->spectator && !sv_spectalk.integer)
			if (!client->spectator)
				continue;

		if (team)
		{
			// the spectator team
			if (host_client->spectator)
			{
				if (!client->spectator)
					continue;
			}
			else
			{
				t2 = Info_ValueForKey (client->userinfo, "team");
				if (dmMode.integer == DM_SIEGE)
				{
					if ( (host_client->edict->v.skin == 102 && client->edict->v.skin != 102) ||
							(client->edict->v.skin == 102 && host_client->edict->v.skin != 102))
						// noteam players can team chat with each other,
						// cannot recieve team chat of other players
						continue;

					if (client->siege_team != host_client->siege_team)
						// on different teams
						continue;
				}
				else if (strcmp(t1, t2) || client->spectator)
					continue;	// on different teams
			}
		}
		if (speaknum == -1)
		{
			if (dmMode.integer == DM_SIEGE && host_client->siege_team != client->siege_team)
				//other team speaking
				SV_ClientPrintf(client, PRINT_CHAT, "%s", text);//fixme: print biege
			else
				SV_ClientPrintf(client, PRINT_CHAT, "%s", text);
		}
		else
		{
			SV_ClientPrintf(client, PRINT_SOUND + speaknum-1, "%s", text);
		}
	}
}

/*
==================
SV_Say_f
==================
*/
static void SV_Say_f(void)
{
	SV_Say (false);
}

/*
==================
SV_Say_Team_f
==================
*/
static void SV_Say_Team_f(void)
{
	SV_Say (true);
}


//============================================================================

/*
=================
SV_Pings_f

The client is showing the scoreboard, so send new ping times for all
clients
=================
*/
static void SV_Pings_f (void)
{
	client_t	*client;
	int		j;

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		MSG_WriteByte (&host_client->netchan.message, svc_updateping);
		MSG_WriteByte (&host_client->netchan.message, j);
		MSG_WriteShort (&host_client->netchan.message, SV_CalcPing(client));
	}
}

/*
==================
SV_Kill_f
==================
*/
static void SV_Kill_f (void)
{
	if (sv_player->v.health <= 0 && sv_player->v.deadflag != DEAD_NO)
	{
		SV_ClientPrintf (host_client, PRINT_HIGH, "Can't suicide -- already dead!\n");
		return;
	}

	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (pr_global_struct->ClientKill);
}

/*
=================
SV_Drop_f

The client is going to disconnect, so remove the connection immediately
=================
*/
static void SV_Drop_f (void)
{
	SV_EndRedirect ();
	if (!host_client->spectator)
		SV_BroadcastPrintf (PRINT_HIGH, "%s dropped\n", host_client->name);
	SV_DropClient (host_client);
}

/*
=================
SV_PTrack_f

Change the bandwidth estimate for a client
=================
*/
static void SV_PTrack_f (void)
{
	int		i;

	if (Cmd_Argc() != 2)
	{
		// turn off tracking
		host_client->spec_track = 0;
		return;
	}

	i = atoi(Cmd_Argv(1));
	if (i < 0 || i >= MAX_CLIENTS || svs.clients[i].state != cs_spawned || svs.clients[i].spectator)
	{
		SV_ClientPrintf (host_client, PRINT_HIGH, "Invalid client to track\n");
		host_client->spec_track = 0;
		return;
	}
	host_client->spec_track = i + 1; // now tracking
}

/*
=================
SV_Rate_f

Change the bandwidth estimate for a client
=================
*/
static void SV_Rate_f (void)
{
	int		rate;

	if (Cmd_Argc() != 2)
	{
		SV_ClientPrintf (host_client, PRINT_HIGH, "Current rate is %i\n",
				(int)(1.0/host_client->netchan.rate + 0.5));
		return;
	}

	rate = atoi(Cmd_Argv(1));
	if (rate < 500)
		rate = 500;
	if (rate > 10000)
		rate = 10000;

	SV_ClientPrintf (host_client, PRINT_HIGH, "Net rate set to %i\n", rate);
	host_client->netchan.rate = 1.0/rate;
}

/*
=================
SV_Msg_f

Change the message level for a client
=================
*/
static void SV_Msg_f (void)
{
	if (Cmd_Argc() != 2)
	{
		SV_ClientPrintf (host_client, PRINT_HIGH, "Current msg level is %i\n",
				host_client->messagelevel);
		return;
	}

	host_client->messagelevel = atoi(Cmd_Argv(1));

	SV_ClientPrintf (host_client, PRINT_HIGH, "Msg level set to %i\n", host_client->messagelevel);
}

/*
==================
SV_SetInfo_f

Allow clients to change userinfo
==================
*/
static void SV_SetInfo_f (void)
{
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("User info settings:\n");
		Info_Print (host_client->userinfo);
		return;
	}

	if (Cmd_Argc() != 3)
	{
		Con_Printf ("usage: setinfo [ <key> <value> ]\n");
		return;
	}

	if (Cmd_Argv(1)[0] == '*')
		return;	// don't set priveledged values

	Info_SetValueForKey (host_client->userinfo, Cmd_Argv(1), Cmd_Argv(2), MAX_INFO_STRING);
	q_strlcpy (host_client->name, Info_ValueForKey (host_client->userinfo, "name"), sizeof(host_client->name));
//	SV_FullClientUpdate (host_client, &sv.reliable_datagram);
	host_client->sendinfo = true;

	// process any changed values
	SV_ExtractFromUserinfo (host_client);
}

/*
==================
SV_ShowServerinfo_f

Dumps the serverinfo info string
==================
*/
static void SV_ShowServerinfo_f (void)
{
	Info_Print (svs.info);
}


/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

typedef struct
{
	char	*name;
	void	(*func) (void);
} ucmd_t;

static ucmd_t ucmds[] =
{
	{"new", SV_New_f},
	{"modellist", SV_Modellist_f},
	{"soundlist", SV_Soundlist_f},
	{"prespawn", SV_PreSpawn_f},
	{"spawn", SV_Spawn_f},
	{"begin", SV_Begin_f},

	{"drop", SV_Drop_f},
	{"pings", SV_Pings_f},

// issued by hand at client consoles
	{"rate", SV_Rate_f},
	{"kill", SV_Kill_f},
	{"msg", SV_Msg_f},

	{"say", SV_Say_f},
	{"say_team", SV_Say_Team_f},

	{"setinfo", SV_SetInfo_f},

	{"serverinfo", SV_ShowServerinfo_f},

	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},

	{"ptrack", SV_PTrack_f}, //ZOID - used with autocam

	{NULL, NULL}
};

/*
==================
SV_ExecuteUserCommand
==================
*/
void SV_ExecuteUserCommand (char *s)
{
	ucmd_t	*u;

	Cmd_TokenizeString (s);
	sv_player = host_client->edict;

	SV_BeginRedirect (RD_CLIENT);

	for (u = ucmds; u->name; u++)
	{
		if (!strcmp (Cmd_Argv(0), u->name) )
		{
			u->func ();
			break;
		}
	}

	if (!u->name)
		Con_Printf ("Bad user command: %s\n", Cmd_Argv(0));

	SV_EndRedirect ();
}

//============================================================================

/*
===============
V_CalcRoll

Used by view and sv_user
===============
*/
static float V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	vec3_t	forward, right, up;
	float	sign;
	float	side;
	float	value;

	AngleVectors (angles, forward, right, up);
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = cl_rollangle.value;

	if (side < cl_rollspeed.value)
		side = side * value / cl_rollspeed.value;
	else
		side = value;

	return side*sign;
}


//============================================================================

static vec3_t	pmove_mins, pmove_maxs;

/*
====================
AddLinksToPmove

====================
*/
static void AddLinksToPmove (areanode_t *node)
{
	link_t		*l, *next;
	edict_t		*check;
	int			pl;
	int			i;
	physent_t	*pe;

	pl = EDICT_TO_PROG(sv_player);

	// touch linked edicts
	for (l = node->solid_edicts.next ; l != &node->solid_edicts ; l = next)
	{
		next = l->next;
		check = EDICT_FROM_AREA(l);

		if (check->v.owner == pl)
			continue;		// player's own missile

		if (check->v.solid == SOLID_BSP
			|| check->v.solid == SOLID_BBOX
			|| check->v.solid == SOLID_SLIDEBOX)
		{
			if (check == sv_player)
				continue;

			for (i = 0; i < 3; i++)
			{
				if (check->v.absmin[i] > pmove_maxs[i]
						|| check->v.absmax[i] < pmove_mins[i])
					break;
			}

			if (i != 3)
				continue;

			if (pmove.numphysent == MAX_PHYSENTS)
				return;

			pe = &pmove.physents[pmove.numphysent];
			pmove.numphysent++;

			VectorCopy (check->v.origin, pe->origin);
			VectorCopy (check->v.angles, pe->angles);
			pe->info = NUM_FOR_EDICT(check);

			if (check->v.solid == SOLID_BSP)
				pe->model = sv.models[(int)(check->v.modelindex)];
			else
			{
				pe->model = NULL;
				VectorCopy (check->v.mins, pe->mins);
				VectorCopy (check->v.maxs, pe->maxs);
			}
		}
	}

	// recurse down both sides
	if (node->axis == -1)
		return;

	if ( pmove_maxs[node->axis] > node->dist )
		AddLinksToPmove ( node->children[0] );
	if ( pmove_mins[node->axis] < node->dist )
		AddLinksToPmove ( node->children[1] );
}

/*
================
AddAllEntsToPmove

For debugging
================
*/
#if 0
static void AddAllEntsToPmove (void)
{
	int			e;
	edict_t		*check;
	int			i;
	physent_t	*pe;
	int			pl;

	pl = EDICT_TO_PROG(sv_player);
	check = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.owner == pl)
			continue;
		if (check->v.solid == SOLID_BSP
			|| check->v.solid == SOLID_BBOX
			|| check->v.solid == SOLID_SLIDEBOX)
		{
			if (check == sv_player)
				continue;

			for (i = 0; i < 3; i++)
			{
				if (check->v.absmin[i] > pmove_maxs[i]
						|| check->v.absmax[i] < pmove_mins[i])
					break;
			}

			if (i != 3)
				continue;

			pe = &pmove.physents[pmove.numphysent];

			VectorCopy (check->v.origin, pe->origin);
			VectorCopy (check->v.angles, pe->angles);
			pmove.physents[pmove.numphysent].info = e;

			if (check->v.solid == SOLID_BSP)
				pe->model = sv.models[(int)(check->v.modelindex)];
			else
			{
				pe->model = NULL;
				VectorCopy (check->v.mins, pe->mins);
				VectorCopy (check->v.maxs, pe->maxs);
			}

			if (++pmove.numphysent == MAX_PHYSENTS)
				break;
		}
	}
}
#endif

/*
===========
SV_PreRunCmd
Done before running a player command.  Clears the touch array
===========
*/
static byte playertouch[(MAX_EDICTS+7)/8];

static void SV_PreRunCmd(void)
{
	memset(playertouch, 0, sizeof(playertouch));
}

/*
===========
SV_RunCmd
===========
*/
static void SV_RunCmd (usercmd_t *ucmd)
{
	edict_t		*ent;
	int			i, n;
	int			oldmsec;

	cmd = *ucmd;

	// chop up very long commands
	if (cmd.msec > 50)
	{
		oldmsec = ucmd->msec;
		cmd.msec = oldmsec/2;
		SV_RunCmd (&cmd);
		cmd.msec = oldmsec/2;
		cmd.impulse = 0;
		SV_RunCmd (&cmd);
		return;
	}

	if (!sv_player->v.fixangle)
		VectorCopy (ucmd->angles, sv_player->v.v_angle);

	sv_player->v.button0 = ucmd->buttons & 1;
	sv_player->v.button2 = (ucmd->buttons & 2)>>1;

	if (ucmd->buttons & 4 || sv_player->v.playerclass == CLASS_DWARF) // crouched?
		sv_player->v.flags2 = ((int)sv_player->v.flags2) | FL2_CROUCHED;
	else
		sv_player->v.flags2 = ((int)sv_player->v.flags2) & (~FL2_CROUCHED);

	if (ucmd->impulse)
		sv_player->v.impulse = ucmd->impulse;

//
// angles
// show 1/3 the pitch angle and all the roll angle
	if (sv_player->v.health > 0)
	{
		if (!sv_player->v.fixangle)
		{
			sv_player->v.angles[PITCH] = -sv_player->v.v_angle[PITCH]/3;
			sv_player->v.angles[YAW] = sv_player->v.v_angle[YAW];
		}
		sv_player->v.angles[ROLL] = 
				V_CalcRoll (sv_player->v.angles, sv_player->v.velocity)*4;
	}

	host_frametime = ucmd->msec * 0.001;
	if (host_frametime > HX_FRAME_TIME)
		host_frametime = HX_FRAME_TIME;

	if (!host_client->spectator)
	{
		pr_global_struct->frametime = host_frametime;

		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->PlayerPreThink);

		SV_RunThink (sv_player);
	}

	for (i = 0; i < 3; i++)
		pmove.origin[i] = sv_player->v.origin[i] + (sv_player->v.mins[i] - player_mins[i]);

	VectorCopy (sv_player->v.velocity, pmove.velocity);
	VectorCopy (sv_player->v.v_angle, pmove.angles);

	pmove.spectator = host_client->spectator;
//	pmove.waterjumptime = sv_player->v.teleport_time;
	pmove.numphysent = 1;
	pmove.physents[0].model = sv.worldmodel;
	pmove.cmd = *ucmd;
	pmove.dead = sv_player->v.health <= 0;
	pmove.oldbuttons = host_client->oldbuttons;
	pmove.hasted = sv_player->v.hasted;
	pmove.movetype = sv_player->v.movetype;
	pmove.crouched = (sv_player->v.hull == HULL_CROUCH);
	pmove.teleport_time = realtime + (sv_player->v.teleport_time - sv.time);

//	movevars.entgravity = host_client->entgravity;
	movevars.entgravity = sv_player->v.gravity;
	movevars.maxspeed = host_client->maxspeed;

	for (i = 0; i < 3; i++)
	{
		pmove_mins[i] = pmove.origin[i] - 256;
		pmove_maxs[i] = pmove.origin[i] + 256;
	}
#if 1
	AddLinksToPmove ( sv_areanodes );
#else
	AddAllEntsToPmove ();
#endif

#if 0
	{
		int	before, after;

		before = PM_TestPlayerPosition (pmove.origin);
		PlayerMove ();
		after = PM_TestPlayerPosition (pmove.origin);

		if (sv_player->v.health > 0 && before && !after )
			Con_Printf ("player %s got stuck in playermove!!!!\n", host_client->name);
	}
#else
	PlayerMove ();
#endif

	host_client->oldbuttons = pmove.oldbuttons;
//	sv_player->v.teleport_time = pmove.waterjumptime;
	sv_player->v.waterlevel = waterlevel;
	sv_player->v.watertype = watertype;
	if (onground != -1)
	{
		sv_player->v.flags = (int)sv_player->v.flags | FL_ONGROUND;
		sv_player->v.groundentity = EDICT_TO_PROG(EDICT_NUM(pmove.physents[onground].info));
	}
	else
		sv_player->v.flags = (int)sv_player->v.flags & ~FL_ONGROUND;

	for (i = 0; i < 3; i++)
		sv_player->v.origin[i] = pmove.origin[i] - (sv_player->v.mins[i] - player_mins[i]);

#if 0
	// truncate velocity the same way the net protocol will
	for (i = 0; i < 3; i++)
		sv_player->v.velocity[i] = (int)pmove.velocity[i];
#else
	VectorCopy (pmove.velocity, sv_player->v.velocity);
#endif

	VectorCopy (pmove.angles, sv_player->v.v_angle);

	if (!host_client->spectator)
	{
		// link into place and touch triggers
		SV_LinkEdict (sv_player, true);

		// touch other objects
		for (i = 0; i < pmove.numtouch; i++)
		{
			n = pmove.physents[pmove.touchindex[i]].info;
			ent = EDICT_NUM(n);

		// Why not just do an SV_Impact here?
		//	SV_Impact(sv_player,ent);

			if (sv_player->v.touch)
			{
				pr_global_struct->self = EDICT_TO_PROG(sv_player);
				pr_global_struct->other = EDICT_TO_PROG(ent);
				PR_ExecuteProgram (sv_player->v.touch);
			}
			if (!ent->v.touch || (playertouch[n/8]&(1<<(n%8))))
				continue;
			pr_global_struct->self = EDICT_TO_PROG(ent);
			pr_global_struct->other = EDICT_TO_PROG(sv_player);
			PR_ExecuteProgram (ent->v.touch);
			playertouch[n/8] |= 1 << (n%8);
		}
	}
}

/*
===========
SV_PostRunCmd
Done after running a player command.
===========
*/
static void SV_PostRunCmd(void)
{
	// run post-think

	if (!host_client->spectator)
	{
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->PlayerPostThink);
		SV_RunNewmis ();
	}
	else if (SpectatorThink)
	{
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (SpectatorThink);
	}
}


/*
===================
SV_ExecuteClientMessage

The current net_message is parsed for the given client
===================
*/
void SV_ExecuteClientMessage (client_t *cl)
{
	int		c;
	char	*s;
	usercmd_t	oldest, oldcmd, newcmd;
	client_frame_t	*frame;
	vec3_t	o;

	// calc ping time
	frame = &cl->frames[cl->netchan.incoming_acknowledged & UPDATE_MASK];
	frame->ping_time = realtime - frame->senttime;

	// make sure the reply sequence number matches the incoming
	// sequence number
	if (cl->netchan.incoming_sequence >= cl->netchan.outgoing_sequence)
		cl->netchan.outgoing_sequence = cl->netchan.incoming_sequence;
	else
		cl->send_message = false;	// don't reply, sequences have slipped

	// save time for ping calculations
	cl->frames[cl->netchan.outgoing_sequence & UPDATE_MASK].senttime = realtime;
	cl->frames[cl->netchan.outgoing_sequence & UPDATE_MASK].ping_time = -1;

	host_client = cl;
	sv_player = host_client->edict;

	// mark time so clients will know how much to predict
	// other players
	cl->localtime = sv.time;
	cl->delta_sequence = -1;	// no delta unless requested
	while (1)
	{
		if (msg_badread)
		{
			Con_Printf ("%s: badread\n", __thisfunc__);
			SV_DropClient (cl);
			return;
		}

		c = MSG_ReadByte ();
		if (c == -1)
			break;

		switch (c)
		{
		default:
			Con_Printf ("%s: unknown command char\n", __thisfunc__);
			SV_DropClient (cl);
			return;

		case clc_nop:
			break;

		case clc_delta:
			cl->delta_sequence = MSG_ReadByte ();
			break;

		case clc_move:
			MSG_ReadUsercmd (&oldest, false);
			MSG_ReadUsercmd (&oldcmd, false);
			MSG_ReadUsercmd (&newcmd, true);

			if ( cl->state != cs_spawned )
				break;

			SV_PreRunCmd();

			if (net_drop < 20)
			{
				while (net_drop > 2)
				{
					SV_RunCmd (&cl->lastcmd);
					net_drop--;
				}
				if (net_drop > 1)
					SV_RunCmd (&oldest);
				if (net_drop > 0)
					SV_RunCmd (&oldcmd);
			}
			SV_RunCmd (&newcmd);

			SV_PostRunCmd();

			cl->lastcmd = newcmd;
			cl->lastcmd.buttons = 0; // avoid multiple fires on lag
			break;

		case clc_stringcmd:
			s = MSG_ReadString ();
			SV_ExecuteUserCommand (s);
			break;

		case clc_tmove:
			o[0] = MSG_ReadCoord();
			o[1] = MSG_ReadCoord();
			o[2] = MSG_ReadCoord();
			// only allowed by spectators
			if (host_client->spectator)
			{
				VectorCopy(o, sv_player->v.origin);
				SV_LinkEdict(sv_player, false);
			}
			break;

		case clc_inv_select:
			cl->edict->v.inventory = MSG_ReadByte();
			break;

		case clc_get_effect:
			c = MSG_ReadByte();
			if (sv.Effects[c].type)
			{
				Con_Printf("Getting effect %d\n",(int)c);
				SV_SendEffect(&host_client->netchan.message, c);
			}
			break;
		}
	}
}


/*
==============
SV_UserInit
==============
*/
void SV_UserInit (void)
{
	Cvar_RegisterVariable (&cl_rollspeed);
	Cvar_RegisterVariable (&cl_rollangle);
	Cvar_RegisterVariable (&sv_spectalk);
	Cvar_RegisterVariable (&sv_allowtaunts);
}


