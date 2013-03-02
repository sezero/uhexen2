/*
 * server/host_cmd.c -- console commands
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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
#include <ctype.h>

static	double	old_svtime;
	/* sv.time of prev. level when changing levels, saved by changelevel2().
	 * used for adjusting time globalvars of progs by RestoreClients() when
	 * travelling levels back and forth (see clients.hc::ClientReEnter().  */

static int LoadGamestate (const char *level, const char *startspot, int ClientsMode);
static void RestoreClients (int ClientsMode);

#define TESTSAVE

/*
==================
Host_Quit_f
==================
*/

void Host_Quit_f (void)
{
	Host_ShutdownServer(false);
	Sys_Quit ();
}


/*
==================
Host_Status_f
==================
*/
static void Host_Status_f (void)
{
	client_t	*client;
	int			seconds;
	int			minutes;
	int			hours = 0;
	int			j;
	void		(*print_fn)(unsigned int flg, const char *fmt, ...)
				__fp_attribute__((__format__(__printf__,2,3)));

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Con_Printf("Server not active\n");
			return;
		}
		print_fn = CON_Printf;
	}
	else
		print_fn = SV_ClientPrintf;

	print_fn (_PRINT_NORMAL, "host:    %s\n", Cvar_VariableString ("hostname"));
	print_fn (_PRINT_NORMAL, "version: %4.2f\n", ENGINE_VERSION);
	if (tcpipAvailable)
		print_fn (_PRINT_NORMAL, "tcp/ip:  %s\n", my_tcpip_address);
	if (ipxAvailable)
		print_fn (_PRINT_NORMAL, "ipx:     %s\n", my_ipx_address);
	print_fn (_PRINT_NORMAL, "map:     %s\n", sv.name);
	print_fn (_PRINT_NORMAL, "players: %i active (%i max)\n\n",
					net_activeconnections, svs.maxclients);
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - NET_QSocketGetTime(client->netconnection));
		minutes = seconds / 60;
		if (minutes)
		{
			seconds -= (minutes * 60);
			hours = minutes / 60;
			if (hours)
				minutes -= (hours * 60);
		}
		else
			hours = 0;
		print_fn (_PRINT_NORMAL, "#%-2u %-16.16s  %3i  %2i:%02i:%02i\n",
					j + 1, client->name, (int)client->edict->v.frags,
					hours, minutes, seconds);
		print_fn (_PRINT_NORMAL, "   %s\n", NET_QSocketGetAddressString(client->netconnection));
	}
}


/*
==================
Host_God_f

Sets client to godmode
==================
*/
static void Host_God_f (void)
{
	if (cmd_source == src_command)
		return;

	if (*sv_globals.deathmatch || *sv_globals.coop || skill.integer > 2)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
	if (!((int)sv_player->v.flags & FL_GODMODE) )
		SV_ClientPrintf (0, "godmode OFF\n");
	else
		SV_ClientPrintf (0, "godmode ON\n");
}

static void Host_Notarget_f (void)
{
	if (cmd_source == src_command)
		return;

	if (*sv_globals.deathmatch || skill.integer > 2)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
	if (!((int)sv_player->v.flags & FL_NOTARGET) )
		SV_ClientPrintf (0, "notarget OFF\n");
	else
		SV_ClientPrintf (0, "notarget ON\n");
}

static void Host_Noclip_f (void)
{
	if (cmd_source == src_command)
		return;

	if (*sv_globals.deathmatch || *sv_globals.coop || skill.integer > 2)
		return;

	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		SV_ClientPrintf (0, "noclip ON\n");
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf (0, "noclip OFF\n");
	}
}


/*
==================
Host_Ping_f

==================
*/
static void Host_Ping_f (void)
{
	int		i, j;
	float		total;
	client_t	*client;

	if (cmd_source == src_command)
		return;

	SV_ClientPrintf (0, "Client ping times:\n");
	for (i = 0, client = svs.clients; i < svs.maxclients; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j = 0; j < NUM_PING_TIMES; j++)
			total += client->ping_times[j];
		total /= NUM_PING_TIMES;
		SV_ClientPrintf (0, "%4i %s\n", (int)(total*1000), client->name);
	}
}

/*
===============================================================================

SERVER TRANSITIONS

===============================================================================
*/


/*
======================
Host_Map_f

handle a 
map <servername>
command from the console.  Active clients are kicked off.
======================
*/
static void Host_Map_f (void)
{
	char	name[MAX_QPATH];

	if (Cmd_Argc() < 2)	//no map name given
	{
		Con_Printf ("map <levelname>: start a new server\n");
		if (sv.active)
		{
			Con_Printf ("Current level: %s [ %s ]\n",
					SV_GetLevelname(), sv.name);
		}
		return;
	}

	if (cmd_source != src_command)
		return;

	Host_ShutdownServer(false);

	info_mask = 0;
	if (!coop.integer && deathmatch.integer)
		info_mask2 = 0x80000000;
	else
		info_mask2 = 0;

	svs.serverflags = 0;		// haven't completed an episode yet
	q_strlcpy (name, Cmd_Argv(1), sizeof(name));

	SV_SpawnServer (name, NULL);
}

/*
==================
Host_Changelevel_f

Goes to a new map, taking all clients along
==================
*/
static void Host_Changelevel_f (void)
{
	char	level[MAX_QPATH];
	char	_startspot[MAX_QPATH];
	char	*startspot;

	if (Cmd_Argc() < 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	if (!sv.active)
	{
		Con_Printf ("Server not active\n");
		return;
	}

	q_snprintf (level, sizeof(level), "maps/%s.bsp", Cmd_Argv(1));
	if (!FS_FileExists(level, NULL))
		Host_Error ("%s: cannot find map %s", __thisfunc__, level);

	q_strlcpy (level, Cmd_Argv(1), sizeof(level));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		q_strlcpy (_startspot, Cmd_Argv(2), sizeof(_startspot));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();
	SV_SpawnServer (level, startspot);
	if (!sv.active)
		Host_Error ("%s: cannot run map %s", __thisfunc__, level);
}

/*
==================
Host_Changelevel2_f

changing levels within a unit
==================
*/
static void Host_Changelevel2_f (void)
{
	char	level[MAX_QPATH];
	char	_startspot[MAX_QPATH];
	char	*startspot;

	if (Cmd_Argc() < 2)
	{
		Con_Printf ("changelevel2 <levelname> : continue game on a new level in the unit\n");
		return;
	}
	if (!sv.active)
	{
		Con_Printf ("Server not active\n");
		return;
	}

	q_snprintf (level, sizeof(level), "maps/%s.bsp", Cmd_Argv(1));
	if (!FS_FileExists(level, NULL))
		Host_Error ("%s: cannot find map %s", __thisfunc__, level);

	q_strlcpy (level, Cmd_Argv(1), sizeof(level));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		q_strlcpy (_startspot, Cmd_Argv(2), sizeof(_startspot));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();

	// save the current level's state
	old_svtime = sv.time;
	if (SaveGamestate(false) != 0)
		return;

	// try to restore the new level
	if (LoadGamestate(level, startspot, 0) != 0)
	{
		SV_SpawnServer (level, startspot);
		if (!sv.active)
			Host_Error ("%s: cannot run map %s", __thisfunc__, level);
		RestoreClients (0);
	}
}

/*
==================
Host_Restart_f

Restarts the current server for a dead player
==================
*/
static void Host_Restart_f (void)
{
	char	mapname[MAX_QPATH];
	char	startspot[MAX_QPATH];

	if (cmd_source != src_command)
		return;

	if (!sv.active)
	{
		Con_Printf ("Server not active\n");
		return;
	}

	q_strlcpy (mapname, sv.name, sizeof(mapname));	// mapname gets cleared in spawnserver
	q_strlcpy (startspot, sv.startspot, sizeof(startspot));

	if (Cmd_Argc() == 2 && q_strcasecmp(Cmd_Argv(1),"restore") == 0)
	{
		if (LoadGamestate(mapname, startspot, 3) != 0)
		{
			SV_SpawnServer (mapname, startspot);
			if (!sv.active)
				Host_Error ("%s: cannot restart map %s", mapname, __thisfunc__);
			RestoreClients (0);
		}
	}
	else
	{
		SV_SpawnServer (mapname, startspot);
		if (!sv.active)
			Host_Error ("%s: cannot restart map %s", mapname, __thisfunc__);
	}
}

/*
===============================================================================

LOAD / SAVE GAME

===============================================================================
*/

static char	savename[MAX_OSPATH], savedest[MAX_OSPATH];

/*
===============
Host_SavegameComment

Writes a SAVEGAME_COMMENT_LENGTH character comment describing the game saved
===============
*/
static void Host_SavegameComment (char *text)
{
	size_t		i;
	char		temp[20];
	const char	*levelname;

	for (i = 0; i < SAVEGAME_COMMENT_LENGTH; i++)
	{
		text[i] = ' ';
	}

/* see SAVEGAME_COMMENT_LENGTH definition in quakedef.h */
	levelname = SV_GetLevelname ();
	i = strlen(levelname);
	if (i > 20)
		i = 20;
	memcpy (text, levelname, i);

	Sys_DateTimeString (temp);
	temp[16] = '\0'; // eliminate seconds
	i = strlen(temp);
	memcpy (text+21, temp, i);

// convert space to _ to make stdio happy
	for (i = 0; i < SAVEGAME_COMMENT_LENGTH; i++)
	{
		if (text[i] == ' ')
			text[i] = '_';
	}

	text[SAVEGAME_COMMENT_LENGTH] = '\0';
}

/*
===============
Host_Savegame_f
===============
*/
static void Host_Savegame_f (void)
{
	FILE	*f;
	int		i, error_state;
	char		comment[SAVEGAME_COMMENT_LENGTH+1];
	const char	*p;

	if (cmd_source != src_command)
		return;

	if (!sv.active)
	{
		Con_Printf ("Server not active\n");
		return;
	}

#ifndef TESTSAVE
	if (svs.maxclients != 1)
	{
		Con_Printf ("Can't save multiplayer games.\n");
		return;
	}
#endif

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("save <savename> : save a game\n");
		return;
	}

	p = Cmd_Argv(1);
	if (*p == '.' || strstr(p, ".."))
	{
		Con_Printf ("Invalid save name.\n");
		return;
	}

	for (i = 0; i < svs.maxclients; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			Con_Printf ("Can't savegame with a dead player\n");
			return;
		}
	}

	error_state = SaveGamestate (false);
	// don't bother doing more if SaveGamestate failed
	if (error_state)
		return;

	FS_MakePath_BUF (FS_USERDIR, &error_state, savename, sizeof(savename), p);
	if (error_state)
	{
		Con_Printf ("%s: save directory name too long\n", __thisfunc__);
		return;
	}
	if (Sys_mkdir(savename, false) != 0)
	{
		Con_Printf ("Unable to create save directory\n");
		return;
	}

	Host_RemoveGIPFiles(savename);

	FS_MakePath_BUF (FS_USERDIR, NULL, savename, sizeof(savename), "clients.gip");
	Sys_unlink(savename);

	FS_MakePath_BUF (FS_USERDIR, NULL, savedest, sizeof(savedest), p);
	Con_Printf ("Saving game to %s...\n", savedest);

	error_state = Host_CopyFiles(FS_GetUserdir(), "*.gip", savedest);
	if (error_state)
		goto finish;

	FS_MakePath_VABUF (FS_USERDIR, &error_state, savedest, sizeof(savedest), "%s/info.dat", p);
	if (error_state)
	{
		Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
		return;
	}
	f = fopen (savedest, "w");
	if (!f)
	{
		error_state = 1;
		Con_Printf ("%s: Unable to open %s for writing!\n", __thisfunc__, savedest);
		goto finish;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
	for (i = 0; i < NUM_SPAWN_PARMS; i++)
		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%d\n", current_skill);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n", sv.time);
	fprintf (f, "%d\n", svs.maxclients);
	fprintf (f, "%f\n", deathmatch.value);
	fprintf (f, "%f\n", coop.value);
	fprintf (f, "%f\n", teamplay.value);
	fprintf (f, "%f\n", randomclass.value);
	fprintf (f, "%f\n", 1.0);	// dummy cl_playerclass.value
	// mission pack, objectives strings
	fprintf (f, "%u\n", info_mask);
	fprintf (f, "%u\n", info_mask2);

	error_state = ferror(f);
	fclose(f);

finish:
	if (error_state)
		Host_Error ("%s: The game could not be saved properly!", __thisfunc__);
}


/*
===============
Host_DeleteSave_f
===============
*/
static void Host_DeleteSave_f (void)
{
	const char	*p;
	int	err;

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("deletesave <savename> : delete a saved game\n");
		return;
	}

	p = Cmd_Argv(1);
	if (*p == '.' || strstr(p, ".."))
	{
		Con_Printf ("Invalid save name.\n");
		return;
	}

	FS_MakePath_BUF (FS_USERDIR, &err, savename, sizeof(savename), p);
	if (!err)
		Host_DeleteSave (savename);
}


/*
===============
Host_Loadgame_f
===============
*/
static void Host_Loadgame_f (void)
{
	FILE	*f;
	char		mapname[MAX_QPATH];
	float		playtime;
	char		str[32768];
	int		version;
	int		i, error_state;
	int		tempi;
	float		tempf;
	edict_t		*ent;
	float		spawn_parms[NUM_SPAWN_PARMS];

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("load <savename> : load a game\n");
		return;
	}

	Host_RemoveGIPFiles(NULL);

	FS_MakePath_BUF (FS_USERDIR, &error_state, savename, sizeof(savename), Cmd_Argv(1));
	if (error_state)
	{
		Con_Printf ("%s: save directory name too long\n", __thisfunc__);
		return;
	}
	Con_Printf ("Loading game from %s...\n", savename);

	if (q_snprintf(savedest, sizeof(savedest), "%s/info.dat", savename) >= (int)sizeof(savedest))
	{
		Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
		return;
	}

	f = fopen (savedest, "r");
	if (!f)
	{
		Con_Printf ("%s: ERROR: couldn't open savefile\n", __thisfunc__);
		return;
	}

	fscanf (f, "%i\n", &version);

	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}
	fscanf (f, "%s\n", str);
	for (i = 0; i < NUM_SPAWN_PARMS; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);
// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tempf);
	current_skill = (int)(tempf + 0.1);
	Cvar_SetValue ("skill", current_skill);

	Cvar_Set ("deathmatch", "0");
	Cvar_Set ("coop", "0");
	Cvar_Set ("teamplay", "0");
	Cvar_Set ("randomclass", "0");

	fscanf (f, "%s\n", mapname);
	fscanf (f, "%f\n", &playtime);

	tempi = -1;
	fscanf (f, "%d\n", &tempi);
	if (tempi >= 1)
		svs.maxclients = tempi;

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	if (tempf >= 0)
		Cvar_SetValue ("deathmatch", tempf);

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	if (tempf >= 0)
		Cvar_SetValue ("coop", tempf);

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	if (tempf >= 0)
		Cvar_SetValue ("teamplay", tempf);

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	if (tempf >= 0)
		Cvar_SetValue ("randomclass", tempf);

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
		// nothing to do with this tempf (cl_playerclass.value)

	// mission pack, objectives strings
	fscanf (f, "%u\n", &info_mask);
	fscanf (f, "%u\n", &info_mask2);

	fclose (f);

	Host_RemoveGIPFiles(FS_GetUserdir());

	FS_MakePath_BUF (FS_USERDIR, NULL, savedest, sizeof(savedest), Cmd_Argv(1));
	error_state = Host_CopyFiles(savedest, "*.gip", FS_GetUserdir());
	if (error_state)
	{
		Host_Error ("%s: The game could not be loaded properly!", __thisfunc__);
		return;
	}

	if (LoadGamestate(mapname, NULL, 2) != 0)
		return;

	SV_SaveSpawnparms ();

	ent = EDICT_NUM(1);

	// this may be rudundant with the setting in PR_LoadProgs, but not sure so its here too
	if (sv_globals.cl_playerclass)
		*sv_globals.cl_playerclass = ent->v.playerclass;

	svs.clients->playerclass = ent->v.playerclass;

	sv.paused = true;		// pause until all clients connect
	sv.loadgame = true;
}

int SaveGamestate (qboolean ClientsOnly)
{
	FILE	*f;
	edict_t		*ent;
	int		i, error_state;
	int		start, end;
	char		comment[SAVEGAME_COMMENT_LENGTH+1];

	if (ClientsOnly)
	{
		start = 1;
		end = svs.maxclients+1;

		FS_MakePath_BUF (FS_USERDIR, &error_state, savename, sizeof(savename), "clients.gip");
		if (error_state)
		{
			Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
			return -1;
		}
	}
	else
	{
		start = 1;
		end = sv.num_edicts;

		FS_MakePath_VABUF (FS_USERDIR, &error_state, savename, sizeof(savename), "%s.gip", sv.name);
		if (error_state)
		{
			Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
			return -1;
		}
	}

	f = fopen (savename, "w");
	if (!f)
	{
		error_state = -1;
		Con_Printf ("%s: Unable to open %s for writing!\n", __thisfunc__, savename);
		goto finish;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);

	if (!ClientsOnly)
	{
		Host_SavegameComment (comment);
		fprintf (f, "%s\n", comment);
	//	for (i = 0; i < NUM_SPAWN_PARMS; i++)
	//		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
		fprintf (f, "%f\n", skill.value);
		fprintf (f, "%s\n", sv.name);
		fprintf (f, "%f\n", sv.time);

	// write the light styles
		for (i = 0; i < MAX_LIGHTSTYLES; i++)
		{
			if (sv.lightstyles[i])
				fprintf (f, "%s\n", sv.lightstyles[i]);
			else
				fprintf (f, "m\n");
		}
		SV_SaveEffects (f);
		fprintf (f, "-1\n");
		ED_WriteGlobals (f);
	}

	host_client = svs.clients;

// save the client states
	for (i = start; i < end; i++)
	{
		ent = EDICT_NUM(i);
		if ((int)ent->v.flags & FL_ARCHIVE_OVERRIDE)
			continue;
		if (ClientsOnly)
		{
			if (host_client->active)
			{
				fprintf (f, "%i\n", i);
				ED_Write (f, ent);
				fflush (f);
			}
			host_client++;
		}
		else
		{
			fprintf (f, "%i\n", i);
			ED_Write (f, ent);
			fflush (f);
		}
	}

	error_state = ferror(f);
	fclose (f);

finish:
	if (error_state)
		Host_Error ("%s: The level could not be saved properly!", __thisfunc__);

	return error_state;
}

static void RestoreClients (int ClientsMode)
{
	int		i, j;
	edict_t		*ent;
	double		time_diff;

	if (LoadGamestate(NULL, NULL, 1) != 0)
		return;

/* O.S. -- mode 3 is only in response to the single player game "restart restore"
 * command issued by progs.dat::client.hc::respawn() function.  No level change,
 * just respawning in the same map with the same playtime from when clients.gip
 * was saved, therefore there _CANNOT_ be a time_diff.  See uhexen2 bug #2176023:
 * http://sourceforge.net/tracker/?group_id=124987&atid=701006&aid=2176023&func=detail
 */
	time_diff = (ClientsMode == 3) ? 0 : sv.time - old_svtime;

	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (host_client->active)
		{
			ent = host_client->edict;

			//ent->v.colormap = NUM_FOR_EDICT(ent);
			ent->v.team = (host_client->colors & 15) + 1;
			ent->v.netname = PR_SetEngineString(host_client->name);
			ent->v.playerclass = host_client->playerclass;

			// copy spawn parms out of the client_t
			for (j = 0; j < NUM_SPAWN_PARMS; j++)
				sv_globals.parm[j] = host_client->spawn_parms[j];
			// call the spawn function
			*sv_globals.time = sv.time;
			*sv_globals.self = EDICT_TO_PROG(ent);
			G_FLOAT(OFS_PARM0) = time_diff;
			PR_ExecuteProgram (*sv_globals.ClientReEnter);
		}
	}

	SaveGamestate (true);
}

static int LoadGamestate (const char *level, const char *startspot, int ClientsMode)
{
	FILE	*f;
	char		mapname[MAX_QPATH];
	float		playtime, sk;
	char		str[32768];
	const char	*start;
	int		i, r;
	edict_t		*ent;
	int		entnum;
	int		version;
//	float		spawn_parms[NUM_SPAWN_PARMS];
	qboolean	auto_correct = false;

	if (ClientsMode == 1)	/* for RestoreClients() only: map must be active */
	{
		if (!sv.active)
		{
			Con_Printf ("%s: server not active\n", __thisfunc__);
			return -1;
		}
		FS_MakePath_BUF (FS_USERDIR, &r, savename, sizeof(savename), "clients.gip");
		if (r)
		{
			Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
			return -1;
		}
	}
	else
	{
		FS_MakePath_VABUF (FS_USERDIR, &r, savename, sizeof(savename), "%s.gip", level);
		if (r)
		{
			Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
			return -1;
		}

		if (ClientsMode != 2 && ClientsMode != 3)
			Con_Printf ("Loading game from %s...\n", savename);
	}

	f = fopen (savename, "r");
	if (!f)
	{
		if (ClientsMode == 2)
			Con_Printf ("%s: ERROR: couldn't open savefile\n", __thisfunc__);

		return -1;
	}

	fscanf (f, "%i\n", &version);

	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return -1;
	}

	if (ClientsMode != 1)
	{
		fscanf (f, "%s\n", str);
	//	for (i = 0; i < NUM_SPAWN_PARMS; i++)
	//		fscanf (f, "%f\n", &spawn_parms[i]);
		fscanf (f, "%f\n", &sk);
		Cvar_SetValue ("skill", sk);

		fscanf (f, "%s\n", mapname);
		fscanf (f, "%f\n", &playtime);

		SV_SpawnServer (mapname, startspot);
		if (!sv.active)
		{
			fclose (f);
			Con_Printf ("Couldn't load map\n");
			return -1;
		}

	// load the light styles
		for (i = 0; i < MAX_LIGHTSTYLES; i++)
		{
			fscanf (f, "%s\n", str);
			sv.lightstyles[i] = (const char *)Hunk_Strdup (str, "lightstyles");
		}
		SV_LoadEffects (f);
	}

// load the edicts out of the savegame file
	while (!feof(f))
	{
		fscanf (f, "%i\n", &entnum);
		for (i = 0; i < (int) sizeof(str) - 1; i++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == (int) sizeof(str) - 1)
		{
			fclose (f);
			Host_Error ("%s: Loadgame buffer overflow", __thisfunc__);
		}
		str[i] = 0;
		start = str;
		start = COM_Parse(str);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token,"{"))
		{
			fclose (f);
			Host_Error ("%s: First token isn't a brace", __thisfunc__);
		}

		// parse an edict
		if (entnum == -1)
		{
			ED_ParseGlobals (start);
			// Need to restore this
			*sv_globals.startspot = PR_SetEngineString(sv.startspot);
		}
		else
		{
			ent = EDICT_NUM(entnum);
			/* default to active edict: ED_ParseEdict() set it
			 * to free if it really is free.  cf. ED_Write()  */
			ent->free = false;
			/* ED_ParseEdict() will always memset ent->v to 0,
			 * because SaveGamestate() doesn't write entnum 0 */
			ED_ParseEdict (start, ent);

			if (ClientsMode == 1 || ClientsMode == 2 || ClientsMode == 3)
				ent->v.stats_restored = true;

			// link it into the bsp tree
			if (!ent->free)
			{
				if (entnum >= sv.num_edicts)
				{
				/* This is necessary to restore "generated" edicts which were
				 * not available during the map parsing by ED_LoadFromFile().
				 * This includes items dropped by monsters, items "dropped" by
				 * an item_spawner such as the "prizes" in the Temple of Mars
				 * (romeric5), a health sphere generated by the Crusader's
				 * Holy Strength ability, or a respawning-candidate killed
				 * monster in the expansion pack's nightmare mode. -- THOMAS */
				/* Moved this into the if (!ent->free) construct: less debug
				 * chatter.  Even if this skips a free edict in between, the
				 * skipped free edict wasn't parsed by ED_LoadFromFile() and
				 * it will remain as a freed edict. (There is no harm because
				 * we are dealing with extra edicts not originally present in
				 * the map.)  -- O.S. */
					Con_DPrintf("%s: entnum %d >= sv.num_edicts (%d)\n",
							__thisfunc__, entnum, sv.num_edicts);
					if (entnum < MAX_EDICTS)
					{
						sv.num_edicts = entnum + 1;
					}
					else
					{
					/* if we reach here, it can only mean trouble: the
					 * saved game is either incompatible or corrupt. */
						Con_DPrintf("%s: entity %d NOT restored!\n",
									__thisfunc__, entnum);
					}
				}

				SV_LinkEdict (ent, false);
				if (ent->v.modelindex && ent->v.model)
				{
					i = SV_ModelIndex(PR_GetString(ent->v.model));
					if (i != ent->v.modelindex)
					{
						ent->v.modelindex = i;
						auto_correct = true;
					}
				}
			}
		}
	}

	fclose (f);

	if (ClientsMode == 0)
	{
		sv.time = playtime;
		sv.paused = true;
		*sv_globals.serverflags = svs.serverflags;
		RestoreClients (0);
	}
	else if (ClientsMode == 2)
	{
		sv.time = playtime;
	}
	else if (ClientsMode == 3)
	{
		sv.time = playtime;
		*sv_globals.serverflags = svs.serverflags;
		RestoreClients (3);
	}

	if (ClientsMode != 1 && auto_correct)
	{
		Con_DPrintf("*** Auto-corrected model indexes!\n");
	}

//	for (i = 0; i < NUM_SPAWN_PARMS; i++)
//		svs.clients->spawn_parms[i] = spawn_parms[i];

	return 0;
}


//============================================================================

/*
======================
Host_Name_f
======================
*/
static void Host_Name_f (void)
{
	char	newName[32];
	char	*pdest;

	if (cmd_source == src_command)
		return;

	if (Cmd_Argc () == 2)
		q_strlcpy (newName, Cmd_Argv(1), sizeof(newName));
	else
		q_strlcpy (newName, Cmd_Args(), sizeof(newName));
	newName[15] = 0;	// client_t structure actually says name[32].

	//this is for the fuckers who put braces in the name causing loadgame to crash.
	pdest = strchr(newName,'{');
	if (pdest)
	{
		*pdest = 0;	//zap the brace
		Con_Printf ("Illegal char in name removed!\n");
	}

	if (host_client->name[0] && strcmp(host_client->name, "unconnected") )
		if (strcmp(host_client->name, newName) != 0)
			Con_Printf ("%s renamed to %s\n", host_client->name, newName);
	strcpy (host_client->name, newName);
	host_client->edict->v.netname = PR_SetEngineString(host_client->name);

// send notification to all clients
	MSG_WriteByte (&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteString (&sv.reliable_datagram, host_client->name);
}

static void Host_Class_f (void)
{
	float	newClass;

	if (cmd_source == src_command)
		return;

	if (Cmd_Argc () == 2)
		newClass = atof(Cmd_Argv(1));
	else
		newClass = atof(Cmd_Args());

	if (newClass < 0 || newClass > MAX_PLAYER_CLASS)
	{
		Con_Printf("Invalid player class.\n");
		return;
	}

#if ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO && (newClass != CLASS_PALADIN && newClass != CLASS_THEIF))
	{
		Con_Printf("That class is not available in this demo version.\n");
		return;
	}
#endif	/* OLD_DEMO */
	if (newClass == CLASS_DEMON)
	{
		if (!(gameflags & GAME_PORTALS))
		{
			Con_Printf("That class is only available in the mission pack.\n");
			return;
		}
		if (sv.active && (progs->crc != PROGS_V112_CRC))
		{	/* FIXME: This isn't right!!!  A custom progs can actually
			 * support 5 classes and can have v1.11 structures at the
			 * same time.  I don't know a way to detect any such thing,
			 * hence this lame solution! -- O.S.  */
			Con_Printf("progs.dat in use doesn't support that class.\n");
			return;
		}
	}

	if (sv.loadgame || host_client->playerclass)
	{
		if (host_client->edict->v.playerclass)
			newClass = host_client->edict->v.playerclass;
		else if (host_client->playerclass)
			newClass = host_client->playerclass;
	}

	host_client->playerclass = newClass;
	host_client->edict->v.playerclass = newClass;

	// Change the weapon model used
	*sv_globals.self = EDICT_TO_PROG(host_client->edict);
	PR_ExecuteProgram (*sv_globals.ClassChangeWeapon);

// send notification to all clients
	MSG_WriteByte (&sv.reliable_datagram, svc_updateclass);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, (byte)newClass);
}

static void Host_Say (qboolean teamonly)
{
	int		j;
	client_t	*client;
	client_t	*save;
	const char	*p;
	char		text[64], *p2;
	qboolean	quoted;
	qboolean	fromServer = false;

	if (cmd_source == src_command)
	{
		fromServer = true;
		teamonly = false;
	}

	if (Cmd_Argc () < 2)
		return;

	save = host_client;

	p = Cmd_Args();
// remove quotes if present
	quoted = false;
	if (*p == '\"')
	{
		p++;
		quoted = true;
	}
// turn on color set 1
	if (!fromServer)
		q_snprintf (text, sizeof(text), "\001%s: %s", save->name, p);
	else
		q_snprintf (text, sizeof(text), "\001<%s> %s", hostname.string, p);

// check length & truncate if necessary
	j = (int) strlen(text);
	if (j >= (int) sizeof(text) - 1)
	{
		text[sizeof(text) - 2] = '\n';
		text[sizeof(text) - 1] = '\0';
	}
	else
	{
		p2 = text + j;
		while ((const char *)p2 > (const char *)text &&
			(p2[-1] == '\r' || p2[-1] == '\n' || (p2[-1] == '\"' && quoted)) )
		{
			if (p2[-1] == '\"' && quoted)
				quoted = false;
			p2[-1] = '\0';
			p2--;
		}
		p2[0] = '\n';
		p2[1] = '\0';
	}

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;
		if (teamplay.integer && teamonly && client->edict->v.team != save->edict->v.team)
			continue;
		host_client = client;
		SV_ClientPrintf (0, "%s", text);
	}
	host_client = save;

	Sys_Printf("%s", &text[1]);
}


static void Host_Say_f (void)
{
	Host_Say(false);
}


static void Host_Say_Team_f (void)
{
	Host_Say(true);
}


static void Host_Tell_f (void)
{
	int		j;
	client_t	*client;
	client_t	*save;
	const char	*p;
	char		text[64], *p2;
	qboolean	quoted;

	if (cmd_source == src_command)
		return;

	if (Cmd_Argc () < 3)
		return;

	p = Cmd_Args();
// remove quotes if present
	quoted = false;
	if (*p == '\"')
	{
		p++;
		quoted = true;
	}
	q_snprintf (text, sizeof(text), "%s: %s", host_client->name, p);

// check length & truncate if necessary
	j = (int) strlen(text);
	if (j >= (int) sizeof(text) - 1)
	{
		text[sizeof(text) - 2] = '\n';
		text[sizeof(text) - 1] = '\0';
	}
	else
	{
		p2 = text + j;
		while ((const char *)p2 > (const char *)text &&
			(p2[-1] == '\r' || p2[-1] == '\n' || (p2[-1] == '\"' && quoted)) )
		{
			if (p2[-1] == '\"' && quoted)
				quoted = false;
			p2[-1] = '\0';
			p2--;
		}
		p2[0] = '\n';
		p2[1] = '\0';
	}

	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;
		if (q_strcasecmp(client->name, Cmd_Argv(1)))
			continue;
		host_client = client;
		SV_ClientPrintf (0, "%s", text);
		break;
	}
	host_client = save;
}


/*
==================
Host_Color_f
==================
*/
static void Host_Color_f (void)
{
	int		top, bottom;
	int		playercolor;

	if (cmd_source == src_command)
		return;

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else
	{
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}

	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

	playercolor = top*16 + bottom;

	host_client->colors = playercolor;
	host_client->edict->v.team = bottom + 1;

// send notification to all clients
	MSG_WriteByte (&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, host_client->colors);
}

/*
==================
Host_Kill_f
==================
*/
static void Host_Kill_f (void)
{
	if (cmd_source == src_command)
		return;

	if (sv_player->v.health <= 0 && sv_player->v.deadflag != DEAD_NO)
	{
		SV_ClientPrintf (0, "Can't suicide -- already dead!\n");
		return;
	}

	*sv_globals.time = sv.time;
	*sv_globals.self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (*sv_globals.ClientKill);
}


/*
==================
Host_Pause_f
==================
*/
static void Host_Pause_f (void)
{
	if (cmd_source == src_command)
		return;

	if (!pausable.integer)
		SV_ClientPrintf (0, "Pause not allowed.\n");
	else
	{
		sv.paused ^= 1;

		if (sv.paused)
		{
			SV_BroadcastPrintf ("%s paused the game\n", PR_GetString(sv_player->v.netname));
		}
		else
		{
			SV_BroadcastPrintf ("%s unpaused the game\n",PR_GetString(sv_player->v.netname));
		}

	// send notification to all clients
		MSG_WriteByte (&sv.reliable_datagram, svc_setpause);
		MSG_WriteByte (&sv.reliable_datagram, sv.paused);
	}
}

//===========================================================================


/*
==================
Host_PreSpawn_f
==================
*/
static void Host_PreSpawn_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("prespawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("prespawn not valid -- already spawned\n");
		return;
	}

	SZ_Write (&host_client->message, sv.signon.data, sv.signon.cursize);
	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 2);
	host_client->sendsignon = true;
}

/*
==================
Host_Spawn_f
==================
*/
static void Host_Spawn_f (void)
{
	int		i;
	client_t	*client;
	edict_t		*ent;

	if (cmd_source == src_command)
	{
		Con_Printf ("spawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("Spawn not valid -- already spawned\n");
		return;
	}

// send all current names, colors, and frag counts
	SZ_Clear (&host_client->message);

// run the entrance script
	if (sv.loadgame)
	{	// loaded games are fully inited already
		// if this is the last client to be connected, unpause
		sv.paused = false;
	}
	else
	{
		// set up the edict
		ent = host_client->edict;
		sv.paused = false;

		if (!ent->v.stats_restored || deathmatch.integer)
		{
			memset (&ent->v, 0, progs->entityfields * 4);

			//ent->v.colormap = NUM_FOR_EDICT(ent);
			ent->v.team = (host_client->colors & 15) + 1;
			ent->v.netname = PR_SetEngineString(host_client->name);
			ent->v.playerclass = host_client->playerclass;

			// copy spawn parms out of the client_t
			for (i = 0; i < NUM_SPAWN_PARMS; i++)
				sv_globals.parm[i] = host_client->spawn_parms[i];
			// call the spawn function
			*sv_globals.time = sv.time;
			*sv_globals.self = EDICT_TO_PROG(sv_player);
			PR_ExecuteProgram (*sv_globals.ClientConnect);

			if ((Sys_DoubleTime() - NET_QSocketGetTime(host_client->netconnection)) <= sv.time)
				Sys_Printf ("%s entered the game\n", host_client->name);

			PR_ExecuteProgram (*sv_globals.PutClientInServer);
		}
	}

// send time of update
	MSG_WriteByte (&host_client->message, svc_time);
	MSG_WriteFloat (&host_client->message, sv.time);

	for (i = 0, client = svs.clients; i < svs.maxclients; i++, client++)
	{
		MSG_WriteByte (&host_client->message, svc_updatename);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, client->name);

		MSG_WriteByte (&host_client->message, svc_updateclass);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->playerclass);

		MSG_WriteByte (&host_client->message, svc_updatefrags);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteShort (&host_client->message, client->old_frags);

		MSG_WriteByte (&host_client->message, svc_updatecolors);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->colors);
	}

// send all current light styles
	for (i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		MSG_WriteByte (&host_client->message, svc_lightstyle);
		MSG_WriteByte (&host_client->message, (char)i);
		MSG_WriteString (&host_client->message, sv.lightstyles[i]);
	}

//
// send some stats
//
	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->message, *sv_globals.total_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, *sv_globals.total_monsters);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, *sv_globals.found_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->message, *sv_globals.killed_monsters);

	SV_UpdateEffects(&host_client->message);

//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	MSG_WriteByte (&host_client->message, svc_setangle);
	for (i = 0; i < 2; i++)
		MSG_WriteAngle (&host_client->message, ent->v.angles[i] );
	MSG_WriteAngle (&host_client->message, 0);

	SV_WriteClientdataToMessage (host_client, sv_player, &host_client->message);

	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 3);
	host_client->sendsignon = true;
}

/*
==================
Host_Begin_f
==================
*/
static void Host_Begin_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("begin is not valid from the console\n");
		return;
	}

	host_client->spawned = true;
}

//===========================================================================


/*
==================
Host_Kick_f

Kicks a user off of the server
==================
*/
static void Host_Kick_f (void)
{
	const char	*who;
	const char	*message = NULL;
	client_t	*save;
	int			i;
	qboolean	byNumber = false;

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Con_Printf("Server not active\n");
			return;
		}
	}
	else if (*sv_globals.deathmatch)
		return;

	save = host_client;

	if (Cmd_Argc() > 2 && strcmp(Cmd_Argv(1), "#") == 0)
	{
		i = atof(Cmd_Argv(2)) - 1;
		if (i < 0 || i >= svs.maxclients)
			return;
		if (!svs.clients[i].active)
			return;
		host_client = &svs.clients[i];
		byNumber = true;
	}
	else
	{
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (!host_client->active)
				continue;
			if (q_strcasecmp(host_client->name, Cmd_Argv(1)) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (cmd_source == src_command)
			who = "Console";
		else
			who = save->name;

		// can't kick yourself!
		if (host_client == save)
			return;

		if (Cmd_Argc() > 2)
		{
			message = COM_Parse(Cmd_Args());
			if (byNumber)
			{
				message++;			// skip the #
				while (*message == ' ')		// skip white space
					message++;
				message += strlen(Cmd_Argv(2));	// skip the number
			}
			while (*message && *message == ' ')
				message++;
		}
		if (message)
			SV_ClientPrintf (0, "Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf (0, "Kicked by %s\n", who);
		SV_DropClient (false);
	}

	host_client = save;
}

/*
===============================================================================

DEBUGGING TOOLS

===============================================================================
*/

/*
==================
Host_Give_f
==================
*/
static void Host_Give_f (void)
{
	const char	*t;
	int	v;

	if (cmd_source == src_command)
		return;

	if (*sv_globals.deathmatch || skill.integer > 2)
		return;

	t = Cmd_Argv(1);
	v = atoi (Cmd_Argv(2));

	switch (t[0])
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (t[0] >= '2')
			   sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
			break;
		case 's':
		case 'n':
		case 'l':
		case 'r':
		case 'm':
			break;
		case 'h':
			sv_player->v.health = v;
			break;
		case 'c':
		case 'p':
			break;
	}
}

//=============================================================================

/*
==================
Host_InitCommands
==================
*/
void Host_InitCommands (void)
{
	Cmd_AddCommand ("status", Host_Status_f);
	Cmd_AddCommand ("quit", Host_Quit_f);
	Cmd_AddCommand ("god", Host_God_f);
	Cmd_AddCommand ("notarget", Host_Notarget_f);
	Cmd_AddCommand ("map", Host_Map_f);
	Cmd_AddCommand ("restart", Host_Restart_f);
	Cmd_AddCommand ("changelevel", Host_Changelevel_f);
	Cmd_AddCommand ("changelevel2", Host_Changelevel2_f);
	Cmd_AddCommand ("name", Host_Name_f);
	Cmd_AddCommand ("playerclass", Host_Class_f);
	Cmd_AddCommand ("noclip", Host_Noclip_f);
	Cmd_AddCommand ("say", Host_Say_f);
	Cmd_AddCommand ("say_team", Host_Say_Team_f);
	Cmd_AddCommand ("tell", Host_Tell_f);
	Cmd_AddCommand ("color", Host_Color_f);
	Cmd_AddCommand ("kill", Host_Kill_f);
	Cmd_AddCommand ("pause", Host_Pause_f);
	Cmd_AddCommand ("spawn", Host_Spawn_f);
	Cmd_AddCommand ("begin", Host_Begin_f);
	Cmd_AddCommand ("prespawn", Host_PreSpawn_f);
	Cmd_AddCommand ("kick", Host_Kick_f);
	Cmd_AddCommand ("ping", Host_Ping_f);
	Cmd_AddCommand ("load", Host_Loadgame_f);
	Cmd_AddCommand ("save", Host_Savegame_f);
	Cmd_AddCommand ("deletesave", Host_DeleteSave_f);
	Cmd_AddCommand ("give", Host_Give_f);
}

