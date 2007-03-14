/*
	bspinfo.c
	$Id: bspinfo.c,v 1.4 2007-03-14 21:04:38 sezero Exp $
*/

#include "util_inc.h"
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
