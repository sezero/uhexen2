/*
	cvar.h
	dynamic variable tracking

	$Id: cvar.h,v 1.15 2007-06-01 20:01:49 sezero Exp $
*/

#ifndef __CVAR_H__
#define __CVAR_H__

/*

cvar_t variables are used to hold scalar or string variables that can
be changed or displayed at the console or prog code as well as accessed
directly in C code.

it is sufficient to initialize a cvar_t with just the first two fields,
or you can add a ,true flag for variables that you want saved to the
configuration file when the game is quit:

cvar_t	r_draworder = {"r_draworder","1"};
cvar_t	scr_screensize = {"screensize","1",true};

Cvars must be registered before use, or they will have a 0 value instead
of the float interpretation of the string.
Generally, all cvar_t declarations should be registered in the apropriate
init function before any console commands are executed:

Cvar_RegisterVariable (&host_framerate);


C code usually just references a cvar in place:
if ( r_draworder.value )

It could optionally ask for the value to be looked up for a string name:
if (Cvar_VariableValue ("r_draworder"))

Interpreted prog code can access cvars with the cvar(name) or
cvar_set (name, value) internal functions:
teamplay = cvar("teamplay");
cvar_set ("registered", "1");

The user can access cvars from the console in two ways:
r_draworder		prints the current value
r_draworder 0		sets the current value to 0

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

*/

#define	CVAR_NONE		0
#define	CVAR_ARCHIVE		1	// if set, causes it to be saved to config
#define	CVAR_NOTIFY		2	// changes will be broadcasted to all players (H2)
#define	CVAR_SERVERINFO		4	// added to serverinfo will be sent to clients (H2/net_dgrm.c and H2W)
#define	CVAR_USERINFO		8	// added to userinfo, will be sent to server (H2W)
#define	CVAR_ROM		64
#define	CVAR_LOCKED		256	// locked temporarily
#define	CVAR_REGISTERED		1024	// the var is added to the list of variables


typedef struct cvar_s
{
	char	*name;
	char	*string;
	unsigned	flags;
	float		value;
	int		integer;
	struct cvar_s	*next;
} cvar_t;

void	Cvar_RegisterVariable (cvar_t *variable);
// registers a cvar that already has the name, string, and optionally
// the archive elements set.

void	Cvar_Set (const char *var_name, const char *value);
// equivelant to "<name> <variable>" typed at the console

void	Cvar_SetValue (const char *var_name, const float value);
// expands value to a string and calls Cvar_Set

float	Cvar_VariableValue (const char *var_name);
// returns 0 if not defined or non numeric

char	*Cvar_VariableString (const char *var_name);
// returns an empty string if not defined

qboolean Cvar_Command (void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void	Cvar_WriteVariables (FILE *f);
// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.

cvar_t	*Cvar_FindVar (const char *var_name);
cvar_t	*Cvar_FindVarAfter (const char *prev_name, unsigned int with_flags);

void	Cvar_LockVar (const char *var_name);
void	Cvar_UnlockVar (const char *var_name);
void	Cvar_UnlockAll (void);

#endif	/* __CVAR_H__ */

