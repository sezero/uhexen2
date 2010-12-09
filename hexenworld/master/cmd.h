/*
	cmd.h
	command buffer and command execution

	$Id: cmd.h,v 1.4 2007-11-11 13:18:22 sezero Exp $
*/

#ifndef __HX2_CMD_H
#define __HX2_CMD_H

//===========================================================================


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

typedef void (*xcommand_t) (void);

void	Cmd_Init (void);

void	Cmd_AddCommand (const char *cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory

int		Cmd_Argc (void);
const char	*Cmd_Argv (int arg);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

void Cmd_TokenizeString (const char *text);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString (const char *text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

#endif	/* __HX2_CMD_H */

