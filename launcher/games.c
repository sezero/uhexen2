/*
	games.c
	hexen2 launcher, game installation scanning

	$Id$

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
		Boston, MA  02110-1301, USA
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
	int	crc;
	const char	*dirname;
} pakdata_t;

static pakdata_t pakdata[] =
{
	{ 696,	34289, "data1"	},	/* pak0.pak, registered	*/
	{ 523,	2995 , "data1"	},	/* pak1.pak, registered	*/
	{ 183,	4807 , "data1"	},	/* pak2.pak, oem, v1.11 */
	{ 245,	1478 , "portals"},	/* pak3.pak, portals	*/
	{ 102,	41062, "hw"	}	/* pak4.pak, hexenworld	*/
};
#define	MAX_PAKDATA	(int)(sizeof(pakdata) / sizeof(pakdata[0]))

static pakdata_t demo_pakdata[] =
{
	{ 797,	22780, "data1"	}	/* pak0.pak, demo v1.11	*/
};

static pakdata_t oem0_pakdata[] =
{
	{ 697,	9787 , "data1"	}	/* pak0.pak, oem, v1.11	*/
};

static pakdata_t old_pakdata[] =
{
	{ 697,	53062, "data1"	},	/* pak0.pak, original cdrom (1.03) version	*/
	{ 525,	47762, "data1"	},	/* pak1.pak, original cdrom (1.03) version	*/
	{ 701,	20870, "data1"	},	/* pak0.pak, Raven's first version of the demo	*/
					/* The old (28.8.1997, v0.42? 1.07?) demo is not supported:
					 * pak0.pak::progs.dat : 19267 crc, progheader crc: 14046 */
/* FIXME: add pak0 and pak2 data for the
 * oem (Matrox m3D bundle) 1.08 original
 * version here.  */
};

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

	if (numpackfiles != pakdata[paknum].numfiles)
	{
		if (paknum == 0)
		{
			/* demo ?? */
			if (crc == demo_pakdata[0].crc &&
			    numpackfiles == demo_pakdata[0].numfiles)
			{
				gameflags |= GAME_DEMO;
			}
			/* oem ?? */
			else if (crc == oem0_pakdata[0].crc &&
				 numpackfiles == oem0_pakdata[0].numfiles)
			{
				gameflags |= GAME_OEM0;
			}
			/* old version of demo ?? */
			else if (crc == old_pakdata[2].crc &&
				 numpackfiles == old_pakdata[2].numfiles)
			{
				gameflags |= GAME_OLD_DEMO;
			}
			/* old, un-patched cdrom version ?? */
			else if (crc == old_pakdata[0].crc &&
				 numpackfiles == old_pakdata[0].numfiles)
			{
				gameflags |= GAME_OLD_CDROM0;
			}
			/*
			 * FIXME: add old oem version pak file
			 * checks here...
			 */
		}
		else if (paknum == 1)
		{
			/* old, un-patched cdrom version ?? */
			if (crc == old_pakdata[1].crc &&
			    numpackfiles == old_pakdata[1].numfiles)
			{
				gameflags |= GAME_OLD_CDROM1;
			}
		}
	}
	else if (crc == pakdata[paknum].crc)
	{
		switch (paknum)
		{
		case 0:	/* pak0 of full version 1.11 */
			gameflags |= GAME_REGISTERED0;
			break;
		case 1:	/* pak1 of full version 1.11 */
			gameflags |= GAME_REGISTERED1;
			break;
		case 2:	/* bundle version */
			gameflags |= GAME_OEM2;
			break;
		case 3:	/* mission pack */
			gameflags |= GAME_PORTALS;
			break;
		case 4:	/* hexenworld */
			gameflags |= GAME_HEXENWORLD;
			break;
		}
	}
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

	if ((gameflags & GAME_OEM2 && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_DEMO|GAME_OLD_DEMO)) ||
	    (gameflags & (GAME_REGISTERED1|GAME_OLD_CDROM1) && gameflags & (GAME_DEMO|GAME_OLD_DEMO|GAME_OEM0|GAME_OEM2)))
		gameflags |= GAME_INSTBAD|GAME_INSTBAD2;	/* mix'n'match: bad	*/

	if (!(gameflags & GAME_INSTBAD2))
	{
		if (gameflags & (GAME_REGISTERED_OLD|GAME_OLD_OEM) ||
		    gameflags & (GAME_CANPATCH0|GAME_CANPATCH1))
			gameflags |= GAME_CANPATCH;	/* 1.11 pak patch can fix this thing	*/
	}

	if (gameflags & (GAME_CANPATCH0|GAME_CANPATCH1))
		gameflags |= GAME_INSTBAD|GAME_INSTBAD2;	/* still a mix'n'match.	*/

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

