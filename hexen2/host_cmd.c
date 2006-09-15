/*
	host_cmd.c

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/host_cmd.c,v 1.52 2006-09-15 09:24:33 sezero Exp $
*/

#include "quakedef.h"

#ifdef _WIN32
#include <io.h>
#endif
#ifdef PLATFORM_UNIX
#include <unistd.h>
#endif
#include <ctype.h>
#include <time.h>

extern	cvar_t	pausable;
extern	cvar_t	sv_flypitch;
extern	cvar_t	sv_walkpitch;

int	current_skill;
static double		old_time;

static int LoadGamestate(char *level, char *startspot, int ClientsMode);
qboolean SaveGamestate(qboolean ClientsOnly);
static void RestoreClients(void);

extern unsigned int	info_mask, info_mask2;	// mission pack objectives

extern qboolean	mousestate_sa;
extern void IN_ActivateMouse (void);
extern void IN_DeactivateMouse (void);
#define TESTSAVE

/*
==================
Host_Quit_f
==================
*/

void Host_Quit_f (void)
{
	if (key_dest != key_console && cls.state != ca_dedicated)
	{
		M_Menu_Quit_f ();
		return;
	}
	CL_Disconnect ();
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
	void		(*print) (char *fmt, ...);

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
		print = Con_Printf;
	}
	else
		print = SV_ClientPrintf;

	print ("host:    %s\n", Cvar_VariableString ("hostname"));
	print ("version: %4.2f\n", ENGINE_VERSION);
	if (tcpipAvailable)
		print ("tcp/ip:  %s\n", my_tcpip_address);
	if (ipxAvailable)
		print ("ipx:     %s\n", my_ipx_address);
	print ("map:     %s\n", sv.name);
	print ("players: %i active (%i max)\n\n", net_activeconnections, svs.maxclients);
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - client->netconnection->connecttime);
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
		print ("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n", j+1, client->name, (int)client->edict->v.frags, hours, minutes, seconds);
		print ("   %s\n", client->netconnection->address);
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
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (PR_GLOBAL_STRUCT(deathmatch) || PR_GLOBAL_STRUCT(coop) || skill.value > 2)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
	if (!((int)sv_player->v.flags & FL_GODMODE) )
		SV_ClientPrintf ("godmode OFF\n");
	else
		SV_ClientPrintf ("godmode ON\n");
}

static void Host_Notarget_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (PR_GLOBAL_STRUCT(deathmatch) || skill.value > 2)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
	if (!((int)sv_player->v.flags & FL_NOTARGET) )
		SV_ClientPrintf ("notarget OFF\n");
	else
		SV_ClientPrintf ("notarget ON\n");
}

qboolean noclip_anglehack;

static void Host_Noclip_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (PR_GLOBAL_STRUCT(deathmatch) || PR_GLOBAL_STRUCT(coop) || skill.value > 2)
		return;

	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		noclip_anglehack = true;
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		SV_ClientPrintf ("noclip ON\n");
	}
	else
	{
		noclip_anglehack = false;
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("noclip OFF\n");
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
	float	total;
	client_t	*client;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	SV_ClientPrintf ("Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total+=client->ping_times[j];
		total /= NUM_PING_TIMES;
		SV_ClientPrintf ("%4i %s\n", (int)(total*1000), client->name);
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
	int		i;
	char	name[MAX_QPATH];

	if (Cmd_Argc() < 2)	//no map name given
	{
		Con_Printf ("map <levelname>: start a new server\nCurrently on: %s\n",cl.levelname);
		Con_Printf ("%s\n",cls.mapstring);
		return;
	}

	if (cmd_source != src_command)
		return;

	cls.demonum = -1;		// stop demo loop in case this fails

	CL_Disconnect ();
	Host_ShutdownServer(false);

	key_dest = key_game;		// remove console or menu
	SCR_BeginLoadingPlaque ();

	info_mask = 0;
	if (!coop.value && deathmatch.value)
		info_mask2 = 0x80000000;
	else
		info_mask2 = 0;

	cls.mapstring[0] = 0;
	for (i=0 ; i<Cmd_Argc() ; i++)
	{
		Q_strlcat (cls.mapstring, Cmd_Argv(i), MAX_QPATH);
		Q_strlcat (cls.mapstring, " ", MAX_QPATH);
	}
	Q_strlcat (cls.mapstring, "\n", MAX_QPATH);

	svs.serverflags = 0;		// haven't completed an episode yet
	Q_strlcpy (name, Cmd_Argv(1), sizeof(name));

	SV_SpawnServer (name, NULL);

	if (!sv.active)
		return;

	if (cls.state != ca_dedicated)
	{
		loading_stage = 2;

		memset (cls.spawnparms, 0, MAX_MAPSTRING);
		for (i=2 ; i<Cmd_Argc() ; i++)
		{
			Q_strlcat (cls.spawnparms, Cmd_Argv(i), MAX_MAPSTRING);
			Q_strlcat (cls.spawnparms, " ", MAX_MAPSTRING);
		}

		Cmd_ExecuteString ("connect local", src_command);
	}
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
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}

	Q_strlcpy (level, Cmd_Argv(1), sizeof(level));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		Q_strlcpy (_startspot, Cmd_Argv(2), sizeof(_startspot));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();
	SV_SpawnServer (level, startspot);

	//updatePlaqueMessage();
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

	if (cls.demoplayback || !sv.active)
		return;

	if (cmd_source != src_command)
		return;

	Q_strlcpy (mapname, sv.name, sizeof(mapname));	// must copy out, because it gets cleared
	Q_strlcpy (startspot, sv.startspot, sizeof(startspot));

	if (Cmd_Argc() == 2 && Q_strcasecmp(Cmd_Argv(1),"restore") == 0)
	{
		if (LoadGamestate (mapname, startspot, 3))
		{
			SV_SpawnServer (mapname, startspot);
			RestoreClients();
		}
	}
	else
	{
		// in sv_spawnserver
		SV_SpawnServer (mapname, startspot);
	}

//	updatePlaqueMessage();
}

/*
==================
Host_Reconnect_f

This command causes the client to wait for the signon messages again.
This is sent just before a server changes levels
==================
*/
extern void R_ClearParticles (void);
extern qboolean		demohack;

static void Host_Reconnect_f (void)
{
	R_ClearParticles ();	//jfm: for restarts which didn't use to clear parts.
	if (demohack)
	{
		demohack = false;
		Cbuf_AddText("-attack\n");
	}
	if (oem.value && cl.intermission == 9)
	{
		CL_Disconnect();
		return;
	}

	//updatePlaqueMessage();

	SCR_BeginLoadingPlaque ();
	cls.signon = 0;		// need new connection messages
}

/*
=====================
Host_Connect_f

User command to connect to server
=====================
*/
static void Host_Connect_f (void)
{
	char	name[MAX_QPATH];

	cls.demonum = -1;		// stop demo loop in case this fails
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
		CL_Disconnect ();
	}
	Q_strlcpy (name, Cmd_Argv(1), sizeof(name));
	CL_EstablishConnection (name);
	Host_Reconnect_f ();
}


/*
===============================================================================

LOAD / SAVE GAME

===============================================================================
*/

#define	SAVEGAME_VERSION	5

#define ShortTime "%m/%d/%Y %H:%M"

static char	savename[MAX_OSPATH], savedest[MAX_OSPATH];

/*
===============
Host_SavegameComment

Writes a SAVEGAME_COMMENT_LENGTH character comment describing the game saved
===============
*/
static void Host_SavegameComment (char *text)
{
	int		i;
	char	kills[20];
	struct tm *tblock;
	time_t TempTime;

	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		text[i] = ' ';
	memcpy (text, cl.levelname, strlen(cl.levelname));
//	sprintf (kills,"kills:%3i/%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);

	TempTime = time(NULL);
	tblock = localtime(&TempTime);
	strftime(kills,sizeof(kills),ShortTime,tblock);

	memcpy (text+21, kills, strlen(kills));
// convert space to _ to make stdio happy
	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		if (text[i] == ' ')
			text[i] = '_';

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
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];
	qboolean error_state = false;
	int attempts = 0;
	char *message;

	if (cmd_source != src_command)
		return;

	if (!sv.active)
	{
		Con_Printf ("Not playing a local game.\n");
		return;
	}

	if (cl.intermission)
	{
		Con_Printf ("Can't save in intermission.\n");
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

	if (strstr(Cmd_Argv(1), ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			Con_Printf ("Can't savegame with a dead player\n");
			return;
		}
	}

	error_state = SaveGamestate(false);
	// don't bother doing more if SaveGamestate failed
	if (error_state)
		return;

retry:
	attempts++;

	if (snprintf(savename, sizeof(savename), "%s/%s", com_savedir, Cmd_Argv(1)) >= sizeof(savename))
	{
		Con_Printf ("%s: save directory name too long\n", __FUNCTION__);
		return;
	}
	if (Sys_mkdir(savename))
	{
		Con_Printf ("Unable to create save directory\n");
		return;
	}

	Host_RemoveGIPFiles(savename);

	snprintf (savename, sizeof(savename), "%s/clients.gip", com_savedir);
	unlink(savename);

	snprintf (savedest, sizeof(savedest), "%s/%s", com_savedir, Cmd_Argv(1));
	Con_Printf ("Saving game to %s...\n", savedest);

	error_state = Host_CopyFiles(com_savedir, "*.gip", savedest);
	if (error_state)
		goto retrymsg;

	if (snprintf(savedest, sizeof(savedest), "%s/%s/info.dat", com_savedir, Cmd_Argv(1)) >= sizeof(savedest))
	{
		Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
		return;
	}
	f = fopen (savedest, "w");
	if (!f)
	{
		error_state = true;
		goto retrymsg;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%d\n", current_skill);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n",sv.time);
	fprintf (f, "%d\n",svs.maxclients);
	fprintf (f, "%f\n",deathmatch.value);
	fprintf (f, "%f\n",coop.value);
	fprintf (f, "%f\n",teamplay.value);
	fprintf (f, "%f\n",randomclass.value);
	fprintf (f, "%f\n",cl_playerclass.value);
	// mission pack, objectives strings
	fprintf (f, "%d\n",info_mask);
	fprintf (f, "%d\n",info_mask2);
	if (ferror(f))
		error_state = true;

	fclose(f);

retrymsg:
	if (error_state)
	{
		if (attempts == 1)
			message = "The game could not be saved properly.  You may be out of hard drive space!  You can ALT-TAB out to try and free up some space.  Type 'Y' if you want to try and re-save the game, otherwise 'N' to ignore.";
		else
			message = "The game could not be saved properly on the previous attempt.  You may be out of hard drive space!  You can ALT-TAB out to try and free up some space.  Type 'Y' if you want to try and re-save the game, otherwise 'N' to ignore.";

		key_lastpress = 0;
		if (SCR_ModalMessage(message))
		{
			goto retry;
		}
	}
}


/*
===============
Host_Loadgame_f
===============
*/
static void Host_Loadgame_f (void)
{
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	playtime, tfloat;
	char	str[32768];
	int	i;
	edict_t	*ent;
	int	version;
	float	tempf;
	int	tempi;
	float	spawn_parms[NUM_SPAWN_PARMS];
	qboolean error_state = false;
	int	attempts = 0;
	char *message;

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("load <savename> : load a game\n");
		return;
	}

	cls.demonum = -1;		// stop demo loop in case this fails
	CL_Disconnect();
	Host_RemoveGIPFiles(NULL);

	if (snprintf(savename, sizeof(savename), "%s/%s", com_savedir, Cmd_Argv(1)) >= sizeof(savename))
	{
		Con_Printf ("%s: save directory name too long\n", __FUNCTION__);
		return;
	}
	Con_Printf ("Loading game from %s...\n", savename);

	if (snprintf(savedest, sizeof(savedest), "%s/info.dat", savename) >= sizeof(savedest))
	{
		Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
		return;
	}

	f = fopen (savedest, "r");
	if (!f)
	{
		Con_Printf ("%s: ERROR: couldn't open savefile\n", __FUNCTION__);
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
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);
// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tfloat);
	current_skill = (int)(tfloat + 0.1);
	Cvar_SetValue ("skill", (float)current_skill);

	Cvar_SetValue ("deathmatch", 0);
	Cvar_SetValue ("coop", 0);
	Cvar_SetValue ("teamplay", 0);
	Cvar_SetValue ("randomclass", 0);

	fscanf (f, "%s\n",mapname);
	fscanf (f, "%f\n",&playtime);

	tempi = -1;
	fscanf (f, "%d\n",&tempi);
	if (tempi >= 1)
		svs.maxclients = tempi;

	tempf = -1;
	fscanf (f, "%f\n",&tempf);
	if (tempf >= 0)
		Cvar_SetValue ("deathmatch", tempf);

	tempf = -1;
	fscanf (f, "%f\n",&tempf);
	if (tempf >= 0)
		Cvar_SetValue ("coop", tempf);

	tempf = -1;
	fscanf (f, "%f\n",&tempf);
	if (tempf >= 0)
		Cvar_SetValue ("teamplay", tempf);

	tempf = -1;
	fscanf (f, "%f\n",&tempf);
	if (tempf >= 0)
		Cvar_SetValue ("randomclass", tempf);

	tempf = -1;
	fscanf (f, "%f\n",&tempf);
	if (tempf >= 0)
		Cvar_SetValue ("_cl_playerclass", tempf);

	// mission pack, objectives strings
	fscanf (f, "%d\n",&info_mask);
	fscanf (f, "%d\n",&info_mask2);

	fclose (f);

	Host_RemoveGIPFiles(com_savedir);

retry:
	attempts++;

	snprintf (savedest, sizeof(savedest), "%s/%s", com_savedir, Cmd_Argv(1));
	error_state = Host_CopyFiles(savedest, "*.gip", com_savedir);

	if (error_state)
	{
		if (attempts == 1)
			message = "The game could not be loaded properly.  You may be out of hard drive space!  You can ALT-TAB out to try and free up some space.  Type 'Y' if you want to try and re-load the game, otherwise 'N' to abort.";
		else
			message = "The game could not be loaded properly on the previous attempt.  You may be out of hard drive space!  You can ALT-TAB out to try and free up some space.  Type 'Y' if you want to try and re-load the game, otherwise 'N' to abort.";

		key_lastpress = 0;
		if (SCR_ModalMessage(message))
		{
			goto retry;
		}
		else
			return;
	}

	LoadGamestate (mapname, NULL, 2);

	SV_SaveSpawnparms ();

	ent = EDICT_NUM(1);

	Cvar_SetValue ("_cl_playerclass", ent->v.playerclass);//this better be the same as above...

	// this may be rudundant with the setting in PR_LoadProgs, but not sure so its here too
	if (progs->crc == PROGS_V112_CRC)
		pr_global_struct->cl_playerclass = ent->v.playerclass;

	svs.clients->playerclass = ent->v.playerclass;

	sv.paused = true;		// pause until all clients connect
	sv.loadgame = true;

	if (cls.state != ca_dedicated)
	{
		CL_EstablishConnection ("local");
		Host_Reconnect_f ();
	}
}

qboolean SaveGamestate(qboolean ClientsOnly)
{
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];
	edict_t	*ent;
	int start,end;
	qboolean error_state = false;
	int attempts = 0;
	char *message;

retry:
	attempts++;

	if (ClientsOnly)
	{
		start = 1;
		end = svs.maxclients+1;

		if (snprintf(savename, sizeof(savename), "%s/clients.gip", com_savedir) >= sizeof(savename))
		{
			Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
			return true;
		}
	}
	else
	{
		start = 1;
		end = sv.num_edicts;

		if (snprintf(savename, sizeof(savename), "%s/%s.gip", com_savedir, sv.name) >= sizeof(savename))
		{
			Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
			return true;
		}
	}

	f = fopen (savename, "w");
	if (!f)
	{
		error_state = true;
		goto retrymsg;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);

	if (!ClientsOnly)
	{
		Host_SavegameComment (comment);
		fprintf (f, "%s\n", comment);
	//	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
	//		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
		fprintf (f, "%f\n", skill.value);
		fprintf (f, "%s\n", sv.name);
		fprintf (f, "%f\n", sv.time);

// mission pack, objectives strings
//		fprintf (f, "%d\n", info_mask);
//		fprintf (f, "%d\n", info_mask2);

	// write the light styles

		for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
		{
			if (sv.lightstyles[i])
				fprintf (f, "%s\n", sv.lightstyles[i]);
			else
				fprintf (f,"m\n");
		}
		SV_SaveEffects(f);
		fprintf(f,"-1\n");
		ED_WriteGlobals (f);
	}
	else
	{
// mission pack, objectives strings
//		fprintf(f, "%d\n", info_mask);
//		fprintf(f, "%d\n", info_mask2);
	}

	host_client = svs.clients;

//	for (i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
//  to save the client states
	for (i=start ; i<end ; i++)
	{
		ent = EDICT_NUM(i);
		if ((int)ent->v.flags & FL_ARCHIVE_OVERRIDE)
			continue;
		if (ClientsOnly)
		{
			if (host_client->active)
			{
				fprintf (f, "%i\n",i);
				ED_Write (f, ent);
				fflush (f);
			}
			host_client++;
		}
		else
		{
			fprintf (f, "%i\n",i);
			ED_Write (f, ent);
			fflush (f);
		}
	}

	if (ferror(f))
	{
		error_state = true;
	}
	fclose (f);

retrymsg:
	if (error_state)
	{
		if (attempts == 1)
			message = "The level could not be saved properly.  You may be out of hard drive space!  You can ALT-TAB out to try and free up some space.  Type 'Y' if you want to try and re-save the level, otherwise 'N' to ignore.";
		else
			message = "The level could not be saved properly on the previous attempt.  You may be out of hard drive space!  You can ALT-TAB out to try and free up some space.  Type 'Y' if you want to try and re-save the level, otherwise 'N' to ignore.";

		key_lastpress = 0;
		if (SCR_ModalMessage(message))
		{
			goto retry;
		}
	}

	return error_state;
}

static void RestoreClients(void)
{
	int i,j;
	edict_t	*ent;
	double time_diff;

	if (LoadGamestate(NULL,NULL,1))
		return;

	time_diff = sv.time - old_time;

	for (i = 0, host_client = svs.clients ; i < svs.maxclients ; i++, host_client++)
	{
		if (host_client->active)
		{
			ent = host_client->edict;

			//ent->v.colormap = NUM_FOR_EDICT(ent);
			ent->v.team = (host_client->colors & 15) + 1;
			ent->v.netname = host_client->name - pr_strings;
			ent->v.playerclass = host_client->playerclass;


			if (old_progdefs)
			{
			// copy spawn parms out of the client_t
				for (j = 0 ; j < NUM_SPAWN_PARMS ; j++)
					(&pr_global_struct_v111->parm1)[j] = host_client->spawn_parms[j];
			// call the spawn function
				pr_global_struct_v111->time = sv.time;
				pr_global_struct_v111->self = EDICT_TO_PROG(ent);
				G_FLOAT(OFS_PARM0) = time_diff;
				PR_ExecuteProgram (pr_global_struct_v111->ClientReEnter);
			}
			else
			{
			// copy spawn parms out of the client_t
				for (j = 0 ; j < NUM_SPAWN_PARMS ; j++)
					(&pr_global_struct->parm1)[j] = host_client->spawn_parms[j];
			// call the spawn function
				pr_global_struct->time = sv.time;
				pr_global_struct->self = EDICT_TO_PROG(ent);
				G_FLOAT(OFS_PARM0) = time_diff;
				PR_ExecuteProgram (pr_global_struct->ClientReEnter);
			}
		}
	}

	SaveGamestate(true);
}

static int LoadGamestate(char *level, char *startspot, int ClientsMode)
{
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	playtime, sk;
	char	str[32768], *start;
	int		i, r;
	edict_t	*ent;
	int		entnum;
	int		version;
//	float	spawn_parms[NUM_SPAWN_PARMS];
	qboolean auto_correct = false;

	if (ClientsMode == 1)
	{
		if (snprintf(savename, sizeof(savename), "%s/clients.gip", com_savedir) >= sizeof(savename))
		{
			Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
			return -1;
		}
	}
	else
	{
		if (snprintf(savename, sizeof(savename), "%s/%s.gip", com_savedir, level) >= sizeof(savename))
		{
			Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
			return -1;
		}

		if (ClientsMode != 2 && ClientsMode != 3)
			Con_Printf ("Loading game from %s...\n", savename);
	}

	f = fopen (savename, "r");
	if (!f)
	{
		if (ClientsMode == 2)
			Con_Printf ("%s: ERROR: couldn't open savefile\n", __FUNCTION__);

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
	//	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
	//		fscanf (f, "%f\n", &spawn_parms[i]);
		fscanf (f, "%f\n", &sk);
		Cvar_SetValue ("skill", sk);

		fscanf (f, "%s\n",mapname);
		fscanf (f, "%f\n",&playtime);

		SV_SpawnServer (mapname, startspot);

		if (!sv.active)
		{
			Con_Printf ("Couldn't load map\n");
			return -1;
		}

// mission pack, objectives strings
//		fscanf (f, "%d\n",&info_mask);
//		fscanf (f, "%d\n",&info_mask2);

	// load the light styles
		for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
		{
			fscanf (f, "%s\n", str);
			sv.lightstyles[i] = Hunk_AllocName (strlen(str)+1, "lightstyles");
			strcpy (sv.lightstyles[i], str);
		}
		SV_LoadEffects(f);
	}

// load the edicts out of the savegame file
	while (!feof(f))
	{
		fscanf (f, "%i\n",&entnum);
		for (i=0 ; i<sizeof(str)-1 ; i++)
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
		if (i == sizeof(str)-1)
			Sys_Error ("Loadgame buffer overflow");
		str[i] = 0;
		start = str;
		start = COM_Parse(str);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token,"{"))
			Sys_Error ("First token isn't a brace");

		// parse an edict

		if (entnum == -1)
		{
			ED_ParseGlobals (start);
			// Need to restore this
			if (old_progdefs)
				pr_global_struct_v111->startspot = sv.startspot - pr_strings;
			else
				pr_global_struct->startspot = sv.startspot - pr_strings;
		}
		else
		{
			ent = EDICT_NUM(entnum);
			memset (&ent->v, 0, progs->entityfields * 4);
			//ent->free = false;
			ED_ParseEdict (start, ent);

			if (ClientsMode == 1 || ClientsMode == 2 || ClientsMode == 3)
				ent->v.stats_restored = true;

			// link it into the bsp tree
			if (!ent->free)
			{
				SV_LinkEdict (ent, false);
				if (ent->v.modelindex && ent->v.model)
				{
					i = SV_ModelIndex(ent->v.model + pr_strings);
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

//	sv.num_edicts = entnum;
	if (ClientsMode == 0)
	{
		sv.time = playtime;
		sv.paused = true;

		if (old_progdefs)
			pr_global_struct_v111->serverflags = svs.serverflags;
		else
			pr_global_struct->serverflags = svs.serverflags;

		RestoreClients();
	}
	else if (ClientsMode == 2)
	{
		sv.time = playtime;
	}
	else if (ClientsMode == 3)
	{
		sv.time = playtime;

		if (old_progdefs)
			pr_global_struct_v111->serverflags = svs.serverflags;
		else
			pr_global_struct->serverflags = svs.serverflags;

		RestoreClients();
	}

	if (ClientsMode != 1 && auto_correct)
	{
		Con_DPrintf("*** Auto-corrected model indexes!\n");
	}

//	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
//		svs.clients->spawn_parms[i] = spawn_parms[i];

	return 0;
}

// changing levels within a unit
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
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}

	Q_strlcpy (level, Cmd_Argv(1), sizeof(level));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		Q_strlcpy (_startspot, Cmd_Argv(2), sizeof(_startspot));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();

	// save the current level's state
	old_time = sv.time;
	SaveGamestate (false);

	// try to restore the new level
	if (LoadGamestate (level, startspot, 0))
	{
		SV_SpawnServer (level, startspot);
		RestoreClients();
	}
}


//============================================================================

/*
======================
Host_Name_f
======================
*/
static void Host_Name_f (void)
{
	char	*newName;
	char	*pdest;

	if (Cmd_Argc () == 1)
	{
		Con_Printf ("\"name\" is \"%s\"\n", cl_name.string);
		return;
	}
	if (Cmd_Argc () == 2)
		newName = Cmd_Argv(1);
	else
		newName = Cmd_Args();
	newName[15] = 0;	// client_t structure actually says name[32].

	//this is for the fuckers who put braces in the name causing loadgame to crash.
	pdest = strchr(newName,'{');
	if (pdest)
	{
		*pdest = 0;	//zap the brace
		Con_Printf ("Illegal char in name removed!\n");
	}

	if (cmd_source == src_command)
	{
		if (strcmp(cl_name.string, newName) == 0)
			return;
		Cvar_Set ("_cl_name", newName);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
	}

	if (host_client->name[0] && strcmp(host_client->name, "unconnected") )
		if (strcmp(host_client->name, newName) != 0)
			Con_Printf ("%s renamed to %s\n", host_client->name, newName);
	strcpy (host_client->name, newName);
	host_client->edict->v.netname = host_client->name - pr_strings;

// send notification to all clients

	MSG_WriteByte (&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteString (&sv.reliable_datagram, host_client->name);
}

extern char *ClassNames[MAX_PLAYER_CLASS];	//from menu.c
static void Host_Class_f (void)
{
	float	newClass;

	if (Cmd_Argc () == 1)
	{
		if (!(int)cl_playerclass.value);
		else
			Con_Printf ("\"playerclass\" is %d (\"%s\")\n", (int)cl_playerclass.value,ClassNames[(int)cl_playerclass.value-1]);
		return;
	}
	if (Cmd_Argc () == 2)
		newClass = atof(Cmd_Argv(1));
	else
		newClass = atof(Cmd_Args());

//	if (!registered.value && !oem.value && (newClass == CLASS_CLERIC || newClass == CLASS_NECROMANCER))
//	{
//		Con_Printf("That class is not available in the demo version.\n");
//		return;
//	}

	if ((newClass == CLASS_DEMON) && !(gameflags & GAME_PORTALS))
	{
		Con_Printf("That class is only available in the mission pack.\n");
		return;
	}

	if (cmd_source == src_command)
	{
		Cvar_SetValue ("_cl_playerclass", newClass);

		// when classes changes after map load, update cl_playerclass, cl_playerclass should 
		// probably only be used in worldspawn, though
		if (pr_global_struct && (progs->crc == PROGS_V112_CRC))
			pr_global_struct->cl_playerclass = newClass;

		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
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
	if (old_progdefs)
	{
		pr_global_struct_v111->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram (pr_global_struct_v111->ClassChangeWeapon);
	}
	else
	{
		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram (pr_global_struct->ClassChangeWeapon);
	}

// send notification to all clients

	MSG_WriteByte (&sv.reliable_datagram, svc_updateclass);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, (byte)newClass);
}

//just an easy place to do some profile testing
#if 0
static void Host_Version_f (void)
{
	int		i;
	int		repcount = 10000;
	float	time1, time2, r1, r2;

	if (Cmd_Argc() == 2)
	{
		repcount = atof(Cmd_Argv(1));
		if (repcount < 0)
			repcount =0;
	}
	Con_Printf ("looping %d times.\n", repcount);

	time1 = Sys_DoubleTime();
	for (i=repcount;i;i--)
	{
		char buf[2048];
		memset(buf,i,2048);
	}
	time2 = Sys_DoubleTime();
	r1 = time2-time1;
	Con_Printf ("loop 1 = %f\n", r1);

	time1 = Sys_DoubleTime();
	for (i=repcount;i;i--)
	{
		char buf[2048];
		memset(buf,i,2048);
	}
	time2 = Sys_DoubleTime();
	r2 = time2-time1;
	Con_Printf ("loop 2 = %f\n", r2);

	if (r2 < r1)
	{
		Con_Printf ("loop 2 is faster by %f\n", r1-r2);
	}
	else
	{
		Con_Printf ("loop 1 is faster by %f\n", r2-r1);
	}
	Con_Printf ("Version %4.2f\n", ENGINE_VERSION);
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
}
#else
static void Host_Version_f (void)
{
	Con_Printf ("Version %4.2f\n", ENGINE_VERSION);
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
}
#endif

static void Host_Say(qboolean teamonly)
{
	client_t *client;
	client_t *save;
	int		j;
	char	*p;
	char	text[64];
	qboolean	fromServer = false;

	if (cmd_source == src_command)
	{
		if (cls.state == ca_dedicated)
		{
			fromServer = true;
			teamonly = false;
		}
		else
		{
			Cmd_ForwardToServer ();
			return;
		}
	}

	if (Cmd_Argc () < 2)
		return;

	save = host_client;

	p = Cmd_Args();
// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// turn on color set 1
	if (!fromServer)
		snprintf (text, sizeof(text), "%c%s: ", 1, save->name);
	else
		snprintf (text, sizeof(text), "%c<%s> ", 1, hostname.string);

	Q_strlcat (text, p, sizeof(text));
	if (Q_strlcat (text, "\n", sizeof(text)) >= sizeof(text))
		text[sizeof(text)-2] = '\n';

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;
		if (teamplay.value && teamonly && client->edict->v.team != save->edict->v.team)
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
	}
	host_client = save;

	Sys_Printf("%s", &text[1]);
}


static void Host_Say_f(void)
{
	Host_Say(false);
}


static void Host_Say_Team_f(void)
{
	Host_Say(true);
}


static void Host_Tell_f(void)
{
	client_t *client;
	client_t *save;
	int		j;
	char	*p;
	char	text[64];

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (Cmd_Argc () < 3)
		return;

	Q_strlcpy(text, host_client->name, sizeof(text));
	Q_strlcat(text, ": ", sizeof(text));

	p = Cmd_Args();

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// check length & truncate if necessary
	Q_strlcat (text, p, sizeof(text));
	if (Q_strlcat (text, "\n", sizeof(text)) >= sizeof(text))
		text[sizeof(text)-2] = '\n';

	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;

		if (Q_strcasecmp(client->name, Cmd_Argv(1)))
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
		break;
	}
	host_client = save;
}


/*
==================
Host_Color_f
==================
*/
static void Host_Color_f(void)
{
	int		top, bottom;
	int		plyrcolor;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"color\" is \"%i %i\"\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 0x0f);
		Con_Printf ("color <0-10> [0-10]\n");
		return;
	}

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

	plyrcolor = top*16 + bottom;

	if (cmd_source == src_command)
	{
		Cvar_SetValue ("_cl_color", plyrcolor);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
	}

	host_client->colors = plyrcolor;
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
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (sv_player->v.health <= 0 && sv_player->v.deadflag != DEAD_NO)
	{
		SV_ClientPrintf ("Can't suicide -- already dead!\n");
		return;
	}

	if (old_progdefs)
	{
		pr_global_struct_v111->time = sv.time;
		pr_global_struct_v111->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct_v111->ClientKill);
	}
	else
	{
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientKill);
	}
}


/*
==================
Host_Pause_f
==================
*/
static void Host_Pause_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}
	if (!pausable.value)
		SV_ClientPrintf ("Pause not allowed.\n");
	else
	{
		sv.paused ^= 1;

		// release and grab mouse with pause S.A
		if (sv.paused)
		{
			SV_BroadcastPrintf ("%s paused the game\n", pr_strings + sv_player->v.netname);
			IN_DeactivateMouse ();
		}
		else
		{
			SV_BroadcastPrintf ("%s unpaused the game\n",pr_strings + sv_player->v.netname);
			IN_ActivateMouse ();
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
	edict_t	*ent;

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

		if (!ent->v.stats_restored || deathmatch.value)
		{
			memset (&ent->v, 0, progs->entityfields * 4);

			//ent->v.colormap = NUM_FOR_EDICT(ent);
			ent->v.team = (host_client->colors & 15) + 1;
			ent->v.netname = host_client->name - pr_strings;
			ent->v.playerclass = host_client->playerclass;

			if (old_progdefs)
			{
			// copy spawn parms out of the client_t
				for (i=0 ; i < NUM_SPAWN_PARMS ; i++)
					(&pr_global_struct_v111->parm1)[i] = host_client->spawn_parms[i];
			// call the spawn function
				pr_global_struct_v111->time = sv.time;
				pr_global_struct_v111->self = EDICT_TO_PROG(sv_player);
				PR_ExecuteProgram (pr_global_struct_v111->ClientConnect);
			}
			else
			{
			// copy spawn parms out of the client_t
				for (i=0 ; i < NUM_SPAWN_PARMS ; i++)
					(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];
			// call the spawn function
				pr_global_struct->time = sv.time;
				pr_global_struct->self = EDICT_TO_PROG(sv_player);
				PR_ExecuteProgram (pr_global_struct->ClientConnect);
			}

			if ((Sys_DoubleTime() - host_client->netconnection->connecttime) <= sv.time)
				Sys_Printf ("%s entered the game\n", host_client->name);

			PR_ExecuteProgram (PR_GLOBAL_STRUCT(PutClientInServer));
		}
	}

// send time of update
	MSG_WriteByte (&host_client->message, svc_time);
	MSG_WriteFloat (&host_client->message, sv.time);

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
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
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
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
	MSG_WriteLong (&host_client->message, PR_GLOBAL_STRUCT(total_secrets));

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL_STRUCT(total_monsters));

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL_STRUCT(found_secrets));

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL_STRUCT(killed_monsters));

	SV_UpdateEffects(&host_client->message);

//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	MSG_WriteByte (&host_client->message, svc_setangle);
	for (i=0 ; i < 2 ; i++)
		MSG_WriteAngle (&host_client->message, ent->v.angles[i] );
	MSG_WriteAngle (&host_client->message, 0);

	SV_WriteClientdataToMessage (host_client, sv_player, &host_client->message);

	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 3);
	host_client->sendsignon = true;
}

dfunction_t *ED_FindFunctioni (char *fn_name);

extern char	key_lines[32][MAXCMDLINE];
extern int		key_linepos;
extern int		edit_line;

static int strdiff(char *s1, char *s2)
{
	int	L1, L2, i;

	L1 = strlen(s1);
	L2 = strlen(s2);

	for(i=0;(i < L1 && i < L2);i++)
	{
		if (tolower(s1[i]) != tolower(s2[i]))
			break;
	}

	return i;
}

static void Host_Create_f(void)
{
	char	*FindName;
	dfunction_t	*Search, *func;
	edict_t		*ent;
	int		i,fLength,NumFound,Diff,NewDiff;

	if (!sv.active)
	{
		Con_Printf("server is not active!\n");
		return;
	}

	if ((svs.maxclients != 1) || (skill.value >2))
	{
		Con_Printf("can't cheat anymore!\n");
		return;
	}

	if (Cmd_Argc () == 1)
	{
		Con_Printf("create <quake-ed spawn function>\n");
		return;
	}

	FindName = Cmd_Argv(1);

	func = ED_FindFunctioni ( FindName );

	if (!func)
	{
		fLength = strlen(FindName);
		NumFound = 0;

		Diff = 999;

		for (i=0 ; i<progs->numfunctions ; i++)
		{
			Search = &pr_functions[i];
			if (!Q_strncasecmp(pr_strings + Search->s_name,FindName,fLength) )
			{
				if (NumFound == 1)
				{
					Con_Printf("   %s\n",pr_strings+func->s_name);
				}
				if (NumFound)
				{
					Con_Printf("   %s\n",pr_strings+Search->s_name);
					NewDiff = strdiff(pr_strings+Search->s_name,pr_strings+func->s_name);
					if (NewDiff < Diff)
						Diff = NewDiff;
				}

				func = Search;
				NumFound++;
			}
		}

		if (!NumFound)
		{
			Con_Printf("Could not find spawn function\n");
			return;
		}

		if (NumFound != 1)
		{
			snprintf(key_lines[edit_line], MAXCMDLINE, ">create %s", func->s_name+pr_strings);
			key_lines[edit_line][Diff+8] = 0;
			key_linepos = strlen(key_lines[edit_line]);
			return;
		}
	}

	Con_Printf("Executing %s...\n",pr_strings+func->s_name);

	ent = ED_Alloc ();

	ent->v.classname = func->s_name;
	VectorCopy(r_origin,ent->v.origin);
	ent->v.origin[0] += vpn[0] * 80;
	ent->v.origin[1] += vpn[1] * 80;
	ent->v.origin[2] += vpn[2] * 80;
	VectorCopy(ent->v.origin,ent->v.absmin);
	VectorCopy(ent->v.origin,ent->v.absmax);
	ent->v.absmin[0] -= 16;
	ent->v.absmin[1] -= 16;
	ent->v.absmin[2] -= 16;
	ent->v.absmax[0] += 16;
	ent->v.absmax[1] += 16;
	ent->v.absmax[2] += 16;

	if (old_progdefs)
		pr_global_struct_v111->self = EDICT_TO_PROG(ent);
	else
		pr_global_struct->self = EDICT_TO_PROG(ent);
	ignore_precache = true;
	PR_ExecuteProgram (func - pr_functions);
	ignore_precache = false;
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
	char		*who;
	char		*message = NULL;
	client_t	*save;
	int			i;
	qboolean	byNumber = false;

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
	}
	else if (PR_GLOBAL_STRUCT(deathmatch))
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
			if (Q_strcasecmp(host_client->name, Cmd_Argv(1)) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (cmd_source == src_command)
		{
			if (cls.state == ca_dedicated)
				who = "Console";
			else
				who = cl_name.string;
		}
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
			SV_ClientPrintf ("Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf ("Kicked by %s\n", who);
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
	char	*t;
	int	v;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (PR_GLOBAL_STRUCT(deathmatch) || skill.value > 2)
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

static edict_t *FindViewthing (void)
{
	int		i;
	edict_t	*e;

	for (i=0 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		if ( !strcmp (pr_strings + e->v.classname, "viewthing") )
			return e;
	}
	Con_Printf ("No viewthing on map\n");
	return NULL;
}

/*
==================
Host_Viewmodel_f
==================
*/
static void Host_Viewmodel_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = Mod_ForName (Cmd_Argv(1), false);
	if (!m)
	{
		Con_Printf ("Can't load %s\n", Cmd_Argv(1));
		return;
	}

	e->v.frame = 0;
	cl.model_precache[(int)e->v.modelindex] = m;
}

/*
==================
Host_Viewframe_f
==================
*/
static void Host_Viewframe_f (void)
{
	edict_t	*e;
	int		f;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	f = atoi(Cmd_Argv(1));
	if (f >= m->numframes)
		f = m->numframes-1;

	e->v.frame = f;
}


static void PrintFrameName (model_t *m, int frame)
{
	aliashdr_t 			*hdr;
	maliasframedesc_t	*pframedesc;

	hdr = (aliashdr_t *)Mod_Extradata (m);
	if (!hdr)
		return;
	pframedesc = &hdr->frames[frame];

	Con_Printf ("frame %i: %s\n", frame, pframedesc->name);
}

/*
==================
Host_Viewnext_f
==================
*/
static void Host_Viewnext_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame + 1;
	if (e->v.frame >= m->numframes)
		e->v.frame = m->numframes - 1;

	PrintFrameName (m, e->v.frame);
}

/*
==================
Host_Viewprev_f
==================
*/
static void Host_Viewprev_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame - 1;
	if (e->v.frame < 0)
		e->v.frame = 0;

	PrintFrameName (m, e->v.frame);
}

/*
===============================================================================

DEMO LOOP CONTROL

===============================================================================
*/


/*
==================
Host_Startdemos_f
==================
*/
static void Host_Startdemos_f (void)
{
	int		i, c;

	if (cls.state == ca_dedicated)
	{
	//	if (!sv.active)
	//		Cbuf_AddText ("map start\n");
		return;
	}

	c = Cmd_Argc() - 1;
	if (c > MAX_DEMOS)
	{
		Con_Printf ("Max %i demos in demoloop\n", MAX_DEMOS);
		c = MAX_DEMOS;
	}
	Con_Printf ("%i demo(s) in loop\n", c);

	for (i=1 ; i<c+1 ; i++)
		strncpy (cls.demos[i-1], Cmd_Argv(i), sizeof(cls.demos[0])-1);

	if (!sv.active && cls.demonum != -1 && !cls.demoplayback)
	{
		cls.demonum = 0;
		CL_NextDemo ();
	}
	else
		cls.demonum = -1;
}


/*
==================
Host_Demos_f

Return to looping demos
==================
*/
static void Host_Demos_f (void)
{
	if (cls.state == ca_dedicated)
		return;
	if (cls.demonum == -1)
		cls.demonum = 1;
	CL_Disconnect_f ();
	CL_NextDemo ();
}

/*
==================
Host_Stopdemo_f

Return to looping demos
==================
*/
static void Host_Stopdemo_f (void)
{
	if (cls.state == ca_dedicated)
		return;
	if (!cls.demoplayback)
		return;
	CL_StopPlayback ();
	CL_Disconnect ();
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
	Cmd_AddCommand ("connect", Host_Connect_f);
	Cmd_AddCommand ("reconnect", Host_Reconnect_f);
	Cmd_AddCommand ("name", Host_Name_f);
	Cmd_AddCommand ("playerclass", Host_Class_f);
	Cmd_AddCommand ("noclip", Host_Noclip_f);
	Cmd_AddCommand ("version", Host_Version_f);
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
	Cmd_AddCommand ("give", Host_Give_f);

	Cmd_AddCommand ("startdemos", Host_Startdemos_f);
	Cmd_AddCommand ("demos", Host_Demos_f);
	Cmd_AddCommand ("stopdemo", Host_Stopdemo_f);

	Cmd_AddCommand ("viewmodel", Host_Viewmodel_f);
	Cmd_AddCommand ("viewframe", Host_Viewframe_f);
	Cmd_AddCommand ("viewnext", Host_Viewnext_f);
	Cmd_AddCommand ("viewprev", Host_Viewprev_f);

	Cmd_AddCommand ("create", Host_Create_f);
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.51  2006/09/15 09:21:40  sezero
 * made the MAXCMDLINE definition public through keys.h
 *
 * Revision 1.50  2006/09/13 05:53:22  sezero
 * re-visited the includes, gathered all net includes into
 * the new net_sys.h, did a platform defines clean-up.
 *
 * Revision 1.49  2006/07/02 11:36:35  sezero
 * uppercased the pr_global_struct() macro for easier detection
 * and searching. put that macro in use in hexenworld server for
 * smaller diffs between the two versions. there are no actual
 * code changes here, only style and cosmetics.
 *
 * Revision 1.48  2006/06/25 12:01:48  sezero
 * renamed CL_CopyFiles to Host_CopyFiles and CL_RemoveGIPFiles to
 * Host_RemoveGIPFiles, moved them to host.c
 *
 * Revision 1.47  2006/06/25 00:02:54  sezero
 * moved mousestate activation stuff to CL_ParseServerInfo
 *
 * Revision 1.46  2006/06/23 14:45:09  sezero
 * better handling of the +attack/-attack demo recording hack
 *
 * Revision 1.45  2006/06/17 06:04:22  sezero
 * skip intermissions while recording demos across multiple levels,
 * but stop recording at ending scenes.
 *
 * Revision 1.44  2006/05/26 08:21:24  sezero
 * hopefully fixed all linkage errors on MacOSX
 *
 * Revision 1.43  2006/04/05 06:10:43  sezero
 * added support for both hexen2-v1.11 and h2mp-v1.12 progs into a single hexen2
 * binary. this essentially completes the h2/h2mp binary merge started with the
 * previous patch. many conditionals had to be added especially on the server side,but couldn't notice any serious performance loss on a PIII-733 computer. Supportfor multiple progs.dat is now advised to be left enabled in order to support
 * mods which uses that feature.
 *
 * Revision 1.42  2006/04/05 06:09:23  sezero
 * killed (almost) all H2MP ifdefs: this is the first step in making a single
 * binary which handles both h2 and h2mp properly. the only H2MP ifdefs left
 * are actually the ones for determining the icon and window manager text, so
 * nothing serious. the binary normally will only run the original h2 game.
 * if given a -portals or -missionpack or -h2mp argument, it will look for the
 * mission pack and run it (this is the same logic that quake used.) The only
 * serious side effect is that h2 and h2mp progs being different: This will be
 * solved by the next patch by adding support for the two progs versions into
 * a single binary.
 *
 * Revision 1.41  2006/03/21 22:24:08  sezero
 * continue making static functions and vars static. whitespace and coding
 * style cleanup. part 44: model.c, gl_model.c. also moved the mcache cmd
 * addition to Mod_Init and added save-to-file functionality.
 *
 * Revision 1.40  2006/03/17 14:12:48  sezero
 * put back mission-pack only objectives stuff back into pure h2 builds.
 * it was a total screw-up...
 *
 * Revision 1.39  2006/03/13 22:28:51  sezero
 * removed the expansion pack only feature of objective strings from
 * hexen2-only builds (many new ifdef H2MP stuff). removed the expansion
 * pack only intermission picture and string searches from hexen2-only
 * builds.
 *
 * Revision 1.38  2006/02/18 08:51:10  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. also renamed the variables name and dest to savename and savedest in
 * host_cmd.c to prevent any confusion and pollution.
 *
 * Revision 1.37  2006/01/06 12:19:08  sezero
 * put the new Sys_FindFirstFile/Sys_FindNextFile stuff into action. also killed
 * the tempdir and trailing slash funnies in host_cmd.c when calling CL_CopyFiles
 * and CL_RemoveGIPFiles in saving and loading games.
 *
 * Revision 1.36  2005/12/28 14:20:23  sezero
 * made COM_CopyFile return int and added ferror() calls after every fread()
 * and fwrite() calls, so that CL_CopyFiles can behave correctly under unix.
 * made SaveGamestate return qboolean, replaced the silly "ERROR: couldn't
 * open" message by goto retry_message calls. made Host_Savegame_f to return
 * immediately upon SaveGamestate failure.
 *
 * Revision 1.35  2005/11/05 20:22:10  sezero
 * fixed some gcc4 warnings
 *
 * Revision 1.34  2005/11/02 18:44:11  sezero
 * killed a silly quake left-over code in Host_Startdemos_f
 *
 * Revision 1.33  2005/10/29 21:43:22  sezero
 * unified cmd layer
 *
 * Revision 1.32  2005/10/25 17:14:22  sezero
 * added a STRINGIFY macro. unified version macros. simplified version
 * printing. simplified and enhanced version watermark print onto console
 * background. added HoT lines to the quit menu (shameless plug)
 *
 * Revision 1.31  2005/10/02 15:43:08  sezero
 * killed -Wshadow warnings
 *
 * Revision 1.30  2005/09/24 23:50:36  sezero
 * fixed a bunch of compiler warnings
 *
 * Revision 1.29  2005/09/19 19:50:10  sezero
 * fixed those famous spelling errors
 *
 * Revision 1.28  2005/09/19 19:27:09  sezero
 * can't kill zombie patch (from Maddes / QIP)
 *
 * Revision 1.27  2005/08/20 13:06:34  sezero
 * favored unlink() over DeleteFile() on win32. removed unnecessary
 * platform defines for directory path separators. removed a left-
 * over CL_RemoveGIPFiles() from sys_win.c. fixed temporary gip files
 * not being removed and probably causing "bad" savegames on win32.
 *
 * Revision 1.26  2005/07/31 00:45:11  sezero
 * platform defines cleanup
 *
 * Revision 1.25  2005/07/09 09:07:58  sezero
 * changed all Hunk_Alloc() usage to Hunk_AllocName() for easier memory usage tracing.
 *
 * Revision 1.24  2005/07/02 13:12:27  sezero
 * commands.txt and edicts.txt will be saved into com_userdir
 *
 * Revision 1.23  2005/06/07 07:06:32  sezero
 * Moved flush_textures decision to svmain.c:SV_SpawnServer() again, this
 * time fixing it by not clearing the server struct in Host_ShutdownServer().
 * In fact this logic is still slightly flawed, because flush_textures isn't
 * set on map changes in client-to-remote server map-change situations.
 *
 * Revision 1.22  2005/06/03 13:25:28  sezero
 * Latest mouse fixes and clean-ups
 *
 * Revision 1.21  2005/05/29 08:53:57  sezero
 * get rid of silly name changes
 *
 * Revision 1.20  2005/05/29 08:38:12  sezero
 * get rid of the silly func name difference
 *
 * Revision 1.19  2005/05/21 17:04:16  sezero
 * - revived -nomouse that "disables mouse no matter what"
 * - renamed _windowed_mouse to _enable_mouse which is our intention,
 *   that is, dynamically disabling/enabling mouse while in game
 * - old code had many oversights/leftovers that prevented mouse being
 *   really disabled in fullscreen mode. fixed and cleaned-up here
 * - even in windowed mode, when mouse was disabled, mouse buttons and
 *   the wheel got processed. fixed it here.
 * - mouse cursor is never shown while the game is alive, regardless
 *   of mouse being enabled/disabled (I never liked an ugly pointer
 *   around while playing) Its only intention would be to be able to
 *   use the desktop, and for that see, the grab notes below
 * - if mouse is disabled, it is un-grabbed in windowed mode. Note: One
 *   can always use the keyboard shortcut CTRL-G for grabbing-ungrabbing
 *   the mouse regardless of mouse being enabled/disabled.
 * - ToggleFullScreenSA() used to update vid_mode but always forgot
 *   modestate. It now updates modestate as well.
 * - Now that IN_ActivateMouse() and IN_DeactivateMouse() are fixed,
 *   IN_ActivateMouseSA() and IN_DeactivateMouseSA() are redundant and
 *   are removed. BTW, I added a new qboolean mousestate_sa (hi Steve)
 *   which keeps track of whether we intentionally disabled the mouse.
 * - mouse disabling in fullscreen mode (in the absence of -nomouse
 *   arg) is not allowed in this patch, but this is done by a if 1/if 0
 *   conditional compilation. Next patch will change all those if 1 to
 *   if 0, and voila!, we can fully disable/enable mouse in fullscreen.
 * - moved modestate enums/defines to vid.h so that they can be used
 *   by other code properly.
 *
 * Revision 1.18  2005/05/20 16:17:50  sezero
 * keep ID style in declerations (less diff between h2/h2w...)
 *
 * Revision 1.17  2005/05/19 16:47:18  sezero
 * killed client->privileged (was only available to IDGODS)
 *
 * Revision 1.16  2005/05/19 16:41:50  sezero
 * removed all unused (never used) non-RJNET and non-QUAKE2RJ code
 *
 * Revision 1.15  2005/05/19 16:35:51  sezero
 * removed all unused IDGODS code
 *
 * Revision 1.14  2005/05/17 22:56:19  sezero
 * cleanup the "stricmp, strcmpi, strnicmp, Q_strcasecmp, Q_strncasecmp" mess:
 * Q_strXcasecmp will now be used throughout the code which are implementation
 * dependant defines for __GNUC__ (strXcasecmp) and _WIN32 (strXicmp)
 *
 * Revision 1.13  2005/05/17 06:50:02  sezero
 * removed underscored versions of string comparison functions
 * Q_strXXXXX is now only for !PLATFORM_UNIX
 * updated linux_inc.h
 *
 * Revision 1.12  2005/04/30 08:47:07  sezero
 * silenced warnings about name decleration being shadowed
 *
 * Revision 1.11  2005/04/30 08:45:17  sezero
 * silenced warnings about Lenght decleration being shadowed
 *
 * Revision 1.10  2005/04/15 20:21:49  sezero
 * Kill warning: cl_main.c:207: warning: `return' with no value, in function returning non-void
 *
 * Revision 1.9  2005/01/24 20:29:43  sezero
 * fix flush_textures decision which used to be always true
 *
 * Revision 1.8  2005/01/01 21:43:47  sezero
 * prototypes clean-up
 *
 * Revision 1.7  2004/12/18 14:15:35  sezero
 * Clean-up and kill warnings 10:
 * Remove some already commented-out functions and code fragments.
 * They seem to be of no-future use. Also remove some unused functions.
 *
 * Revision 1.6  2004/12/18 14:08:07  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.5  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.4  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.3  2004/12/05 10:52:18  sezero
 * Sync with Steven, 2004-12-04 :
 *  Fix the "Old Mission" menu PoP
 *  Also release the windowed mouse on pause
 *  Heapsize is now 32768 default
 *  The final splash screens now centre the messages properly
 *  Add more mods to the video mods table
 *  Add the docs folder and update it
 *
 * Revision 1.2  2004/11/28 00:58:08  sezero
 *
 * Commit Steven's changes as of 2004.11.24:
 *
 * * Rewritten Help/Version message(s)
 * * Proper fullscreen mode(s) for OpenGL.
 * * Screen sizes are selectable with "-width" and "-height" options.
 * * Mouse grab in window modes , which is released when menus appear.
 * * Interactive video modes in software game disabled.
 * * Replaced Video Mode menu with a helpful message.
 * * New menu items for GL Glow, Chase mode, Draw Shadows.
 * * Changes to initial cvar_t variables:
 *      r_shadows, gl_other_glows, _windowed_mouse,
 *
 * Revision 1.1.1.1  2004/11/28 00:04:34  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 25    4/01/98 4:53p Jmonroe
 * moved stack names to global
 * 
 * 24    3/21/98 6:02p Jmonroe
 * 
 * 23    3/17/98 4:35p Jmonroe
 * 
 * 22    3/16/98 8:53p Jmonroe
 * 
 * 21    3/16/98 8:39p Mgummelt
 * 
 * 20    3/16/98 8:31p Jweier
 * 
 * 19    3/16/98 3:52p Jmonroe
 * fixed info_masks for load/save changelevel
 * 
 * 18    3/05/98 12:53p Jmonroe
 * 
 * 17    3/05/98 11:24a Jmonroe
 * reset timer when going to single player
 * 
 * 16    3/03/98 9:59p Jmonroe
 * 
 * 15    3/03/98 1:41p Jmonroe
 * removed old mp stuff
 * 
 * 14    3/03/98 10:48a Jmonroe
 * clean up old infomask stuff
 * 
 * 13    3/02/98 11:32p Jmonroe
 * fixed the change class then load crash.
 * 
 * 12    3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 11    2/27/98 11:53p Jweier
 * 
 * 10    2/23/98 2:54p Jmonroe
 * made the MAP command display the current map if no argument was given
 * 
 * 9     2/20/98 3:17p Jweier
 * 
 * 8     2/19/98 3:31p Jweier
 * 
 * 7     2/18/98 2:55p Jweier
 * 
 * 6     2/10/98 12:56p Jmonroe
 * prevents using { in name now.
 * 
 * 5     2/02/98 10:28a Mgummelt
 * 
 * 4     1/20/98 11:09a Jmonroe
 * fix for bad model shit because of the remaining particles in restarts
 * 
 * 3     1/18/98 8:06p Jmonroe
 * all of rick's patch code is in now
 * 
 * 50    10/29/97 5:39p Jheitzman
 * 
 * 49    10/28/97 2:58p Jheitzman
 * 
 * 47    10/06/97 6:04p Rjohnson
 * Fix for save games and version update
 * 
 * 46    10/03/97 4:07p Rjohnson
 * Copy file updates
 * 
 * 45    9/30/97 4:26p Rjohnson
 * Updates
 * 
 * 44    9/25/97 11:55p Rjohnson
 * Error checking for loads / saves
 * 
 * 43    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 42    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 41    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 40    9/02/97 12:25a Rjohnson
 * Save update
 * 
 * 39    8/30/97 6:17p Rjohnson
 * Network changes
 * 
 * 38    8/29/97 2:49p Rjohnson
 * Fix for load games and playerclass
 * 
 * 37    8/27/97 12:12p Rjohnson
 * Fix for not saving unspawned clients
 * 
 * 36    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 35    8/21/97 2:45p Rjohnson
 * Fix for god mode
 * 
 * 34    8/19/97 4:13p Rjohnson
 * Fix for class
 * 
 * 33    8/18/97 11:44p Rjohnson
 * Fixes for loading
 * 
 * 32    8/18/97 12:03a Rjohnson
 * Added loading progress
 * 
 * 31    8/17/97 3:28p Rjohnson
 * Fix for color selection
 * 
 * 30    8/17/97 1:02p Rjohnson
 * Fix for spawn place
 * 
 * 29    8/15/97 11:47a Rjohnson
 * Fix for save games
 * 
 * 28    8/14/97 11:32p Rjohnson
 * Co-Op Fix
 * 
 * 27    8/14/97 10:20p Rjohnson
 * Console command for playerclass can not be set by player
 * 
 * 26    8/14/97 2:37p Rjohnson
 * Fix for save games
 * 
 * 25    7/28/97 2:44p Rjohnson
 * Fix for tinting
 * 
 * 24    7/24/97 5:21p Rlove
 * 
 * 23    7/21/97 12:45p Rjohnson
 * Calcs the time difference going between levels and passes it in to the
 * hcode function
 * 
 * 22    7/08/97 6:31p Rjohnson
 * Fix for going back to a level
 * 
 * 21    7/08/97 12:20p Rjohnson
 * Fix for going back to a level
 * 
 * 20    7/01/97 4:09p Rjohnson
 * Saving / Loading of client effects
 * 
 * 19    6/27/97 11:34a Rjohnson
 * Added a create function
 * 
 * 18    6/05/97 4:43p Rlove
 * Fly mode is network friendly
 * 
 * 17    5/20/97 11:32a Rjohnson
 * Revised Effects
 * 
 * 16    5/14/97 3:36p Rjohnson
 * Initial Stats Implementation
 * 
 * 15    5/02/97 8:05a Rlove
 * 
 * 14    4/16/97 12:16p Rjohnson
 * Made the GIP files be stored in the temp directory
 * 
 * 13    4/16/97 7:59a Rlove
 * Removed references to ammo_  fields
 * 
 * 12    4/15/97 4:29p Rjohnson
 * Forgot to include changelevel2 in the command set
 * 
 * 11    4/15/97 11:52a Rjohnson
 * Updates from quake2 for multi-level trigger stuff
 * 
 * 10    4/04/97 3:06p Rjohnson
 * Networking updates and corrections
 * 
 * 9     4/01/97 12:37p Rjohnson
 * Shows the name of the class
 * 
 * 8     4/01/97 11:25a Rjohnson
 * Added a way for the h-code to set the player class
 * 
 * 7     3/31/97 7:24p Rjohnson
 * Added a playerclass field and made sure the server/clients handle it
 * properly
 * 
 * 6     3/15/97 3:08p Rlove
 * Added COMA console command
 * 
 * 5     3/07/97 1:37p Rjohnson
 * Id Updates
 * 
 * 4     2/18/97 11:49a Rjohnson
 * Id Updates
 */
