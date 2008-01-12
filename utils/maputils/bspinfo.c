/*
	bspinfo.c
	$Id: bspinfo.c,v 1.6 2008-01-12 09:46:19 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "q_endian.h"
#include "byteordr.h"
#include "pathutil.h"
#include "bspfile.h"

int main (int argc, char **argv)
{
	int			i;
	char		source[1024];

	if (argc == 1)
		Error ("usage: bspinfo bspfile [bspfiles]");

	ValidateByteorder ();

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
