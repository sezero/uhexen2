/*
 * qfiles.c
 * $Id: qfiles.c,v 1.11 2009-05-05 16:02:52 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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
#include "crc.h"
#include "q_endian.h"
#include "byteordr.h"
#include "qdir.h"
#include "pakfile.h"

#define	MAX_SOUNDS		1024
#define	MAX_MODELS		1024
#define	MAX_FILES		1024

#define	MAX_DATA_PATH	64


char		precache_sounds[MAX_SOUNDS][MAX_DATA_PATH];
int			precache_sounds_block[MAX_SOUNDS];
int			numsounds;

char		precache_models[MAX_MODELS][MAX_DATA_PATH];
int			precache_models_block[MAX_SOUNDS];
int			nummodels;

char		precache_files[MAX_FILES][MAX_DATA_PATH];
int			precache_files_block[MAX_SOUNDS];
int			numfiles;


packfile_t	pfiles[4096], *pf;
FILE		*packhandle;
int			packbytes;


/*
===========
PackFile

Copy a file into the pak file
===========
*/
void PackFile (const char *src, const char *name)
{
	FILE	*in;
	int		remaining, count;
	char	buf[4096];

	if ((byte *)pf - (byte *)pfiles > (ptrdiff_t)sizeof(pfiles))
		COM_Error ("Too many files in pak file");

	in = SafeOpenRead (src);
	remaining = Q_filelength (in);

	pf->filepos = LittleLong (ftell (packhandle));
	pf->filelen = LittleLong (remaining);
	q_strlcpy (pf->name, name, sizeof(pfiles[0].name));
	printf ("%64s : %7i\n", pf->name, remaining);

	packbytes += remaining;

	while (remaining)
	{
		if (remaining < (int)sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);
		SafeRead (in, buf, count);
		SafeWrite (packhandle, buf, count);
		remaining -= count;
	}

	fclose (in);
	pf++;
}


/*
===========
CopyQFiles
===========
*/
void CopyQFiles (int blocknum)
{
	int		i, dirlen;
	char	srcfile[1024];
	char	destfile[1024];
	char	name[1024];
	packheader_t	header;
	unsigned short		crc;

	// create a pak file
	pf = pfiles;

	q_snprintf (destfile, sizeof(destfile), "%spak%i.pak", gamedir, blocknum);
	packhandle = SafeOpenWrite (destfile);
	SafeWrite (packhandle, &header, sizeof(header));

	blocknum++;

	for (i = 0 ; i < numsounds ; i++)
	{
		if ((blocknum != 2 && precache_sounds_block[i] != blocknum) ||
			(blocknum == 2 && precache_sounds_block[i] < blocknum))
			continue;

		q_snprintf (name, sizeof(name), "sound/%s", precache_sounds[i]);
		q_snprintf (srcfile, sizeof(srcfile), "%s%s", gamedir, name);
		PackFile (srcfile, name);
	}
	for (i = 0 ; i < nummodels ; i++)
	{
		if ((blocknum != 2 && precache_models_block[i] != blocknum) ||
			(blocknum == 2 && precache_models_block[i] < blocknum))
			continue;

		q_snprintf (srcfile, sizeof(srcfile), "%s%s", gamedir, precache_models[i]);
		PackFile (srcfile, precache_models[i]);
	}
	for (i = 0 ; i < numfiles ; i++)
	{
		if ((blocknum != 2 && precache_files_block[i] != blocknum) ||
			(blocknum == 2 && precache_files_block[i] < blocknum))
			continue;

		q_snprintf (srcfile, sizeof(srcfile), "%s%s", gamedir, precache_files[i]);
		PackFile (srcfile, precache_files[i]);
	}

	header.id[0] = 'P';
	header.id[1] = 'A';
	header.id[2] = 'C';
	header.id[3] = 'K';
	dirlen = (byte *)pf - (byte *)pfiles;
	header.dirofs = LittleLong(ftell (packhandle));
	header.dirlen = LittleLong(dirlen);

	SafeWrite (packhandle, pfiles, dirlen);

	fseek (packhandle, 0, SEEK_SET);
	SafeWrite (packhandle, &header, sizeof(header));
	fclose (packhandle);

// do a crc of the file
	CRC_Init (&crc);
	for (i = 0 ; i < dirlen ; i++)
		CRC_ProcessByte (&crc, ((byte *)pfiles)[i]);

	i = pf - pfiles;
	printf ("%i files packed in %i bytes (%i crc)\n",i, packbytes, crc);
}


/*
=================
BspModels

Runs qbsp and light on all of the models with a .bsp extension
=================
*/
void BspModels (void)
{
	int		i;
	char	*m;
	char	cmd[1024];
	char	name[256];

	for (i = 0 ; i < nummodels ; i++)
	{
		m = precache_models[i];
		if (strcmp(m + strlen(m) - 4, ".bsp"))
			continue;
		q_strlcpy (name, m, sizeof(name));
		name[strlen(m) - 4] = '\0';

		q_snprintf (cmd, sizeof(cmd), "qbsp %s%s",gamedir, name);
		system (cmd);
		q_snprintf (cmd, sizeof(cmd), "light -extra %s%s", gamedir, name);
		system (cmd);
	}
}

/*
=============
ReadFiles
=============
*/
void ReadFiles (void)
{
	FILE	*f;
	int		i;

	f = SafeOpenRead ("files.dat");

	fscanf (f, "%i\n", &numsounds);
	for (i = 0 ; i < numsounds ; i++)
		fscanf (f, "%i %s\n", &precache_sounds_block[i], precache_sounds[i]);

	fscanf (f, "%i\n", &nummodels);
	for (i = 0 ; i < nummodels ; i++)
		fscanf (f, "%i %s\n", &precache_models_block[i], precache_models[i]);

	fscanf (f, "%i\n", &numfiles);
	for (i = 0 ; i < numfiles ; i++)
		fscanf (f, "%i %s\n", &precache_files_block[i], precache_files[i]);

	fclose (f);

	printf ("%3i sounds\n", numsounds);
	printf ("%3i models\n", nummodels);
	printf ("%3i files\n", numfiles);
}


/*
=============
main
=============
*/
__attribute__((__noreturn__)) static void usage (void) {
	printf ("qfiles -pak <0 / 1 / 2 / 3> : build a .pak file\n");
	printf ("qfiles -bspmodels : regenerates all brush models\n");
	exit (1);
}

int main (int argc, char **argv)
{
	if (argc == 1)
		usage ();

	ValidateByteorder ();

	if (!strcmp (argv[1], "-pak"))
	{
		if (argc != 3) usage ();
		SetQdirFromPath ("");
		ReadFiles ();
		CopyQFiles (atoi(argv[2]));
	}
	else if (!strcmp (argv[1], "-bspmodels"))
	{
		SetQdirFromPath ("");
		ReadFiles ();
		BspModels ();
	}
	else
	{
		COM_Error ("unknown command: %s", argv[1]);
	}

	return 0;
}

