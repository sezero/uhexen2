/*
	dcc.c
	An hcode compiler/decompiler for Hexen II by Eric Hobbs

	$Id: dcc.c,v 1.43 2007-11-14 15:53:17 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "util_inc.h"
#include "cmdlib.h"
#include "util_io.h"
#include "qcc.h"
#include "q_endian.h"

// MACROS ------------------------------------------------------------------

#define	MAX_DEC_FILES	1024

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern const char *PR_String (const char *string);
extern def_t	*PR_DefForFieldOfs (gofs_t ofs);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

void		DEC_ReadData (const char *srcfile);
void		Dcc_Functions (void);
void		FindBuiltinParameters (int func);
void		DccFunctionOP (unsigned short op);
void		PR_PrintFunction (const char *name);

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static char	*Make_Immediate (gofs_t ofs, const char *linestr, int mode);
static void	PR_Indent (void);
static void	PR_FunctionHeader (dfunction_t *df);
static void	PR_Print (const char *s,...) __attribute__((format(printf,1,2)));
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
static const char *DCC_ValueString (etype_t type, void *val);
static unsigned short	GetReturnType (int func);
static unsigned short	BackBuildReturnType (dfunction_t *df, dstatement_t *dsf, gofs_t ofs);
static unsigned short	GetType (gofs_t ofs);
static unsigned short	GetLastFunctionReturn (dfunction_t *df, dstatement_t *ds);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern float		pr_globals[MAX_REGS];
extern int			numpr_globals;

extern char		strings[MAX_STRINGS];
extern int			strofs;

extern dstatement_t	statements[MAX_STATEMENTS];
extern int			numstatements;

extern dfunction_t	functions[MAX_FUNCTIONS];
extern int			numfunctions;

extern ddef_t		globals[MAX_GLOBALS];
extern int			numglobaldefs;

extern ddef_t		fields[MAX_FIELDS];
extern int			numfielddefs;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

FILE		*PR_FILE;
int		FILE_NUM_FOR_NAME = 0;
char		*temp_val[MAX_REGS];
char		*func_headers[MAX_FUNCTIONS];

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

static char	vsline[256], dsline[2048], funcname[512];

static int	regs_used = 0;
static int	lindent;
static char	*DEC_FilesSeen[MAX_DEC_FILES];
static int	DEC_FileCtr = 0;
static qboolean	printassign = false;
static dfunction_t	*cfunc = NULL;


// CODE --------------------------------------------------------------------

static const char *PR_PrintStringAtOfs (gofs_t ofs, def_t* typ)
{
	int		i;
	ddef_t	*def = NULL;
	ddef_t	*d = NULL;

	for (i = 0 ; i < numglobaldefs ; i++)
	{
		d = &globals[i];
		if (d->ofs == ofs)
		{
			def = d;
			break;
		}
	}

	if (!def)
	{
		return Make_Immediate(ofs, NULL, 2);
	}

	if ( !strcmp(strings + def->s_name, IMMEDIATE_VALUE) )
		return DCC_ValueString ((etype_t)def->type, &pr_globals[ofs]);

	if (typ)
	{
	//	printf("type %s %d\n", strings + def->s_name, typ->type->type);
		if (typ->type->type == ev_float && d->type == ev_vector)
		{
			def = &globals[i+1];
		}
	}
//	printf("found %s\n", strings + def->s_name);
	return (strings + def->s_name);
}

static const char *PR_PrintGlobal (gofs_t ofs, def_t* typ)
{
	int		i;
	ddef_t	*def = NULL;
	ddef_t	*d = NULL;

	for (i = 0 ; i < numglobaldefs ; i++)
	{
		d = &globals[i];
		if (d->ofs == ofs)
		{
			def = d;
			break;
		}
	}

	if (!def)
	{
		return NULL;
	}

	if ( !strcmp(strings + def->s_name, IMMEDIATE_VALUE) )
		return DCC_ValueString ((etype_t)def->type, &pr_globals[ofs]);

	if (typ)
	{
		if (typ->type->type == ev_float && d->type == ev_vector)
		{
			def = &globals[i+1];
		}
	}
	return  (strings + def->s_name);
}


static void DccStatement (dstatement_t *s)
{
	const char	*arg1, *arg2, *arg3;
	char		a1[1024], a2[1024], a3[1024];
	int		nargs, i, j;
	dstatement_t	*t, *k;
	unsigned short	doc, ifc, tom;
	def_t		*typ1 = NULL, *typ2 = NULL, *typ3 = NULL;
	ddef_t		*par;

	a1[0] = a2[0] = a3[0] = dsline[0] = funcname[0] = '\0';

	doc =  s->op / 10000;
	ifc = (s->op % 10000) / 100;

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

/* remove all program flow information */
	s->op %= 100;

	typ1 = pr_opcodes[s->op].type_a;
	typ2 = pr_opcodes[s->op].type_b;
	typ3 = pr_opcodes[s->op].type_c;

	if (pr_dumpasm)
	{
		arg1 = PR_PrintGlobal(s->a, typ1);
		arg2 = PR_PrintGlobal(s->b, typ2);
		arg3 = PR_PrintGlobal(s->c, typ3);
		PR_Print("\n%s(%d): %s(%d) %s(%d) %s(%d):\n",
			  pr_opcodes[s->op].opname,s->op,
			  arg1, (signed short)s->a,
			  arg2, (signed short)s->b,
			  arg3, (signed short)s->c);
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
	else if (s->op == OP_RETURN )
	{
		PR_Indent();
		PR_Print("return ");

		if (s->a)
		{
			arg1 = PR_PrintStringAtOfs(s->a, typ1);
			PR_Print("( %s )", arg1);
		}

		PR_Print(";\n");
	}
	else if ( ( OP_MUL_F <= s->op && s->op <= OP_SUB_V ) ||
		  ( OP_EQ_F  <= s->op && s->op <= OP_GT    ) ||
		  ( OP_AND   <= s->op && s->op <= OP_BITOR ) )
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintGlobal(s->c, typ3);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s %s %s;\n", arg3, a1, pr_opcodes[s->op].name, a2);
		}
		else
		{
			sprintf(dsline, "(%s %s %s)", a1, pr_opcodes[s->op].name, a2);
			Make_Immediate(s->c, dsline, 1);
		}
	}
	else if ( (OP_LOAD_F <= s->op) && (s->op <= OP_ADDRESS) )
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintGlobal(s->c, typ3);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s.%s;\n", arg3, a1, a2);
		}
		else
		{
			sprintf(dsline, "%s.%s", a1, a2);
		//	printf("%s.%s making immediate at %d\n",a1,a2,s->c);
			Make_Immediate(s->c, dsline, 1);
		}
	}
	else if ((OP_STORE_F <= s->op) && (s->op <= OP_STORE_FNC))
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1,arg1);

/*		par = DEC_GetParameter (s->a);
		if (par && s->op == OP_STORE_F)
		{
			if (par->type == ev_vector)
			{
				strcat(a1, "_x");
			}
		}
*/
		arg3 = PR_PrintGlobal(s->b, typ2);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s;\n", arg3, a1);
		}
		else
		{
			sprintf(dsline,"%s", a1);
			Make_Immediate(s->b, dsline, 1);
		}
	}
	else if ((OP_STOREP_F <= s->op) && (s->op <= OP_STOREP_FNC))
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		par = DEC_GetParameter (s->a);
		if (par && s->op == OP_STOREP_F)
		{
			if (par->type == ev_vector)
			{
				strcat(a1, "_x");
			}
		}

		arg2 = PR_PrintStringAtOfs(s->b, typ2);

		PR_Indent();
		PR_Print("%s = %s;\n", arg2, a1);
	}
	else if ((OP_NOT_F <= s->op) && (s->op <= OP_NOT_FNC))
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		sprintf(dsline, "!%s", arg1);
		Make_Immediate(s->c, dsline, 1);
	}
	else if ((OP_CALL0 <= s->op) && (s->op <= OP_CALL8))
	{
		nargs = s->op - OP_CALL0;

		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);
	//	printf("fname: %s %s\n", a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

	//	PR_Print(" type3 %d\n", typ3->type->type);

		arg3 = PR_PrintStringAtOfs(s->c, typ3);
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
					if ( (strcmp(&arg3[len], "_x")) &&
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
			arg1 = temp_val[OFS_PARM0+(i*3)];
			arg2 = Make_Immediate(OFS_PARM0+(i*3), NULL, 2);
			if (!arg2)
			{
				continue;
			}
			strcat(dsline, arg2);
		//	temp_val[OFS_PARM0+(i*3)] = NULL;
/*
#ifndef DONT_USE_DIRTY_TRICKS
			if ( (!strcmp(funcname, "WriteCoord")) && (i == 1))
			{
				if ( (strcmp(&arg1[strlen(a1)-2], "_x")) &&
					!(!strcmp(&a1[strlen(arg1)-2], "_y") ||
					  !strcmp(&a1[strlen(arg1)-2], "_z")) )
				    strcat(dsline, "_x");
			}
#endif
*/
		//	if (arg1)
		//		free(arg1);
		}

		strcat(dsline, ")");
		Make_Immediate(OFS_RETURN, dsline, 1);
		j = 1;	//print now

		for (i = 1; (s+i)->op; i++)
		{
		//	printf ("\n%d\n", (s+i)->op);
			if ((s+i)->op % 100 == OP_DONE)
			{
				break;
			}

			if ((s+i)->op % 100 >= 92)
			{
				break;
			}

			if ( (s+i)->a == OFS_RETURN ||
			     (s+i)->b == OFS_RETURN ||
			     (s+i)->c == OFS_RETURN  )
			{
				j = 0;
			//	printf("ofs_return is used before another call %d %d %d\n",j,i,(s+i)->op);
				break;
			}

			if ( OP_CALL0 <= ((s+i)->op % 100) &&
			    ((s+i)->op % 100) <= OP_CALL8 )
			{
			//	if (i == 1)
			//	{
			//		j = 0;
			//	}
			//	printf("another call %d  %d %d %d \n", j, i, (s+i)->a, (s+i)->b);
				break;
			}
		}

		if (j)
		{
			PR_Indent();
			PR_Print("%s;\n", temp_val[OFS_RETURN]);
		}

	//	PR_Print("\n%s;\n", temp_val[OFS_RETURN]);
	}
	else if ( s->op == OP_IF || s->op == OP_IFNOT )
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		if ( s->op == OP_IFNOT)
		{
			if (s->b < 1)
				Error("Found a negative IFNOT jump.");

			// get instruction right before the target
			t = s + s->b - 1;
			tom = t->op % 100;

			if (tom != OP_GOTO)
			{
			// pure if
				PR_Indent();
				PR_Print("if ( %s ) {\n\n", a1);
				lindent++;
			}
			else
			{
				if ((signed short)t->a > 0)
				{
				// ite
					PR_Indent();
					PR_Print("if ( %s ) {\n\n", a1);
					lindent++;
				}
				else
				{
					if ( (signed short)(t->a + s->b) > 1 )
					{
					// pure if
						PR_Indent();
						PR_Print("if ( %s ) {\n\n", a1);
						lindent++;
					}
					else
					{
						int		dum = 1;
						for ( k = t+(t->a) ; k < s ; k++)
						{
							tom = k->op % 100;
							if (tom == OP_GOTO || tom == OP_IF || tom == OP_IFNOT)
								dum = 0;
						}

						if (dum)
						{
						// while
							PR_Indent();
							PR_Print("while ( %s ) {\n\n", a1);
							lindent++;
						}
						else
						{
						// pure if
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
		// do ... while
			if (((signed short)s->b) < 0)
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
		if ((signed short)s->a > 0)
		{
		// else
			lindent--;
			PR_Indent();
			PR_Print("} else {\n\n");
			lindent++;
		}
 		else
		{
		// while
			PR_Print("\n");
			lindent--;
			PR_Indent();
			PR_Print("}\n");
		}
	}
	else if ( (s->op >= 66 && s->op <= 79) || (s->op >= 88 && s->op <= 91) )
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		if (arg2)
			sprintf(dsline, "%s", arg2);

		PR_Indent();
		PR_Print("%s %s %s;\n", arg2, pr_opcodes[s->op].name, a1);

		if (s->c)
		{
			Make_Immediate(s->c, dsline, 1);
		}
	}
	else if ( s->op == 80 || s->op == 81)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		if (arg2)
			strcpy(a2, arg2);

		arg3 = PR_PrintStringAtOfs(s->c, typ3);
		if (arg3)
		{
			PR_Indent();
			PR_Print("%s = %s->%s;\n", arg3, a2, a1);
		}
		else
		{
			sprintf(dsline, "(%s->%s)", a1, a2);
			Make_Immediate(s->c, dsline, 1);
		}
	}
	else if ( s->op == 85)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		PR_Indent();
		PR_Print("AdvanceFrame( %s, %s);\n", a1, arg2);
	}
	else if ( s->op == 87)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		PR_Indent();
		PR_Print("AdvanceThinkTime(%s,%s);\n", a1, arg2);
	}
	else if ( s->op == 92)
	{
		sprintf(dsline,"random()");
		Make_Immediate(OFS_RETURN, dsline, 1);
	}
	else if ( s->op == 93)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		sprintf(dsline, "random(%s)", a1);
		Make_Immediate(OFS_RETURN, dsline, 1);
	}
	else if ( s->op == 94)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		sprintf(dsline, "random(%s,%s)", a1, arg2);
		Make_Immediate(OFS_RETURN, dsline, 1);
	}
	else if ( s->op == 95)
	{
		sprintf(dsline,"random( )");
		Make_Immediate(OFS_RETURN, dsline, 1);
	}
	else if ( s->op == 96)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		sprintf(dsline, "random(%s)", a1);
		Make_Immediate(OFS_RETURN, dsline, 1);
	}
	else if ( s->op == 97)
	{
		arg1 = PR_PrintStringAtOfs(s->a, typ1);
		if (arg1)
			strcpy(a1, arg1);

		arg2 = PR_PrintStringAtOfs(s->b, typ2);
		sprintf(dsline, "random(%s,%s)", a1, arg2);
		Make_Immediate(OFS_RETURN, dsline, 1);
	}
	else
	{
		PR_Print("\n/* ERROR: UNKNOWN COMMAND */\n OP: %d  ", s->op);
		arg1 = PR_PrintStringAtOfs(s->a, NULL);
		PR_Print("a: %s(%d)", arg1, s->a);
		arg2 = PR_PrintStringAtOfs(s->b, NULL);
		PR_Print(" b: %s(%d)", arg2, s->b);
		arg3 = PR_PrintStringAtOfs(s->c, NULL);
		PR_Print(" c: %s(%d)\n", arg3, s->c);
	}
}

static char *Make_Immediate (gofs_t ofs, const char *linestr, int mode)
{
	unsigned short	i;

	if (mode == 0)
	{
		for (i = 0; i < MAX_REGS; i++)
		{
			if (temp_val[i])
				free(temp_val[i]);
			temp_val[i] = NULL;
		}
		return NULL;
	}
	else
	{
		i = ofs;

		if (i >= MAX_REGS)
			Error ("MAX_REGS REACHED IN MAKE_IMMEDIATE %d max is %d currently in use %d(%d %d %d %d)\n",
				 i, MAX_REGS, regs_used, i, cfunc->parm_start, cfunc->locals, ofs);
		if (mode == 1)
		{ //write
			size_t	len = strlen(linestr);
			regs_used++;
			if (temp_val[i])
			{
				free(temp_val[i]);
				temp_val[i] = NULL;
			}
			temp_val[i] = (char *) malloc(len + 1);
			if (temp_val[i] == NULL)
				Error("%s: failed to create new string for %s\n", __thisfunc__, linestr);
			memcpy(temp_val[i], linestr, len);
			temp_val[i][len] = '\0';

			return temp_val[i];
		}
		else if (mode == 2)
		{ //read
			regs_used--;
			return  temp_val[i];
		}
	}

	return NULL;
}


static void AddProgramFlowInfo (dfunction_t *df)
{
	dstatement_t	*ds, *ts;
	signed short	dom, tom;
	dstatement_t	*k;

	ds = statements + df->first_statement;

	while (1)
	{
		dom = (ds->op) % 100;
		if (!dom)
		{
			break;
		}
		else if (dom == OP_GOTO)
		{
		/* check for i-t-e */
			if ((signed short)ds->a > 0)
			{
				ts = ds + ds->a;
				ts->op += 100;	/* mark the end of a if/ite construct */
			}
		}
		else if (dom == OP_IFNOT)
		{
		/* check for pure if */
			ts  = ds + ds->b;
			tom = (ts-1)->op % 100;

			if ( tom != OP_GOTO)
			{
				ts->op += 100;	/* mark the end of a if/ite construct */
			}
			else
			{
				if ( ((signed short)(ts-1)->a) < 0)
				{
					if ( ((signed short)(ts-1)->a + (signed short)ds->b) > 1 )
					{ // pure if
						ts->op += 100;	/* mark the end of a if/ite construct */
					}
					else
					{
						int	dum = 1;
						for ( k = (ts-1)+((signed short)(ts-1)->a) ; k < ds ; k++)
						{
							tom = k->op % 100;
							if (tom == OP_GOTO || tom == OP_IF || tom == OP_IFNOT)
								dum = 0;
						}
						if (!dum)
						{
						// pure if
							ts->op += 100;	/* mark the end of a if/ite construct */
						}
					}
				}
			}
		}
		else if (dom == OP_IF)
		{
			ts = ds + (signed short)ds->b;
			if (((signed short)ds->b) < 0)
			{
				ts->op += 10000; /* mark the start of a do construct */
			}
			else
			{
				ts->op += 100; /* mark the end of a if/ite construct */
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

	for (i = 0 ; i < lindent ; i++)
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

	for (j = start; j < df->parm_start+df->locals; j++)
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

			if (!strcmp("I+", strings + par->s_name))
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

			PR_Print(" %s", strings + par->s_name);

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

	if (func_headers[df-functions])	//already done
		return;

	if (df->first_statement < 0)
	{ //builtin
		FindBuiltinParameters(df-functions);
		return;
	}

//get return type
	linetxt[0] = '\0';
	t1 = GetReturnType(df-functions);

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

//print parameters
	strcat(linetxt, " (");

	start = df->parm_start;
	//i = df->numparms;

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

			strcat(linetxt, strings + par->s_name);
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
	func_headers[df-functions] = (char *) malloc(strlen(linetxt) + 1);
	if (func_headers[df-functions] == NULL)
		Error ("%s: malloc failed.", __thisfunc__);
	strcpy(func_headers[df-functions], linetxt);
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

	df = functions + func;
	ds = statements + df->first_statement;

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
		{
			break;
		}

		if ( ds->op == OP_RETURN )
		{ //find 2 differnent returns if possible just to be certain(ie: findtarget)

			j++; /* we do come here with j == 1 already. put j > 1 checks below. */

			if (ds->a != 0)
			{
				if (ds->a == OFS_RETURN)
				{
					di = ds - 1;
					while ((di-statements) >= df->first_statement)
					{ //that stupid equal, what a bitch
						if ( OP_CALL0  <= di->op && di->op <= OP_CALL8 )
						{
							type1 = pr_opcodes[di->op].type_a;
							arg1 = PR_PrintStringAtOfs(di->a, type1);
							if (!arg1)
								Error("function name not found!!!\n");
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
						} //end if call
						else if (92  <= di->op && di->op <= 94 )
						{
							if (j < 2)
								rtype[j] = ev_float;
							/* else: array out of bounds */
							break;
						}
						else if (95  <= di->op && di->op <= 97 )
						{
							if (j < 2)
								rtype[j] = ev_vector;
							/* else: array out of bounds */
							break;
						}

						di--;
					} //end while ofs_return
				}
				else
				{
					if (j > 1)
						break;	/* array out of bounds */
					par = DEC_GetParameter (ds->a);
					if (par)
					{
						rtype[j] = par->type;
					}
					else
					{
					// find the op where the reg was created,
					// it can tell what the val is
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

#if 0	// this is unused
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
#endif	// end of unused function

static ddef_t *PR_GetField (const char *name, ddef_t *dd)
{
	int	i;
	ddef_t	*d;

	for (i = 1 ; i < numfielddefs ; i++)
	{
		d = &fields[i];
		if (!strcmp(strings + d->s_name, name))
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

	for (i = 1 ; i < numfielddefs ; i++)
	{
		d = &fields[i];
		if ( d->ofs == dd->ofs )
		{
			if ( (d->type == ev_vector) && ((dd-1) == d) )
				return dd;

		//	printf("%s %d %d\n", dd ? strings + dd->s_name : "null", dd ? dd->ofs : 0, d ? d->ofs : 0);
			return d;
		}
	}

	return NULL;
}


void Dcc_Functions (void)
{
	int		i;
	dfunction_t	*df;
	char		fname[1024];
	FILE		*prgs;

	prgs = fopen("progs.src","w");
	if (!prgs)
		Error("unable to open progs.src!!!\n");

	fprintf(prgs, "%s", "../progs.dat\n\n");

	for (i = 1; i < numfunctions; i++)
	{
		df = functions + i;

		fname[0] = '\0';
		sprintf (fname, "%s", strings + df->s_file);

		if (FILE_NUM_FOR_NAME)
			sprintf (fname, "%d.hc", df->s_file);

		if ( !DEC_AlreadySeen(fname) )
			fprintf(prgs, "%s\n", fname);

		PR_FILE = fopen(fname, "a+");

		if (PR_FILE == NULL)
		{
			PR_FILE = stdout;
			Error("unable to open %s\n", fname);
		}

		PR_PrintFunction(strings + df->s_name);
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

	for (j++ ; j < end; j++)
	{
		par = DEC_GetParameter (j);
		if (par && strcmp(strings + par->s_name, IMMEDIATE_VALUE))
		{
		//	printf("next is %s at %d\n", strings + par->s_name, par->ofs);
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
		if (j < size-1)
		{
			if ( !(j & 0x3) )
				PR_Print(" %s,\n", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs + (j*type_size[par->type])]));
			else
				PR_Print(" %s,", DCC_ValueString((etype_t)par->type, &pr_globals[par->ofs + (j*type_size[par->type])]));
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
	ddef_t		*par, *par2;
	ddef_t		*ef;
	const char	*arg2;

	printf("finding declared globals:.");

	df = cfunc;
	dfpred = df - 1;

	for (j = 0, ps = 0 ; j < dfpred->numparms ; j++)
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
		par2 = DEC_GetParameter (j+1);
		if (! (j & 0xf) )
			printf(".");

		if (!par)
			cnt++;

		if (par/* && strcmp(strings + par->s_name, IMMEDIATE_VALUE)*/)
		{
		//	printf("cnt is %d\n", cnt);
			cnt = 0;
			bsize = CalcArraySize(j, end);

			if (par->type & (1<<15))
				par->type -= (1<<15);

		//	printf("%s type: %d ofs: %d par2: %d bsize: %d\n",
		//		  strings + par->s_name, par->type,
		//		  par->ofs, par2 ? (par2->ofs-par->ofs) : -1,
		//		  bsize-par->ofs);

			if (par->type == ev_function)
			{
				if ( strcmp(strings + par->s_name, IMMEDIATE_VALUE) )
				{
					if ( strcmp(strings + par->s_name, strings + cfunc->s_name) )
					{
						i = DEC_GetFunctionIdxByName(strings + par->s_name);
						PR_FunctionHeader(functions + i);
						PR_Print("%s%s;\n", func_headers[i],strings + (functions + i)->s_name );
					}
				}
			}
			else
			{
				if (par->type != ev_pointer)
				{
					if ( strcmp(strings + par->s_name, IMMEDIATE_VALUE) )
					{
						if (par->type == ev_field)
						{
							ef = PR_GetField(strings + par->s_name, par);
							if (!ef)
								Error("Could not locate a field named \"%s\"", strings + par->s_name);
							i = (ef->type - (ef->type & (0x1<<15)));
							if (i == ev_vector)
								j += 3;
							if (i == ev_function)
							{
								arg2 = GetFieldFunctionHeader(strings + ef->s_name);
							//	printf("function .%s %s;\n", arg2, strings + ef->s_name);
								PR_Print(".%s %s;\n", arg2, strings + ef->s_name);
							}
							else
							{
							//	printf("variable %d .%s %s;\n", i, type_names[i], strings + ef->s_name);
								par = PR_FieldIsUnique(ef);
								if (par == ef)
									PR_Print(".%s %s;\n", type_names[i], strings + ef->s_name);
								else
								//	PR_Print(".%s %s;\n", type_names[i], strings + ef->s_name);
									PR_Print(".%s %s alias %s;\n", type_names[i], strings + ef->s_name, strings + par->s_name);
							}
						}
						else
						{
							bsize = (bsize - par->ofs) / type_size[par->type];
							if ((!par->ofs) || ((bsize - 1) < (*(int *)&pr_globals[par->ofs-1])))
							{
							//	printf("bsize %d %d\n", bsize, (*(int *)&pr_globals[par->ofs-1]));
								bsize = 1;
							}
						 	if (bsize != 1)
							{
								bsize = (*(int *)&pr_globals[par->ofs-1]) + 1;
							}
							if (par->type == ev_vector)
								j += 2;
							if (par->type == ev_entity || par->type == ev_void)
							{
								if (!strcmp(strings + par->s_name, "end_sys_fields"))
									printassign = true;
								PR_Print("%s %s;\n", type_names[par->type], strings + par->s_name);
							}
							else
							{
								if (pr_dumpasm  ||
								    printassign ||
								   !strcmp(strings + par->s_name, "string_null"))
								{
									if (bsize < 2)
										PR_Print("%s %s   = ", type_names[par->type], strings + par->s_name);
									else
										PR_Print("%s %s [%d]   = ", type_names[par->type], strings + par->s_name, bsize);
									PR_InitValues(par, bsize);
								}
								else
								{
									PR_Print("%s %s;\n", type_names[par->type], strings + par->s_name);
								}
							}
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
	def_t		*typ1, *typ2;
	const char	*arg1, *arg2, *arg3;

	for (i = 1; i < numstatements; i++)
	{
		d = statements + i;

		if (d->op == OP_ADDRESS)
		{
			typ1 = pr_opcodes[d->op].type_a;
			typ2 = pr_opcodes[d->op].type_b;
			arg1 = PR_PrintGlobal(d->a, typ1);
			arg2 = PR_PrintGlobal(d->b, typ2);

			if (arg2)
			{
				if (!strcmp(s_name, arg2))
				{
					if ((d+1)->op == OP_STOREP_FNC)
					{
						d++;
						typ1 = pr_opcodes[d->op].type_a;
						arg3 = PR_PrintGlobal(d->a, typ1);
						if (!arg3)
							continue;
						j = DEC_GetFunctionIdxByName(arg3);
						PR_FunctionHeader(functions + j);
						if (strcmp("void  ()", func_headers[j]))
						{ //if void () continue checking just to be certain (ie: th_pain)
							return func_headers[j];
						}
					}

					if ((d+1)->op == OP_LOAD_FNC && (d+2)->op == OP_STOREP_FNC)
					{
						d++;
						typ1 = pr_opcodes[d->op].type_a;
						arg1 = PR_PrintGlobal(d->a, typ1);
						typ2 = pr_opcodes[d->op].type_b;
						arg2 = PR_PrintGlobal(d->b, typ2);
						if (!arg2)
							continue;
						if (strcmp(s_name, arg2))
						{
							return GetFieldFunctionHeader(arg2);
						}
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


void FindBuiltinParameters (int func)
{
	int		i, j;
	unsigned short	type[9];
	dstatement_t	*ds, *dsf = NULL;
	dfunction_t	*df, *dft = NULL;
	const char	*arg1;
	char		sname[512], plist[512], parm[128];

	if (func_headers[func])
		return;

//	func = DEC_GetFunctionIdxByName("starteffect");

	df = functions + func;

//	printf("%s %d\n", strings + df->s_name, df->numparms);

	printf("looking for builtin %s...: ", strings+df->s_name);

	for (i = 1; i < numfunctions; i++)
	{//let'em know its working, not hanging!!!
		if (! (i & 0xf) )
			printf(".");

		j = (functions+i)->first_statement;

		if (j < 0)
			continue;

		ds = statements + j;
		//j = 0;

		while (ds && ds->op)
		{
			if (ds->op >= OP_CALL0 && ds->op <= OP_CALL8)
			{
				arg1 = PR_PrintStringAtOfs(ds->a, 0);

				if (arg1)
				{
					if (!strcmp(strings + df->s_name, arg1))
					{
						dsf = ds;
						dft = functions + i;
						printf("\nfound!!\ndetermining parameters\n");
						break;
					}
				}
			}

			arg1 = NULL;
			ds++;
		//	ds = statements + (functions+i)->first_statement + j;
		}

		if (dsf)
			break;
	}

	if (!dsf)
	{
		printf("NOT found!!\nsetting parameters to void\n");
		func_headers[func] = (char *) malloc(strlen("void ()") + 1);
		if (func_headers[func] == NULL)
			Error ("%s: malloc failed.", __thisfunc__);
		sprintf(func_headers[func], "%s", "void ()");
		return;
	}

	j = ((dsf->op - OP_CALL0));
	type[8] = ev_void;

	for (ds = dsf + 1 ; ds && ds->op ; ds++)
	{
		if ((ds->op >= OP_CALL0 && ds->op <= OP_CALL8) || ds->op >= 92)
			break;

		if (ds->a == OFS_RETURN)
		{
		//	printf("%d %d %d %d 1 rt %d %d\n", ds->op, ds->a, ds->b, ds->c, pr_opcodes[ds->op].type_a->type->type, ev_pointer);
			if (ds->c)
			{
				type[8] = pr_opcodes[ds->op].type_c->type->type;
				if (type[8] == ev_pointer)
				{
						type[8] = BackBuildReturnType(dft, ds, ds->c);
				}
			}
			else
			{
				type[8] = pr_opcodes[ds->op].type_b->type->type;
				if (type[8] == ev_pointer)
				{
					type[8] = BackBuildReturnType(dft, ds, ds->b);
				}
			}

			break;
		}

		if (ds->b == OFS_RETURN)
		{
		//	printf("%d %d %d %d 2 rt %d\n", ds->op, ds->a, ds->b, ds->c, pr_opcodes[ds->op].type_b->type->type);
			type[8] = pr_opcodes[ds->op].type_b->type->type;
			if (type[8] == ev_pointer)
			{
				type[8] = BackBuildReturnType(dft, ds, ds->b);
			}

			break;
		}

		if (ds->c == OFS_RETURN)
		{
		//	printf("%d %d %d %d 3 rt %d\n", ds->op, ds->a, ds->b, ds->c, pr_opcodes[ds->op].type_c->type->type);
			type[8] = pr_opcodes[ds->op].type_c->type->type;
			if (type[8] == ev_pointer)
			{
				type[8] = BackBuildReturnType(dft, ds, ds->c);
			}

			break;
		}
	}

//find name
	memset (sname, 0, sizeof(sname));
	arg1 = PR_PrintStringAtOfs(dsf->a, 0);
	sprintf(sname, "%s", arg1);

//look for first two parms
	if (dsf->b == 1)
		type[0] = GetLastFunctionReturn(dft, dsf);
	else
		type[0] = GetType(dsf->b);

	if (type[0] == ev_pointer)
		type[0] = BackBuildReturnType(dft, dsf, dsf->b);

	if (dsf->c == 1)
		type[1] = GetLastFunctionReturn(dft, dsf);
	else
		type[1] = GetType(dsf->c);

	printf("type b %d %d\n", type[0], dsf->b);
	printf("type c %d %d\n", type[1], dsf->c);
	if (type[1] == ev_pointer)
		type[1] = BackBuildReturnType(dft, dsf, dsf->c);

//look in regs for rest of parms
	i = 2;

	while (i < j)
	{
		if (i > 8)	/* just in case.. */
			Error ("%s (%d): array out of bounds.", __thisfunc__, __LINE__);
		type[i] = ev_void;
		for (ds = dsf; (ds-statements) >= dft->first_statement; ds--)
		{
			if (ds->a == ((3*i)+4))
			{
			//	printf("a ");
				type[i] = GetType(ds->a);
				if (type[i] == ev_pointer)
					type[i] = BackBuildReturnType(dft, ds, ds->a);
				break;
			}

			if (ds->b == ((3*i)+4))
			{
			//	printf("b ");
				if (pr_opcodes[ds->op].right_associative)
				{
					type[i] = GetType(ds->a);
					if (type[i] == ev_pointer || type[i] == ev_field)
						type[i] = BackBuildReturnType(dft, ds, ds->a);
				}
				else
				{
					type[i] = GetType(ds->c);
					if (type[i] == ev_pointer || type[i] == ev_field)
						type[i] = BackBuildReturnType(dft, ds, ds->c);
				}

				break;
			}

			if (ds->c == ((3*i)+4))
			{
			//	printf("c ");
				type[i] = GetType(ds->a);
				if (type[i] == ev_pointer)
					type[i] = BackBuildReturnType(dft, ds, ds->c);
				break;
			}
		}

	//	printf("type %i: op: %d a:%d b: %d c: %d rt: %d\n", i, ds->op, ds->a, ds->b, ds->c, type[i]);
		i++;
	}

//print results to string
	memset (plist, 0, sizeof(plist));
	if (type[8] & (1<<15))
	{
		sprintf(plist,".");
		type[8] -= (type[8] & (1<<15));
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
			if (type[i] & (1<<15))
			{
				strcat(plist, ".");
				type[i] -= (type[i] & (1<<15));
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

			if (i < j-1)
				strcat(plist, ", ");
		}
	}

	strcat(plist, ")");
	func_headers[func] = (char *) malloc(strlen(plist) + 1);
	if (func_headers[func] == NULL)
		Error ("%s: malloc failed.", __thisfunc__);
	sprintf(func_headers[func], "%s", plist);
	printf("%s%s\nin %s in file %s\n", plist, sname, strings + dft->s_name, strings + dft->s_file);
}


static unsigned short BackBuildReturnType (dfunction_t *df,dstatement_t *dsf, gofs_t ofs)
{
	dstatement_t	*ds;
	unsigned short	rtype = ev_void;
	ddef_t		*par;

//	printf("backbuilding...\n");

	for (ds = dsf-1; (ds-statements) >= df->first_statement; ds--)
	{
		if (ds->a == ofs)
		{
			rtype = pr_opcodes[ds->op].type_a->type->type;
			par = DEC_GetParameter (ds->b);
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
			par = DEC_GetParameter (ds->a);
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
		//	rtype = GetType(ds->b);
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
		if (par->type & (1<<15))
			par->type -= (1<<15);

		if (par->type == ev_field)
		{
			ef = PR_GetField(strings + par->s_name, par);
			if (!ef)
				Error("Could not locate a field named \"%s\"", strings + par->s_name);
			rtype = (ef->type | (0x1<<15));
		}
		else
		{
			rtype = par->type;
		}
	}

	return rtype;
}


void DccFunctionOP (unsigned short op)
{
	int		i, j = 0;
	dfunction_t	*df;
	dstatement_t	*ds;

	for (i = 1; i < numfunctions; i++)
	{
		df = functions + i;

		if (df->first_statement < 0)
			continue;

		for (ds = &statements[df->first_statement]; ds && ds->op; ds++)
		{
			if (ds->op == op && j++)
			{
				PR_PrintFunction(strings + df->s_name);
				return;
			}
		}
	}
}


void DEC_ReadData (const char *srcfile)
{
	dprograms_t	progs;
	FILE*		h;
	int			i;

	h = SafeOpenRead (srcfile);
	SafeRead (h, &progs, sizeof(progs));

        // byte swap the header
	for (i = 0; i < sizeof(progs)/4; i++)
		((int *)&progs)[i] = LittleLong ( ((int *)&progs)[i] );

	fseek (h, progs.ofs_strings, SEEK_SET);
	strofs = progs.numstrings;
	SafeRead (h, strings, strofs);

	fseek (h, progs.ofs_statements, SEEK_SET);
	numstatements = progs.numstatements;
	SafeRead (h, statements, numstatements*sizeof(dstatement_t));

	fseek (h, progs.ofs_functions, SEEK_SET);
	numfunctions = progs.numfunctions;
	SafeRead (h, functions, numfunctions*sizeof(dfunction_t));

	fseek (h, progs.ofs_globaldefs, SEEK_SET);
	numglobaldefs = progs.numglobaldefs;
	SafeRead (h, globals, numglobaldefs*sizeof(ddef_t));

	fseek (h, progs.ofs_fielddefs, SEEK_SET);
	numfielddefs = progs.numfielddefs;
	SafeRead (h, fields, numfielddefs*sizeof(ddef_t));

	fseek (h, progs.ofs_globals, SEEK_SET);
	numpr_globals = progs.numglobals;
	SafeRead (h, pr_globals, numpr_globals*4);

#if BYTE_ORDER == BIG_ENDIAN
	// byte swap the lumps
	for (i = 0; i < numstatements; i++)
	{
		statements[i].op = LittleShort(statements[i].op);
		statements[i].a = LittleShort(statements[i].a);
		statements[i].b = LittleShort(statements[i].b);
		statements[i].c = LittleShort(statements[i].c);
	}

	for (i = 0; i < numfunctions; i++)
	{
		functions[i].first_statement = LittleLong (functions[i].first_statement);
		functions[i].parm_start = LittleLong (functions[i].parm_start);
		functions[i].s_name = LittleLong (functions[i].s_name);
		functions[i].s_file = LittleLong (functions[i].s_file);
		functions[i].numparms = LittleLong (functions[i].numparms);
		functions[i].locals = LittleLong (functions[i].locals);
	}

	for (i = 0; i < numglobaldefs; i++)
	{
		globals[i].type = LittleShort (globals[i].type);
		globals[i].ofs = LittleShort (globals[i].ofs);
		globals[i].s_name = LittleLong (globals[i].s_name);
	}

	for (i = 0; i < numfielddefs; i++)
	{
		fields[i].type = LittleShort (fields[i].type);
	//	if (fields[i].type & DEF_SAVEGLOBAL)
	//		Error ("%s: pr_fielddefs[i].type & DEF_SAVEGLOBAL", __thisfunc__);
		fields[i].ofs = LittleShort (fields[i].ofs);
		fields[i].s_name = LittleLong (fields[i].s_name);
	}

	for (i = 0; i < numpr_globals; i++)
		*(int *)&pr_globals[i] = LittleLong (*(int *)&pr_globals[i]);
#endif	// BIG_ENDIAN

	printf ("read data from %s:\n", srcfile);
	printf ("total size is: %7i\n", Q_filelength(h));
	printf ("version code is %i\n", progs.version);
	printf ("crc is %i\n", progs.crc);
	printf ("%6i strofs\n", strofs);
	printf ("%6i numstatements\n", numstatements);
	printf ("%6i numfunctions\n", numfunctions);
	printf ("%6i numglobaldefs\n", numglobaldefs);
	printf ("%6i numfielddefs\n", numfielddefs);
	printf ("%6i numpr_globals\n", numpr_globals);
	printf ("--------------------------\n");

	fclose (h);
}


static int DEC_GetFunctionIdxByName (const char *name)
{
	int	i;

	for (i = 1 ; i < numfunctions ; i++)
	{
		if (!strcmp (name, strings + functions[i].s_name) )
			break;
	}

	return i;
}


static ddef_t *DEC_GetParameter (gofs_t ofs)
{
	int		i;
	ddef_t		*def;

	def = NULL;

	for (i = 0 ; i < numglobaldefs ; i++)
	{
		def = &globals[i];

		if (def->ofs == ofs )
		{
			return def;
		}
	}

	return NULL;
}


static int DEC_AlreadySeen (const char *fname)
{
	int		i;
	char		*new1;

//	if (DEC_FileCtr >= MAX_DEC_FILES)
//		Error("%s: DEC_FileCtr: %d", __thisfunc__, DEC_FileCtr);

	for (i = 0 ; i < DEC_FileCtr ; i++)
	{
		if ( !strcmp(fname, DEC_FilesSeen[i] ) )
			return 1;
	}

	if (DEC_FileCtr >= MAX_DEC_FILES-1)
		Error("%s: too many source files.", __thisfunc__);

	new1 = (char *) malloc(strlen(fname)+1);
	if (new1 == NULL)
		Error ("%s: malloc failed.", __thisfunc__);
	strcpy (new1, fname);
	DEC_FilesSeen[DEC_FileCtr++] = new1;

	printf("decompiling %s\n", fname);

	return 0;
}


#if 0	//not used
static void FixFunctionNames (void)
{
	int		i, j;
	dfunction_t	*d;
	char		s[128], *c;

	for (i = 1 ; i < numfunctions ; i++)
	{
		d = &functions[i];
	//	printf ("%s : %s : %i %i (", strings + d->s_file, strings + d->s_name, d->first_statement, d->parm_start);
		s[0] = '\0';
		j = strlen(strings + d->s_file) + 1;
		sprintf (s, "%d.qc", d->s_file);
		c = strings;
		c += d->s_file;
		sprintf (c, "%s", s);
	}
}
#endif


static const char *DCC_ValueString (etype_t type, void *val)
{
	def_t		*def;
	dfunction_t	*f;

	switch (type)
	{
		case ev_string:
			sprintf (vsline, "%s", PR_String(strings + *(int *)val));
			break;
		case ev_entity:
			sprintf (vsline, "entity %i", *(int *)val);
			break;
		case ev_function:
			f = functions + *(int *)val;
			if (!f)
				sprintf (vsline, "undefined function");
			else
				sprintf (vsline, "%s()", strings + f->s_name);
			break;
		case ev_field:
			def = PR_DefForFieldOfs ( *(int *)val );
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


void PR_PrintFunction (const char *name)
{
	int		i;
	dstatement_t	*ds;
	dfunction_t	*df;
	const char	*arg1, *arg2;
	def_t		*typ1, *typ2;

	for (i = 0 ; i < numfunctions ; i++)
	{
		if (!strcmp (name, strings + functions[i].s_name))
			break;
	}

	if (i == numfunctions)
		Error ("No function names \"%s\"", name);

	df = functions + i;
	cfunc = df;
	printf("Statements for %s:\n", name);
	ds = statements + df->first_statement;
	Make_Immediate(0, 0, 0);
	PR_LocalGlobals();
	PR_FunctionHeader(df);

	if (df->first_statement < 0)
	{
		PR_Print("\n%s%s = #%d;\n", func_headers[df-functions], strings + df->s_name, -df->first_statement);
		return;
	}

	PR_Print("\n%s%s = ", func_headers[df-functions], strings + df->s_name);

	if (ds->op == OP_STATE)
	{
		typ1 = pr_opcodes[ds->op].type_a;
		typ2 = pr_opcodes[ds->op].type_b;
		arg1 = PR_ValueString(typ1->type->type, &pr_globals[ds->a]);
		arg2 = PR_PrintStringAtOfs(ds->b, typ2);
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

	while ((di-statements) >= df->first_statement)
	{ //that stupid equal, what a bitch
		printf(" op %d a: %d b: %d c: %d\n", di->op, di->a, di->b, di->c);

		if ( OP_CALL0  <= di->op && di->op <= OP_CALL8 )
		{
			type1 = pr_opcodes[di->op].type_a;
			arg1 = PR_PrintStringAtOfs(di->a, type1);
			if (!arg1)
				Error("function name not found!!!\n");

			i = DEC_GetFunctionIdxByName(arg1);
			if (i == 0)
				break;

			if (i != (df-functions))
			{
				i = GetReturnType(i);
				printf("%s %d is found\n", arg1, i);
				return i;
			}
			else
			{
				break;
			}
		}//end if call
		else if (92  <= di->op && di->op <= 94 )
		{
			return  ev_float;
		}
		else if (95  <= di->op && di->op <= 97 )
		{
			return ev_vector;
		}

		di--;
	}//end while ofs_return

	return ev_void;
}

