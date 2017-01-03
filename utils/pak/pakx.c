/*
 * pakx.c -- pack file extraction tool.
 * $Id$
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
#include "filenames.h"

//======================================================================

static void ExtractFile (pack_t *pak, const char *filename, const char *destdir)
{
	char	dest[1024], *dptr;
	int	i;

	if (!destdir || !*destdir)
	{
		dptr = dest;
	}
	else
	{
		strcpy (dest, destdir);
		i = strlen (destdir);
		if (!IS_DIR_SEPARATOR(dest[i - 1]))
		{
			dest[i] = DIR_SEPARATOR_CHAR;
			dest[i + 1] = '\0';
		}
		dptr = strchr(dest, '\0');
	}

	for (i = 0; i < pak->numfiles; i++)
	{
		if (!filename)
		{
			fseek (pak->handle, pak->files[i].filepos, SEEK_SET);
			strcpy (dptr, pak->files[i].name);
			dest[sizeof(dest) - 1] = '\0';
			printf ("%s --> %s\n", pak->files[i].name, dest);
			if (Q_WriteFileFromHandle(pak->handle, dest, pak->files[i].filelen) != 0)
				COM_Error ("I/O errors during copy.");
			continue;
		}
		if (!strcmp (pak->files[i].name, filename))
		{
			fseek (pak->handle, pak->files[i].filepos, SEEK_SET);
			strcpy (dptr, pak->files[i].name);
			dest[sizeof(dest) - 1] = '\0';
			printf ("%s --> %s\n", pak->files[i].name, dest);
			if (Q_WriteFileFromHandle(pak->handle, dest, pak->files[i].filelen) != 0)
				COM_Error ("I/O errors during copy.");
			break;
		}
	}
	if (filename != NULL && i == pak->numfiles)
		fprintf (stderr, "** %s not in %s\n", filename, pak->filename);
}

FUNC_NORETURN static void usage (int ret) {
	printf ("Usage:  pakx [-outdir <destdir>] <pakfile> [file [file ....]]\n");
	printf ("        pakx  -h  to display this help message.\n");
	printf ("<destdir> :  Optional. Output directory to extract the files into.\n");
	printf ("Without the [file] arguments, all pak file contents get extracted.\n");
	printf ("\n");
	exit (ret);
}

int main (int argc, char **argv)
{
	pack_t 	*pak;
	const char	*destdir;
	int	i;

	if (argc < 2)
		usage (1);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-h"))
			usage (0);
	}

	if (!strcmp(argv[1], "-outdir"))
	{
		if (argc < 4)
			usage (1);
		i = 3;
		destdir = argv[2];
	}
	else
	{
		i = 1;
		destdir = NULL;
	}

	ValidateByteorder ();

	pak = LoadPackFile (argv[i]);
	if (!pak)
		COM_Error ("Unable to open file %s", argv[i]);
	printf ("Opened %s (%i files)\n", pak->filename, pak->numfiles);
	if (!pak->numfiles)
		COM_Error ("%s has no files.", pak->filename);
	if (++i >= argc)
		ExtractFile (pak, NULL, destdir);
	else
	{
		for ( ; i < argc; i++)
			ExtractFile (pak, argv[i], destdir);
	}

	return 0;
}

