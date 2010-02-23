/*
	pakfile.c
	pakfile common lib.

	$Id: pakfile.c,v 1.1 2010-02-23 12:40:12 sezero Exp $

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
#include "pathutil.h"
#include "pakfile.h"
#include "pak.h"
#include "crc.h"

//======================================================================

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
	{	// let the caller worry about it
		pack->crc = 0;
		pack->files = (pakfiles_t *) SafeMalloc (sizeof(pakfiles_t));
		return pack;
	}
// this is for reading, so don't mind about MAX_FILES_IN_PACK limit.
	info = (dpackfile_t *) SafeMalloc (header.dirlen);
	newfiles = (pakfiles_t *) SafeMalloc (numpackfiles * sizeof(pakfiles_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

// crc the directory
	CRC_Init (&pack->crc);
	for (i = 0; i < header.dirlen; i++)
		CRC_ProcessByte (&pack->crc, ((byte *)info)[i]);

// parse the directory
	for (i = 0; i < numpackfiles; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	free (info);
	pack->files = newfiles;

	return pack;
}

