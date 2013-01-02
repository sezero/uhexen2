/*
 * pr_comp.h
 * PROGS structures: This file is shared by Hexen II and HCC
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

#ifndef __PR_COMP_H
#define __PR_COMP_H

typedef int	func_t;
typedef int	string_t;

typedef enum
{
	ev_bad = -1,
	ev_void = 0,
	ev_string,
	ev_float,
	ev_vector,
	ev_entity,
	ev_field,
	ev_function,
	ev_pointer
} etype_t;

#define	OFS_NULL		0
#define	OFS_RETURN		1
#define	OFS_PARM0		4	// leave 3 ofs for each parm to hold vectors
#define	OFS_PARM1		7
#define	OFS_PARM2		10
#define	OFS_PARM3		13
#define	OFS_PARM4		16
#define	OFS_PARM5		19
#define	OFS_PARM6		22
#define	OFS_PARM7		25
#define	RESERVED_OFS	28

#define	IMMEDIATE_NAME	"I+"		/* was "IMMEDIATE" in qcc */

enum
{
	OP_DONE,
	OP_MUL_F,
	OP_MUL_V,
	OP_MUL_FV,
	OP_MUL_VF,
	OP_DIV_F,
	OP_ADD_F,
	OP_ADD_V,
	OP_SUB_F,
	OP_SUB_V,

	OP_EQ_F,
	OP_EQ_V,
	OP_EQ_S,
	OP_EQ_E,
	OP_EQ_FNC,

	OP_NE_F,
	OP_NE_V,
	OP_NE_S,
	OP_NE_E,
	OP_NE_FNC,

	OP_LE,
	OP_GE,
	OP_LT,
	OP_GT,

	OP_LOAD_F,
	OP_LOAD_V,
	OP_LOAD_S,
	OP_LOAD_ENT,
	OP_LOAD_FLD,
	OP_LOAD_FNC,

	OP_ADDRESS,

	OP_STORE_F,
	OP_STORE_V,
	OP_STORE_S,
	OP_STORE_ENT,
	OP_STORE_FLD,
	OP_STORE_FNC,

	OP_STOREP_F,
	OP_STOREP_V,
	OP_STOREP_S,
	OP_STOREP_ENT,
	OP_STOREP_FLD,
	OP_STOREP_FNC,

	OP_RETURN,
	OP_NOT_F,
	OP_NOT_V,
	OP_NOT_S,
	OP_NOT_ENT,
	OP_NOT_FNC,
	OP_IF,
	OP_IFNOT,
	OP_CALL0,
	OP_CALL1,
	OP_CALL2,
	OP_CALL3,
	OP_CALL4,
	OP_CALL5,
	OP_CALL6,
	OP_CALL7,
	OP_CALL8,
	OP_STATE,
	OP_GOTO,
	OP_AND,
	OP_OR,

	OP_BITAND,
	OP_BITOR,

	OP_MULSTORE_F,
	OP_MULSTORE_V,
	OP_MULSTOREP_F,
	OP_MULSTOREP_V,

	OP_DIVSTORE_F,
	OP_DIVSTOREP_F,

	OP_ADDSTORE_F,
	OP_ADDSTORE_V,
	OP_ADDSTOREP_F,
	OP_ADDSTOREP_V,

	OP_SUBSTORE_F,
	OP_SUBSTORE_V,
	OP_SUBSTOREP_F,
	OP_SUBSTOREP_V,

	OP_FETCH_GBL_F,
	OP_FETCH_GBL_V,
	OP_FETCH_GBL_S,
	OP_FETCH_GBL_E,
	OP_FETCH_GBL_FNC,

	OP_CSTATE,
	OP_CWSTATE,

	OP_THINKTIME,

	OP_BITSET,
	OP_BITSETP,
	OP_BITCLR,
	OP_BITCLRP,

	OP_RAND0,
	OP_RAND1,
	OP_RAND2,
	OP_RANDV0,
	OP_RANDV1,
	OP_RANDV2,

	OP_SWITCH_F,
	OP_SWITCH_V,
	OP_SWITCH_S,
	OP_SWITCH_E,
	OP_SWITCH_FNC,

	OP_CASE,
	OP_CASERANGE
};

typedef struct statement_v6_s
{
	unsigned short	op;
	short	a, b, c;
} dstatement_v6_t;

typedef struct statement_v7_s
{
	unsigned short	pad;
	unsigned short	op;
	int	a, b, c;
} dstatement_v7_t;

typedef struct
{
	unsigned short	type;	// if DEF_SAVEGLOBAL bit is set
				// the variable needs to be saved in savegames
	unsigned short	ofs;
	int		s_name;
} ddef_v6_t;

typedef struct
{
	unsigned short	pad;
	unsigned short	type;	// if DEF_SAVEGLOBAL bit is set
				// the variable needs to be saved in savegames
	int		ofs;
	int		s_name;
} ddef_v7_t;

COMPILE_TIME_ASSERT(dstatement_v6_t, sizeof(dstatement_v6_t) == 8);
COMPILE_TIME_ASSERT(dstatement_v7_t, sizeof(dstatement_v7_t) == 16);
COMPILE_TIME_ASSERT(ddef_v6_t, sizeof(ddef_v6_t) == 8);
COMPILE_TIME_ASSERT(ddef_v7_t, sizeof(ddef_v7_t) == 12);

#define	DEF_SAVEGLOBAL	(1<<15)

#define	MAX_PARMS	8

typedef struct
{
	int		first_statement;	// negative numbers are builtins
	int		parm_start;
	int		locals;			// total ints of parms + locals

	int		profile;		// runtime

	int		s_name;
	int		s_file;			// source file defined in

	int		numparms;
	byte	parm_size[MAX_PARMS];
} dfunction_t;


#define	PROG_VERSION_V6		6
#define	PROG_VERSION_V7		7

// process the progs internally as version 7:
#define	PROG_VERSION		(PROG_VERSION_V7)
typedef ddef_v7_t	ddef_t;
typedef dstatement_v7_t	dstatement_t;

typedef struct
{
	int		version;
	int		crc;		// check of header file

	int		ofs_statements;
	int		numstatements;	// statement 0 is an error

	int		ofs_globaldefs;
	int		numglobaldefs;

	int		ofs_fielddefs;
	int		numfielddefs;

	int		ofs_functions;
	int		numfunctions;	// function 0 is an empty

	int		ofs_strings;
	int		numstrings;	// first string is a null string

	int		ofs_globals;
	int		numglobals;

	int		entityfields;
} dprograms_t;

#endif	/* __PR_COMP_H */

