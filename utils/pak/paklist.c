/*
 * pakx.c -- pack file listing tool.
 * $Id: paklist.c,v 1.1 2010-02-23 12:40:12 sezero Exp $
 *
 * Copyright (C) 1996-2001 Id Software, Inc.
 * Copyright (C) 2010 Ozkan Sezer <sezero@users.sourceforge.net>
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
#include "util_io.h"
#include "byteordr.h"
#include "pathutil.h"
#include "pakfile.h"
#include "pak.h"

//======================================================================

FUNC_NORETURN static void usage (int ret) {
	printf ("Usage:  paklist <pakfile>\n");
	printf ("        paklist  -h  to display this help message.\n");
	printf ("\n");
	exit (ret);
}

int main (int argc, char **argv)
{
	pack_t 	*pak;
	int	i;

	if (argc < 2)
		usage (1);
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-h"))
			usage (0);
	}

	ValidateByteorder ();

	pak = LoadPackFile (argv[1]);
	if (!pak)
		COM_Error ("Unable to open file %s", argv[i]);
	printf ("PAK file %s: %li bytes, %i files, header crc %u.\n",
			pak->filename, Q_filelength(pak->handle),
			pak->numfiles, pak->crc);
	if (!pak->numfiles)
		COM_Error ("%s has no files.", argv[1]);
	printf ("============================================================================\n");
	printf ("%-56s%10s%10s\n", "Filename", "Length", "Offset");
	printf ("============================================================================\n");
	for (i = 0; i < pak->numfiles; i++)
		printf ("%-56s%10d%10d\n", pak->files[i].name, pak->files[i].filelen, pak->files[i].filepos);

	return 0;
}

