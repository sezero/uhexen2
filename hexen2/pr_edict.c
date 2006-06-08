/*
	sv_edict.c
	entity dictionary

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/pr_edict.c,v 1.22 2006-06-08 18:49:33 sezero Exp $
*/

#include "quakedef.h"

dprograms_t		*progs;
dfunction_t		*pr_functions;
char			*pr_strings;
ddef_t			*pr_fielddefs;
ddef_t			*pr_globaldefs;
dstatement_t	*pr_statements;
globalvars_t	*pr_global_struct;
globalvars_v111_t	*pr_global_struct_v111;
qboolean	old_progdefs;	// whether we have a Hexen2-v1.11 globals struct
float		*pr_globals;		// same as pr_global_struct
int		pr_edict_size;		// in bytes

qboolean	ignore_precache = false;

unsigned short	pr_crc;

int		type_size[8] = {
				1,
				sizeof(string_t)/4,
				1,
				3,
				1,
				1,
				sizeof(func_t)/4,
				sizeof(void *)/4
};

static ddef_t	*ED_FieldAtOfs (int ofs);
static qboolean	ED_ParseEpair (void *base, ddef_t *key, char *s);

static char field_name[256], class_name[256];
static qboolean RemoveBadReferences;

#define	MAX_FIELD_LEN	64
#define	GEFV_CACHESIZE	2

typedef struct {
	ddef_t	*pcache;
	char	field[MAX_FIELD_LEN];
} gefv_cache;

static gefv_cache	gefvCache[GEFV_CACHESIZE] =
{
		{ NULL,	"" },
		{ NULL,	"" }
};

cvar_t	max_temp_edicts = {"max_temp_edicts", "30", CVAR_ARCHIVE};

cvar_t	nomonsters = {"nomonsters", "0", CVAR_NONE};

static	cvar_t	gamecfg = {"gamecfg", "0", CVAR_NONE};
static	cvar_t	savedgamecfg = {"savedgamecfg", "0", CVAR_ARCHIVE};
static	cvar_t	saved1 = {"saved1", "0", CVAR_ARCHIVE};
static	cvar_t	saved2 = {"saved2", "0", CVAR_ARCHIVE};
static	cvar_t	saved3 = {"saved3", "0", CVAR_ARCHIVE};
static	cvar_t	saved4 = {"saved4", "0", CVAR_ARCHIVE};
static	cvar_t	scratch1 = {"scratch1", "0", CVAR_NONE};
static	cvar_t	scratch2 = {"scratch2", "0", CVAR_NONE};
static	cvar_t	scratch3 = {"scratch3", "0", CVAR_NONE};
static	cvar_t	scratch4 = {"scratch4", "0", CVAR_NONE};

//===========================================================================


/*
=================
ED_ClearEdict

Sets everything to NULL
=================
*/
void ED_ClearEdict (edict_t *e)
{
	memset (&e->v, 0, progs->entityfields * 4);
	memset (&e->baseline, 0, sizeof(e->baseline));
	e->free = false;
}

/*
=================
ED_Alloc

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *ED_Alloc (void)
{
	int			i;
	edict_t		*e;

	for ( i=svs.maxclients+1+max_temp_edicts.value ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && ( e->freetime < 2 || sv.time - e->freetime > 0.5 ) )
		{
			ED_ClearEdict (e);
			return e;
		}
	}

	if (i == MAX_EDICTS)
	{
		SV_Edicts("edicts.txt");
		Sys_Error ("ED_Alloc: no free edicts");
	}

	sv.num_edicts++;
	e = EDICT_NUM(i);
	ED_ClearEdict (e);

	return e;
}

edict_t *ED_Alloc_Temp (void)
{
	int			i, j;
	edict_t		*e,*Least;
	float		LeastTime;
	qboolean	LeastSet;

	LeastTime = -1;
	LeastSet = false;
	Least = NULL;	// shut up compiler
	for ( i=svs.maxclients+1,j=0 ; j < max_temp_edicts.value ; i++,j++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && ( e->freetime < 2 || sv.time - e->freetime > 0.5 ) )
		{
			ED_ClearEdict (e);
			e->alloctime = sv.time;

			return e;
		}
		else if (e->alloctime < LeastTime || !LeastSet)
		{
			Least = e;
			LeastTime = e->alloctime;
			LeastSet = true;
		}
	}

	ED_Free(Least);
	ED_ClearEdict (Least);
	Least->alloctime = sv.time;

	return Least;
}

/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void ED_Free (edict_t *ed)
{
	SV_UnlinkEdict (ed);		// unlink from world bsp

	ed->free = true;
	ed->v.model = 0;
	ed->v.takedamage = 0;
	ed->v.modelindex = 0;
	ed->v.colormap = 0;
	ed->v.skin = 0;
	ed->v.frame = 0;
	VectorCopy (vec3_origin, ed->v.origin);
	VectorCopy (vec3_origin, ed->v.angles);
	ed->v.nextthink = -1;
	ed->v.solid = 0;

	ed->freetime = sv.time;
	ed->alloctime = -1;
}

//===========================================================================

/*
============
ED_GlobalAtOfs
============
*/
static ddef_t *ED_GlobalAtOfs (int ofs)
{
	ddef_t		*def;
	int			i;

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (def->ofs == ofs)
			return def;
	}
	return NULL;
}

/*
============
ED_FieldAtOfs
============
*/
static ddef_t *ED_FieldAtOfs (int ofs)
{
	ddef_t		*def;
	int			i;

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (def->ofs == ofs)
			return def;
	}
	return NULL;
}

/*
============
ED_FindField
============
*/
static ddef_t *ED_FindField (char *name)
{
	ddef_t		*def;
	int			i;

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (!strcmp(pr_strings + def->s_name,name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindGlobal
============
*/
static ddef_t *ED_FindGlobal (char *name)
{
	ddef_t		*def;
	int			i;

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (!strcmp(pr_strings + def->s_name,name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindFunction
============
*/
static dfunction_t *ED_FindFunction (char *fn_name)
{
	dfunction_t		*func;
	int				i;

	for (i=0 ; i<progs->numfunctions ; i++)
	{
		func = &pr_functions[i];
		if (!strcmp(pr_strings + func->s_name, fn_name))
			return func;
	}
	return NULL;
}

dfunction_t *ED_FindFunctioni (char *fn_name)
{
	dfunction_t		*func;
	int				i;
	
	for (i=0 ; i<progs->numfunctions ; i++)
	{
		func = &pr_functions[i];
		if (!Q_strcasecmp(pr_strings + func->s_name,fn_name) )
			return func;
	}
	return NULL;
}


eval_t *GetEdictFieldValue(edict_t *ed, char *field)
{
	ddef_t			*def = NULL;
	int				i;
	static int		rep = 0;

	for (i=0 ; i<GEFV_CACHESIZE ; i++)
	{
		if (!strcmp(field, gefvCache[i].field))
		{
			def = gefvCache[i].pcache;
			goto Done;
		}
	}

	def = ED_FindField (field);

	if (strlen(field) < MAX_FIELD_LEN)
	{
		gefvCache[rep].pcache = def;
		strcpy (gefvCache[rep].field, field);
		rep ^= 1;
	}

Done:
	if (!def)
		return NULL;

	return (eval_t *)((char *)&ed->v + def->ofs*4);
}


/*
============
PR_ValueString

Returns a string describing *data in a type specific manner
=============
*/
static char *PR_ValueString (etype_t type, eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", pr_strings + val->string);
		break;
	case ev_entity:
		sprintf (line, "entity %i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)) );
		break;
	case ev_function:
		f = pr_functions + val->function;
		sprintf (line, "%s()", pr_strings + f->s_name);
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		sprintf (line, ".%s", pr_strings + def->s_name);
		break;
	case ev_void:
		sprintf (line, "void");
		break;
	case ev_float:
		sprintf (line, "%5.1f", val->_float);
		break;
	case ev_vector:
		sprintf (line, "'%5.1f %5.1f %5.1f'", val->vector[0], val->vector[1], val->vector[2]);
		break;
	case ev_pointer:
		sprintf (line, "pointer");
		break;
	default:
		sprintf (line, "bad type %i", type);
		break;
	}

	return line;
}

/*
============
PR_UglyValueString

Returns a string describing *data in a type specific manner
Easier to parse than PR_ValueString
=============
*/
static char *PR_UglyValueString (etype_t type, eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", pr_strings + val->string);
		break;
	case ev_entity:
		sprintf (line, "%i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
		break;
	case ev_function:
		f = pr_functions + val->function;
		sprintf (line, "%s", pr_strings + f->s_name);
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		sprintf (line, "%s", pr_strings + def->s_name);
		break;
	case ev_void:
		sprintf (line, "void");
		break;
	case ev_float:
		sprintf (line, "%f", val->_float);
		break;
	case ev_vector:
		sprintf (line, "%f %f %f", val->vector[0], val->vector[1], val->vector[2]);
		break;
	default:
		sprintf (line, "bad type %i", type);
		break;
	}

	return line;
}

/*
============
PR_GlobalString

Returns a string with a description and the contents of a global,
padded to 20 field width
============
*/
char *PR_GlobalString (int ofs)
{
	char	*s;
	int		i;
	ddef_t	*def;
	void	*val;
	static char	line[128];

	val = (void *)&pr_globals[ofs];
	def = ED_GlobalAtOfs(ofs);
	if (!def)
		sprintf (line,"%i(?)", ofs);
	else
	{
		s = PR_ValueString (def->type, val);
		sprintf (line,"%i(%s)%s", ofs, pr_strings + def->s_name, s);
	}

	i = strlen(line);
	for ( ; i<20 ; i++)
		strcat (line," ");
	strcat (line," ");

	return line;
}

char *PR_GlobalStringNoContents (int ofs)
{
	int		i;
	ddef_t	*def;
	static char	line[128];

	def = ED_GlobalAtOfs(ofs);
	if (!def)
		sprintf (line,"%i(?)", ofs);
	else
		sprintf (line,"%i(%s)", ofs, pr_strings + def->s_name);

	i = strlen(line);
	for ( ; i<20 ; i++)
		strcat (line," ");
	strcat (line," ");

	return line;
}


/*
=============
ED_Print

For debugging
=============
*/
void ED_Print (edict_t *ed)
{
	int		l;
	ddef_t	*d;
	int		*v;
	int		i, j;
	char	*name;
	int		type;

	if (ed->free)
	{
		Con_Printf ("FREE\n");
		return;
	}

	Con_Printf("\nEDICT %i:\n", NUM_FOR_EDICT(ed));
	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if ((name[strlen(name)-2] == '_') && 
			((name[strlen(name)-1] == 'x') || (name[strlen(name)-1] == 'y') || (name[strlen(name)-1] == 'z')))
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;

		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;

		Con_Printf ("%s",name);
		l = strlen (name);
		while (l++ < 15)
			Con_Printf (" ");

		Con_Printf ("%s\n", PR_ValueString(d->type, (eval_t *)v));
	}
}

/*
=============
ED_Write

For savegames
=============
*/
void ED_Write (FILE *f, edict_t *ed)
{
	ddef_t	*d;
	int		*v;
	int		i, j;
	char	*name;
	int		type;
	int		length;

	fprintf (f, "{\n");

	if (ed->free)
	{
		fprintf (f, "}\n");
		return;
	}

	RemoveBadReferences = true;

	if (ed->v.classname)
		strcpy(class_name,pr_strings + ed->v.classname);
	else
		class_name[0] = 0;

	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		length = strlen(name);
		if (name[length-2] == '_' && name[length-1] >= 'x' && name[length-1] <= 'z')
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;

		strcpy(field_name,name);
		fprintf (f,"\"%s\" ",name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(d->type, (eval_t *)v));
	}

	field_name[0] = 0;
	class_name[0] = 0;

	fprintf (f, "}\n");

	RemoveBadReferences = false;
}

void ED_PrintNum (int ent)
{
	ED_Print (EDICT_NUM(ent));
}

/*
=============
ED_PrintEdicts

For debugging, prints all the entities in the current server
=============
*/
void ED_PrintEdicts (void)
{
	int		i;

	Con_Printf ("%i entities\n", sv.num_edicts);
	for (i=0 ; i<sv.num_edicts ; i++)
		ED_PrintNum (i);
}

/*
=============
ED_PrintEdict_f

For debugging, prints a single edicy
=============
*/
static void ED_PrintEdict_f (void)
{
	int		i;

	i = atoi (Cmd_Argv(1));
	if (i >= sv.num_edicts)
	{
		Con_Printf("Bad edict number\n");
		return;
	}
	ED_PrintNum (i);
}

/*
=============
ED_Count

For debugging
=============
*/
static void ED_Count (void)
{
	int		i;
	edict_t	*ent;
	int		active, models, solid, step;

	active = models = solid = step = 0;
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ent = EDICT_NUM(i);
		if (ent->free)
			continue;
		active++;
		if (ent->v.solid)
			solid++;
		if (ent->v.model)
			models++;
		if (ent->v.movetype == MOVETYPE_STEP)
			step++;
	}

	Con_Printf ("num_edicts:%3i\n", sv.num_edicts);
	Con_Printf ("active    :%3i\n", active);
	Con_Printf ("view      :%3i\n", models);
	Con_Printf ("touch     :%3i\n", solid);
	Con_Printf ("step      :%3i\n", step);
}


/*
==============================================================================

ARCHIVING GLOBALS

FIXME: need to tag constants, doesn't really work
==============================================================================
*/

/*
=============
ED_WriteGlobals
=============
*/
void ED_WriteGlobals (FILE *f)
{
	ddef_t		*def;
	int			i;
	char		*name;
	int			type;

	fprintf (f,"{\n");
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type;
		if ( !(def->type & DEF_SAVEGLOBAL) )
			continue;
		type &= ~DEF_SAVEGLOBAL;

		if (type != ev_string && type != ev_float && type != ev_entity)
			continue;

		name = pr_strings + def->s_name;
		fprintf (f,"\"%s\" ", name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(type, (eval_t *)&pr_globals[def->ofs]));
	}
	fprintf (f,"}\n");
}

/*
=============
ED_ParseGlobals
=============
*/
void ED_ParseGlobals (char *data)
{
	char	keyname[64];
	ddef_t	*key;

	while (1)
	{
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		strcpy (keyname, com_token);

	// parse value
		data = COM_Parse (data);
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

		key = ED_FindGlobal (keyname);
		if (!key)
		{
			Con_Printf ("'%s' is not a global\n", keyname);
			continue;
		}

		if (!ED_ParseEpair ((void *)pr_globals, key, com_token))
			Host_Error ("ED_ParseGlobals: parse error");
	}
}

//============================================================================


/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*new, *new_p;
	int		i, l;

	l = strlen(string) + 1;
	new = Hunk_AllocName (l, "string");
	new_p = new;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}

	return new;
}


/*
=============
ED_ParseEval

Can parse either fields or globals
returns false if error
=============
*/
static qboolean	ED_ParseEpair (void *base, ddef_t *key, char *s)
{
	int		i;
	char	string[128];
	ddef_t	*def;
	char	*v, *w;
	void	*d;
	dfunction_t	*func;

	d = (void *)((int *)base + key->ofs);

	switch (key->type & ~DEF_SAVEGLOBAL)
	{
	case ev_string:
		*(string_t *)d = ED_NewString (s) - pr_strings;
		break;

	case ev_float:
		*(float *)d = atof (s);
		break;

	case ev_vector:
		strcpy (string, s);
		v = string;
		w = string;
		for (i=0 ; i<3 ; i++)
		{
			while (*v && *v != ' ')
				v++;
			*v = 0;
			((float *)d)[i] = atof (w);
			w = v = v+1;
		}
		break;

	case ev_entity:
		*(int *)d = EDICT_TO_PROG(EDICT_NUM(atoi (s)));
		break;

	case ev_field:
		def = ED_FindField (s);
		if (!def)
		{
			Con_Printf ("Can't find field %s\n", s);
			return false;
		}
		*(int *)d = G_INT(def->ofs);
		break;

	case ev_function:
		func = ED_FindFunction (s);
		if (!func)
		{
			Con_Printf ("Can't find function %s\n", s);
			return false;
		}
		*(func_t *)d = func - pr_functions;
		break;

	default:
		break;
	}
	return true;
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
Used for initial level load and for savegames.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
	ddef_t		*key;
	qboolean	anglehack;
	qboolean	init;
	char		keyname[256];
	int			n;

	init = false;

	// clear it
	if (ent != sv.edicts)	// hack
		memset (&ent->v, 0, progs->entityfields * 4);

	// go through all the dictionary pairs
	while (1)
	{
		// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		if (!strcmp(com_token, "angle"))
		{
			strcpy (com_token, "angles");
			anglehack = true;
		}
		else
			anglehack = false;

		// FIXME: change light to _light to get rid of this hack
		if (!strcmp(com_token, "light"))
			strcpy (com_token, "light_lev");	// hack for single light def

		strcpy (keyname, com_token);

		// another hack to fix heynames with trailing spaces
		n = strlen(keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

		// parse value
		data = COM_Parse (data);
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

		init = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		if (Q_strcasecmp(keyname,"MIDI") == 0)
		{
			strcpy(sv.midi_name,com_token);
			continue;
		}
		else if (Q_strcasecmp(keyname,"CD") == 0)
		{
			sv.cd_track = (byte)atol(com_token);
			continue;
		}

		key = ED_FindField (keyname);
		if (!key)
		{
			Con_Printf ("'%s' is not a field\n", keyname);
			continue;
		}

		if (anglehack)
		{
			char	temp[32];
			strcpy (temp, com_token);
			sprintf (com_token, "0 %s 0", temp);
		}

		if (!ED_ParseEpair ((void *)&ent->v, key, com_token))
			Host_Error ("ED_ParseEdict: parse error");
	}

	if (!init)
		ent->free = true;

	return data;
}


extern int entity_file_size;

/*
================
ED_LoadFromFile

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

Used for both fresh maps and savegame loads.  A fresh map would also need
to call ED_CallSpawnFunctions () to let the objects initialize themselves.
================
*/
void ED_LoadFromFile (char *data)
{
	edict_t		*ent;
	int		inhibit, skip;
	dfunction_t	*func;
	char		*orig;
	int		start_amount;

	ent = NULL;
	inhibit = 0;
	if (old_progdefs)
		pr_global_struct_v111->time = sv.time;
	else
		pr_global_struct->time = sv.time;
	orig = data;

	start_amount = current_loading_size;

	// parse ents
	while (1)
	{
		// parse the opening brace
		data = COM_Parse (data);
		if (!data)
			break;

		if (entity_file_size)
		{
			current_loading_size = start_amount + ((data-orig)*80/entity_file_size);
			D_ShowLoadingSize();
		}

		if (com_token[0] != '{')
			Sys_Error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = EDICT_NUM(0);
		else
			ent = ED_Alloc ();
		data = ED_ParseEdict (data, ent);

#if 0
		//jfm fuckup test
		//remove for final release
		if ((ent->v.spawnflags >1) && !strcmp("worldspawn",pr_strings + ent->v.classname) )
		{
			Host_Error ("invalid SpawnFlags on World!!!\n");
		}
#endif

		// remove things from different skill levels or deathmatch
		if (deathmatch.value)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH))
			{
				ED_Free (ent);
				inhibit++;
				continue;
			}
		}
		else if (coop.value)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_COOP))
			{
				ED_Free (ent);
				inhibit++;
				continue;
			}
		}
		else
		{ // Gotta be single player
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_SINGLE))
			{
				ED_Free (ent);
				inhibit++;
				continue;
			}

			skip = 0;

			switch ((int)cl_playerclass.value)
			{
			case CLASS_PALADIN:
				if ((int)ent->v.spawnflags & SPAWNFLAG_NOT_PALADIN)
				{
					skip = 1;
				}
				break;

			case CLASS_CLERIC:
				if ((int)ent->v.spawnflags & SPAWNFLAG_NOT_CLERIC)
				{
					skip = 1;
				}
				break;

			case CLASS_DEMON:
			case CLASS_NECROMANCER:
				if ((int)ent->v.spawnflags & SPAWNFLAG_NOT_NECROMANCER)
				{
					skip = 1;
				}
				break;

			case CLASS_THEIF:
				if ((int)ent->v.spawnflags & SPAWNFLAG_NOT_THEIF)
				{
					skip = 1;
				}
				break;
			}

			if (skip)
			{
				ED_Free (ent);
				inhibit++;
				continue;
			}
		}

		if ((current_skill == 0 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_EASY))
			|| (current_skill == 1 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_MEDIUM))
			|| (current_skill >= 2 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_HARD)) )
		{
			ED_Free (ent);
			inhibit++;
			continue;
		}

//
// immediately call spawn function
//
		if (!ent->v.classname)
		{
			Con_Printf ("No classname for:\n");
			ED_Print (ent);
			ED_Free (ent);
			continue;
		}

	// look for the spawn function
		func = ED_FindFunction ( pr_strings + ent->v.classname );

		if (!func)
		{
			Con_Printf ("No spawn function for:\n");
			ED_Print (ent);
			ED_Free (ent);
			continue;
		}

		if (old_progdefs)
			pr_global_struct_v111->self = EDICT_TO_PROG(ent);
		else
			pr_global_struct->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (func - pr_functions);
	}

	Con_DPrintf ("%i entities inhibited\n", inhibit);
}


/*
===============
PR_LoadProgs
===============
*/
void PR_LoadProgs (void)
{
	int			i;
	char	finalprogname[MAX_OSPATH];
#if USE_MULTIPLE_PROGS
	FILE	*FH;
#endif

	// flush the non-C variable lookup cache
	for (i=0 ; i<GEFV_CACHESIZE ; i++)
		gefvCache[i].field[0] = 0;

	CRC_Init (&pr_crc);

	strcpy(finalprogname, "progs.dat");

#if USE_MULTIPLE_PROGS
	// see the comments in progs.h about multiple progs
	COM_FOpenFile ("maplist.txt", &FH, true);
	if (FH)
	{
		char	build[2048], *test;
		char	mapname[MAX_QPATH], progname[MAX_OSPATH];
		int			j;

		fgets(build, sizeof(build), FH);
		j = atol(build);
		for (i = 0; i < j; i++)
		{
			test = fgets (build, sizeof(build), FH);
			if (test)
			{
				build[strlen(build)-2] = 0;
				test = strchr(build, ' ');
				if (test)
				{
					*test = 0;
					strcpy(mapname, build);
					strcpy(progname, test+1);
					if (Q_strcasecmp(mapname, sv.name) == 0)
					{
						strcpy(finalprogname, progname);
						break;
					}
				}
			}
		}
		fclose (FH);
	}
#endif	// end of USE_MULTIPLE_PROGS

	progs = (dprograms_t *)COM_LoadHunkFile (finalprogname);
	if (!progs)
		Sys_Error ("PR_LoadProgs: couldn't load %s",finalprogname);
	Con_DPrintf ("Programs occupy %iK.\n", com_filesize/1024);

	for (i=0 ; i<com_filesize ; i++)
		CRC_ProcessByte (&pr_crc, ((byte *)progs)[i]);

	// byte swap the header
	for (i=0 ; i<sizeof(*progs)/4 ; i++)
		((int *)progs)[i] = LittleLong ( ((int *)progs)[i] );

	if (progs->version != PROG_VERSION)
		Sys_Error ("progs.dat has wrong version number (%i should be %i)", progs->version, PROG_VERSION);
	if (progs->crc != PROGS_V111_CRC && progs->crc != PROGS_V112_CRC)
		Sys_Error ("progs.dat system vars have been modified, progdefs.h is out of date");

	pr_functions = (dfunction_t *)((byte *)progs + progs->ofs_functions);
	pr_strings = (char *)progs + progs->ofs_strings;
	pr_globaldefs = (ddef_t *)((byte *)progs + progs->ofs_globaldefs);
	pr_fielddefs = (ddef_t *)((byte *)progs + progs->ofs_fielddefs);
	pr_statements = (dstatement_t *)((byte *)progs + progs->ofs_statements);

	Con_Printf ("Loaded %s ", finalprogname);
	if (progs->crc == PROGS_V111_CRC)
	{
		Con_Printf ("(%d crc: 1.11)\n", PROGS_V111_CRC);
		old_progdefs = true;
		pr_global_struct_v111 = (globalvars_v111_t *)((byte *)progs + progs->ofs_globals);
		pr_globals = (float *)pr_global_struct_v111;
		pr_global_struct = NULL;
	}
	else
	{
		Con_Printf ("(%d crc: 1.12)\n", PROGS_V112_CRC);
		old_progdefs = false;
		pr_global_struct = (globalvars_t *)((byte *)progs + progs->ofs_globals);
		pr_globals = (float *)pr_global_struct;
		pr_global_struct_v111 = NULL;
	}

	pr_edict_size = progs->entityfields * 4 + sizeof (edict_t) - sizeof(entvars_t);

#if BYTE_ORDER == BIG_ENDIAN
	// byte swap the lumps
	for (i=0 ; i<progs->numstatements ; i++)
	{
		pr_statements[i].op = LittleShort(pr_statements[i].op);
		pr_statements[i].a = LittleShort(pr_statements[i].a);
		pr_statements[i].b = LittleShort(pr_statements[i].b);
		pr_statements[i].c = LittleShort(pr_statements[i].c);
	}

	for (i=0 ; i<progs->numfunctions; i++)
	{
		pr_functions[i].first_statement = LittleLong (pr_functions[i].first_statement);
		pr_functions[i].parm_start = LittleLong (pr_functions[i].parm_start);
		pr_functions[i].s_name = LittleLong (pr_functions[i].s_name);
		pr_functions[i].s_file = LittleLong (pr_functions[i].s_file);
		pr_functions[i].numparms = LittleLong (pr_functions[i].numparms);
		pr_functions[i].locals = LittleLong (pr_functions[i].locals);
	}

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		pr_globaldefs[i].type = LittleShort (pr_globaldefs[i].type);
		pr_globaldefs[i].ofs = LittleShort (pr_globaldefs[i].ofs);
		pr_globaldefs[i].s_name = LittleLong (pr_globaldefs[i].s_name);
	}

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		pr_fielddefs[i].type = LittleShort (pr_fielddefs[i].type);
		if (pr_fielddefs[i].type & DEF_SAVEGLOBAL)
			Sys_Error ("PR_LoadProgs: pr_fielddefs[i].type & DEF_SAVEGLOBAL");
		pr_fielddefs[i].ofs = LittleShort (pr_fielddefs[i].ofs);
		pr_fielddefs[i].s_name = LittleLong (pr_fielddefs[i].s_name);
	}

	for (i=0 ; i<progs->numglobals ; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
#endif	// BIG_ENDIAN

	// set the cl_playerclass value after pr_global_struct has been created
	if (progs->crc == PROGS_V112_CRC)
		pr_global_struct->cl_playerclass = cl_playerclass.value;
}


/*
===============
PR_Init
===============
*/
void PR_Init (void)
{
	Cmd_AddCommand ("edict", ED_PrintEdict_f);
	Cmd_AddCommand ("edicts", ED_PrintEdicts);
	Cmd_AddCommand ("edictcount", ED_Count);
	Cmd_AddCommand ("profile", PR_Profile_f);

	Cvar_RegisterVariable (&max_temp_edicts);

	Cvar_RegisterVariable (&nomonsters);
	Cvar_RegisterVariable (&gamecfg);
	Cvar_RegisterVariable (&savedgamecfg);
	Cvar_RegisterVariable (&scratch1);
	Cvar_RegisterVariable (&scratch2);
	Cvar_RegisterVariable (&scratch3);
	Cvar_RegisterVariable (&scratch4);
	Cvar_RegisterVariable (&saved1);
	Cvar_RegisterVariable (&saved2);
	Cvar_RegisterVariable (&saved3);
	Cvar_RegisterVariable (&saved4);
}


edict_t *EDICT_NUM(int n)
{
	if (n < 0 || n >= sv.max_edicts)
		Sys_Error ("EDICT_NUM: bad number %i", n);
	return (edict_t *)((byte *)sv.edicts+ (n)*pr_edict_size);
}

int NUM_FOR_EDICT(edict_t *e)
{
	int		b;

	b = (byte *)e - (byte *)sv.edicts;
	b = b / pr_edict_size;

	if (b < 0 || b >= sv.num_edicts)
	{
		if (!RemoveBadReferences)
			Con_DPrintf ("NUM_FOR_EDICT: bad pointer, Class: %s Field: %s, Index %d, Total %d",class_name,field_name,b,sv.num_edicts);
		return(0);
	}
	if (e->free && RemoveBadReferences)
	{
//		Con_Printf ("NUM_FOR_EDICT: freed edict, Class: %s Field: %s, Index %d, Total %d",class_name,field_name,b,sv.num_edicts);
		return(0);
	}
	return b;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.21  2006/04/24 08:38:55  sezero
 * char seems to be of different signedness on MorphOS, so changed the type of
 * NewLineChar to signed char. Thanks go to Michal Wozniak for catching this.
 *
 * Revision 1.20  2006/04/05 06:10:44  sezero
 * added support for both hexen2-v1.11 and h2mp-v1.12 progs into a single hexen2
 * binary. this essentially completes the h2/h2mp binary merge started with the
 * previous patch. many conditionals had to be added especially on the server side,but couldn't notice any serious performance loss on a PIII-733 computer. Supportfor multiple progs.dat is now advised to be left enabled in order to support
 * mods which uses that feature.
 *
 * Revision 1.19  2006/04/05 06:09:23  sezero
 * killed (almost) all H2MP ifdefs: this is the first step in making a single
 * binary which handles both h2 and h2mp properly. the only H2MP ifdefs left
 * are actually the ones for determining the icon and window manager text, so
 * nothing serious. the binary normally will only run the original h2 game.
 * if given a -portals or -missionpack or -h2mp argument, it will look for the
 * mission pack and run it (this is the same logic that quake used.) The only
 * serious side effect is that h2 and h2mp progs being different: This will be
 * solved by the next patch by adding support for the two progs versions into
 * a single binary.
 *
 * Revision 1.18  2006/03/24 15:05:39  sezero
 * killed the archive, server and info members of the cvar structure.
 * the new flags member is now employed for all those purposes. also
 * made all non-globally used cvars static.
 *
 * Revision 1.17  2006/03/17 14:12:48  sezero
 * put back mission-pack only objectives stuff back into pure h2 builds.
 * it was a total screw-up...
 *
 * Revision 1.16  2006/03/13 22:28:51  sezero
 * removed the expansion pack only feature of objective strings from
 * hexen2-only builds (many new ifdef H2MP stuff). removed the expansion
 * pack only intermission picture and string searches from hexen2-only
 * builds.
 *
 * Revision 1.15  2006/02/20 23:34:39  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. (part 16: pr_edict.c and progs.h)
 *
 * Revision 1.14  2005/07/09 09:07:58  sezero
 * changed all Hunk_Alloc() usage to Hunk_AllocName() for easier memory usage tracing.
 *
 * Revision 1.13  2005/06/12 07:28:51  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.12  2005/05/19 16:41:50  sezero
 * removed all unused (never used) non-RJNET and non-QUAKE2RJ code
 *
 * Revision 1.11  2005/05/17 22:56:19  sezero
 * cleanup the "stricmp, strcmpi, strnicmp, Q_strcasecmp, Q_strncasecmp" mess:
 * Q_strXcasecmp will now be used throughout the code which are implementation
 * dependant defines for __GNUC__ (strXcasecmp) and _WIN32 (strXicmp)
 *
 * Revision 1.10  2005/04/30 08:47:07  sezero
 * silenced warnings about name decleration being shadowed
 *
 * Revision 1.9  2005/04/30 08:45:17  sezero
 * silenced warnings about Lenght decleration being shadowed
 *
 * Revision 1.8  2005/04/05 19:28:40  sezero
 * clean-ups in endianness which now is decided at compile time
 *
 * Revision 1.7  2004/12/23 21:12:14  sezero
 * fix a comment and give more detail
 *
 * Revision 1.6  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.5  2004/12/18 14:08:07  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.4  2004/12/12 14:25:18  sezero
 * add and fix some comments
 *
 * Revision 1.3  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.2  2004/11/28 00:30:35  sezero
 * allow original hexen2 to load the progs2.dat, fixes the no-boss bug of AoT
 *
 * Revision 1.1.1.1  2004/11/28 00:06:00  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 11    3/27/98 2:12p Jmonroe
 * 
 * 10    3/16/98 11:46p Jmonroe
 * 
 * 9     3/02/98 3:43p Jmonroe
 * 
 * 8     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 7     2/24/98 12:11p Jmonroe
 * put in test for invalid spawnflags on world
 * 
 * 6     2/07/98 6:53p Jweier
 * 
 * 5     1/22/98 3:10p Jmonroe
 * minor optimize
 * 
 * 4     1/21/98 10:29a Plipo
 * 
 * 31    10/29/97 5:39p Jheitzman
 * 
 * 30    10/28/97 2:58p Jheitzman
 * 
 * 27    8/27/97 12:10p Rjohnson
 * Support for multiple progs.dat
 * 
 * 26    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 25    8/24/97 11:07a Rjohnson
 * Changed a message to be developer only
 * 
 * 24    8/19/97 5:23p Rjohnson
 * Fix
 * 
 * 23    8/19/97 3:44p Rjohnson
 * Changed error message to warning
 * 
 * 22    8/19/97 3:42p Rjohnson
 * Changed num_for_edict message to warning
 * 
 * 21    8/18/97 12:03a Rjohnson
 * Added loading progress
 * 
 * 20    8/13/97 5:53p Rjohnson
 * Fix for player class spawning
 * 
 * 19    8/08/97 11:27a Rjohnson
 * Made it more safe
 * 
 * 18    8/07/97 3:38p Rjohnson
 * More info for the num_for_edict error message
 * 
 * 17    7/21/97 11:48a Rjohnson
 * Fixed for particleexplosion (network friendly) and spawn_temp
 * 
 * 16    7/16/97 3:13p Rjohnson
 * Fix for saving fields that use a _
 * 
 * 15    7/03/97 4:18p Rlove
 * 
 * 14    7/03/97 12:22p Rjohnson
 * Fix for string reading
 * 
 * 13    6/27/97 11:34a Rjohnson
 * Added a create function
 * 
 * 12    6/25/97 12:54p Rjohnson
 * Changed how the file was layed out
 * 
 * 11    6/25/97 12:49p Rjohnson
 * Added a global text file 
 * 
 * 10    6/23/97 4:14p Rjohnson
 * Created temp edicts (gibs)
 * 
 * 9     6/06/97 11:10a Rjohnson
 * Added a command to print out the edicts in memory
 * 
 * 8     4/20/97 5:05p Rjohnson
 * Networking Update
 * 
 * 7     3/26/97 12:56p Bgokey
 * 
 * 6     3/03/97 5:00p Rjohnson
 * Added spawn flags and code to prevent items flagged from being spawned
 * 
 * 5     3/03/97 4:03p Rjohnson
 * Added cd specifications to the world-spawn entity
 */
