/*
	pr_exec.c
	PROGS execution

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/pr_exec.c,v 1.25 2008-01-22 12:01:04 sezero Exp $
*/

// HEADER FILES ------------------------------------------------------------

#include "quakedef.h"
#include <ctype.h>

// MACROS ------------------------------------------------------------------

#define MAX_STACK_DEPTH 32
#define LOCALSTACK_SIZE 2048

#if defined(_MSC_VER) && defined(_WIN32) && defined(DEBUG_BUILD)
// Uses the Pentium specific opcode RDTSC (ReaD TimeStamp Counter)
#define TIMESNAP_ACTIVE
#define TIMESNAP(clock) __asm push eax\
	__asm push edx\
	__asm _emit 0x0F\
	__asm _emit 0x31\
	__asm mov clock, eax\
	__asm pop edx\
	__asm pop eax
#endif

// TYPES -------------------------------------------------------------------

typedef struct
{
	int		s;
	dfunction_t	*f;
} prstack_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

const char *PR_GlobalString(int ofs);
const char *PR_GlobalStringNoContents(int ofs);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static int EnterFunction(dfunction_t *f);
static int LeaveFunction(void);
static void PrintStatement(dstatement_t *s);
static void PrintCallHistory(void);
#ifdef TIMESNAP_ACTIVE
static unsigned int ProgsTimer(void);
#endif

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

//switch types
enum {SWITCH_F,SWITCH_V,SWITCH_S,SWITCH_E,SWITCH_FNC};

void PR_ExecuteProgram (func_t fnum)
{
	int		i;
	int		s;
	eval_t		*a, *b, *c;
	eval_t		*ptr;
	dstatement_t	*st;
	dfunction_t	*f, *newf;
	int		runaway;
	edict_t		*ed;
	int		exitdepth;
	int		startFrame;
	int		endFrame;
	float		val;
	int		case_type = -1;
	float		switch_float = 0;	// shut up compiler

	if (!fnum || fnum >= progs->numfunctions)
	{
		if (PR_GLOBAL_STRUCT(self))
		{
			ED_Print(PROG_TO_EDICT(PR_GLOBAL_STRUCT(self)));
		}
		Host_Error("%s: NULL function", __thisfunc__);
	}

	f = &pr_functions[fnum];

	runaway = 100000;
	pr_trace = false;

	exitdepth = pr_depth;

	s = EnterFunction(f);
#ifdef TIMESNAP_ACTIVE
	ProgsTimer(); // Init
#endif

    while (1)
    {
	s++; // Next statement

	st = &pr_statements[s];
	a = (eval_t *)&pr_globals[(unsigned short)st->a];
	b = (eval_t *)&pr_globals[(unsigned short)st->b];
	c = (eval_t *)&pr_globals[(unsigned short)st->c];

	if (!--runaway)
	{
		PR_RunError("runaway loop error");
	}

#ifndef TIMESNAP_ACTIVE
	pr_xfunction->profile++;
#endif

	pr_xstatement = s;

	if (pr_trace)
	{
		PrintStatement(st);
	}

	switch (st->op)
	{
	case OP_ADD_F:
		c->_float = a->_float + b->_float;
		break;
	case OP_ADD_V:
		c->vector[0] = a->vector[0] + b->vector[0];
		c->vector[1] = a->vector[1] + b->vector[1];
		c->vector[2] = a->vector[2] + b->vector[2];
		break;

	case OP_SUB_F:
		c->_float = a->_float - b->_float;
		break;
	case OP_SUB_V:
		c->vector[0] = a->vector[0] - b->vector[0];
		c->vector[1] = a->vector[1] - b->vector[1];
		c->vector[2] = a->vector[2] - b->vector[2];
		break;

	case OP_MUL_F:
		c->_float = a->_float * b->_float;
		break;
	case OP_MUL_V:
		c->_float = a->vector[0]*b->vector[0]
			  + a->vector[1]*b->vector[1]
			  + a->vector[2]*b->vector[2];
		break;
	case OP_MUL_FV:
		c->vector[0] = a->_float * b->vector[0];
		c->vector[1] = a->_float * b->vector[1];
		c->vector[2] = a->_float * b->vector[2];
		break;
	case OP_MUL_VF:
		c->vector[0] = b->_float * a->vector[0];
		c->vector[1] = b->_float * a->vector[1];
		c->vector[2] = b->_float * a->vector[2];
		break;

	case OP_DIV_F:
		c->_float = a->_float / b->_float;
		break;

	case OP_BITAND:
		c->_float = (int)a->_float & (int)b->_float;
		break;

	case OP_BITOR:
		c->_float = (int)a->_float | (int)b->_float;
		break;

	case OP_GE:
		c->_float = a->_float >= b->_float;
		break;
	case OP_LE:
		c->_float = a->_float <= b->_float;
		break;
	case OP_GT:
		c->_float = a->_float > b->_float;
		break;
	case OP_LT:
		c->_float = a->_float < b->_float;
		break;
	case OP_AND:
		c->_float = a->_float && b->_float;
		break;
	case OP_OR:
		c->_float = a->_float || b->_float;
		break;

	case OP_NOT_F:
		c->_float = !a->_float;
		break;
	case OP_NOT_V:
		c->_float = !a->vector[0] && !a->vector[1] && !a->vector[2];
		break;
	case OP_NOT_S:
		c->_float = !a->string || !*PR_GetString(a->string);
		break;
	case OP_NOT_FNC:
		c->_float = !a->function;
		break;
	case OP_NOT_ENT:
		c->_float = (PROG_TO_EDICT(a->edict) == sv.edicts);
		break;

	case OP_EQ_F:
		c->_float = a->_float == b->_float;
		break;
	case OP_EQ_V:
		c->_float = (a->vector[0] == b->vector[0])
			&& (a->vector[1] == b->vector[1])
			&& (a->vector[2] == b->vector[2]);
		break;
	case OP_EQ_S:
		c->_float = !strcmp(PR_GetString(a->string), PR_GetString(b->string));
		break;
	case OP_EQ_E:
		c->_float = a->_int == b->_int;
		break;
	case OP_EQ_FNC:
		c->_float = a->function == b->function;
		break;

	case OP_NE_F:
		c->_float = a->_float != b->_float;
		break;
	case OP_NE_V:
		c->_float = (a->vector[0] != b->vector[0])
			|| (a->vector[1] != b->vector[1])
			|| (a->vector[2] != b->vector[2]);
		break;
	case OP_NE_S:
		c->_float = strcmp(PR_GetString(a->string), PR_GetString(b->string));
		break;
	case OP_NE_E:
		c->_float = a->_int != b->_int;
		break;
	case OP_NE_FNC:
		c->_float = a->function != b->function;
		break;

	case OP_STORE_F:
	case OP_STORE_ENT:
	case OP_STORE_FLD:	// integers
	case OP_STORE_S:
	case OP_STORE_FNC:	// pointers
		b->_int = a->_int;
		break;
	case OP_STORE_V:
		b->vector[0] = a->vector[0];
		b->vector[1] = a->vector[1];
		b->vector[2] = a->vector[2];
		break;

	case OP_STOREP_F:
	case OP_STOREP_ENT:
	case OP_STOREP_FLD:	// integers
	case OP_STOREP_S:
	case OP_STOREP_FNC:	// pointers
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->_int = a->_int;
		break;
	case OP_STOREP_V:
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->vector[0] = a->vector[0];
		ptr->vector[1] = a->vector[1];
		ptr->vector[2] = a->vector[2];
		break;

	case OP_MULSTORE_F:	// f *= f
		b->_float *= a->_float;
		break;
	case OP_MULSTORE_V:	// v *= f
		b->vector[0] *= a->_float;
		b->vector[1] *= a->_float;
		b->vector[2] *= a->_float;
		break;
	case OP_MULSTOREP_F:	// e.f *= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float *= a->_float);
		break;
	case OP_MULSTOREP_V:	// e.v *= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->vector[0] = (ptr->vector[0] *= a->_float);
		c->vector[0] = (ptr->vector[1] *= a->_float);
		c->vector[0] = (ptr->vector[2] *= a->_float);
		break;

	case OP_DIVSTORE_F:	// f /= f
		b->_float /= a->_float;
		break;
	case OP_DIVSTOREP_F:	// e.f /= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float /= a->_float);
		break;

	case OP_ADDSTORE_F:	// f += f
		b->_float += a->_float;
		break;
	case OP_ADDSTORE_V:	// v += v
		b->vector[0] += a->vector[0];
		b->vector[1] += a->vector[1];
		b->vector[2] += a->vector[2];
		break;
	case OP_ADDSTOREP_F:	// e.f += f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float += a->_float);
		break;
	case OP_ADDSTOREP_V:	// e.v += v
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->vector[0] = (ptr->vector[0] += a->vector[0]);
		c->vector[1] = (ptr->vector[1] += a->vector[1]);
		c->vector[2] = (ptr->vector[2] += a->vector[2]);
		break;

	case OP_SUBSTORE_F:	// f -= f
		b->_float -= a->_float;
		break;
	case OP_SUBSTORE_V:	// v -= v
		b->vector[0] -= a->vector[0];
		b->vector[1] -= a->vector[1];
		b->vector[2] -= a->vector[2];
		break;
	case OP_SUBSTOREP_F:	// e.f -= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float -= a->_float);
		break;
	case OP_SUBSTOREP_V:	// e.v -= v
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->vector[0] = (ptr->vector[0] -= a->vector[0]);
		c->vector[1] = (ptr->vector[1] -= a->vector[1]);
		c->vector[2] = (ptr->vector[2] -= a->vector[2]);
		break;

	case OP_ADDRESS:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
		{
			PR_RunError("assignment to world entity");
		}
		c->_int = (byte *)((int *)&ed->v + b->_int)-(byte *)sv.edicts;
		break;

	case OP_LOAD_F:
	case OP_LOAD_FLD:
	case OP_LOAD_ENT:
	case OP_LOAD_S:
	case OP_LOAD_FNC:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		a = (eval_t *)((int *)&ed->v + b->_int);
		c->_int = a->_int;
		break;

	case OP_LOAD_V:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		a = (eval_t *)((int *)&ed->v + b->_int);
		c->vector[0] = a->vector[0];
		c->vector[1] = a->vector[1];
		c->vector[2] = a->vector[2];
		break;

	case OP_FETCH_GBL_F:
	case OP_FETCH_GBL_S:
	case OP_FETCH_GBL_E:
	case OP_FETCH_GBL_FNC:
		i = (int)b->_float;
		if (i < 0 || i > G_INT((unsigned short)st->a - 1))
		{
			PR_RunError("array index out of bounds: %d", i);
		}
		a = (eval_t *)&pr_globals[(unsigned short)st->a + i];
		c->_int = a->_int;
		break;
	case OP_FETCH_GBL_V:
		i = (int)b->_float;
		if (i < 0 || i > G_INT((unsigned short)st->a - 1))
		{
			PR_RunError("array index out of bounds: %d", i);
		}
		a = (eval_t *)&pr_globals[(unsigned short)st->a + ((int)b->_float)*3];
		c->vector[0] = a->vector[0];
		c->vector[1] = a->vector[1];
		c->vector[2] = a->vector[2];
		break;

	case OP_IFNOT:
		if (!a->_int)
		{
			s += st->b - 1;	// -1 to offset the s++
		}
		break;

	case OP_IF:
		if (a->_int)
		{
			s += st->b - 1;	// -1 to offset the s++
		}
		break;

	case OP_GOTO:
		s += st->a - 1;		// -1 to offset the s++
		break;

	case OP_CALL8:
	case OP_CALL7:
	case OP_CALL6:
	case OP_CALL5:
	case OP_CALL4:
	case OP_CALL3:
	case OP_CALL2:	// Copy second arg to shared space
		VectorCopy(c->vector, G_VECTOR(OFS_PARM1));
	case OP_CALL1:	// Copy first arg to shared space
		VectorCopy(b->vector, G_VECTOR(OFS_PARM0));
	case OP_CALL0:
		pr_argc = st->op-OP_CALL0;
		if (!a->function)
		{
			PR_RunError("NULL function");
		}
		newf = &pr_functions[a->function];
		if (newf->first_statement < 0)
		{ // Built-in function
			i = -newf->first_statement;
			if (i >= pr_numbuiltins)
			{
				PR_RunError("Bad builtin call number");
			}
			pr_builtins[i]();
			break;
		}
		// Normal function
#ifdef TIMESNAP_ACTIVE
		pr_xfunction->profile += ProgsTimer();
#endif
		s = EnterFunction(newf);
		break;

	case OP_DONE:
	case OP_RETURN:
		pr_globals[OFS_RETURN] = pr_globals[(unsigned short)st->a];
		pr_globals[OFS_RETURN + 1] = pr_globals[(unsigned short)st->a + 1];
		pr_globals[OFS_RETURN + 2] = pr_globals[(unsigned short)st->a + 2];
#ifdef TIMESNAP_ACTIVE
		pr_xfunction->profile += ProgsTimer();
#endif
		s = LeaveFunction();
		if (pr_depth == exitdepth)
		{ // Done
			return;
		}
		break;

	case OP_STATE:
		ed = PROG_TO_EDICT(PR_GLOBAL_STRUCT(self));
/* Id 1.07 changes
#ifdef FPS_20
		ed->v.nextthink = PR_GLOBAL_STRUCT(time) + 0.05;
#else
		ed->v.nextthink = PR_GLOBAL_STRUCT(time) + 0.1;
#endif
*/
		ed->v.nextthink = PR_GLOBAL_STRUCT(time) + HX_FRAME_TIME;
		if (a->_float != ed->v.frame)
		{
			ed->v.frame = a->_float;
		}
		ed->v.think = b->function;
		break;

	case OP_CSTATE:	// Cycle state
		ed = PROG_TO_EDICT(PR_GLOBAL_STRUCT(self));
		ed->v.nextthink = PR_GLOBAL_STRUCT(time) + HX_FRAME_TIME;
		ed->v.think = pr_xfunction - pr_functions;
		if (is_progdefs111)
			pr_global_struct_v111->cycle_wrapped = false;
		else
			pr_global_struct->cycle_wrapped = false;
		startFrame = (int)a->_float;
		endFrame = (int)b->_float;
		if (startFrame <= endFrame)
		{ // Increment
			if (ed->v.frame < startFrame || ed->v.frame > endFrame)
			{
				ed->v.frame = startFrame;
				break;
			}
			ed->v.frame++;
			if (ed->v.frame > endFrame)
			{
				if (is_progdefs111)
					pr_global_struct_v111->cycle_wrapped = true;
				else
					pr_global_struct->cycle_wrapped = true;
				ed->v.frame = startFrame;
			}
			break;
		}
		// Decrement
		if (ed->v.frame > startFrame || ed->v.frame < endFrame)
		{
			ed->v.frame = startFrame;
			break;
		}
		ed->v.frame--;
		if (ed->v.frame < endFrame)
		{
			if (is_progdefs111)
				pr_global_struct_v111->cycle_wrapped = true;
			else
				pr_global_struct->cycle_wrapped = true;
			ed->v.frame = startFrame;
		}
		break;

	case OP_CWSTATE:	// Cycle weapon state
		ed = PROG_TO_EDICT(PR_GLOBAL_STRUCT(self));
		ed->v.nextthink = PR_GLOBAL_STRUCT(time) + HX_FRAME_TIME;
		ed->v.think = pr_xfunction - pr_functions;
		if (is_progdefs111)
			pr_global_struct_v111->cycle_wrapped = false;
		else
			pr_global_struct->cycle_wrapped = false;
		startFrame = (int)a->_float;
		endFrame = (int)b->_float;
		if (startFrame <= endFrame)
		{ // Increment
			if (ed->v.weaponframe < startFrame
				|| ed->v.weaponframe > endFrame)
			{
				ed->v.weaponframe = startFrame;
				break;
			}
			ed->v.weaponframe++;
			if (ed->v.weaponframe > endFrame)
			{
				if (is_progdefs111)
					pr_global_struct_v111->cycle_wrapped = true;
				else
					pr_global_struct->cycle_wrapped = true;
				ed->v.weaponframe = startFrame;
			}
			break;
		}
		// Decrement
		if (ed->v.weaponframe > startFrame
			|| ed->v.weaponframe < endFrame)
		{
			ed->v.weaponframe = startFrame;
			break;
		}
		ed->v.weaponframe--;
		if (ed->v.weaponframe < endFrame)
		{
			if (is_progdefs111)
				pr_global_struct_v111->cycle_wrapped = true;
			else
				pr_global_struct->cycle_wrapped = true;
			ed->v.weaponframe = startFrame;
		}
		break;

	case OP_THINKTIME:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);	// Make sure it's in range
#endif
		if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
		{
			PR_RunError("assignment to world entity");
		}
		ed->v.nextthink = PR_GLOBAL_STRUCT(time) + b->_float;
		break;

	case OP_BITSET:		// f (+) f
		b->_float = (int)b->_float | (int)a->_float;
		break;
	case OP_BITSETP:	// e.f (+) f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->_float = (int)ptr->_float | (int)a->_float;
		break;
	case OP_BITCLR:		// f (-) f
		b->_float = (int)b->_float & ~((int)a->_float);
		break;
	case OP_BITCLRP:	// e.f (-) f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->_float = (int)ptr->_float & ~((int)a->_float);
		break;

	case OP_RAND0:
	//	val = (rand() & 0x7fff) / ((float)0x7fff);
		val = rand() * (1.0 / RAND_MAX);
		G_FLOAT(OFS_RETURN) = val;
		break;
	case OP_RAND1:
		val = rand() * (1.0 / RAND_MAX) * a->_float;
		G_FLOAT(OFS_RETURN) = val;
		break;
	case OP_RAND2:
		if (a->_float < b->_float)
		{
			val = a->_float + (rand() * (1.0 / RAND_MAX) * (b->_float - a->_float));
		}
		else
		{
			val = b->_float + (rand() * (1.0 / RAND_MAX) * (a->_float - b->_float));
		}
		G_FLOAT(OFS_RETURN) = val;
		break;
	case OP_RANDV0:
		val = rand() * (1.0 / RAND_MAX);
		G_FLOAT(OFS_RETURN + 0) = val;
		val = rand() * (1.0 / RAND_MAX);
		G_FLOAT(OFS_RETURN + 1) = val;
		val = rand() * (1.0 / RAND_MAX);
		G_FLOAT(OFS_RETURN + 2) = val;
		break;
	case OP_RANDV1:
		val = rand() * (1.0 / RAND_MAX) * a->vector[0];
		G_FLOAT(OFS_RETURN + 0) = val;
		val = rand() * (1.0 / RAND_MAX) * a->vector[1];
		G_FLOAT(OFS_RETURN + 1) = val;
		val = rand() * (1.0 / RAND_MAX) * a->vector[2];
		G_FLOAT(OFS_RETURN + 2) = val;
		break;
	case OP_RANDV2:
		for (i = 0; i < 3; i++)
		{
			if (a->vector[i] < b->vector[i])
			{
				val = a->vector[i] + (rand() * (1.0 / RAND_MAX) * (b->vector[i] - a->vector[i]));
			}
			else
			{
				val = b->vector[i] + (rand() * (1.0 / RAND_MAX) * (a->vector[i] - b->vector[i]));
			}
			G_FLOAT(OFS_RETURN + i) = val;
		}
		break;
	case OP_SWITCH_F:
		case_type = SWITCH_F;
		switch_float = a->_float;
		s += st->b - 1;	// -1 to offset the s++
		break;
	case OP_SWITCH_V:
		PR_RunError("switch v not done yet!");
		break;
	case OP_SWITCH_S:
		PR_RunError("switch s not done yet!");
		break;
	case OP_SWITCH_E:
		PR_RunError("switch e not done yet!");
		break;
	case OP_SWITCH_FNC:
		PR_RunError("switch fnc not done yet!");
		break;

	case OP_CASERANGE:
		if (case_type != SWITCH_F)
			PR_RunError("caserange fucked!");
		if ((switch_float >= a->_float) && (switch_float <= b->_float))
		{
			s += st->c - 1;		// -1 to offset the s++
		}
		break;
	case OP_CASE:
		switch (case_type)
		{
		case SWITCH_F:
			if (switch_float == a->_float)
			{
				s += st->b - 1;	// -1 to offset the s++
			}
			break;
		case SWITCH_V:
		case SWITCH_S:
		case SWITCH_E:
		case SWITCH_FNC:
			PR_RunError("case not done yet!");
			break;
		default:
			PR_RunError("fucked case!");
		}
		break;

	default:
		PR_RunError("Bad opcode %i", st->op);
	}

    }	// end of while(1) loop

}


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
		Sys_Error("prog stack underflow");
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
	int		max;
	dfunction_t	*f, *bestFunc;
	int		total;
	int		funcCount;
	qboolean	byHC;
	char	saveName[MAX_OSPATH];
	FILE	*saveFile = NULL;
	int		currentFile;
	int		bestFile;
	int		tally;
	const char	*s;

	byHC = false;
	funcCount = 10;
	*saveName = 0;
	for (i = 1; i < Cmd_Argc(); i++)
	{
		s = Cmd_Argv(i);
		if (tolower(*s) == 'h')
		{ // Sort by HC source file
			byHC = true;
		}
		else if (tolower(*s) == 's')
		{ // Save to file
			if (i + 1 < Cmd_Argc() && !isdigit(*Cmd_Argv(i + 1)))
			{
				i++;
				sprintf(saveName, "%s/%s", fs_userdir, Cmd_Argv(i));
			}
			else
			{
				sprintf(saveName, "%s/profile.txt", fs_userdir);
			}
		}
		else if (isdigit(*s))
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

	if (*saveName)
	{ // Create the output file
		saveFile = fopen(saveName, "w");
		if (saveFile == NULL)
			Con_Printf("Could not open %s\n", saveName);
	}

#ifdef TIMESNAP_ACTIVE
	if (saveFile)
	{
		fprintf(saveFile, "(Timesnap Profile)\n");
	}
	else
	{
		Con_Printf("(Timesnap Profile)\n");
	}
#endif

	if (byHC == false)
	{
		j = 0;
		do
		{
			max = 0;
			bestFunc = NULL;
			for (i = 0; i < progs->numfunctions; i++)
			{
				f = &pr_functions[i];
				if (f->profile > max)
				{
					max = f->profile;
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
				max = 0;
				bestFunc = NULL;
				for (i = 0; i < progs->numfunctions; i++)
				{
					f = &pr_functions[i];
					if (f->s_file == currentFile && f->profile > max)
					{
						max = f->profile;
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


//==========================================================================
//
// ProgsTimer
//
//==========================================================================

#ifdef TIMESNAP_ACTIVE
static unsigned int ProgsTimer(void)
{
	unsigned int c;
	unsigned int cycleCount;
	static unsigned int cycleTimer;

	TIMESNAP(c);
	if (cycleTimer > c)
	{
		cycleCount = ((unsigned int)0xffffffff-(cycleTimer-c));
	}
	else
	{
		cycleCount = c-cycleTimer;
	}
	cycleTimer = c;
	return cycleCount>>8;
}
#endif

