/*
	parseerr.c
	parse error and parse warning routines for the progs compilation

	$Header: /home/ozzie/Download/0000/uhexen2/utils/h2mp_utils/hcc/pr_parse.c,v 1.1 2006-02-27 14:20:52 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "cmdlib.h"
#include "hcc.h"

// CODE --------------------------------------------------------------------

/*
==============
PR_ParseError

==============
*/
void PR_ParseError(char *error, ...)
{
	va_list argptr;
	char string[1024];

	va_start (argptr, error);
	vsnprintf (string, sizeof (string), error, argptr);
	va_end (argptr);
	printf ("%s(%d) : %s\n", strings+s_file, lx_SourceLine, string);
	longjmp (pr_parse_abort, 1);
}

/*
==============
PR_ParseWarning

==============
*/
void PR_ParseWarning(char *error, ...)
{
	va_list argptr;
	char string[1024];

	if (hcc_WarningsActive == false)
	{
		return;
	}
	va_start (argptr, error);
	vsnprintf (string, sizeof (string), error, argptr);
	va_end (argptr);
	printf ("%s(%d) : warning : %s\n", strings+s_file, lx_SourceLine, string);
}

