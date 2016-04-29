/*
 * cl_main.c -- hexenworld client main loop
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

#include "q_stdinc.h"
#include <setjmp.h>
#include "arch_def.h"
#if defined(PLATFORM_WINDOWS)
#include "winquake.h"
#endif
#include "quakedef.h"
#include "huffman.h"
#include "cfgfile.h"
#include "debuglog.h"
#include "bgmusic.h"
#include "cdaudio.h"

static	cvar_t	rcon_password = {"rcon_password", "", CVAR_NONE};
static	cvar_t	rcon_address = {"rcon_address", "", CVAR_NONE};

static	cvar_t	cl_timeout = {"cl_timeout", "60", CVAR_NONE};

cvar_t	cl_shownet = {"cl_shownet", "0", CVAR_NONE};	// can be 0, 1, or 2

cvar_t	cfg_unbindall = {"cfg_unbindall", "1", CVAR_ARCHIVE};

cvar_t	lookspring = {"lookspring", "0", CVAR_ARCHIVE};
cvar_t	lookstrafe = {"lookstrafe", "0", CVAR_ARCHIVE};
cvar_t	sensitivity = {"sensitivity", "3", CVAR_ARCHIVE};
cvar_t	mwheelthreshold = {"mwheelthreshold", "120", CVAR_ARCHIVE};

cvar_t	m_pitch = {"m_pitch", "0.022", CVAR_ARCHIVE};
cvar_t	m_yaw = {"m_yaw", "0.022", CVAR_NONE};
cvar_t	m_forward = {"m_forward", "1", CVAR_NONE};
cvar_t	m_side = {"m_side", "0.8", CVAR_NONE};

cvar_t	entlatency = {"entlatency", "20", CVAR_NONE};
cvar_t	cl_predict_players = {"cl_predict_players", "1", CVAR_NONE};
cvar_t	cl_predict_players2 = {"cl_predict_players2", "1", CVAR_NONE};
cvar_t	cl_solid_players = {"cl_solid_players", "1", CVAR_NONE};

//
// info mirrors
//
cvar_t	password = {"password", "", CVAR_USERINFO};
cvar_t	spectator = {"spectator", "", CVAR_USERINFO};
cvar_t	name = {"name", "unnamed", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	playerclass = {"playerclass", "1", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	team = {"team", "", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	skin = {"skin", "", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	topcolor = {"topcolor", "0", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	bottomcolor = {"bottomcolor", "0", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	rate = {"rate", "2500", CVAR_USERINFO|CVAR_ARCHIVE};	/* 5760 (72 fps) ? */
cvar_t	noaim = {"noaim", "0", CVAR_USERINFO|CVAR_ARCHIVE};
cvar_t	talksounds = {"talksounds", "1", CVAR_ARCHIVE};
cvar_t	msg = {"msg", "1", CVAR_USERINFO|CVAR_ARCHIVE};

extern	cvar_t		baseskin;
extern	cvar_t		noskins;

client_static_t	cls;
client_state_t	cl;

entity_state_t	cl_baselines[MAX_EDICTS];
efrag_t		cl_efrags[MAX_EFRAGS];
entity_t	cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t	cl_dlights[MAX_DLIGHTS];

// refresh list
// this is double buffered so the last frame
// can be scanned for oldorigins of trailing objects
int		cl_numvisedicts, cl_oldnumvisedicts;
entity_t	*cl_visedicts, *cl_oldvisedicts;
entity_t	cl_visedicts_list[2][MAX_VISEDICTS];

static double	connect_time = -1;	// for connection retransmits

quakeparms_t	*host_parms;

qboolean	host_initialized;	// true if into command execution

static jmp_buf	host_abort;

double		host_frametime;
double		realtime;		// without any filtering or bounding
static double	oldrealtime;		// last frame run
int		host_framecount;
static int	host_hunklevel;

byte		*host_basepal;
byte		*host_colormap;

netadr_t	master_adr;			// address of the master server

static	cvar_t	host_speeds = {"host_speeds", "0", CVAR_NONE};	// set for running times
cvar_t	developer = {"developer", "0", CVAR_NONE};

float	server_version = 0;	// version of server we connected to

//
// globals for Siege:
//
qboolean	cl_siege;	// whether this is a Siege game
byte		cl_fraglimit;
float		cl_timelimit;
float		cl_server_time_offset;
int		cl_keyholder = -1;
int		cl_doc = -1;	// Defender of Crown (Excalibur)
unsigned int	defLosses;	// Defender losses
unsigned int	attLosses;	// Attacker losses


static void CL_Callback_Userinfo (cvar_t *var)
{
	Info_SetValueForKey (cls.userinfo, var->name, var->string, MAX_INFO_STRING);
	if (cls.state >= ca_connected)
	{
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, va("setinfo \"%s\" \"%s\"\n", var->name, var->string));
	}
}

/*
==================
CL_Quit_f
==================
*/
static void CL_Quit_f (void)
{
	if (1 /* Key_GetDest() != key_console && cls.state == ca_active */)
	{
		M_Menu_Quit_f ();
		return;
	}
	CL_Disconnect ();
	Sys_Quit ();
}

/*
=======================
CL_Version_f
======================
*/
static void CL_Version_f (void)
{
	Con_Printf ("Version %4.2f\n", ENGINE_VERSION);
	Con_Printf ("Exe: " __TIME__ " " __DATE__ "\n");
}


/*
=======================
CL_SendConnectPacket

called by CL_Connect_f and CL_CheckResend
======================
*/
extern qboolean menu_disabled_mouse;
void CL_SendConnectPacket (void)
{
	netadr_t	adr;
	char	data[2048];
	double t1, t2;
// JACK: Fixed bug where DNS lookups would cause two connects real fast
//	 Now, adds lookup time to the connect time.
//	 Should I add it to realtime instead?!?!

	t1 = Sys_DoubleTime ();

	if (!NET_StringToAdr (cls.servername, &adr))
	{
		Con_Printf ("Bad server address\n");
		connect_time = -1;
		return;
	}

	if (adr.port == 0)
		adr.port = BigShort (PORT_SERVER);

	t2 = Sys_DoubleTime ();

	connect_time = realtime + t2 - t1;	// for retransmit requests

	Con_Printf ("Connecting to %s...\n", cls.servername);
	q_snprintf (data, sizeof(data), "%c%c%c%cconnect %d \"%s\"\n",
			255, 255, 255, 255,
			((gameflags & GAME_PORTALS) == GAME_PORTALS),
			cls.userinfo);
	NET_SendPacket (strlen(data), data, &adr);

	// When we connect to a server, check the mouse is going - S.A.
	menu_disabled_mouse = false;
	IN_ActivateMouse();
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out

=================
*/
static void CL_CheckForResend (void)
{
	if (connect_time == -1)
		return;
	if (cls.state != ca_disconnected)
		return;
	if (realtime - connect_time > 5.0)
		CL_SendConnectPacket ();
}


/*
================
CL_Connect_f

================
*/
static void CL_Connect_f (void)
{
	const char	*server;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("usage: connect <server>\n");
		return;	
	}

	server = Cmd_Argv (1);

	CL_Disconnect ();

	Key_SetDest (key_game);		// remove console or menu
	q_strlcpy (cls.servername, server, sizeof(cls.servername));
	CL_SendConnectPacket ();
}


/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
static void CL_Rcon_f (void)
{
	char	message[1024];
	int		i;
	netadr_t	to;

	if (!rcon_password.string)
	{
		Con_Printf ("You must set 'rcon_password' before\n"
					"issuing an rcon command.\n");
		return;
	}

	message[0] = 255;
	message[1] = 255;
	message[2] = 255;
	message[3] = 255;
	message[4] = 0;

	q_strlcat (message, "rcon ", sizeof(message));

	q_strlcat (message, rcon_password.string, sizeof(message));
	q_strlcat (message, " ", sizeof(message));

	for (i = 1; i < Cmd_Argc(); i++)
	{
		q_strlcat (message, Cmd_Argv(i), sizeof(message));
		q_strlcat (message, " ", sizeof(message));
	}

	if (cls.state >= ca_connected)
	{
		to = cls.netchan.remote_address;
	}
	else
	{
		if (rcon_address.string[0] == '\0')
		{
			Con_Printf ("You must either be connected,\n"
					"or set the 'rcon_address' cvar\n"
					"to issue rcon commands\n");

			return;
		}
		NET_StringToAdr (rcon_address.string, &to);
		if (to.port == 0)
		{
			to.port = BigShort (PORT_SERVER);
		}
	}

	NET_SendPacket (strlen(message)+1, message, &to);
}


/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	int	i;

	S_StopAllSounds (true);

	Con_DPrintf ("Clearing memory\n");
	D_FlushCaches ();
	Mod_ClearAll ();
/* host_hunklevel MUST be set at this point */
	Hunk_FreeToLowMark (host_hunklevel);

	CL_ClearTEnts ();
	CL_ClearEffects();

// wipe the entire cl structure
	memset (&cl, 0, sizeof(cl));

	SZ_Clear (&cls.netchan.message);

// clear other arrays
	memset (cl_efrags, 0, sizeof(cl_efrags));
	memset (cl_dlights, 0, sizeof(cl_dlights));
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));

//
// allocate the efrags and chain together into a free list
//
	cl.free_efrags = cl_efrags;
	for (i = 0; i < MAX_EFRAGS-1; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;

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
	byte	final[10];

	connect_time = -1;

#ifdef PLATFORM_WINDOWS
	SetWindowText (mainwindow, "HexenWorld: disconnected");
#endif

// don't get stuck in chat mode
	if (Key_GetDest() == key_message)
		Key_EndChat ();

// reset any active palette shifts (see view.c:V_UpdatePalette())
	memset (cl.cshifts, 0, sizeof(cl.cshifts));

// no more siege display, etc.
	cl_siege = false;

// stop sounds (especially looping!)
	S_StopAllSounds (true);
	BGM_Stop();
	CDAudio_Stop();

// if running a local server, shut it down
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
	}
	else if (cls.state != ca_disconnected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		final[0] = clc_stringcmd;
		strcpy ((char *) &final[1], "drop");
		Netchan_Transmit (&cls.netchan, 6, final);
		Netchan_Transmit (&cls.netchan, 6, final);
		Netchan_Transmit (&cls.netchan, 6, final);

		cls.state = ca_disconnected;

		cls.demoplayback = cls.demorecording = cls.timedemo = false;
	}
	Cam_Reset();
	cl.intermission = 0;
}

static void CL_Disconnect_f (void)
{
	CL_Disconnect ();
}

/*
=====================
CL_Map_f

prints the current map and level names
=====================
*/
static void CL_Map_f (void)
{
	if (Cmd_Argc() > 1)
		Con_Printf ("only a server can start or change a map\n");
	else if (cls.state == ca_active)
		Con_Printf ("Current level: %s [ %s ]\n", cl.levelname, cl.mapname);
	else
		Con_Printf ("Not connected to a server\n");
}

/*
====================
CL_User_f

user <name or userid>

Dump userdata / masterdata for a user
====================
*/
static void CL_User_f (void)
{
	int	i, uid;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: user <username / userid>\n");
		return;
	}

	uid = atoi(Cmd_Argv(1));

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.players[i].name[0])
			continue;
		if (cl.players[i].userid == uid
				|| !strcmp(cl.players[i].name, Cmd_Argv(1)) )
		{
			Info_Print (cl.players[i].userinfo);
			return;
		}
	}
	Con_Printf ("User not in server.\n");
}

/*
====================
CL_Users_f

Dump userids for all current players
====================
*/
static void CL_Users_f (void)
{
	int	i, c;

	c = 0;
	Con_Printf ("userid frags name\n");
	Con_Printf ("------ ----- ----\n");
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (cl.players[i].name[0])
		{
			Con_Printf ("%6i %4i %s\n", cl.players[i].userid, cl.players[i].frags, cl.players[i].name);
			c++;
		}
	}

	Con_Printf ("%i total users\n", c);
}

static void CL_Color_f (void)
{
	// just for quake compatability...
	int	top, bottom;
	char	num[16];

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"color\" is \"%s %s\"\n",
			Info_ValueForKey (cls.userinfo, "topcolor"),
			Info_ValueForKey (cls.userinfo, "bottomcolor") );
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (Cmd_Argc() == 2)
	{
		top = bottom = atoi(Cmd_Argv(1));
	}
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

	q_snprintf (num, sizeof(num), "%i", top);
	Cvar_SetQuick (&topcolor, num);
	q_snprintf (num, sizeof(num), "%i", bottom);
	Cvar_SetQuick (&bottomcolor, num);
}

/*
==================
CL_FullServerinfo_f

Sent by server when serverinfo changes
==================
*/
static void CL_FullServerinfo_f (void)
{
	const char	*p;
	float	v;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("usage: fullserverinfo <complete info string>\n");
		return;
	}

	q_strlcpy (cl.serverinfo, Cmd_Argv(1), MAX_SERVERINFO_STRING);

	p = Info_ValueForKey(cl.serverinfo, "*version");
	if (*p)
	{
		v = atof(p);
		if (v)
		{
			if (!server_version)
				Con_Printf("Version %1.2f Server\n", v);
			server_version = v;
		}
	}
}

/*
==================
CL_FullInfo_f

Allow clients to change userinfo
==================
*/
static void CL_FullInfo_f (void)
{
	char	key[512];
	char	value[512];
	char		*o;
	const char	*s;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("fullinfo <complete info string>\n");
		return;
	}

	s = Cmd_Argv(1);
	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (!*s)
		{
			Con_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;

		Info_SetValueForKey (cls.userinfo, key, value, MAX_INFO_STRING);
	}
}

/*
==================
CL_SetInfo_f

Allow clients to change userinfo
==================
*/
static void CL_SetInfo_f (void)
{
	if (Cmd_Argc() == 1)
	{
		Info_Print (cls.userinfo);
		return;
	}
	if (Cmd_Argc() != 3)
	{
		Con_Printf ("usage: setinfo [ <key> <value> ]\n");
		return;
	}
	if (!q_strcasecmp(Cmd_Argv(1), "pmodel") || !strcmp(Cmd_Argv(1), "emodel"))
		return;

	Info_SetValueForKey (cls.userinfo, Cmd_Argv(1), Cmd_Argv(2), MAX_INFO_STRING);
	if (cls.state >= ca_connected)
		Cmd_ForwardToServer ();
}

/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
static void CL_Packet_f (void)
{
	char	senddata[2048];
	int		i, l;
	const char	*in;
	char		*out;
	netadr_t	adr;

	if (Cmd_Argc() != 3)
	{
		Con_Printf ("packet <destination> <contents>\n");
		return;
	}

	if (!NET_StringToAdr (Cmd_Argv(1), &adr))
	{
		Con_Printf ("Bad address\n");
		return;
	}

	in = Cmd_Argv(2);
	out = senddata + 4;
	senddata[0] = senddata[1] = senddata[2] = senddata[3] = 0xff;

	l = strlen (in);
	for (i = 0; i < l; i++)
	{
		if (in[i] == '\\' && in[i+1] == 'n')
		{
			*out++ = '\n';
			i++;
		}
		else
		{
			*out++ = in[i];
		}
	}
	*out = 0;

	NET_SendPacket (out-senddata, senddata, &adr);
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
		return;	// don't play demos

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
//			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			return;
		}
	}

	q_snprintf (str, sizeof(str),"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}


/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
static void CL_Changing_f (void)
{
	S_StopAllSounds (true);
	cl.intermission = 0;
	cls.state = ca_connected;	// not active anymore, but not disconnected
	Con_Printf ("\nChanging map...\n");
}


/*
=================
CL_Reconnect_f

The server is changing levels
=================
*/
static void CL_Reconnect_f (void)
{
	S_StopAllSounds (true);

	if (cls.state == ca_connected)
	{
		Con_Printf ("reconnecting...\n");
		MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "new");
		return;
	}

	if (!*cls.servername)
	{
		Con_Printf("No server to reconnect to...\n");
		return;
	}

	CL_Disconnect();
	CL_SendConnectPacket ();
}

/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
static void CL_ConnectionlessPacket (void)
{
	const char	*s;
	int	c;

	MSG_BeginReading ();
	MSG_ReadLong ();	// skip the -1

	c = MSG_ReadByte ();
	if (!cls.demoplayback)
		Con_Printf ("%s:\n", NET_AdrToString (&net_from));
	Con_DPrintf ("%s", net_message.data + 5);
	if (c == S2C_CONNECTION)
	{
		if (cls.state == ca_connected)
		{
			if (!cls.demoplayback)
				Con_Printf ("Dup connect received.  Ignored.\n");
			return;
		}
		Netchan_Setup (&cls.netchan, &net_from);
		MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "new");
		cls.state = ca_connected;
		Con_Printf ("Connected.\n");
		return;
	}
	// remote command from gui front end
	if (c == A2C_CLIENT_COMMAND)
	{
		if (!NET_CompareBaseAdr(&net_from, &net_local_adr)
			&& !NET_CompareBaseAdr(&net_from, &net_loopback_adr))
		{
			Con_Printf ("Command packet from remote host. Ignored.\n");
			return;
		}
#ifdef PLATFORM_WINDOWS
		ShowWindow (mainwindow, SW_RESTORE);
		SetForegroundWindow (mainwindow);
#endif
		s = MSG_ReadString ();
		Cbuf_AddText (s);
		return;
	}
	// print command from somewhere
	if (c == A2C_PRINT)
	{
		s = MSG_ReadString ();
		Con_Printf ("%s", s);
		return;
	}

	// ping from somewhere
	if (c == A2A_PING)
	{
		byte	data[6];

		data[0] = 0xff;
		data[1] = 0xff;
		data[2] = 0xff;
		data[3] = 0xff;
		data[4] = A2A_ACK;
		data[5] = 0;

		NET_SendPacket (6, data, &net_from);
		return;
	}

	if (c == svc_disconnect)
	{
		Host_EndGame ("Server disconnected\n");
		return;
	}

	Con_Printf ("Unknown command:\n%c\n", c);
}


/*
=================
CL_ReadPackets
=================
*/
static void CL_ReadPackets (void)
{
//	while (NET_GetPacket())
	while (CL_GetMessage())
	{
		// remote command packet
		if (*(int *)net_message.data == -1)
		{
			CL_ConnectionlessPacket ();
			continue;
		}

		if (net_message.cursize < 8)
		{
			Con_Printf ("%s: Runt packet\n",NET_AdrToString(&net_from));
			continue;
		}

		// packet from server
		if (!cls.demoplayback && 
			!NET_CompareAdr (&net_from, &cls.netchan.remote_address))
		{
			Con_Printf ("%s: sequenced packet without connection\n",
						NET_AdrToString(&net_from));
			continue;
		}
		if (!Netchan_Process(&cls.netchan))
			continue;	// wasn't accepted for some reason
		CL_ParseServerMessage ();
	}

	// check timeout
	if (cls.state >= ca_connected
		 && realtime - cls.netchan.last_received > cl_timeout.value)
	{
		Con_Printf ("\nServer connection timed out.\n");
		CL_Disconnect ();
		return;
	}
}

//=============================================================================

/*
=====================
CL_Download_f
=====================
*/
static void CL_Download_f (void)
{
	char	tmp[MAX_OSPATH];
	int	err;

	if (cls.state == ca_disconnected)
	{
		Con_Printf ("Must be connected.\n");
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: download <datafile>\n");
		return;
	}

	if (strstr(Cmd_Argv(1), ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}

	FS_MakePath_BUF (FS_USERDIR, &err, tmp, sizeof(tmp), Cmd_Argv(1));
	if (err)
	{
		Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
		return;
	}

	if (FS_CreatePath(tmp) != 0)
	{
		Con_Printf ("Unable to create directory for downloading %s\n", Cmd_Argv(1));
		return;
	}

	cls.download = fopen (tmp, "wb");
	if (!cls.download)
	{
		Con_Printf ("Unable to create file for downloading %s\n", Cmd_Argv(1));
		return;
	}

	// don't use the full user path in order to avoid rename failed messages
	q_strlcpy (cls.downloadname, Cmd_Argv(1), MAX_OSPATH);
	q_strlcpy (cls.downloadtempname, Cmd_Argv(1), MAX_OSPATH);
	cls.downloadtype = dl_single;

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	SZ_Print (&cls.netchan.message, va("download %s\n",Cmd_Argv(1)));
}

#ifdef _WINDOWS
/*
=================
CL_Minimize_f
=================
*/
static void CL_Windows_f (void)
{
	SendMessage(mainwindow, WM_SYSKEYUP, VK_TAB, 1 | (0x0F << 16) | (1<<29));
}
#endif

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
static void Host_SaveConfig_f (void);
void CL_Init (void)
{
	cls.state = ca_disconnected;

	Info_SetValueForKey (cls.userinfo, "name", "unnamed", MAX_INFO_STRING);
	Info_SetValueForKey (cls.userinfo, "playerclass", "1", MAX_INFO_STRING);
	Info_SetValueForKey (cls.userinfo, "topcolor", "0", MAX_INFO_STRING);
	Info_SetValueForKey (cls.userinfo, "bottomcolor", "0", MAX_INFO_STRING);
	Info_SetValueForKey (cls.userinfo, "rate", "2500", MAX_INFO_STRING);
	Info_SetValueForKey (cls.userinfo, "msg", "1", MAX_INFO_STRING);

	// capabilities info (single char flags) -- adapted from QuakeForge:
	// c: chunked connection sequence for sound/modellists (protocol 26)
	Info_SetValueForStarKey (cls.userinfo, "*cap", "c", MAX_INFO_STRING);

	CL_InitInput ();
	CL_InitTEnts ();
	CL_InitPrediction ();
	CL_InitEffects ();
	CL_InitCam ();
	Pmove_Init ();

// register our commands
	Cmd_AddCommand ("saveconfig", Host_SaveConfig_f);

	Cvar_RegisterVariable (&developer);
	if (COM_CheckParm("-developer"))
	{
		Cvar_Set ("developer", "1");
		Cvar_LockVar ("developer");
	}

	Cvar_RegisterVariable (&sys_throttle);

	Cvar_RegisterVariable (&host_speeds);

	Cvar_RegisterVariable (&cl_upspeed);
	Cvar_RegisterVariable (&cl_forwardspeed);
	Cvar_RegisterVariable (&cl_backspeed);
	Cvar_RegisterVariable (&cl_sidespeed);
	Cvar_RegisterVariable (&cl_movespeedkey);
	Cvar_RegisterVariable (&cl_yawspeed);
	Cvar_RegisterVariable (&cl_pitchspeed);
	Cvar_RegisterVariable (&cl_anglespeedkey);
	Cvar_RegisterVariable (&cl_shownet);
	Cvar_RegisterVariable (&cl_timeout);
	Cvar_RegisterVariable (&lookspring);
	Cvar_RegisterVariable (&lookstrafe);
	Cvar_RegisterVariable (&sensitivity);
	Cvar_RegisterVariable (&mwheelthreshold);

	Cvar_RegisterVariable (&m_pitch);
	Cvar_RegisterVariable (&m_yaw);
	Cvar_RegisterVariable (&m_forward);
	Cvar_RegisterVariable (&m_side);

	Cvar_RegisterVariable (&rcon_password);
	Cvar_RegisterVariable (&rcon_address);

	Cvar_RegisterVariable (&entlatency);
	Cvar_RegisterVariable (&cl_predict_players2);
	Cvar_RegisterVariable (&cl_predict_players);
	Cvar_RegisterVariable (&cl_solid_players);

	Cvar_RegisterVariable (&baseskin);
	Cvar_RegisterVariable (&noskins);

	Cvar_RegisterVariable (&cfg_unbindall);

// info mirrors
	Cvar_SetCallback (&name, CL_Callback_Userinfo);
	Cvar_SetCallback (&playerclass, CL_Callback_Userinfo);
	Cvar_SetCallback (&password, CL_Callback_Userinfo);
	Cvar_SetCallback (&spectator, CL_Callback_Userinfo);
	Cvar_SetCallback (&skin, CL_Callback_Userinfo);
	Cvar_SetCallback (&team, CL_Callback_Userinfo);
	Cvar_SetCallback (&topcolor, CL_Callback_Userinfo);
	Cvar_SetCallback (&bottomcolor, CL_Callback_Userinfo);
	Cvar_SetCallback (&rate, CL_Callback_Userinfo);
	Cvar_SetCallback (&msg, CL_Callback_Userinfo);
	Cvar_SetCallback (&noaim, CL_Callback_Userinfo);
	Cvar_RegisterVariable (&name);
	Cvar_RegisterVariable (&playerclass);
	Cvar_RegisterVariable (&password);
	Cvar_RegisterVariable (&spectator);
	Cvar_RegisterVariable (&skin);
	Cvar_RegisterVariable (&team);
	Cvar_RegisterVariable (&topcolor);
	Cvar_RegisterVariable (&bottomcolor);
	Cvar_RegisterVariable (&rate);
	Cvar_RegisterVariable (&msg);
	Cvar_RegisterVariable (&noaim);
	Cvar_RegisterVariable (&talksounds);

	Cmd_AddCommand ("version", CL_Version_f);

	Cmd_AddCommand ("changing", CL_Changing_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("rerecord", CL_ReRecord_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("map", CL_Map_f);

	Cmd_AddCommand ("skins", Skin_Skins_f);
	Cmd_AddCommand ("allskins", Skin_AllSkins_f);

	Cmd_AddCommand ("quit", CL_Quit_f);

	Cmd_AddCommand ("connect", CL_Connect_f);
	Cmd_AddCommand ("reconnect", CL_Reconnect_f);

	Cmd_AddCommand ("rcon", CL_Rcon_f);
	Cmd_AddCommand ("packet", CL_Packet_f);
	Cmd_AddCommand ("user", CL_User_f);
	Cmd_AddCommand ("users", CL_Users_f);

	Cmd_AddCommand ("setinfo", CL_SetInfo_f);
	Cmd_AddCommand ("fullinfo", CL_FullInfo_f);
	Cmd_AddCommand ("fullserverinfo", CL_FullServerinfo_f);

	Cmd_AddCommand ("color", CL_Color_f);
	Cmd_AddCommand ("download", CL_Download_f);

	Cmd_AddCommand ("sensitivity_save", CL_Sensitivity_save_f);

// forward to server commands
	Cmd_AddCommand ("kill", NULL);
	Cmd_AddCommand ("say", NULL);
	Cmd_AddCommand ("say_team", NULL);
	Cmd_AddCommand ("serverinfo", NULL);

// Windows commands
#ifdef _WINDOWS
	Cmd_AddCommand ("windows", CL_Windows_f);
#endif
}


/*
================
Host_EndGame

Call this to drop to a console without exiting the qwcl
Does not return due longjmp()
================
*/
void Host_EndGame (const char *message, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,message);
	q_vsnprintf (string, sizeof(string), message, argptr);
	va_end (argptr);
	Con_Printf ("\n===========================\n");
	Con_Printf ("%s: %s\n", __thisfunc__, string);
	Con_Printf ("===========================\n\n");

	CL_Disconnect ();

	longjmp (host_abort, 1);
}

/*
================
Host_Error

This shuts down the client and exits qwcl
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

	CL_Disconnect ();
	cls.demonum = -1;

	inerror = false;

// FIXME
	Sys_Error ("%s: %s\n", __thisfunc__, string);
}


/*
===============
Host_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
void Host_WriteConfiguration (const char *fname)
{
	FILE	*f;

	if (host_initialized)
	{
		f = fopen (FS_MakePath(FS_USERDIR,NULL,fname), "w");
		if (!f)
		{
			Con_Printf ("Couldn't write %s.\n",fname);
			return;
		}

		Key_WriteBindings (f);
		Cvar_WriteVariables (f);
		// if mlook was down, keep it that way:
		if (in_mlook.state & 1)
			fprintf (f, "+mlook\n");

		fclose (f);
	}
}

static void Host_SaveConfig_f (void)
{
	const char	*p;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("saveconfig <savename> : save a config file\n");
		return;
	}

	p = Cmd_Argv(1);
	if (*p == '.' || strstr(p, ".."))
	{
		Con_Printf ("Invalid config name.\n");
		return;
	}

	Host_WriteConfiguration (p);
}

//============================================================================

/*
==================
Host_Frame

Runs all active servers
==================
*/
void Host_Frame (float time)
{
	static double		time1 = 0;
	static double		time2 = 0;
	static double		time3 = 0;
	int			pass1, pass2, pass3;
	float			fps;

	if (setjmp(host_abort))
		return;	// something bad happened, or the server disconnected

	// decide the simulation time
	realtime += time;
	if (oldrealtime > realtime)
		oldrealtime = 0;

	fps = q_max(30.0, q_min(rate.value/80.0, 72.0));

	if (!cls.timedemo && realtime - oldrealtime < 1.0/fps)
		return;	// framerate is too high
	host_frametime = realtime - oldrealtime;
	oldrealtime = realtime;
	if (host_frametime > 0.2)
		host_frametime = 0.2;

	// get new key events
	Sys_SendKeyEvents ();

	// allow mice or other external controllers to add commands
	IN_Commands ();

	// process console commands
	Cbuf_Execute ();

	// fetch results from server
	CL_ReadPackets ();

	// send intentions now
	// resend a connection request if necessary
	if (cls.state == ca_disconnected)
	{
		CL_CheckForResend ();
	}
	else
	{
		CL_SendCmd ();
	}

	// Set up prediction for other players
	CL_SetUpPlayerPrediction(false);

	// do client side motion prediction
	CL_PredictMove ();

	// Set up prediction for other players
	CL_SetUpPlayerPrediction(true);

	// build a refresh entity list
	CL_EmitEntities ();

	// update video
	if (host_speeds.integer)
		time1 = Sys_DoubleTime ();

	SCR_UpdateScreen ();

	if (host_speeds.integer)
		time2 = Sys_DoubleTime ();

	// update audio
	BGM_Update();	// adds music raw samples and/or advances midi driver
	if (cls.state == ca_active)
	{
		S_Update (r_origin, vpn, vright, vup);
		CL_DecayLights ();
	}
	else
	{
		S_Update (vec3_origin, vec3_origin, vec3_origin, vec3_origin);
	}

	CDAudio_Update();

	if (host_speeds.integer)
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
	HuffInit ();
	Cbuf_Init ();
	Cmd_Init ();

	Cvar_RegisterVariable (&sys_nostdout);
	COM_Init ();
	FS_Init ();
	CL_Cmd_Init ();

	NET_Init (PORT_CLIENT);
	Netchan_Init ();

	CFG_OpenConfig ("config.cfg");

	Mod_Init ();

	Con_Printf ("Exe: " __TIME__ " " __DATE__ "\n");
	Con_Printf ("%4.1f megs RAM used.\n", host_parms->memsize/(1024*1024.0));

	R_InitTextures ();

	V_Init ();
	W_LoadWadFile ("gfx.wad");
	Key_Init ();
	Con_Init ();
	M_Init ();

	host_basepal = (byte *)FS_LoadHunkFile ("gfx/palette.lmp", NULL);
	if (!host_basepal)
		Sys_Error ("Couldn't load gfx/palette.lmp");

	host_colormap = (byte *)FS_LoadHunkFile ("gfx/colormap.lmp", NULL);
	if (!host_colormap)
		Sys_Error ("Couldn't load gfx/colormap.lmp");

	VID_Init (host_basepal);
	Draw_Init ();
	SCR_Init ();
	R_Init ();
	Sbar_Init ();

	cls.state = ca_disconnected;

	S_Init ();
	CDAudio_Init ();
	MIDI_Init ();
	BGM_Init ();

	CL_Init ();
	IN_Init ();

	CFG_CloseConfig();

#ifdef GLQUAKE
/* analogous to host_hunklevel, this will mark OpenGL texture
 * beyond which everything will need to be purged on new map */
	gl_texlevel = numgltextures;
#endif

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	host_initialized = true;
	Con_Printf ("\n======= HexenWorld Initialized ========\n\n");

/* execute the hexen.rc file: a valid file runs default.cfg, config.cfg
 * and autoexec.cfg in this order, then processes the cmdline arguments
 * by sending "stuffcmds". */
	Cbuf_InsertText ("exec hexen.rc\n");
	if (!setjmp(host_abort))		/* in case exec fails with a longjmp(), e.g. Host_Error() */
		Cbuf_Execute ();

	Cvar_UnlockAll ();			/* unlock the early-set cvars after init */
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

	Host_WriteConfiguration ("config.cfg"); 

	BGM_Shutdown ();
	CDAudio_Shutdown ();
	MIDI_Cleanup ();
	NET_Shutdown ();
	S_Shutdown();
	IN_Shutdown ();
	if (host_basepal)
		VID_Shutdown();
	LOG_Close ();
}

