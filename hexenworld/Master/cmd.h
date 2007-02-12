
// cmd.h -- Command buffer and command execution

#ifndef __HX2_CMD_H
#define __HX2_CMD_H

//===========================================================================


typedef void (*xcommand_t) (void);

int Cmd_Argc (void);
char *Cmd_Argv (int arg);
void Cmd_TokenizeString (char *text);
void Cmd_AddCommand (char *cmd_name, xcommand_t function);
void Cmd_Init();
void Cmd_ExecuteString (char *text);
void Cbuf_Init (void);
void Cbuf_AddText (const char *text);
void Cbuf_InsertText (const char *text);
void Cbuf_Execute (void);
qboolean Cmd_Exists (const char *cmd_name);

#endif	/* __HX2_CMD_H */

