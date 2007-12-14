/*
	bspinfo.c
	$Id: bspinfo.c,v 1.5 2007-12-14 16:41:23 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "pathutil.h"
#include "bspfile.h"

int main (int argc, char **argv)
{
	int			i;
	char		source[1024];

	if (argc == 1)
		Error ("usage: bspinfo bspfile [bspfiles]");

	for (i = 1 ; i < argc ; i++)
	{
		printf ("---------------------\n");
		strcpy (source, argv[i]);
		DefaultExtension (source, ".bsp");
		printf ("%s\n", source);

		LoadBSPFile (source);
		PrintBSPFileSizes ();
		printf ("---------------------\n");
	}

	exit (0);
}
