/*
	parseerr.c
	parse error and parse warning routines for the progs compilation

	$Id: pr_parse.c,v 1.4 2007-09-22 15:27:41 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "util_inc.h"
#include "hcc.h"

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

	printf ("%s(%d) : %s\n", strings+s_file, lx_SourceLine, string);

	longjmp (pr_parse_abort, 1);
}

/*
==============
PR_ParseWarning

==============
*/
void PR_ParseWarning (const char *error, ...)
{
	va_list		argptr;
	char		string[1024];

	if (hcc_WarningsActive == false)
	{
		return;
	}
	va_start (argptr, error);
	q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	printf ("%s(%d) : warning : %s\n", strings+s_file, lx_SourceLine, string);
}

