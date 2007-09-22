/*
	parseerr.c
	parse error and parse warning routines for the progs compilation

	$Id: pr_parse.c,v 1.4 2007-09-22 17:40:13 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "util_inc.h"
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

