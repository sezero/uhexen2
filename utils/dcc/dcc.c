/*
 * dcc.c -- An hcode compiler/decompiler for Hexen II by Eric Hobbs
 * $Id$
 *
 * Copyright (C) 1996-1997 Id Software, Inc.
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


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "util_io.h"
#include "qcc.h"
#include "dcc.h"
#include "q_endian.h"

// MACROS ------------------------------------------------------------------

#define	MAX_DEC_FILES	512

// TYPES -------------------------------------------------------------------

struct flowinfo_t
{
	int	if_cnt;
	int	do_cnt;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

/* IMPORTANT NOTE about functions taking a gofs_t argument:  If you pass a
 * dstatement_t->a, b or c (signed short) value to them and the function
 * compares that value to a ddef_t->ofs value (unsigned short), then you
 * must explicitly cast that dstatement_t->a, b or c to unsigned short.
 * BackBuildReturnType() is an exception, because it compares its gofs_t
 * argument to dstatement_t->a, b or c values, therefore you must not do
 * that casting for that case.
 */
static void	Make_Immediate (gofs_t ofs, const char *s);
static char	*Get_Immediate (gofs_t ofs);
static void	Clear_Immediates (void);
static void	PR_Indent (void);
static void	PR_FunctionHeader (dfunction_t *df);
static void	PR_Print (const char *s,...) __attribute__((__format__(__printf__,1,2)));
static const char *PR_PrintGlobal (gofs_t ofs, def_t* typ);
static ddef_t	*PR_GetField (const char *name, ddef_t*);
static int	DEC_GetFunctionIdxByName (const char *name);
static void	PR_LocalGlobals (void);
static int	DEC_AlreadySeen (const char *fname);
static ddef_t	*DEC_GetParameter (gofs_t ofs);
static const char *GetFieldFunctionHeader (const char *s_name);
static void	DccStatement (dstatement_t *s);
static void	AddProgramFlowInfo (dfunction_t *df);
static void	PR_Locals (dfunction_t *df);
static const char * PR_ValueString (etype_t type, void *val);
static const char *DCC_ValueString (etype_t type, void *val);
		/* same as PR_ValueString(), differing
		 * in float/vector print precision.  */
static unsigned short	GetReturnType (int func);
static unsigned short	BackBuildReturnType (dfunction_t *df, dstatement_t *dsf, gofs_t ofs);
static unsigned short	GetType (gofs_t ofs);
static unsigned short	GetLastFunctionReturn (dfunction_t *df, dstatement_t *ds);

static void	Init_Dcc (void);
static void	DEC_ReadData (const char *srcfile);
static void	Dcc_Functions (void);
static void	FindBuiltinParameters (int func);
static void	DccFunctionOP (unsigned short op);
static void	PR_PrintFunction (const char *name);
static def_t	*PR_DefForFieldOfs (gofs_t ofs);
static const char *PR_String (const char *string);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const char *type_names[8] =
{
	"void",
	"string",
	"float",
	"vector",
	"entity",
	"ev_field",
	"void()",
	"ev_pointer"
};

static dprograms_t	*progs;
static dfunction_t	*pr_functions;
static ddef_t		*pr_globaldefs;
static ddef_t		*pr_fielddefs;
static dstatement_t	*pr_statements;
static char		*pr_strings;
static int		progs_length;

static dfunction_t	*cfunc;
static struct flowinfo_t	*flowinfo;

static qboolean		FILE_NUM_FOR_NAME;
static qboolean		pr_dumpasm;
static qboolean		printassign;

static FILE	*PR_FILE;
static char	**func_headers;
static char	**temp_val;

static int	lindent;
static char	**DEC_FilesSeen;
static int	DEC_FileCtr;

/* avoid infinite recursion in GetFieldFunctionHeader() which has
 * thankfully only one caller, PR_LocalGlobals(). this solution
 * is naive, cheesy, [insert more here.. or find a better one...] */
#define GFFH_MAX_DEPTH	64
static int	GFFH_depth = 0;

// CODE --------------------------------------------------------------------

static const char *PR_PrintStringAtOfs (gofs_t ofs, def_t* typ)
{
	int		i;
	ddef_t	*def = NULL;
	ddef_t	*d = NULL;

	for (i = 0; i < progs->numglobaldefs; i++)
	{
		d = &pr_globaldefs[i];
		if (d->ofs == ofs)
		{
			def = d;
			break;
		}
	}

	if (!def)
		return Get_Immediate(ofs);

	if (!strcmp(pr_strings + def->s_name, IMMEDIATE_NAME))
		return DCC_ValueString ((etype_t)def->type, &pr_globals[ofs]);

	if (typ)
	{
	//	printf("type %s %d\n", pr_strings + def->s_name, typ->type->type);
		if (typ->type->type == ev_float && d->type == ev_vector)
			def = &pr_globaldefs[i + 1];
	}
//	printf("found %s\n", pr_strings + def->s_name);
	return (pr_strings + def->s_name);
}

static const char *PR_PrintGlobal (gofs_t ofs, def_t* typ)
{
	int		i;
	ddef_t	*def = NULL;
	ddef_t	*d = NULL;

	for (i = 0; i < progs->numglobaldefs; i++)
	{
		d = &pr_globaldefs[i];
		if (d->ofs == ofs)
		{
			def = d;
			break;
		}
	}

	if (!def)
		return NULL;

	if (!strcmp(pr_strings + def->s_name, IMMEDIATE_NAME))
		return DCC_ValueString ((etype_t)def->type, &pr_globals[ofs]);

	if (typ)
	{
		if (typ->type->type == ev_float && d->type == ev_vector)
			def = &pr_globaldefs[i + 1];
	}
	return (pr_strings + def->s_name);
}


static void DccStatement (dstatement_t *s)
{
	static char	dsline[2048], funcname[512];
	const char	*arg1, *arg2, *arg3;
	char		a1[1024], a2[1024], a3[1024];
	int		nargs, i, j;
	dstatement_t	*t, *k;
	int		doc, ifc;
	unsigned short	tom;
	def_t		*typ1 = NULL, *typ2 = NULL, *typ3 = NULL;
	ddef_t		*par;

	a1[0] = a2[0] = a3[0] = dsline[0] = funcname[0] = '\0';

	doc = flowinfo[s - pr_statements].do_cnt;
	ifc = flowinfo[s - pr_statements].if_cnt;

/* use program flow information */
	for (i = 0; i < ifc; i++)
	{
		PR_Print("\n");
		lindent--;
		PR_Indent();
		PR_Print("}\n");
	//	printf("entered program flow\n");
	}

	for (i = 0; i < doc; i++)
	{
		PR_Indent();
		PR_Print("do {\n\n");
		lindent++;
	}

	typ1 = pr_opcodes[s->op].type_a;
	typ2 = pr_opcodes[s->op].type_b;
	typ3 = pr_opcodes[s->op].type_c;

	if (pr_dumpasm)
	{
		arg1 = PR_PrintGlobal((unsigned short)s->a, typ1);
		arg2 = PR_PrintGlobal((unsigned short)s->b, typ2);
		arg3 = PR_PrintGlobal((unsigned short)s->c, typ3);
		PR_Print("\n%s(%d): %s(%d) %s(%d) %s(%d):\n",
			  pr_opcodes[s->op].opname, s->op,
			  arg1, s->a, arg2, s->b, arg3, s->c);
	}

	arg1 = arg2 = arg3 = NULL;

	if (s->op == OP_DONE)
	{
		lindent--;
		PR_Indent();
		PR_Print("};\n\n");
		return;
	}
	else if (s->op == OP_STATE)
	{
	}
	else if (s->op == OP_RETURN)
	{
		PR_Indent();
		PR_Print("return ");

		if (s->a)
		{
			arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
			PR_Print("( %s )", arg1);
		}

		PR_Print(";\n");
	}
	else if ((OP_MUL_F <= s->op && s->op <= OP_SUB_V) ||
		 (OP_EQ_F  <= s->op && s->op <= OP_GT   ) ||
		 (OP_AND   <= s->op && s->op <= OP_BITOR))
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintGlobal((unsigned short)s->c, typ3);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s %s %s;\n", arg3, a1, pr_opcodes[s->op].name, a2);
		}
		else
		{
			sprintf(dsline, "(%s %s %s)", a1, pr_opcodes[s->op].name, a2);
			Make_Immediate((unsigned short)s->c, dsline);
		}
	}
	else if (OP_LOAD_F <= s->op && s->op <= OP_ADDRESS)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintGlobal((unsigned short)s->c, typ3);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s.%s;\n", arg3, a1, a2);
		}
		else
		{
			sprintf(dsline, "%s.%s", a1, a2);
		//	printf("%s.%s making immediate at %d\n",a1,a2,s->c);
			Make_Immediate((unsigned short)s->c, dsline);
		}
	}
	else if ((OP_STORE_F <= s->op) && (s->op <= OP_STORE_FNC))
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1,arg1);

		/*
		par = DEC_GetParameter ((unsigned short)s->a);
		if (par && s->op == OP_STORE_F)
		{
			if (par->type == ev_vector)
				strcat(a1, "_x");
		}
		*/
		arg3 = PR_PrintGlobal((unsigned short)s->b, typ2);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s;\n", arg3, a1);
		}
		else
		{
			sprintf(dsline,"%s", a1);
			Make_Immediate((unsigned short)s->b, dsline);
		}
	}
	else if ((OP_STOREP_F <= s->op) && (s->op <= OP_STOREP_FNC))
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		par = DEC_GetParameter ((unsigned short)s->a);
		if (par && s->op == OP_STOREP_F)
		{
			if (par->type == ev_vector)
				strcat(a1, "_x");
		}

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);

		PR_Indent();
		PR_Print("%s = %s;\n", arg2, a1);
	}
	else if ((OP_NOT_F <= s->op) && (s->op <= OP_NOT_FNC))
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		sprintf(dsline, "!%s", arg1);
		Make_Immediate((unsigned short)s->c, dsline);
	}
	else if ((OP_CALL0 <= s->op) && (s->op <= OP_CALL8))
	{
		nargs = s->op - OP_CALL0;

		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);
	//	printf("fname: %s %s\n", a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintStringAtOfs((unsigned short)s->c, typ3);
		sprintf(dsline, "%s (", a1);
		sprintf(funcname, "%s", a1);
		if (arg2)
		{
			strcat(dsline, " ");
			strcat(dsline, a2);
			i++;
		}

		if (arg3 && nargs > 1)
		{
			size_t	len = strlen(arg3);
			strcat(dsline, ", ");
			strcat(dsline, arg3);
			if (!strcmp(funcname, "WriteCoord") || !strcmp(funcname, "WriteAngle"))
			{
				if (len >= 2)
				{
					len -= 2;
					if (strcmp(&arg3[len], "_x") &&
						!(!strcmp(&arg3[len], "_y") ||
						  !strcmp(&arg3[len], "_z") ||
						  !strcmp(&arg3[len+1], ")")))
					    strcat(dsline, "_x");
				}
			}
		}

		for (i = 2; i < nargs; i++)
		{
			strcat(dsline, ", ");
			arg2 = Get_Immediate(OFS_PARM0 + (i * 3));
			if (!arg2)
				continue;
			strcat(dsline, arg2);
		}

		strcat(dsline, ")");
		Make_Immediate(OFS_RETURN, dsline);
		j = 1;	/* print now */

		for (i = 1; (s + i)->op; i++)
		{
		//	printf ("\n%d\n", (s + i)->op);
			if ((s + i)->op == OP_DONE)
				break;

			if ((s + i)->op >= OP_RAND0)
				break;

			if ((s + i)->a == OFS_RETURN ||
			    (s + i)->b == OFS_RETURN ||
			    (s + i)->c == OFS_RETURN)
			{
				j = 0;
			//	printf("ofs_return is used before another call %d %d %d\n",
			//		j, i, (s + i)->op);
				break;
			}

			if (OP_CALL0 <= (s + i)->op && (s + i)->op <= OP_CALL8)
			{
			//	if (i == 1)
			//		j = 0;
			//	printf("another call %d  %d %d %d \n",
			//		j, i, (s + i)->a, (s + i)->b);
				break;
			}
		}

		if (j)
		{
			PR_Indent();
			PR_Print("%s;\n", temp_val[OFS_RETURN]);
		}
	}
	else if (s->op == OP_IF || s->op == OP_IFNOT)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		if (s->op == OP_IFNOT)
		{
			if (s->b < 1)
				COM_Error("Found a negative IFNOT jump.");

			/* get instruction right before the target */
			t = s + s->b - 1; /* */
			tom = t->op;

			if (tom != OP_GOTO)
			{
			/* pure if */
				PR_Indent();
				PR_Print("if ( %s ) {\n\n", a1);
				lindent++;
			}
			else
			{
				if (t->a > 0)
				{
				/* ite */
					PR_Indent();
					PR_Print("if ( %s ) {\n\n", a1);
					lindent++;
				}
				else
				{
					if (t->a + s->b > 1)
					{
					/* pure if */
						PR_Indent();
						PR_Print("if ( %s ) {\n\n", a1);
						lindent++;
					}
					else
					{
						int		dum = 1;
						for (k = t + t->a; k < s; k++)
						{
							tom = k->op;
							if (tom == OP_GOTO || tom == OP_IF || tom == OP_IFNOT)
								dum = 0;
						}

						if (dum)
						{
						/* while */
							PR_Indent();
							PR_Print("while ( %s ) {\n\n", a1);
							lindent++;
						}
						else
						{
						/* pure if */
							PR_Indent();
							PR_Print("if ( %s ) {\n\n", a1);
							lindent++;
						}
					}
				}
			}
		}
		else
		{
		/* do ... while */
			if (s->b < 0)
			{
				lindent--;
				PR_Indent();
				PR_Print("} while ( %s );\n", a1);
			}
			else
			{
				PR_Indent();
				PR_Print("if ( !(%s) ) {\n\n", a1);
				lindent++;
			}
		}
	}
	else if (s->op == OP_GOTO)
	{
		if (s->a > 0)
		{
		/* else */
			lindent--;
			PR_Indent();
			PR_Print("} else {\n\n");
			lindent++;
		}
		else
		{
		/* while */
			PR_Print("\n");
			lindent--;
			PR_Indent();
			PR_Print("}\n");
		}
	}
	else if ((s->op >= OP_MULSTORE_F && s->op <= OP_SUBSTOREP_V) ||
		 (s->op >= OP_BITSET && s->op <= OP_BITCLRP))
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		if (arg2)
			sprintf(dsline, "%s", arg2);

		PR_Indent();
		PR_Print("%s %s %s;\n", arg2, pr_opcodes[s->op].name, a1);

		if (s->c)
			Make_Immediate((unsigned short)s->c, dsline);
	}
	else if (s->op == OP_FETCH_GBL_F || s->op == OP_FETCH_GBL_V)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintStringAtOfs((unsigned short)s->c, typ3);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s->%s;\n", arg3, a2, a1);
		}
		else
		{
			sprintf(dsline, "(%s->%s)", a1, a2);
			Make_Immediate((unsigned short)s->c, dsline);
		}
	}
	else if (s->op == OP_CSTATE)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		PR_Indent();
		PR_Print("AdvanceFrame( %s, %s);\n", a1, arg2);
	}
	else if (s->op == OP_THINKTIME)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		PR_Indent();
		PR_Print("AdvanceThinkTime(%s,%s);\n", a1, arg2);
	}
	else if (s->op == OP_RAND0)
	{
		sprintf(dsline,"random()");
		Make_Immediate(OFS_RETURN, dsline);
	}
	else if (s->op == OP_RAND1)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		sprintf(dsline, "random(%s)", a1);
		Make_Immediate(OFS_RETURN, dsline);
	}
	else if (s->op == OP_RAND2)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		sprintf(dsline, "random(%s,%s)", a1, arg2);
		Make_Immediate(OFS_RETURN, dsline);
	}
	else if (s->op == OP_RANDV0)
	{
		sprintf(dsline,"random( )");
		Make_Immediate(OFS_RETURN, dsline);
	}
	else if (s->op == OP_RANDV1)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		sprintf(dsline, "random(%s)", a1);
		Make_Immediate(OFS_RETURN, dsline);
	}
	else if (s->op == OP_RANDV2)
	{
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, typ2);
		sprintf(dsline, "random(%s,%s)", a1, arg2);
		Make_Immediate(OFS_RETURN, dsline);
	}
	else
	{
		PR_Print("\n/* ERROR: UNKNOWN COMMAND */\n OP: %d  ", s->op);
		arg1 = PR_PrintStringAtOfs((unsigned short)s->a, NULL);
		PR_Print("a: %s(%d)", arg1, s->a);
		arg2 = PR_PrintStringAtOfs((unsigned short)s->b, NULL);
		PR_Print(" b: %s(%d)", arg2, s->b);
		arg3 = PR_PrintStringAtOfs((unsigned short)s->c, NULL);
		PR_Print(" c: %s(%d)\n", arg3, s->c);
	}
}

static void Make_Immediate (gofs_t ofs, const char *s)
{
	if (ofs >= progs->numglobals) /* happens with opcodes that DHCC doesn't know, i.e. switches. */
	{
	//	printf ("%s: ofs %d is past numpr_globals %d (cfunc->parm_start: %d, cfunc->locals: %d)\n",
	//			__thisfunc__, ofs, progs->numglobals, cfunc->parm_start, cfunc->locals);
		return;
	}
	if (temp_val[ofs])
		free(temp_val[ofs]);
	temp_val[ofs] = SafeStrdup(s);
}

static char *Get_Immediate (gofs_t ofs)
{
	if (ofs >= progs->numglobals) /* happens with opcodes that DHCC doesn't know, i.e. switches. */
	{
	//	printf ("%s: ofs %d is past numpr_globals %d (cfunc->parm_start: %d, cfunc->locals: %d)\n",
	//			__thisfunc__, ofs, progs->numglobals, cfunc->parm_start, cfunc->locals);
		return NULL;
	}

	return temp_val[ofs];
}

static void Clear_Immediates (void)
{
	int	i;
	for (i = 0; i < progs->numglobals; i++)
	{
		if (temp_val[i])
		{
			free(temp_val[i]);
			temp_val[i] = NULL;
		}
	}
}


static void AddProgramFlowInfo (dfunction_t *df)
{
	dstatement_t	*ds, *ts;
	unsigned short	dom, tom;
	dstatement_t	*k;

	ds = pr_statements + df->first_statement;

	while (1)
	{
		dom = ds->op;
		if (!dom)
		{
			break;
		}
		else if (dom == OP_GOTO)
		{
		/* check for i-t-e */
			if (ds->a > 0)
			{
				ts = ds + ds->a;
				/* mark the end of a if/ite construct */
				flowinfo[ts - pr_statements].if_cnt++;
			}
		}
		else if (dom == OP_IFNOT)
		{
		/* check for pure if */
			ts  = ds + ds->b; /* FIXME: ds->b < 0 possible? */
			tom = (ts - 1)->op;

			if (tom != OP_GOTO)
			{
				/* mark the end of a if/ite construct */
				flowinfo[ts - pr_statements].if_cnt++;
			}
			else
			{
				if ((ts - 1)->a < 0)
				{
					if ((ts - 1)->a + ds->b > 1)
					{ /* pure if */
						/* mark the end of a if/ite construct */
						flowinfo[ts - pr_statements].if_cnt++;
					}
					else
					{
						int	dum = 1;
						for (k = (ts - 1) + (ts - 1)->a; k < ds; k++)
						{
							tom = k->op;
							if (tom == OP_GOTO || tom == OP_IF || tom == OP_IFNOT)
								dum = 0;
						}
						if (!dum)
						{ /* pure if */
							/* mark the end of a if/ite construct */
							flowinfo[ts - pr_statements].if_cnt++;
						}
					}
				}
			}
		}
		else if (dom == OP_IF)
		{
			ts = ds + ds->b;
			if (ds->b < 0)
			{
				/* mark the start of a do construct */
				flowinfo[ts - pr_statements].do_cnt++;
			}
			else
			{
				/* mark the end of a if/ite construct */
				flowinfo[ts - pr_statements].if_cnt++;
			}
		}

		ds++;
	}
}


static void PR_Indent (void)
{
	int	i;

	if (lindent < 0)
		lindent = 0;

	for (i = 0; i < lindent; i++)
	{
		PR_Print("   ");
	}
}

static void PR_Locals (dfunction_t *df)
{
	int	start, i, j, k;
	ddef_t	*par;

	start = df->parm_start;
	i = df->numparms;

	for (j = k = 0; j < i; j++)
		k += df->parm_size[j];

	start += k;

	for (j = start; j < df->parm_start + df->locals; j++)
	{
		PR_Indent();
		par = DEC_GetParameter (j);
		if (par)
		{
			if (par->type == ev_void  || par->type == ev_void ||
			    par->type == ev_field || par->type == ev_function ||
			    par->type == ev_pointer)
			{
				continue;
			}

			if (!strcmp(IMMEDIATE_NAME, pr_strings + par->s_name))
				continue;

			PR_Print("local ");

			switch (par->type)
			{
			case ev_string:
				PR_Print("string");
				break;
			case ev_void:
				PR_Print("void");
				break;
			case ev_float:
				PR_Print("float");
				break;
			case ev_vector:
				PR_Print("vector");
				j += 2;
				break;
			case ev_entity:
				PR_Print("entity");
				break;
			case ev_field:
				PR_Print("ev_field");
				break;
			case ev_function:
				PR_Print("void()");
				break;
			case ev_pointer:
				PR_Print("ev_pointer");
				break;
			}

			PR_Print(" %s", pr_strings + par->s_name);

			if (par->type == ev_float || par->type == ev_vector)
				PR_Print(" = %s", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs]));

			PR_Print(";\n");
		}
	}
}


static void PR_FunctionHeader (dfunction_t *df)
{
	int	j, start;
	ddef_t	*par;
	char	linetxt[512];
	unsigned short	t1;

	if (func_headers[df - pr_functions])	/* already done */
		return;

	if (df->first_statement < 0)
	{ /* builtin */
		FindBuiltinParameters(df - pr_functions);
		return;
	}

	/* get return type */
	linetxt[0] = '\0';
	t1 = GetReturnType(df - pr_functions);

	switch (t1)
	{
	case ev_string:
		sprintf(linetxt, "string ");
		break;
	case ev_void:
		sprintf(linetxt, "void ");
		break;
	case ev_float:
		sprintf(linetxt, "float ");
		break;
	case ev_vector:
		sprintf(linetxt, "vector ");
		break;
	case ev_entity:
		sprintf(linetxt, "entity ");
		break;
	case ev_field:
		sprintf(linetxt, "ev_field ");
		break;
	case ev_function:
		sprintf(linetxt, "void() ");
		break;
	case ev_pointer:
		sprintf(linetxt, "ev_pointer ");
		break;
	default:
		sprintf(linetxt, "void ");
		break;
	}

	/* print parameters */
	strcat(linetxt, " (");

	start = df->parm_start;

	for (j = 0; j < df->numparms; j++)
	{
		par = DEC_GetParameter (start);

		if (par)
		{
			switch (par->type)
			{
			case ev_string:
				strcat(linetxt, "string ");
				break;
			case ev_void:
				strcat(linetxt, "void ");
				break;
			case ev_float:
				strcat(linetxt, "float ");
				break;
			case ev_vector:
				strcat(linetxt, "vector ");
				break;
			case ev_entity:
				strcat(linetxt, "entity ");
				break;
			case ev_field:
				strcat(linetxt, "ev_field ");
				break;
			case ev_function:
				strcat(linetxt, "void() ");
				break;
			case ev_pointer:
				strcat(linetxt, "ev_pointer ");
				break;
			}

			strcat(linetxt, pr_strings + par->s_name);
		}
		else
		{
			sprintf(linetxt, "void unknown ");
		}

		if (j < (df->numparms - 1))
			strcat(linetxt, ",");

		start += df->parm_size[j];
	}

	strcat(linetxt, ")");
	func_headers[df - pr_functions] = SafeStrdup(linetxt);
}


static void PR_Print (const char *s,...)
{
	va_list argptr;

	va_start (argptr, s);
	vfprintf (PR_FILE, s, argptr);
	va_end (argptr);
}

static unsigned short GetReturnType (int func)
{
	int		i, j;
	ddef_t		*par = NULL;
	dstatement_t	*ds, *di;
	dfunction_t	*df;
	const char	*arg1;
	unsigned short	rtype[2] = {ev_void, ev_void};
	def_t		*type1;

	if (func == 0)
		return ev_void;

	df = pr_functions + func;
	ds = pr_statements + df->first_statement;

	if (df->first_statement < 0)
	{
	/* ??? */
	}

	j = 0;
	while (1)
	{
		if (j > 1)
			break;

		if (ds->op == OP_DONE)
			break;

		if (ds->op == OP_RETURN)
		{ /* find 2 different returns if possible just to be certain (ie: findtarget) */
			j++; /* we do come here with j == 1 already.  put j > 1 checks below. */
			if (ds->a != 0)
			{
				if (ds->a == OFS_RETURN)
				{
					di = ds - 1;
					while ((di - pr_statements) >= df->first_statement)
					{	/* that stupid equal, what a bitch */
						if (OP_CALL0 <= di->op && di->op <= OP_CALL8)
						{
							type1 = pr_opcodes[di->op].type_a;
							arg1 = PR_PrintStringAtOfs((unsigned short)di->a, type1);
							if (!arg1)
								COM_Error("function name not found!!!\n");
							i = DEC_GetFunctionIdxByName(arg1);
							if (i == 0)
								break;
							if (i != func)
							{
								if (j < 2)
									rtype[j] = GetReturnType(i);
								/* else: array out of bounds */
								break;
							}
							else
							{
								j--;
								break;
							}
						} /* end if call */
						else if (OP_RAND0 <= di->op && di->op <= OP_RAND2)
						{
							if (j < 2)
								rtype[j] = ev_float;
							/* else: array out of bounds */
							break;
						}
						else if (OP_RANDV0 <= di->op && di->op <= OP_RANDV2)
						{
							if (j < 2)
								rtype[j] = ev_vector;
							/* else: array out of bounds */
							break;
						}

						di--;
					} /* end while ofs_return */
				}
				else
				{
					if (j > 1)
						break;	/* array out of bounds */
					par = DEC_GetParameter ((unsigned short)ds->a);
					if (par)
					{
						rtype[j] = par->type;
					}
					else
					{
					/* find the op where the reg was created,
					 * it can tell what the val is */
						rtype[j] = pr_opcodes[ds->op].type_a->type->type;
						if (rtype[j] == ev_pointer)
						{
							rtype[j] = BackBuildReturnType(df, ds, ds->a);
						}
					}
				}
			}
			else
			{
				if (j > 1)
					break;	/* array out of bounds */
				rtype[j] = ev_void;
			}
		}

		ds++;
	}

	if ((rtype[0] != rtype[1]) && (rtype[0] == ev_void))
		rtype[0] = rtype[1];

	return rtype[0];
}

#if 0	/* this is unused */
static unsigned short OP_StoreValue (dstatement_t *ds)
{
	switch (ds->op)
	{
	case OP_CALL0:
	case OP_CALL1:
	case OP_CALL2:
	case OP_CALL3:
	case OP_CALL4:
	case OP_CALL5:
	case OP_CALL6:
	case OP_CALL7:
	case OP_CALL8:
		return ev_function;

	case OP_NOT_V:
	case OP_STORE_V:
	case OP_LOAD_V:
	case OP_NE_V:
	case OP_EQ_V:
	case OP_SUB_V:
	case OP_ADD_V:
	case OP_MUL_V:
	case OP_MUL_FV:
	case OP_MUL_VF:
	case OP_STOREP_V:
		return ev_vector;

	case OP_NE_F:
	case OP_EQ_F:
	case OP_MUL_F:
	case OP_DIV_F:
	case OP_ADD_F:
	case OP_SUB_F:
	case OP_LE:
	case OP_GE:
	case OP_LT:
	case OP_GT:
	case OP_LOAD_F:
	case OP_STORE_F:
	case OP_NOT_F:
	case OP_STOREP_F:
	case OP_IF:
	case OP_IFNOT:
	case OP_STATE:
	case OP_AND:
	case OP_OR:
	case OP_BITAND:
	case OP_BITOR:
		return ev_float;

	case OP_EQ_S:
	case OP_NE_S:
	case OP_LOAD_S:
	case OP_STORE_S:
	case OP_STOREP_S:
	case OP_NOT_S:
		return ev_string;

	case OP_NE_E:
	case OP_EQ_E:
	case OP_LOAD_ENT:
	case OP_STORE_ENT:
	case OP_NOT_ENT:
	case OP_STOREP_ENT:
		return ev_entity;

	case OP_EQ_FNC:
	case OP_NE_FNC:
	case OP_LOAD_FNC:
	case OP_STORE_FNC:
	case OP_STOREP_FNC:
		return ev_function;

	case OP_LOAD_FLD:
	case OP_ADDRESS:
	case OP_STORE_FLD:
	case OP_STOREP_FLD:
		return ev_field;

	case OP_RETURN:
	case OP_NOT_FNC:
	case OP_DONE:
	case OP_GOTO:
		return ev_void;
	}

	return ev_void;
}
#endif	/* end of unused */

static ddef_t *PR_GetField (const char *name, ddef_t *dd)
{
	int	i;
	ddef_t	*d;

	for (i = 1; i < progs->numfielddefs; i++)
	{
		d = &pr_fielddefs[i];
		if (!strcmp(pr_strings + d->s_name, name))
		{
		//	printf("%s %d %d\n", name, dd ? dd->ofs : 0, d ? d->ofs : 0);
			return d;
		}
	}

	return NULL;
}


static ddef_t *PR_FieldIsUnique (ddef_t *dd)
{
	int	i;
	ddef_t	*d;

	for (i = 1; i < progs->numfielddefs; i++)
	{
		d = &pr_fielddefs[i];
		if (d->ofs == dd->ofs)
		{
			if (d->type == ev_vector && dd - 1 == d)
				return dd;
		//	printf("%s %d %d\n", dd ? pr_strings + dd->s_name : "null", dd ? dd->ofs : 0, d ? d->ofs : 0);
			return d;
		}
	}

	return NULL;
}


static void Dcc_Functions (void)
{
	int		i;
	dfunction_t	*df;
	char	fname[1024], *p;
	FILE		*prgs;

	prgs = fopen("progs.src","w");
	if (!prgs)
		COM_Error("unable to open progs.src!!!\n");

	fprintf(prgs, "%s", "progs.dat\n\n");

	for (i = 1; i < progs->numfunctions; i++)
	{
		df = pr_functions + i;

		if (FILE_NUM_FOR_NAME)
			sprintf (fname, "%d.hc", df->s_file);
		else	sprintf (fname, "%s", pr_strings + df->s_file);
		/* unixify the path, */
		p = fname;
		while (*p)
		{
			if (*p == '\\')
				*p = '/';
			++p;
		}
		/* create parent directories, if any */
		CreatePath(fname);

		if (! DEC_AlreadySeen(fname))
		{
			fprintf(prgs, "%s\n", fname);
			printf("decompiling %s\n", fname);
		}

		PR_FILE = fopen(fname, "a+");
		if (PR_FILE == NULL)
		{
			PR_FILE = stdout;
			COM_Error("unable to open %s\n", fname);
		}

		PR_PrintFunction(pr_strings + df->s_name);
		fclose(PR_FILE);
		PR_FILE = stdout;
	}

	fclose(prgs);
}


static int CalcArraySize (int j, int end)
{
	ddef_t	*par;

	if (j == end)
		return 0;

	for (j++; j < end; j++)
	{
		par = DEC_GetParameter (j);
		if (par && strcmp(pr_strings + par->s_name, IMMEDIATE_NAME))
		{
		//	printf("next is %s at %d\n", pr_strings + par->s_name, par->ofs);
			return par->ofs;
		}
	}

//	printf("can not find next %d %d!!!!!!!!!!!!!!!!!!!!\n", j, end);
	return j;
}


static void PR_InitValues (ddef_t *par, int size)
{
	int	j;

//	printf("size %d\n", size);

	if (size > 1)
	{
		PR_Print("{ %s,\n", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs]));
	}
	else
	{
		PR_Print(" %s;\n", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs]));
		return;
	}

	lindent++;

	for (j = 1; j < size; j++)
	{
		PR_Indent();
		if (j < size - 1)
		{
			if (! (j & 0x3))
				PR_Print(" %s,\n", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs + (j*type_size[par->type])]));
			else	PR_Print(" %s,",   DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs + (j*type_size[par->type])]));
		}
		else
		{
			PR_Print(" %s", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs + (j*type_size[par->type])]));
		}
	}

	PR_Print("};\n");
	lindent--;
}


static void PR_LocalGlobals (void)
{
	int		i, j, ps, cnt = 0;
	int		start, end, bsize;
	dfunction_t	*df, *dfpred;
	ddef_t		*par;
//	ddef_t		*par2;
	ddef_t		*ef;
	const char	*arg2;

	printf("finding declared globals:.");

	df = cfunc;
	dfpred = df - 1;

	for (j = 0, ps = 0; j < dfpred->numparms; j++)
		ps += dfpred->parm_size[j];

	start = dfpred->parm_start + dfpred->locals + ps;

	if (dfpred->first_statement < 0 && df->first_statement > 0)
		start -= 1;

	if (start == 0)
		start = 1;

//	printf("starting at %d ending at %d\n", start, end);

	end = df->parm_start;

	for (j = start; j < end; j++)
	{
		par = DEC_GetParameter (j);
	//	par2 = DEC_GetParameter (j+1);
		if (! (j & 0xf))
			printf(".");

		if (!par)
			cnt++;

		if (par/* && strcmp(pr_strings + par->s_name, IMMEDIATE_NAME)*/)
		{
		//	printf("cnt is %d\n", cnt);
			cnt = 0;
			bsize = CalcArraySize(j, end);

			par->type &= ~DEF_SAVEGLOBAL;

		//	printf("%s type: %d ofs: %d par2: %d bsize: %d\n",
		//		  pr_strings + par->s_name, par->type,
		//		  par->ofs, par2 ? (par2->ofs-par->ofs) : -1,
		//		  bsize-par->ofs);

			if (par->type == ev_function)
			{
				if (strcmp(pr_strings + par->s_name, IMMEDIATE_NAME) == 0)
					continue;
				if (strcmp(pr_strings + par->s_name, pr_strings + cfunc->s_name) == 0)
					continue;
				i = DEC_GetFunctionIdxByName(pr_strings + par->s_name);
				/* check for i == 0 here ???? */
				PR_FunctionHeader(pr_functions + i);
				PR_Print("%s%s;\n", func_headers[i],pr_strings + (pr_functions + i)->s_name);
			}
			else if (par->type != ev_pointer)
			{
				if (strcmp(pr_strings + par->s_name, IMMEDIATE_NAME) == 0)
					continue;
				if (par->type == ev_field)
				{
					ef = PR_GetField(pr_strings + par->s_name, par);
					if (!ef)
						COM_Error("Could not locate a field named \"%s\"", pr_strings + par->s_name);
					i = (ef->type & ~DEF_SAVEGLOBAL);
					if (i == ev_vector)
						j += 3;
					if (i == ev_function)
					{
						arg2 = GetFieldFunctionHeader(pr_strings + ef->s_name);
						GFFH_depth = 0;
					//	printf("function .%s %s;\n", arg2, pr_strings + ef->s_name);
						PR_Print(".%s %s;\n", arg2, pr_strings + ef->s_name);
					}
					else
					{
					//	printf("variable %d .%s %s;\n", i, type_names[i], pr_strings + ef->s_name);
						par = PR_FieldIsUnique(ef);
						if (par == ef)
							PR_Print(".%s %s;\n", type_names[i], pr_strings + ef->s_name);
						else
							PR_Print(".%s %s alias %s;\n", type_names[i], pr_strings + ef->s_name, pr_strings + par->s_name);
					}
				}
				else
				{
					bsize = (bsize - par->ofs) / type_size[par->type];
					if (!par->ofs || bsize - 1 < *(int *)&pr_globals[par->ofs - 1])
					{
					//	printf("bsize %d %d\n", bsize, *(int *)&pr_globals[par->ofs - 1]);
						bsize = 1;
					}
				 	if (bsize != 1)
					{
						bsize = *(int *)&pr_globals[par->ofs - 1] + 1;
					}
					if (par->type == ev_vector)
						j += 2;
					if (par->type == ev_entity || par->type == ev_void)
					{
						if (!strcmp(pr_strings + par->s_name, "end_sys_fields"))
							printassign = true;
						PR_Print("%s %s;\n", type_names[par->type], pr_strings + par->s_name);
					}
					else
					{
						if (pr_dumpasm  ||
						    printassign ||
						   !strcmp(pr_strings + par->s_name, "string_null"))
						{
							if (bsize < 2)
								PR_Print("%s %s   = ", type_names[par->type], pr_strings + par->s_name);
							else
								PR_Print("%s %s [%d]   = ", type_names[par->type], pr_strings + par->s_name, bsize);
							PR_InitValues(par, bsize);
						}
						else
						{
							PR_Print("%s %s;\n", type_names[par->type], pr_strings + par->s_name);
						}
					}
				}
			}
		}
	}

	printf("\n");
}


static const char *GetFieldFunctionHeader (const char *s_name)
{
	int		i, j = 0;
	dstatement_t	*d;
//	const char	*arg1;
	const char	*arg2, *arg3;
	def_t		*typ;

	if (++GFFH_depth > GFFH_MAX_DEPTH) /* see below */
	{
	//	printf("\n%s: MAX_DEPTH reached for \"%s\", returning void ()\n",
	//						__thisfunc__, s_name);
		return "void  ()";
	}

	for (i = 1; i < progs->numstatements; i++)
	{
		d = pr_statements + i;

		if (d->op == OP_ADDRESS)
		{
		//	typ  = pr_opcodes[d->op].type_a;
		//	arg1 = PR_PrintGlobal((unsigned short)d->a, typ);
			typ  = pr_opcodes[d->op].type_b;
			arg2 = PR_PrintGlobal((unsigned short)d->b, typ);

			if (arg2)
			{
				if (!strcmp(s_name, arg2))
				{
					if ((d + 1)->op == OP_STOREP_FNC)
					{
						d++;
						typ  = pr_opcodes[d->op].type_a;
						arg3 = PR_PrintGlobal((unsigned short)d->a, typ);
						if (!arg3)
							continue;
						j = DEC_GetFunctionIdxByName(arg3);
						/* check for j == 0 here ???? */
						PR_FunctionHeader(pr_functions + j);
						if (strcmp("void  ()", func_headers[j]))
							return func_headers[j];
						/* if void (), continue checking just to be certain (ie: th_pain) */

						/* NOTE:  When decompiling progs.dat of hcbots, this
						 * results in an infinite recursion below by the function
						 * calling itself with "think" and "th_stand" as the
						 * arguments alternating between each succession,
						 * hence the MAX_DEPTH check at the function's entrance.
						 * I don't have a better solution so far.. - O.S. */
					}

					if ((d + 1)->op == OP_LOAD_FNC && (d + 2)->op == OP_STOREP_FNC)
					{
						d++;
					//	typ  = pr_opcodes[d->op].type_a;
					//	arg1 = PR_PrintGlobal((unsigned short)d->a, typ);
						typ  = pr_opcodes[d->op].type_b;
						arg2 = PR_PrintGlobal((unsigned short)d->b, typ);
						if (!arg2)
							continue;
						if (strcmp(s_name, arg2))
							return GetFieldFunctionHeader(arg2);
					}
				}
			}
		}
	}

//	printf ("returning found statement %5d %s %s\n", i, arg3, func_headers[j]);

	if (j == 0)
		return "void  ()";

	return func_headers[j];
}


static void FindBuiltinParameters (int func)
{
	int		i, j;
	unsigned short	type[9];
	dstatement_t	*ds, *dsf = NULL;
	dfunction_t	*df, *dft = NULL;
	const char	*arg1;
	char		sname[512], plist[512], parm[128];

	if (func_headers[func])
		return;

	df = pr_functions + func;

//	printf("%s %d\n", pr_strings + df->s_name, df->numparms);
	printf("looking for builtin %s...: ", pr_strings + df->s_name);

	for (i = 1; i < progs->numfunctions; i++)
	{
		if (! (i & 0xf))
			printf(".");	/* let'em know its working, not hanging */

		j = (pr_functions + i)->first_statement;
		if (j < 0)
			continue;

		ds = pr_statements + j;

		while (ds && ds->op)
		{
			if (ds->op >= OP_CALL0 && ds->op <= OP_CALL8)
			{
				arg1 = PR_PrintStringAtOfs((unsigned short)ds->a, 0);

				if (arg1)
				{
					if (!strcmp(pr_strings + df->s_name, arg1))
					{
						dsf = ds;
						dft = pr_functions + i;
						printf("\nfound!!\ndetermining parameters\n");
						break;
					}
				}
			}

			arg1 = NULL;
			ds++;
		//	ds = pr_statements + (pr_functions + i)->first_statement + j;
		}

		if (dsf)
			break;
	}

	if (!dsf)
	{
		printf("NOT found!!\nsetting parameters to void\n");
		func_headers[func] = SafeStrdup("void ()");
		return;
	}

	j = ((dsf->op - OP_CALL0));
	type[8] = ev_void;

	for (ds = dsf + 1; ds && ds->op; ds++)
	{
		if ((ds->op >= OP_CALL0 && ds->op <= OP_CALL8) || ds->op >= OP_RAND0)
			break;

		if (ds->a == OFS_RETURN)
		{
		//	printf("%d %d %d %d 1 rt %d %d\n", ds->op, ds->a, ds->b, ds->c, pr_opcodes[ds->op].type_a->type->type, ev_pointer);
			if (ds->c)
			{
				type[8] = pr_opcodes[ds->op].type_c->type->type;
				if (type[8] == ev_pointer)
					type[8] = BackBuildReturnType(dft, ds, ds->c);
			}
			else
			{
				type[8] = pr_opcodes[ds->op].type_b->type->type;
				if (type[8] == ev_pointer)
					type[8] = BackBuildReturnType(dft, ds, ds->b);
			}
			break;
		}

		if (ds->b == OFS_RETURN)
		{
		//	printf("%d %d %d %d 2 rt %d\n", ds->op, ds->a, ds->b, ds->c, pr_opcodes[ds->op].type_b->type->type);
			type[8] = pr_opcodes[ds->op].type_b->type->type;
			if (type[8] == ev_pointer)
				type[8] = BackBuildReturnType(dft, ds, ds->b);
			break;
		}

		if (ds->c == OFS_RETURN)
		{
		//	printf("%d %d %d %d 3 rt %d\n", ds->op, ds->a, ds->b, ds->c, pr_opcodes[ds->op].type_c->type->type);
			type[8] = pr_opcodes[ds->op].type_c->type->type;
			if (type[8] == ev_pointer)
				type[8] = BackBuildReturnType(dft, ds, ds->c);
			break;
		}
	}

	/* find name */
	memset (sname, 0, sizeof(sname));
	arg1 = PR_PrintStringAtOfs((unsigned short)dsf->a, 0);
	sprintf(sname, "%s", arg1);

	/* look for first two parms */
	if (dsf->b == 1)
		type[0] = GetLastFunctionReturn(dft, dsf);
	else	type[0] = GetType((unsigned short)dsf->b);

	if (type[0] == ev_pointer)
		type[0] = BackBuildReturnType(dft, dsf, dsf->b);

	if (dsf->c == 1)
		type[1] = GetLastFunctionReturn(dft, dsf);
	else	type[1] = GetType((unsigned short)dsf->c);

	printf("type b %d %d\n", type[0], dsf->b);
	printf("type c %d %d\n", type[1], dsf->c);
	if (type[1] == ev_pointer)
		type[1] = BackBuildReturnType(dft, dsf, dsf->c);

	/* look in regs for rest of parms */
	i = 2;

	while (i < j)
	{
		if (i > 8)	/* just in case.. */
			COM_Error ("%s (%d): array out of bounds.", __thisfunc__, __LINE__);
		type[i] = ev_void;
		for (ds = dsf; ds - pr_statements >= dft->first_statement; ds--)
		{
			if (ds->a == ((3 * i) + 4))
			{
			//	printf("a ");
				type[i] = GetType((unsigned short)ds->a);
				if (type[i] == ev_pointer)
					type[i] = BackBuildReturnType(dft, ds, ds->a);
				break;
			}

			if (ds->b == ((3 * i) + 4))
			{
			//	printf("b ");
				if (pr_opcodes[ds->op].right_associative)
				{
					type[i] = GetType((unsigned short)ds->a);
					if (type[i] == ev_pointer || type[i] == ev_field)
						type[i] = BackBuildReturnType(dft, ds, ds->a);
				}
				else
				{
					type[i] = GetType((unsigned short)ds->c);
					if (type[i] == ev_pointer || type[i] == ev_field)
						type[i] = BackBuildReturnType(dft, ds, ds->c);
				}
				break;
			}

			if (ds->c == ((3 * i) + 4))
			{
			//	printf("c ");
				type[i] = GetType((unsigned short)ds->a);
				if (type[i] == ev_pointer)
					type[i] = BackBuildReturnType(dft, ds, ds->c);
				break;
			}
		}

	//	printf("type %i: op: %d a:%d b: %d c: %d rt: %d\n", i, ds->op, ds->a, ds->b, ds->c, type[i]);
		i++;
	}

	/* print results to string */
	memset (plist, 0, sizeof(plist));
	if (type[8] & DEF_SAVEGLOBAL)
	{
		sprintf(plist,".");
		type[8] &= ~DEF_SAVEGLOBAL;
	}

	switch (type[8])
	{
	case ev_string:
		strcat(plist, "string ");
		break;
	case ev_void:
		strcat(plist, "void ");
		break;
	case ev_float:
		strcat(plist, "float ");
		break;
	case ev_vector:
		strcat(plist, "vector ");
		break;
	case ev_entity:
		strcat(plist, "entity ");
		break;
	case ev_field:
		strcat(plist, "ev_field ");
		break;
	case ev_function:
		strcat(plist, "void() ");
		break;
	case ev_pointer:
		strcat(plist, "ev_pointer ");
		break;
	default:
		strcat(plist, "unknown ");
		break;
	}

	strcat(plist, "(");
	if (df->numparms < 0)
	{
		strcat(plist, "...");
	}
	else
	{
		for (i = 0; i < j; i++)
		{
			if (type[i] & DEF_SAVEGLOBAL)
			{
				strcat(plist, ".");
				type[i] &= ~DEF_SAVEGLOBAL;
			}

			switch (type[i])
			{
			case ev_string:
				strcat(plist, "string ");
				break;
			case ev_void:
				strcat(plist, "void ");
				break;
			case ev_float:
				strcat(plist, "float ");
				break;
			case ev_vector:
				strcat(plist, "vector ");
				break;
			case ev_entity:
				strcat(plist, "entity ");
				break;
			case ev_field:
				strcat(plist, "ev_field ");
				break;
			case ev_function:
				strcat(plist, "void() ");
				break;
			case ev_pointer:
				strcat(plist, "ev_pointer ");
				break;
			default:
				sprintf(parm, "unknown%d",type[i]);
				strcat(plist, parm);
				break;
			}

			sprintf(parm, "%c", 'a'+i);
			strcat(plist, parm);

			if (i < j - 1)
				strcat(plist, ", ");
		}
	}

	strcat(plist, ")");
	func_headers[func] = SafeStrdup(plist);
	printf("%s%s\nin %s in file %s\n", plist, sname, pr_strings + dft->s_name, pr_strings + dft->s_file);
}


static unsigned short BackBuildReturnType (dfunction_t *df,dstatement_t *dsf, gofs_t ofs)
{
	dstatement_t	*ds;
	unsigned short	rtype = ev_void;
	ddef_t		*par;

//	printf("backbuilding...\n");

	for (ds = dsf - 1; ds - pr_statements >= df->first_statement; ds--)
	{
		if (ds->a == ofs)
		{
			rtype = pr_opcodes[ds->op].type_a->type->type;
			par = DEC_GetParameter ((unsigned short)ds->b);
			if (par)
			{
			//	printf("ds->b type: %d\n", par->type);
				rtype = par->type;
			}
			break;
		}

		if (ds->b == ofs)
		{
			rtype = pr_opcodes[ds->op].type_b->type->type;
			par = DEC_GetParameter ((unsigned short)ds->a);
			if (par)
			{
			//	printf("ds->a type: %d\n", par->type);
				rtype = par->type;
			}
			break;
		}

		if (ds->c == ofs)
		{
			rtype = pr_opcodes[ds->op].type_c->type->type;
		//	rtype = GetType((unsigned short)ds->b);
			break;
		}
	}

	return rtype;
}


static unsigned short GetType (gofs_t ofs)
{
	ddef_t		*par;
	unsigned short	rtype;
	ddef_t		*ef;

	rtype = ev_pointer;
	par = DEC_GetParameter (ofs);

	if (par)
	{
		par->type &= ~DEF_SAVEGLOBAL;

		if (par->type == ev_field)
		{
			ef = PR_GetField(pr_strings + par->s_name, par);
			if (!ef)
				COM_Error("Could not locate a field named \"%s\"", pr_strings + par->s_name);
			rtype = (ef->type | DEF_SAVEGLOBAL);
		}
		else
		{
			rtype = par->type;
		}
	}

	return rtype;
}


static void DccFunctionOP (unsigned short op)
{
	int		i, j = 0;
	dfunction_t	*df;
	dstatement_t	*ds;

	for (i = 1; i < progs->numfunctions; i++)
	{
		df = pr_functions + i;

		if (df->first_statement < 0)
			continue;

		for (ds = &pr_statements[df->first_statement]; ds && ds->op; ds++)
		{
			if (ds->op == op && j++)
			{
				PR_PrintFunction(pr_strings + df->s_name);
				return;
			}
		}
	}
}


static void DEC_ReadData (const char *srcfile)
{
	void		*p;
	int		i;

	progs_length = LoadFile (srcfile, &p);
	progs = (dprograms_t *) p;

	/* byte swap the header */
	for (i = 0; i < (int) sizeof(*progs) / 4; i++)
		((int *)progs)[i] = LittleLong ( ((int *)progs)[i] );

	printf ("read data from %s:\n", srcfile);
	printf ("%10i bytes, version %i, crc: %i\n", progs_length, progs->version, progs->crc);
	printf ("%10i strofs\n", progs->numstrings);
	printf ("%10i numstatements\n", progs->numstatements);
	printf ("%10i numfunctions\n", progs->numfunctions);
	printf ("%10i numglobaldefs\n", progs->numglobaldefs);
	printf ("%10i numfielddefs\n", progs->numfielddefs);
	printf ("%10i numpr_globals\n", progs->numglobals);
	printf ("----------------------------------------\n");

	if (progs->version != PROG_VERSION)
		COM_Error ("%s is of unsupported version (%d, should be %d)", srcfile, progs->version, PROG_VERSION);

	pr_functions = (dfunction_t *)((byte *)progs + progs->ofs_functions);
	pr_globaldefs = (ddef_t *)((byte *)progs + progs->ofs_globaldefs);
	pr_fielddefs = (ddef_t *)((byte *)progs + progs->ofs_fielddefs);
	pr_statements = (dstatement_t *)((byte *)progs + progs->ofs_statements);
	pr_globals = (float *)((byte *)progs + progs->ofs_globals);
	pr_strings = (char *)progs + progs->ofs_strings;

	/* byte swap the lumps */
	for (i = 0; i < progs->numstatements; i++)
	{
		pr_statements[i].op = LittleShort(pr_statements[i].op);
		pr_statements[i].a = LittleShort(pr_statements[i].a);
		pr_statements[i].b = LittleShort(pr_statements[i].b);
		pr_statements[i].c = LittleShort(pr_statements[i].c);
	}

	for (i = 0; i < progs->numfunctions; i++)
	{
		pr_functions[i].first_statement = LittleLong (pr_functions[i].first_statement);
		pr_functions[i].parm_start = LittleLong (pr_functions[i].parm_start);
		pr_functions[i].s_name = LittleLong (pr_functions[i].s_name);
		pr_functions[i].s_file = LittleLong (pr_functions[i].s_file);
		pr_functions[i].numparms = LittleLong (pr_functions[i].numparms);
		pr_functions[i].locals = LittleLong (pr_functions[i].locals);
	}

	for (i = 0; i < progs->numglobaldefs; i++)
	{
		pr_globaldefs[i].type = LittleShort (pr_globaldefs[i].type);
		pr_globaldefs[i].ofs = LittleShort (pr_globaldefs[i].ofs);
		pr_globaldefs[i].s_name = LittleLong (pr_globaldefs[i].s_name);
	}

	for (i = 0; i < progs->numfielddefs; i++)
	{
		pr_fielddefs[i].type = LittleShort (pr_fielddefs[i].type);
	//	if (pr_fielddefs[i].type & DEF_SAVEGLOBAL)
	//		COM_Error ("%s: pr_fielddefs[i].type & DEF_SAVEGLOBAL", __thisfunc__);
		pr_fielddefs[i].ofs = LittleShort (pr_fielddefs[i].ofs);
		pr_fielddefs[i].s_name = LittleLong (pr_fielddefs[i].s_name);
	}

	for (i = 0; i < progs->numglobals; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
}


static void Init_Dcc (void)
{
	int		i;

	def_ret.ofs = OFS_RETURN;
	for (i = 0; i < MAX_PARMS; i++)
		def_parms[i].ofs = OFS_PARM0 + 3*i;

	if (CheckParm("-fix"))		/* fix mangled names */
		FILE_NUM_FOR_NAME = true;
	else	FILE_NUM_FOR_NAME = false;

	if (CheckParm("-dump"))
		pr_dumpasm = true;
	else	pr_dumpasm = false;

	PR_FILE = stdout;
	cfunc = NULL;
	printassign = false;

	DEC_FileCtr = 0;

	func_headers = (char **) SafeMalloc (progs->numfunctions * sizeof(char *));
	temp_val = (char **) SafeMalloc (progs->numglobals * sizeof(char *));
	DEC_FilesSeen = (char **) SafeMalloc (MAX_DEC_FILES * sizeof(char *));
	flowinfo = (struct flowinfo_t *) SafeMalloc (progs->numstatements * sizeof(struct flowinfo_t));
}


static int DEC_GetFunctionIdxByName (const char *name)
{
	int	i;

	for (i = 1; i < progs->numfunctions; i++)
	{
		if (!strcmp (name, pr_strings + pr_functions[i].s_name))
			return i;
	}

	return 0;
}


static ddef_t *DEC_GetParameter (gofs_t ofs)
{
	int		i;
	ddef_t		*def;

	def = NULL;

	for (i = 0; i < progs->numglobaldefs; i++)
	{
		def = &pr_globaldefs[i];
		if (def->ofs == ofs)
			return def;
	}

	return NULL;
}


static int DEC_AlreadySeen (const char *fname)
{
	int		i;
	char		*new1;

	for (i = 0; i < DEC_FileCtr; i++)
	{
		if (!strcmp(fname, DEC_FilesSeen[i]))
			return 1;
	}

	if (DEC_FileCtr >= MAX_DEC_FILES - 1)
		COM_Error("%s: too many source files.", __thisfunc__);

	new1 = SafeStrdup(fname);
	DEC_FilesSeen[DEC_FileCtr++] = new1;

	return 0;
}


#if 0	/* not used */
static void FixFunctionNames (void)
{
	int		i, j;
	dfunction_t	*d;
	char	s[128], *c;

	for (i = 1; i < progs->numfunctions; i++)
	{
		d = &pr_functions[i];
	//	printf ("%s : %s : %i %i (", pr_strings + d->s_file, pr_strings + d->s_name, d->first_statement, d->parm_start);
		s[0] = '\0';
		j = strlen(pr_strings + d->s_file) + 1;
		sprintf (s, "%d.hc", d->s_file);
		c = pr_strings;
		c += d->s_file;
		sprintf (c, "%s", s);
	}
}
#endif	/* end of unused */


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

static const char *PR_ValueString (etype_t type, void *val)
{
	static char	line[256];
	def_t		*def;
	dfunction_t	*f;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", PR_String(pr_strings + *(int *)val));
		break;
	case ev_entity:
		sprintf (line, "entity %i", *(int *)val);
		break;
	case ev_function:
		f = pr_functions + *(int *)val;
		if (!f)
			sprintf (line, "undefined function");
		else
			sprintf (line, "%s()", pr_strings + f->s_name);
		break;
	case ev_field:
		def = PR_DefForFieldOfs ( *(int *)val );
		sprintf (line, ".%s", def->name);
		break;
	case ev_void:
		sprintf (line, "void");
		break;
	case ev_float:
		sprintf (line, "%5.1f", *(float *)val);
		break;
	case ev_vector:
		sprintf (line, "'%5.1f %5.1f %5.1f'", ((float *)val)[0], ((float *)val)[1], ((float *)val)[2]);
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

static const char *DCC_ValueString (etype_t type, void *val)
{
	static char	vsline[256];
	def_t		*def;
	dfunction_t	*f;

	switch (type)
	{
	case ev_string:
		sprintf (vsline, "%s", PR_String(pr_strings + *(int *)val));
		break;
	case ev_entity:
		sprintf (vsline, "entity %i", *(int *)val);
		break;
	case ev_function:
		f = pr_functions + *(int *)val;
		if (!f)
			sprintf (vsline, "undefined function");
		else	sprintf (vsline, "%s()", pr_strings + f->s_name);
		break;
	case ev_field:
		def = PR_DefForFieldOfs (*(int *)val);
		sprintf (vsline, ".%s", def->name);
		break;
	case ev_void:
		sprintf (vsline, "void");
		break;
	case ev_float:
		sprintf (vsline, "%.5f", *(float *)val);
		break;
	case ev_vector:
		sprintf (vsline, "'%.5f %.5f %.5f'", ((float *)val)[0], ((float *)val)[1], ((float *)val)[2]);
		break;
	case ev_pointer:
		sprintf (vsline, "pointer");
		break;
	default:
		sprintf (vsline, "bad type %i", type);
		break;
	}

	return vsline;
}


static void PR_PrintFunction (const char *name)
{
	int		i;
	dstatement_t	*ds;
	dfunction_t	*df;
	const char	*arg1, *arg2;
	def_t		*typ1, *typ2;

	for (i = 0; i < progs->numfunctions; i++)
	{
		if (!strcmp (name, pr_strings + pr_functions[i].s_name))
			break;
	}

	if (i == progs->numfunctions)
		COM_Error ("No function names \"%s\"", name);

	df = pr_functions + i;
	cfunc = df;
	printf("Statements for %s:\n", name);
	ds = pr_statements + df->first_statement;
	Clear_Immediates();
	PR_LocalGlobals();
	PR_FunctionHeader(df);

	if (df->first_statement < 0)
	{
		PR_Print("\n%s%s = #%d;\n", func_headers[df - pr_functions], pr_strings + df->s_name, -df->first_statement);
		return;
	}

	PR_Print("\n%s%s = ", func_headers[df - pr_functions], pr_strings + df->s_name);

	if (ds->op == OP_STATE)
	{
		typ1 = pr_opcodes[ds->op].type_a;
		typ2 = pr_opcodes[ds->op].type_b;
		arg1 = PR_ValueString(typ1->type->type, &pr_globals[(unsigned short)ds->a]);
		arg2 = PR_PrintStringAtOfs((unsigned short)ds->b, typ2);
		PR_Print(" [%s, %s]", arg1, arg2);
	}

	PR_Print(" {\n");
	AddProgramFlowInfo(df);
	PR_Locals(df);
	lindent = 1;

	while (1)
	{
		DccStatement(ds);
		if (!ds->op)
			break;
		ds++;
	}
}


static unsigned short GetLastFunctionReturn (dfunction_t *df, dstatement_t *ds)
{
	dstatement_t	*di;
	int		i;
	const char	*arg1;
	def_t		*type1;

	printf("looking for last return type\n");

	di = ds - 1;

	while ((di - pr_statements) >= df->first_statement)
	{	/* that stupid equal, what a bitch */
		printf(" op %d a: %d b: %d c: %d\n", di->op, di->a, di->b, di->c);

		if (OP_CALL0 <= di->op && di->op <= OP_CALL8)
		{
			type1 = pr_opcodes[di->op].type_a;
			arg1 = PR_PrintStringAtOfs((unsigned short)di->a, type1);
			if (!arg1)
				COM_Error("function name not found!!!\n");

			i = DEC_GetFunctionIdxByName(arg1);
			if (i == 0)
				break;

			if (i != (df - pr_functions))
			{
				i = GetReturnType(i);
				printf("%s %d is found\n", arg1, i);
				return i;
			}
			else
			{
				break;
			}
		} /* end if call */
		else if (OP_RAND0 <= di->op && di->op <= OP_RAND2)
		{
			return ev_float;
		}
		else if (OP_RANDV0 <= di->op && di->op <= OP_RANDV2)
		{
			return ev_vector;
		}

		di--;
	} /* end while ofs_return */

	return ev_void;
}

static void PrintGlobaldefs (void)
{
	int		i;
	ddef_t		*d;

	for (i = 0; i < progs->numglobaldefs; i++)
	{
		d = &pr_globaldefs[i];
		printf ("%5i : (%i) %s\n", d->ofs, d->type, pr_strings + d->s_name);
	}
}

static void PrintPRGlobals (void)
{
	int		i;

	for (i = 0; i < progs->numglobals; i++)
	{
		printf ("%5i %5.5f %5d\n", i, pr_globals[i], ((int *)pr_globals)[i]);
	}
}

static void PrintStatements (void)
{
	int		i;
	dstatement_t	*ds;

	for (i = 0; i < progs->numstatements; i++)
	{
		ds = pr_statements + i;
		printf ("%5d op: %2d a: %5d b: %5d c: %5d\n", i, ds->op, ds->a, ds->b, ds->c);
	}
}

static void PrintStrings (void)
{
	int		i, l, j;

	for (i = 0; i < progs->numstrings; i += l)
	{
		l = strlen(pr_strings+i) + 1;
		printf ("%5i : ", i);
		for (j = 0; j < l; j++)
		{
			if (pr_strings[i+j] == '\n')
			{
				putchar ('\\');
				putchar ('n');
			}
			else
				putchar (pr_strings[i+j]);
		}
		printf ("\n");
	}
}

static void PrintFunctions (void)
{
	int		i, j;
	dfunction_t	*d;

	for (i = 0; i < progs->numfunctions; i++)
	{
		d = &pr_functions[i];
		printf ("%s : %s : %i %i (", pr_strings + d->s_file, pr_strings + d->s_name, d->first_statement, d->parm_start);
		for (j = 0; j < d->numparms; j++)
			printf ("%i ", d->parm_size[j]);
		printf (")\n");
	}
}

static void PrintFields (void)
{
	int		i;
	ddef_t		*d;

	for (i = 0; i < progs->numfielddefs; i++)
	{
		d = &pr_fielddefs[i];
		printf ("%5i %5i : (%i) %s\n", i, d->ofs, d->type, pr_strings + d->s_name);
	}
}

int Dcc_main (int argc, char **argv)
{
	int		p;
	double	start, stop;

	start = COM_GetTime ();

	p = CheckParm("-name");
	if (p == 0)
		DEC_ReadData ("progs.dat");
	else
	{
		if (p >= argc - 1)
			COM_Error ("No input filename specified with -name");
		DEC_ReadData (argv[p + 1]);
	}

	/* don't need Init_Dcc () for the following info stuff */
	if (CheckParm("-fields") != 0)
	{
		printf("\nFIELDS =========================\n");
		PrintFields ();
		return 0;
	}
	if (CheckParm("-functions") != 0)
	{
		printf("\nFUNCTIONS ======================\n");
		PrintFunctions ();
		return 0;
	}
	if (CheckParm("-globaldefs") != 0)
	{
		printf("\nGLOBALDEFS =====================\n");
		PrintGlobaldefs();
		return 0;
	}
	if (CheckParm("-prglobals") != 0)
	{
		printf("\nPR_GLOBALS =====================\n");
		PrintPRGlobals ();
		return 0;
	}
	if (CheckParm("-statements") != 0)
	{
		printf("\nSTATEMENTS =====================\n");
		PrintStatements();
		return 0;
	}
	if (CheckParm("-strings") != 0)
	{
		printf("\nSTRINGS ========================\n");
		PrintStrings ();
		return 0;
	}

	Init_Dcc ();

	p = CheckParm("-bbb");
	if (p != 0)
	{
		/*
		i= -999;
		for (p = 0; p < progs->numstatements; ++p)
		{
			if ((pr_statements + p)->op > i)
				i = (pr_statements + p)->op;
		}
		printf("largest op %d\n", i);
		*/
		FindBuiltinParameters(1);
		return 0;
	}

	p = CheckParm("-ddd");
	if (p != 0)
	{
		for (++p; p < argc; ++p)
		{
			if (argv[p][0] == '-')
				break;
			DccFunctionOP (atoi(argv[p]));
		}
		return 0;
	}

	p = CheckParm("-asm");
	if (p != 0)
	{
		for (++p; p < argc; ++p)
		{
			if (argv[p][0] == '-')
				break;
			PR_PrintFunction(argv[p]);
		}
	}
	else
	{
		Dcc_Functions ();
		stop = COM_GetTime ();
		printf("\n%d seconds elapsed.\n", (int)(stop - start));
	}

	return 0;
}

