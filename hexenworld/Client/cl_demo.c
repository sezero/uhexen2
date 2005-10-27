
#include "quakedef.h"

void CL_FinishTimeDemo (void);

/*
==============================================================================

DEMO CODE

When a demo is playing back, all NET_SendMessages are skipped, and
NET_GetMessages are read from the demo file.

Whenever cl.time gets past the last received message, another message is
read from the demo file.
==============================================================================
*/

/*
==============
CL_StopPlayback

Called when a demo file runs out, or the user starts a game
==============
*/
void CL_StopPlayback (void)
{
	if (!cls.demoplayback)
		return;

	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.state = ca_disconnected;
	cls.demoplayback = 0;

	if (cls.timedemo)
		CL_FinishTimeDemo ();
}

#define dem_cmd		0
#define dem_read	1

/*
====================
CL_WriteDemoCmd

Writes the current user cmd
====================
*/
void CL_WriteDemoCmd (usercmd_t *pcmd)
{
	int	i;
	float	fl;
	byte	c;
	usercmd_t cmd;

	//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	fl = LittleFloat((float)realtime);
	fwrite (&fl, sizeof(fl), 1, cls.demofile);

	c = dem_cmd;
	fwrite (&c, sizeof(c), 1, cls.demofile);

	// correct for byte order, bytes don't matter
	cmd = *pcmd;

	for (i = 0; i < 3; i++)
		cmd.angles[i] = LittleFloat(cmd.angles[i]);
	cmd.forwardmove = LittleShort(cmd.forwardmove);
	cmd.sidemove    = LittleShort(cmd.sidemove);
	cmd.upmove      = LittleShort(cmd.upmove);

	fwrite(&cmd, sizeof(cmd), 1, cls.demofile);

	for (i=0 ; i<3 ; i++)
	{
		fl = LittleFloat (cl.viewangles[i]);
		fwrite (&fl, 4, 1, cls.demofile);
	}

	fflush (cls.demofile);
}

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
void CL_WriteDemoMessage (sizebuf_t *msg)
{
	int	len;
	float	fl;
	byte	c;

	//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	if (!cls.demorecording)
		return;

	fl = LittleFloat((float)realtime);
	fwrite (&fl, sizeof(fl), 1, cls.demofile);

	c = dem_read;
	fwrite (&c, sizeof(c), 1, cls.demofile);

	len = LittleLong (msg->cursize);
	fwrite (&len, 4, 1, cls.demofile);
	fwrite (msg->data, msg->cursize, 1, cls.demofile);

	fflush (cls.demofile);
}

/*
====================
CL_GetDemoMessage

  FIXME...
====================
*/
qboolean CL_GetDemoMessage (void)
{
	int	r, i, j;
	float	f;
	float	demotime;
	byte	c;
	usercmd_t *pcmd;

	// read the time from the packet
	fread(&demotime, sizeof(demotime), 1, cls.demofile);
	demotime = LittleFloat(demotime);

	// decide if it is time to grab the next message
	if (cls.timedemo)
	{
		if (cls.td_lastframe < 0)
		{
			cls.td_lastframe = demotime;
		}
		else if (demotime > cls.td_lastframe)
		{
			cls.td_lastframe = demotime;
			// rewind back to time
			fseek(cls.demofile, ftell(cls.demofile) - sizeof(demotime), SEEK_SET);
			return 0;	// already read this frame's message
		}

		if (!cls.td_starttime && cls.state == ca_active)
		{
			cls.td_starttime = Sys_DoubleTime();
			cls.td_startframe = host_framecount;
		}
		realtime = demotime; // warp
	}
	else if (cls.state >= ca_onserver)
	{	// always grab until fully connected
		if (realtime + 1.0 < demotime)
		{
			// too far back
			realtime = demotime - 1.0;
			// rewind back to time
			fseek(cls.demofile, ftell(cls.demofile) - sizeof(demotime), SEEK_SET);
			return 0;
		}
		else if (realtime < demotime)
		{
			// rewind back to time
			fseek(cls.demofile, ftell(cls.demofile) - sizeof(demotime), SEEK_SET);
			return 0;	// don't need another message yet
		}
	}
	else
	{
		realtime = demotime; // we're warping
	}

	if (cls.state < ca_demostart)
		Host_Error ("CL_GetDemoMessage: cls.state != ca_active");

	// get the msg type
	fread (&c, sizeof(c), 1, cls.demofile);

	switch (c)
	{
	case dem_cmd :
		// user sent input
		i = cls.netchan.outgoing_sequence & UPDATE_MASK;
		pcmd = &cl.frames[i].cmd;
		r = fread (pcmd, sizeof(*pcmd), 1, cls.demofile);
		if (r != 1)
		{
			CL_StopPlayback ();
			return 0;
		}
		// byte order stuff
		for (j = 0; j < 3; j++)
			pcmd->angles[j] = LittleFloat(pcmd->angles[j]);
		pcmd->forwardmove = LittleShort(pcmd->forwardmove);
		pcmd->sidemove    = LittleShort(pcmd->sidemove);
		pcmd->upmove      = LittleShort(pcmd->upmove);
		cl.frames[i].senttime = demotime;
		cl.frames[i].receivedtime = -1;	// we haven't gotten a reply yet
		cls.netchan.outgoing_sequence++;
		for (i=0 ; i<3 ; i++)
		{
			r = fread (&f, 4, 1, cls.demofile);
			cl.viewangles[i] = LittleFloat (f);
		}
		break;

	case dem_read:
		// get the next message
		fread (&net_message.cursize, 4, 1, cls.demofile);
		net_message.cursize = LittleLong (net_message.cursize);
		//Con_Printf("read: %ld bytes\n", net_message.cursize);
		if (net_message.cursize > MAX_MSGLEN)
			Sys_Error ("Demo message > MAX_MSGLEN");
		r = fread (net_message.data, net_message.cursize, 1, cls.demofile);
		if (r != 1)
		{
			CL_StopPlayback ();
			return 0;
		}
		break;

	default :
		Con_Printf("Corrupted demo.\n");
		CL_StopPlayback ();
		return 0;
	}

	return 1;
}

/*
====================
CL_GetMessage

Handles recording and playback of demos, on top of NET_ code
====================
*/
qboolean CL_GetMessage (void)
{
	if (cls.demoplayback)
		return CL_GetDemoMessage ();

	if (!NET_GetPacket ())
		return false;

	CL_WriteDemoMessage (&net_message);

	return true;
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop_f (void)
{
	if (!cls.demorecording)
	{
		Con_Printf ("Not recording a demo.\n");
		return;
	}

// write a disconnect message to the demo file
	SZ_Clear (&net_message);
	MSG_WriteLong (&net_message, -1);	// -1 sequence means out of band
	MSG_WriteByte (&net_message, svc_disconnect);
	MSG_WriteString (&net_message, "EndOfDemo");
	CL_WriteDemoMessage (&net_message);

// finish up
	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.demorecording = false;
	Con_Printf ("Completed demo\n");
}

/*
====================
CL_Record_f

record <demoname> <server>
====================
*/
void CL_Record_f (void)
{
	int	c;
	char	name[MAX_OSPATH];

	c = Cmd_Argc();
	if (c != 3)
	{
		Con_Printf ("record <demoname> <server>\n");
		return;
	}

	if (cls.demorecording)
		CL_Stop_f();

	sprintf (name, "%s/%s", com_userdir, Cmd_Argv(1));

//
// open the demo file
//
	COM_DefaultExtension (name, ".qwd");

	cls.demofile = fopen (name, "wb");
	if (!cls.demofile)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	if (cls.state != ca_disconnected)
		CL_Disconnect();

	Con_Printf ("recording to %s.\n", name);
	cls.demorecording = true;

//
// start the map up
//
	Cmd_ExecuteString ( va("connect %s", Cmd_Argv(2)));
}

/*
====================
CL_ReRecord_f

record <demoname>
====================
*/
void CL_ReRecord_f (void)
{
	int	c;
	char	name[MAX_OSPATH];

	c = Cmd_Argc();
	if (c != 2)
	{
		Con_Printf ("rerecord <demoname>\n");
		return;
	}

	if (!*cls.servername)
	{
		Con_Printf("No server to reconnect to...\n");
		return;
	}

	if (cls.demorecording)
		CL_Stop_f();

	sprintf (name, "%s/%s", com_userdir, Cmd_Argv(1));

//
// open the demo file
//
	COM_DefaultExtension (name, ".qwd");

	cls.demofile = fopen (name, "wb");
	if (!cls.demofile)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	Con_Printf ("recording to %s.\n", name);
	cls.demorecording = true;

	CL_Disconnect();
	CL_SendConnectPacket ();
}


/*
====================
CL_PlayDemo_f

play [demoname]
====================
*/
void CL_PlayDemo_f (void)
{
	char	name[256];

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("play <demoname> : plays a demo\n");
		return;
	}

//
// disconnect from server
//
	CL_Disconnect ();

//
// open the demo file
//
	strcpy (name, Cmd_Argv(1));
	COM_DefaultExtension (name, ".qwd");

	Con_Printf ("Playing demo from %s.\n", name);
	COM_FOpenFile (name, &cls.demofile, false);
	if (!cls.demofile)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		cls.demonum = -1;	// stop demo loop
		return;
	}

	cls.demoplayback = true;
	cls.state = ca_demostart;
	Netchan_Setup (&cls.netchan, net_from);
	realtime = 0;
}

/*
====================
CL_FinishTimeDemo

====================
*/
void CL_FinishTimeDemo (void)
{
	int		frames;
	float	time;

	cls.timedemo = false;

// the first frame didn't count
	frames = (host_framecount - cls.td_startframe) - 1;
	time = Sys_DoubleTime() - cls.td_starttime;
	if (!time)
		time = 1;
	Con_Printf ("%i frames %5.1f seconds %5.1f fps\n", frames, time, frames/time);
}

/*
====================
CL_TimeDemo_f

timedemo [demoname]
====================
*/
void CL_TimeDemo_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("timedemo <demoname> : gets demo speeds\n");
		return;
	}

	CL_PlayDemo_f ();

//	if (cls.state != ca_active)
//		return;

// cls.td_starttime will be grabbed at the second frame of the demo, so
// all the loading time doesn't get counted

	cls.timedemo = true;
	cls.td_starttime = 0;
	cls.td_startframe = host_framecount;
	cls.td_lastframe = -1;	// get a new message this frame
}

