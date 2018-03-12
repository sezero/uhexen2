/*
 * bsp2wal.c
 *
 * Copyright (C) 2007-2012 O.Sezer <sezero@users.sourceforge.net>
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
#include "pathutil.h"
#include "util_io.h"
#include "q_endian.h"
#include "byteordr.h"
#include "bspfile.h"
#include "hwal.h"
#include "filenames.h"

// the miptex_wal_t structure has two extra int fields at the beginning
// and the name field is 32 chars long instead of 16, hence this shift.
// the rest, ie. the offsets, are the same.
#define	HWAL_SHIFT		(sizeof(miptex_wal_t) - sizeof(miptex_t))

static char	workpath[1024];
static int	workpath_size;
static int		miponly;

//===========================================================================

static char *MakeWorkPath (char *infilename)
{
	char		*tmp;

	workpath_size = (int) sizeof(workpath);
	memset (workpath, 0, sizeof(workpath));
	tmp = FIND_LAST_DIRSEP(infilename);
	if (!tmp)
		tmp = workpath;
	else
	{
		tmp++;
		if (tmp - infilename > workpath_size)
			COM_Error("%s: insufficient buffer size", __thisfunc__);
		memcpy (workpath, infilename, tmp - infilename);
		workpath_size -= (tmp - infilename);
		tmp = workpath + (tmp - infilename);
	}
	if (workpath_size < (int)sizeof(WAL_EXT_DIRNAME) + 1)
		COM_Error("%s: insufficient buffer size", __thisfunc__);
	memcpy (tmp, WAL_EXT_DIRNAME, sizeof(WAL_EXT_DIRNAME));
	tmp += sizeof(WAL_EXT_DIRNAME) - 1;
	workpath_size -= (int)sizeof(WAL_EXT_DIRNAME) + 1;
	Q_mkdir (workpath);
	*tmp++ = DIR_SEPARATOR_CHAR;

	return tmp;
}

static void WriteWALFile (char *bspfilename)
{
	int		i, j;
	dmiptexlump_t	*m;
	miptex_t	*mt;
	miptex_wal_t	*wt;
	int		pixels;
	char		*tmp;

	LoadBSPFile (bspfilename);

	printf ("Extracting MIPTEX data from %s...\n", bspfilename);

	tmp = MakeWorkPath (bspfilename);

	m = (dmiptexlump_t *)dtexdata;
	for (i = 0; i < m->nummiptex; i++)
	{
		if (m->dataofs[i] == -1)
			continue;

	// LoadBSPFile() does byte-swap the dmiptexlump_t structures
	// but not the individual miptex_t ones:
		mt = (miptex_t *)((byte *)m + m->dataofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);
		pixels = mt->width * mt->height / 64 * 85;

		if (miponly)
		{
			q_snprintf (tmp, (size_t)workpath_size, "%s.mip", mt->name);
			if (tmp[0] == '*')
				tmp[0] = WAL_REPLACE_ASTERIX;
			printf ("%15s (%4u x %-4u) -> %s\n", mt->name, mt->width, mt->height, workpath);
			mt->width = LittleLong (mt->width);
			mt->height = LittleLong (mt->height);
			SaveFile (workpath, (byte *)mt, sizeof(miptex_t) + pixels);
			continue;
		}

		wt = (miptex_wal_t *) SafeMalloc (sizeof(miptex_wal_t) + pixels);

		wt->ident = LittleLong (IDWALHEADER);
		wt->version = LittleLong (WALVERSION);
		wt->width = LittleLong (mt->width);
		wt->height = LittleLong (mt->height);
		strcpy(wt->name, mt->name);
		for (j = 0; j < MIPLEVELS; j++)
		{
			mt->offsets[j] = LittleLong (mt->offsets[j]);
			wt->offsets[j] = LittleLong (HWAL_SHIFT + mt->offsets[j]);
		}

		memcpy (wt + 1,  (byte *)m + m->dataofs[i] + sizeof(miptex_t), pixels);

		// save file
		q_snprintf (tmp, (size_t)workpath_size, "%s.wal", mt->name);
		if (tmp[0] == '*')
			tmp[0] = WAL_REPLACE_ASTERIX;
		printf ("%15s (%4u x %-4u) -> %s\n", mt->name, mt->width, mt->height, workpath);
		SaveFile (workpath, (byte *)wt, sizeof(miptex_wal_t) + pixels);

		free (wt);
	}
}

static void print_help (void)
{
	printf ("BSP2WAL v1.0\n");
	printf ("Extracts all miptex data from bsp files, converts them into\n");
	printf ("HWAL format. The -miponly command line switch makes it skip\n");
	printf ("the conversion and write them directly in miptex format.\n");
	printf ("Usage: bsp2wal [-miponly] bspfile [bspfiles]\n");
}

int main (int argc, char **argv)
{
	int			i;
	char		source[1024];

	ValidateByteorder ();

	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp(argv[i], "-miponly"))
			miponly = 1;
		else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			print_help ();
			exit (0);
		}
		else if (argv[i][0] == '-')
		{
			print_help ();
			COM_Error ("Unknown option \"%s\"", argv[i]);
		}
		else
			break;
	}

	if (i == argc)
	{
		print_help ();
		COM_Error ("No input file specified.");
	}

	for ( ; i < argc ; i++)
	{
		printf ("---------------------\n");
		q_strlcpy (source, argv[i], sizeof(source));
		DefaultExtension (source, ".bsp", sizeof(source));

		WriteWALFile (source);
		printf ("---------------------\n");
	}

	return 0;
}

