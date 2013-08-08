/*
 * expr.c
 * $Id: expr.c,v 1.10 2007-12-14 16:41:17 sezero Exp $
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


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "hcc.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static def_t *Term(void);
static def_t *ParseFunctionCall(def_t *func);
static def_t *ParseIntrinsicFunc(const char *name);
static void PrecacheSound(def_t *e, int ch);
static void PrecacheModel(def_t *e, int ch);
static void PrecacheFile(def_t *e, int ch);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int ex_FunctionCallCount;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int TokenToOp[] =
{
	OP_DONE,			// TK_NONE
	OP_DONE,			// TK_SEMICOLON
	OP_DONE,			// TK_LPAREN
	OP_DONE,			// TK_RPAREN
	OP_DONE,			// TK_COMMA
	OP_ADD_F,			// TK_PLUS
	OP_MUL_F,			// TK_ASTERISK
	OP_DIV_F,			// TK_SLASH
	OP_DONE,			// TK_LBRACE
	OP_DONE,			// TK_RBRACE
	OP_DONE,			// TK_LBRACKET
	OP_DONE,			// TK_RBRACKET
	OP_DONE,			// TK_NUMBERSIGN
	OP_EQ_F,			// TK_EQ
	OP_NE_F,			// TK_NE
	OP_LT,				// TK_LT
	OP_GT,				// TK_GT
	OP_LE,				// TK_LE
	OP_GE,				// TK_GE
	OP_STORE_F,			// TK_ASSIGN
	OP_AND,				// TK_AND
	OP_BITAND,			// TK_BITAND
	OP_OR,				// TK_OR
	OP_BITOR,			// TK_BITOR
	OP_DONE,			// TK_NOT
	OP_LOAD_F,			// TK_PERIOD
	OP_DONE,			// TK_ELLIPSIS
	OP_SUB_F,			// TK_MINUS
	OP_DONE,			// TK_INC
	OP_DONE,			// TK_DEC
	OP_MULSTORE_F,			// TK_MULASSIGN
	OP_DIVSTORE_F,			// TK_DIVASSIGN
	OP_ADDSTORE_F,			// TK_ADDASSIGN
	OP_SUBSTORE_F,			// TK_SUBASSIGN
	OP_DONE,			// TK_COLON
	OP_DONE,			// TK_RANGE
	OP_BITSET,			// TK_BITSET
	OP_BITCLR			// TK_BITCLR
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// EX_Init
//
//==========================================================================

void EX_Init (void)
{
	ex_FunctionCallCount = 0;
}

//==========================================================================
//
// EX_Expression
//
//==========================================================================

def_t *EX_Expression (int priority)
{
	opcode_t	*op;
	def_t	*e;
	def_t	*e2;
	etype_t	type_a;
	etype_t	type_b;
	etype_t	type_c;
	int	tag;
	int	opIndex;

	if (priority == 0)
	{
		return Term();
	}

	e = EX_Expression(priority-1);

	while (1)
	{
		if (priority == 1 && TK_CHECK(TK_LPAREN))
		{
			return ParseFunctionCall(e);
		}

		opIndex = TokenToOp[pr_tokenclass];
		if (opIndex == OP_DONE)
		{
			return e;
		}

		for (op = &pr_opcodes[opIndex]; op->name; op++)
		{
			if (op->priority != priority)
			{
				return e;
			}
			if (!LX_CheckFetch(op->name))
			{
				return e;
			}
			if (op->right_associative)
			{
				if ((unsigned int)(statements[numstatements-1].op - OP_LOAD_F) < 6)
				{
					// The preceding statement was an indirect.  Change it to
					// an address of.
					statements[numstatements-1].op = OP_ADDRESS;
					def_pointer.type->aux_type = e->type;
					e->type = def_pointer.type;
				}
				/*
				else if ((unsigned int)(statements[numstatements-1].op - OP_FETCH_GBL_F) < 5)
				{
					// The preceding statement was an array lookup.  Assignment
					// is currently not allowed to arrays.
					PR_ParseError("assignment not allowed to arrays");
				}
				*/
				e2 = EX_Expression(priority);
			}
			else
			{
				e2 = EX_Expression(priority-1);
			}

			// Set types a, b, and c
			type_a = e->type->type;
			type_b = e2->type->type;
			if (op->name[0] == '.')
			{ // Field access gets type from field
				if (e2->type->aux_type)
				{
					type_c = e2->type->aux_type->type;
				}
				else
				{ // Not a field
					type_c = ev_bad;
				}
			}
			else
			{
				type_c = ev_void;
			}

			// Find the opcode that matches the types
			tag = op->tag;
			while ( type_a != op->type_a->type->type ||
				type_b != op->type_b->type->type ||
				(type_c != ev_void && type_c != op->type_c->type->type))
			{
				op++;
				if (tag != op->tag)
				{
					op--;
					PR_ParseError("type mismatch for %s", op->name);
				}
			}
			if (type_a == ev_pointer && type_b != e->type->aux_type->type)
			{
				PR_ParseError("type mismatch for %s", op->name);
			}

			// Emit the statement
			if (op->right_associative)
			{
				e = CO_GenCode(op, e2, e);
			}
			else
			{
				e = CO_GenCode(op, e, e2);
			}

			if (type_c != ev_void)
			{ // Field access gets type from field
				e->type = e2->type->aux_type;
			}

			break;
		}

		if (!op->name)
		{ // Next token isn't at this priority level
			break;
		}
	}

	return e;
}

//==========================================================================
//
// Term
//
//==========================================================================

static def_t *Term (void)
{
	def_t	*d, *e, *e2;
	etype_t		t;
	const char	*name;

	if (TK_CHECK(TK_NOT))
	{
		e = EX_Expression(NOT_PRIORITY);
		t = e->type->type;
		if (t == ev_float)
			e2 = CO_GenCode(&pr_opcodes[OP_NOT_F], e, NULL);
		else if (t == ev_string)
			e2 = CO_GenCode(&pr_opcodes[OP_NOT_S], e, NULL);
		else if (t == ev_entity)
			e2 = CO_GenCode(&pr_opcodes[OP_NOT_ENT], e, NULL);
		else if (t == ev_vector)
			e2 = CO_GenCode(&pr_opcodes[OP_NOT_V], e, NULL);
		else if (t == ev_function)
			e2 = CO_GenCode(&pr_opcodes[OP_NOT_FNC], e, NULL);
		else
		{
			e2 = NULL; // Shut up compiler warning
			PR_ParseError("type mismatch for !");
		}
		return e2;
	}

	if (TK_CHECK(TK_LPAREN))
	{
		e = EX_Expression(TOP_PRIORITY);
		LX_Require(")");
		return e;
	}

	if (pr_token_type == tt_immediate)
	{
		d = CO_ParseImmediate();
		LX_Fetch();
		return d;
	}

	name = PR_ParseName();
	if ((d = ParseIntrinsicFunc(name)) != NULL)
	{ // Found and parsed an intrinsic function
		d->referenceCount++;
		if (d->parentVector != NULL)
			d->parentVector->referenceCount++;
		return d;
	}

	d = PR_GetDef(NULL, name, pr_scope, false);
	if (!d)
	{
		PR_ParseError("unknown value \"%s\"", name);
	}

	d->referenceCount++;
	if (d->parentVector != NULL)
		d->parentVector->referenceCount++;

	if (TK_CHECK(TK_LBRACKET))
	{
		e = EX_Expression(TOP_PRIORITY);
		LX_Require("]");

		if (TK_TEST(TK_ASSIGN))
		{
			LX_Fetch();
			e2 = EX_Expression(TOP_PRIORITY);
			//PR_ParseError("assignment is not allowed to arrays");

			if (d->type->type != e2->type->type)
			{
				PR_ParseError("type mismatch for =");
			}
			return NULL;
		}

		switch (d->type->type)
		{
		case ev_float:
			e2 = CO_GenCode(&pr_opcodes[OP_FETCH_GBL_F], d, e);
			break;
		case ev_vector:
			e2 = CO_GenCode(&pr_opcodes[OP_FETCH_GBL_V], d, e);
			break;
		case ev_string:
			e2 = CO_GenCode(&pr_opcodes[OP_FETCH_GBL_S], d, e);
			break;
		case ev_entity:
			e2 = CO_GenCode(&pr_opcodes[OP_FETCH_GBL_E], d, e);
			break;
		case ev_function:
			e2 = CO_GenCode(&pr_opcodes[OP_FETCH_GBL_FNC], d, e);
			break;
		default:
			e2 = NULL; // Shut up compiler warning
			PR_ParseError("type mismatch for []");
			break;
		}
		return e2;
	}
	return d;
}

//==========================================================================
//
// ParseFunctionCall
//
//==========================================================================

static def_t *ParseFunctionCall (def_t *func)
{
	def_t	*e;
	def_t	*args[2];
	int	argCount;
	type_t	*t;

	t = func->type;
	if (t->type != ev_function)
	{
		PR_ParseError("not a function");
	}

	func->referenceCount++;
	if (func->parentVector != NULL)
	{
		func->parentVector->referenceCount++;
	}

	argCount = 0;
	args[0] = NULL;
	args[1] = NULL;
	if (!TK_CHECK(TK_RPAREN))
	{
		do
		{
			if (argCount == 8)
			{
				PR_ParseError("more than eight parameters");
			}
			if (t->num_parms != -1 && argCount >= t->num_parms)
			{
				PR_ParseError("too many parameters");
			}
			e = EX_Expression(TOP_PRIORITY);

			if (argCount == 0 && func->name)
			{ // Check for sound / model / file caching
				if (!strncmp(func->name, "precache_sound", 14))
				{
					PrecacheSound(e, func->name[14]);
				}
				else if (!strncmp(func->name, "precache_model", 14))
				{
					PrecacheModel(e, func->name[14]);
				}
				else if (!strncmp(func->name, "precache_file", 13))
				{
					PrecacheFile(e, func->name[13]);
				}
			}

			if (t->num_parms != -1 && (e->type != t->parm_types[argCount]))
			{
				PR_ParseError("type mismatch on parm %i", argCount);
			}

			def_parms[argCount].type = t->parm_types[argCount];
			if (argCount < 2)
			{
				args[argCount] = e;
			}
			else
			{
				if (t->parm_types[argCount] == NULL ||		// Variable args
				    t->parm_types[argCount]->type == ev_vector)
				{
					CO_GenCode(&pr_opcodes[OP_STORE_V], e,
							&def_parms[argCount]);
				}
				else
				{
					CO_GenCode(&pr_opcodes[OP_STORE_F], e,
							&def_parms[argCount]);
				}
			}
			argCount++;
		} while (TK_CHECK(TK_COMMA));
		LX_Require(")");
	}

	if (t->num_parms != -1 && argCount != t->num_parms)
	{
		PR_ParseError("too few parameters");
	}

	CO_GenCodeDirect(&pr_opcodes[OP_CALL0+argCount], func, args[0], args[1]);

	ex_FunctionCallCount++;

	def_ret.type = t->aux_type;
	return &def_ret;
}

//==========================================================================
//
// PrecacheSound
//
//==========================================================================

static void PrecacheSound (def_t *e, int ch)
{
	int		i;
	char	*n;

	if (!e->ofs || !e->initialized)
	{
		return;
	}
	n = G_STRING(e->ofs);
	for (i = 0; i < numsounds; i++)
	{
		if (!strcmp(n, precache_sounds[i]))
			return;
	}
	if (numsounds == MAX_SOUNDS)
	{
		COM_Error("%s: numsounds == MAX_SOUNDS", __thisfunc__);
	}
	strcpy(precache_sounds[i], n);
	if (ch >= '1' && ch <= '9')
	{
		precache_sounds_block[i] = ch - '0';
	}
	else
	{
		precache_sounds_block[i] = 1;
	}
	numsounds++;
}

//==========================================================================
//
// PrecacheModel
//
//==========================================================================

static void PrecacheModel (def_t *e, int ch)
{
	int		i;
	char	*n;

	if (!e->ofs || !e->initialized)
	{
		return;
	}
	n = G_STRING(e->ofs);
	for (i = 0; i < nummodels; i++)
	{
		if (!strcmp(n, precache_models[i]))
			return;
	}
	if (nummodels == MAX_MODELS)
	{
		COM_Error("%s: nummodels == MAX_MODELS", __thisfunc__);
	}
	strcpy(precache_models[i], n);
	if (ch >= '1' && ch <= '9')
	{
		precache_models_block[i] = ch - '0';
	}
	else
	{
		precache_models_block[i] = 1;
	}
	nummodels++;
}

//==========================================================================
//
// PrecacheFile
//
//==========================================================================
static void PrecacheFileName (const char *n, int ch)
{
	int		i;

	for (i = 0; i < numfiles; i++)
	{
		if (!strcmp(n, precache_files[i]))
			return;
	}
	if (numfiles == MAX_FILES)
	{
		COM_Error("%s: numfiles == MAX_FILES", __thisfunc__);
	}
	strcpy(precache_files[i], n);
	if (ch >= '1' && ch <= '9')
	{
		precache_files_block[i] = ch - '0';
	}
	else
	{
		precache_files_block[i] = 1;
	}
	numfiles++;
}

static void PrecacheFile (def_t *e, int ch)
{
	int		i;
	char	*n;

	if (!e->ofs || !e->initialized)
	{
		return;
	}
	n = G_STRING(e->ofs);
	for (i = 0; i < numfiles; i++)
	{
		if (!strcmp(n, precache_files[i]))
			return;
	}
	if (numfiles == MAX_FILES)
	{
		COM_Error("%s: numfiles == MAX_FILES", __thisfunc__);
	}
	strcpy(precache_files[i], n);
	if (ch >= '1' && ch <= '9')
	{
		precache_files_block[i] = ch - '0';
	}
	else
	{
		precache_files_block[i] = 1;
	}
	numfiles++;
}

//==========================================================================
//
// ParseIntrinsicFunc
//
//==========================================================================

static def_t *ParseIntrinsicFunc (const char *name)
{
	def_t	*expr1, *expr2;

	if (strcmp(name, "random") == 0)
	{
		LX_Require("(");
		if (TK_CHECK(TK_RPAREN))
		{
			CO_GenCode(&pr_opcodes[OP_RAND0], NULL, NULL);
		}
		else
		{
			expr1 = EX_Expression(TOP_PRIORITY);
			if (expr1->type->type != ev_float)
			{
				PR_ParseError("'random' : incompatible parameter type");
			}
			if (TK_CHECK(TK_COMMA))
			{
				expr2 = EX_Expression(TOP_PRIORITY);
				if (expr2->type->type != ev_float)
				{
					PR_ParseError("'random' : incompatible parameter type");
				}
				LX_Require(")");
				CO_GenCode(&pr_opcodes[OP_RAND2], expr1, expr2);
			}
			else
			{
				LX_Require(")");
				CO_GenCode(&pr_opcodes[OP_RAND1], expr1, NULL);
			}
		}
		def_ret.type = &type_float;
		return &def_ret;
	}

	if (strcmp(name, "randomv") == 0)
	{
		LX_Require("(");
		if (TK_CHECK(TK_RPAREN))
		{
			CO_GenCode(&pr_opcodes[OP_RANDV0], NULL, NULL);
		}
		else
		{
			expr1 = EX_Expression(TOP_PRIORITY);
			if (expr1->type->type != ev_vector)
			{
				PR_ParseError("'randomv' : incompatible parameter type");
			}
			if (TK_CHECK(TK_COMMA))
			{
				expr2 = EX_Expression(TOP_PRIORITY);
				if (expr2->type->type != ev_vector)
				{
					PR_ParseError("'randomv' : incompatible parameter type");
				}
				LX_Require(")");
				CO_GenCode(&pr_opcodes[OP_RANDV2], expr1, expr2);
			}
			else
			{
				LX_Require(")");
				CO_GenCode(&pr_opcodes[OP_RANDV1], expr1, NULL);
			}
		}
		def_ret.type = &type_vector;
		return &def_ret;
	}

	if (!strncmp(name, "precache_file", 13) && !hcc_Compat_precache_file)	//keep it from going into progs.dat
	{
		def_ret.type = &type_void;
		LX_Require("(");
		if (TK_CHECK(TK_RPAREN))
			return &def_ret;	//it's empty for some reason.
		//i should be getting an immediate string here
		if (pr_token_type != tt_immediate)	//oops
			return NULL;
		/*
		expr1 = CO_ParseImmediate();
		if (expr1->type->type != ev_string)
			PR_ParseError("'precache_file' : parm not a string");
		*/
		if (pr_immediate_type != &type_string)
			PR_ParseError("'precache_file' : parm not a string");
		PrecacheFileName(pr_immediate_string, name[13]);
		LX_Fetch();
		LX_Require(")");
		return &def_ret;
	}

	return NULL;
}

