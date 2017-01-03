/*
 * stmt.c
 *
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "hcc.h"

// MACROS ------------------------------------------------------------------

#define MAX_STATEMENT_DEPTH	64
#define MAX_CASE		256
#define MAX_BREAK		256
#define MAX_CONTINUE		128

// TYPES -------------------------------------------------------------------

typedef enum
{
	SCONTEXT_FUNCTION,
	SCONTEXT_IF,
	SCONTEXT_ELSE,
	SCONTEXT_DO,
	SCONTEXT_WHILE,
	SCONTEXT_UNTIL,
	SCONTEXT_SWITCH,
	SCONTEXT_LOOP
} scontext_t;

typedef struct
{
	int		level;
	dstatement_t	*patch;
} patchInfo_t;

typedef struct
{
	int		level;
	def_t	*value1;
	def_t	*value2;
	qboolean	isDefault;
	int		statement;
	etype_t	type;
} caseInfo_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void ParseStatement(scontext_t owner);
static void ParseReturn(void);
static void ParseLoop(void);
static void ParseWhile(void);
static void ParseUntil(void);
static void ParseDo(void);
static void ParseIf(void);
static void ParseLocalDefs(void);
static void ParseSwitch(void);
static void ParseCase(void);
static void ParseBreak(void);
static void ParseContinue(void);
static void ParseDefault(void);
static void ParseThinktime(void);
static void AddCase(etype_t type, def_t *value1, def_t *value2, qboolean isDefault);
static int GetCaseInfo(caseInfo_t **info);
static void AddBreak(void);
static qboolean BreakAncestor(void);
static void FixBreaks(void);
static void AddContinue(void);
static qboolean ContinueAncestor(void);
static void FixContinues(int statement);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

type_t	*st_ReturnType;
qboolean st_ReturnParsed;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int CaseIndex;
static int BreakIndex;
static int ContinueIndex;
static int StatementIndex;
static int ContextLevel;
static scontext_t ContextHistory[MAX_STATEMENT_DEPTH];
static caseInfo_t CaseInfo[MAX_CASE];
static patchInfo_t BreakInfo[MAX_BREAK];
static patchInfo_t ContinueInfo[MAX_CONTINUE];

static int EnterContext[] =
{
	0,		// SCONTEXT_FUNCTION
	0,		// SCONTEXT_IF
	0,		// SCONTEXT_ELSE
	1,		// SCONTEXT_DO
	1,		// SCONTEXT_WHILE
	1,		// SCONTEXT_UNTIL
	1,		// SCONTEXT_SWITCH
	1		// SCONTEXT_LOOP
};

static qboolean BreakAllowed[] =
{
	false,	// SCONTEXT_FUNCTION
	false,	// SCONTEXT_IF
	false,	// SCONTEXT_ELSE
	true,	// SCONTEXT_DO
	true,	// SCONTEXT_WHILE
	true,	// SCONTEXT_UNTIL
	true,	// SCONTEXT_SWITCH
	true	// SCONTEXT_LOOP
};

static qboolean ContinueAllowed[] =
{
	false,	// SCONTEXT_FUNCTION
	false,	// SCONTEXT_IF
	false,	// SCONTEXT_ELSE
	true,	// SCONTEXT_DO
	true,	// SCONTEXT_WHILE
	true,	// SCONTEXT_UNTIL
	false,	// SCONTEXT_SWITCH
	true	// SCONTEXT_LOOP
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// ST_ParseStatement
//
//==========================================================================

void ST_ParseStatement (void)
{
	CaseIndex = 0;
	BreakIndex = 0;
	ContinueIndex = 0;
	StatementIndex = 0;
	ContextLevel = 0;
	ParseStatement(SCONTEXT_FUNCTION);
}

//==========================================================================
//
// ParseStatement
//
//==========================================================================

static void ParseStatement (scontext_t owner)
{
	if (StatementIndex == MAX_STATEMENT_DEPTH)
	{
		PR_ParseError("statement overflow");
	}
	ContextHistory[StatementIndex++] = owner;

	if (TK_CHECK(TK_LBRACE))
	{
		ContextLevel += EnterContext[owner];
		do
		{
			ParseStatement(owner);
		} while (!TK_CHECK(TK_RBRACE));

		ContextLevel -= EnterContext[owner];
		StatementIndex--;
		return;
	}

	if (TK_CHECK(TK_SEMICOLON))
	{
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("return"))
	{
		ParseReturn();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("loop"))
	{
		ParseLoop();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("while"))
	{
		ParseWhile();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("until"))
	{
		ParseUntil();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("do"))
	{
		ParseDo();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("switch"))
	{
		ParseSwitch();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("case"))
	{
		if (owner != SCONTEXT_SWITCH)
		{
			PR_ParseError("misplaced case");
		}
		ParseCase();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("break"))
	{
		if (BreakAncestor() == false)
		{
			PR_ParseError("misplaced break");
		}
		ParseBreak();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("continue"))
	{
		if (ContinueAncestor() == false)
		{
			PR_ParseError("misplaced continue");
		}
		ParseContinue();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("default"))
	{
		ParseDefault();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("thinktime"))
	{
		ParseThinktime();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("local"))
	{
		ParseLocalDefs();
		StatementIndex--;
		return;
	}
	if (LX_Check("float") || LX_Check("vector")
		|| LX_Check("entity") || LX_Check("string")
		|| LX_Check("void"))
	{
		ParseLocalDefs();
		StatementIndex--;
		return;
	}
	if (LX_CheckFetch("if"))
	{
		ParseIf();
		StatementIndex--;
		return;
	}

	EX_Expression(TOP_PRIORITY);
	LX_Require(";");
	StatementIndex--;
}

//==========================================================================
//
// ParseReturn
//
//==========================================================================

static void ParseReturn (void)
{
	def_t	*e;

	//if (TK_CHECK(TK_SEMICOLON))
	if (pr_tokenclass == TK_SEMICOLON)
	{
		if (st_ReturnType->type != ev_void)
		{
			PR_ParseError("missing return value");
		}
		CO_GenCode(&pr_opcodes[OP_RETURN], NULL, NULL);
		LX_Fetch();
		return;
	}
	e = EX_Expression(TOP_PRIORITY);
	if (e->type != st_ReturnType)
	{
		PR_ParseError("return type mismatch");
	}
	LX_Require(";");
	CO_GenCode(&pr_opcodes[OP_RETURN], e, NULL);
	st_ReturnParsed = true;
}

//==========================================================================
//
// ParseLoop
//
//==========================================================================

static void ParseLoop (void)
{
	dstatement_t	*patch1;
	def_t	tempDef;
	int	contStatement;

	contStatement = numstatements;
	patch1 = &statements[numstatements];
	ParseStatement(SCONTEXT_LOOP);
	tempDef.ofs = patch1 - &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_GOTO], &tempDef, NULL);
	FixContinues(contStatement);
	FixBreaks();
}

//==========================================================================
//
// ParseWhile
//
//==========================================================================

static void ParseWhile (void)
{
	def_t	*e;
	dstatement_t	*patch1, *patch2;
	def_t	tempDef;
	int	contStatement;

	LX_Require("(");
	contStatement = numstatements;
	patch2 = &statements[numstatements];
	e = EX_Expression(TOP_PRIORITY);
	LX_Require(")");
	LX_CheckFetch("do");
	patch1 = &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_IFNOT], e, NULL);
	ParseStatement(SCONTEXT_WHILE);
	tempDef.ofs = patch2 - &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_GOTO], &tempDef, NULL);
	patch1->b = &statements[numstatements] - patch1;
	FixContinues(contStatement);
	FixBreaks();
}

//==========================================================================
//
// ParseUntil
//
//==========================================================================

static void ParseUntil (void)
{
	def_t	*e;
	dstatement_t	*patch1, *patch2;
	def_t	tempDef;
	int	contStatement;

	LX_Require("(");
	contStatement = numstatements;
	patch2 = &statements[numstatements];
	e = EX_Expression(TOP_PRIORITY);
	LX_Require(")");
	LX_CheckFetch("do");
	patch1 = &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_IF], e, NULL);
	ParseStatement(SCONTEXT_UNTIL);
	tempDef.ofs = patch2 - &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_GOTO], &tempDef, NULL);
	patch1->b = &statements[numstatements] - patch1;
	FixContinues(contStatement);
	FixBreaks();
}

//==========================================================================
//
// ParseDo
//
//==========================================================================

static void ParseDo (void)
{
	def_t	*e;
	dstatement_t	*patch1;
	int	ifOpcode;
	def_t	tempDef;
	int	contStatement;

	patch1 = &statements[numstatements];
	ParseStatement(SCONTEXT_DO);
	if (LX_CheckFetch("until"))
	{
		ifOpcode = OP_IFNOT;
	}
	else
	{
		LX_Require("while");
		ifOpcode = OP_IF;
	}
	LX_Require("(");
	contStatement = numstatements;
	e = EX_Expression(TOP_PRIORITY);
	LX_Require(")");
	LX_Require(";");
	tempDef.ofs = patch1 - &statements[numstatements];
	CO_GenCode(&pr_opcodes[ifOpcode], e, &tempDef);
	FixContinues(contStatement);
	FixBreaks();
}

//==========================================================================
//
// ParseIf
//
//==========================================================================

static void ParseIf (void)
{
	def_t	*e;
	dstatement_t	*patch1, *patch2;
	int	ifOpcode;

	if (LX_CheckFetch("not"))
	{
		ifOpcode = OP_IF;
	}
	else
	{
		ifOpcode = OP_IFNOT;
	}

	LX_Require("(");
	e = EX_Expression(TOP_PRIORITY);
	LX_Require(")");

	patch1 = &statements[numstatements];
	CO_GenCode(&pr_opcodes[ifOpcode], e, NULL);

	ParseStatement(SCONTEXT_IF);

	if (LX_CheckFetch("else"))
	{
		patch2 = &statements[numstatements];
		CO_GenCode(&pr_opcodes[OP_GOTO], NULL, NULL);
		patch1->b = &statements[numstatements] - patch1;
		ParseStatement(SCONTEXT_ELSE);
		patch2->a = &statements[numstatements] - patch2;
	}
	else
	{
		patch1->b = &statements[numstatements] - patch1;
	}
}

//==========================================================================
//
// ParseLocalDefs
//
//==========================================================================

static void ParseLocalDefs (void)
{
	CO_ParseDefs();
	locals_end = numpr_globals;
}

//==========================================================================
//
// ParseSwitch
//
//==========================================================================

static void ParseSwitch (void)
{
	int		i;
	def_t	*e;
	int		count;
	int		opcode;
	def_t	tempDef;
	caseInfo_t	*cInfo;
	dstatement_t	*patch;
	int	defaultStatement;
	etype_t	switchType;

	LX_Require("(");
	e = EX_Expression(TOP_PRIORITY);
	LX_Require(")");
	switchType = e->type->type;
	switch (switchType)
	{
	case ev_float:
		opcode = OP_SWITCH_F;
		break;
	case ev_string:
		opcode = OP_SWITCH_S;
		break;
	case ev_vector:
		opcode = OP_SWITCH_V;
		break;
	case ev_entity:
		opcode = OP_SWITCH_E;
		break;
	case ev_function:
		opcode = OP_SWITCH_FNC;
		break;
	default:
		PR_ParseError("bad type for switch");
		return; /* silence compiler */
	}
	patch = &statements[numstatements];
	CO_GenCode(&pr_opcodes[opcode], e, NULL);
	ParseStatement(SCONTEXT_SWITCH);

	// Switch opcode fixup
	patch->b = &statements[numstatements]-patch;

	if (statements[numstatements-1].op != OP_GOTO)
	{ // Implicit break
		patch->b++; /* because we are generating an additional op */
		patch = &statements[numstatements];
		CO_GenCode(&pr_opcodes[OP_GOTO], NULL, NULL);
	}
	else
	{
		patch = NULL;
	}

	count = GetCaseInfo(&cInfo);
	if (count == 0)
	{
		PR_ParseError("switch has no case");
	}
	defaultStatement = -1;
	for (i = 0; i < count; i++, cInfo++)
	{
		if (cInfo->isDefault == true)
		{
			defaultStatement = cInfo->statement;
			continue;
		}
		if (cInfo->type != switchType)
		{
			PR_ParseError("type mismatch within switch");
		}
		tempDef.ofs = &statements[cInfo->statement]
				-&statements[numstatements];
		if (cInfo->value2 == NULL)
		{
			CO_GenCodeDirect(&pr_opcodes[OP_CASE],
					cInfo->value1, &tempDef, NULL);
		}
		else
		{
			CO_GenCodeDirect(&pr_opcodes[OP_CASERANGE],
					cInfo->value1, cInfo->value2, &tempDef);
		}
	}
	if (defaultStatement != -1)
	{
		tempDef.ofs = &statements[defaultStatement]
				-&statements[numstatements];
		CO_GenCode(&pr_opcodes[OP_GOTO], &tempDef, NULL);
	}

	if (patch != NULL)
	{ // Implicit break fixup
		patch->a = &statements[numstatements]-patch;
	}

	FixBreaks();
}

//==========================================================================
//
// ParseCase
//
//==========================================================================

static void ParseCase (void)
{
	def_t	*e;
	def_t	*e2;

	do
	{
		e = EX_Expression(TOP_PRIORITY);
		if (TK_CHECK(TK_RANGE))
		{
			e2 = EX_Expression(TOP_PRIORITY);
			if (e->type->type != ev_float || e2->type->type != ev_float)
			{
				PR_ParseError("type mismatch for case range");
			}
		}
		else
		{
			e2 = NULL;
		}
		AddCase(e->type->type, e, e2, false);
	} while (TK_CHECK(TK_COMMA));

	LX_Require(":");
}

//==========================================================================
//
// AddCase
//
//==========================================================================

static void AddCase(etype_t type, def_t *value1, def_t *value2, qboolean isDefault)
{
	if (CaseIndex == MAX_CASE)
	{
		PR_ParseError("case overflow");
	}
	CaseInfo[CaseIndex].level = ContextLevel;
	CaseInfo[CaseIndex].value1 = value1;
	CaseInfo[CaseIndex].value2 = value2;
	CaseInfo[CaseIndex].isDefault = isDefault;
	CaseInfo[CaseIndex].statement = numstatements;
	CaseInfo[CaseIndex].type = type;
	CaseIndex++;
}

//==========================================================================
//
// GetCaseInfo
//
//==========================================================================

static int GetCaseInfo (caseInfo_t **info)
{
	int		i;
	int		count;

	i = CaseIndex;
	while (i > 0 && CaseInfo[i-1].level > ContextLevel)
	{
		i--;
	}
	*info = &CaseInfo[i];
	count = CaseIndex-i;
	CaseIndex = i;
	return count;
}

//==========================================================================
//
// ParseBreak
//
//==========================================================================

static void ParseBreak (void)
{
	LX_Require(";");
	AddBreak();
}

//==========================================================================
//
// AddBreak
//
//==========================================================================

static void AddBreak (void)
{
	if (BreakIndex == MAX_BREAK)
	{
		PR_ParseError("break overflow");
	}
	BreakInfo[BreakIndex].level = ContextLevel;
	BreakInfo[BreakIndex].patch = &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_GOTO], NULL, NULL);
	BreakIndex++;
}

//==========================================================================
//
// FixBreaks
//
//==========================================================================

static void FixBreaks (void)
{
	if (BreakIndex == 0)
	{
		return;
	}
	while (BreakInfo[BreakIndex-1].level > ContextLevel)
	{
		BreakIndex--;
		BreakInfo[BreakIndex].patch->a =
				&statements[numstatements]-BreakInfo[BreakIndex].patch;
		if (BreakIndex == 0)
		{
			return;
		}
	}
}

//==========================================================================
//
// BreakAncestor
//
//==========================================================================

static qboolean BreakAncestor (void)
{
	int		i;

	for (i = 0; i < StatementIndex; i++)
	{
		if (BreakAllowed[ContextHistory[i]])
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
// ParseDefault
//
//==========================================================================

static void ParseDefault (void)
{
	LX_Require(":");
	AddCase(ev_void, NULL, NULL, true);
}

//==========================================================================
//
// ParseContinue
//
//==========================================================================

static void ParseContinue (void)
{
	LX_Require(";");
	AddContinue();
}

//==========================================================================
//
// AddContinue
//
//==========================================================================

static void AddContinue (void)
{
	if (ContinueIndex == MAX_CONTINUE)
	{
		PR_ParseError("continue overflow");
	}
	ContinueInfo[ContinueIndex].level = ContextLevel;
	ContinueInfo[ContinueIndex].patch = &statements[numstatements];
	CO_GenCode(&pr_opcodes[OP_GOTO], NULL, NULL);
	ContinueIndex++;
}

//==========================================================================
//
// FixContinues
//
//==========================================================================

static void FixContinues (int statement)
{
	if (ContinueIndex == 0)
	{
		return;
	}
	while (ContinueInfo[ContinueIndex-1].level > ContextLevel)
	{
		ContinueIndex--;
		ContinueInfo[ContinueIndex].patch->a =
				&statements[statement]-ContinueInfo[ContinueIndex].patch;
		if (ContinueIndex == 0)
		{
			return;
		}
	}
}

//==========================================================================
//
// ContinueAncestor
//
//==========================================================================

static qboolean ContinueAncestor (void)
{
	int		i;

	for (i = 0; i < StatementIndex; i++)
	{
		if (ContinueAllowed[ContextHistory[i]])
		{
			return true;
		}
	}
	return false;
}

//==========================================================================
//
// ParseThinktime
//
//==========================================================================

static void ParseThinktime (void)
{
	def_t	*expr1;
	def_t	*expr2;

	expr1 = EX_Expression(TOP_PRIORITY);
	LX_Require(":");
	expr2 = EX_Expression(TOP_PRIORITY);
	if (expr1->type->type != ev_entity || expr2->type->type != ev_float)
	{
		PR_ParseError("type mismatch for thinktime");
	}
	LX_Require(";");
	CO_GenCode(&pr_opcodes[OP_THINKTIME], expr1, expr2);
}
