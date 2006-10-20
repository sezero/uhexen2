/*
	cvar.c
	dynamic variable tracking

*/

#include "quakedef.h"

cvar_t	*cvar_vars;
static	char	*cvar_null_string = "";

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (char *var_name)
{
	cvar_t	*var;

	for (var = cvar_vars ; var ; var = var->next)
		if (!strcmp (var_name, var->name))
			return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float	Cvar_VariableValue (char *var_name)
{
	cvar_t	*var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atof (var->string);
}


/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (char *var_name)
{
	cvar_t *var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}


/*
============
Cvar_Set
============
*/
void Cvar_Set (char *var_name, char *value)
{
	cvar_t	*var;
	qboolean changed;

	var = Cvar_FindVar (var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	if (var->flags & CVAR_ROM)
		return;	// cvar is marked read-only

	changed = strcmp(var->string, value);

	Z_Free (var->string);	// free the old value string

	var->string = Z_Malloc (strlen(value)+1);
	strcpy (var->string, value);
	var->value = atof (var->string);
	if ((var->flags & CVAR_NOTIFY) && changed)
	{
		if (sv.active)
			SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
	}

	// Don't allow deathmatch and coop at the same time
	if (!strcmp(var->name, deathmatch.name) && var->value != 0)
		Cvar_Set("coop", "0");
	if (!strcmp(var->name, coop.name) && var->value != 0)
		Cvar_Set("deathmatch", "0");
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (char *var_name, float value)
{
	char	val[32];
	int	i;

	if (value == (int)value)
	{
		snprintf (val, 32, "%i", (int)value);
	}
	else
	{
		snprintf (val, 32, "%1f", value);	// no leading spaces
		for (i = strlen(val)-1; i > 0 && val[i] == '0' && val[i-1] != '.'; i--)	// no trailing zeroes
		{
			val[i] = 0;
		}
	}

	Cvar_Set (var_name, val);
}


/*
============
Cvar_RegisterVariable

Adds a freestanding variable to the variable list.
============
*/
void Cvar_RegisterVariable (cvar_t *variable)
{
	char	value[512];
	qboolean	set_rom = false;

// first check to see if it has already been defined
	if (Cvar_FindVar (variable->name))
	{
		Con_Printf ("Can't register variable %s, already defined\n", variable->name);
		return;
	}

// check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", variable->name);
		return;
	}

// link the variable in
	variable->next = cvar_vars;
	cvar_vars = variable;

// copy the value off, because future sets will Z_Free it
	strncpy (value, variable->string, 511);
	value[511] = '\0';
	variable->string = Z_Malloc (1);

// set it through the function to be consistant
	set_rom = (variable->flags & CVAR_ROM);
	variable->flags &= ~CVAR_ROM;
	Cvar_Set (variable->name, value);
	if (set_rom)
		variable->flags |= CVAR_ROM;
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean	Cvar_Command (void)
{
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (Cmd_Argv(0));
	if (!v)
		return false;

// perform a variable print or set
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set (v->name, Cmd_Argv(1));
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;

	for (var = cvar_vars ; var ; var = var->next)
		if (var->flags & CVAR_ARCHIVE)
			fprintf (f, "%s \"%s\"\n", var->name, var->string);
}

