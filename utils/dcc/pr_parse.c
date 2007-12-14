/*
	parseerr.c
	parse error and parse warning routines for the progs compilation

	$Id: pr_parse.c,v 1.5 2007-12-14 16:41:17 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "qcc.h"

// CODE --------------------------------------------------------------------

/*
==============
PR_ParseError

==============
*/
void PR_ParseError (const char *error, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr, error);
	q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	printf ("%s(%i) : %s\n", strings+s_file, pr_source_line, string);

	longjmp (pr_parse_abort, 1);
}

