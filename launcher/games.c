/*
 * games.c
 * hexen2 launcher, game installation scanning
 *
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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

#include "common.h"
#include "q_endian.h"
#include "games.h"
#include "crc.h"
#include "pakfile.h"
#include "launcher_defs.h"
#include "config_file.h"

unsigned int	gameflags;
static char	*scan_dir;

typedef struct
{
	int	numfiles;
	unsigned int	crc;
	long	size;
	const char	*dirname;
} pakdata_t;

#define	MAX_PAKDATA	5 /* pak0...4 */
static pakdata_t pakdata[MAX_PAKDATA] =
{
	{ 696,	34289,	22704056, "data1"	},	/* pak0.pak, registered, up-to-date, v1.11
							 *	MD5: c9675191e75dd25a3b9ed81ee7e05eff	*/
	{ 523,	2995 ,	75601170, "data1"	},	/* pak1.pak, registered, up-to-date, v1.11
							 *	MD5: c2ac5b0640773eed9ebe1cda2eca2ad0	*/
	{ 183,	4807 ,	17742721, "data1"	},	/* pak2.pak, oem (Matrox m3D bundle) v1.10
							 *	MD5: 99e0054861e94f66fc8e0e29416859c9	*/
	{ 245,	1478 ,	49089114, "portals"	},	/* pak3.pak, Portal of Praevus expansion pack
							 *	MD5: 77ae298dd0dcd16ab12f4a68067ff2c3	*/
	{ 102,	41062,	10780245, "hw"		}	/* pak4.pak, hexenworld, versions 0.14 - 0.15
							 *	MD5: 88109ee385d9723ac5f1015e034a44dd	*/
};

static pakdata_t demo_pakdata[] =
{
	{ 797,	22780,	27750257, "data1"	}	/* pak0.pak, demo v1.11 from Nov. 1997
							 *	MD5: 8e598d82bf53436ed7a0e133aa4b9f09	*/
};

static pakdata_t oem0_pakdata[] =	/* Continent of Blackmarsh */
{
	{ 697,	9787 ,	22720659, "data1"	}	/* pak0.pak, oem (Matrox m3D bundle) v1.10
							 *	MD5: 8c9c6118117baca7b9349d477403fcc0	*/
};

static pakdata_t old_pakdata[] =
{
	{ 697,	53062,	21714275, "data1"	},	/* pak0.pak, original cdrom (1.03) version
							 *	MD5: b53c9391d16134cb3baddc1085f18683	*/
	{ 525,	47762,	76958474, "data1"	},	/* pak1.pak, original cdrom (1.03) version
							 *	MD5: 9a2010aafb9c0fe71c37d01292030270	*/
	{ 701,	20870,	23537707, "data1"	},	/* pak0.pak, original demo v0.42 from Aug. 1997
							 *	(h2.exe -> console -> version says 1.07!)
							 *	MD5: 208643a09193dafbca4b851762479438	*/
/* !!! FIXME:  I don't have the original v1.08 of Continent of Blackmarsh. I only know the file sizes.	*/
	{ -1,	0,	22719295, "data1"	},	/* pak0.pak, original oem (Matrox m3D) v1.08
							 *	MD5: ????????????????????????????????	*/
	{ -1,	0,	17739969, "data1"	},	/* pak2.pak, original oem (Matrox m3D) v1.08
							 *	MD5: ????????????????????????????????	*/
	{  98,	25864,	10678369, "hw"	},		/* pak4.pak, Hexen2World v0.11 (ugh..)
							 *	MD5: c311a30ac8ee1f112019723b4fe42268	*/
};

static unsigned int check_known_paks (int paknum, int numfiles, unsigned short crc)
{
	if (paknum >= MAX_PAKDATA)
		return GAME_MODIFIED;
#if 0
	if (strcmp(fs_gamedir_nopath, pakdata[paknum].dirname) != 0)
		return GAME_MODIFIED;	/* Raven didn't ship like that */
#endif
	if (numfiles != pakdata[paknum].numfiles)
	{
		switch (paknum)
		{
		case 0:	/* demo ?? */
			if (numfiles == demo_pakdata[0].numfiles &&
					crc == demo_pakdata[0].crc)
				return GAME_DEMO;
			/* oem ?? */
			if (numfiles == oem0_pakdata[0].numfiles &&
					crc == oem0_pakdata[0].crc)
				return GAME_OEM0;
			/* old version of demo ?? */
			if (numfiles == old_pakdata[2].numfiles &&
					crc == old_pakdata[2].crc)
				return GAME_OLD_DEMO;
			/* old cdrom version ?? */
			if (numfiles == old_pakdata[0].numfiles &&
					crc == old_pakdata[0].crc)
				return GAME_OLD_CDROM0;
			/* old oem version ?? */
			if (numfiles == old_pakdata[3].numfiles &&
					crc == old_pakdata[3].crc)
				return GAME_OLD_OEM0;
			/* not original: */
			return GAME_MODIFIED;
		case 1:	/* old cdrom version ?? */
			if (numfiles == old_pakdata[1].numfiles &&
					crc == old_pakdata[1].crc)
				return GAME_OLD_CDROM1;
			/* not original: */
			return GAME_MODIFIED;
		case 2:	/* old oem version ?? */
			if (numfiles == old_pakdata[4].numfiles &&
					crc == old_pakdata[4].crc)
				return GAME_OLD_OEM2;
			/* not original: */
			return GAME_MODIFIED;
		case 4:	/* old HW version ?? */
			if (numfiles == old_pakdata[5].numfiles &&
					crc == old_pakdata[5].crc)
				return GAME_HEXENWORLD;
			/* not original: */
			return GAME_MODIFIED;
		default:/* not original */
			return GAME_MODIFIED;
		}
	}

	if (crc != pakdata[paknum].crc)
		return GAME_MODIFIED;	/* not original */

	/* both crc and numfiles are good, we are still original */
	switch (paknum)
	{
	case 0:	/* pak0 of full version 1.11 */
		return GAME_REGISTERED0;
	case 1:	/* pak1 of full version 1.11 */
		return GAME_REGISTERED1;
	case 2:	/* bundle version */
		return GAME_OEM2;
	case 3:	/* mission pack */
		return GAME_PORTALS;
	case 4:	/* hexenworld */
		return GAME_HEXENWORLD;
	}

	return GAME_MODIFIED;	/* we shouldn't reach here */
}

static void scan_pak_files (const char *packfile, int paknum)
{
	dpackheader_t	header;
	int			i, numpackfiles;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned short		crc;

	packhandle = fopen (packfile, "rb");
	if (!packhandle)
		return;

	fread (&header, 1, sizeof(header), packhandle);
	if (header.id[0] != 'P' || header.id[1] != 'A' ||
	    header.id[2] != 'C' || header.id[3] != 'K')
		goto finish;

	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		goto finish;

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

	/* crc the directory */
	CRC_Init (&crc);
	for (i = 0; i < header.dirlen; i++)
		CRC_ProcessByte (&crc, ((unsigned char *)info)[i]);

	gameflags |= check_known_paks (paknum, numpackfiles, crc);
finish:
	fclose (packhandle);
}

#if !defined(DEMOBUILD)
h2game_t h2game_names[] =	/* first entry is always available */
{
	{  NULL    , "(  None  )"	,   NULL,		0, 1, -1 },
	{ "hcbots" , "BotMatch: HC bots",   "progs.dat",	1, 0, -1 },
	{ "apocbot", "BotMatch: ApocBot",   "progs.dat",	1, 0, -1 },
	{ "fo4d"   , "Fortress of 4 Doors", "maps/u_world.bsp",	0, 0, -1 },
	{ "peanut" , "Project Peanut"	,   "progs.dat",	0, 0, -1 },
};

const int MAX_H2GAMES = sizeof(h2game_names) / sizeof(h2game_names[0]);

h2game_t hwgame_names[] =	/* first entry is always available */
{
	{  NULL     , "Plain DeathMatch", NULL,			0, 1, -1 },
	{ "hexarena", "HexArena"	, "sound/ha/fight.wav", 0, 0, -1 },
	{ "hwctf"   , "Capture the Flag", "models/ctf_flag.mdl",0, 0, -1 },
	{ "siege"   , "Siege"		, "models/h_hank.mdl",  0, 0, -1 },
	{ "db"      , "Dungeon Break"	, "sound/donewell.wav", 0, 0, -1 },
	{ "rk"      , "Rival Kingdoms"	, "troll/h_troll.mdl",  0, 0, -1 },
};

const int MAX_HWGAMES = sizeof(hwgame_names) / sizeof(hwgame_names[0]);

static size_t	string_size;

static void FindMaxStringSize (void)
{
	int	i;
	size_t	len;

	string_size = 0;

	for (i = 1; i < MAX_H2GAMES; i++)
	{
		len = strlen(h2game_names[i].dirname) + strlen(hwgame_names[i].checkfile);
		if (string_size < len)
			string_size = len;
	}

	for (i = 1; i < MAX_HWGAMES; i++)
	{
		len = strlen(hwgame_names[i].dirname) + 11;	/* strlen("hwprogs.dat") == 11 */
		if (string_size < len)
			string_size = len;
		len = len + strlen(hwgame_names[i].checkfile) - 11;
		if (string_size < len)
			string_size = len;
	}

	string_size = string_size + strlen(scan_dir) + 3;	/* 2 for two "/" + 1 for null termination */
}

static void scan_h2_mods (void)
{
	int	i;
	char	*path;

#ifdef DEBUG
	printf ("Scanning for known hexen2 mods\n");
#endif
	path = (char *)malloc(string_size);
	for (i = 1; i < MAX_H2GAMES; i++)
	{
		sprintf (path, "%s/%s/%s", scan_dir, h2game_names[i].dirname, h2game_names[i].checkfile);
		if (access(path, R_OK) == 0)
			h2game_names[i].available = 1;
		else	h2game_names[i].available = 0;
	}
	free (path);
}

static void scan_hw_mods (void)
{
	int	i, j;
	char	*path;

#ifdef DEBUG
	printf ("Scanning for known hexenworld mods\n");
#endif
	path = (char *)malloc(string_size);
	for (i = 1; i < MAX_HWGAMES; i++)
	{
		sprintf (path, "%s/%s/hwprogs.dat", scan_dir, hwgame_names[i].dirname);
		j = access(path, R_OK);
		if (j == 0)
		{
			sprintf (path, "%s/%s/%s", scan_dir, hwgame_names[i].dirname, hwgame_names[i].checkfile);
			j = access(path, R_OK);
		}
		if (j == 0)
			hwgame_names[i].available = 1;
		else	hwgame_names[i].available = 0;
	}
	free (path);
}

#endif	/* DEMOBUILD */


static void scan_binaries (void)
{
	gameflags &= ~(HAVE_H2_BIN|HAVE_HW_BIN|HAVE_GLH2_BIN|HAVE_GLHW_BIN);

	if (access(H2_BINARY_NAME, X_OK) == 0)
		gameflags |= HAVE_H2_BIN;
	if (access(HW_BINARY_NAME, X_OK) == 0)
		gameflags |= HAVE_HW_BIN;
	if (access(BIN_OGL_PREFIX H2_BINARY_NAME, X_OK) == 0)
		gameflags |= HAVE_GLH2_BIN;
	if (access(BIN_OGL_PREFIX HW_BINARY_NAME, X_OK) == 0)
		gameflags |= HAVE_GLHW_BIN;
}

void scan_game_installation (void)
{
	int		i;
	char	pakfile[MAX_OSPATH];

	gameflags = 0;
	if (basedir_nonstd && game_basedir[0])
		scan_dir = game_basedir;
	else
		scan_dir = basedir;

#ifdef DEBUG
	printf ("Scanning base hexen2 installation\n");
#endif
	for (i = 0; i < MAX_PAKDATA; i++)
	{
		snprintf (pakfile, sizeof(pakfile), "%s/%s/pak%d.pak", scan_dir, pakdata[i].dirname, i);
		scan_pak_files (pakfile, i);
	}

	if (gameflags & GAME_REGISTERED0 && gameflags & GAME_REGISTERED1)
		gameflags |= GAME_REGISTERED;
	if (gameflags & GAME_OEM0 && gameflags & GAME_OEM2)
		gameflags |= GAME_OEM;
	if (gameflags & GAME_OLD_CDROM0 && gameflags & GAME_OLD_CDROM1)
		gameflags |= GAME_REGISTERED_OLD;
	if (gameflags & GAME_OLD_OEM0 && gameflags & GAME_OLD_OEM2)
		gameflags |= GAME_OLD_OEM;

	if (gameflags & GAME_REGISTERED0 && gameflags & GAME_OLD_CDROM1)
		gameflags |= GAME_CANPATCH0;
	if (gameflags & GAME_REGISTERED1 && gameflags & GAME_OLD_CDROM0)
		gameflags |= GAME_CANPATCH1;

	if (gameflags & GAME_OEM0 && gameflags & GAME_OLD_OEM2)
		gameflags |= GAME_CANPATCH0;
	if (gameflags & GAME_OEM2 && gameflags & GAME_OLD_OEM0)
		gameflags |= GAME_CANPATCH1;

	if ((gameflags & (GAME_OEM2|GAME_OLD_OEM2) && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_DEMO|GAME_OLD_DEMO)) ||
	    (gameflags & (GAME_REGISTERED1|GAME_OLD_CDROM1) &&
					 gameflags & (GAME_DEMO|GAME_OLD_DEMO|GAME_OEM0|GAME_OLD_OEM0|GAME_OEM2|GAME_OLD_OEM2)))
	{
		gameflags |= GAME_INSTBAD|GAME_INSTBAD2;	/* mix'n'match: bad	*/
	}

	if (!(gameflags & GAME_INSTBAD2))
	{
		if (gameflags & (GAME_REGISTERED_OLD|GAME_OLD_OEM) ||
		    gameflags & (GAME_CANPATCH0|GAME_CANPATCH1))
		{
			gameflags |= GAME_CANPATCH;	/* 1.11 pak patch can fix this	*/
		}
	}

	if (gameflags & (GAME_CANPATCH0|GAME_CANPATCH1))
		gameflags |= GAME_INSTBAD|GAME_INSTBAD2;	/* still a mix'n'match	*/

#if !ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
		gameflags |= GAME_INSTBAD|GAME_INSTBAD3;
#endif	/* OLD_DEMO */
#if !ENABLE_OLD_RETAIL
	if (gameflags & (GAME_OLD_CDROM0|GAME_OLD_CDROM1|GAME_OLD_OEM0|GAME_OLD_OEM2))
		gameflags |= GAME_INSTBAD|GAME_INSTBAD0;
#endif	/* OLD_RETAIL */
	if ( !(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_DEMO|GAME_OLD_DEMO|GAME_OEM)) )
	{
		if ( !(gameflags & GAME_CANPATCH) )
			gameflags |= GAME_INSTBAD|GAME_INSTBAD1;	/* no proper Raven data */
	}

#if !defined(DEMOBUILD)
	FindMaxStringSize ();
	scan_h2_mods ();
	scan_hw_mods ();
#endif	/* DEMOBUILD */

	scan_binaries();
}

