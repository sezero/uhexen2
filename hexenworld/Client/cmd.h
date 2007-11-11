/*
	cmd.h
	command buffer and command execution

	$Id: cmd.h,v 1.12 2007-11-11 13:17:44 sezero Exp $
*/

#ifndef __HX2_CMD_H
#define __HX2_CMD_H

//===========================================================================

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/


void Cbuf_Init (void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText (const char *text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText (const char *text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_Execute (void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

For Quake/Hexen2, commands can come from three sources, but the handler
functions may choose to dissallow the action or forward it to a remote
server if the source is not apropriate. On the other hand, QuakeWorld
and HexenWorld forward the command to the server when the handler function
is NULL.

*/

typedef void (*xcommand_t) (void);

typedef enum
{
	src_client,		// came in over a net connection as a clc_stringcmd
					// host_client will be valid during this state.
	src_command		// from the command buffer
} cmd_source_t;

extern	cmd_source_t	cmd_source;

void	Cmd_Init (void);

void	Cmd_AddCommand (const char *cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// QuakeWorld/HexenWorld allows that the function be NULL and in that
// case the command will be forwarded to the server as a clc_stringcmd
// instead of being executed locally

qboolean Cmd_Exists (const char *cmd_name);
// used by the cvar code to check for cvar / command name overlap

qboolean Cmd_CheckCommand (const char *partial);
// attempts to match a given text to known commands, cvars or aliases
// returns true if there is an exact match, false otherwise

int		Cmd_Argc (void);
const char	*Cmd_Argv (int arg);
const char	*Cmd_Args (void);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

int Cmd_CheckParm (const char *parm);
// Returns the position (1 to argc-1) in the command's argument list
// where the given parameter apears, or 0 if not present

void Cmd_TokenizeString (const char *text);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString (const char *text, cmd_source_t src);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console
// Quake/Hexen II behaves differently to command text coming from the
// command buffer, a remote client, or stdin (the src argument). For QW
// and H2W, the src argument is for compatibility only.

void Cmd_StuffCmds_f (void);
// Executes the commandline parameters with a leading "+" as script
// statements.

int ListCommands (const char *prefix, const char **buf, int pos);
int ListCvars (const char *prefix, const char **buf, int pos);
int ListAlias (const char *prefix, const char **buf, int pos);
// These three listers are either used privately in the respective
// list commands, or by keys.c in console tab-completion. They
// return the number of matches.
#define MAX_MATCHES	128
// maximum number of matches for console tab completion

#endif	/* __HX2_CMD_H */

