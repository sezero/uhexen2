/*
 * sv_edict.c -- entity dictionary
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

#include "quakedef.h"

dprograms_t		*progs;
dfunction_t		*pr_functions;

static	char		pr_null_string[] = "";
static	char		*pr_strings;
static	int		pr_stringssize;
static	const char	**pr_knownstrings;
static	int		pr_maxknownstrings;
static	int		pr_numknownstrings;
static	ddef_t		*pr_fielddefs;
static	ddef_t		*pr_globaldefs;

dstatement_t	*pr_statements;
float		*pr_globals;
sv_globals_t	sv_globals;
int		pr_edict_size;		/* in bytes */

qboolean	is_progs_v6;

qboolean	ignore_precache = false;

unsigned short	pr_crc;

static int	type_size[8] = {
	1,	/* ev_void */
	1,	/* ev_string */
	1,	/* ev_float */
	3,	/* ev_vector */
	1,	/* ev_entity */
	1,	/* ev_field */
	1,	/* ev_function */
	1	/* ev_pointer */
};

typedef struct sv_def_s
{
	etype_t		type;
	int		offset;
	void		*field;
} sv_def_t;

#define OFS_V103(m)	(int)offsetof(globalvars_v103_t,m)/4
#define OFS_V111(m)	(int)offsetof(globalvars_v111_t,m)/4
#define OFS_V112(m)	(int)offsetof(globalvars_v112_t,m)/4

COMPILE_TIME_ASSERT(v103_gofs, offsetof(globalvars_v103_t,ClassChangeWeapon) == 412);
static sv_def_t globals_v103[] = {
	{ev_entity,	OFS_V103(self),			&sv_globals.self},
	{ev_entity,	OFS_V103(other),		&sv_globals.other},
	{ev_entity,	OFS_V103(world),		&sv_globals.world},
	{ev_float,	OFS_V103(time),			&sv_globals.time},
	{ev_float,	OFS_V103(frametime),		&sv_globals.frametime},
	{ev_float,	OFS_V103(force_retouch),	&sv_globals.force_retouch},
	{ev_string,	OFS_V103(mapname),		&sv_globals.mapname},
	{ev_string,	OFS_V103(startspot),		&sv_globals.startspot},
	{ev_float,	OFS_V103(deathmatch),		&sv_globals.deathmatch},
	{ev_float,	OFS_V103(coop),			&sv_globals.coop},
	{ev_float,	OFS_V103(teamplay),		&sv_globals.teamplay},
	{ev_float,	OFS_V103(serverflags),		&sv_globals.serverflags},
	{ev_float,	OFS_V103(total_secrets),	&sv_globals.total_secrets},
	{ev_float,	OFS_V103(total_monsters),	&sv_globals.total_monsters},
	{ev_float,	OFS_V103(found_secrets),	&sv_globals.found_secrets},
	{ev_float,	OFS_V103(killed_monsters),	&sv_globals.killed_monsters},
	{ev_float,	OFS_V103(chunk_cnt),		&sv_globals.chunk_cnt},
	{ev_float,	OFS_V103(done_precache),	&sv_globals.done_precache},
	{ev_float,	OFS_V103(parm1),		&sv_globals.parm},
	{ev_vector,	OFS_V103(v_forward),		&sv_globals.v_forward},
	{ev_vector,	OFS_V103(v_up),			&sv_globals.v_up},
	{ev_vector,	OFS_V103(v_right),		&sv_globals.v_right},
	{ev_float,	OFS_V103(trace_allsolid),	&sv_globals.trace_allsolid},
	{ev_float,	OFS_V103(trace_startsolid),	&sv_globals.trace_startsolid},
	{ev_float,	OFS_V103(trace_fraction),	&sv_globals.trace_fraction},
	{ev_vector,	OFS_V103(trace_endpos),		&sv_globals.trace_endpos},
	{ev_vector,	OFS_V103(trace_plane_normal),	&sv_globals.trace_plane_normal},
	{ev_float,	OFS_V103(trace_plane_dist),	&sv_globals.trace_plane_dist},
	{ev_entity,	OFS_V103(trace_ent),		&sv_globals.trace_ent},
	{ev_float,	OFS_V103(trace_inopen),		&sv_globals.trace_inopen},
	{ev_float,	OFS_V103(trace_inwater),	&sv_globals.trace_inwater},
	{ev_entity,	OFS_V103(msg_entity),		&sv_globals.msg_entity},
	{ev_float,	OFS_V103(cycle_wrapped),	&sv_globals.cycle_wrapped},
	{ev_float,	OFS_V103(crouch_cnt),		&sv_globals.crouch_cnt},
	{ev_float,	OFS_V103(modelindex_assassin),	&sv_globals.modelindex_assassin},
	{ev_float,	OFS_V103(modelindex_crusader),	&sv_globals.modelindex_crusader},
	{ev_float,	OFS_V103(modelindex_paladin),	&sv_globals.modelindex_paladin},
	{ev_float,	OFS_V103(modelindex_necromancer),&sv_globals.modelindex_necromancer},
	{ev_float,	OFS_V103(modelindex_sheep),	&sv_globals.modelindex_sheep},
	{ev_float,	OFS_V103(num_players),		&sv_globals.num_players},
	{ev_float,	OFS_V103(exp_mult),		&sv_globals.exp_mult},

	{ev_function,	OFS_V103(main),			&sv_globals.main},
	{ev_function,	OFS_V103(StartFrame),		&sv_globals.StartFrame},
	{ev_function,	OFS_V103(PlayerPreThink),	&sv_globals.PlayerPreThink},
	{ev_function,	OFS_V103(PlayerPostThink),	&sv_globals.PlayerPostThink},
	{ev_function,	OFS_V103(ClientKill),		&sv_globals.ClientKill},
	{ev_function,	OFS_V103(ClientConnect),	&sv_globals.ClientConnect},
	{ev_function,	OFS_V103(PutClientInServer),	&sv_globals.PutClientInServer},
	{ev_function,	OFS_V103(ClientReEnter),	&sv_globals.ClientReEnter},
	{ev_function,	OFS_V103(ClientDisconnect),	&sv_globals.ClientDisconnect},
	{ev_function,	OFS_V103(ClassChangeWeapon),	&sv_globals.ClassChangeWeapon},
	{ev_void,	0,				NULL }
};

COMPILE_TIME_ASSERT(v111_gofs, offsetof(globalvars_v111_t,ClassChangeWeapon) == 416);
static sv_def_t globals_v111[] = {
	{ev_entity,	OFS_V111(self),			&sv_globals.self},
	{ev_entity,	OFS_V111(other),		&sv_globals.other},
	{ev_entity,	OFS_V111(world),		&sv_globals.world},
	{ev_float,	OFS_V111(time),			&sv_globals.time},
	{ev_float,	OFS_V111(frametime),		&sv_globals.frametime},
	{ev_float,	OFS_V111(force_retouch),	&sv_globals.force_retouch},
	{ev_string,	OFS_V111(mapname),		&sv_globals.mapname},
	{ev_string,	OFS_V111(startspot),		&sv_globals.startspot},
	{ev_float,	OFS_V111(deathmatch),		&sv_globals.deathmatch},
	{ev_float,	OFS_V111(randomclass),		&sv_globals.randomclass},
	{ev_float,	OFS_V111(coop),			&sv_globals.coop},
	{ev_float,	OFS_V111(teamplay),		&sv_globals.teamplay},
	{ev_float,	OFS_V111(serverflags),		&sv_globals.serverflags},
	{ev_float,	OFS_V111(total_secrets),	&sv_globals.total_secrets},
	{ev_float,	OFS_V111(total_monsters),	&sv_globals.total_monsters},
	{ev_float,	OFS_V111(found_secrets),	&sv_globals.found_secrets},
	{ev_float,	OFS_V111(killed_monsters),	&sv_globals.killed_monsters},
	{ev_float,	OFS_V111(chunk_cnt),		&sv_globals.chunk_cnt},
	{ev_float,	OFS_V111(done_precache),	&sv_globals.done_precache},
	{ev_float,	OFS_V111(parm1),		&sv_globals.parm},
	{ev_vector,	OFS_V111(v_forward),		&sv_globals.v_forward},
	{ev_vector,	OFS_V111(v_up),			&sv_globals.v_up},
	{ev_vector,	OFS_V111(v_right),		&sv_globals.v_right},
	{ev_float,	OFS_V111(trace_allsolid),	&sv_globals.trace_allsolid},
	{ev_float,	OFS_V111(trace_startsolid),	&sv_globals.trace_startsolid},
	{ev_float,	OFS_V111(trace_fraction),	&sv_globals.trace_fraction},
	{ev_vector,	OFS_V111(trace_endpos),		&sv_globals.trace_endpos},
	{ev_vector,	OFS_V111(trace_plane_normal),	&sv_globals.trace_plane_normal},
	{ev_float,	OFS_V111(trace_plane_dist),	&sv_globals.trace_plane_dist},
	{ev_entity,	OFS_V111(trace_ent),		&sv_globals.trace_ent},
	{ev_float,	OFS_V111(trace_inopen),		&sv_globals.trace_inopen},
	{ev_float,	OFS_V111(trace_inwater),	&sv_globals.trace_inwater},
	{ev_entity,	OFS_V111(msg_entity),		&sv_globals.msg_entity},
	{ev_float,	OFS_V111(cycle_wrapped),	&sv_globals.cycle_wrapped},
	{ev_float,	OFS_V111(crouch_cnt),		&sv_globals.crouch_cnt},
	{ev_float,	OFS_V111(modelindex_assassin),	&sv_globals.modelindex_assassin},
	{ev_float,	OFS_V111(modelindex_crusader),	&sv_globals.modelindex_crusader},
	{ev_float,	OFS_V111(modelindex_paladin),	&sv_globals.modelindex_paladin},
	{ev_float,	OFS_V111(modelindex_necromancer),&sv_globals.modelindex_necromancer},
	{ev_float,	OFS_V111(modelindex_sheep),	&sv_globals.modelindex_sheep},
	{ev_float,	OFS_V111(num_players),		&sv_globals.num_players},
	{ev_float,	OFS_V111(exp_mult),		&sv_globals.exp_mult},

	{ev_function,	OFS_V111(main),			&sv_globals.main},
	{ev_function,	OFS_V111(StartFrame),		&sv_globals.StartFrame},
	{ev_function,	OFS_V111(PlayerPreThink),	&sv_globals.PlayerPreThink},
	{ev_function,	OFS_V111(PlayerPostThink),	&sv_globals.PlayerPostThink},
	{ev_function,	OFS_V111(ClientKill),		&sv_globals.ClientKill},
	{ev_function,	OFS_V111(ClientConnect),	&sv_globals.ClientConnect},
	{ev_function,	OFS_V111(PutClientInServer),	&sv_globals.PutClientInServer},
	{ev_function,	OFS_V111(ClientReEnter),	&sv_globals.ClientReEnter},
	{ev_function,	OFS_V111(ClientDisconnect),	&sv_globals.ClientDisconnect},
	{ev_function,	OFS_V111(ClassChangeWeapon),	&sv_globals.ClassChangeWeapon},
	{ev_void,	0,				NULL }
};

COMPILE_TIME_ASSERT(v112_gofs, offsetof(globalvars_v112_t,ClassChangeWeapon) == 404);
static sv_def_t globals_v112[] = {
	{ev_entity,	OFS_V112(self),			&sv_globals.self},
	{ev_entity,	OFS_V112(other),		&sv_globals.other},
	{ev_entity,	OFS_V112(world),		&sv_globals.world},
	{ev_float,	OFS_V112(time),			&sv_globals.time},
	{ev_float,	OFS_V112(frametime),		&sv_globals.frametime},
	{ev_float,	OFS_V112(force_retouch),	&sv_globals.force_retouch},
	{ev_string,	OFS_V112(mapname),		&sv_globals.mapname},
	{ev_string,	OFS_V112(startspot),		&sv_globals.startspot},
	{ev_float,	OFS_V112(deathmatch),		&sv_globals.deathmatch},
	{ev_float,	OFS_V112(randomclass),		&sv_globals.randomclass},
	{ev_float,	OFS_V112(coop),			&sv_globals.coop},
	{ev_float,	OFS_V112(teamplay),		&sv_globals.teamplay},
	{ev_float,	OFS_V112(cl_playerclass),	&sv_globals.cl_playerclass},
	{ev_float,	OFS_V112(serverflags),		&sv_globals.serverflags},
	{ev_float,	OFS_V112(total_secrets),	&sv_globals.total_secrets},
	{ev_float,	OFS_V112(total_monsters),	&sv_globals.total_monsters},
	{ev_float,	OFS_V112(found_secrets),	&sv_globals.found_secrets},
	{ev_float,	OFS_V112(killed_monsters),	&sv_globals.killed_monsters},
	{ev_float,	OFS_V112(chunk_cnt),		&sv_globals.chunk_cnt},
	{ev_float,	OFS_V112(done_precache),	&sv_globals.done_precache},
	{ev_float,	OFS_V112(parm1),		&sv_globals.parm},
	{ev_vector,	OFS_V112(v_forward),		&sv_globals.v_forward},
	{ev_vector,	OFS_V112(v_up),			&sv_globals.v_up},
	{ev_vector,	OFS_V112(v_right),		&sv_globals.v_right},
	{ev_float,	OFS_V112(trace_allsolid),	&sv_globals.trace_allsolid},
	{ev_float,	OFS_V112(trace_startsolid),	&sv_globals.trace_startsolid},
	{ev_float,	OFS_V112(trace_fraction),	&sv_globals.trace_fraction},
	{ev_vector,	OFS_V112(trace_endpos),		&sv_globals.trace_endpos},
	{ev_vector,	OFS_V112(trace_plane_normal),	&sv_globals.trace_plane_normal},
	{ev_float,	OFS_V112(trace_plane_dist),	&sv_globals.trace_plane_dist},
	{ev_entity,	OFS_V112(trace_ent),		&sv_globals.trace_ent},
	{ev_float,	OFS_V112(trace_inopen),		&sv_globals.trace_inopen},
	{ev_float,	OFS_V112(trace_inwater),	&sv_globals.trace_inwater},
	{ev_entity,	OFS_V112(msg_entity),		&sv_globals.msg_entity},
	{ev_float,	OFS_V112(cycle_wrapped),	&sv_globals.cycle_wrapped},
	{ev_float,	OFS_V112(crouch_cnt),		&sv_globals.crouch_cnt},
	{ev_float,	OFS_V112(modelindex_sheep),	&sv_globals.modelindex_sheep},
	{ev_float,	OFS_V112(num_players),		&sv_globals.num_players},
	{ev_float,	OFS_V112(exp_mult),		&sv_globals.exp_mult},

	{ev_function,	OFS_V112(main),			&sv_globals.main},
	{ev_function,	OFS_V112(StartFrame),		&sv_globals.StartFrame},
	{ev_function,	OFS_V112(PlayerPreThink),	&sv_globals.PlayerPreThink},
	{ev_function,	OFS_V112(PlayerPostThink),	&sv_globals.PlayerPostThink},
	{ev_function,	OFS_V112(ClientKill),		&sv_globals.ClientKill},
	{ev_function,	OFS_V112(ClientConnect),	&sv_globals.ClientConnect},
	{ev_function,	OFS_V112(PutClientInServer),	&sv_globals.PutClientInServer},
	{ev_function,	OFS_V112(ClientReEnter),	&sv_globals.ClientReEnter},
	{ev_function,	OFS_V112(ClientDisconnect),	&sv_globals.ClientDisconnect},
	{ev_function,	OFS_V112(ClassChangeWeapon),	&sv_globals.ClassChangeWeapon},
	{ev_void,	0,				NULL }
};

static ddef_t	*ED_FieldAtOfs (int ofs);
static qboolean	ED_ParseEpair (void *base, ddef_t *key, const char *s);

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

// these actually are not used in hexen2, but mods may use them.
cvar_t	nomonsters = {"nomonsters", "0", CVAR_NONE};
cvar_t	gamecfg = {"gamecfg", "0", CVAR_NONE};
cvar_t	savedgamecfg = {"savedgamecfg", "0", CVAR_ARCHIVE};
cvar_t	saved1 = {"saved1", "0", CVAR_ARCHIVE};
cvar_t	saved2 = {"saved2", "0", CVAR_ARCHIVE};
cvar_t	saved3 = {"saved3", "0", CVAR_ARCHIVE};
cvar_t	saved4 = {"saved4", "0", CVAR_ARCHIVE};
cvar_t	scratch1 = {"scratch1", "0", CVAR_NONE};
cvar_t	scratch2 = {"scratch2", "0", CVAR_NONE};
cvar_t	scratch3 = {"scratch3", "0", CVAR_NONE};
cvar_t	scratch4 = {"scratch4", "0", CVAR_NONE};

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

	for (i = svs.maxclients + 1 + max_temp_edicts.integer; i < sv.num_edicts; i++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && (e->freetime < 2 || sv.time - e->freetime > 0.5))
		{
			ED_ClearEdict (e);
			return e;
		}
	}

	if (i == MAX_EDICTS)
	{
		SV_Edicts("edicts.txt");
		Host_Error ("%s: no free edicts", __thisfunc__);
	}

	sv.num_edicts++;
	e = EDICT_NUM(i);
	ED_ClearEdict (e);

	return e;
}

edict_t *ED_Alloc_Temp (void)
{
	int			i, j;
	edict_t		*e, *Least;

	Least = NULL;
	for (i = svs.maxclients + 1, j = 0; j < max_temp_edicts.integer; i++, j++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && (e->freetime < 2 || sv.time - e->freetime > 0.5))
		{
			ED_ClearEdict (e);
			e->alloctime = sv.time;

			return e;
		}
		if (Least == NULL || e->alloctime < Least->alloctime)
		{
			Least = e;
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
	VectorClear (ed->v.origin);
	VectorClear (ed->v.angles);
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

	for (i = 0; i < progs->numglobaldefs; i++)
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

	for (i = 0; i < progs->numfielddefs; i++)
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
static ddef_t *ED_FindField (const char *name)
{
	ddef_t		*def;
	int			i;

	for (i = 0; i < progs->numfielddefs; i++)
	{
		def = &pr_fielddefs[i];
		if ( !strcmp(PR_GetString(def->s_name), name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindGlobal
============
*/
static ddef_t *ED_FindGlobal (const char *name)
{
	ddef_t		*def;
	int			i;

	for (i = 0; i < progs->numglobaldefs; i++)
	{
		def = &pr_globaldefs[i];
		if ( !strcmp(PR_GetString(def->s_name), name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindFunction
============
*/
static dfunction_t *ED_FindFunction (const char *fn_name)
{
	dfunction_t		*func;
	int				i;

	for (i = 0; i < progs->numfunctions; i++)
	{
		func = &pr_functions[i];
		if ( !strcmp(PR_GetString(func->s_name), fn_name) )
			return func;
	}
	return NULL;
}

dfunction_t *ED_FindFunctioni (const char *fn_name)
{
	dfunction_t		*func;
	int				i;
	
	for (i = 0; i < progs->numfunctions; i++)
	{
		func = &pr_functions[i];
		if ( !q_strcasecmp(PR_GetString(func->s_name), fn_name) )
			return func;
	}
	return NULL;
}


eval_t *GetEdictFieldValue(edict_t *ed, const char *field)
{
	ddef_t			*def = NULL;
	int				i;
	static int		rep = 0;

	for (i = 0; i < GEFV_CACHESIZE; i++)
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
(etype_t type, eval_t *val)

Returns a string describing *data in a type specific manner
=============
*/
static const char *PR_ValueString (int type, eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", PR_GetString(val->string));
		break;
	case ev_entity:
		sprintf (line, "entity %i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)) );
		break;
	case ev_function:
		f = pr_functions + val->function;
		sprintf (line, "%s()", PR_GetString(f->s_name));
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		sprintf (line, ".%s", PR_GetString(def->s_name));
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
(etype_t type, eval_t *val)

Returns a string describing *data in a type specific manner
Easier to parse than PR_ValueString
=============
*/
static const char *PR_UglyValueString (int type, eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", PR_GetString(val->string));
		break;
	case ev_entity:
		sprintf (line, "%i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
		break;
	case ev_function:
		f = pr_functions + val->function;
		sprintf (line, "%s", PR_GetString(f->s_name));
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		sprintf (line, "%s", PR_GetString(def->s_name));
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
const char *PR_GlobalString (int ofs)
{
	const char	*s;
	int		i;
	ddef_t		*def;
	void		*val;
	static char	line[128];

	val = (void *)&pr_globals[ofs];
	def = ED_GlobalAtOfs(ofs);
	if (!def)
		sprintf (line,"%i(?)", ofs);
	else
	{
		s = PR_ValueString (def->type, (eval_t *)val);
		sprintf (line,"%i(%s)%s", ofs, PR_GetString(def->s_name), s);
	}

	i = strlen(line);
	for ( ; i < 20; i++)
		strcat (line, " ");
	strcat (line, " ");

	return line;
}

const char *PR_GlobalStringNoContents (int ofs)
{
	int		i;
	ddef_t		*def;
	static char	line[128];

	def = ED_GlobalAtOfs(ofs);
	if (!def)
		sprintf (line,"%i(?)", ofs);
	else
		sprintf (line,"%i(%s)", ofs, PR_GetString(def->s_name));

	i = strlen(line);
	for ( ; i < 20; i++)
		strcat (line, " ");
	strcat (line, " ");

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
	ddef_t	*d;
	int		*v;
	int		i, j, l;
	const char	*name;
	int		type;

	if (ed->free)
	{
		Con_Printf ("FREE\n");
		return;
	}

	Con_Printf("\nEDICT %i:\n", NUM_FOR_EDICT(ed));
	for (i = 1; i < progs->numfielddefs; i++)
	{
		d = &pr_fielddefs[i];
		name = PR_GetString(d->s_name);
		l = strlen (name);
		j = l - 1;
		if (j > 0 && name[j-1] == '_' && name[j] >= 'x' && name[j] <= 'z')
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;

		for (j = 0; j < type_size[type]; j++)
		{
			if (v[j])
				break;
		}
		if (j == type_size[type])
			continue;

		Con_Printf ("%s", name);
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
	const char	*name;
	int		type;

	fprintf (f, "{\n");

	if (ed->free)
	{
		fprintf (f, "}\n");
		return;
	}

	RemoveBadReferences = true;

	if (ed->v.classname)
		q_strlcpy (class_name, PR_GetString(ed->v.classname), sizeof(class_name));
	else
		class_name[0] = 0;

	for (i = 1; i < progs->numfielddefs; i++)
	{
		d = &pr_fielddefs[i];
		name = PR_GetString(d->s_name);
		j = strlen(name) - 1;
		if (j > 0 && name[j-1] == '_' && name[j] >= 'x' && name[j] <= 'z')
			continue;	// skip _x, _y, _z vars

		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;
		for (j = 0; j < type_size[type]; j++)
		{
			if (v[j])
				break;
		}
		if (j == type_size[type])
			continue;

		q_strlcpy(field_name, name, sizeof(field_name));
		fprintf (f, "\"%s\" ", name);
		fprintf (f, "\"%s\"\n", PR_UglyValueString(d->type, (eval_t *)v));
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

	if (!sv.active)
		return;

	Con_Printf ("%i entities\n", sv.num_edicts);
	for (i = 0; i < sv.num_edicts; i++)
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

	if (!sv.active)
		return;

	i = atoi (Cmd_Argv(1));
	if (i < 0 || i >= sv.num_edicts)
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
	edict_t	*ent;
	int	i, active, models, solid, step;

	if (!sv.active)
		return;

	active = models = solid = step = 0;
	for (i = 0; i < sv.num_edicts; i++)
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
	const char		*name;
	int			type;

	fprintf (f, "{\n");
	for (i = 0; i < progs->numglobaldefs; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type;
		if ( !(def->type & DEF_SAVEGLOBAL) )
			continue;
		type &= ~DEF_SAVEGLOBAL;

		if (type != ev_string && type != ev_float && type != ev_entity)
			continue;

		name = PR_GetString(def->s_name);
		fprintf (f, "\"%s\" ", name);
		fprintf (f, "\"%s\"\n", PR_UglyValueString(type, (eval_t *)&pr_globals[def->ofs]));
	}
	fprintf (f, "}\n");
}

/*
=============
ED_ParseGlobals
=============
*/
void ED_ParseGlobals (const char *data)
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
			Host_Error ("%s: EOF without closing brace", __thisfunc__);

		q_strlcpy (keyname, com_token, sizeof(keyname));

	// parse value
		data = COM_Parse (data);
		if (!data)
			Host_Error ("%s: EOF without closing brace", __thisfunc__);

		if (com_token[0] == '}')
			Host_Error ("%s: closing brace without data", __thisfunc__);

		key = ED_FindGlobal (keyname);
		if (!key)
		{
			Con_Printf ("'%s' is not a global\n", keyname);
			continue;
		}

		if (!ED_ParseEpair ((void *)pr_globals, key, com_token))
			Host_Error ("%s: parse error", __thisfunc__);
	}
}

//============================================================================


/*
=============
ED_NewString
=============
*/
static string_t ED_NewString (const char *string)
{
	char	*new_p;
	int		i, l;
	string_t	num;

	l = strlen(string) + 1;
	num = PR_AllocString (l, &new_p);

	for (i = 0; i < l; i++)
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

	return num;
}


/*
=============
ED_ParseEval

Can parse either fields or globals
returns false if error
=============
*/
static qboolean	ED_ParseEpair (void *base, ddef_t *key, const char *s)
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
		*(string_t *)d = ED_NewString(s);
		break;

	case ev_float:
		*(float *)d = atof (s);
		break;

	case ev_vector:
		q_strlcpy (string, s, sizeof(string));
		v = string;
		w = string;
		for (i = 0; i < 3; i++)
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
const char *ED_ParseEdict (const char *data, edict_t *ent)
{
	ddef_t		*key;
	char		keyname[256];
	qboolean	anglehack, init;
	int		n;

	init = false;

	// clear it
	if (ent != sv.edicts)	// hack // we rely on this..
		memset (&ent->v, 0, progs->entityfields * 4);

	// go through all the dictionary pairs
	while (1)
	{
		// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Host_Error ("%s: EOF without closing brace", __thisfunc__);

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

		q_strlcpy (keyname, com_token, sizeof(keyname));

		// another hack to fix keynames with trailing spaces
		n = strlen(keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

		// parse value
		data = COM_Parse (data);
		if (!data)
			Host_Error ("%s: EOF without closing brace", __thisfunc__);

		if (com_token[0] == '}')
			Host_Error ("%s: closing brace without data", __thisfunc__);

		init = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		if (q_strcasecmp(keyname,"MIDI") == 0)
		{
			q_strlcpy(sv.midi_name, com_token, sizeof(sv.midi_name));
			continue;
		}
		else if (q_strcasecmp(keyname,"CD") == 0)
		{
			sv.cd_track = (byte)atoi(com_token);
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
			Host_Error ("%s: parse error", __thisfunc__);
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
void ED_LoadFromFile (const char *data)
{
	dfunction_t	*func;
	edict_t		*ent = NULL;
	int		inhibit = 0;
#if !defined(SERVERONLY)
	int		start_amount = current_loading_size;
	const char	*orig = data;
#endif	/* SERVERONLY */

	*sv_globals.time = sv.time;

	// parse ents
	while (1)
	{
		// parse the opening brace
		data = COM_Parse (data);
		if (!data)
			break;

#if !defined(SERVERONLY)
		if (entity_file_size)
		{
			current_loading_size = start_amount + ((data - orig) * 80 / entity_file_size);
			D_ShowLoadingSize();
		}
#endif	/* SERVERONLY */

		if (com_token[0] != '{')
			Host_Error ("%s: found %s when expecting {", __thisfunc__, com_token);

		if (!ent)
			ent = EDICT_NUM(0);
		else
			ent = ED_Alloc ();
		data = ED_ParseEdict (data, ent);

#if 0
		//jfm fuckup test
		//remove for final release
		if (ent->v.spawnflags > 1 && !strcmp("worldspawn", PR_GetString(ent->v.classname)))
		{
			Host_Error ("invalid SpawnFlags on World!!!\n");
		}
#endif

		// remove things from different skill levels or deathmatch
		if (deathmatch.integer)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH))
			{
				ED_Free (ent);
				inhibit++;
				continue;
			}
		}
		else if (coop.integer)
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
			int		skip;

			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_SINGLE))
			{
				ED_Free (ent);
				inhibit++;
				continue;
			}

			skip = 0;

#if !defined(SERVERONLY)
			switch (cl_playerclass.integer)
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
#endif	/* !SERVERONLY */

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
		func = ED_FindFunction ( PR_GetString(ent->v.classname) );

		if (!func)
		{
			Con_Printf ("No spawn function for:\n");
			ED_Print (ent);
			ED_Free (ent);
			continue;
		}

		*sv_globals.self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (func - pr_functions);
	}

	Con_DPrintf ("%i entities inhibited\n", inhibit);
}


/*
===============
PR_GetProgFilename

return the correct progs filename based on map name
by parsing maplist.txt
===============
*/
static const char def_progname[] = "progs.dat";
static const char *PR_GetProgFilename (void)
{
#if !USE_MULTIPLE_PROGS
	return def_progname;
#else
static const char maplist_name[] = "maplist.txt";
/* original format:
 * line #1 : <number of lines excluding this one>
 * line #2+: <map name><one space><prog filename>
 */
	static char	finalprogname[MAX_QPATH];
	unsigned int	id0, id1;
	fshandle_t	FH;

	FH.length = (long) FS_OpenFile (maplist_name, & FH.file, &id1);
	FH.pak = file_from_pak;
	if (FH.file == NULL)
		return def_progname;
	else if (FS_FileExists(def_progname, &id0) && id1 < id0)
	{
		Con_DPrintf("ignored %s from a gamedir with lower priority\n", maplist_name);
		goto _fail;
	}
	else
	{
		char	build[256], *test;
		int	entries;

		FH.pos = 0;
		FH.start = ftell(FH.file);

		if (!FS_fgets(build, sizeof(build), &FH))
			goto _fail;
		entries = atoi(build);
		if (entries <= 0)
			goto _fail;

		while (--entries >= 0)
		{
			if (!(test = FS_fgets (build, sizeof(build), &FH)))
				goto _fail; /* unexpected EOF */
			while (*test)
			{
				if (*test == '\r' || *test == '\n')
				{
					*test = '\0';
					break;
				}
				if (*test == '\t')
					*test = ' ';
				++test;
			}
			while (--test > &build[0])
			{
				if (*test == ' ')
					*test = '\0';
				else
					break;
			}
			if (!(test = strchr(build, ' ')))
				continue;
			*test = 0;
			if (q_strcasecmp(build, sv.name) == 0)
			{
				FS_fclose (&FH);
				while (*(++test) == ' ')
					;
				q_strlcpy(finalprogname, test, sizeof(finalprogname));
				return finalprogname;
			}
		}
	}
_fail:
	FS_fclose (&FH);
	return def_progname;
#endif	/* end of USE_MULTIPLE_PROGS */
}

static void set_address (sv_def_t *def, void *address)
{
	switch (def->type) {
		case ev_void:
		case ev_bad:
			break;
		case ev_float:
		case ev_vector:
			*(float **)def->field = (float *) address;
			break;
		case ev_string:
		case ev_entity:
		case ev_field:
		case ev_function:
		case ev_pointer:
			*(int **)def->field = (int *) address;
			break;
	}
}

/*
===============
PR_ConvertV6Defs, PR_ConvertV6Stmts -- Pa3PyX

Convert ddef_v6_t and dstatement_v6_t arrays into _v7 format
with byte swapping.  See PR_ExecuteProgram() for more info.
===============
*/
static ddef_v7_t *PR_ConvertV6Defs (ddef_v6_t *v6defs, int numdefs)
{
	int		i;
	ddef_v7_t	*v7defs, *v7ptr;
	ddef_v6_t	*v6ptr;

	v7defs = (ddef_v7_t *) Hunk_AllocName(sizeof(ddef_v7_t) * numdefs, "prog7defs");
	for (i = 0, v6ptr = v6defs, v7ptr = v7defs; i < numdefs; i++, v6ptr++, v7ptr++)
	{
		v7ptr->type = LittleShort(v6ptr->type);
		v7ptr->ofs = (unsigned short)LittleShort(v6ptr->ofs);
		v7ptr->s_name = LittleLong(v6ptr->s_name);
	}

	return v7defs;
}

static dstatement_v7_t *PR_ConvertV6Stmts (dstatement_v6_t *v6stmts, int numstmts)
{
	int		i;
	dstatement_v7_t	*v7stmts, *v7ptr;
	dstatement_v6_t	*v6ptr;

	v7stmts = (dstatement_v7_t *) Hunk_AllocName(sizeof(dstatement_v7_t) * numstmts, "prog7stmt");
	for (i = 0, v6ptr = v6stmts, v7ptr = v7stmts; i < numstmts; i++, v6ptr++, v7ptr++)
	{
		v7ptr->op = LittleShort(v6ptr->op);
		v7ptr->a = (unsigned short)LittleShort(v6ptr->a);
		v7ptr->b = (unsigned short)LittleShort(v6ptr->b);
		v7ptr->c = (unsigned short)LittleShort(v6ptr->c);
	}

	return v7stmts;
}

/*
===============
PR_LoadProgs
===============
*/
void PR_LoadProgs (void)
{
	int			i;
	const char	*progname;
	const char	*progvstr;
	sv_def_t	*def;

	// flush the non-C variable lookup cache
	for (i = 0; i < GEFV_CACHESIZE; i++)
		gefvCache[i].field[0] = 0;

	progname = PR_GetProgFilename();
	progs = (dprograms_t *)FS_LoadHunkFile (progname, NULL);
	if (!progs)
		Host_Error ("%s: couldn't load %s", __thisfunc__, progname);
	Con_DPrintf ("Programs occupy %luK.\n", (unsigned long)(fs_filesize/1024));

	pr_crc = CRC_Block ((byte *)progs, fs_filesize);

	// byte swap the header
	for (i = 0; i < (int) sizeof(*progs) / 4; i++)
		((int *)progs)[i] = LittleLong ( ((int *)progs)[i] );

	switch (progs->version)
	{
	case PROG_VERSION_V6:
		is_progs_v6 = true;
		break;
	case PROG_VERSION_V7:
		is_progs_v6 = false;
		break;
	default:
		Host_Error ("%s is of unsupported version (%d, should be %d or %d)",
			    progname, progs->version, PROG_VERSION_V6, PROG_VERSION_V7);
		return; /* silence compiler */
	}
	switch (progs->crc)
	{
	case PROGS_V103_CRC:
		def = globals_v103;
		progvstr = "H2/v1.03";
		break;
	case PROGS_V111_CRC:
		def = globals_v111;
		progvstr = "H2/v1.11";
		break;
	case PROGS_V112_CRC:
		def = globals_v112;
		progvstr = "H2MP/v1.12";
		break;
	default:
		Host_Error ("Unexpected crc ( %d ) for %s", progs->crc, progname);
		return; /* silence compiler */
	}

	pr_functions = (dfunction_t *)((byte *)progs + progs->ofs_functions);
	pr_strings = (char *)progs + progs->ofs_strings;
	if (progs->ofs_strings + progs->numstrings >= (int)fs_filesize)
		Host_Error ("%s: strings go past end of file\n", progname);

	// initialize the strings
	pr_numknownstrings = 0;
	pr_maxknownstrings = 0;
	pr_stringssize = progs->numstrings;
	if (pr_knownstrings)
		Z_Free ((void *)pr_knownstrings);
	pr_knownstrings = NULL;
	PR_SetEngineString(pr_null_string);

	if (progs->version == PROG_VERSION_V6)
	{
		pr_globaldefs = PR_ConvertV6Defs ((ddef_v6_t *)((byte *)progs + progs->ofs_globaldefs), progs->numglobaldefs);
		pr_fielddefs  = PR_ConvertV6Defs ((ddef_v6_t *)((byte *)progs + progs->ofs_fielddefs),  progs->numfielddefs);
		pr_statements = PR_ConvertV6Stmts((dstatement_v6_t *)((byte *)progs + progs->ofs_statements), progs->numstatements);
	}
	else
	{
		pr_globaldefs = (ddef_t *)((byte *)progs + progs->ofs_globaldefs);
		pr_fielddefs = (ddef_t *)((byte *)progs + progs->ofs_fielddefs);
		pr_statements = (dstatement_t *)((byte *)progs + progs->ofs_statements);
	}

	Con_DPrintf ("Loaded %s, v%d, %d crc, %s structures\n",
			progname, progs->version, progs->crc, progvstr);

	memset (&sv_globals, 0, sizeof(sv_globals));
	pr_globals = (float *)((byte *)progs + progs->ofs_globals);
	for (; def->field; def++)
		set_address (def, &G_FLOAT(def->offset));

	// byte swap the lumps
	for (i = 0; i < progs->numfunctions; i++)
	{
		pr_functions[i].first_statement = LittleLong (pr_functions[i].first_statement);
		pr_functions[i].parm_start = LittleLong (pr_functions[i].parm_start);
		pr_functions[i].s_name = LittleLong (pr_functions[i].s_name);
		pr_functions[i].s_file = LittleLong (pr_functions[i].s_file);
		pr_functions[i].numparms = LittleLong (pr_functions[i].numparms);
		pr_functions[i].locals = LittleLong (pr_functions[i].locals);
	}

	if (progs->version == PROG_VERSION_V7)
	{
		for (i = 0; i < progs->numstatements; i++)
		{
			pr_statements[i].op = LittleShort(pr_statements[i].op);
			pr_statements[i].a = LittleLong(pr_statements[i].a);
			pr_statements[i].b = LittleLong(pr_statements[i].b);
			pr_statements[i].c = LittleLong(pr_statements[i].c);
		}

		for (i = 0; i < progs->numglobaldefs; i++)
		{
			pr_globaldefs[i].type = LittleShort (pr_globaldefs[i].type);
			pr_globaldefs[i].ofs = LittleLong (pr_globaldefs[i].ofs);
			pr_globaldefs[i].s_name = LittleLong (pr_globaldefs[i].s_name);
		}

		for (i = 0; i < progs->numfielddefs; i++)
		{
			pr_fielddefs[i].type = LittleShort (pr_fielddefs[i].type);
			pr_fielddefs[i].ofs = LittleLong (pr_fielddefs[i].ofs);
			pr_fielddefs[i].s_name = LittleLong (pr_fielddefs[i].s_name);
		}
	}

	for (i = 0; i < progs->numfielddefs; i++)
	{
		if (pr_fielddefs[i].type & DEF_SAVEGLOBAL)
			Host_Error ("%s: pr_fielddefs[i].type & DEF_SAVEGLOBAL", __thisfunc__);
	}

	for (i = 0; i < progs->numglobals; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);

	pr_edict_size = progs->entityfields * 4 + sizeof(edict_t) - sizeof(entvars_t);
	// round off to next highest whole word address (esp for Alpha)
	// this ensures that pointers in the engine data area are always
	// properly aligned
	pr_edict_size += sizeof(void *) - 1;
	pr_edict_size &= ~(sizeof(void *) - 1);

#if !defined(SERVERONLY)
	// set the cl_playerclass value after sv_globals has been created
	if (sv_globals.cl_playerclass)
		*sv_globals.cl_playerclass = cl_playerclass.value;
#endif
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

//===========================================================================


edict_t *EDICT_NUM(int n)
{
	if (n < 0 || n >= MAX_EDICTS)
		Host_Error ("%s: bad number %i", __thisfunc__, n);
	return (edict_t *)((byte *)sv.edicts + (n)*pr_edict_size);
}

int NUM_FOR_EDICT(edict_t *e)
{
	int		b;

	b = (byte *)e - (byte *)sv.edicts;
	b = b / pr_edict_size;

	if (b < 0 || b >= sv.num_edicts)
	{
		if (!RemoveBadReferences)
		{
			Con_DPrintf ("%s: bad pointer, Class: %s Field: %s, Index %d, Total %d\n",
					__thisfunc__, class_name, field_name, b, sv.num_edicts);
		}
		return 0;
	}
	if (e->free && RemoveBadReferences)
	{
	//	Con_DPrintf ("%s: freed edict, Class: %s Field: %s, Index %d, Total %d\n",
	//			__thisfunc__, class_name, field_name, b, sv.num_edicts);
		return 0;
	}
	return b;
}

//===========================================================================


#define	PR_STRING_ALLOCSLOTS	256

static void PR_AllocStringSlots (void)
{
	pr_maxknownstrings += PR_STRING_ALLOCSLOTS;
	Sys_DPrintf("%s: realloc'ing for %d slots\n", __thisfunc__, pr_maxknownstrings);
	pr_knownstrings = (const char **) Z_Realloc ((void *)pr_knownstrings, pr_maxknownstrings * sizeof(char *), Z_MAINZONE);
}

const char *PR_GetString (int num)
{
	if (num >= 0 && num < pr_stringssize)
		return pr_strings + num;
	else if (num < 0 && num >= -pr_numknownstrings)
	{
		if (!pr_knownstrings[-1 - num])
		{
			Host_Error ("%s: attempt to get a non-existant string %d\n",
								__thisfunc__, num);
			return "";
		}
		return pr_knownstrings[-1 - num];
	}
	else
	{
		Host_Error ("%s: invalid string offset %d\n", __thisfunc__, num);
		return "";
	}
}

int PR_SetEngineString (const char *s)
{
	int		i;

	if (!s)
		return 0;
#if 0	/* can't: sv.model_precache & sv.sound_precache points to pr_strings */
	if (s >= pr_strings && s <= pr_strings + pr_stringssize)
		Host_Error ("%s: \"%s\" is in pr_strings area\n", __thisfunc__, s);
#else
	if (s >= pr_strings && s <= pr_strings + pr_stringssize - 2)
		return (int)(s - pr_strings);
#endif
	for (i = 0; i < pr_numknownstrings; i++)
	{
		if (pr_knownstrings[i] == s)
			return -1 - i;
	}
	// new unknown engine string
	DEBUG_Printf ("%s: new engine string %p\n", __thisfunc__, s);
#if 0
	for (i = 0; i < pr_numknownstrings; i++)
	{
		if (!pr_knownstrings[i])
			break;
	}
#endif
//	if (i >= pr_numknownstrings)
//	{
		if (i >= pr_maxknownstrings)
			PR_AllocStringSlots();
		pr_numknownstrings++;
//	}
	pr_knownstrings[i] = s;
	return -1 - i;
}

int PR_AllocString (int size, char **ptr)
{
	int		i;

	if (!size)
		return 0;
	for (i = 0; i < pr_numknownstrings; i++)
	{
		if (!pr_knownstrings[i])
			break;
	}
//	if (i >= pr_numknownstrings)
//	{
		if (i >= pr_maxknownstrings)
			PR_AllocStringSlots();
		pr_numknownstrings++;
//	}
	pr_knownstrings[i] = (char *)Hunk_AllocName(size, "string");
	if (ptr)
		*ptr = (char *) pr_knownstrings[i];
	return -1 - i;
}

