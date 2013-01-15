/*
 * lexi.c
 *
 * $Id: pr_lex.c,v 1.14 2007-12-14 16:41:17 sezero Exp $
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

const char	*pr_file_p;
int		pr_source_line;

static int	pr_bracelevel;

char		pr_token[2048];
token_type_t	pr_token_type;
type_t		*pr_immediate_type;
eval_t		pr_immediate;

char		pr_immediate_string[2048];

int		pr_error_count;

static const char	*pr_punctuation[] =
{
	// longer symbols must be before
	// a shorter partial match
	"&&",		"||",
	"<=",		">=",
	"==",		"!=",
	";",		",",
	"!",		"*=",
	"*",		"/=",
	"/",		"(", ")",
	"-=",		"->",
	"-",		"+=",
	"+",		"=",
	"[", "]",	"{", "}",
	"...",		".",
	"<",		">",
	"#",		"&",
	"|=",		"|",
	"^=",		":",
	NULL
};

// simple types.  function types are dynamically allocated
type_t	type_void	= {ev_void, &def_void};
type_t	type_string	= {ev_string, &def_string};
type_t	type_float	= {ev_float, &def_float};
type_t	type_vector	= {ev_vector, &def_vector};
type_t	type_entity	= {ev_entity, &def_entity};
type_t	type_field	= {ev_field, &def_field};
type_t	type_function	= {ev_function, &def_function,NULL,&type_void};
// type_function is a void() function used for state defs
type_t	type_pointer	= {ev_pointer, &def_pointer};
type_t	type_floatfield	= {ev_field, &def_field, NULL, &type_float};

int	type_size[8]	= {1, 1, 1, 3, 1, 1, 1, 1};

def_t	def_void	= {&type_void, "temp"};
def_t	def_string	= {&type_string, "temp"};
def_t	def_float	= {&type_float, "temp"};
def_t	def_vector	= {&type_vector, "temp"};
def_t	def_entity	= {&type_entity, "temp"};
def_t	def_field	= {&type_field, "temp"};
def_t	def_function	= {&type_function, "temp"};
def_t	def_pointer	= {&type_pointer, "temp"};

def_t	def_ret, def_parms[MAX_PARMS];

def_t *def_for_type[8] =
{
	&def_void, &def_string, &def_float, &def_vector,
	&def_entity, &def_field, &def_function, &def_pointer
};

static void PR_LexWhitespace (void);


/*
==============
PR_NewLine

Call at start of file and when *pr_file_p == '\n'
==============
*/
void PR_NewLine (void)
{
	pr_source_line++;
}

/*
==============
PR_LexString

Parses a quoted string
==============
*/
static void PR_LexString (void)
{
	int		c;
	int		len;

	len = 0;
	pr_file_p++;
	do
	{
		c = *pr_file_p++;
		if (!c)
			PR_ParseError ("EOF inside quote");
		if (c == '\n')
			PR_ParseError ("newline inside quote");
		if (c == '\\')
		{	// escape char
			c = *pr_file_p++;
			if (!c)
				PR_ParseError ("EOF inside quote");
			if (c == 'n')
				c = '\n';
			else if (c == '"')
				c = '"';
			else
				PR_ParseError ("Unknown escape char");
		}
		else if (c == '\"')
		{
			pr_token[len] = 0;
			pr_token_type = tt_immediate;
			pr_immediate_type = &type_string;
			strcpy (pr_immediate_string, pr_token);
			return;
		}
		pr_token[len] = c;
		len++;
	} while (1);
}

/*
==============
PR_LexNumber
==============
*/
static float PR_LexNumber (void)
{
	int		c;
	int		len;

	len = 0;
	c = *pr_file_p;
	do
	{
		pr_token[len] = c;
		len++;
		pr_file_p++;
		c = *pr_file_p;
	} while ((c >= '0' && c<= '9') || c == '.');

	pr_token[len] = 0;
	return atof (pr_token);
}

/*
==============
PR_LexVector

Parses a single quoted vector
==============
*/
static void PR_LexVector (void)
{
	int		i;

	pr_file_p++;
	pr_token_type = tt_immediate;
	pr_immediate_type = &type_vector;
	for (i = 0; i < 3; i++)
	{
		pr_immediate.vector[i] = PR_LexNumber ();
		PR_LexWhitespace ();
	}
	if (*pr_file_p != '\'')
		PR_ParseError ("Bad vector");
	pr_file_p++;
}

/*
==============
PR_LexName

Parses an identifier
==============
*/
static void PR_LexName (void)
{
	int		c;
	int		len;

	len = 0;
	c = *pr_file_p;
	do
	{
		pr_token[len] = c;
		len++;
		pr_file_p++;
		c = *pr_file_p;
	} while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'
						|| (c >= '0' && c <= '9'));
	pr_token[len] = 0;
	pr_token_type = tt_name;
}

/*
==============
PR_LexPunctuation
==============
*/
static void PR_LexPunctuation (void)
{
	int		i;
	int		len;
	const char	*p;

	pr_token_type = tt_punct;

	for (i = 0; (p = pr_punctuation[i]) != NULL; i++)
	{
		len = strlen(p);
		if (!strncmp(p, pr_file_p, len) )
		{
			strcpy (pr_token, p);
			if (p[0] == '{')
				pr_bracelevel++;
			else if (p[0] == '}')
				pr_bracelevel--;
			pr_file_p += len;
			return;
		}
	}

	PR_ParseError ("Unknown punctuation");
}

/*
==============
PR_LexWhitespace
==============
*/
static void PR_LexWhitespace (void)
{
	int		c;

	while (1)
	{
	// skip whitespace
		while ((c = *pr_file_p) <= ' ')
		{
			if (c == '\n')
			{
				PR_NewLine ();
				pr_file_p++;
				if (*pr_file_p == 0)	// EOF
					return;
			}
			else
			{
				if (c == 0)
					return;		// EOF
				pr_file_p++;
			}
		}

	// skip // comments
		if (c == '/' && pr_file_p[1] == '/')
		{
			while (*pr_file_p && *pr_file_p != '\n')
				pr_file_p++;
			PR_NewLine();
			if (*pr_file_p == '\n')	// not when EOF
				pr_file_p++;
			continue;
		}

	// skip /* */ comments
		if (c == '/' && pr_file_p[1] == '*')
		{
			pr_file_p += 2;
			do
			{
				if (pr_file_p[0] == '\n')
					PR_NewLine();
				if (pr_file_p[0] == 0 || pr_file_p[1] == 0)
					PR_ParseError("EOF inside comment");
				pr_file_p++;
			} while (pr_file_p[-1] != '*' || pr_file_p[0] != '/');
			pr_file_p++;
			continue;
		}

		break;	// a real character has been found
	}
}

//============================================================================

#define	MAX_FRAMES	256

static char	pr_framemacros[MAX_FRAMES][16];
static int		pr_nummacros;

void PR_ClearGrabMacros (void)
{
	pr_nummacros = 0;
}

static void PR_FindMacro (void)
{
	int		i;

	for (i = 0; i < pr_nummacros; i++)
	{
		if (!strcmp (pr_token, pr_framemacros[i]))
		{
			sprintf (pr_token, "%d", i);
			pr_token_type = tt_immediate;
			pr_immediate_type = &type_float;
			pr_immediate._float = i;
			return;
		}
	}
	PR_ParseError ("Unknown frame macro $%s", pr_token);
}

// just parses text, returning false if an eol is reached
static qboolean PR_SimpleGetToken (void)
{
	int		c;
	int		i;

// skip whitespace
	while ((c = *pr_file_p) <= ' ')
	{
		if (c == '\n' || c == 0)
			return false;
		pr_file_p++;
	}

	i = 0;
	while ((c = *pr_file_p) > ' ' && c != ',' && c != ';')
	{
		pr_token[i] = c;
		i++;
		pr_file_p++;
	}
	pr_token[i] = 0;
	return true;
}

static void PR_ParseFrame (void)
{
	while (PR_SimpleGetToken ())
	{
		strcpy (pr_framemacros[pr_nummacros], pr_token);
		pr_nummacros++;
	}
}

/*
==============
PR_LexGrab

Deals with counting sequence numbers and replacing frame macros
==============
*/
static void PR_LexGrab (void)
{
	pr_file_p++;	// skip the $
	if (!PR_SimpleGetToken ())
		PR_ParseError ("hanging $");

// check for $frame
	if (!strcmp (pr_token, "frame"))
	{
		PR_ParseFrame ();
		PR_Lex ();
	}
// ignore other known $commands
	else if (!strcmp (pr_token, "cd") ||
		 !strcmp (pr_token, "origin") ||
		 !strcmp (pr_token, "base") ||
		 !strcmp (pr_token, "flags") ||
		 !strcmp (pr_token, "scale") ||
		 !strcmp (pr_token, "skin"))
	{	// skip to end of line
		while (PR_SimpleGetToken ())
			;
		PR_Lex ();
	}
// look for a frame name macro
	else
	{
		PR_FindMacro ();
	}
}

//============================================================================

/*
==============
PR_Lex

Sets pr_token, pr_token_type, and possibly pr_immediate and pr_immediate_type
==============
*/
void PR_Lex (void)
{
	int		c;

	pr_token[0] = 0;

	if (!pr_file_p)
	{
		pr_token_type = tt_eof;
		return;
	}

	PR_LexWhitespace ();

	c = *pr_file_p;

	if (!c)
	{
		pr_token_type = tt_eof;
		return;
	}

// handle quoted strings as a unit
	if (c == '\"')
	{
		PR_LexString ();
		return;
	}

// handle quoted vectors as a unit
	if (c == '\'')
	{
		PR_LexVector ();
		return;
	}

// if the first character is a valid identifier, parse until a non-id
// character is reached
	if ((c >= '0' && c <= '9') || ( c == '-' && pr_file_p[1] >= '0' && pr_file_p[1] <= '9'))
	{
		pr_token_type = tt_immediate;
		pr_immediate_type = &type_float;
		pr_immediate._float = PR_LexNumber ();
		return;
	}

	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' )
	{
		PR_LexName ();
		return;
	}

	if (c == '$')
	{
		PR_LexGrab ();
		return;
	}

// parse symbol strings until a non-symbol is found
	PR_LexPunctuation ();
}

//=============================================================================

/*
=============
PR_Expect

Issues an error if the current token isn't equal to string
Gets the next token
=============
*/
void PR_Expect (const char *string)
{
	if (strcmp (string, pr_token))
		PR_ParseError ("expected %s, found %s", string, pr_token);
	PR_Lex ();
}

/*
=============
PR_Check

Returns true and gets the next token if the current token equals string
Returns false and does nothing otherwise
=============
*/
qboolean PR_Check (const char *string)
{
	if (strcmp (string, pr_token))
		return false;

	PR_Lex ();
	return true;
}

/*
============
PR_ParseName

Checks to see if the current token is a valid name
============
*/
const char *PR_ParseName (void)
{
	static char	ident[MAX_NAME];

	if (pr_token_type != tt_name)
		PR_ParseError ("not a name %s", pr_token);
	if (strlen(pr_token) >= MAX_NAME-1)
		PR_ParseError ("name too long");
	strcpy (ident, pr_token);
	PR_Lex ();

	return ident;
}

/*
============
PR_FindType

Returns a preexisting complex type that matches the parm, or allocates
a new one and copies it out.
============
*/
static type_t *PR_FindType (type_t *type)
{
	def_t	*def;
	type_t	*check;
	int	i;

	for (check = pr.types; check; check = check->next)
	{
		if (check->type != type->type ||
		    check->aux_type != type->aux_type ||
		    check->num_parms != type->num_parms)
		{
			continue;
		}
		for (i = 0; i < type->num_parms; i++)
		{
			if (check->parm_types[i] != type->parm_types[i])
				break;
		}
		if (i == type->num_parms)
		{
			return check;
		}
	}

// allocate a new one
	check = (type_t *) SafeMalloc (sizeof (*check));
	*check = *type;
	check->next = pr.types;
	pr.types = check;

// allocate a generic def for the type, so fields can reference it
	def = (def_t *) SafeMalloc (sizeof(def_t));
	def->name = "COMPLEX TYPE";
	def->type = check;
	check->def = def;
	return check;
}

/*
============
PR_SkipToSemicolon

For error recovery, also pops out of nested braces
============
*/
void PR_SkipToSemicolon (void)
{
	do
	{
		if (!pr_bracelevel && PR_Check (";"))
			return;
		PR_Lex ();
	} while (pr_token[0]);	// eof will return a null token
}

/*
============
PR_ParseType

Parses a variable type, including field and functions types
============
*/
char	pr_parm_names[MAX_PARMS][MAX_NAME];

type_t *PR_ParseType (void)
{
	const char	*name;
	type_t	newtype;
	type_t	*type;

	if (PR_Check ("."))
	{
		memset (&newtype, 0, sizeof(newtype));
		newtype.type = ev_field;
		newtype.aux_type = PR_ParseType ();
		return PR_FindType (&newtype);
	}

	if (!strcmp (pr_token, "float"))
		type = &type_float;
	else if (!strcmp (pr_token, "vector"))
		type = &type_vector;
	else if (!strcmp (pr_token, "entity"))
		type = &type_entity;
	else if (!strcmp (pr_token, "string"))
		type = &type_string;
	else if (!strcmp (pr_token, "void"))
		type = &type_void;
	else
	{
		PR_ParseError ("\"%s\" is not a type", pr_token);
		type = &type_float;	// shut up compiler warning
	}
	PR_Lex ();

	if (!PR_Check ("("))
		return type;

// function type
	memset (&newtype, 0, sizeof(newtype));
	newtype.type = ev_function;
	newtype.aux_type = type;	// return type
	newtype.num_parms = 0;
	if (!PR_Check (")"))
	{
		if (PR_Check ("..."))
			newtype.num_parms = -1;	// variable args
		else
		{
			do
			{
				type = PR_ParseType ();
				name = PR_ParseName ();
				strcpy (pr_parm_names[newtype.num_parms], name);
				newtype.parm_types[newtype.num_parms] = type;
				newtype.num_parms++;
			} while (PR_Check (","));
		}

		PR_Expect (")");
	}

	return PR_FindType (&newtype);
}

