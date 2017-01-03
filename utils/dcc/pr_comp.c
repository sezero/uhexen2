/*
 * comp.c
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
#include "qcc.h"

pr_info_t	pr;
def_t		*pr_global_defs[MAX_REGS];	// to find def for a global variable
int		pr_edict_size;

//========================================

def_t		*pr_scope;		// the function being parsed, or NULL
string_t	s_file;			// filename for function definition

int		locals_end;		// for tracking local variables vs temps

jmp_buf		pr_parse_abort;		// longjump with this on parse error

static void	PR_ParseDefs (void);

//========================================


opcode_t pr_opcodes[] =
{
	{"<DONE>", "DONE", -1, false, &def_entity, &def_field, &def_void},

	{"*", "MUL_F", 2, false, &def_float, &def_float, &def_float},
	{"*", "MUL_V", 2, false, &def_vector, &def_vector, &def_float},
	{"*", "MUL_FV", 2, false, &def_float, &def_vector, &def_vector},
	{"*", "MUL_VF", 2, false, &def_vector, &def_float, &def_vector},

	{"/", "DIV", 2, false, &def_float, &def_float, &def_float},

	{"+", "ADD_F", 3, false, &def_float, &def_float, &def_float},
	{"+", "ADD_V", 3, false, &def_vector, &def_vector, &def_vector},

	{"-", "SUB_F", 3, false, &def_float, &def_float, &def_float},
	{"-", "SUB_V", 3, false, &def_vector, &def_vector, &def_vector},

	{"==", "EQ_F", 4, false, &def_float, &def_float, &def_float},
	{"==", "EQ_V", 4, false, &def_vector, &def_vector, &def_float},
	{"==", "EQ_S", 4, false, &def_string, &def_string, &def_float},
	{"==", "EQ_E", 4, false, &def_entity, &def_entity, &def_float},
	{"==", "EQ_FNC", 4, false, &def_function, &def_function, &def_float},

	{"!=", "NE_F", 4, false, &def_float, &def_float, &def_float},
	{"!=", "NE_V", 4, false, &def_vector, &def_vector, &def_float},
	{"!=", "NE_S", 4, false, &def_string, &def_string, &def_float},
	{"!=", "NE_E", 4, false, &def_entity, &def_entity, &def_float},
	{"!=", "NE_FNC", 4, false, &def_function, &def_function, &def_float},

	{"<=", "LE", 4, false, &def_float, &def_float, &def_float},
	{">=", "GE", 4, false, &def_float, &def_float, &def_float},
	{"<", "LT", 4, false, &def_float, &def_float, &def_float},
	{">", "GT", 4, false, &def_float, &def_float, &def_float},

	{".", "INDIRECT", 1, false, &def_entity, &def_field, &def_float},
	{".", "INDIRECT", 1, false, &def_entity, &def_field, &def_vector},
	{".", "INDIRECT", 1, false, &def_entity, &def_field, &def_string},
	{".", "INDIRECT", 1, false, &def_entity, &def_field, &def_entity},
	{".", "INDIRECT", 1, false, &def_entity, &def_field, &def_field},
	{".", "INDIRECT", 1, false, &def_entity, &def_field, &def_function},

	{".", "ADDRESS", 1, false, &def_entity, &def_field, &def_pointer},

	{"=", "STORE_F", 5, true, &def_float, &def_float, &def_float},
	{"=", "STORE_V", 5, true, &def_vector, &def_vector, &def_vector},
	{"=", "STORE_S", 5, true, &def_string, &def_string, &def_string},
	{"=", "STORE_ENT", 5, true, &def_entity, &def_entity, &def_entity},
	{"=", "STORE_FLD", 5, true, &def_field, &def_field, &def_field},
	{"=", "STORE_FNC", 5, true, &def_function, &def_function, &def_function},

	{"=", "STOREP_F", 5, true, &def_pointer, &def_float, &def_float},
	{"=", "STOREP_V", 5, true, &def_pointer, &def_vector, &def_vector},
	{"=", "STOREP_S", 5, true, &def_pointer, &def_string, &def_string},
	{"=", "STOREP_ENT", 5, true, &def_pointer, &def_entity, &def_entity},
	{"=", "STOREP_FLD", 5, true, &def_pointer, &def_field, &def_field},
	{"=", "STOREP_FNC", 5, true, &def_pointer, &def_function, &def_function},

	{"<RETURN>", "RETURN", -1, false, &def_void, &def_void, &def_void},

	{"!", "NOT_F", -1, false, &def_float, &def_void, &def_float},
	{"!", "NOT_V", -1, false, &def_vector, &def_void, &def_float},
	{"!", "NOT_S", -1, false, &def_vector, &def_void, &def_float},
	{"!", "NOT_ENT", -1, false, &def_entity, &def_void, &def_float},
	{"!", "NOT_FNC", -1, false, &def_function, &def_void, &def_float},

	{"<IF>", "IF", -1, false, &def_float, &def_float, &def_void},
	{"<IFNOT>", "IFNOT", -1, false, &def_float, &def_float, &def_void},

// calls returns REG_RETURN
	{"<CALL0>", "CALL0", -1, false, &def_function, &def_void, &def_void},
	{"<CALL1>", "CALL1", -1, false, &def_function, &def_void, &def_void},
	{"<CALL2>", "CALL2", -1, false, &def_function, &def_void, &def_void},
	{"<CALL3>", "CALL3", -1, false, &def_function, &def_void, &def_void},
	{"<CALL4>", "CALL4", -1, false, &def_function, &def_void, &def_void},
	{"<CALL5>", "CALL5", -1, false, &def_function, &def_void, &def_void},
	{"<CALL6>", "CALL6", -1, false, &def_function, &def_void, &def_void},
	{"<CALL7>", "CALL7", -1, false, &def_function, &def_void, &def_void},
	{"<CALL8>", "CALL8", -1, false, &def_function, &def_void, &def_void},

	{"<STATE>", "STATE", -1, false, &def_float, &def_float, &def_void},

	{"<GOTO>", "GOTO", -1, false, &def_float, &def_void, &def_void},

	{"&&", "AND", 6, false, &def_float, &def_float, &def_float},
	{"||", "OR", 6, false, &def_float, &def_float, &def_float},

	{"&", "BITAND", 2, false, &def_float, &def_float, &def_float},
	{"|", "BITOR", 2, false, &def_float, &def_float, &def_float},

// added by hexen2
	{"*=", "MUL_E_FF", 5, true, &def_float, &def_float, &def_float},
	{"*=", "MUL_E_FV", 5, true, &def_vector, &def_float, &def_vector},
	{"*=", "MULP_E_FV", 5, true, &def_pointer, &def_float, &def_float},
	{"<UNKNOWN>", "<UNKNOWN>", 3, false, &def_void, &def_void, &def_float},
	{"/=", "DIV_E_FF", 5, true, &def_float, &def_float, &def_float},
	{"/=", "DIVP_E_FF", 5, true, &def_pointer, &def_float, &def_float},
	{"+=", "ADD_E_FF", 5, true, &def_float, &def_float, &def_float},
	{"+=", "ADD_E_VV", 5, true, &def_vector, &def_vector, &def_vector},
	{"+=", "ADDP_E_FF", 5, true, &def_pointer, &def_float, &def_float},
	{"+=", "SUBP_E_VV", 5, true, &def_pointer, &def_vector, &def_vector},
	{"-=", "SUB_E_FF", 5, true, &def_float, &def_float, &def_float},
	{"-=", "SUB_E_VV", 5, true, &def_vector, &def_vector, &def_vector},
	{"-=", "SUBP_E_FF", 5, true, &def_pointer, &def_float, &def_float},
	{"-=", "SUBP_E_VV", 5, true, &def_pointer, &def_vector, &def_vector},
	{"->", "ADDRESS", 3, false, &def_float, &def_float, &def_float},
	{"->", "ADDRESS", 3, false, &def_vector, &def_float, &def_vector},
	{"<UNKNOWN>", "<UNKNOWN>", 5, true, &def_void, &def_void, &def_void},
	{"<UNKNOWN>", "<UNKNOWN>", 5, true, &def_void, &def_void, &def_void},
	{"<UNKNOWN>", "<UNKNOWN>", 3, true, &def_void, &def_void, &def_void},
	{"<SETFRAME>", "SET_FRAME", -1, false, &def_float, &def_float, &def_float},
	{"<UNKNOWN>", "<UNKNOWN>", 3, true, &def_pointer, &def_float, &def_float},
	{"<SETTHINK>", "SET_THINKTIME", -1, false, &def_entity, &def_float, &def_void},
	{"|=", "BITOR_E_FF", 5, true, &def_float, &def_float, &def_float},
	{"|=", "BITORP_E_FF", 5, true, &def_pointer, &def_float, &def_float},
	{"^=", "XOR_E_FF", 5, true, &def_float, &def_float, &def_float},	// 90 I think this is right???
	{"^=", "XORP_E_FF", 5, true, &def_pointer, &def_float, &def_float},
	{":>", "RANDOMf", 3, false, &def_float, &def_float, &def_float},
	{":>", "RANDOMf", 3, false, &def_float, &def_float, &def_float},
	{":", "RANDOMf", 3, false, &def_float, &def_float, &def_float},
	{":>", "RANDOMv", 3, false, &def_float, &def_float, &def_float},
	{":>", "RANDOMv", 3, false, &def_vector, &def_vector, &def_vector},
	{":", "RANDOMv", 3, false, &def_vector, &def_vector, &def_vector},	// 97

	{NULL}
};

#define	TOP_PRIORITY	6
#define	NOT_PRIORITY	4

static def_t	*PR_Expression (int priority);

static def_t	junkdef;

//===========================================================================


/*
============
PR_Statement

Emits a primitive statement, returning the var it places it's value in
============
*/
static def_t *PR_Statement ( opcode_t *op, def_t *var_a, def_t *var_b)
{
	dstatement_t	*statement;
	def_t			*var_c;

	statement = &statements[numstatements];
	numstatements++;

	statement_linenums[statement-statements] = pr_source_line;
	statement->op = op - pr_opcodes;
	statement->a = var_a ? var_a->ofs : 0;
	statement->b = var_b ? var_b->ofs : 0;
	if (((op->type_c == &def_void) ^ op->right_associative) || statement->op == OP_BITSETP)
	{
		var_c = NULL;
		statement->c = 0;	// ifs, gotos, and assignments
					// don't need vars allocated
	}
	else
	{	// allocate result space
		var_c = (def_t *) SafeMalloc (sizeof(def_t));
		var_c->ofs = numpr_globals;
		var_c->type = op->type_c->type;

		statement->c = numpr_globals;
		numpr_globals += type_size[op->type_c->type->type];
	}

	if (op->right_associative)
		return var_a;
	return var_c;
}

/*
============
PR_ParseImmediate

Looks for a preexisting constant
============
*/
static def_t *PR_ParseImmediate (void)
{
	def_t	*cn;

	// check for a constant with the same value
	for (cn = pr.def_head.next; cn; cn = cn->next)
	{
		if (!cn->initialized)
			continue;
		if (cn->type != pr_immediate_type)
			continue;
		if (pr_immediate_type == &type_string)
		{
			if (!strcmp(G_STRING(cn->ofs), pr_immediate_string))
			{
				PR_Lex ();
				return cn;
			}
		}
		else if (pr_immediate_type == &type_float)
		{
			if ( G_FLOAT(cn->ofs) == pr_immediate._float )
			{
				PR_Lex ();
				return cn;
			}
		}
		else if	(pr_immediate_type == &type_vector)
		{
			if ((G_FLOAT(cn->ofs) == pr_immediate.vector[0]) &&
			    (G_FLOAT(cn->ofs+1) == pr_immediate.vector[1]) &&
			    (G_FLOAT(cn->ofs+2) == pr_immediate.vector[2]))
			{
				PR_Lex ();
				return cn;
			}
		}
		else
			PR_ParseError ("weird immediate type");
	}

	// allocate a new one
	cn = (def_t *) SafeMalloc (sizeof(def_t));
	cn->next = NULL;

	pr.def_tail->next = cn;
	pr.def_tail = cn;

	cn->search_next = pr.search;
	pr.search = cn;

	cn->type = pr_immediate_type;
	cn->name = IMMEDIATE_NAME;
	cn->initialized = 1;
	cn->scope = NULL;		// always share immediates

	// copy the immediate to the global area
	cn->ofs = numpr_globals;
	pr_global_defs[cn->ofs] = cn;
	numpr_globals += type_size[pr_immediate_type->type];
	if (pr_immediate_type == &type_string)
		pr_immediate.string = CopyString (pr_immediate_string);

	memcpy (pr_globals + cn->ofs, &pr_immediate, 4*type_size[pr_immediate_type->type]);

	PR_Lex ();

	return cn;
}

static void PrecacheSound (def_t *e, int ch)
{
	char	*n;
	int		i;

	if (!e->ofs)
		return;
	n = G_STRING(e->ofs);
	for (i = 0; i < numsounds; i++)
	{
		if (!strcmp(n, precache_sounds[i]))
			return;
	}
	if (numsounds == MAX_SOUNDS)
		COM_Error ("%s: numsounds == MAX_SOUNDS", __thisfunc__);
	strcpy (precache_sounds[i], n);
	if (ch >= '1'  && ch <= '9')
		precache_sounds_block[i] = ch - '0';
	else
		precache_sounds_block[i] = 1;
	numsounds++;
}

static void PrecacheModel (def_t *e, int ch)
{
	char	*n;
	int		i;

	if (!e->ofs)
		return;
	n = G_STRING(e->ofs);
	for (i = 0; i < nummodels; i++)
		if (!strcmp(n, precache_models[i]))
			return;
	if (nummodels == MAX_MODELS)
		COM_Error ("%s: nummodels == MAX_MODELS", __thisfunc__);
	strcpy (precache_models[i], n);
	if (ch >= '1' && ch <= '9')
		precache_models_block[i] = ch - '0';
	else
		precache_models_block[i] = 1;
	nummodels++;
}

static void PrecacheFile (def_t *e, int ch)
{
	char	*n;
	int		i;

	if (!e->ofs)
		return;
	n = G_STRING(e->ofs);
	for (i = 0; i < numfiles; i++)
	{
		if (!strcmp(n, precache_files[i]))
			return;
	}
	if (numfiles == MAX_FILES)
		COM_Error ("%s: numfiles == MAX_FILES", __thisfunc__);
	strcpy (precache_files[i], n);
	if (ch >= '1' && ch <= '9')
		precache_files_block[i] = ch - '0';
	else
		precache_files_block[i] = 1;
	numfiles++;
}

/*
============
PR_ParseFunctionCall
============
*/
static def_t *PR_ParseFunctionCall (def_t *func)
{
	def_t		*e, *a1 = NULL, *a2 = NULL;
	int			arg;
	type_t		*t;
	dstatement_t	*statement = NULL;

	t = func->type;

	if (t->type != ev_function)
		PR_ParseError ("not a function");

	// copy the arguments to the global parameter variables
	arg = 0;
	if (!PR_Check(")"))
	{
		do
		{
			if (t->num_parms != -1 && arg >= t->num_parms)
				PR_ParseError ("too many parameters");
			e = PR_Expression (TOP_PRIORITY);
			if (arg == 0 && func->name)
			{
			// save information for model and sound caching
				if (!strncmp(func->name,"precache_sound", 14))
					PrecacheSound (e, func->name[14]);
				else if (!strncmp(func->name,"precache_model", 14))
					PrecacheModel (e, func->name[14]);
				else if (!strncmp(func->name,"precache_file", 13))
					PrecacheFile (e, func->name[13]);
			}

			if (t->num_parms != -1 && (e->type != t->parm_types[arg]))
				PR_ParseError ("type mismatch on parm %i", arg);
		// a vector copy will copy everything
			def_parms[arg].type = t->parm_types[arg];
			if (arg == 0)
			{
				a1 = e;
			}
			else if (arg == 1)
			{
				a2 = e;
			}
			else
			{
				if (e->type->type == ev_vector)
					PR_Statement (&pr_opcodes[OP_STORE_V], e, &def_parms[arg]);
				else
					PR_Statement (&pr_opcodes[OP_STORE_F], e, &def_parms[arg]);
			}
			arg++;
		} while (PR_Check (","));

		if (t->num_parms != -1 && arg != t->num_parms)
			PR_ParseError ("too few parameters");
		PR_Expect (")");
	}

	if (arg > 8)
		PR_ParseError ("More than eight parameters");

	statement = &statements[numstatements];
	numstatements++;

	statement_linenums[statement-statements] = pr_source_line;
	statement->op = OP_CALL0 + arg;
	statement->a = func ? func->ofs : 0;
	statement->b = a1 ? a1->ofs : 0;
	statement->c = a2 ? a2->ofs : 0;

//	PR_Statement (&pr_opcodes[OP_CALL0+arg], func, NULL);

	def_ret.type = t->aux_type;
	return &def_ret;
}

/*
=============
PR_ParseRandom
=============
*/
static def_t *PR_ParseRandom (void)
{
	def_t		*e = NULL, *e2 = NULL;

	PR_Expect("(");

	if (PR_Check(")"))
	{
		PR_Statement (&pr_opcodes[OP_RAND0], NULL, NULL);
		def_ret.type = def_float.type;
		return &def_ret;
	}
	else
	{
		e = PR_Expression (TOP_PRIORITY);
		if (PR_Check(","))
		{
			e2 = PR_Expression (TOP_PRIORITY);
			PR_Expect(")");
			if (e->type == e2->type)
			{
				if (e->type == def_float.type)
					PR_Statement (&pr_opcodes[OP_RAND2], e, e2);
				else if (e->type == def_vector.type)
					PR_Statement (&pr_opcodes[OP_RANDV2], e, e2);
				def_ret.type = e2->type;
				return &def_ret;
			}
		}
		else
		{
			PR_Expect(")");
			if (e->type == def_float.type)
				PR_Statement (&pr_opcodes[OP_RAND1], e, NULL);
			else if (e->type == def_vector.type)
				PR_Statement (&pr_opcodes[OP_RANDV1], e, NULL);
			def_ret.type = e->type;
			return &def_ret;
		}
	}

	PR_ParseError ("type mismatch on random");
	return NULL; /* silence compiler */
}

/*
============
PR_ParseValue

Returns the global ofs for the current token
============
*/
static def_t *PR_ParseValue (void)
{
	def_t		*d;
	const char	*name;

	// if the token is an immediate, allocate a constant for it
	if (pr_token_type == tt_immediate)
		return PR_ParseImmediate ();

	name = PR_ParseName ();

	// look through the defs
	d = PR_GetDef (NULL, name, pr_scope, false);
	if (!d)
		PR_ParseError ("Unknown value \"%s\"", name);
	return d;
}

/*
============
PR_Term
============
*/
static def_t *PR_Term (void)
{
	def_t	*e, *e2;
	etype_t	t;

	if (PR_Check ("!"))
	{
		e = PR_Expression (NOT_PRIORITY);
		t = e->type->type;
		if (t == ev_float)
			e2 = PR_Statement (&pr_opcodes[OP_NOT_F], e, NULL);
		else if (t == ev_string)
			e2 = PR_Statement (&pr_opcodes[OP_NOT_S], e, NULL);
		else if (t == ev_entity)
			e2 = PR_Statement (&pr_opcodes[OP_NOT_ENT], e, NULL);
		else if (t == ev_vector)
			e2 = PR_Statement (&pr_opcodes[OP_NOT_V], e, NULL);
		else if (t == ev_function)
			e2 = PR_Statement (&pr_opcodes[OP_NOT_FNC], e, NULL);
		else
		{
			e2 = NULL;	// shut up compiler warning;
			PR_ParseError ("type mismatch for !");
		}
		return e2;
	}

	if (PR_Check ("("))
	{
		e = PR_Expression (TOP_PRIORITY);
		PR_Expect (")");
		return e;
	}

	return PR_ParseValue ();
}

/*
==============
PR_Expression
==============
*/
static def_t *PR_Expression (int priority)
{
	opcode_t	*op, *oldop;
	def_t		*e, *e2;
	etype_t		type_a, type_b, type_c;

	if (priority == 0)
		return PR_Term ();
	if (priority == 1 && PR_Check ("random"))
		return PR_ParseRandom();

	e = PR_Expression (priority-1);

	while (1)
	{
		if (priority == 1 && PR_Check ("("))
			return PR_ParseFunctionCall (e);

		for (op = pr_opcodes; op->name; op++)
		{
			if (op->priority != priority)
				continue;
			if (!PR_Check (op->name))
				continue;
			if ( op->right_associative )
			{
			// if last statement is an indirect, change it to an address of
				if ((unsigned int)(statements[numstatements-1].op - OP_LOAD_F) < 6)
				{
				//	printf("op was %s %s %d \n", pr_opcodes[statements[numstatements-1].op].opname, pr_opcodes[statements[numstatements-1].op].name, statements[numstatements-1].op);
				//	getche();
					statements[numstatements-1].op = OP_ADDRESS;
					def_pointer.type->aux_type = e->type;
					e->type = def_pointer.type;
				}
				e2 = PR_Expression (priority);
			}
			else
			{
				if (((op - pr_opcodes) >= OP_RAND0) && ((op - pr_opcodes) <= OP_RAND2))
				{
					e2 = PR_Expression (priority);
				//	printf("finding e2 %s %dright assoc %d\n", e2 ? e2->name : "???", e2 ? e2->type->type : -1, priority);
					PR_Statement (op, e, e2);
				//	printf("tryying to write %d rand to ofs 1 e:%s e2: %s\n", priority, e ? e->name : "??", e2 ? e2->name : "??");
					def_ret.type = e->type;
					return &def_ret;
				}
				else if (((op - pr_opcodes) >= OP_RANDV0) && ((op - pr_opcodes) <= OP_RANDV2))
				{
					e2 = PR_Expression (priority);
				//	printf("finding e2 %s %dright assoc %d\n", e2 ? e2->name: "???", e2 ? e2->type->type : -1, priority);
					PR_Statement (op, e, e2);
				//	printf("tryying to write %d rand to ofs 1 e:%s e2: %s\n", priority, e ? e->name : "??", e2 ? e2->name: "??");
					def_ret.type = e->type;
					return &def_ret;
				}
				e2 = PR_Expression (priority-1);
			}

		// type check
			type_a = e->type->type;
			type_b = e2->type->type;

			if (op->name[0] == '.')// field access gets type from field
			{
				if (e2->type->aux_type)
					type_c = e2->type->aux_type->type;
				else
					type_c = ev_bad;	// not a field
			}
			else
				type_c = ev_void;

			oldop = op;
			while ( type_a != op->type_a->type->type ||
				type_b != op->type_b->type->type ||
				(type_c != ev_void && type_c != op->type_c->type->type))
			{
				op++;
				if (!op->name || strcmp (op->name , oldop->name))
				{
					op--;
					printf("op : %s(%ld) a: %s b: %s\n",
						op->name, (long)(op - pr_opcodes), e->name, e2->name);
					printf("a: %d %d b: %d %d c: %d %d\n",
						type_a, op->type_a->type->type,
						type_b, op->type_b->type->type,
						type_c, op->type_c->type->type);
					PR_ParseError ("type mismatch for %s here", oldop->name);
				}
			}

			if (type_a == ev_pointer && type_b != e->type->aux_type->type)
				PR_ParseError ("type mismatch for %s", op->name);

			if (op->right_associative)
				e = PR_Statement (op, e2, e);
			else
				e = PR_Statement (op, e, e2);

			if (type_c != ev_void)	// field access gets type from field
				e->type = e2->type->aux_type;

			break;
		}
		if (!op->name)
			break;	// next token isn't at this priority level
	}

	return e;
}

/*
============
PR_ParseStatement

============
*/
static void PR_ParseStatement (void)
{
	def_t			*e = NULL, *e2 = NULL;
	dstatement_t	*patch1, *patch2;

	if (PR_Check ("{"))
	{
		do
		{
			PR_ParseStatement ();
		} while (!PR_Check ("}"));

		return;
	}

	if (PR_Check("return"))
	{
		if (PR_Check (";"))
		{
			PR_Statement (&pr_opcodes[OP_RETURN], NULL, NULL);
			return;
		}
		e = PR_Expression (TOP_PRIORITY);
	//	printf("%s return type %d wants %d\n", pr_scope->name, e->type->type, pr_scope->type->aux_type->type);
		if (e->type->type != pr_scope->type->aux_type->type)
			PR_ParseError ("type mismatch on return for %s, expected %d found %d",
						pr_scope->name, pr_scope->type->aux_type->type, e->type->type);
		PR_Expect (";");
		PR_Statement (&pr_opcodes[OP_RETURN], e, NULL);
		return;
	}

	if (PR_Check("while"))
	{
		PR_Expect ("(");
		patch2 = &statements[numstatements];
		e = PR_Expression (TOP_PRIORITY);
		PR_Expect (")");
		patch1 = &statements[numstatements];
		PR_Statement (&pr_opcodes[OP_IFNOT], e, NULL);
		PR_ParseStatement ();
		junkdef.ofs = patch2 - &statements[numstatements];
		PR_Statement (&pr_opcodes[OP_GOTO], &junkdef, NULL);
		patch1->b = &statements[numstatements] - patch1;
		return;
	}

	if (PR_Check("do"))
	{
		patch1 = &statements[numstatements];
		PR_ParseStatement ();
		PR_Expect ("while");
		PR_Expect ("(");
		e = PR_Expression (TOP_PRIORITY);
		PR_Expect (")");
		PR_Expect (";");
		junkdef.ofs = patch1 - &statements[numstatements];
		PR_Statement (&pr_opcodes[OP_IF], e, &junkdef);
		return;
	}

	if (PR_Check("local"))
	{
		PR_ParseDefs ();
		locals_end = numpr_globals;
		return;
	}

	/*
	if (PR_Check("random"))
	{
		PR_Expect ("(");
		if (PR_Check(")"))
		{
			PR_Expect(";");
			PR_Statement (&pr_opcodes[OP_RAND0], NULL, NULL);
			return;
		}
		else
		{
			e = PR_Expression (TOP_PRIORITY);
			if (PR_Check(")"))
			{
				PR_Expect(";");
				PR_Statement (&pr_opcodes[OP_RAND1], e, NULL);
				return;
			}
			else
			{
				PR_Expect(",");
				e2 = PR_Expression (TOP_PRIORITY);
				PR_Expect(")");
				PR_Expect(";");
				PR_Statement (&pr_opcodes[OP_RAND2], e, e2);
				return;
			}
		}

		return;
	}
	*/

	if (PR_Check("AdvanceThinkTime"))
	{
		PR_Expect ("(");
		e = PR_Expression (TOP_PRIORITY);
		if (e->type->type != ev_entity)
			PR_ParseError ("type mismatch for %s", e->name);
		PR_Expect (",");
		e2 = PR_Expression (TOP_PRIORITY);
		if (e2->type->type != ev_float)
			PR_ParseError ("type mismatch for %s", e2->name);
		PR_Expect (")");
		PR_Expect (";");
		PR_Statement (&pr_opcodes[OP_THINKTIME], e, e2);
		return;
	}

	if (PR_Check("AdvanceFrame"))
	{
		PR_Expect ("(");
		e = PR_Expression (TOP_PRIORITY);
		if (e->type->type != ev_float)
			PR_ParseError ("type mismatch for %s", e->name);
		PR_Expect (",");
		e2 = PR_Expression (TOP_PRIORITY);
		if (e2->type->type != ev_float)
			PR_ParseError ("type mismatch for %s", e2->name);
		PR_Expect (")");
		PR_Expect (";");
		PR_Statement (&pr_opcodes[OP_CSTATE], e, e2);
		return;
	}

	if (PR_Check("if"))
	{
		PR_Expect ("(");
		e = PR_Expression (TOP_PRIORITY);
		PR_Expect (")");

		patch1 = &statements[numstatements];
		PR_Statement (&pr_opcodes[OP_IFNOT], e, NULL);

		PR_ParseStatement ();

		if (PR_Check ("else"))
		{
			patch2 = &statements[numstatements];
			PR_Statement (&pr_opcodes[OP_GOTO], NULL, NULL);
			patch1->b = &statements[numstatements] - patch1;
			PR_ParseStatement ();
			patch2->a = &statements[numstatements] - patch2;
		}
		else
			patch1->b = &statements[numstatements] - patch1;

		return;
	}

	PR_Expression (TOP_PRIORITY);
	PR_Expect (";");
}


/*
==============
PR_ParseState

States are special functions made for convenience.  They automatically
set frame, nextthink (implicitly), and think (allowing forward definitions).

// void() name = [framenum, nextthink] {code}
// expands to:
// function void name ()
// {
//		self.frame=framenum;
//		self.nextthink = time + 0.1;
//		self.think = nextthink
//		<code>
// };
==============
*/
static void PR_ParseState (void)
{
	const char	*name;
	def_t	*s1, *def;

	if (pr_token_type != tt_immediate || pr_immediate_type != &type_float)
		PR_ParseError ("state frame must be a number");
	s1 = PR_ParseImmediate ();

	PR_Expect (",");

	name = PR_ParseName ();
	def = PR_GetDef (&type_function, name, NULL, true);

	PR_Expect ("]");

	PR_Statement (&pr_opcodes[OP_STATE], s1, def);
}

/*
============
PR_ParseImmediateStatements

Parse a function body
============
*/
static function_t *PR_ParseImmediateStatements (type_t *type)
{
	int			i;
	function_t	*f;
	def_t		*defs[MAX_PARMS];

	f = (function_t *) SafeMalloc (sizeof(function_t));

//
// check for builtin function definition #1, #2, etc
//
	if (PR_Check ("#"))
	{
		if (pr_token_type != tt_immediate
				|| pr_immediate_type != &type_float
				|| pr_immediate._float != (int)pr_immediate._float)
			PR_ParseError ("Bad builtin immediate");
		f->builtin = (int)pr_immediate._float;
		PR_Lex ();
		return f;
	}

	f->builtin = 0;
//
// define the parms
//
	for (i = 0; i < type->num_parms; i++)
	{
		defs[i] = PR_GetDef (type->parm_types[i], pr_parm_names[i], pr_scope, true);
		f->parm_ofs[i] = defs[i]->ofs;
		if (i > 0 && f->parm_ofs[i] < f->parm_ofs[i-1])
			COM_Error ("bad parm order");
	}

	f->code = numstatements;

//
// check for a state opcode
//
	if (PR_Check ("["))
		PR_ParseState ();

//
// parse regular statements
//
	PR_Expect ("{");

	while (!PR_Check("}"))
		PR_ParseStatement ();

// emit an end of statements opcode
	PR_Statement (pr_opcodes, NULL, NULL);

	return f;
}

/*
============
PR_GetDef

If type is NULL, it will match any type
If allocate is true, a new def will be allocated if it can't be found
============
*/
def_t *PR_GetDef (type_t *type, const char *name, def_t *scope, qboolean allocate)
{
	def_t		*def, **old;
	char	element[MAX_NAME];

	// see if the name is already in use
	old = &pr.search;
	for (def = *old; def; old = &def->search_next, def = *old)
	{
		if (!strcmp(def->name,name))
		{
			if (def->scope && def->scope != scope)
				continue;	// in a different function

			if (type && def->type != type)
				PR_ParseError ("Type mismatch on redeclaration of %s", name);

			// move to head of list to find fast next time
			*old = def->search_next;
			def->search_next = pr.search;
			pr.search = def;
			return def;
		}
	}

	if (!allocate)
		return NULL;

	// allocate a new def
	def = (def_t *) SafeMalloc (sizeof(def_t));
	def->next = NULL;
	pr.def_tail->next = def;
	pr.def_tail = def;

	def->search_next = pr.search;
	pr.search = def;

	def->name = SafeStrdup(name);
	def->type = type;
	def->scope = scope;

	def->ofs = numpr_globals;
	pr_global_defs[numpr_globals] = def;

//
// make automatic defs for the vectors elements
// .origin can be accessed as .origin_x, .origin_y, and .origin_z
//
	if (type->type == ev_vector)
	{
		sprintf (element, "%s_x", name);
		PR_GetDef (&type_float, element, scope, true);

		sprintf (element, "%s_y", name);
		PR_GetDef (&type_float, element, scope, true);

		sprintf (element, "%s_z", name);
		PR_GetDef (&type_float, element, scope, true);
	}
	else
		numpr_globals += type_size[type->type];

	if (type->type == ev_field)
	{
		*(int *)&pr_globals[def->ofs] = pr.size_fields;

		if (type->aux_type->type == ev_vector)
		{
			sprintf (element, "%s_x", name);
			PR_GetDef (&type_floatfield, element, scope, true);

			sprintf (element, "%s_y", name);
			PR_GetDef (&type_floatfield, element, scope, true);

			sprintf (element, "%s_z", name);
			PR_GetDef (&type_floatfield, element, scope, true);
		}
		else
			pr.size_fields += type_size[type->aux_type->type];
	}

	return def;
}

static void PR_InitArray (def_t *scope,int size)
{
	int		i;

	PR_Expect("{");
	for (i = 0; i < size; i++)
	{
		PR_Lex();
		if (pr_immediate_type->type != scope->type->type)
			PR_ParseError ("Type mismatch on redeclaration of %s", scope->name);
		memcpy (pr_globals + scope->ofs + (i*type_size[pr_immediate_type->type]), &pr_immediate, 4*type_size[pr_immediate_type->type]);
		/*
		if (pr_immediate_type->type == ev_float)
			printf("%s[%d] = %f\n", scope->name, i, pr_immediate._float);
		if (pr_immediate_type->type == ev_vector)
			printf("%s[%d] = '%f %f %f'\n", scope->name, i, pr_immediate.vector[0], pr_immediate.vector[1], pr_immediate.vector[2]);
		*/
		if (!PR_Check(","))
			break;
		if (PR_Check("}"))
			return;
	}
	PR_Expect("}");
	printf("successfully read in %d array values\n", i+1);
}

static def_t *PR_AllocateArray (type_t *type, const char *name, def_t *scope, qboolean allocate, int size)
{
	def_t		*def, **old;
	char	element[MAX_NAME];

	// see if the name is already in use
	old = &pr.search;
	for (def = *old; def; old = &def->search_next, def = *old)
	{
		if (!strcmp(def->name, name))
		{
			if (def->scope && def->scope != scope)
				continue;	// in a different function

			if (type && def->type != type)
				PR_ParseError ("Type mismatch on redeclaration of %s", name);

			// move to head of list to find fast next time
			*old = def->search_next;
			def->search_next = pr.search;
			pr.search = def;
			return def;
		}
	}

	if (!allocate)
		return NULL;

	// allocate a new def
	def = (def_t *) SafeMalloc (sizeof(def_t));
	def->next = NULL;
	pr.def_tail->next = def;
	pr.def_tail = def;

	def->search_next = pr.search;
	pr.search = def;

	def->name = SafeStrdup(name);
	def->type = type;
	def->scope = scope;
	if (size > 1)
	{
		*(int *)&pr_globals[numpr_globals++] = size - 1;
	}
	def->ofs = numpr_globals;
	pr_global_defs[numpr_globals] = def;

//
// make automatic defs for the vectors elements
// .origin can be accessed as .origin_x, .origin_y, and .origin_z
//
	if (type->type == ev_vector && size < 2)
	{
		sprintf (element, "%s_x", name);
		PR_GetDef (&type_float, element, scope, true);

		sprintf (element, "%s_y", name);
		PR_GetDef (&type_float, element, scope, true);

		sprintf (element, "%s_z", name);
		PR_GetDef (&type_float, element, scope, true);
	}
	else
		numpr_globals += size*type_size[type->type];

	if (type->type == ev_field)
	{
		*(int *)&pr_globals[def->ofs] = pr.size_fields;

		if (type->aux_type->type == ev_vector && size < 2)
		{
			sprintf (element, "%s_x", name);
			PR_GetDef (&type_floatfield, element, scope, true);

			sprintf (element, "%s_y", name);
			PR_GetDef (&type_floatfield, element, scope, true);

			sprintf (element, "%s_z", name);
			PR_GetDef (&type_floatfield, element, scope, true);
		}
		else
			pr.size_fields += size*type_size[type->aux_type->type];
	}

	return def;
}

/*
================
PR_ParseDefs

Called at the outer layer and when a local statement is hit
================
*/
static void PR_ParseDefs (void)
{
	char		a1[500];
	const char	*name, *name2;
	type_t		*type;
	def_t		*def,*def2;
	function_t	*f;
	dfunction_t	*df;
	int			i;
	int			locals_start;

	type = PR_ParseType ();

	if (pr_scope && (type->type == ev_field || type->type == ev_function))
		PR_ParseError ("Fields and functions must be global");

	do
	{
		name = PR_ParseName ();
		if (PR_Check ("alias"))
		{
			def = PR_GetDef (type, name, pr_scope, true);
			numpr_globals--;
			a1[0] = 0;
			if (name)
				strcpy(a1,name);
			name2 = PR_ParseName ();
			def2 = PR_GetDef (type, name2, pr_scope, false);
			if (!def2)
				PR_ParseError ("can not alias %s to %s\n%s is undeclared\n", name, name2, name2);
		//	printf("%s %d %s %d\n", a1, def->ofs, name2, def2->ofs);
		//	getche();
			def->ofs = def2->ofs;
		//	printf("%s %s %d %s\n", a1, def2 ? def2->name : "nope", type->type, def ? def->name : "nope");
		//	getche();
			break;
		}

		if (PR_Check("["))
		{
				PR_Lex();
				def = PR_AllocateArray (type, name, pr_scope, true, (int)pr_immediate._float);
				def->initialized = 1;
			//	printf("\nfound brace for array init of %s as type %d making %d spaces\n",
			//		def->name, pr_immediate_type->type, (int)pr_immediate._float);
			//	getche();
				PR_Expect("]");
				if (PR_Check("="))
				{
					printf("initializing array\n");
					PR_InitArray (def, (int)pr_immediate._float);
				}
				break;
		}
		else
			def = PR_GetDef (type, name, pr_scope, true);

		// check for an initialization
		if (PR_Check ("="))
		{
			if (def->initialized)
				PR_ParseError ("%s redeclared", name);

			if (type->type == ev_function)
			{
				locals_start = locals_end = numpr_globals;
				pr_scope = def;
				f = PR_ParseImmediateStatements (type);
				pr_scope = NULL;
				def->initialized = 1;
				G_FUNCTION(def->ofs) = numfunctions;
				f->def = def;

			// fill in the dfunction
				df = &functions[numfunctions];
				numfunctions++;
				if (f->builtin)
					df->first_statement = -f->builtin;
				else
					df->first_statement = f->code;
				df->s_name = CopyString (f->def->name);
				df->s_file = s_file;
				df->numparms =  f->def->type->num_parms;
				df->locals = locals_end - locals_start;
				df->parm_start = locals_start;
				for (i = 0; i < df->numparms; i++)
					df->parm_size[i] = type_size[f->def->type->parm_types[i]->type];

				continue;
			}
			else if (pr_immediate_type != type)
				PR_ParseError ("wrong immediate type for %s", name);

			def->initialized = 1;
		//	if (pr_immediate_type->type == ev_float)
		//		printf("init %s to %f

			PR_Lex ();
			memcpy (pr_globals + def->ofs, &pr_immediate, 4*type_size[pr_immediate_type->type]);
		//	printf("writing %s to %d type %d %d\n", def->name, def->ofs, pr_immediate_type->type, def->type->type);
		//	if (pr_immediate_type->type == ev_float)
		//		printf("%s = %f\n", def->name, pr_immediate._float);
		}

	} while (PR_Check (","));

	PR_Expect (";");
}

/*
============
PR_CompileFile

compiles the 0 terminated text, adding defintions to the pr structure
============
*/
qboolean PR_CompileFile (const char *string, const char *filename)
{
	PR_ClearGrabMacros ();	// clear the frame macros

	pr_file_p = string;
	s_file = CopyString (filename);

	pr_source_line = 0;

	PR_NewLine ();

	PR_Lex ();	// read first token

	while (pr_token_type != tt_eof)
	{
		if (setjmp(pr_parse_abort))
		{
			if (++pr_error_count > MAX_ERRORS)
				return false;
			PR_SkipToSemicolon ();
			if (pr_token_type == tt_eof)
				return false;
		}

		pr_scope = NULL;	// outside all functions

		PR_ParseDefs ();
	}

	return (pr_error_count == 0);
}

