/*
 * quakefs.c -- Hexen II filesystem
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

#include "quakedef.h"
#include "pakfile.h"
#include <errno.h>
#ifdef PLATFORM_WINDOWS
#include <io.h>
#endif
#include "filenames.h"

typedef struct
{
	char	name[MAX_QPATH];
	int	filepos, filelen;
} pakfiles_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int	numfiles;
	pakfiles_t	*files;
} pack_t;

typedef struct searchpath_s
{
	unsigned int	path_id;	/* identifier assigned to the game directory
					 *	Note that <install_dir>/game1 and
					 *	<userdir>/game1 have the same id. */
	char		filename[MAX_OSPATH];
	struct pack_s		*pack;	/* only one of filename / pack will be used */
	struct searchpath_s	*next;
} searchpath_t;

static searchpath_t	*fs_searchpaths;
static searchpath_t	*fs_base_searchpaths;	/* without gamedirs */

static const char	*fs_basedir;
static char	fs_gamedir[MAX_OSPATH];
static char	fs_userdir[MAX_OSPATH];
char	fs_gamedir_nopath[MAX_QPATH];

unsigned int	gameflags;

cvar_t	oem = {"oem", "0", CVAR_ROM};
cvar_t	registered = {"registered", "0", CVAR_ROM};

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
	{  40,	48258,	 3357888, "hw"	},		/* pak4.pak, Hexen2World v0.09 (ugh ugh ugh!)
							 *	MD5: 7708da4323f668cf9c71f99315704baa	*/
};

/* this graphic needs to be in the pak file to use registered features */
static const unsigned short pop[] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x6600, 0x0000, 0x0000, 0x0000, 0x6600, 0x0000,
	0x0000, 0x0066, 0x0000, 0x0000, 0x0000, 0x0000, 0x0067, 0x0000,
	0x0000, 0x6665, 0x0000, 0x0000, 0x0000, 0x0000, 0x0065, 0x6600,
	0x0063, 0x6561, 0x0000, 0x0000, 0x0000, 0x0000, 0x0061, 0x6563,
	0x0064, 0x6561, 0x0000, 0x0000, 0x0000, 0x0000, 0x0061, 0x6564,
	0x0064, 0x6564, 0x0000, 0x6469, 0x6969, 0x6400, 0x0064, 0x6564,
	0x0063, 0x6568, 0x6200, 0x0064, 0x6864, 0x0000, 0x6268, 0x6563,
	0x0000, 0x6567, 0x6963, 0x0064, 0x6764, 0x0063, 0x6967, 0x6500,
	0x0000, 0x6266, 0x6769, 0x6a68, 0x6768, 0x6a69, 0x6766, 0x6200,
	0x0000, 0x0062, 0x6566, 0x6666, 0x6666, 0x6666, 0x6562, 0x0000,
	0x0000, 0x0000, 0x0062, 0x6364, 0x6664, 0x6362, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0062, 0x6662, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0061, 0x6661, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x6500, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x6400, 0x0000, 0x0000, 0x0000
};

static char *FSERR_MakePath_BUF (const char *caller, int linenum, int base,
				 char *buf, size_t siz, const char *path);
static char *FSERR_MakePath_VABUF (const char *caller, int linenum, int base,
				char *buf, size_t siz, const char *format, ...)
				__attribute__((__format__(__printf__,6,7)));
static char *do_MakePath_VA (int base, int *error, char *buf, size_t siz,
					const char *format, va_list args)
				__attribute__((__format__(__printf__,5,0)));

/*
All of Quake's data access is through a hierchal file system, but the contents
of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the exe and all game
directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.
This can be overridden with the "-basedir" command line parm to allow code
debugging in a different directory.  The base directory is only used during
filesystem initialization.

The "game directory" is the first tree on the search path and directory that
all generated files (savegames, screenshots, demos, config files) will be saved
to.  This can be overridden with the "-game" command line parameter.  The game
directory can never be changed while quake is executing.  This is a precacution
against having a malicious server instruct clients to write files over areas
they shouldn't.
*/


static unsigned int check_known_paks (int paknum, int numfiles, unsigned short crc)
{
	if (paknum >= MAX_PAKDATA)
		return GAME_MODIFIED;

	if (strcmp(fs_gamedir_nopath, pakdata[paknum].dirname) != 0)
		return GAME_MODIFIED;	/* Raven didn't ship like that */

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

/*
=================
FS_LoadPackFile

Takes a path to a pak file.  Loads the header and directory.
=================
*/
static pack_t *FS_LoadPackFile (const char *packfile, int paknum, qboolean base_fs)
{
	dpackheader_t	header;
	int	i, numpackfiles;
	pakfiles_t	*newfiles;
	pack_t		*pack;
	FILE		*packhandle;
	dpackfile_t	info[MAX_FILES_IN_PACK];
	unsigned short	crc;

	packhandle = fopen (packfile, "rb");
	if (!packhandle)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (header.id[0] != 'P' || header.id[1] != 'A' ||
	    header.id[2] != 'C' || header.id[3] != 'K')
	{
		Sys_Printf ("WARNING: %s is not a packfile, ignored\n", packfile);
		goto pak_error;
	}

	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (header.dirlen < 0 || header.dirofs < 0)
	{
		Sys_Error ("Invalid packfile %s (dirlen: %i, dirofs: %i)",
					packfile, header.dirlen, header.dirofs);
	}
	if (!numpackfiles)
	{
		Sys_Printf ("WARNING: %s has no files, ignored\n", packfile);
		goto pak_error;
	}
	if (numpackfiles > MAX_FILES_IN_PACK)
	{
		Sys_Printf ("WARNING: %s has %i files (max. allowed is %i), ignored\n",
					packfile, numpackfiles, MAX_FILES_IN_PACK);
		goto pak_error;
	}

	newfiles = (pakfiles_t *) Z_Malloc (numpackfiles * sizeof(pakfiles_t), Z_MAINZONE);

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info,  1, header.dirlen, packhandle);

	/* crc the directory */
	CRC_Init (&crc);
	for (i = 0; i < header.dirlen; i++)
		CRC_ProcessByte (&crc, ((byte *)info)[i]);

	/* check for modifications */
	if (base_fs)
		gameflags |= check_known_paks (paknum, numpackfiles, crc);
	else	gameflags |= GAME_MODIFIED;

	/* parse the directory */
	for (i = 0; i < numpackfiles; i++)
	{
		qerr_strlcpy(__thisfunc__, __LINE__, newfiles[i].name, info[i].name, MAX_QPATH);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = (pack_t *) Z_Malloc (sizeof(pack_t), Z_MAINZONE);
	qerr_strlcpy(__thisfunc__, __LINE__, pack->filename, packfile, MAX_OSPATH);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	Sys_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
pak_error:
	fclose (packhandle);
	return NULL;
}


/*
================
FS_AddGameDirectory

Sets fs_gamedir, fs_userdir and fs_gamedir_nopath.  adds the directory
to the head of the path, then loads and adds pak1.pak pak2.pak ...
================
*/
static void FS_AddGameDirectory (const char *dir, qboolean base_fs)
{
	unsigned int	path_id;
	searchpath_t	*search;
	pack_t		*pak;
	char	pakfile[MAX_OSPATH];
	qboolean do_userdir = false;
	int	i;

	qerr_strlcpy(__thisfunc__, __LINE__, fs_gamedir_nopath, dir,
						sizeof(fs_gamedir_nopath));
	FSERR_MakePath_BUF (__thisfunc__, __LINE__, FS_BASEDIR,
				fs_gamedir, sizeof(fs_gamedir), dir);
	FSERR_MakePath_BUF (__thisfunc__, __LINE__, FS_USERBASE,
				fs_userdir, sizeof(fs_userdir), dir);

/* assign a path_id to this game directory */
	if (fs_searchpaths)
		path_id = fs_searchpaths->path_id << 1;
	else	path_id = 1U;

#if DO_USERDIRS
add_pakfile:
#endif
/* add any pak files in the format pak0.pak pak1.pak, ...
 * unlike Quake, Hexen II can't stop at first unavailable
 * pak: the mission pack has only pak3, hw has only pak4.
 */
	for (i = 0; i < 10; i++)
	{
		FSERR_MakePath_VABUF (__thisfunc__, __LINE__,
					(do_userdir) ? FS_USERDIR : FS_GAMEDIR,
					pakfile, sizeof(pakfile), "pak%i.pak", i);
		pak = FS_LoadPackFile (pakfile, i, base_fs);
		if (!pak) continue;
		search = (searchpath_t *) Z_Malloc (sizeof(searchpath_t), Z_MAINZONE);
		search->path_id = path_id;
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}

/* add the directory itself to the search path.  unlike Quake,
 * Hexen II does this ~after~ adding the pakfiles in this dir,
 * so that the dir itself will be placed above the pakfiles in
 * the search order which, in turn, will allow override files.
 */
	search = (searchpath_t *) Z_Malloc (sizeof(searchpath_t), Z_MAINZONE);
	if (do_userdir)
		qerr_strlcpy(__thisfunc__, __LINE__, search->filename, fs_userdir, MAX_OSPATH);
	else	qerr_strlcpy(__thisfunc__, __LINE__, search->filename, fs_gamedir, MAX_OSPATH);
	search->path_id = path_id;
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	if (do_userdir)
		return;
	do_userdir = true;

#if DO_USERDIRS
/* add user's directory to the search path and
 * add any pak files in the user's directory.
 */
	if (strcmp(fs_gamedir, fs_userdir))
	{
		Sys_mkdir (fs_userdir, true);
		goto add_pakfile;
	}
#endif
}

/*
================
FS_Gamedir

Sets the gamedir and path to a different directory.

Hexen II uses this for setting the game directory from a -game
command line argument.

HexenWorld uses this to set the gamedir on both server and client
sides while the game is running: The client calls this upon every
map change from within CL_ParseServerData(), and the Server calls
this upon a gamedir command from within SV_Gamedir_f().
================
*/
void FS_Gamedir (const char *dir)
{
	searchpath_t	*next;

	if (!*dir || !strcmp(dir, ".") || strstr(dir, "..") || strstr(dir, "/") || strstr(dir, "\\") || strstr(dir, ":"))
	{
		if (!host_initialized)
			Sys_Error ("gamedir should be a single directory name, not a path\n");
		else {
			Con_Printf("gamedir should be a single directory name, not a path\n");
			return;
		}
	}

	if (!q_strcasecmp(fs_gamedir_nopath, dir))
		return;		/* still the same */

/* free up any current game dir info: our top searchpath dir will be hw
 * and any gamedirs set before by this very procedure will be removed.
 * since hexen2 doesn't use this during game execution there will be no
 * changes for it: it has portals or data1 at the top.
 */
	while (fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack)
		{
			fclose (fs_searchpaths->pack->handle);
			Z_Free (fs_searchpaths->pack->files);
			Z_Free (fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		Z_Free (fs_searchpaths);
		fs_searchpaths = next;
	}

/* flush all data, so it will be forced to reload */
#if !defined(SERVERONLY)
	Cache_Flush ();
#endif	/* SERVERONLY */

/* check for reserved gamedirs */
	if (!q_strcasecmp(dir, "hw"))
	{
#if defined(H2W)
	/* that we reached here means the hw server decided to abandon
	 * whatever the previous mod it was running and went back to
	 * pure hw. weird.. do as he wishes anyway and adjust our variables. */
	_do_hw:
		qerr_strlcpy(__thisfunc__, __LINE__, fs_gamedir_nopath, "hw",
						sizeof(fs_gamedir_nopath));
		FSERR_MakePath_BUF (__thisfunc__, __LINE__, FS_BASEDIR,
					fs_gamedir, sizeof(fs_gamedir), "hw");
		FSERR_MakePath_BUF (__thisfunc__, __LINE__, FS_USERBASE,
					fs_userdir, sizeof(fs_userdir), "hw");
# ifdef SERVERONLY
	/* change the *gamedir serverinfo properly */
		Info_SetValueForStarKey (svs.info, "*gamedir", "hw", MAX_SERVERINFO_STRING);
# endif /* HWSV */
#else	/* hexen2 case: */
	/* hw is reserved for hexenworld only. hexen2 shouldn't use it */
		Con_Printf ("WARNING: Gamedir not set to hw :\n"
			    "It is reserved for HexenWorld.\n");
#endif	/* H2W */
		return;
	}

	if (!q_strcasecmp(dir, "portals"))
	{
	/* no hw server is supposed to set gamedir to portals
	 * and hw must be above portals in hierarchy. this is
	 * actually a hypothetical case.
	 * as for hexen2, it cannot reach here.  */
#ifdef H2W
		goto _do_hw;
#endif
		return;
	}

	if (!q_strcasecmp(dir, "data1"))
	{
	/* another hypothetical case: no hw mod is supposed to
	 * do this and hw must stay above data1 in hierarchy.
	 * as for hexen2, it can only reach here by a silly
	 * command line argument like -game data1, ignore it. */
#ifdef H2W
		goto _do_hw;
#endif
		return;
	}

/* a new gamedir: let's set it here. */
	FS_AddGameDirectory(dir, false);
#if defined(H2W) && defined(SERVERONLY)
/* change the *gamedir serverinfo properly */
	Info_SetValueForStarKey (svs.info, "*gamedir", dir, MAX_SERVERINFO_STRING);
#endif
}


/*
==============================================================================

FILE I/O within QFS

==============================================================================
*/

size_t		fs_filesize;	/* size of the last file opened through QFS */
int		file_from_pak;	/* ZOID: global indicating that file came from a pak */


/*
===========
FS_CopyFile

Copies the FROMPATH file as TOPATH file, creating any dirs needed.
Used for saving the game. Returns 0 on success, non-zero on error.
===========
*/
int FS_CopyFile (const char *frompath, const char *topath)
{
	char	*tmp;
	int	err;

	if (!frompath || !topath)
	{
		Con_Printf ("%s: null input\n", __thisfunc__);
		return 1;
	}
	/* create directories up to the dest file */
	tmp = Z_Strdup (topath);
	err = FS_CreatePath(tmp);
	Z_Free (tmp);
	if (err != 0)
	{
		Con_Printf ("%s: unable to create directory\n", __thisfunc__);
		return err;
	}

	err = Sys_CopyFile (frompath, topath);
	return err;
}

#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int FS_WriteFileFromHandle (FILE *fromfile, const char *topath, size_t size)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*out;
/*	off_t	remaining, count;*/
	size_t	remaining, count;
	char	*tmp;
	int	err;

	if (!fromfile || !topath)
	{
		Con_Printf ("%s: null input\n", __thisfunc__);
		return 1;
	}

	/* create directories up to the dest file */
	tmp = Z_Strdup (topath);
	err = FS_CreatePath(tmp);
	Z_Free (tmp);
	if (err != 0)
	{
		Con_Printf ("%s: unable to create directory\n", __thisfunc__);
		return err;
	}

	out = fopen (topath, "wb");
	if (!out)
	{
		Con_Printf ("%s: unable to create %s\n", topath, __thisfunc__);
		return 1;
	}

	remaining = size;
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else	count = sizeof(buf);

		if (fread(buf, 1, count, fromfile) != count)
			break;
		if (fwrite(buf, 1, count, out) != count)
			break;

		remaining -= count;
	}

	fclose (out);

	return (remaining == 0)? 0 : 1;
}

/*
============
FS_WriteFile

The filename will be prefixed by the current game directory
Returns 0 on success, 1 on error.
============
*/
int FS_WriteFile (const char *filename, const void *data, size_t len)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	size_t	size;
	int	err;

	FS_MakePath_BUF (FS_USERDIR, &err, name, sizeof(name), filename);
	if (err)
	{
		Host_Error("%s: %d: string buffer overflow!", __thisfunc__, __LINE__);
		return 1;
	}

	f = fopen (name, "wb");
	if (!f)
	{
		Con_Printf ("Error opening %s\n", filename);
		return 1;
	}

	Sys_Printf ("%s: %s\n", __thisfunc__, name);
	size = fwrite (data, 1, len, f);
	fclose (f);
	if (size != len)
	{
		Con_Printf ("Error in writing %s\n", filename);
		return 1;
	}
	return 0;
}


/*
============
FS_CreatePath

Creates directory under user's path, making parent directories
as needed. The path must either be a path to a file, or, if the
full path is meant to be created, it must have the trailing path
seperator. Returns 0 on success, non-zero on error.
============
*/
int FS_CreatePath (char *path)
{
	char	*ofs, c;
	int	err = 0;
	size_t	offset;

	if (!path || !*path)
	{
		Con_Printf ("%s: no path!\n", __thisfunc__);
		return 1;
	}

	if (strstr(path, ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return 1;
	}

	offset = strlen(host_parms->userdir);
	if (offset && strstr(path, host_parms->userdir) != path)
	{
		Sys_Error ("Attempted to create a directory out of user's path");
		return 1;
	}

	ofs = path + offset;
	if (*ofs == '\0')	/* not necessarily an error. */
		return 0;
	/* check for the path separator after the userdir. */
	if (IS_DIR_SEPARATOR(*ofs))
		ofs++;
	else if (offset)
	{
		/* if the userdir itself has no trailing DIRSEP
		 * either, then it is a bad path: */
		if (!IS_DIR_SEPARATOR(host_parms->userdir[offset - 1]))
		{
			Con_Printf ("%s: bad path\n", __thisfunc__);
			return 1;
		}
	}

	for ( ; *ofs; ofs++)
	{
		c = *ofs;
		if (IS_DIR_SEPARATOR(c))
		{	/* create the directory */
			*ofs = 0;
			err = Sys_mkdir (path, false);
			*ofs = c;
			if (err)
				break;
		}
	}

	return err;
}


/*
===========
FS_OpenFile

Finds the file in the search path, returns fs_filesize.
===========
*/
size_t FS_OpenFile (const char *filename, FILE **file, unsigned int *path_id)
{
	searchpath_t	*search;
	pack_t		*pak;
	char	ospath[MAX_OSPATH];
	int	i;

	file_from_pak = 0;

	/* search through the path, one element at a time */
	for (search = fs_searchpaths ; search ; search = search->next)
	{
		if (search->pack)	/* look through all the pak file elements */
		{
			pak = search->pack;
			for (i = 0; i < pak->numfiles; i++)
			{
				if (strcmp(pak->files[i].name, filename) != 0)
					continue;
				/* found it! */
				fs_filesize = (size_t) pak->files[i].filelen;
				file_from_pak = 1;
				if (path_id)
					*path_id = search->path_id;
				if (!file) /* for FS_FileExists() */
					return fs_filesize;
				/* open a new file on the pakfile */
				*file = fopen (pak->filename, "rb");
				if (!*file)
					Sys_Error ("Couldn't reopen %s", pak->filename);
				fseek (*file, pak->files[i].filepos, SEEK_SET);
				return fs_filesize;
			}
		}
		else	/* check a file in the directory tree */
		{
			q_snprintf (ospath, sizeof(ospath), "%s/%s",search->filename, filename);
			fs_filesize = (size_t) Sys_filesize (ospath);
			if (fs_filesize == (size_t)-1)
				continue;
			if (path_id)
				*path_id = search->path_id;
			if (!file) /* for FS_FileExists() */
				return fs_filesize;
			*file = fopen (ospath, "rb");
			if (!*file)
				Sys_Error ("Couldn't reopen %s", ospath);
			return fs_filesize;
		}
	}

	Sys_DPrintf ("%s: can't find %s\n", __thisfunc__, filename);

	if (file) *file = NULL;
	fs_filesize = (size_t)-1;
	return fs_filesize;
}

/*
===========
FS_FileExists

Returns whether the file is found in the hexen2 filesystem.
===========
*/
qboolean FS_FileExists (const char *filename, unsigned int *path_id)
{
	size_t ret = FS_OpenFile (filename, NULL, path_id);
	return (ret == (size_t)-1) ? false : true;
}

/*
============
FS_FileInGamedir

Reports the existance of a file with read permissions in
fs_gamedir or fs_userdir. *NOT* for files in pakfiles!
============
*/
qboolean FS_FileInGamedir (const char *filename)
{
	int	ret;

	ret = Sys_FileType(FS_MakePath(FS_USERDIR, NULL, filename));
	if (ret & FS_ENT_FILE)
		return true;
	ret = Sys_FileType(FS_MakePath(FS_GAMEDIR, NULL, filename));
	if (ret & FS_ENT_FILE)
		return true;

	return false;
}

/*
============
FS_LoadFile

Filename are reletive to the quake directory.
Allways appends a 0 byte to the loaded data.
============
*/
#define	LOADFILE_ZONE		0
#define	LOADFILE_HUNK		1
#define	LOADFILE_TEMPHUNK	2
#define	LOADFILE_CACHE		3
#define	LOADFILE_STACK		4
#define	LOADFILE_MALLOC		5

static byte	*loadbuf;
#if !defined(SERVERONLY)
static cache_user_t *loadcache;
#endif	/* SERVERONLY */
static size_t		loadsize;
static int		zone_num;

#if defined (SERVERONLY)
#define Draw_BeginDisc()
#define Draw_EndDisc()
#endif	/* SERVERONLY */

static byte *FS_LoadFile (const char *path, int usehunk, unsigned int *path_id)
{
	FILE	*h;
	byte	*buf;
	char	base[32];
	size_t	len;

/* look for it in the filesystem or pack files */
	len = FS_OpenFile (path, &h, path_id);
	if (!h)
		return NULL;

/* extract the file's base name for hunk tag */
	COM_FileBase (path, base, sizeof(base));
	buf = NULL;	/* quiet compiler warning */

	switch (usehunk)
	{
	case LOADFILE_HUNK:
		buf = (byte *) Hunk_AllocName (len+1, base);
		break;
	case LOADFILE_TEMPHUNK:
		buf = (byte *) Hunk_TempAlloc (len+1);
		break;
	case LOADFILE_ZONE:
		buf = (byte *) Z_Malloc (len+1, zone_num);
		break;
#if !defined(SERVERONLY)
	case LOADFILE_CACHE:
		buf = (byte *) Cache_Alloc (loadcache, len+1, base);
		break;
#endif	/* SERVERONLY */
	case LOADFILE_STACK:
		if (len < loadsize)
			buf = loadbuf;
		else
			buf = (byte *) Hunk_TempAlloc (len+1);
		break;
	case LOADFILE_MALLOC:
		buf = (byte *) malloc (len+1);
		break;
	default:
		Sys_Error ("%s: bad usehunk", __thisfunc__);
	}

	if (!buf)
		Sys_Error ("%s: not enough space for %s", __thisfunc__, path);

	((byte *)buf)[len] = 0;

	Draw_BeginDisc ();
	fread (buf, 1, len, h);
	fclose (h);
	Draw_EndDisc ();

	return buf;
}

byte *FS_LoadHunkFile (const char *path, unsigned int *path_id)
{
	return FS_LoadFile (path, LOADFILE_HUNK, path_id);
}

byte *FS_LoadZoneFile (const char *path, int zone_id, unsigned int *path_id)
{
	zone_num = zone_id;
	return FS_LoadFile (path, LOADFILE_ZONE, path_id);
}

byte *FS_LoadTempFile (const char *path, unsigned int *path_id)
{
	return FS_LoadFile (path, LOADFILE_TEMPHUNK, path_id);
}

#if !defined(SERVERONLY)
void FS_LoadCacheFile (const char *path, struct cache_user_s *cu, unsigned int *path_id)
{
	loadcache = cu;
	FS_LoadFile (path, LOADFILE_CACHE, path_id);
}
#endif	/* SERVERONLY */

/* uses temp hunk if larger than bufsize */
byte *FS_LoadStackFile (const char *path, void *buffer, size_t bufsize, unsigned int *path_id)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = FS_LoadFile (path, LOADFILE_STACK, path_id);

	return buf;
}

/* returns malloc'd memory */
byte *FS_LoadMallocFile (const char *path, unsigned int *path_id)
{
	return FS_LoadFile (path, LOADFILE_MALLOC, path_id);
}


/*
==============================================================================

MISC CONSOLE COMMANDS

==============================================================================
*/

/*
============
FS_Path_f
Prints the search path to the console
============
*/
static void FS_Path_f (void)
{
	searchpath_t	*s;

	Con_Printf ("Current search path:\n");
	for (s = fs_searchpaths ; s ; s = s->next)
	{
		if (s == fs_base_searchpaths)
			Con_Printf ("----------\n");
		if (s->pack)
			Con_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Con_Printf ("%s\n", s->filename);
	}
}

/*
===========
processMapname: Callback for FS_Maplist_f.
Returns 0 if a name is skipped, the current
number of names added to the list if the name
is added, or -1 upon failures.
===========
*/
#if !defined(SERVERONLY)	/* dedicated servers dont need this command */

#define MAX_NUMMAPS	256	/* max number of maps to list */
static int	map_count = 0;
static char	*maplist[MAX_NUMMAPS];

static int processMapname (const char *mapname, const char *partial, size_t len_partial)
{
	char	cur_name[MAX_QPATH];
	int	j, len;

	if (map_count >= MAX_NUMMAPS)
	{
		Con_Printf ("WARNING: reached maximum number of maps to list\n");
		return -1;
	}

	if (len_partial)
	{
		if (q_strncasecmp(partial, mapname, len_partial) != 0)
			return 0;	/* doesn't match the prefix. skip. */
	}

	len = q_strlcpy (cur_name, mapname, sizeof(cur_name)) - 4; /* -4 to kill ".bsp" */
	if (len <= 0)
		return 0;
	if (q_strcasecmp(&cur_name[len], ".bsp") != 0)
		return 0;

	cur_name[len] = 0;

	for (j = 0; j < map_count; j++)
	{
		if (! q_strcasecmp(maplist[j], cur_name))
			return 0;	/* duplicated name. skip. */
	}

	/* add to the maplist */
	maplist[map_count] = Z_Strdup (cur_name);
	return (++map_count);
}

/*
===========
FS_Maplist_f
Prints map filenames to the console
===========
*/
static void FS_Maplist_f (void)
{
	searchpath_t	*search;
	const char	*prefix;
	size_t		preLen;

	if (Cmd_Argc() > 1)
	{
		prefix = Cmd_Argv(1);
		preLen = strlen(prefix);
	}
	else
	{
		preLen = 0;
		prefix = NULL;
	}

	for (search = fs_searchpaths; search; search = search->next)
	{
		if (search->pack)
		{
			int i = 0;
			for (; i < search->pack->numfiles; ++i)
			{
				if (strncmp("maps/", search->pack->files[i].name, 5) != 0)
					continue;
				if (processMapname(search->pack->files[i].name + 5, prefix, preLen) < 0)
					goto done;
			}
		}
		else
		{
			const char *findname = Sys_FindFirstFile(va("%s/maps",search->filename), "*.bsp");
			while (findname)
			{
				if (processMapname(findname, prefix, preLen) < 0)
				{
					Sys_FindClose ();
					goto done;
				}
				findname = Sys_FindNextFile ();
			}
			Sys_FindClose ();
		}
	}

done:
	if (!map_count)
	{
		Con_Printf ("No maps found.\n\n");
		return;
	}

	Con_Printf ("Found %d maps:\n\n", map_count);
	/* sort the list */
	if (map_count > 1)
		qsort (maplist, map_count, sizeof(char *), COM_StrCompare);
	Con_ShowList (map_count, (const char**)maplist);
	Con_Printf ("\n");

	/* free the memory and zero map_count */
	while (map_count)
		Z_Free (maplist[--map_count]);
}
#endif	/* SERVERONLY */


/*
==============================================================================

INIT

==============================================================================
*/

/*
================
CheckRegistered

Looks for the pop.txt file and verifies it.
Sets the registered flag.
================
*/
static int CheckRegistered (void)
{
	FILE	*h;
	unsigned short	check[128];
	int	i;

	FS_OpenFile("gfx/pop.lmp", &h, NULL);
	if (!h)
		return -1;

	fread (check, 1, sizeof(check), h);
	fclose (h);

	for (i = 0; i < 128; i++)
	{
		if (pop[i] != (unsigned short)BigShort(check[i]))
		{
			Sys_Printf ("Corrupted data file\n");
			return -1;
		}
	}

	return 0;
}

/*
================
FS_Init
================
*/
void FS_Init (void)
{
	qboolean check_portals = false;
	int	i;

	Cvar_RegisterVariable (&oem);
	Cvar_RegisterVariable (&registered);

	Cmd_AddCommand ("path", FS_Path_f);
#if !defined(SERVERONLY)
	Cmd_AddCommand ("maplist", FS_Maplist_f);
#endif	/* SERVERONLY */

/* -basedir <path> overrides the system supplied base directory */
	i = COM_CheckParm ("-basedir");
	if (i && i < com_argc-1)
	{
		fs_basedir = com_argv[i+1];
		if (!*fs_basedir) Sys_Error("Bad argument to -basedir");
#if !DO_USERDIRS
		host_parms->userdir = com_argv[i+1];
#endif
		Sys_Printf ("%s: basedir changed to: %s\n", __thisfunc__, fs_basedir);
	}
	else
	{
		fs_basedir = host_parms->basedir;
	}

/* step 1: start up with data1 by default */
	FS_AddGameDirectory ("data1", true);

	if (gameflags & GAME_REGISTERED0 && gameflags & GAME_REGISTERED1)
		gameflags |= GAME_REGISTERED;
	if (gameflags & GAME_OEM0 && gameflags & GAME_OEM2)
		gameflags |= GAME_OEM;
	if (gameflags & GAME_OLD_CDROM0 && gameflags & GAME_OLD_CDROM1)
		gameflags |= GAME_REGISTERED_OLD;
	if (gameflags & GAME_OLD_OEM0 && gameflags & GAME_OLD_OEM2)
		gameflags |= GAME_OLD_OEM;
	/* check for bad installations (mix'n'match data): */
	if ((gameflags & GAME_REGISTERED0 && gameflags & GAME_OLD_CDROM1) ||
	    (gameflags & GAME_REGISTERED1 && gameflags & GAME_OLD_CDROM0) ||
	    (gameflags & (GAME_OEM2|GAME_OLD_OEM2) && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_DEMO|GAME_OLD_DEMO)) ||
	    (gameflags & (GAME_REGISTERED1|GAME_OLD_CDROM1) &&
					 gameflags & (GAME_DEMO|GAME_OLD_DEMO|GAME_OEM0|GAME_OLD_OEM0|GAME_OEM2|GAME_OLD_OEM2)))
	{
		Sys_Error ("Bad Hexen II installation: mixed data from incompatible versions");
	}
#if !ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
		Sys_Error ("Old version of Hexen II demo isn't supported");
#endif	/* OLD_DEMO */
#if !ENABLE_OLD_RETAIL
	/* check if we have 1.11 versions of pak0.pak and pak1.pak */
	if (gameflags & (GAME_OLD_CDROM0|GAME_OLD_CDROM1|GAME_OLD_OEM0|GAME_OLD_OEM2))
		Sys_Error ("You must patch your installation with Raven's 1.11 update");
#endif	/* OLD_RETAIL */

	/* finish the base filesystem setup */
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
	{
		Cvar_SetROM ("registered", "1");
		Sys_Printf ("Playing the registered version.\n");
	}
	else if (gameflags & GAME_OEM)
	{
		Cvar_SetROM ("oem", "1");
		Sys_Printf ("Playing the oem (Matrox m3D bundle) version \"Continent of Blackmarsh\"\n");
	}
	else if (gameflags & (GAME_DEMO|GAME_OLD_DEMO))
	{
		Sys_Printf ("Playing the demo version.\n");
	}
	else
	{
	/* no proper Raven data: it's best to error out here */
		Sys_Error ("Unable to find a proper Hexen II installation");
	}
	if (gameflags & (GAME_OLD_DEMO|GAME_REGISTERED_OLD|GAME_OLD_OEM))
		Sys_Printf ("Using old/unsupported, pre-1.11 version pak files.\n");
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
	{
		if (CheckRegistered() != 0)
			Sys_Error ("Unable to verify retail version data.");
	}
#if !(defined(H2W) && defined(SERVERONLY))
	if (gameflags & GAME_MODIFIED && !(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		Sys_Error ("You must have the full version of Hexen II to play modified games");
#endif

/* step 2: portals directory (mission pack) */
#if defined(H2MP)
	if (! COM_CheckParm ("-noportals"))
		check_portals = true;
	if (check_portals && !(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		Sys_Error ("Portal of Praevus requires registered version of Hexen II");
#elif defined(H2W)
	if (! COM_CheckParm ("-noportals") && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
		check_portals = true;
#else
	/* see if the user wants mission pack support */
	check_portals = (COM_CheckParm ("-portals")) ||
			(COM_CheckParm ("-missionpack")) ||
			(COM_CheckParm ("-h2mp"));
	i = COM_CheckParm ("-game");
	if (i && i < com_argc-1)
	{
		if (!q_strcasecmp(com_argv[i+1], "portals"))
			check_portals = true;
	}
	if (check_portals && !(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		Sys_Error ("Portal of Praevus requires registered version of Hexen II");
#endif
#if !defined(H2W)
	if (sv_protocol == PROTOCOL_RAVEN_111 && check_portals)
		Sys_Error ("Old protocol request not compatible with the Mission Pack");
#endif

	if (check_portals)
	{
#if defined(H2W)
		searchpath_t	*mark = fs_searchpaths;
#endif
		FS_AddGameDirectory ("portals", true);
		if (! (gameflags & GAME_PORTALS))
		{
#if !defined(H2W)
			Sys_Error ("Missing or invalid mission pack installation\n");
#else
			/* back out searchpaths from invalid mission pack
			 * installations because the portals directory is
			 * reserved for the mission pack */
			searchpath_t	*next;
			Sys_Printf ("Missing or invalid mission pack installation\n");
			while (fs_searchpaths != mark)
			{
				if (fs_searchpaths->pack)
				{
					fclose (fs_searchpaths->pack->handle);
					Z_Free (fs_searchpaths->pack->files);
					Z_Free (fs_searchpaths->pack);
					Sys_Printf ("Removed packfile %s\n", fs_searchpaths->pack->filename);
				}
				else
				{
					Sys_Printf ("Removed path %s\n", fs_searchpaths->filename);
				}
				next = fs_searchpaths->next;
				Z_Free (fs_searchpaths);
				fs_searchpaths = next;
			}
			fs_searchpaths = mark;
			/* back to data1 */
			FS_MakePath_BUF (FS_BASEDIR, NULL, fs_gamedir, sizeof(fs_gamedir), "data1");
			FS_MakePath_BUF (FS_USERBASE,NULL, fs_userdir, sizeof(fs_userdir), "data1");
#endif	/* H2W */
		}
	}

/* step 3: hw directory (hexenworld) */
#if defined(H2W)
	FS_AddGameDirectory ("hw", true);
	/* error out if GAME_HEXENWORLD isn't set */
	if (!(gameflags & GAME_HEXENWORLD))
		Sys_Error ("You must have the HexenWorld data installed");
#endif	/* H2W */

/* this is the end of our base searchpath:
 * any set gamedirs, such as those from -game commandline
 * arguments, from exec'ed configs or the ones dictated by
 * the server, will be freed up to here upon a new gamedir
 * command */
	fs_base_searchpaths = fs_searchpaths;

	i = COM_CheckParm ("-game");
	if (i != 0)
	{
		/* only registered versions can do -game */
		if (! (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
			Sys_Error ("You must have the full version of Hexen II to play modified games");
		/* add basedir/gamedir as an override game */
		if (i < com_argc - 1)
			FS_Gamedir (com_argv[i+1]);
	}
}

#define	FS_NUM_BUFFS	4
#define	FS_BUFFERLEN	1024

static char *get_fs_buffer(void)
{
	static char fs_buffers[FS_NUM_BUFFS][FS_BUFFERLEN];
	static int buffer_idx = 0;
	buffer_idx = (buffer_idx + 1) & (FS_NUM_BUFFS - 1);
	return fs_buffers[buffer_idx];
}

static int init_MakePath (int base, char *buf, size_t siz)
{
	int	len;

	switch (base)
	{
	case FS_USERDIR:
		len = q_strlcpy(buf, fs_userdir, siz);
		break;
	case FS_GAMEDIR:
		len = q_strlcpy(buf, fs_gamedir, siz);
		break;
	case FS_USERBASE:
		len = q_strlcpy(buf, host_parms->userdir, siz);
		break;
	case FS_BASEDIR:
		len = q_strlcpy(buf, fs_basedir, siz);
		break;
	default:
		Sys_Error ("%s: Bad FS_BASE", __thisfunc__);
		return -1;
	}

	if (len >= (int)siz - 1)
		return -1;
	if (len && !IS_DIR_SEPARATOR(buf[len - 1]))
		buf[len++] = DIR_SEPARATOR_CHAR;
	buf[len] = '\0';
	return len;
}

static char *do_MakePath (int base, int *error, char *buf, size_t siz, const char *path)
{
	int	len;

	len = init_MakePath(base, buf, siz);
	if (len < 0) goto _bad;

	len = q_strlcat(buf, path, siz);
	if (len < (int)siz) {
		if (error) *error = 0;
	} else {
	_bad:
		if (error) *error = 1;
		Con_DPrintf("%s: overflow (string truncated)\n", __thisfunc__);
	}

	return buf;
}

static char *do_MakePath_VA (int base, int *error, char *buf, size_t siz,
					const char *format, va_list args)
{
	int	len, ret;

	len = init_MakePath(base, buf, siz);
	if (len < 0) goto _bad;

	ret = q_vsnprintf(&buf[len], siz - len, format, args);
	if (ret < (int)siz - len) {
		if (error) *error = 0;
	} else {
	_bad:
		if (error) *error = 1;
		Con_DPrintf("%s: overflow (string truncated)\n", __thisfunc__);
	}

	return buf;
}

static char *FSERR_MakePath_BUF (const char *caller, int linenum, int base,
				 char *buf, size_t siz, const char *path)
{
	int	err;
	char	*p;

	p = do_MakePath(base, &err, buf, siz, path);

	if (err) Sys_Error("%s: %d: string buffer overflow!", caller, linenum);
	return p;
}

static char *FSERR_MakePath_VABUF (const char *caller, int linenum, int base,
				char *buf, size_t siz, const char *format, ...)
{
	va_list	argptr;
	int	err;
	char	*p;

	va_start (argptr, format);
	p = do_MakePath_VA(base, &err, buf, siz, format, argptr);
	va_end (argptr);

	if (err) Sys_Error("%s: %d: string buffer overflow!", caller, linenum);
	return p;
}

char *FS_MakePath (int base, int *error, const char *path)
{
	return do_MakePath(base, error, get_fs_buffer(), FS_BUFFERLEN, path);
}

char *FS_MakePath_BUF (int base, int *error, char *buf, size_t siz, const char *path)
{
	return do_MakePath(base, error, buf, siz, path);
}

char *FS_MakePath_VA (int base, int *error, const char *format, ...)
{
	va_list	argptr;
	char	*p;

	va_start (argptr, format);
	p = do_MakePath_VA(base, error, get_fs_buffer(), FS_BUFFERLEN, format, argptr);
	va_end (argptr);

	return p;
}

char *FS_MakePath_VABUF (int base, int *error, char *buf, size_t siz, const char *format, ...)
{
	va_list	argptr;
	char	*p;

	va_start (argptr, format);
	p = do_MakePath_VA(base, error, buf, siz, format, argptr);
	va_end (argptr);

	return p;
}

const char *FS_GetBasedir (void)
{
	return fs_basedir;
}

const char *FS_GetUserbase (void)
{
	return host_parms->userdir;
}

const char *FS_GetGamedir (void)
{
	return fs_gamedir;
}

const char *FS_GetUserdir (void)
{
	return fs_userdir;
}

/* The following FS_*() stdio replacements are necessary if one is
 * to perform non-sequential reads on files reopened on pak files
 * because we need the bookkeeping about file start/end positions.
 * Allocating and filling in the fshandle_t structure is the users'
 * responsibility when the file is initially opened. */

size_t FS_fread(void *ptr, size_t size, size_t nmemb, fshandle_t *fh)
{
	long byte_size;
	long bytes_read;
	size_t nmemb_read;

	if (!fh) {
		errno = EBADF;
		return 0;
	}
	if (!ptr) {
		errno = EFAULT;
		return 0;
	}
	if (!size || !nmemb) {	/* no error, just zero bytes wanted */
		errno = 0;
		return 0;
	}

	byte_size = nmemb * size;
	if (byte_size > fh->length - fh->pos)	/* just read to end */
		byte_size = fh->length - fh->pos;
	bytes_read = fread(ptr, 1, byte_size, fh->file);
	fh->pos += bytes_read;

	/* fread() must return the number of elements read,
	 * not the total number of bytes. */
	nmemb_read = bytes_read / size;
	/* even if the last member is only read partially
	 * it is counted as a whole in the return value. */
	if (bytes_read % size)
		nmemb_read++;

	return nmemb_read;
}

int FS_fseek(fshandle_t *fh, long offset, int whence)
{
/* I don't care about 64 bit off_t or fseeko() here.
 * the quake/hexen2 file system is 32 bits, anyway. */
	int ret;

	if (!fh) {
		errno = EBADF;
		return -1;
	}

	/* the relative file position shouldn't be smaller
	 * than zero or bigger than the filesize. */
	switch (whence)
	{
	case SEEK_SET:
		break;
	case SEEK_CUR:
		offset += fh->pos;
		break;
	case SEEK_END:
		offset = fh->length + offset;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	if (offset < 0) {
		errno = EINVAL;
		return -1;
	}

	if (offset > fh->length)	/* just seek to end */
		offset = fh->length;

	ret = fseek(fh->file, fh->start + offset, SEEK_SET);
	if (ret < 0)
		return ret;

	fh->pos = offset;
	return 0;
}

int FS_fclose(fshandle_t *fh)
{
	if (!fh) {
		errno = EBADF;
		return -1;
	}
	return fclose(fh->file);
}

long FS_ftell(fshandle_t *fh)
{
	if (!fh) {
		errno = EBADF;
		return -1;
	}
	return fh->pos;
}

void FS_rewind(fshandle_t *fh)
{
	if (!fh) return;
	clearerr(fh->file);
	fseek(fh->file, fh->start, SEEK_SET);
	fh->pos = 0;
}

int FS_feof(fshandle_t *fh)
{
	if (!fh) {
		errno = EBADF;
		return -1;
	}
	if (fh->pos >= fh->length)
		return -1;
	return 0;
}

int FS_ferror(fshandle_t *fh)
{
	if (!fh) {
		errno = EBADF;
		return -1;
	}
	return ferror(fh->file);
}

int FS_fgetc(fshandle_t *fh)
{
	if (!fh) {
		errno = EBADF;
		return EOF;
	}
	if (fh->pos >= fh->length)
		return EOF;
	fh->pos += 1;
	return fgetc(fh->file);
}

char *FS_fgets(char *s, int size, fshandle_t *fh)
{
	char *ret;

	if (FS_feof(fh))
		return NULL;

	if (size > (fh->length - fh->pos) + 1)
		size = (fh->length - fh->pos) + 1;

	ret = fgets(s, size, fh->file);
	fh->pos = ftell(fh->file) - fh->start;

	return ret;
}

long FS_filelength (fshandle_t *fh)
{
	if (!fh) {
		errno = EBADF;
		return -1;
	}
	return fh->length;
}

