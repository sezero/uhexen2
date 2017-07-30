/*
 * pr_exec.c -- PROGS execution
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


// HEADER FILES ------------------------------------------------------------

#include "quakedef.h"
#include "q_ctype.h"

// MACROS ------------------------------------------------------------------

#define MAX_STACK_DEPTH	64	/* was 32 */
#define LOCALSTACK_SIZE	2048

// TYPES -------------------------------------------------------------------

typedef struct
{
	int		s;
	dfunction_t	*f;
} prstack_t;

/* switch types */
enum {
	SWITCH_F,
	SWITCH_V,
	SWITCH_S,
	SWITCH_E,
	SWITCH_FNC
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

const char *PR_GlobalString(int ofs);
const char *PR_GlobalStringNoContents(int ofs);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int EnterFunction(dfunction_t *f);
static int LeaveFunction(void);
static void PrintStatement(dstatement_t *s);
static void PrintCallHistory(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

qboolean	pr_trace;
dfunction_t	*pr_xfunction;
int		pr_xstatement;
int		pr_argc;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static prstack_t pr_stack[MAX_STACK_DEPTH];
static int pr_depth;
static int localstack[LOCALSTACK_SIZE];
static int localstack_used;

static const char *pr_opnames[] =
{
	"DONE",
	"MUL_F", "MUL_V",  "MUL_FV", "MUL_VF",
	"DIV",
	"ADD_F", "ADD_V",
	"SUB_F", "SUB_V",
	"EQ_F", "EQ_V", "EQ_S", "EQ_E", "EQ_FNC",
	"NE_F", "NE_V", "NE_S", "NE_E", "NE_FNC",
	"LE", "GE", "LT", "GT",
	"INDIRECT", "INDIRECT", "INDIRECT",
	"INDIRECT", "INDIRECT", "INDIRECT",
	"ADDRESS",
	"STORE_F", "STORE_V", "STORE_S",
	"STORE_ENT", "STORE_FLD", "STORE_FNC",
	"STOREP_F", "STOREP_V", "STOREP_S",
	"STOREP_ENT", "STOREP_FLD", "STOREP_FNC",
	"RETURN",
	"NOT_F", "NOT_V", "NOT_S", "NOT_ENT", "NOT_FNC",
	"IF", "IFNOT",
	"CALL0", "CALL1", "CALL2", "CALL3", "CALL4",
	"CALL5", "CALL6", "CALL7", "CALL8",
	"STATE",
	"GOTO",
	"AND", "OR",
	"BITAND", "BITOR",
	"OP_MULSTORE_F", "OP_MULSTORE_V", "OP_MULSTOREP_F", "OP_MULSTOREP_V",
	"OP_DIVSTORE_F", "OP_DIVSTOREP_F",
	"OP_ADDSTORE_F", "OP_ADDSTORE_V", "OP_ADDSTOREP_F", "OP_ADDSTOREP_V",
	"OP_SUBSTORE_F", "OP_SUBSTORE_V", "OP_SUBSTOREP_F", "OP_SUBSTOREP_V",
	"OP_FETCH_GBL_F",
	"OP_FETCH_GBL_V",
	"OP_FETCH_GBL_S",
	"OP_FETCH_GBL_E",
	"OP_FETCH_GBL_FNC",
	"OP_CSTATE", "OP_CWSTATE",

	"OP_THINKTIME",

	"OP_BITSET", "OP_BITSETP", "OP_BITCLR",	"OP_BITCLRP",

	"OP_RAND0", "OP_RAND1",	"OP_RAND2",	"OP_RANDV0", "OP_RANDV1", "OP_RANDV2",

	"OP_SWITCH_F", "OP_SWITCH_V", "OP_SWITCH_S", "OP_SWITCH_E", "OP_SWITCH_FNC",

	"OP_CASE",
	"OP_CASERANGE"

};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// PR_ExecuteProgram
//
//==========================================================================

#if 0
#define OPA ((eval_t *)&pr_globals[(unsigned short)st->a])
#define OPB ((eval_t *)&pr_globals[(unsigned short)st->b])
#define OPC ((eval_t *)&pr_globals[(unsigned short)st->c])
#endif
/* unsigned short casts were needed with progs version 6.
 * we are now processing progs internally as version 7. */
#define OPA ((eval_t *)&pr_globals[st->a])
#define OPB ((eval_t *)&pr_globals[st->b])
#define OPC ((eval_t *)&pr_globals[st->c])

void PR_ExecuteProgram (func_t fnum)
{
	eval_t		*ptr, *opa, *opb, *opc;
	dstatement_t	*st;
	dfunction_t	*f, *newf;
	int profile, startprofile;
	edict_t		*ed;
	int		exitdepth;
	int		jump_ofs;
	/* switch/case support:  */
	int		case_type = -1;
	float	switch_float = 0; /* avoid 'maybe used unititialized' */
	float	*vecptr;

	if (!fnum || fnum >= progs->numfunctions)
	{
		if (*sv_globals.self)
		{
			ED_Print(PROG_TO_EDICT(*sv_globals.self));
		}
		Host_Error("%s: NULL function", __thisfunc__);
	}

	f = &pr_functions[fnum];

	pr_trace = false;

	exitdepth = pr_depth;

	st = &pr_statements[EnterFunction(f)];
	startprofile = profile = 0;

    while (1)
    {
	st++;	/* next statement */
	opa = OPA;
	opb = OPB;
	opc = OPC;

	if (++profile > 100000)
	{
		pr_xstatement = st - pr_statements;
		PR_RunError("runaway loop error");
	}

	if (pr_trace)
	{
		PrintStatement(st);
	}

	switch (st->op)
	{
	case OP_ADD_F:
		opc->_float = opa->_float + opb->_float;
		break;
	case OP_ADD_V:
		opc->vector[0] = opa->vector[0] + opb->vector[0];
		opc->vector[1] = opa->vector[1] + opb->vector[1];
		opc->vector[2] = opa->vector[2] + opb->vector[2];
		break;

	case OP_SUB_F:
		opc->_float = opa->_float - opb->_float;
		break;
	case OP_SUB_V:
		opc->vector[0] = opa->vector[0] - opb->vector[0];
		opc->vector[1] = opa->vector[1] - opb->vector[1];
		opc->vector[2] = opa->vector[2] - opb->vector[2];
		break;

	case OP_MUL_F:
		opc->_float = opa->_float * opb->_float;
		break;
	case OP_MUL_V:
		opc->_float = opa->vector[0] * opb->vector[0] +
			      opa->vector[1] * opb->vector[1] +
			      opa->vector[2] * opb->vector[2];
		break;
	case OP_MUL_FV:
		opc->vector[0] = opa->_float * opb->vector[0];
		opc->vector[1] = opa->_float * opb->vector[1];
		opc->vector[2] = opa->_float * opb->vector[2];
		break;
	case OP_MUL_VF:
		opc->vector[0] = opb->_float * opa->vector[0];
		opc->vector[1] = opb->_float * opa->vector[1];
		opc->vector[2] = opb->_float * opa->vector[2];
		break;

	case OP_DIV_F:
		opc->_float = opa->_float / opb->_float;
		break;

	case OP_BITAND:
		opc->_float = (int)opa->_float & (int)opb->_float;
		break;

	case OP_BITOR:
		opc->_float = (int)opa->_float | (int)opb->_float;
		break;

	case OP_GE:
		opc->_float = opa->_float >= opb->_float;
		break;
	case OP_LE:
		opc->_float = opa->_float <= opb->_float;
		break;
	case OP_GT:
		opc->_float = opa->_float > opb->_float;
		break;
	case OP_LT:
		opc->_float = opa->_float < opb->_float;
		break;
	case OP_AND:
		opc->_float = opa->_float && opb->_float;
		break;
	case OP_OR:
		opc->_float = opa->_float || opb->_float;
		break;

	case OP_NOT_F:
		opc->_float = !opa->_float;
		break;
	case OP_NOT_V:
		opc->_float = !opa->vector[0] && !opa->vector[1] && !opa->vector[2];
		break;
	case OP_NOT_S:
		opc->_float = !opa->string || !*PR_GetString(opa->string);
		break;
	case OP_NOT_FNC:
		opc->_float = !opa->function;
		break;
	case OP_NOT_ENT:
		opc->_float = (PROG_TO_EDICT(opa->edict) == sv.edicts);
		break;

	case OP_EQ_F:
		opc->_float = opa->_float == opb->_float;
		break;
	case OP_EQ_V:
		opc->_float = (opa->vector[0] == opb->vector[0]) &&
			      (opa->vector[1] == opb->vector[1]) &&
			      (opa->vector[2] == opb->vector[2]);
		break;
	case OP_EQ_S:
		opc->_float = !strcmp(PR_GetString(opa->string), PR_GetString(opb->string));
		break;
	case OP_EQ_E:
		opc->_float = opa->_int == opb->_int;
		break;
	case OP_EQ_FNC:
		opc->_float = opa->function == opb->function;
		break;

	case OP_NE_F:
		opc->_float = opa->_float != opb->_float;
		break;
	case OP_NE_V:
		opc->_float = (opa->vector[0] != opb->vector[0]) ||
			      (opa->vector[1] != opb->vector[1]) ||
			      (opa->vector[2] != opb->vector[2]);
		break;
	case OP_NE_S:
		opc->_float = strcmp(PR_GetString(opa->string), PR_GetString(opb->string));
		break;
	case OP_NE_E:
		opc->_float = opa->_int != opb->_int;
		break;
	case OP_NE_FNC:
		opc->_float = opa->function != opb->function;
		break;

	case OP_STORE_F:
	case OP_STORE_ENT:
	case OP_STORE_FLD:	// integers
	case OP_STORE_S:
	case OP_STORE_FNC:	// pointers
		opb->_int = opa->_int;
		break;
	case OP_STORE_V:
		opb->vector[0] = opa->vector[0];
		opb->vector[1] = opa->vector[1];
		opb->vector[2] = opa->vector[2];
		break;

	case OP_STOREP_F:
	case OP_STOREP_ENT:
	case OP_STOREP_FLD:	// integers
	case OP_STOREP_S:
	case OP_STOREP_FNC:	// pointers
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		ptr->_int = opa->_int;
		break;
	case OP_STOREP_V:
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		ptr->vector[0] = opa->vector[0];
		ptr->vector[1] = opa->vector[1];
		ptr->vector[2] = opa->vector[2];
		break;

	case OP_MULSTORE_F:	// f *= f
		opb->_float *= opa->_float;
		break;
	case OP_MULSTORE_V:	// v *= f
		opb->vector[0] *= opa->_float;
		opb->vector[1] *= opa->_float;
		opb->vector[2] *= opa->_float;
		break;
	case OP_MULSTOREP_F:	// e.f *= f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->_float = (ptr->_float *= opa->_float);
		break;
	case OP_MULSTOREP_V:	// e.v *= f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->vector[0] = (ptr->vector[0] *= opa->_float);
		opc->vector[0] = (ptr->vector[1] *= opa->_float);
		opc->vector[0] = (ptr->vector[2] *= opa->_float);
		break;

	case OP_DIVSTORE_F:	// f /= f
		opb->_float /= opa->_float;
		break;
	case OP_DIVSTOREP_F:	// e.f /= f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->_float = (ptr->_float /= opa->_float);
		break;

	case OP_ADDSTORE_F:	// f += f
		opb->_float += opa->_float;
		break;
	case OP_ADDSTORE_V:	// v += v
		opb->vector[0] += opa->vector[0];
		opb->vector[1] += opa->vector[1];
		opb->vector[2] += opa->vector[2];
		break;
	case OP_ADDSTOREP_F:	// e.f += f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->_float = (ptr->_float += opa->_float);
		break;
	case OP_ADDSTOREP_V:	// e.v += v
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->vector[0] = (ptr->vector[0] += opa->vector[0]);
		opc->vector[1] = (ptr->vector[1] += opa->vector[1]);
		opc->vector[2] = (ptr->vector[2] += opa->vector[2]);
		break;

	case OP_SUBSTORE_F:	// f -= f
		opb->_float -= opa->_float;
		break;
	case OP_SUBSTORE_V:	// v -= v
		opb->vector[0] -= opa->vector[0];
		opb->vector[1] -= opa->vector[1];
		opb->vector[2] -= opa->vector[2];
		break;
	case OP_SUBSTOREP_F:	// e.f -= f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->_float = (ptr->_float -= opa->_float);
		break;
	case OP_SUBSTOREP_V:	// e.v -= v
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		opc->vector[0] = (ptr->vector[0] -= opa->vector[0]);
		opc->vector[1] = (ptr->vector[1] -= opa->vector[1]);
		opc->vector[2] = (ptr->vector[2] -= opa->vector[2]);
		break;

	case OP_ADDRESS:
		ed = PROG_TO_EDICT(opa->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
		{
			pr_xstatement = st - pr_statements;
			PR_RunError("assignment to world entity");
		}
		opc->_int = (byte *)((int *)&ed->v + opb->_int) - (byte *)sv.edicts;
		break;

	case OP_LOAD_F:
	case OP_LOAD_FLD:
	case OP_LOAD_ENT:
	case OP_LOAD_S:
	case OP_LOAD_FNC:
		ed = PROG_TO_EDICT(opa->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		opc->_int = ((eval_t *)((int *)&ed->v + opb->_int))->_int;
		break;

	case OP_LOAD_V:
		ed = PROG_TO_EDICT(opa->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		ptr = (eval_t *)((int *)&ed->v + opb->_int);
		opc->vector[0] = ptr->vector[0];
		opc->vector[1] = ptr->vector[1];
		opc->vector[2] = ptr->vector[2];
		break;

	case OP_FETCH_GBL_F:
	case OP_FETCH_GBL_S:
	case OP_FETCH_GBL_E:
	case OP_FETCH_GBL_FNC:
	  {	int i = (int)opb->_float;
		if (i < 0 || i > G_INT(st->a - 1))
		{
			pr_xstatement = st - pr_statements;
			PR_RunError("array index out of bounds: %d", i);
		}
		opc->_int = ((eval_t *)&pr_globals[st->a + i])->_int;
	  }	break;
	case OP_FETCH_GBL_V:
	  {	int i = (int)opb->_float;
		if (i < 0 || i > G_INT(st->a - 1))
		{
			pr_xstatement = st - pr_statements;
			PR_RunError("array index out of bounds: %d", i);
		}
		ptr = (eval_t *)&pr_globals[st->a + (i * 3)];
		opc->vector[0] = ptr->vector[0];
		opc->vector[1] = ptr->vector[1];
		opc->vector[2] = ptr->vector[2];
	  }	break;

	case OP_IFNOT:
		if (!opa->_int)
		{
		/* Pa3PyX: a, b, and c used to be signed shorts for progs v6,
		 * now they are signed ints.  The problem is, they were used
		 * as signed sometimes and as unsigned other times - most of
		 * the time they were used as unsigned with an explicit cast
		 * in PR_ExecuteProgram().  When we convert the old progs to
		 * to the new format in PR_ConvertOldStmts(), we zero-extend
		 * them instead of sign-extending them for that reason: if we
		 * sign-extend them, most of the code will not work - we will
		 * have negative array offsets in PR_ExecuteProgram(), among
		 * other things.  Note that they are cast to unsigned short
		 * in PR_ConvertOldStmts() prior to assigning them to what is
		 * now int.  There are a few instances where these shorts are
		 * used as signed as in the case below where negative offsets
		 * are needed.  Since we now have a zero-extended number in a,
		 * b, and c, we must change it back to signed short, so that
		 * when it is added with and assigned to an int, the result
		 * ends up sign-extended and we get a proper negative offset,
		 * if there is one.
		 */
			jump_ofs = st->b;
			if (is_progs_v6) jump_ofs = (signed short)jump_ofs;
			st += jump_ofs - 1;	/* -1 to offset the st++ */
		}
		break;

	case OP_IF:
		if (opa->_int)
		{
			jump_ofs = st->b;
			if (is_progs_v6) jump_ofs = (signed short)jump_ofs;
			st += jump_ofs - 1;	/* -1 to offset the st++ */
		}
		break;

	case OP_GOTO:
		jump_ofs = st->a;
		if (is_progs_v6) jump_ofs = (signed short)jump_ofs;
		st += jump_ofs - 1;	/* -1 to offset the st++ */
		break;

	case OP_CALL8:
	case OP_CALL7:
	case OP_CALL6:
	case OP_CALL5:
	case OP_CALL4:
	case OP_CALL3:
	case OP_CALL2:	// Copy second arg to shared space
		vecptr = G_VECTOR(OFS_PARM1);
		VectorCopy(opc->vector, vecptr);
	case OP_CALL1:	// Copy first arg to shared space
		vecptr = G_VECTOR(OFS_PARM0);
		VectorCopy(opb->vector, vecptr);
	case OP_CALL0:
		pr_xfunction->profile += profile - startprofile;
		startprofile = profile;
		pr_xstatement = st - pr_statements;
		pr_argc = st->op - OP_CALL0;
		if (!opa->function)
		{
			PR_RunError("NULL function");
		}
		newf = &pr_functions[opa->function];
		if (newf->first_statement < 0)
		{ // Built-in function
			int i = -newf->first_statement;
			if (i >= pr_numbuiltins)
			{
				PR_RunError("Bad builtin call number %d", i);
			}
			pr_builtins[i]();
			break;
		}
		// Normal function
		st = &pr_statements[EnterFunction(newf)];
		break;

	case OP_DONE:
	case OP_RETURN:
	  {
		float *retptr = &pr_globals[OFS_RETURN];
		float *valptr = &pr_globals[st->a];
		pr_xfunction->profile += profile - startprofile;
		startprofile = profile;
		pr_xstatement = st - pr_statements;
		*retptr++ = *valptr++;
		*retptr++ = *valptr++;
		*retptr   = *valptr;
		st = &pr_statements[LeaveFunction()];
		if (pr_depth == exitdepth)
		{ // Done
			return;
		}
	  }	break;

	case OP_STATE:
		ed = PROG_TO_EDICT(*sv_globals.self);
/* Id 1.07 changes
#ifdef FPS_20
		ed->v.nextthink = *sv_globals.time + 0.05;
#else
		ed->v.nextthink = *sv_globals.time + 0.1;
#endif
*/
		ed->v.nextthink = *sv_globals.time + HX_FRAME_TIME;
		ed->v.frame = opa->_float;
		ed->v.think = opb->function;
		break;

	case OP_CSTATE:	// Cycle state
	  {	int startFrame, endFrame;
		ed = PROG_TO_EDICT(*sv_globals.self);
		ed->v.nextthink = *sv_globals.time + HX_FRAME_TIME;
		ed->v.think = pr_xfunction - pr_functions;
		*sv_globals.cycle_wrapped = false;
		startFrame = (int)opa->_float;
		endFrame = (int)opb->_float;
		if (startFrame <= endFrame)
		{ // Increment
			if (ed->v.frame < startFrame || ed->v.frame > endFrame)
			{
				ed->v.frame = startFrame;
			}
			else
			{
				ed->v.frame++;
				if (ed->v.frame > endFrame)
				{
					*sv_globals.cycle_wrapped = true;
					ed->v.frame = startFrame;
				}
			}
		}
		else
		{ // Decrement
			if (ed->v.frame > startFrame || ed->v.frame < endFrame)
			{
				ed->v.frame = startFrame;
			}
			else
			{
				ed->v.frame--;
				if (ed->v.frame < endFrame)
				{
					*sv_globals.cycle_wrapped = true;
					ed->v.frame = startFrame;
				}
			}
		}
	  }	break;

	case OP_CWSTATE:	// Cycle weapon state
	  {	int startFrame, endFrame;
		ed = PROG_TO_EDICT(*sv_globals.self);
		ed->v.nextthink = *sv_globals.time + HX_FRAME_TIME;
		ed->v.think = pr_xfunction - pr_functions;
		*sv_globals.cycle_wrapped = false;
		startFrame = (int)opa->_float;
		endFrame = (int)opb->_float;
		if (startFrame <= endFrame)
		{ // Increment
			if (ed->v.weaponframe < startFrame
				|| ed->v.weaponframe > endFrame)
			{
				ed->v.weaponframe = startFrame;
			}
			else
			{
				ed->v.weaponframe++;
				if (ed->v.weaponframe > endFrame)
				{
					*sv_globals.cycle_wrapped = true;
					ed->v.weaponframe = startFrame;
				}
			}
		}
		else
		{ // Decrement
			if (ed->v.weaponframe > startFrame
				|| ed->v.weaponframe < endFrame)
			{
				ed->v.weaponframe = startFrame;
			}
			else
			{
				ed->v.weaponframe--;
				if (ed->v.weaponframe < endFrame)
				{
					*sv_globals.cycle_wrapped = true;
					ed->v.weaponframe = startFrame;
				}
			}
		}
	  }	break;

	case OP_THINKTIME:
		ed = PROG_TO_EDICT(opa->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
		{
			pr_xstatement = st - pr_statements;
			PR_RunError("assignment to world entity");
		}
		ed->v.nextthink = *sv_globals.time + opb->_float;
		break;

	case OP_BITSET:		// f (+) f
		opb->_float = (int)opb->_float | (int)opa->_float;
		break;
	case OP_BITSETP:	// e.f (+) f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		ptr->_float = (int)ptr->_float | (int)opa->_float;
		break;
	case OP_BITCLR:		// f (-) f
		opb->_float = (int)opb->_float & ~((int)opa->_float);
		break;
	case OP_BITCLRP:	// e.f (-) f
		ptr = (eval_t *)((byte *)sv.edicts + opb->_int);
		ptr->_float = (int)ptr->_float & ~((int)opa->_float);
		break;

	case OP_RAND0:
	  {	float val;
		val = rand() * (1.0 / RAND_MAX);
		G_FLOAT(OFS_RETURN) = val;
	  }	break;
	case OP_RAND1:
	  {	float val;
		val = rand() * (1.0 / RAND_MAX) * opa->_float;
		G_FLOAT(OFS_RETURN) = val;
	  }	break;
	case OP_RAND2:
	  {	float val;
		if (opa->_float < opb->_float)
		{
			val = opa->_float + (rand() * (1.0 / RAND_MAX) * (opb->_float - opa->_float));
		}
		else
		{
			val = opb->_float + (rand() * (1.0 / RAND_MAX) * (opa->_float - opb->_float));
		}
		G_FLOAT(OFS_RETURN) = val;
	  }	break;
	case OP_RANDV0:
	  {	float val;
		float *retptr = &G_FLOAT(OFS_RETURN);
		val = rand() * (1.0 / RAND_MAX);
		//G_FLOAT(OFS_RETURN + 0) = val;
		*retptr++ = val;
		val = rand() * (1.0 / RAND_MAX);
		//G_FLOAT(OFS_RETURN + 1) = val;
		*retptr++ = val;
		val = rand() * (1.0 / RAND_MAX);
		//G_FLOAT(OFS_RETURN + 2) = val;
		*retptr = val;
	  }	break;
	case OP_RANDV1:
	  {	float val;
		float *retptr = &G_FLOAT(OFS_RETURN);
		val = rand() * (1.0 / RAND_MAX) * opa->vector[0];
		//G_FLOAT(OFS_RETURN + 0) = val;
		*retptr++ = val;
		val = rand() * (1.0 / RAND_MAX) * opa->vector[1];
		//G_FLOAT(OFS_RETURN + 1) = val;
		*retptr++ = val;
		val = rand() * (1.0 / RAND_MAX) * opa->vector[2];
		//G_FLOAT(OFS_RETURN + 2) = val;
		*retptr = val;
	  }	break;
	case OP_RANDV2:
	  {	float val;
		int	i;
		float *retptr = &G_FLOAT(OFS_RETURN);
		for (i = 0; i < 3; i++)
		{
			if (opa->vector[i] < opb->vector[i])
			{
				val = opa->vector[i] + (rand() * (1.0 / RAND_MAX) * (opb->vector[i] - opa->vector[i]));
			}
			else
			{
				val = opb->vector[i] + (rand() * (1.0 / RAND_MAX) * (opa->vector[i] - opb->vector[i]));
			}
			//G_FLOAT(OFS_RETURN + i) = val;
			*retptr++ = val;
		}
	  }	break;
	case OP_SWITCH_F:
		case_type = SWITCH_F;
		switch_float = opa->_float;
		jump_ofs = st->b;
		if (is_progs_v6) jump_ofs = (signed short)jump_ofs;
		st += jump_ofs - 1;	/* -1 to offset the st++ */
		break;
	case OP_SWITCH_V:
	case OP_SWITCH_S:
	case OP_SWITCH_E:
	case OP_SWITCH_FNC:
		pr_xstatement = st - pr_statements;
		PR_RunError("%s not done yet!", pr_opnames[st->op]);
		break;

	case OP_CASERANGE:
		if (case_type != SWITCH_F)
		{
			pr_xstatement = st - pr_statements;
			PR_RunError("caserange fucked!");
		}
		if ((switch_float >= opa->_float) && (switch_float <= opb->_float))
		{
			jump_ofs = st->c;
			if (is_progs_v6) jump_ofs = (signed short)jump_ofs;
			st += jump_ofs - 1;	/* -1 to offset the st++ */
		}
		break;
	case OP_CASE:
		switch (case_type)
		{
		case SWITCH_F:
			if (switch_float == opa->_float)
			{
				jump_ofs = st->b;
				if (is_progs_v6) jump_ofs = (signed short)jump_ofs;
				st += jump_ofs - 1;	/* -1 to offset the st++ */
			}
			break;
		case SWITCH_V:
		case SWITCH_S:
		case SWITCH_E:
		case SWITCH_FNC:
			pr_xstatement = st - pr_statements;
			PR_RunError("OP_CASE for %s not done yet!",
					pr_opnames[case_type + OP_SWITCH_F - SWITCH_F]);
			break;
		default:
			pr_xstatement = st - pr_statements;
			PR_RunError("fucked case!");
		}
		break;

	default:
		pr_xstatement = st - pr_statements;
		PR_RunError("Bad opcode %i", st->op);
	}
    }	/* end of while(1) loop */
}
#undef OPA
#undef OPB
#undef OPC


//==========================================================================
//
// EnterFunction
//
//==========================================================================

static int EnterFunction (dfunction_t *f)
{
	int	i, j, c, o;

	pr_stack[pr_depth].s = pr_xstatement;
	pr_stack[pr_depth].f = pr_xfunction;
	pr_depth++;
	if (pr_depth >= MAX_STACK_DEPTH)
	{
		PR_RunError("stack overflow");
	}

	// save off any locals that the new function steps on
	c = f->locals;
	if (localstack_used + c > LOCALSTACK_SIZE)
	{
		PR_RunError ("%s: locals stack overflow", __thisfunc__);
	}

	for (i = 0; i < c ; i++)
	{
		localstack[localstack_used + i] = ((int *)pr_globals)[f->parm_start + i];
	}
	localstack_used += c;

	// copy parameters
	o = f->parm_start;
	for (i = 0; i < f->numparms; i++)
	{
		for (j = 0; j < f->parm_size[i]; j++)
		{
			((int *)pr_globals)[o] = ((int *)pr_globals)[OFS_PARM0 + i*3 + j];
			o++;
		}
	}

	pr_xfunction = f;
	return f->first_statement - 1;	// offset the s++
}


//==========================================================================
//
// LeaveFunction
//
//==========================================================================

static int LeaveFunction (void)
{
	int	i, c;

	if (pr_depth <= 0)
	{
		Host_Error("prog stack underflow");
	}

	// Restore locals from the stack
	c = pr_xfunction->locals;
	localstack_used -= c;
	if (localstack_used < 0)
	{
		PR_RunError("%s: locals stack underflow", __thisfunc__);
	}

	for (i = 0; i < c; i++)
	{
		((int *)pr_globals)[pr_xfunction->parm_start + i] = localstack[localstack_used + i];
	}

	// up stack
	pr_depth--;
	pr_xfunction = pr_stack[pr_depth].f;
	return pr_stack[pr_depth].s;
}


//==========================================================================
//
// PR_RunError
//
//==========================================================================

void PR_RunError (const char *error, ...)
{
	va_list	argptr;
	char	string[1024];

	va_start (argptr, error);
	q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	PrintStatement(pr_statements + pr_xstatement);
	PrintCallHistory();

	Con_Printf("%s\n", string);

	pr_depth = 0;	// dump the stack so host_error can shutdown functions

	Host_Error("Program error");
}


//==========================================================================
//
// PrintCallHistory
//
//==========================================================================

static void PrintCallHistory (void)
{
	int		i;
	dfunction_t	*f;

	if (pr_depth == 0)
	{
		Con_Printf("<NO STACK>\n");
		return;
	}

	pr_stack[pr_depth].f = pr_xfunction;
	for (i = pr_depth; i >= 0; i--)
	{
		f = pr_stack[i].f;
		if (!f)
		{
			Con_Printf("<NO FUNCTION>\n");
		}
		else
		{
			Con_Printf("%12s : %s\n", PR_GetString(f->s_file), PR_GetString(f->s_name));
		}
	}
}


//==========================================================================
//
// PrintStatement
//
//==========================================================================

static void PrintStatement (dstatement_t *s)
{
	int	i;

	if ((unsigned int)s->op < sizeof(pr_opnames)/sizeof(pr_opnames[0]))
	{
		Con_Printf("%s ", pr_opnames[s->op]);
		i = strlen(pr_opnames[s->op]);
		for ( ; i < 10; i++)
		{
			Con_Printf(" ");
		}
	}

	if (s->op == OP_IF || s->op == OP_IFNOT)
	{
		Con_Printf("%sbranch %i", PR_GlobalString(s->a), s->b);
	}
	else if (s->op == OP_GOTO)
	{
		Con_Printf("branch %i", s->a);
	}
	else if ((unsigned int)(s->op-OP_STORE_F) < 6)
	{
		Con_Printf("%s", PR_GlobalString(s->a));
		Con_Printf("%s", PR_GlobalStringNoContents(s->b));
	}
	else
	{
		if (s->a)
		{
			Con_Printf("%s", PR_GlobalString(s->a));
		}
		if (s->b)
		{
			Con_Printf("%s", PR_GlobalString(s->b));
		}
		if (s->c)
		{
			Con_Printf("%s", PR_GlobalStringNoContents(s->c));
		}
	}
	Con_Printf("\n");
}


//==========================================================================
//
// PR_Profile_f
//
//==========================================================================

void PR_Profile_f (void)
{
	int		i, j;
	int		pmax;
	dfunction_t	*f, *bestFunc;
	int		total;
	int		funcCount;
	qboolean	byHC;
	const char	*saveName = NULL;
	FILE	*saveFile = NULL;
	int		currentFile;
	int		bestFile;
	int		tally;
	const char	*s;

	if (!sv.active)
		return;

	byHC = false;
	funcCount = 10;
	for (i = 1; i < Cmd_Argc(); i++)
	{
		s = Cmd_Argv(i);
		if (*s == 'h' || *s == 'H')
		{ // Sort by HC source file
			byHC = true;
		}
		else if (*s == 's' || *s == 'S')
		{ // Save to file
			if (i + 1 < Cmd_Argc() && !q_isdigit(*Cmd_Argv(i + 1)))
			{
				i++;
				saveName = FS_MakePath(FS_USERDIR, NULL, Cmd_Argv(i));
			}
			else
			{
				saveName = FS_MakePath(FS_USERDIR, NULL, "profile.txt");
			}
		}
		else if (q_isdigit(*s))
		{ // Specify function count
			funcCount = atoi(Cmd_Argv(i));
			if (funcCount < 1)
			{
				funcCount = 1;
			}
		}
	}

	total = 0;
	for (i = 0; i < progs->numfunctions; i++)
	{
		total += pr_functions[i].profile;
	}

	if (saveName)
	{ // Create the output file
		saveFile = fopen(saveName, "w");
		if (saveFile == NULL)
			Con_Printf("Could not open %s\n", saveName);
	}

	if (byHC == false)
	{
		j = 0;
		do
		{
			pmax = 0;
			bestFunc = NULL;
			for (i = 0; i < progs->numfunctions; i++)
			{
				f = &pr_functions[i];
				if (f->profile > pmax)
				{
					pmax = f->profile;
					bestFunc = f;
				}
			}
			if (bestFunc)
			{
				if (j < funcCount)
				{
					if (saveFile)
					{
						fprintf(saveFile, "%05.2f %s\n",
								((float)bestFunc->profile / (float)total) * 100.0,
								PR_GetString(bestFunc->s_name));
					}
					else
					{
						Con_Printf("%05.2f %s\n",
								((float)bestFunc->profile / (float)total) * 100.0,
								PR_GetString(bestFunc->s_name));
					}
				}
				j++;
				bestFunc->profile = 0;
			}
		} while (bestFunc);

		if (saveFile)
		{
			fclose(saveFile);
		}
		return;
	}

	currentFile = -1;
	do
	{
		tally = 0;
		bestFile = Q_MAXINT;
		for (i = 0; i < progs->numfunctions; i++)
		{
			if (pr_functions[i].s_file > currentFile
				&& pr_functions[i].s_file < bestFile)
			{
				bestFile = pr_functions[i].s_file;
				tally = pr_functions[i].profile;
				continue;
			}
			if (pr_functions[i].s_file == bestFile)
			{
				tally += pr_functions[i].profile;
			}
		}
		currentFile = bestFile;
		if (tally && currentFile != Q_MAXINT)
		{
			if (saveFile)
			{
				fprintf(saveFile, "\"%s\"\n", PR_GetString(currentFile));
			}
			else
			{
				Con_Printf("\"%s\"\n", PR_GetString(currentFile));
			}

			j = 0;
			do
			{
				pmax = 0;
				bestFunc = NULL;
				for (i = 0; i < progs->numfunctions; i++)
				{
					f = &pr_functions[i];
					if (f->s_file == currentFile && f->profile > pmax)
					{
						pmax = f->profile;
						bestFunc = f;
					}
				}
				if (bestFunc)
				{
					if (j < funcCount)
					{
						if (saveFile)
						{
							fprintf(saveFile, "   %05.2f %s\n",
									((float)bestFunc->profile / (float)total) * 100.0,
									PR_GetString(bestFunc->s_name));
						}
						else
						{
							Con_Printf("   %05.2f %s\n",
									((float)bestFunc->profile / (float)total) * 100.0,
									PR_GetString(bestFunc->s_name));
						}
					}
					j++;
					bestFunc->profile = 0;
				}
			} while (bestFunc);
		}
	} while (currentFile != Q_MAXINT);

	if (saveFile)
	{
		fclose(saveFile);
	}
}

