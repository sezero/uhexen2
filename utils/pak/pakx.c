/*
	pakx.c
	pack file extraction tool.

	$Id: pakx.c,v 1.1 2010-02-23 10:50:17 sezero Exp $

	Copyright (C) 1996-2001 Id Software, Inc.
	Copyright (C) 2010 Ozkan Sezer <sezero@users.sourceforge.net>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301  USA
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "util_io.h"
#include "q_endian.h"
#include "byteordr.h"
#include "pathutil.h"
#include "pakfile.h"

#define	MAX_FILES_IN_PACK	2048

typedef struct
{
	char	name[MAX_OSPATH];
	int		filepos, filelen;
} pakfiles_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	pakfiles_t	*files;
} pack_t;

//======================================================================

void ExtractFile (pack_t *pak, const char *filename, const char *destdir)
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
		if (dest[i - 1] != '/' && dest[i - 1] != '\\')
		{
			dest[i] = '/';
			dest[i + 1] = '\0';
		}
		dptr = dest;
		while (*dptr)
		{
			if (*dptr == '\\')
				*dptr = '/';
			++dptr;
		}
	}

	for (i = 0; i < pak->numfiles; i++)
	{
		if (!filename)
		{
			fseek (pak->handle, pak->files[i].filepos, SEEK_SET);
			strcpy (dptr, pak->files[i].name);
			dest[sizeof(dest) - 1] = '\0';
			printf ("%s --> %s\n", pak->files[i].name, dest);
			if (Q_CopyFromFile(pak->handle, dest, pak->files[i].filelen) != 0)
				Error ("I/O errors during copy.");
			continue;
		}
		if (!strcmp (pak->files[i].name, filename))
		{
			fseek (pak->handle, pak->files[i].filepos, SEEK_SET);
			strcpy (dptr, pak->files[i].name);
			dest[sizeof(dest) - 1] = '\0';
			printf ("%s --> %s\n", pak->files[i].name, dest);
			if (Q_CopyFromFile(pak->handle, dest, pak->files[i].filelen) != 0)
				Error ("I/O errors during copy.");
			break;
		}
	}
	if (filename != NULL && i == pak->numfiles)
		fprintf (stderr, "** %s not in %s\n", filename, pak->filename);
}

pack_t *LoadPackFile (const char *packfile)
{
	dpackheader_t	header;
	int			i, numpackfiles;
	pakfiles_t		*newfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		*info;

	packhandle = fopen (packfile, "rb");
	if (!packhandle)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (header.id[0] != 'P' || header.id[1] != 'A' ||
	    header.id[2] != 'C' || header.id[3] != 'K')
	{
		Error ("%s is not a packfile.", packfile);
	}

	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (header.dirlen < 0 || header.dirofs < 0)
	{
		Error ("Invalid packfile %s (dirlen: %i, dirofs: %i)",
				packfile, header.dirlen, header.dirofs);
	}
	pack = (pack_t *) SafeMalloc (sizeof(pack_t));
	strcpy(pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	if (!numpackfiles)
	{
		pack->files = (pakfiles_t *) SafeMalloc (sizeof(pakfiles_t));
		goto done;
	}
// we are extracting, so don't mind about MAX_FILES_IN_PACK limit.
	info = (dpackfile_t *) SafeMalloc (header.dirlen);
	newfiles = (pakfiles_t *) SafeMalloc (numpackfiles * sizeof(pakfiles_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

// parse the directory
	for (i = 0; i < numpackfiles; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}
	free (info);

	pack->files = newfiles;
   done:
	printf ("Opened %s (%i files)\n", packfile, numpackfiles);
	return pack;
}

void Usage (void)
{
	printf ("Usage:  pakx [-outdir <destdir>] <pakfile> [file [file ....]]\n");
	printf ("        pakx  -h  to display this help message.\n");
	printf ("<destdir> :  Optional. Output directory to extract the files into.\n");
	printf ("Without the [file] arguments, all pak file contents get extracted.\n");
	printf ("\n");
}

int main (int argc, char **argv)
{
	pack_t 	*pak;
	const char	*destdir;
	int	i;

	if (argc < 2)
	{
  usage:
		Usage ();
		exit (1);
	}

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-h"))
		{
			Usage ();
			exit (0);
		}
	}

	if (!strcmp(argv[1], "-outdir"))
	{
		if (argc < 4)
			goto usage;
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
		Error ("Unable to open file %s", argv[i]);
	if (!pak->numfiles)
		Error ("%s has no files.", argv[i]);
	if (++i >= argc)
		ExtractFile (pak, NULL, destdir);
	else
	{
		for ( ; i < argc; i++)
			ExtractFile (pak, argv[i], destdir);
	}

	return 0;
}

