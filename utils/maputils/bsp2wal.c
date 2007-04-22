/*
	bsp2wal.c
	$Id: bsp2wal.c,v 1.4 2007-04-22 08:12:48 sezero Exp $
*/

#include "util_inc.h"
#include "cmdlib.h"
#include "pathutil.h"
#include "util_io.h"
#include "q_endian.h"
#include "bspfile.h"
#include "hwal.h"

// the miptex_wal_t structure has two extra int fields at the beginning
// and the name field is 32 chars long instead of 16, hence this shift.
// the rest, ie. the offsets, are the same.
#define	HWAL_SHIFT		(sizeof(miptex_wal_t) - sizeof(miptex_t))

static char	workpath[1024];

//===========================================================================

static void WriteWALFile (const char *bspfilename)
{
	int		i, j;
	dmiptexlump_t	*m;
	miptex_t	*mt;
	miptex_wal_t	*wt;
	int		pixels;
	char		*tmp;

	LoadBSPFile (bspfilename);

	printf ("Extracting HWAL textures from %s...\n", bspfilename);

	memset (workpath, 0, sizeof(workpath));
	tmp = strrchr (bspfilename, '/');
	if (!tmp)
		tmp = strrchr (bspfilename, '\\');
	if (!tmp)
		tmp = workpath;
	else
	{
		tmp++;
		memcpy (workpath, bspfilename, tmp - bspfilename);
		tmp = workpath + (tmp - bspfilename);
	}
	memcpy (tmp, WAL_EXT_DIRNAME, sizeof(WAL_EXT_DIRNAME));
	tmp += sizeof(WAL_EXT_DIRNAME)-1;
	Q_mkdir (workpath);
	*tmp++ = '/';

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
		for (j = 0; j < MIPLEVELS; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);

		pixels = mt->width * mt->height / 64 * 85;

		wt = malloc (sizeof(miptex_wal_t) + pixels);
		memset (wt, 0, sizeof(miptex_wal_t));

		wt->ident = LittleLong (IDWALHEADER);
		wt->version = LittleLong (WALVERSION);
		wt->width = LittleLong (mt->width);
		wt->height = LittleLong (mt->height);
		strcpy(wt->name, mt->name);
		for (j = 0; j < MIPLEVELS; j++)
			wt->offsets[j] = LittleLong (HWAL_SHIFT + mt->offsets[j]);

		memcpy (wt+1,  (byte *)m + m->dataofs[i] + sizeof(miptex_t), pixels);

		// save file
		sprintf (tmp, "%s.wal", mt->name);
		if (tmp[0] == '*')
			tmp[0] = WAL_REPLACE_ASTERIX;
		printf ("%15s (%4i x %-4i) -> %s\n", mt->name, mt->width, mt->height, workpath);
		SaveFile (workpath, (byte *)wt, sizeof(miptex_wal_t) + pixels);

		free (wt);
	}
}

int main (int argc, char **argv)
{
	int			i;
	char		source[1024];

	if (argc == 1)
		Error ("usage: bsp2wal bspfile [bspfiles]");

	for (i = 1 ; i < argc ; i++)
	{
		printf ("---------------------\n");
		strcpy (source, argv[i]);
		DefaultExtension (source, ".bsp");

		WriteWALFile (source);
		printf ("---------------------\n");
	}

	exit (0);
}

