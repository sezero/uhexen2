/*
	cl_cmd.c
	client command forwarding to server

	$Header: /cvsroot/uhexen2/engine/hexen2/cl_cmd.c,v 1.5 2007-02-25 19:01:03 sezero Exp $
*/

#include "quakedef.h"

/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer (void)
{
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	SZ_Print (&cls.message, Cmd_Argv(0));

	if (Cmd_Argc() > 1)
	{
		SZ_Print (&cls.message, " ");
		SZ_Print (&cls.message, Cmd_Args());
	}
}

// This is the command variant of the above. The only difference
// is that it doesn't forward the first argument, which is "cmd"
void Cmd_ForwardToServer_f (void)
{
	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (cls.demoplayback)
		return;		// not really connected

	if (Cmd_Argc() > 1)
	{
		MSG_WriteByte (&cls.message, clc_stringcmd);
		SZ_Print (&cls.message, Cmd_Args());
	}
}

void CL_Cmd_Init (void)
{
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer_f);
}

