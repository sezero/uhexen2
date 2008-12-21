/*
	quakefs.c
	Hexen II filesystem

	$Id: quakefs.c,v 1.49 2008-12-21 18:10:02 sezero Exp $
*/

#include "quakedef.h"
#include "pakfile.h"
#ifdef PLATFORM_WINDOWS
#include <io.h>
#endif
#ifdef PLATFORM_DOS
#include <unistd.h>
#endif
#ifdef PLATFORM_UNIX
#include <unistd.h>
#include <errno.h>
#endif

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} pakfiles_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	pakfiles_t	*files;
} pack_t;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	struct	pack_s	*pack;		// only one of filename / pack will be used
	struct	searchpath_s *next;
} searchpath_t;

static searchpath_t	*fs_searchpaths;
static searchpath_t	*fs_base_searchpaths;	// without gamedirs

const char	*fs_basedir;
char	fs_gamedir[MAX_OSPATH];
char	fs_gamedir_nopath[MAX_QPATH];
char	fs_userdir[MAX_OSPATH];

unsigned int	gameflags;

cvar_t	oem = {"oem", "0", CVAR_ROM};
cvar_t	registered = {"registered", "0", CVAR_ROM};

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
#define	MAX_PAKDATA	(sizeof(pakdata) / sizeof(pakdata[0]))

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
			//	The old (28.8.1997, v0.42? 1.07?) demo is not supported:
			//	pak0.pak::progs.dat : 19267 crc, progheader crc : 14046.
/*
 * FIXME: add the pak0 and pak2 data for
 * the oem (Matrox m3D bundle) original
 * version here...
 */
};

// this graphic needs to be in the pak file to use registered features
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


//============================================================================

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

The "cache directory" is only used during development to save network bandwidth
especially over ISDN / T1 lines.  If there is a cache directory specified, when
a file is found by the normal search path, it will be mirrored into the cache
directory, then opened there.
*/


/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
static pack_t *FS_LoadPackFile (const char *packfile, int paknum, qboolean base_fs)
{
	dpackheader_t	header;
	int			i, numpackfiles;
	pakfiles_t		*newfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned short		crc;

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
	fread (&info, 1, header.dirlen, packhandle);

// crc the directory
	CRC_Init (&crc);
	for (i = 0; i < header.dirlen; i++)
		CRC_ProcessByte (&crc, ((byte *)info)[i]);

// check for modifications
	if (base_fs && paknum < (int)MAX_PAKDATA)
	{
		if (strcmp(fs_gamedir_nopath, pakdata[paknum].dirname) != 0)
		{
			// raven didnt ship like that
			gameflags |= GAME_MODIFIED;
		}
		else if (numpackfiles != pakdata[paknum].numfiles)
		{
			if (paknum == 0)
			{
				// demo ??
				if (crc == demo_pakdata[0].crc &&
				    numpackfiles == demo_pakdata[0].numfiles)
				{
					gameflags |= GAME_DEMO;
				}
				// oem ??
				else if (crc == oem0_pakdata[0].crc &&
					 numpackfiles == oem0_pakdata[0].numfiles)
				{
					gameflags |= GAME_OEM0;
				}
				// old version of demo ??
				else if (crc == old_pakdata[2].crc &&
					 numpackfiles == old_pakdata[2].numfiles)
				{
					gameflags |= GAME_OLD_DEMO;
				}
				// old, un-patched cdrom version ??
				else if (crc == old_pakdata[0].crc &&
					 numpackfiles == old_pakdata[0].numfiles)
				{
					gameflags |= GAME_OLD_CDROM0;
				}
				/*
				 * FIXME: add old oem version pak file
				 * checks here...
				 */
				else
				{	// not original
					gameflags |= GAME_MODIFIED;
				}
			}
			else if (paknum == 1)
			{
				// old, un-patched cdrom version ??
				if (crc == old_pakdata[1].crc &&
				    numpackfiles == old_pakdata[1].numfiles)
				{
					gameflags |= GAME_OLD_CDROM1;
				}
				else
				{	// not original
					gameflags |= GAME_MODIFIED;
				}
			}
			else
			{
			// not original
				gameflags |= GAME_MODIFIED;
			}
		}
		else if (crc != pakdata[paknum].crc)
		{
		// not original
			gameflags |= GAME_MODIFIED;
		}
		else
		{
			switch (paknum)
			{
			case 0:	// pak0 of full version 1.11
				gameflags |= GAME_REGISTERED0;
				break;
			case 1:	// pak1 of full version 1.11
				gameflags |= GAME_REGISTERED1;
				break;
			case 2:	// bundle version
				gameflags |= GAME_OEM2;
				break;
			case 3:	// mission pack
				gameflags |= GAME_PORTALS;
				break;
			case 4:	// hexenworld
				gameflags |= GAME_HEXENWORLD;
				break;
			default:// we shouldn't reach here
				break;
			}
		}
		// both crc and numfiles are good, we are still original
	}
	else
	{
		gameflags |= GAME_MODIFIED;
	}

// parse the directory
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

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
This is a callback for FS_Init() ONLY. The dir argument must
contain a path information, at least a partial one.
================
*/
static void FS_AddGameDirectory (const char *dir, qboolean base_fs)
{
	int				i;
	searchpath_t		*search;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];
	char			*p;
	qboolean		been_here = false;

	qerr_strlcpy(__thisfunc__, __LINE__, fs_gamedir, dir, sizeof(fs_gamedir));
	p = strrchr (fs_gamedir, '/');
	qerr_strlcpy(__thisfunc__, __LINE__, fs_gamedir_nopath, ++p, sizeof(fs_gamedir_nopath));

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
#if DO_USERDIRS
add_pakfile:
#endif
	for (i = 0; i < 10; i++)
	{
		if (been_here)
		{
			qerr_snprintf(__thisfunc__, __LINE__, pakfile, sizeof(pakfile), "%s/pak%i.pak", fs_userdir, i);
		}
		else
		{
			qerr_snprintf(__thisfunc__, __LINE__, pakfile, sizeof(pakfile), "%s/pak%i.pak", dir, i);
		}
		pak = FS_LoadPackFile (pakfile, i, base_fs);
		if (!pak)
			continue;
		search = (searchpath_t *) Hunk_AllocName (sizeof(searchpath_t), "searchpath");
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}

// add the directory to the search path
// O.S: this needs to be done ~after~ adding the pakfiles in
// this dir, so that the dir itself will be placed above the
// pakfiles in the search order which, in turn, will allow
// override files:
// this way, data1/default.cfg will be opened instead of
// data1/pak0.pak:/default.cfg
	search = (searchpath_t *) Hunk_AllocName (sizeof(searchpath_t), "searchpath");
	if (been_here)
	{
		qerr_strlcpy(__thisfunc__, __LINE__, search->filename, fs_userdir, MAX_OSPATH);
	}
	else
	{
		qerr_strlcpy(__thisfunc__, __LINE__, search->filename, dir, MAX_OSPATH);
	}
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	if (been_here)
		return;
	been_here = true;

// add user's directory to the search path
// add any pak files in the user's directory
#if DO_USERDIRS
	if (strcmp(fs_gamedir, fs_userdir))
		goto add_pakfile;
#endif
}

/*
================
FS_Gamedir

Sets the gamedir and path to a different directory.

Hexen2 uses this for setting the gamedir upon seeing
a -game commandline argument. In addition to this,
hexenworld uses this procedure to set the gamedir on
both server and client sides during game execution:
Client calls this upon every map change from within
CL_ParseServerData() and the Server calls this upon
a gamedir command from within SV_Gamedir_f().
================
*/
void FS_Gamedir (const char *dir)
{
	searchpath_t	*search, *next;
	int				i;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];
	qboolean		been_here = false;

	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":") )
	{
		Sys_Printf ("Gamedir should be a single directory name, not a path\n");
		return;
	}

	if (!q_strcasecmp(fs_gamedir_nopath, dir))
		return;		// still the same
	qerr_strlcpy(__thisfunc__, __LINE__, fs_gamedir_nopath, dir, sizeof(fs_gamedir_nopath));

	// FIXME: Should I check for directory's existence ??

//
// free up any current game dir info: our top searchpath dir will be hw
// and any gamedirs set before by this very procedure will be removed.
// since hexen2 doesn't use this during game execution there will be no
// changes for it: it has portals or data1 at the top.
//
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

//
// flush all data, so it will be forced to reload
//
#if !defined(SERVERONLY)
	Cache_Flush ();
#endif	/* SERVERONLY */

// check for reserved gamedirs
	if (!q_strcasecmp(dir, "hw"))
	{
#if !defined(H2W)
	// hw is reserved for hexenworld only. hexen2 shouldn't use it
		Sys_Printf ("WARNING: Gamedir not set to hw :\n"
			    "It is reserved for HexenWorld.\n");
		return;
#else
	// that we reached here means the hw server decided to abandon
	// whatever the previous mod it was running and went back to
	// pure hw. weird.. do as he wishes anyway and adjust our variables.
		qerr_snprintf(__thisfunc__, __LINE__, fs_gamedir, sizeof(fs_gamedir), "%s/hw", fs_basedir);
#    if DO_USERDIRS
		qerr_snprintf(__thisfunc__, __LINE__, fs_userdir, sizeof(fs_userdir), "%s/hw", host_parms->userdir);
#    else
		qerr_strlcpy (__thisfunc__, __LINE__, fs_userdir, fs_gamedir, sizeof(fs_userdir));
#    endif
#    if defined(SERVERONLY)
	// change the *gamedir serverinfo properly
		Info_SetValueForStarKey (svs.info, "*gamedir", "hw", MAX_SERVERINFO_STRING);
#    endif
		return;
#endif	/* H2W */
	}
	else if (!q_strcasecmp(dir, "portals"))
	{
	// no hw server is supposed to set gamedir to portals
	// and hw must be above portals in hierarchy. this is
	// actually a hypothetical case.
	// as for hexen2, it cannot reach here.
		return;
	}
	else if (!q_strcasecmp(dir, "data1"))
	{
	// another hypothetical case: no hw mod is supposed to
	// do this and hw must stay above data1 in hierarchy.
	// as for hexen2, it can only reach here by a silly
	// command line argument like -game data1, ignore it.
		return;
	}
	else
	{
	// a new gamedir: let's set it here.
		qerr_snprintf(__thisfunc__, __LINE__, fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir, dir);
	}

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
#if DO_USERDIRS
add_pakfiles:
#endif
	for (i = 0; i < 10; i++)
	{
		if (been_here)
		{
			qerr_snprintf(__thisfunc__, __LINE__, pakfile, sizeof(pakfile), "%s/pak%i.pak", fs_userdir, i);
		}
		else
		{
			qerr_snprintf(__thisfunc__, __LINE__, pakfile, sizeof(pakfile), "%s/pak%i.pak", fs_gamedir, i);
		}
		pak = FS_LoadPackFile (pakfile, i, false);
		if (!pak)
			continue;
		search = (searchpath_t *) Z_Malloc (sizeof(searchpath_t), Z_MAINZONE);
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}

// add the directory to the search path
// O.S: this needs to be done ~after~ adding the pakfiles in
// this dir, so that the dir itself will be placed above the
// pakfiles in the search order
	search = (searchpath_t *) Z_Malloc (sizeof(searchpath_t), Z_MAINZONE);
	if (been_here)
	{
		qerr_strlcpy(__thisfunc__, __LINE__, search->filename, fs_userdir, MAX_OSPATH);
	}
	else
	{
		qerr_strlcpy(__thisfunc__, __LINE__, search->filename, fs_gamedir, MAX_OSPATH);
	}
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	if (been_here)
		return;
	been_here = true;

#if defined(H2W) && defined(SERVERONLY)
// change the *gamedir serverinfo properly
	Info_SetValueForStarKey (svs.info, "*gamedir", dir, MAX_SERVERINFO_STRING);
#endif

// add user's directory to the search path
#if DO_USERDIRS
	qerr_snprintf(__thisfunc__, __LINE__, fs_userdir, sizeof(fs_userdir), "%s/%s", host_parms->userdir, dir);
	Sys_mkdir (fs_userdir, true);
// add any pak files in the user's directory
	if (strcmp(fs_gamedir, fs_userdir))
		goto add_pakfiles;
#else
	qerr_strlcpy (__thisfunc__, __LINE__, fs_userdir, fs_gamedir, sizeof(fs_userdir));
#endif
}


//============================================================================

/*
==============================================================================

FILE I/O within QFS

==============================================================================
*/

size_t		fs_filesize;	// size of the last file opened through QFS
char		*fs_filepath;	// path of the last file opened through QFS
						// NULL for files in a pak.
int		file_from_pak;	// ZOID: global indicating that file came from a pak


/*
================
FS_filelength
================
*/
static size_t FS_filelength (FILE *f)
{
	long		pos, end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return (size_t)end;
}

/*
===========
FS_CopyFile

Copies the FROMPATH file as TOPATH file, creating any dirs needed.
Used for saving the game. Returns 0 on success, non-zero on error.
===========
*/
int FS_CopyFile (const char *frompath, const char *topath)
{
	char		*tmp;
	int		err;

	if (!frompath || !topath)
	{
		Con_Printf ("%s: null input\n", __thisfunc__);
		return 1;
	}
	// create directories up to the dest file
	tmp = (char *) Z_Malloc (strlen(topath) + 1, Z_MAINZONE);
	strcpy (tmp, topath);
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
int FS_CopyFromFile (FILE *fromfile, const char *topath, size_t size)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*out;
//	off_t		remaining, count;
	size_t		remaining, count;
	char		*tmp;
	int		err;

	if (!fromfile || !topath)
	{
		Con_Printf ("%s: null input\n", __thisfunc__);
		return 1;
	}

	// create directories up to the dest file
	tmp = (char *) Z_Malloc (strlen(topath) + 1, Z_MAINZONE);
	strcpy (tmp, topath);
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

	memset (buf, 0, sizeof(buf));
	remaining = size;
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);

		fread (buf, 1, count, fromfile);
		err = ferror (fromfile);
		if (err)
			break;

		fwrite (buf, 1, count, out);
		err = ferror (out);
		if (err)
			break;

		remaining -= count;
	}

	fclose (out);

	return err;
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

	if (q_snprintf(name, sizeof(name), "%s/%s", fs_userdir, filename) >= (int)sizeof(name))
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
seperator. Only used for CopyFile and download. Returns 0 on
success, non-zero on error.
============
*/
int FS_CreatePath (char *path)
{
	char	*ofs;
	int		error_state = 0;
	size_t		offset;

	if (!path || !path[0])
	{
		Con_Printf ("%s: no path!\n", __thisfunc__);
		return 1;
	}

	if (strstr(path, ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return 1;
	}

	ofs = host_parms->userdir;
	if (strstr(path, ofs) != path)
	{
error_out:
		Sys_Error ("Attempted to create a directory out of user's path");
		return 1;
	}

	offset = strlen(ofs);
	ofs = path + offset;
	// check for the path separator after the userdir.
	if (!*ofs)
	{
		Con_Printf ("%s: bad path\n", __thisfunc__);
		return 1;
	}
	if (*ofs != '/')
		goto error_out;
	ofs++;

	for ( ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			error_state = Sys_mkdir (path, false);
			*ofs = '/';
			if (error_state)
				break;
		}
	}

	return error_state;
}


/*
===========
FS_OpenFile

Finds the file in the search path, returns fs_filesize.
===========
*/
size_t FS_OpenFile (const char *filename, FILE **file, qboolean override_pack)
{
	searchpath_t	*search;
	char		netpath[MAX_OSPATH];
	pack_t		*pak;
	int			i;

	file_from_pak = 0;

//
// search through the path, one element at a time
//
	for (search = fs_searchpaths ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i = 0; i < pak->numfiles; i++)
			{
				if (!strcmp (pak->files[i].name, filename))
				{	// found it!
					// open a new file on the pakfile
					*file = fopen (pak->filename, "rb");
					if (!*file)
						Sys_Error ("Couldn't reopen %s", pak->filename);
					fseek (*file, pak->files[i].filepos, SEEK_SET);
					fs_filesize = (size_t) pak->files[i].filelen;
					file_from_pak = 1;
					fs_filepath = NULL;
					return fs_filesize;
				}
			}
		}
		else
		{
	// check a file in the directory tree
#if !defined(H2W)
			if (!(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)) && !override_pack)
			{	// if not a registered version, don't ever go beyond base
				if ( strchr (filename, '/') || strchr (filename,'\\'))
					continue;
			}
#endif	/* ! H2W */

			q_snprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
			if (access(netpath, R_OK) == -1)
				continue;

			*file = fopen (netpath, "rb");
			if (!*file)
				Sys_Error ("Couldn't reopen %s", netpath);
			fs_filepath = search->filename;
			fs_filesize = FS_filelength (*file);
			return fs_filesize;
		}
	}

	Sys_DPrintf ("%s: can't find %s\n", __thisfunc__, filename);

	*file = NULL;
	fs_filepath = NULL;
	fs_filesize = (size_t)-1;
	return fs_filesize;
}

/*
============
FS_FileInGamedir

Reports the existance of a file with read permissions in
fs_gamedir or fs_userdir. -1 is returned on failure,
ie. the return value of the access() function Files in
pakfiles are NOT meant for this procedure!
============
*/
int FS_FileInGamedir (const char *fname)
{
	int	ret;

	ret = access (va("%s/%s", fs_userdir, fname), R_OK);
	if (ret == -1)
		ret = access (va("%s/%s", fs_gamedir, fname), R_OK);

	return ret;
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
#define	LOADFILE_BUF		5
#define	LOADFILE_MALLOC		6

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

static byte *FS_LoadFile (const char *path, int usehunk)
{
	FILE	*h;
	byte	*buf;
	char	base[32];
	size_t		len;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = fs_filesize = FS_OpenFile (path, &h, false);
	if (!h)
		return NULL;

// extract the filename base name for hunk tag
	COM_FileBase (path, base, sizeof(base));

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
	case LOADFILE_BUF:
		if (len < loadsize && loadbuf != NULL)
			buf = loadbuf;
		else
			buf = (byte *) Hunk_AllocName(len + 1, path);
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

byte *FS_LoadHunkFile (const char *path)
{
	return FS_LoadFile (path, LOADFILE_HUNK);
}

byte *FS_LoadZoneFile (const char *path, int zone_id)
{
	zone_num = zone_id;
	return FS_LoadFile (path, LOADFILE_ZONE);
}

byte *FS_LoadTempFile (const char *path)
{
	return FS_LoadFile (path, LOADFILE_TEMPHUNK);
}

#if !defined(SERVERONLY)
void FS_LoadCacheFile (const char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	FS_LoadFile (path, LOADFILE_CACHE);
}
#endif	/* SERVERONLY */

// uses temp hunk if larger than bufsize
byte *FS_LoadStackFile (const char *path, void *buffer, size_t bufsize)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = FS_LoadFile (path, LOADFILE_STACK);

	return buf;
}

// loads into a previously allocated buffer. if space is insufficient
// or the buffer is NULL, loads onto the hunk.  bufsize is the actual
// size (without the +1).
byte *FS_LoadBufFile (const char *path, void *buffer, size_t *bufsize)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = (*bufsize) + 1;
	buf = FS_LoadFile (path, LOADFILE_BUF);
	*bufsize = (buf == NULL) ? 0 : fs_filesize;
	if (loadbuf && buf && buf != loadbuf)
		Sys_Printf("%s: insufficient buffer for %s not used.\n", __thisfunc__, path);

	return buf;
}

// returns malloc'd memory
byte *FS_LoadMallocFile (const char *path)
{
	return FS_LoadFile (path, LOADFILE_MALLOC);
}


//============================================================================

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
static int		map_count = 0;
static char		*maplist[MAX_NUMMAPS];

static int processMapname (const char *mapname, const char *partial, size_t len_partial, qboolean from_pak)
{
	size_t			len;
	int			j;
	char	cur_name[MAX_QPATH];

	if (map_count >= MAX_NUMMAPS)
	{
		Con_Printf ("WARNING: reached maximum number of maps to list\n");
		return -1;
	}

	if ( len_partial )
	{
		if ( q_strncasecmp(partial, mapname, len_partial) )
			return 0;	// doesn't match the prefix. skip.
	}

	q_strlcpy (cur_name, mapname, sizeof(cur_name));
	len = strlen(cur_name) - 4;	// ".bsp" : 4
	if ( from_pak )
	{
		if ( strcmp(cur_name + len, ".bsp") )
			return 0;
	}

	cur_name[len] = 0;
	if ( !cur_name[0] )
		return 0;

	for (j = 0; j < map_count; j++)
	{
		if ( !q_strcasecmp(maplist[j], mapname) )
			return 0;	// duplicated name. skip.
	}

	// add to the maplist
	maplist[map_count] = (char *) Z_Malloc (len+1, Z_MAINZONE);
	if (maplist[map_count] == NULL)
	{
		Con_Printf ("WARNING: Failed allocating memory for maplist\n");
		return -1;
	}

	q_strlcpy (maplist[map_count], mapname, len+1);
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

	// search through the path, one element at a time
	// either "search->filename" or "search->pak" is defined
	for (search = fs_searchpaths; search; search = search->next)
	{
		if (search->pack)
		{
			int			i;

			for (i = 0; i < search->pack->numfiles; i++)
			{
				if ( strncmp("maps/", search->pack->files[i].name, 5) )
					continue;
				if ( processMapname(search->pack->files[i].name + 5, prefix, preLen, true) < 0 )
					goto done;
			}
		}
		else
		{	// element is a filename
			char		*findname;

			findname = Sys_FindFirstFile (va("%s/maps",search->filename), "*.bsp");
			while (findname)
			{
				if ( processMapname(findname, prefix, preLen, false) < 0 )
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
	else
	{
		Con_Printf ("Found %d maps:\n\n", map_count);
	}

	// sort the list
	qsort (maplist, map_count, sizeof(char *), COM_StrCompare);
	Con_ShowList (map_count, (const char**)maplist);
	Con_Printf ("\n");

	// free the memory and zero map_count
	while (map_count)
	{
		Z_Free (maplist[--map_count]);
	}
}
#endif	/* SERVERONLY */


//============================================================================

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
	FILE		*h;
	unsigned short	check[128];
	int			i;

	FS_OpenFile("gfx/pop.lmp", &h, false);

	if (!h)
		return -1;

	fread (check, 1, sizeof(check), h);
	fclose (h);

	for (i = 0; i < 128; i++)
	{
		if ( pop[i] != (unsigned short)BigShort(check[i]) )
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
	int		i;
	char		temp[32];
	qboolean	check_portals = false;
	searchpath_t	*search_tmp, *next_tmp;

//
// Register our cvars
//
	Cvar_RegisterVariable (&oem);
	Cvar_RegisterVariable (&registered);

//
// Register our commands
//
	Cmd_AddCommand ("path", FS_Path_f);
#if !defined(SERVERONLY)
	Cmd_AddCommand ("maplist", FS_Maplist_f);
#endif	/* SERVERONLY */

//
// -basedir <path>
// Overrides the system supplied base directory (under data1)
//
	i = COM_CheckParm ("-basedir");
	if (i && i < com_argc-1)
	{
		fs_basedir = com_argv[i+1];
		Sys_Printf ("%s: basedir changed to: %s\n", __thisfunc__, fs_basedir);
	}
	else
	{
		fs_basedir = host_parms->basedir;
	}

	qerr_strlcpy(__thisfunc__, __LINE__, fs_userdir, host_parms->userdir, sizeof(fs_userdir));

//
// step 1: start up with data1 by default
//
	qerr_snprintf(__thisfunc__, __LINE__, fs_userdir, sizeof(fs_userdir), "%s/data1", host_parms->userdir);
	Sys_mkdir (fs_userdir, true);
	FS_AddGameDirectory (va("%s/data1", fs_basedir), true);

	if (gameflags & GAME_REGISTERED0 && gameflags & GAME_REGISTERED1)
		gameflags |= GAME_REGISTERED;
	if (gameflags & GAME_OEM0 && gameflags & GAME_OEM2)
		gameflags |= GAME_OEM;
	if (gameflags & GAME_OLD_CDROM0 && gameflags & GAME_OLD_CDROM1)
		gameflags |= GAME_REGISTERED_OLD;
	if (gameflags & GAME_OLD_OEM0 && gameflags & GAME_OLD_OEM2)
		gameflags |= GAME_OLD_OEM;
	// check for bad installations (mix'n'match data):
	if ((gameflags & GAME_REGISTERED0 && gameflags & GAME_OLD_CDROM1) ||
	    (gameflags & GAME_REGISTERED1 && gameflags & GAME_OLD_CDROM0) ||
	    (gameflags & GAME_OEM2 && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD|GAME_DEMO|GAME_OLD_DEMO)) ||
	    (gameflags & (GAME_REGISTERED1|GAME_OLD_CDROM1) && gameflags & (GAME_DEMO|GAME_OLD_DEMO|GAME_OEM0|GAME_OEM2)))
		Sys_Error ("Bad Hexen II installation: mixed data from incompatible versions");
#if !ENABLE_OLD_DEMO
	if (gameflags & GAME_OLD_DEMO)
		Sys_Error ("Old version of Hexen II demo isn't supported");
#endif	/* OLD_DEMO */
#if !ENABLE_OLD_RETAIL
	// check if we have 1.11 versions of pak0.pak and pak1.pak
	if (gameflags & (GAME_OLD_CDROM0|GAME_OLD_CDROM1|GAME_OLD_OEM0|GAME_OLD_OEM2))
		Sys_Error ("You must patch your installation with Raven's 1.11 update");
#endif	/* OLD_RETAIL */

	// finish the base filesystem setup
	oem.flags &= ~CVAR_ROM;
	registered.flags &= ~CVAR_ROM;
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
	{
		q_snprintf (temp, sizeof(temp), "registered");
		Cvar_SetValue ("registered", 1);
	}
	else if (gameflags & GAME_OEM)
	{
		q_snprintf (temp, sizeof(temp), "oem");
		Cvar_SetValue ("oem", 1);
	}
	else if (gameflags & (GAME_DEMO|GAME_OLD_DEMO))
	{
		q_snprintf (temp, sizeof(temp), "demo");
	}
	else
	{
	// no proper Raven data: it's best to error out here
		Sys_Error ("Unable to find a proper Hexen II installation");
	}
	oem.flags |= CVAR_ROM;
	registered.flags |= CVAR_ROM;
	Sys_Printf ("Playing %s version.\n", temp);
	if (gameflags & (GAME_OLD_DEMO|GAME_REGISTERED_OLD|GAME_OLD_OEM))
		Sys_Printf ("Using old/unsupported, pre-1.11 version pak files.\n");
	if (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
	{
		if (CheckRegistered() != 0)
			Sys_Error ("Unable to verify retail version data.");
	}
#if !( defined(H2W) && defined(SERVERONLY) )
	if (gameflags & GAME_MODIFIED && !(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
		Sys_Error ("You must have the full version of Hexen II to play modified games");
#endif

//
// step 2: portals directory (mission pack)
//
#if defined(H2MP) || defined(H2W)
	if (! COM_CheckParm ("-noportals"))
		check_portals = true;
#else
	// see if the user wants mission pack support
	check_portals = (COM_CheckParm ("-portals")) || (COM_CheckParm ("-missionpack")) || (COM_CheckParm ("-h2mp"));
	i = COM_CheckParm ("-game");
	if (i && i < com_argc-1)
	{
		if (!q_strcasecmp(com_argv[i+1], "portals"))
			check_portals = true;
	}
#endif
#if !defined(H2W)
	if (sv_protocol == PROTOCOL_RAVEN_111)
	{
		if (check_portals)
			Sys_Printf ("Old protocol requested: disabling mission pack support request.\n");
		check_portals = false;
	}
#endif

//	if (check_portals && !(gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
//		Sys_Error ("Portal of Praevus requires registered version of Hexen II");
	if (check_portals && gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD))
	{
		i = Hunk_LowMark ();
		search_tmp = fs_searchpaths;

		qerr_snprintf(__thisfunc__, __LINE__, fs_userdir, sizeof(fs_userdir), "%s/portals", host_parms->userdir);
		Sys_mkdir (fs_userdir, true);
		FS_AddGameDirectory (va("%s/portals", fs_basedir), true);

		// back out searchpaths from invalid mission pack installations
		if ( !(gameflags & GAME_PORTALS))
		{
			Sys_Printf ("Missing or invalid mission pack installation\n");
			while (fs_searchpaths != search_tmp)
			{
				if (fs_searchpaths->pack)
				{
					fclose (fs_searchpaths->pack->handle);
					Sys_Printf ("Removed packfile %s\n", fs_searchpaths->pack->filename);
				}
				else
				{
					Sys_Printf ("Removed path %s\n", fs_searchpaths->filename);
				}
				next_tmp = fs_searchpaths->next;
				fs_searchpaths = next_tmp;
			}
			fs_searchpaths = search_tmp;
			Hunk_FreeToLowMark (i);
			// back to data1
			q_snprintf (fs_gamedir, sizeof(fs_gamedir), "%s/data1", fs_basedir);
			q_snprintf (fs_userdir, sizeof(fs_userdir), "%s/data1", host_parms->userdir);
		}
	}

//
// step 3: hw directory (hexenworld)
//
#if defined(H2W)
	qerr_snprintf(__thisfunc__, __LINE__, fs_userdir, sizeof(fs_userdir), "%s/hw", host_parms->userdir);
	Sys_mkdir (fs_userdir, true);
	FS_AddGameDirectory (va("%s/hw", fs_basedir), true);
	// error out for H2W builds if GAME_HEXENWORLD isn't set
	if (!(gameflags & GAME_HEXENWORLD))
		Sys_Error ("You must have the HexenWorld data installed");
#endif	/* H2W */

// this is the end of our base searchpath:
// any set gamedirs, such as those from -game commandline
// arguments, from exec'ed configs or the ones dictated by
// the server, will be freed up to here upon a new gamedir
// command
	fs_base_searchpaths = fs_searchpaths;

	i = COM_CheckParm ("-game");
	if (i != 0)
	{
		// only registered versions can do -game
		if (! (gameflags & (GAME_REGISTERED|GAME_REGISTERED_OLD)))
			Sys_Error ("You must have the full version of Hexen II to play modified games");
		// add basedir/gamedir as an override game
		if (i < com_argc - 1)
			FS_Gamedir (com_argv[i+1]);
	}
}

