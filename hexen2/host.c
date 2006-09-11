/*
	host.c
	coordinates spawning and killing of local servers

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/host.c,v 1.50 2006-09-11 09:16:24 sezero Exp $
*/

#include "quakedef.h"
#include "r_local.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <unistd.h>

/*

A server can always be started, even if the system started out as a client
to a remote system.

A client can NOT be started if the system started as a dedicated server.

Memory is cleared / released when a server or client begins, not when they end.

*/

static void Host_WriteConfiguration (char *fname);

quakeparms_t host_parms;

qboolean	host_initialized;		// true if into command execution

double		host_frametime;
double		host_time;
double		realtime;			// without any filtering or bounding
static double	oldrealtime;			// last frame run
int		host_framecount;

int		host_hunklevel;

client_t	*host_client;			// current client

jmp_buf 	host_abortserver;

byte		*host_basepal;
byte		*host_colormap;

cvar_t		sys_ticrate = {"sys_ticrate", "0.05", CVAR_NONE};
static	cvar_t	sys_adaptive = {"sys_adaptive", "1", CVAR_ARCHIVE};
static	cvar_t	host_framerate = {"host_framerate", "0", CVAR_NONE};	// set for slow motion
static	cvar_t	host_speeds = {"host_speeds", "0", CVAR_NONE};		// set for running times

static	cvar_t	serverprofile = {"serverprofile", "0", CVAR_NONE};

cvar_t	fraglimit = {"fraglimit", "0", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	timelimit = {"timelimit", "0", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	teamplay = {"teamplay", "0", CVAR_NOTIFY|CVAR_SERVERINFO};

cvar_t	samelevel = {"samelevel", "0", CVAR_NONE};
cvar_t	noexit = {"noexit", "0", CVAR_NOTIFY|CVAR_SERVERINFO};

cvar_t	developer = {"developer", "0", CVAR_ARCHIVE};

cvar_t	skill = {"skill", "1", CVAR_NONE};		// 0 - 3
cvar_t	deathmatch = {"deathmatch", "0", CVAR_NONE};	// 0, 1, or 2
cvar_t	randomclass = {"randomclass", "0", CVAR_NONE};	// 0, 1, or 2
cvar_t	coop = {"coop", "0", CVAR_NONE};		// 0 or 1

cvar_t	pausable = {"pausable", "1", CVAR_NONE};

cvar_t	temp1 = {"temp1", "0", CVAR_NONE};


/*
===============================================================================

SAVEGAME FILES HANDLING

===============================================================================
*/

void Host_RemoveGIPFiles (char *path)
{
	char	*name, tempdir[MAX_OSPATH];

	if (path)
		snprintf(tempdir, MAX_OSPATH, "%s", path);
	else
		snprintf(tempdir, MAX_OSPATH, "%s", com_savedir);

	name = Sys_FindFirstFile (tempdir, "*.gip");

	while (name)
	{
		unlink (va("%s/%s", tempdir, name));

		name = Sys_FindNextFile();
	}

	Sys_FindClose();
}

qboolean Host_CopyFiles(char *source, char *pat, char *dest)
{
	char	*name, tempdir[MAX_OSPATH], tempdir2[MAX_OSPATH];
	qboolean error;

	name = Sys_FindFirstFile(source, pat);
	error = false;

	while (name)
	{
		sprintf(tempdir,"%s/%s", source, name);
		sprintf(tempdir2,"%s/%s", dest, name);
#ifdef _WIN32
		if (!CopyFile(tempdir,tempdir2,FALSE))
#else
		if (COM_CopyFile(tempdir,tempdir2))
#endif
		{
			Con_Printf ("Error copying %s to %s\n",tempdir,tempdir2);
			error = true;
		}

		name = Sys_FindNextFile();
	}

	Sys_FindClose();

	return error;
}


//============================================================================

/*
================
Host_EndGame
================
*/
void Host_EndGame (char *message, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,message);
	vsnprintf (string,sizeof(string),message,argptr);
	va_end (argptr);
	Con_DPrintf ("Host_EndGame: %s\n",string);

	if (sv.active)
		Host_ShutdownServer (false);

	if (cls.state == ca_dedicated)
		Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit

	if (cls.demonum != -1)
		CL_NextDemo ();
	else
		CL_Disconnect ();

	longjmp (host_abortserver, 1);
}

/*
================
Host_Error

This shuts down both the client and server
================
*/
void Host_Error (char *error, ...)
{
	va_list		argptr;
	char		string[1024];
	static	qboolean inerror = false;

	if (inerror)
		Sys_Error ("Host_Error: recursive error!");
	inerror = true;

	SCR_EndLoadingPlaque ();		// reenable screen updates

	va_start (argptr,error);
	vsnprintf (string,sizeof(string),error,argptr);
	va_end (argptr);
	Con_Printf ("Host_Error: %s\n",string);

	if (sv.active)
		Host_ShutdownServer (false);

	if (cls.state == ca_dedicated)
		Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	CL_Disconnect ();
	cls.demonum = -1;

	inerror = false;

	longjmp (host_abortserver, 1);
}

/*
================
Host_FindMaxClients
================
*/
static void Host_FindMaxClients (void)
{
	int		i;

	svs.maxclients = 1;

	i = COM_CheckParm ("-dedicated");
	if (i)
	{
		cls.state = ca_dedicated;
		if (i != (com_argc - 1))
		{
			svs.maxclients = atoi (com_argv[i+1]);
		}
		else
			svs.maxclients = 8;
	}
	else
		cls.state = ca_disconnected;

	i = COM_CheckParm ("-listen");
	if (i)
	{
		if (cls.state == ca_dedicated)
			Sys_Error ("Only one of -dedicated or -listen can be specified");
		if (i != (com_argc - 1))
			svs.maxclients = atoi (com_argv[i+1]);
		else
			svs.maxclients = 8;
	}
	if (svs.maxclients < 1)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	svs.maxclientslimit = svs.maxclients;
	if (svs.maxclientslimit < 4)
		svs.maxclientslimit = 4;
	svs.clients = Hunk_AllocName (svs.maxclientslimit*sizeof(client_t), "clients");

	if (svs.maxclients > 1)
		Cvar_SetValue ("deathmatch", 1.0);
	else
		Cvar_SetValue ("deathmatch", 0.0);
}

/*
===============
Host_SaveConfig_f
===============
*/
static void Host_SaveConfig_f (void)
{
	if (cmd_source != src_command)
		return;

/*	if (!sv.active)
	{
		Con_Printf ("Not playing a local game.\n");
		return;
	}

	if (cl.intermission)
	{
		Con_Printf ("Can't save in intermission.\n");
		return;
	}
*/

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("saveConfig <savename> : save a config file\n");
		return;
	}

	if (strstr(Cmd_Argv(1), ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}

	Host_WriteConfiguration (Cmd_Argv(1));
}


/*
=======================
Host_InitLocal
======================
*/
static void Host_InitLocal (void)
{
	Cmd_AddCommand ("saveconfig", Host_SaveConfig_f);

	Host_InitCommands ();

	Cvar_RegisterVariable (&host_framerate);
	Cvar_RegisterVariable (&host_speeds);

	Cvar_RegisterVariable (&sys_ticrate);
	Cvar_RegisterVariable (&serverprofile);

	Cvar_RegisterVariable (&fraglimit);
	Cvar_RegisterVariable (&timelimit);
	Cvar_RegisterVariable (&teamplay);
	Cvar_RegisterVariable (&samelevel);
	Cvar_RegisterVariable (&noexit);
	Cvar_RegisterVariable (&skill);
	Cvar_RegisterVariable (&developer);
	Cvar_RegisterVariable (&deathmatch);
	Cvar_RegisterVariable (&randomclass);
	Cvar_RegisterVariable (&coop);

	Cvar_RegisterVariable (&pausable);

	Cvar_RegisterVariable (&sys_adaptive);

	Cvar_RegisterVariable (&temp1);

	Host_FindMaxClients ();

	host_time = 1.0;		// so a think at time 0 won't get called
}

/*
===============
Host_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
static void Host_WriteConfiguration (char *fname)
{
	FILE	*f;

// dedicated servers initialize the host but don't parse and set the
// config.cfg cvars
	if (host_initialized && !isDedicated)
	{
		f = fopen (va("%s/%s",com_userdir,fname), "w");
		if (!f)
		{
			Con_Printf ("Couldn't write %s.\n",fname);
			return;
		}

		Key_WriteBindings (f);
		Cvar_WriteVariables (f);

		if (in_mlook.state & 1)		//if mlook was down, keep it that way
			fprintf (f, "+mlook\n");

		fclose (f);
	}
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed 
FIXME: make this just a stuffed echo?
=================
*/
void SV_ClientPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	vsnprintf (string, sizeof (string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_print);
	MSG_WriteString (&host_client->message, string);
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	int			i;

	va_start (argptr,fmt);
	vsnprintf (string, sizeof (string), fmt, argptr);
	va_end (argptr);

	for (i=0 ; i<svs.maxclients ; i++)
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			MSG_WriteByte (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
}

/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	vsnprintf (string, sizeof (string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message, string);
}

/*
=====================
SV_DropClient

Called when the player is getting totally kicked off the host
if (crash = true), don't bother sending signofs
=====================
*/
void SV_DropClient (qboolean crash)
{
	int		saveSelf;
	int		i;
	client_t *client;

	if (!crash)
	{
		// send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			MSG_WriteByte (&host_client->message, svc_disconnect);
			NET_SendMessage (host_client->netconnection, &host_client->message);
		}

		if (host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			if (old_progdefs)
			{
				saveSelf = pr_global_struct_v111->self;
				pr_global_struct_v111->self = EDICT_TO_PROG(host_client->edict);
				PR_ExecuteProgram (pr_global_struct_v111->ClientDisconnect);
				pr_global_struct_v111->self = saveSelf;
			}
			else
			{
				saveSelf = pr_global_struct->self;
				pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
				PR_ExecuteProgram (pr_global_struct->ClientDisconnect);
				pr_global_struct->self = saveSelf;
			}
		}

		Sys_Printf ("Client %s removed\n",host_client->name);
	}

// break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

// free the client (the body stays around)
	host_client->active = false;
	host_client->name[0] = 0;
	host_client->old_frags = -999999;
	memset(&host_client->old_v,0,sizeof(host_client->old_v));
	ED_ClearEdict(host_client->edict);
	host_client->send_all_v = true;
	net_activeconnections--;

// send notification to all clients
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		MSG_WriteByte (&client->message, svc_updatename);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteString (&client->message, "");
		MSG_WriteByte (&client->message, svc_updatefrags);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteShort (&client->message, 0);
		MSG_WriteByte (&client->message, svc_updatecolors);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteByte (&client->message, 0);
	}
}

/*
==================
Host_ShutdownServer

This only happens at the end of a game, not between levels
==================
*/
void Host_ShutdownServer(qboolean crash)
{
	int		i;
	int		count;
	sizebuf_t	buf;
	byte		message[4];
	double	start;

	if (!sv.active)
		return;

	sv.active = false;

// stop all client sounds immediately
	if (cls.state == ca_connected)
		CL_Disconnect ();

// flush any pending messages - like the score!!!
	start = Sys_DoubleTime();
	do
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.cursize)
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage(host_client->netconnection, &host_client->message);
					SZ_Clear (&host_client->message);
				}
				else
				{
					NET_GetMessage(host_client->netconnection);
					count++;
				}
			}
		}
		if ((Sys_DoubleTime() - start) > 3.0)
			break;
	}
	while (count);

// make sure all the clients know we're disconnecting
	SZ_Init (&buf, message, sizeof(message));
	MSG_WriteByte(&buf, svc_disconnect);
	count = NET_SendToAll(&buf, 5);
	if (count)
		Con_Printf("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_DropClient(crash);

//
// clear structures
//
	//memset (&sv, 0, sizeof(sv)); // ServerSpawn will already do this by Host_ClearMemory
	memset (svs.clients, 0, svs.maxclientslimit*sizeof(client_t));
}


/*
================
Host_ClearMemory

This clears all the memory used by both the client and server, but does
not reinitialize anything.
================
*/
void Host_ClearMemory (void)
{
	Con_DPrintf ("Clearing memory\n");
	D_FlushCaches ();
	Mod_ClearAll ();
	if (host_hunklevel)
		Hunk_FreeToLowMark (host_hunklevel);

	cls.signon = 0;
	memset (&sv, 0, sizeof(sv));
	memset (&cl, 0, sizeof(cl));
}


//============================================================================


/*
===================
Host_FilterTime

Returns false if the time is too short to run a frame
===================
*/
static qboolean Host_FilterTime (float time)
{
	realtime += time;

	if (!cls.timedemo && realtime - oldrealtime < 1.0/72.0)
		return false;		// framerate is too high

	host_frametime = realtime - oldrealtime;
	oldrealtime = realtime;

	if (host_framerate.value > 0)
		host_frametime = host_framerate.value;
	else
	{	// don't allow really long or short frames
		if (host_frametime > 0.05 && !sys_adaptive.value)
			host_frametime = 0.05;
		if (host_frametime < 0.001)
			host_frametime = 0.001;
	}

	return true;
}


/*
===================
Host_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
static void Host_GetConsoleCommands (void)
{
	char	*cmd;

	if (!isDedicated)
		return;	// no stdin necessary in graphical mode

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}


//#define FPS_20

/*
==================
Host_ServerFrame

==================
*/
#ifdef FPS_20

static void _Host_ServerFrame (void)
{
// run the world state
	if (old_progdefs)
		pr_global_struct_v111->frametime = host_frametime;
	else
		pr_global_struct->frametime = host_frametime;

// read client messages
	SV_RunClients ();

// move things around and think
// always pause in single player if in console or menus
	if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
	{
		SV_Physics ();

		R_UpdateParticles ();
		CL_UpdateEffects ();
	}
}

static void Host_ServerFrame (void)
{
	float	save_host_frametime;
	float	temp_host_frametime;

// run the world state
	if (old_progdefs)
		pr_global_struct_v111->frametime = host_frametime;
	else
		pr_global_struct->frametime = host_frametime;

// set the time and clear the general datagram
	SV_ClearDatagram ();

// check for new clients
	SV_CheckForNewClients ();

	temp_host_frametime = save_host_frametime = host_frametime;
	while(temp_host_frametime > (1.0/72.0))
	{
		if (temp_host_frametime > 0.05)
			host_frametime = 0.05;
		else
			host_frametime = temp_host_frametime;
		temp_host_frametime -= host_frametime;
		_Host_ServerFrame ();
	}
	host_frametime = save_host_frametime;

// send all messages to the clients
	SV_SendClientMessages ();
}

#else

static void Host_ServerFrame (void)
{
// run the world state
	if (old_progdefs)
		pr_global_struct_v111->frametime = host_frametime;
	else
		pr_global_struct->frametime = host_frametime;

// set the time and clear the general datagram
	SV_ClearDatagram ();

// check for new clients
	SV_CheckForNewClients ();

// read client messages
	SV_RunClients ();

// move things around and think
// always pause in single player if in console or menus
	if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
		SV_Physics ();

// send all messages to the clients
	SV_SendClientMessages ();
}

#endif

/*
==================
Host_Frame

Runs all active servers
==================
*/
static void _Host_Frame (float time)
{
	static double		time1 = 0;
	static double		time2 = 0;
	static double		time3 = 0;
	int			pass1, pass2, pass3;
#if !defined(FPS_20)
	double	save_host_frametime,total_host_frametime;
#endif

	if (setjmp (host_abortserver) )
		return;			// something bad happened, or the server disconnected

// keep the random time dependent
	rand ();

// decide the simulation time
	if (!Host_FilterTime (time))
		return;			// don't run too fast, or packets will flood out

// get new key events
	Sys_SendKeyEvents ();

// allow mice or other external controllers to add commands
	IN_Commands ();

// process console commands
	Cbuf_Execute ();

	NET_Poll();

// if running the server locally, make intentions now
	if (sv.active)
		CL_SendCmd ();

//-------------------
//
// server operations
//
//-------------------

// check for commands typed to the host
	Host_GetConsoleCommands ();

#ifdef FPS_20

	if (sv.active)
		Host_ServerFrame ();

//-------------------
//
// client operations
//
//-------------------

// if running the server remotely, send intentions now after
// the incoming messages have been read
	if (!sv.active)
		CL_SendCmd ();

	host_time += host_frametime;

// fetch results from server
	if (cls.state == ca_connected)
	{
		CL_ReadFromServer ();
	}

#else

	save_host_frametime = total_host_frametime = host_frametime;
	if (sys_adaptive.value)
	{
		if (host_frametime > 0.05)
		{
			host_frametime = 0.05;
		}
	}

	if (total_host_frametime > 1.0)
		total_host_frametime = 0.05;

	do
	{
		if (sv.active)
			Host_ServerFrame ();

	//-------------------
	//
	// client operations
	//
	//-------------------

	// if running the server remotely, send intentions now after
	// the incoming messages have been read
		if (!sv.active)
			CL_SendCmd ();

		host_time += host_frametime;

	// fetch results from server
		if (cls.state == ca_connected)
		{
			CL_ReadFromServer ();
		}

		R_UpdateParticles ();
		CL_UpdateEffects ();

		if (!sys_adaptive.value)
			break;

		total_host_frametime -= 0.05;
		if (total_host_frametime > 0 && total_host_frametime < 0.05)
		{
			save_host_frametime -= total_host_frametime;
			oldrealtime -= total_host_frametime;
			break;
		}

	} while (total_host_frametime > 0);

	host_frametime = save_host_frametime;

#endif

// update video
	if (host_speeds.value)
		time1 = Sys_DoubleTime ();

	SCR_UpdateScreen ();

	if (host_speeds.value)
		time2 = Sys_DoubleTime ();

// update audio
	if (cls.signon == SIGNONS)
	{
		S_Update (r_origin, vpn, vright, vup);
		CL_DecayLights ();
	}
	else
		S_Update (vec3_origin, vec3_origin, vec3_origin, vec3_origin);

	CDAudio_Update();
	MIDI_Update();

	if (host_speeds.value)
	{
		pass1 = (time1 - time3)*1000;
		time3 = Sys_DoubleTime ();
		pass2 = (time2 - time1)*1000;
		pass3 = (time3 - time2)*1000;
		Con_Printf ("%3i tot %3i server %3i gfx %3i snd\n",
					pass1+pass2+pass3, pass1, pass2, pass3);
	}

	host_framecount++;
}

void Host_Frame (float time)
{
	double	time1, time2;
	static double	timetotal;
	static int		timecount;
	int		i, c, m;

	if (!serverprofile.value)
	{
		_Host_Frame (time);
		return;
	}

	time1 = Sys_DoubleTime ();
	_Host_Frame (time);
	time2 = Sys_DoubleTime ();

	timetotal += time2 - time1;
	timecount++;

	if (timecount < 1000)
		return;

	m = timetotal*1000/timecount;
	timecount = 0;
	timetotal = 0;
	c = 0;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n", c, m);
}

//============================================================================


#if NET_USE_VCR
extern FILE *vcrFile;
#define	VCR_SIGNATURE	0x56435231
// "VCR1"

static void Host_InitVCR (quakeparms_t *parms)
{
	int		i, len, n;
	char	*p;

	if (COM_CheckParm("-playback"))
	{
		if (com_argc != 2)
			Sys_Error("No other parameters allowed with -playback\n");

		//vcrFile = fopen (va("%s/quake.vcr",com_userdir), "rb");
		vcrFile = fopen (va("%s/quake.vcr",parms->userdir), "rb");
		if (!vcrFile)
			Sys_Error("playback file not found\n");

		fread (&i, 1, sizeof(int), vcrFile);
		if (i != VCR_SIGNATURE)
			Sys_Error("Invalid signature in vcr file\n");

		fread (&com_argc, 1, sizeof(int), vcrFile);
		com_argv = Z_Malloc(com_argc * sizeof(char *));
		com_argv[0] = parms->argv[0];
		for (i = 0; i < com_argc; i++)
		{
			fread (&len, 1, sizeof(int), vcrFile);
			p = Z_Malloc(len);
			fread (p, 1, len, vcrFile);
			com_argv[i+1] = p;
		}
		com_argc++; /* add one for arg[0] */
		parms->argc = com_argc;
		parms->argv = com_argv;
	}

	if ( (n = COM_CheckParm("-record")) != 0)
	{
		//vcrFile = fopen (va("%s/quake.vcr",com_userdir), "wb");
		vcrFile = fopen (va("%s/quake.vcr",parms->userdir), "wb");

		i = VCR_SIGNATURE;
		fwrite (&i, 1, sizeof(int), vcrFile);
		i = com_argc - 1;
		fwrite (&i, 1, sizeof(int), vcrFile);
		for (i = 1; i < com_argc; i++)
		{
			if (i == n)
			{
				len = 10;
				fwrite (&len, 1, sizeof(int), vcrFile);
				fwrite ("-playback", 1, len, vcrFile);
				continue;
			}
			len = strlen(com_argv[i]) + 1;
			fwrite (&len, 1, sizeof(int), vcrFile);
			fwrite (com_argv[i], 1, len, vcrFile);
		}
	}
}
#endif	// NET_USE_VCR

/*
====================
Host_Init
====================
*/
extern void VID_PostInitFix (void);
void Host_Init (quakeparms_t *parms)
{
	host_parms = *parms;

	com_argc = parms->argc;
	com_argv = parms->argv;

	Memory_Init (parms->membase, parms->memsize);
	Cbuf_Init ();
	Cmd_Init ();
	CL_Cmd_Init ();
	V_Init ();
	Chase_Init ();
#if NET_USE_VCR
	Host_InitVCR (parms);
#endif
	COM_Init ();
	Host_RemoveGIPFiles(NULL);
	Host_InitLocal ();
	W_LoadWadFile ("gfx.wad");
	Key_Init ();
	Con_Init ();
	M_Init ();
	PR_Init ();
	Mod_Init ();
	NET_Init ();
	SV_Init ();

	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
	Con_Printf ("%4.1f megabyte heap\n",parms->memsize/ (1024*1024.0));

	R_InitTextures ();		// needed even for dedicated servers

	if (cls.state != ca_dedicated)	// decided in Host_InitLocal() by calling Host_FindMaxClients()
	{
		host_basepal = (byte *)COM_LoadHunkFile ("gfx/palette.lmp");
		if (!host_basepal)
			Sys_Error ("Couldn't load gfx/palette.lmp");

		host_colormap = (byte *)COM_LoadHunkFile ("gfx/colormap.lmp");
		if (!host_colormap)
			Sys_Error ("Couldn't load gfx/colormap.lmp");

		VID_Init (host_basepal);
		Draw_Init ();
		SCR_Init ();
		R_Init ();

#if defined(GLQUAKE) || defined(PLATFORM_UNIX)
	// VID_Init of vid_win.c already is responsible for S_Init
	// FIXME: gl_vidnt.c doesn't use the new one-window approach yet
		S_Init ();
#endif

		CDAudio_Init();
		MIDI_Init();
		Sbar_Init();
		CL_Init();
		IN_Init();
	}

#ifdef GLQUAKE
/*	analogous to host_hunklevel, this will mark OpenGL texture
	beyond which everything will need to be purged on new map */
	gl_texlevel = numgltextures;
#endif

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	Cbuf_InsertText ("exec hexen.rc\n");
	Cbuf_Execute();
	// fix the early-set cvars after init
	if (cls.state != ca_dedicated)
		VID_PostInitFix ();
	Cbuf_AddText ("cl_warncmd 1\n");

	host_initialized = true;

	Con_Printf("\n======== Hexen II Initialized =========\n\n");
}


/*
===============
Host_Shutdown

FIXME: this is a callback from Sys_Quit and Sys_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void Host_Shutdown(void)
{
	static qboolean isdown = false;

	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

// keep Con_Printf from trying to update the screen
	scr_disabled_for_loading = true;

	Host_WriteConfiguration ("config.cfg");

	NET_Shutdown ();

	if (cls.state != ca_dedicated)
	{
		CDAudio_Shutdown ();
		MIDI_Cleanup();
		S_Shutdown();
		IN_Shutdown ();
		VID_Shutdown();
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.49  2006/06/25 12:01:48  sezero
 * renamed CL_CopyFiles to Host_CopyFiles and CL_RemoveGIPFiles to
 * Host_RemoveGIPFiles, moved them to host.c
 *
 * Revision 1.48  2006/06/23 14:43:33  sezero
 * some minor clean-ups
 *
 * Revision 1.47  2006/06/14 12:46:43  sezero
 * disabled the VCR facility by default. made it a compile time option.
 * After all, it is NOT an ordinary demo recording but a server only
 * feature for debugging server bugs.
 *
 * Revision 1.46  2006/05/18 17:48:10  sezero
 * renamed MIDI_UpdateVolume to MIDI_Update
 *
 * Revision 1.45  2006/04/05 06:10:43  sezero
 * added support for both hexen2-v1.11 and h2mp-v1.12 progs into a single hexen2
 * binary. this essentially completes the h2/h2mp binary merge started with the
 * previous patch. many conditionals had to be added especially on the server side,but couldn't notice any serious performance loss on a PIII-733 computer. Supportfor multiple progs.dat is now advised to be left enabled in order to support
 * mods which uses that feature.
 *
 * Revision 1.44  2006/03/24 15:05:39  sezero
 * killed the archive, server and info members of the cvar structure.
 * the new flags member is now employed for all those purposes. also
 * made all non-globally used cvars static.
 *
 * Revision 1.43  2006/02/24 14:43:55  sezero
 * created a new "opengl features" entry under the options menu and moved opengl
 * options under it. added new opengl menu options for texture filtering, glow
 * effects, multitexturing, stencil buffered shadows and texture purging upon
 * map change.
 *
 * Revision 1.42  2006/02/18 08:51:09  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. also renamed the variables name and dest to savename and savedest in
 * host_cmd.c to prevent any confusion and pollution.
 *
 * Revision 1.41  2006/01/17 17:36:44  sezero
 * A quick'n'dirty patch for making the game to remember its video settings:
 * Essentially it does an early read of config.cfg while in VID_Init to
 * remember the settings (new procedure: VID_EarlyReadConfig). (new cvars:
 * vid_config_glx, vid_config_gly, vid_config_swx, vid_config_swy, and
 * vid_config_fscr). the commandline still acts as an override. then, it fixes
 * the cvar screw-up caused by the actual read of config.cfg by overwriting
 * the affected cvars with the running settings (new tiny procedure:
 * VID_PostInitFix, called from Host_Init).
 *
 * Implemented here are the screen dimensions, color bits (bpp, for win32,
 * cvar: vid_config_bpp), palettized textures and multisampling (fsaa, for
 * unix, cvars: vid_config_gl8bit and vid_config_fsaa) options with their
 * menu representations and cvar memorizations.
 *
 * This method can probably be also used to store/remember the conwidth
 * settings. Also applicable is the sound settings, such as the driver,
 * sampling rate, format, etc.
 *
 * Secondly, the patch sets the fullscreen cvar not by only looking at silly
 * values but by looking at the current SDL_Screen flags.
 *
 * Revision 1.40  2006/01/12 12:57:45  sezero
 * moved init of platform specific variables and function pointers to snd_sys
 *
 * Revision 1.39  2005/12/11 11:56:33  sezero
 * synchronized different sbar function names between h2 and h2w.
 * there was a mess about SB_Changed and Sbar_Changed in h2w, this
 * patch fixes that: h2 (and h2w) version of SB_Changed was never
 * functional. h2w actually called SB_InvChanged, who knows what
 * the original intention was, but that seemed serving to no purpose
 * to me. in any case, watching for any new weirdness in h2w would
 * be advisable. ability string indexes for the demoness and dwarf
 * classes in h2w are fixed. armor class display in h2w is fixed.
 * h2 and h2w versions of gl_vidsdl and gl_vidnt are synchronized.
 *
 * Revision 1.38  2005/12/04 11:20:57  sezero
 * init stuff cleanup
 *
 * Revision 1.37  2005/12/04 11:19:18  sezero
 * gamma stuff update
 *
 * Revision 1.36  2005/12/04 11:14:38  sezero
 * the big vsnprintf patch
 *
 * Revision 1.35  2005/11/02 18:40:21  sezero
 * set host_hunklevel before execing hexen.rc
 *
 * Revision 1.34  2005/10/29 23:52:33  sezero
 * set cl_warncmd to 1 after execing configs
 *
 * Revision 1.33  2005/10/29 21:43:22  sezero
 * unified cmd layer
 *
 * Revision 1.32  2005/10/29 13:50:25  sezero
 * set the gl_texlevel before execing hexen.rc, otherwise an +map xxx
 * console command shall end up with a broken display
 *
 * Revision 1.31  2005/10/21 17:57:14  sezero
 * added support for systems without OSS sound.
 * added a paranoid case to S_SoundInfo.
 * killed a few unnecessary prints.
 *
 * Revision 1.30  2005/09/29 14:04:06  sezero
 * cleaned-up Host_Init and Host_Shutdown stuff. removed
 * unnecessary ca_dedicated checks from CDAudio_Init.
 *
 * Revision 1.29  2005/09/26 18:26:41  sezero
 * flush the command buffer before setting host_initialized to true
 *
 * Revision 1.28  2005/09/19 19:50:10  sezero
 * fixed those famous spelling errors
 *
 * Revision 1.27  2005/09/19 06:18:13  sezero
 * check for isDedicated in Host_WriteConfiguration should be logical
 *
 * Revision 1.26  2005/08/18 14:20:28  sezero
 * moved music volume update back into frame update. it has its merits (such as
 * correct bgmvolume on startup) this way..
 *
 * Revision 1.25  2005/07/23 22:22:08  sezero
 * unified the common funcntions for hexen2-hexenworld
 *
 * Revision 1.24  2005/07/09 07:31:38  sezero
 * use zone instead of malloc
 *
 * Revision 1.23  2005/06/15 22:03:02  sezero
 * vid_setgamma command is for sdl versions only.
 * also added notes on the WITH_SDL define in quakedef.h
 *
 * Revision 1.22  2005/06/12 07:31:18  sezero
 * enabled alsa only on linux platforms
 *
 * Revision 1.21  2005/06/07 07:06:32  sezero
 * Moved flush_textures decision to svmain.c:SV_SpawnServer() again, this
 * time fixing it by not clearing the server struct in Host_ShutdownServer().
 * In fact this logic is still slightly flawed, because flush_textures isn't
 * set on map changes in client-to-remote server map-change situations.
 *
 * Revision 1.20  2005/05/29 08:38:12  sezero
 * get rid of the silly func name difference
 *
 * Revision 1.19  2005/05/26 09:07:21  sezero
 * moved Mesa environment variable handling to gl_vidsdl where they belong
 *
 * Revision 1.18  2005/05/22 11:59:53  sezero
 * killed cvar sys_quake2 (I dont think anyone uses it)
 *
 * Revision 1.17  2005/05/21 08:56:11  sezero
 * MINIMUM_MEMORY_LEVELPAK was never used, switched to MINIMUM_MEMORY
 *
 * Revision 1.16  2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * Revision 1.15  2005/04/30 08:30:09  sezero
 * changed message datatypes to byte in SV_SendReconnect() and Host_ShutdownServer()
 *
 * Revision 1.14  2005/04/30 07:44:35  sezero
 * Always set the MESA_GLX_FX FX_DONT_FAKE_MULTITEX environment variables
 *
 * Revision 1.13  2005/04/15 20:24:21  sezero
 * added working console input (from hexenworld server).
 * disabled console input in non-dedicated mode.
 *
 * Revision 1.12  2005/04/13 12:22:41  sezero
 * - Removed useless -minmemory cmdline argument
 * - Removed useless parms->memsize < minimum_memory check in Host_Init
 * - Added lower/upper boundaries (8mb/96mb) for -heapsize argument
 * - Added lower (48kb for hexen2, 256kb for hw)/upper (1mb) boundaries
 *   for -zone argument (DYNAMIC_SIZE definitions/zonesize requirements
 *   are different for hexen2 and hexenworld)
 * - We won't die if no size is specified after -zone, but will ignore
 * - Added null string terminations to hexen2 zone.c, so as to prevent
 *   garbage on sys_memory console command (found this in Pa3PyX)
 *
 * Revision 1.11  2005/04/08 20:14:20  sezero
 * Added -old3dfx cmdline check. For now, I use it to set the
 * MESA_GLX_FX environment variable to "f" and to disable fxMesa
 * fake multitexturing. Further future uses possible.
 *
 * Revision 1.10  2005/03/06 10:44:41  sezero
 * - move reinit_music to menu.c where it belongs
 * - fix reinit_music so that it works for the F4 key as well
 * - don't mess with music volume on every frame update, it's just silly
 *
 * Revision 1.9  2005/02/14 15:12:32  sezero
 * added ability to disable ALSA support at compile time
 *
 * Revision 1.8  2005/02/11 23:47:02  sezero
 * paranoid sound driver checks
 *
 * Revision 1.7  2005/02/05 16:18:25  sezero
 * added midi volume control (partially from Pa3PyX)
 *
 * Revision 1.6  2005/02/04 13:40:20  sezero
 * build all all the sound drivers in and choose from command line
 *
 * Revision 1.5  2005/01/01 21:43:47  sezero
 * prototypes clean-up
 *
 * Revision 1.4  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.3  2004/12/18 13:30:50  sezero
 * Hack to prevent textures going awol and some info-plaques start looking
 * white upon succesive load games. The solution is not beautiful but seems
 * to work for now. Adapted from Pa3PyX sources.
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:04:28  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.4  2001/12/14 16:31:06  phneutre
 * moved CL_RemoveGIPFiles(NULL) grom main() to Host_Init because of NULL paths
 *
 * Revision 1.3  2001/12/13 18:37:05  phneutre
 * write config.cfg in ~/.aot instead of data1/
 *
 * Revision 1.2  2001/12/13 13:44:25  phneutre
 * apply gamma settings at startup after execing config file where the value is stored
 *
 * 
 * 6     3/12/98 6:31p Mgummelt
 * 
 * 5     3/03/98 1:41p Jmonroe
 * removed old mp stuff
 * 
 * 4     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 3     2/23/98 2:53p Jmonroe
 * made config.cfg save the mlook status
 * 
 * 2     2/13/98 4:08p Jmonroe
 * added saveconfig
 * 
 * 22    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 21    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 20    8/27/97 12:13p Rjohnson
 * Clear out the entity when client removed
 * 
 * 19    8/20/97 3:01p Rjohnson
 * Name change
 * 
 * 18    5/19/97 2:54p Rjohnson
 * Added new client effects
 * 
 * 17    5/05/97 5:34p Rjohnson
 * Added a quake2 console variable
 * 
 * 16    4/30/97 11:20p Bgokey
 * 
 * 15    4/04/97 3:06p Rjohnson
 * Networking updates and corrections
 * 
 * 14    3/31/97 7:28p Rjohnson
 * Cleared out the class when a client is dropped
 * 
 * 13    3/31/97 3:17p Rlove
 * Removing references to Quake
 * 
 * 12    3/07/97 1:29p Rjohnson
 * Id Updates
 * 
 * 11    3/05/97 11:18a Rjohnson
 * Made the developer console var to be saved
 * 
 * 10    2/27/97 4:11p Rjohnson
 * Added midi init and cleanup, as well as limiting the adaptavite timing
 * to 1 sec catchup
 * 
 * 9     2/24/97 6:01p Rjohnson
 * Removed weird character
 * 
 * 8     2/24/97 1:50p Rjohnson
 * Looked into id's 20 fps
 * 
 * 7     2/18/97 11:42a Rjohnson
 * Added headers
 */
