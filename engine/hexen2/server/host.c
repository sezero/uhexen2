/*
 * host.c -- coordinates spawning and killing of local servers
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
#include "debuglog.h"

/*
 * Memory is cleared / released when a server or client begins, not when
 * they end.
 */

quakeparms_t	*host_parms;

qboolean	host_initialized;		// true if into command execution

double		host_frametime;
double		realtime;			// without any filtering or bounding

int		host_hunklevel;

client_t	*host_client;			// current client

cvar_t		sys_ticrate = {"sys_ticrate", "0.05", CVAR_NONE};
static	cvar_t	host_framerate = {"host_framerate", "0", CVAR_NONE};	// set for slow motion

static	cvar_t	serverprofile = {"serverprofile", "0", CVAR_NONE};

cvar_t	fraglimit = {"fraglimit", "0", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	timelimit = {"timelimit", "0", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	teamplay = {"teamplay", "0", CVAR_NOTIFY|CVAR_SERVERINFO};

cvar_t	samelevel = {"samelevel", "0", CVAR_NONE};
cvar_t	noexit = {"noexit", "0", CVAR_NOTIFY|CVAR_SERVERINFO};

cvar_t	developer = {"developer", "0", CVAR_ARCHIVE};

cvar_t	skill = {"skill", "1", CVAR_NONE};		// 0 - 3
cvar_t	coop = {"coop", "0", CVAR_NONE};		// 0 or 1
cvar_t	deathmatch = {"deathmatch", "0", CVAR_NONE};	// 0, 1, or 2
cvar_t	randomclass = {"randomclass", "0", CVAR_NONE};	// 0, 1, or 2

cvar_t	pausable = {"pausable", "1", CVAR_NONE};

cvar_t	temp1 = {"temp1", "0", CVAR_NONE};


/*
===============================================================================

SAVEGAME FILES HANDLING

===============================================================================
*/

void Host_RemoveGIPFiles (const char *path)
{
	const char	*name;
	char	tempdir[MAX_OSPATH], *p;
	size_t	len;

	if (path)
		q_strlcpy(tempdir, path, MAX_OSPATH);
	else	q_strlcpy(tempdir, FS_GetUserdir(), MAX_OSPATH);

	len = strlen(tempdir);
	p = tempdir + len;
	len = sizeof(tempdir) - len;

	name = Sys_FindFirstFile (tempdir, "*.gip");

	while (name)
	{
		q_snprintf (p, len, "/%s", name);
		Sys_unlink (tempdir);
		*p = '\0';
		name = Sys_FindNextFile();
	}

	Sys_FindClose();
}

void Host_DeleteSave (const char *savepath)
{
	char	tmppath[MAX_OSPATH];

	if (strstr(savepath, FS_GetUserdir()) != savepath)
		return;

	Host_RemoveGIPFiles (savepath);
	q_snprintf (tmppath, sizeof(tmppath), "%s/info.dat", savepath);
	Sys_unlink (tmppath);
	Sys_rmdir (savepath);
}


int Host_CopyFiles (const char *source, const char *pat, const char *dest)
{
	const char	*name;
	char	tempdir[MAX_OSPATH], tempdir2[MAX_OSPATH];
	int	error;

	name = Sys_FindFirstFile(source, pat);
	error = 0;

	while (name)
	{
		if (q_snprintf(tempdir, sizeof(tempdir),"%s/%s", source, name) >= (int)sizeof(tempdir) ||
		    q_snprintf(tempdir2, sizeof(tempdir2),"%s/%s", dest, name) >= (int)sizeof(tempdir2))
		{
			Sys_FindClose();
			Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
			return -1;
		}

		error = FS_CopyFile (tempdir, tempdir2);
		if (error)
		{
			Con_Printf ("Error copying %s to %s\n", tempdir, tempdir2);
			goto error_out;
		}

		name = Sys_FindNextFile();
	}

error_out:
	Sys_FindClose();

	return error;
}


//============================================================================

/*
================
Host_Error

This shuts down both the client and server
================
*/
void Host_Error (const char *error, ...)
{
	va_list		argptr;
	char		string[1024];
	static	qboolean inerror = false;

	if (inerror)
		Sys_Error ("%s: recursive error!", __thisfunc__);
	inerror = true;

	va_start (argptr,error);
	q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);
	Con_Printf ("%s: %s\n", __thisfunc__, string);

	Host_ShutdownServer (false);

	Sys_Error ("%s: %s", __thisfunc__, string);	// dedicated servers exit
}

/*
================
Host_FindMaxClients
================
*/
static void Host_FindMaxClients (void)
{
	int		i;

	svs.maxclients = 8;

	i = COM_CheckParm ("-dedicated");
	if (i && i < com_argc-1)
		svs.maxclients = atoi (com_argv[i+1]);

	if (svs.maxclients < 2)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_CLIENTS)
		svs.maxclients = MAX_CLIENTS;

	svs.maxclientslimit = svs.maxclients;
	if (svs.maxclientslimit < 4)
		svs.maxclientslimit = 4;
	svs.clients = (client_t *) Hunk_AllocName (svs.maxclientslimit*sizeof(client_t), "clients");

	Cvar_Set ("deathmatch", "1");
}

static void Host_Version_f (void)
{
	Con_Printf ("Version %4.2f\n", ENGINE_VERSION);
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
}

/* cvar callback functions : */
void Host_Callback_Notify (cvar_t *var)
{
	if (sv.active)
		SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
}

/*
=======================
Host_InitLocal
======================
*/
static void Host_InitLocal (void)
{
	Cmd_AddCommand ("version", Host_Version_f);

	Host_InitCommands ();

	Cvar_RegisterVariable (&developer);
	if (COM_CheckParm("-developer"))
	{
		Cvar_Set ("developer", "1");
		Cvar_LockVar ("developer");
	}

	Cvar_RegisterVariable (&sys_nostdout);

	Cvar_RegisterVariable (&sys_ticrate);

	Cvar_RegisterVariable (&host_framerate);

	Cvar_RegisterVariable (&serverprofile);

	Cvar_RegisterVariable (&fraglimit);
	Cvar_RegisterVariable (&timelimit);
	Cvar_RegisterVariable (&teamplay);
	Cvar_SetCallback (&fraglimit, Host_Callback_Notify);
	Cvar_SetCallback (&timelimit, Host_Callback_Notify);
	Cvar_SetCallback (&teamplay, Host_Callback_Notify);
	Cvar_RegisterVariable (&samelevel);
	Cvar_RegisterVariable (&noexit);
	Cvar_SetCallback (&noexit, Host_Callback_Notify);
	Cvar_RegisterVariable (&skill);
	Cvar_RegisterVariable (&coop);
	Cvar_RegisterVariable (&deathmatch);
	Cvar_RegisterVariable (&randomclass);

	Cvar_RegisterVariable (&pausable);

	Cvar_RegisterVariable (&temp1);

	Host_FindMaxClients ();
}

/*
================
CON_Printf
================
*/
void CON_Printf (unsigned int flags, const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINTMSG];

	if (flags & _PRINT_DEVEL && !developer.integer)
	{
		if (con_debuglog & LOG_DEVEL)	/* full logging */
		{
			va_start (argptr, fmt);
			q_vsnprintf (msg, sizeof(msg), fmt, argptr);
			va_end (argptr);
			LOG_Print (msg);
		}
		return;
	}

	va_start (argptr, fmt);
	q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Sys_PrintTerm (msg);	// echo to the terminal
	if (con_debuglog)
		LOG_Print (msg);
}

/*
=================
SV_ClientPrintf

Sends text across to be displayed 
FIXME: make this just a stuffed echo?
=================
*/
void SV_ClientPrintf (unsigned int unused, const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	q_vsnprintf (string, sizeof (string), fmt, argptr);
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
void SV_BroadcastPrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	int			i;

	va_start (argptr,fmt);
	q_vsnprintf (string, sizeof (string), fmt, argptr);
	va_end (argptr);

	for (i = 0; i < svs.maxclients; i++)
	{
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			MSG_WriteByte (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
	}
}

/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	q_vsnprintf (string, sizeof (string), fmt, argptr);
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

		if (sv.active && host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			saveSelf = *sv_globals.self;
			*sv_globals.self = EDICT_TO_PROG(host_client->edict);
			PR_ExecuteProgram (*sv_globals.ClientDisconnect);
			*sv_globals.self = saveSelf;
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
	for (i = 0, client = svs.clients; i < svs.maxclients; i++, client++)
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

// flush any pending messages - like the score!!!
	start = Sys_DoubleTime();
	do
	{
		count = 0;
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
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
	count = NET_SendToAll (&buf, 5.0);
	if (count)
		Con_Printf("%s: NET_SendToAll failed for %d clients\n", __thisfunc__, count);

	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (host_client->active)
			SV_DropClient(crash);
	}

// clear structures
//	memset (&sv, 0, sizeof(sv)); // ServerSpawn already do this by Host_ClearMemory
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
	Mod_ClearAll ();
/* host_hunklevel MUST be set at this point */
	Hunk_FreeToLowMark (host_hunklevel);

	memset (&sv, 0, sizeof(sv));
}


//============================================================================


/*
===================
Host_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
static void Host_GetConsoleCommands (void)
{
	const char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}


/*
==================
Host_ServerFrame

==================
*/
static void Host_ServerFrame (void)
{
// run the world state
	*sv_globals.frametime = host_frametime;

// set the time and clear the general datagram
	SV_ClearDatagram ();

// check for new clients
	SV_CheckForNewClients ();

// read client messages
	SV_RunClients ();

// move things around and think
// always pause in single player if in console or menus
	if (!sv.paused)
		SV_Physics ();

// send all messages to the clients
	SV_SendClientMessages ();
}

/*
==================
Host_Frame

Runs all active servers
==================
*/
static void _Host_Frame (float time)
{
// keep the random time dependent
	rand ();

// decide the simulation time
	realtime += time;
	if (host_framerate.value > 0)
		host_frametime = host_framerate.value;
	else
	{
		host_frametime = time;
	// don't allow really long or short frames
		if (host_frametime > 0.05)
			host_frametime = 0.05;
		else if (host_frametime < 0.001)
			host_frametime = 0.001;
	}

// process console commands
	Cbuf_Execute ();

	NET_Poll();

// check for commands typed to the host
	Host_GetConsoleCommands ();

	if (sv.active)
		Host_ServerFrame ();
}

void Host_Frame (float time)
{
	double	time1, time2;
	static double	timetotal;
	static int	timecount;
	int		i, c, m;

	if (!serverprofile.integer)
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
	for (i = 0; i < svs.maxclients; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n", c, m);
}

//============================================================================

/*
====================
Host_Init
====================
*/
void Host_Init (void)
{
	Sys_Printf ("Host_Init\n");

	Memory_Init (host_parms->membase, host_parms->memsize);
	Cbuf_Init ();
	Cmd_Init ();
	COM_Init ();
	SV_Init ();
	FS_Init ();
	Host_RemoveGIPFiles(NULL);
	Host_InitLocal ();
	PR_Init ();
	Mod_Init ();
	NET_Init ();

	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
	Con_Printf ("%4.1f megabyte heap\n", host_parms->memsize/(1024*1024.0));

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	host_initialized = true;
	Con_Printf("\n===== Hexen II dedicated server initialized ======\n\n");

	Cvar_UnlockAll ();			/* unlock the early-set cvars after init */

	Cbuf_InsertText ("exec server.cfg\n");
	Cbuf_Execute ();

	Cmd_StuffCmds_f ();				/* process command line arguments */
	Cbuf_Execute ();

	if (!sv.active)		/* no map specified on the command line: spawn demo1.map */
		Cmd_ExecuteString ("map demo1", src_command);
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

	NET_Shutdown ();
	LOG_Close ();
}

