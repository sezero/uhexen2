/*
 * hcc.c
 * HCode compiler based on qcc, modifed by Eric Hobbs to work with DCC
 *
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997 Eric Hobbs <elhobbs@comcast.net>
 * Copyright (C) 2005-2012 O.Sezer <sezero@users.sourceforge.net>
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
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "util_io.h"
#include "crc.h"
#include "qcc.h"
#include "dcc.h"
#include "q_endian.h"
#include "byteordr.h"
#include "filenames.h"

static void PR_BeginCompilation (void);
static qboolean PR_FinishCompilation (void);

int		hcc_version_req;
int		hcc_OptimizeStringHeap;

byte		_pr_globals[MAX_REGS * sizeof(float)];
float		*pr_globals;
int			numpr_globals;

char		strings[MAX_STRINGS];
int			strofs;

dstatement_t	statements[MAX_STATEMENTS];
int			numstatements;
int			statement_linenums[MAX_STATEMENTS];

dfunction_t	functions[MAX_FUNCTIONS];
int			numfunctions;

ddef_t		globals[MAX_GLOBALS];
int			numglobaldefs;

ddef_t		fields[MAX_FIELDS];
int			numfielddefs;

char		precache_sounds[MAX_SOUNDS][MAX_DATA_PATH];
int			precache_sounds_block[MAX_SOUNDS];
int			numsounds;

char		precache_models[MAX_MODELS][MAX_DATA_PATH];
int			precache_models_block[MAX_SOUNDS];
int			nummodels;

char		precache_files[MAX_FILES][MAX_DATA_PATH];
int			precache_files_block[MAX_SOUNDS];
int			numfiles;

static char	sourcedir[1024];
static char	destfile[1024];


/*
============
WriteFiles

  Generates files.dat, which contains all of the
  data files actually used by the game, to be
  processed by qfiles.exe
============
*/
static void WriteFiles (void)
{
	FILE	*f;
	int		i;
	char	filename[1024];

	q_snprintf (filename, sizeof(filename), "%sfiles.dat", sourcedir);
	f = fopen (filename, "w");
	if (!f)
		COM_Error ("Couldn't open %s", filename);

	fprintf (f, "%i\n", numsounds);
	for (i = 0; i < numsounds; i++)
		fprintf (f, "%i %s\n", precache_sounds_block[i], precache_sounds[i]);

	fprintf (f, "%i\n", nummodels);
	for (i = 0; i < nummodels; i++)
		fprintf (f, "%i %s\n", precache_models_block[i], precache_models[i]);

	fprintf (f, "%i\n", numfiles);
	for (i = 0; i < numfiles; i++)
		fprintf (f, "%i %s\n", precache_files_block[i], precache_files[i]);

	fclose (f);
}


/* CopyString returns an offset from the string heap */
typedef struct stringtab_s
{
	struct stringtab_s	*prev;
	char			*ofs;
	int			len;
} stringtab_t;
/* stringtab stuff stolen from FTEQCC */
#define MAX_CHARS 256 /* UCHAR_MAX+1 */
static stringtab_t	**stringtablist;

int CopyString (const char *str)
{
	int	ofs, l;
/*	char		*s;*/
	unsigned char	key;
	stringtab_t	*t;

	if (hcc_OptimizeStringHeap)
	{
		if (!*str) return 0;
		/*
		for (ofs = 0, s = strings; ofs < strofs; ofs += l, s += l)
		{
			if (!strcmp(s, str))
				return ofs;
			l = strlen(s) + 1;
		}
		*/
		l = strlen(str);
		key = str [l-1];
		for (t = stringtablist[key]; t; t = t->prev)
		{
		#if 0
		/* reusing the tails of longer strings result in an extra
		 * size reduction only about 2K: is it worth the trouble? */
			if (t->len >= l)
			{
				if (!strcmp(t->ofs + t->len - l, str))
					return (t->ofs + t->len - l) - strings;
			}
		#else
			if (t->len == l)
			{
				if (!strcmp(t->ofs, str))
					return (t->ofs - strings);
			}
		#endif
		}
		t = (stringtab_t *) SafeMalloc(sizeof(stringtab_t));
		t->prev = stringtablist[key];
		stringtablist[key] = t;
		t->ofs = strings+strofs;
		t->len = l;
	}

	ofs = strofs;
	strcpy (strings+strofs, str);
	strofs += strlen(str)+1;
	return ofs;
}


#if 0	/* all uses are commented out */
static void PrintStrings (void)
{
	int		i, l;
	const char	*s;

	for (i = 0; i < strofs; i += l)
	{
		s = strings + i;
		l = strlen(s) + 1;
		printf ("%5i : ", i);
		for ( ; *s; ++s)
		{
			if (*s == '\n')
			{
				putchar ('\\');
				putchar ('n');
			}
			else
				putchar (*s);
		}
		printf ("\n");
	}
}

static void PrintFunctions (void)
{
	int		i, j;
	dfunction_t	*d;

	for (i = 0; i < numfunctions; i++)
	{
		d = &functions[i];
		printf ("%s : %s : %i %i (", strings + d->s_file, strings + d->s_name, d->first_statement, d->parm_start);
		for (j = 0; j < d->numparms; j++)
			printf ("%i ", d->parm_size[j]);
		printf (")\n");
	}
}

static void PrintFields (void)
{
	int		i;
	ddef_t	*d;

	for (i = 0; i < numfielddefs; i++)
	{
		d = &fields[i];
		printf ("%5i : (%i) %s\n", d->ofs, d->type, strings + d->s_name);
	}
}

static void PrintGlobals (void)
{
	int		i;
	ddef_t	*d;

	for (i = 0; i < numglobaldefs; i++)
	{
		d = &globals[i];
		printf ("%5i : (%i) %s\n", d->ofs, d->type, strings + d->s_name);
	}
}
#endif	/* end of unused stuff */


static void InitData (void)
{
	int		i;

	numstatements = 1;
	strofs = 1;
	numfunctions = 1;
	numglobaldefs = 1;
	numfielddefs = 1;

	strings[0] = '\0';
	pr_globals = (float *)_pr_globals;
	def_ret.ofs = OFS_RETURN;
	for (i = 0; i < MAX_PARMS; i++)
		def_parms[i].ofs = OFS_PARM0 + 3*i;
	if (hcc_OptimizeStringHeap)
		stringtablist = (stringtab_t **) SafeMalloc(MAX_CHARS * sizeof(stringtab_t *));
}


static ddef_v6_t *PR_GenV6Defs (ddef_v7_t *v7defs, int numdefs)
{
	int		i;
	ddef_v6_t	*v6defs, *v6ptr;
	ddef_v7_t	*v7ptr;

	v6defs = (ddef_v6_t *) SafeMalloc(sizeof(ddef_v6_t) * numdefs);
	for (i = 0, v7ptr = v7defs, v6ptr = v6defs; i < numdefs; i++, v7ptr++, v6ptr++)
	{
		v6ptr->type = LittleShort(v7ptr->type);

		if (v7ptr->ofs > 65535 || v7ptr->ofs < 0 /* really? */)
			goto _v6not;
		v6ptr->ofs = LittleShort((unsigned short)v7ptr->ofs);

		v6ptr->s_name = LittleLong(v7ptr->s_name);
	}

	return v6defs;
_v6not:
	free(v6defs);
	printf("Can't generate v6defs due to incompatible ofs values.\n");
	return NULL;
}

static dstatement_v6_t *PR_GenV6Stmts (dstatement_v7_t *v7stmts, int numstmts)
{
	int		i;
	dstatement_v6_t	*v6stmts, *v6ptr;
	dstatement_v7_t	*v7ptr;

	v6stmts = (dstatement_v6_t *) SafeMalloc(sizeof(dstatement_v6_t) * numstmts);
	for (i = 0, v7ptr = v7stmts, v6ptr = v6stmts; i < numstmts; i++, v7ptr++, v6ptr++)
	{
		v6ptr->op = LittleShort(v7ptr->op);

		if (v7ptr->a > 65535 || v7ptr->a < -32768)
			goto _v6not;
		if (v7ptr->b > 65535 || v7ptr->b < -32768)
			goto _v6not;
		if (v7ptr->c > 65535 || v7ptr->c < -32768)
			goto _v6not;
		v6ptr->a = LittleShort((signed short)v7ptr->a);
		v6ptr->b = LittleShort((signed short)v7ptr->b);
		v6ptr->c = LittleShort((signed short)v7ptr->c);
	}

	return v6stmts;
_v6not:
	free(v6stmts);
	printf("Can't generate v6stmts due to incompatible a/b/c values.\n");
	return NULL;
}

static void WriteData (int crc)
{
	def_t		*def;
	ddef_t		*dd;
	dstatement_v6_t	*stmt6;
	ddef_v6_t	*globals6, *fields6;
	qboolean	v6able;
	dprograms_t	progs;
	FILE	*h;
	int	i;

	if (hcc_OptimizeStringHeap)
		printf ("compacting string heap\n");

	for (def = pr.def_head.next; def; def = def->next)
	{
		if (def->type->type == ev_field)
		{
			dd = &fields[numfielddefs];
			numfielddefs++;
			dd->type = def->type->aux_type->type;
			dd->s_name = CopyString (def->name);
			dd->ofs = G_INT(def->ofs);
		}
		dd = &globals[numglobaldefs];
		numglobaldefs++;
		dd->type = def->type->type;
		if (!def->initialized &&
		    def->type->type != ev_function &&
		    def->type->type != ev_field &&
		    def->scope == NULL)
		{
			dd->type |= DEF_SAVEGLOBAL;
		}
		dd->s_name = CopyString (def->name);
		dd->ofs = def->ofs;
	}

//	PrintStrings ();
//	PrintFunctions ();
//	PrintFields ();
//	PrintGlobals ();

	strofs = (strofs + 3) & ~3;

	printf("object file %s\n", destfile);
	printf("      registers: %10d / %10d (%10d bytes)\n", numpr_globals, MAX_REGS, numpr_globals*(int)sizeof(float));
	printf("     statements: %10d / %10d (%10d bytes)\n", numstatements, MAX_STATEMENTS, numstatements*(int)sizeof(dstatement_t));
	printf("      functions: %10d / %10d (%10d bytes)\n", numfunctions, MAX_FUNCTIONS, numfunctions*(int)sizeof(dfunction_t));
	printf("    global defs: %10d / %10d (%10d bytes)\n", numglobaldefs, MAX_GLOBALS, numglobaldefs*(int)sizeof(ddef_t));
	printf("     field defs: %10d / %10d (%10d bytes)\n", numfielddefs, MAX_FIELDS, numfielddefs*(int)sizeof(ddef_t));
	printf("    string heap: %10d / %10d\n", strofs, MAX_STRINGS);
	printf("  entity fields: %10d\n", pr.size_fields);

	stmt6 = NULL;
	globals6 = NULL;
	fields6 = NULL;
	if (numpr_globals < 65536)
		v6able = true;
	else
	{
		v6able = false;
		if (hcc_version_req != PROG_VERSION_V7)
			printf("Too many registers: version 6 output not possible.\n");
	}

	if (v6able && hcc_version_req != PROG_VERSION_V7)
	{
		v6able = (stmt6 = PR_GenV6Stmts(statements, numstatements)) != NULL;
		if ( v6able) v6able = (globals6 = PR_GenV6Defs(globals, numglobaldefs)) != NULL;
		if ( v6able) v6able = (fields6 = PR_GenV6Defs(fields, numfielddefs)) != NULL;
		if (!v6able)
		{
			if (stmt6) free (stmt6);
			if (globals6) free (globals6);
		}
	}

	if (!v6able)
	{
		if (hcc_version_req == PROG_VERSION_V6)
			COM_Error("Can not output version 6 progs: v6 limitations not complied with.");
		hcc_version_req = PROG_VERSION_V7;
	}
	else
	{
		if (hcc_version_req == -1)
			hcc_version_req = PROG_VERSION_V6;
	}

	h = SafeOpenWrite (destfile);
	SafeWrite (h, &progs, sizeof(progs));

	progs.ofs_strings = ftell (h);
	progs.numstrings = strofs;
	SafeWrite (h, strings, strofs);

	progs.ofs_statements = ftell (h);
	progs.numstatements = numstatements;
	if (hcc_version_req == PROG_VERSION_V7)
	{
		for (i = 0; i < numstatements; i++)
		{
			statements[i].op = LittleShort(statements[i].op);
			statements[i].a = LittleLong(statements[i].a);
			statements[i].b = LittleLong(statements[i].b);
			statements[i].c = LittleLong(statements[i].c);
		}
		SafeWrite (h, statements, numstatements*sizeof(dstatement_t));
	}
	else
	{
		SafeWrite (h, stmt6, numstatements*sizeof(dstatement_v6_t));
		free (stmt6);
	}

	progs.ofs_functions = ftell (h);
	progs.numfunctions = numfunctions;
	for (i = 0; i < numfunctions; i++)
	{
		functions[i].first_statement = LittleLong (functions[i].first_statement);
		functions[i].parm_start = LittleLong (functions[i].parm_start);
		functions[i].s_name = LittleLong (functions[i].s_name);
		functions[i].s_file = LittleLong (functions[i].s_file);
		functions[i].numparms = LittleLong (functions[i].numparms);
		functions[i].locals = LittleLong (functions[i].locals);
	}
	SafeWrite (h, functions, numfunctions*sizeof(dfunction_t));

	progs.ofs_globaldefs = ftell (h);
	progs.numglobaldefs = numglobaldefs;
	if (hcc_version_req == PROG_VERSION_V7)
	{
		for (i = 0; i < numglobaldefs; i++)
		{
			globals[i].type = LittleShort (globals[i].type);
			globals[i].ofs = LittleLong (globals[i].ofs);
			globals[i].s_name = LittleLong (globals[i].s_name);
		}
		SafeWrite (h, globals, numglobaldefs*sizeof(ddef_t));
	}
	else
	{
		SafeWrite (h, globals6, numglobaldefs*sizeof(ddef_v6_t));
		free (globals6);
	}

	progs.ofs_fielddefs = ftell (h);
	progs.numfielddefs = numfielddefs;
	if (hcc_version_req == PROG_VERSION_V7)
	{
		for (i = 0; i < numfielddefs; i++)
		{
			fields[i].type = LittleShort (fields[i].type);
			fields[i].ofs = LittleLong (fields[i].ofs);
			fields[i].s_name = LittleLong (fields[i].s_name);
		}
		SafeWrite (h, fields, numfielddefs*sizeof(ddef_t));
	}
	else
	{
		SafeWrite (h, fields6, numfielddefs*sizeof(ddef_v6_t));
		free (fields6);
	}

	progs.ofs_globals = ftell (h);
	progs.numglobals = numpr_globals;
	for (i = 0; i < numpr_globals; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
	SafeWrite (h, pr_globals, numpr_globals*4);

	printf("     total size: %10d bytes\n", (int)ftell(h));
	printf("  progs version: %10d\n", hcc_version_req);

	progs.entityfields = pr.size_fields;

	progs.version = hcc_version_req;
	progs.crc = crc;

// byte swap the header and write it out
	for (i = 0; i < (int)sizeof(progs)/4; i++)
		((int *)&progs)[i] = LittleLong ( ((int *)&progs)[i] );
	fseek (h, 0, SEEK_SET);
	SafeWrite (h, &progs, sizeof(progs));
	fclose (h);
}


/*
===============
PR_String

Returns a string suitable for printing (no newlines, max 60 chars length)
===============
*/
static const char *PR_String (const char *string)
{
	static char	buf[80];
	char	*s;

	s = buf;
	*s++ = '"';
	while (string && *string)
	{
		if (s == buf + sizeof(buf) - 2)
			break;
		if (*string == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (*string == '"')
		{
			*s++ = '\\';
			*s++ = '"';
		}
		else
			*s++ = *string;
		string++;
		if (s - buf > 60)
		{
			*s++ = '.';
			*s++ = '.';
			*s++ = '.';
			break;
		}
	}
	*s++ = '"';
	*s++ = 0;
	return buf;
}

static def_t *PR_DefForFieldOfs (gofs_t ofs)
{
	def_t	*d;

	for (d = pr.def_head.next; d; d = d->next)
	{
		if (d->type->type != ev_field)
			continue;
		if (((int *)pr_globals)[d->ofs] == ofs)
			return d;
	}
	COM_Error ("%s: couldn't find %i", __thisfunc__, ofs);
	return NULL;
}

/*
============
PR_ValueString

Returns a string describing *data in a type specific manner
=============
*/
static const char *PR_ValueString (etype_t type, void *val)
{
	static char	line[256];
	def_t		*def;
	dfunction_t	*f;

	switch (type)
	{
	case ev_string:
		q_snprintf (line, sizeof(line), "%s", PR_String(strings + *(int *)val));
		break;
	case ev_entity:
		q_snprintf (line, sizeof(line), "entity %i", *(int *)val);
		break;
	case ev_function:
		f = functions + *(int *)val;
		if (!f)
			strcpy (line, "undefined function");
		else
			q_snprintf (line, sizeof(line), "%s()", strings + f->s_name);
		break;
	case ev_field:
		def = PR_DefForFieldOfs ( *(int *)val );
		q_snprintf (line, sizeof(line), ".%s", def->name);
		break;
	case ev_void:
		strcpy (line, "void");
		break;
	case ev_float:
		q_snprintf (line, sizeof(line), "%5.1f", *(float *)val);
		break;
	case ev_vector:
		q_snprintf (line, sizeof(line), "'%5.1f %5.1f %5.1f'", ((float *)val)[0], ((float *)val)[1], ((float *)val)[2]);
		break;
	case ev_pointer:
		strcpy (line, "pointer");
		break;
	default:
		q_snprintf (line, sizeof(line), "bad type %i", type);
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
static const char *PR_GlobalStringNoContents (gofs_t ofs)
{
	static char	line[128];
	def_t		*def;
	int	i;

	def = pr_global_defs[ofs];
	if (!def)
	//	COM_Error ("%s: no def for %i", __thisfunc__, ofs);
		q_snprintf (line, sizeof(line), "%i(?)", ofs);
	else
		q_snprintf (line, sizeof(line), "%i(%s)", ofs, def->name);

	i = strlen(line);
	for ( ; i < 16; i++)
		strcat (line, " ");
	strcat (line, " ");

	return line;
}

static const char *PR_GlobalString (gofs_t ofs)
{
	static char	line[128];
	const char	*s;
	def_t		*def;
	int	i;

	def = pr_global_defs[ofs];
	if (!def)
		return PR_GlobalStringNoContents(ofs);
	if (def->initialized && def->type->type != ev_function)
	{
		s = PR_ValueString (def->type->type, &pr_globals[ofs]);
		q_snprintf (line, sizeof(line), "%i(%s)", ofs, s);
	}
	else
		q_snprintf (line, sizeof(line), "%i(%s)", ofs, def->name);

	i = strlen(line);
	for ( ; i < 16; i++)
		strcat (line, " ");
	strcat (line, " ");

	return line;
}

/*
=================
PR_PrintStatement
=================
*/
static void PR_PrintStatement (dstatement_t *s)
{
	int		i;

	printf ("%4i : %4i : %s ", (int)(s - statements), statement_linenums[s-statements], pr_opcodes[s->op].opname);
	i = strlen(pr_opcodes[s->op].opname);
	for ( ; i < 10; i++)
		printf (" ");

	if (s->op == OP_IF || s->op == OP_IFNOT)
		printf ("%sbranch %i", PR_GlobalString(s->a), s->b);
	else if (s->op == OP_GOTO)
	{
		printf ("branch %i", s->a);
	}
	else if ((unsigned int)(s->op - OP_STORE_F) < 6)
	{
		printf ("%s", PR_GlobalString(s->a));
		printf ("%s", PR_GlobalStringNoContents(s->b));
	}
	else
	{
		if (s->a)
			printf ("%s", PR_GlobalString(s->a));
		if (s->b)
			printf ("%s", PR_GlobalString(s->b));
		if (s->c)
			printf ("%s", PR_GlobalStringNoContents(s->c));
	}
	printf ("\n");
}

/*
==============
PR_BeginCompilation

called before compiling a batch of files, clears the pr struct
==============
*/
static void PR_BeginCompilation (void)
{
	int		i;

	numpr_globals = RESERVED_OFS;
	pr.def_tail = &pr.def_head;
	for (i = 0; i < RESERVED_OFS; i++)
	{
		pr_global_defs[i] = &def_void;
	}

	// Link the function type in so state forward declarations match
	// proper type.
	pr.types = &type_function;
	type_function.next = NULL;
	pr_error_count = 0;
}

/*
==============
PR_FinishCompilation

called after all files are compiled to check for errors
Returns false if errors were detected.
==============
*/
static qboolean PR_FinishCompilation (void)
{
	def_t		*d;
	qboolean	errors;

	errors = false;

	// check to make sure all functions prototyped have code
	for (d = pr.def_head.next; d; d = d->next)
	{
		if (d->type->type == ev_function && !d->scope)
		{
			if (!d->initialized)
			{
				printf("function '%s' was not defined\n", d->name);
				errors = true;
			}
		}
	}

	return !errors;
}

/*
============
PR_WriteProgdefs

Writes the global and entity structures out
Returns a crc of the header, to be stored in the progs file for comparison
at load time.
============
*/
static int PR_WriteProgdefs (const char *filename)
{
	def_t	*d;
	FILE	*f;
	unsigned short	crc;
	int	c;

	printf ("writing %s\n", filename);
	f = fopen (filename, "w");

	// print global vars until the first field is defined
	fprintf (f,"\n/* generated by hcc, do not modify */\n\ntypedef struct\n{\tint\tpad[%i];\n", RESERVED_OFS);
	for (d = pr.def_head.next; d; d = d->next)
	{
		if (!strcmp (d->name, "end_sys_globals"))
			break;

		switch (d->type->type)
		{
		case ev_float:
			fprintf (f, "\tfloat\t%s;\n", d->name);
			break;
		case ev_vector:
			fprintf (f, "\tvec3_t\t%s;\n", d->name);
			d = d->next->next->next;	// skip the elements
			break;
		case ev_string:
			fprintf (f, "\tstring_t\t%s;\n", d->name);
			break;
		case ev_function:
			fprintf (f, "\tfunc_t\t%s;\n", d->name);
			break;
		case ev_entity:
			fprintf (f, "\tint\t%s;\n", d->name);
			break;
		default:
			fprintf (f, "\tint\t%s;\n", d->name);
			break;
		}
	}
	fprintf (f, "} globalvars_t;\n\n");

	// print all fields
	fprintf (f, "typedef struct\n{\n");
	for (d = pr.def_head.next; d; d = d->next)
	{
		if (!strcmp (d->name, "end_sys_fields"))
			break;

		if (d->type->type != ev_field)
			continue;

		switch (d->type->aux_type->type)
		{
		case ev_float:
			fprintf (f, "\tfloat\t%s;\n", d->name);
			break;
		case ev_vector:
			fprintf (f, "\tvec3_t\t%s;\n", d->name);
			d = d->next->next->next;	// skip the elements
			break;
		case ev_string:
			fprintf (f, "\tstring_t\t%s;\n", d->name);
			break;
		case ev_function:
			fprintf (f, "\tfunc_t\t%s;\n", d->name);
			break;
		case ev_entity:
			fprintf (f, "\tint\t%s;\n", d->name);
			break;
		default:
			fprintf (f, "\tint\t%s;\n", d->name);
			break;
		}
	}
	fprintf (f, "} entvars_t;\n\n");

	fclose (f);

	// do a crc of the file
	CRC_Init (&crc);
	f = fopen (filename, "r+");
	while ((c = fgetc(f)) != EOF)
		CRC_ProcessByte (&crc, (byte)c);

	fprintf (f, "#define PROGHEADER_CRC %i\n", crc);
	fclose (f);

	return crc;
}

static void PrintFunction (const char *name)
{
	int		i;
	dstatement_t	*ds;
	dfunction_t	*df;

	for (i = 0; i < numfunctions; i++)
		if (!strcmp (name, strings + functions[i].s_name))
			break;
	if (i == numfunctions)
		COM_Error ("No function names \"%s\"", name);
	df = functions + i;

	printf ("Statements for %s:\n", name);
	ds = statements + df->first_statement;
	while (1)
	{
		PR_PrintStatement (ds);
		if (!ds->op)
			break;
		ds++;
	}
}

/*
==============
PR_ParseError
==============
*/
void PR_ParseError (const char *error, ...)
{
	va_list		argptr;
	char	string[1024];

	va_start (argptr, error);
	q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	printf ("%s(%d) : %s\n", strings+s_file, pr_source_line, string);

	longjmp (pr_parse_abort, 1);
}

/*
============
main
============
*/
int main (int argc, char **argv)
{
	const char	*psrc;
	void		*src, *src2;
	char	filename[1024];
	char	*nameptr; /* filename[] without the parent sourcedir */
	int		p, crc;
	double	start, stop;

	myargc = argc;
	myargv = argv;

	if (CheckParm("-?") || CheckParm("-h") || CheckParm("-help") || CheckParm("--help"))
	{
		printf(" Compiles progs.dat using progs.src in the current directory\n");
		printf(" -src <directory> : Specify source directory\n");
		printf(" -name <source>   : Specify the name of the .src file\n");
		printf(" -version <n>     : Output progs as version n (either 6 or 7)\n");
		printf(" -v6              : Output progs as version 6\n");
		printf(" -v7              : Output progs as version 7\n");
		printf(" -dcc (or -dec)   : decompile progs.dat in current directory\n");
		printf(" -dcc -name <progsname> : specify name of progs to decompile\n");
		printf(" -dcc -info : only print brief info about the progs and exit\n");
		printf(" -dcc -fix : fixes mangled names during decompile\n");
		printf(" -dcc -fields     : dumps all fielddefs to stdout\n");
		printf(" -dcc -functions  : dumps all functions to stdout\n");
		printf(" -dcc -globaldefs : dumps all globaldefs to stdout\n");
		printf(" -dcc -prglobals  : dumps all pr_globals to stdout\n");
		printf(" -dcc -statements : dumps all statements to stdout\n");
		printf(" -dcc -strings    : dumps all pr_strings to stdout\n");
		printf(" -dcc -asm <functionname> : decompile filename to the console\n");
		printf(" -dcc -dump -asm <functionname> : same as above but will show\n\t\tinstructions (opcodes and parms) as well\n");
		exit(0);
	}

	ValidateByteorder ();

	if (CheckParm("-dcc") || CheckParm("-dec"))
	{
		Dcc_main (argc, argv);
		exit (0);
	}

	start = COM_GetTime ();

	hcc_version_req = -1;	/* output as v6 if possible, otherwise as v7 */
	p = CheckParm("-version");
	if (p != 0)
	{
		if (p >= argc - 1)
			COM_Error ("No num specified for -version");
		p = atoi(argv[p+1]);
		switch (p)
		{
		case 6:
			hcc_version_req = PROG_VERSION_V6;
			break;
		case 7:
			hcc_version_req = PROG_VERSION_V7;
			break;
		default:
			COM_Error ("Version must be either 6 or 7");
			break;
		}
	}

	if (CheckParm("-v6"))	hcc_version_req = PROG_VERSION_V6;
	if (CheckParm("-v7"))	hcc_version_req = PROG_VERSION_V7;

	p = CheckParm("-src");
	if (p != 0)
	{
	/* everything will now be relative to sourcedir: */
		if (p >= argc - 1)
			COM_Error ("No source dirname specified with -src");
		strcpy(sourcedir, argv[p+1]);
		p = strlen (sourcedir);
		if (p && !IS_DIR_SEPARATOR(sourcedir[p - 1]))
		{
			sourcedir[p] = DIR_SEPARATOR_CHAR;
			sourcedir[p + 1] = '\0';
		}
		printf("Source directory: %s\n", sourcedir);
		strcpy(filename, sourcedir);
		nameptr = strchr(filename, '\0');
	}
	else
	{
		sourcedir[0] = '\0';
		nameptr = filename;
	}

	p = CheckParm("-name");
	if (p != 0)
	{
		if (p >= argc - 1)
			COM_Error ("No input filename specified with -name");
		strcpy(nameptr, argv[p+1]);
		printf("Input file: %s\n", nameptr);
	}
	else
	{
		strcpy(nameptr, "progs.src");
	}
	LoadFile(filename, &src);
	psrc = (char *) src;

	psrc = COM_Parse(psrc);
	if (!psrc)
		COM_Error("No destination filename. dhcc -help for info.");
	q_snprintf(destfile, sizeof(destfile), "%s%s", sourcedir, com_token);
	printf("outputfile: %s\n", destfile);

	hcc_OptimizeStringHeap = 1;

	InitData ();

	PR_BeginCompilation();

	// compile all the files
	do
	{
		psrc = COM_Parse(psrc);
		if (!psrc)
			break;

		strcpy (nameptr, com_token);
		printf ("compiling %s\n", nameptr);
		LoadFile (filename, &src2);

		if (!PR_CompileFile((char *)src2, nameptr))
			exit (1);
	} while (1);

	if (!PR_FinishCompilation())
		COM_Error ("compilation errors");

	p = CheckParm("-asm");
	if (p != 0)
	{
		for (p++; p < argc; p++)
		{
			if (argv[p][0] == '-')
				break;
			PrintFunction(argv[p]);
		}
	}

	// write progdefs.h
	strcpy(nameptr, "progdefs.h");
	crc = PR_WriteProgdefs(filename);

	// write data file
	WriteData(crc);

	// write files.dat
	WriteFiles();
	printf(" precache_sound: %10d / %10d\n", numsounds, MAX_SOUNDS);
	printf(" precache_model: %10d / %10d\n", nummodels, MAX_MODELS);
	printf("  precache_file: %10d / %10d\n", numfiles, MAX_FILES);

	stop = COM_GetTime ();
	printf("\n%d seconds elapsed.\n", (int)(stop - start));

	return 0;
}

