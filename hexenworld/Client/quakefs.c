/*
	quakefs.c
	Hexen II filesystem

	$Id: quakefs.c,v 1.1 2007-02-12 16:53:12 sezero Exp $
*/

#include "quakedef.h"
#include "pakfile.h"
#ifdef _WIN32
#include <io.h>
#endif
#ifdef PLATFORM_UNIX
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#endif

searchpath_t		*fs_searchpaths;
static searchpath_t	*fs_base_searchpaths;	// without gamedirs

char	fs_gamedir[MAX_OSPATH];
char	fs_basedir[MAX_OSPATH];
char	fs_userdir[MAX_OSPATH];
char	fs_savedir[MAX_OSPATH];	// temporary path for saving gip files

char	gamedirfile[MAX_OSPATH];

unsigned int	gameflags;

cvar_t	oem = {"oem", "0", CVAR_ROM};
cvar_t	registered = {"registered", "0", CVAR_ROM};

// look-up table of pak filenames: { numfiles, crc }
// if a packfile directory differs from this, it is assumed to be hacked
#define MAX_PAKDATA	6
static const int pakdata[MAX_PAKDATA][2] = {
	{ 696,	34289 },	/* pak0.pak, registered	*/
	{ 523,	2995  },	/* pak1.pak, registered	*/
	{ 183,	4807  },	/* pak2.pak, oem, data needs verification */
	{ 245,	1478  },	/* pak3.pak, portals	*/
	{ 102,	41062 },	/* pak4.pak, hexenworld	*/
	{ 797,	22780 }		/* pak0.pak, demo v1.11	*/
//	{ 701,	20870 }		/* pak0.pak, old 1.07 version of the demo */
//	The old v1.07 demo on the ID Software ftp isn't supported
//	(pak0.pak::progs.dat : 19267 crc, progheader crc : 14046)
};

// loacations of pak filenames as shipped by raven
static const char *dirdata[MAX_PAKDATA] = {
	"data1",	/* pak0.pak, registered	*/
	"data1",	/* pak1.pak, registered	*/
	"data1",	/* pak2.pak, oem	*/
	"portals",	/* pak3.pak, portals	*/
	"hw",		/* pak4.pak, hexenworld	*/
	"data1"		/* pak0.pak, demo	*/
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
================
CheckRegistered

Looks for the pop.txt file and verifies it.
Sets the registered flag.
================
*/
static void CheckRegistered (void)
{
	FILE		*h;
	unsigned short	check[128];
	int			i;

	QIO_FOpenFile("gfx/pop.lmp", &h, false);

	if (!h)
		return;

	fread (check, 1, sizeof(check), h);
	fclose (h);

	for (i = 0; i < 128; i++)
	{
		if ( pop[i] != (unsigned short)BigShort(check[i]) )
			Sys_Error ("Corrupted data file.");
	}

	// check if we have 1.11 versions of pak0.pak and pak1.pak
	if (!(gameflags & GAME_REGISTERED0) || !(gameflags & GAME_REGISTERED1))
		Sys_Error ("You must patch your installation with Raven's 1.11 update");

	gameflags |= GAME_REGISTERED;
}


/*
============
FS_Path_f

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
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
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
		Sys_Error ("%s is not a packfile", packfile);

	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error ("%s has %i files", packfile, numpackfiles);

	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (&info, 1, header.dirlen, packhandle);

// crc the directory
	CRC_Init (&crc);
	for (i = 0; i < header.dirlen; i++)
		CRC_ProcessByte (&crc, ((byte *)info)[i]);

// check for modifications
	if (base_fs && paknum <= MAX_PAKDATA-2)
	{
		if (strcmp(gamedirfile, dirdata[paknum]) != 0)
		{
			// raven didnt ship like that
			gameflags |= GAME_MODIFIED;
		}
		else if (numpackfiles != pakdata[paknum][0])
		{
			if (paknum == 0)
			{
				// demo ??
				if (numpackfiles != pakdata[MAX_PAKDATA-1][0])
				{
				// not original
					gameflags |= GAME_MODIFIED;
				}
				else if (crc != pakdata[MAX_PAKDATA-1][1])
				{
				// not original
					gameflags |= GAME_MODIFIED;
				}
				else
				{
				// both crc and numfiles matched the demo
					gameflags |= GAME_DEMO;
				}
			}
			else
			{
			// not original
				gameflags |= GAME_MODIFIED;
			}
		}
		else if (crc != pakdata[paknum][1])
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
				gameflags |= GAME_OEM;
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
		Q_strlcpy_err(newfiles[i].name, info[i].name, MAX_QPATH);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Z_Malloc (sizeof (pack_t));
	Q_strlcpy_err(pack->filename, packfile, MAX_OSPATH);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	Sys_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}


/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
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

	if ((p = strrchr(dir, '/')) != NULL)
	{
		Q_strlcpy_err(gamedirfile, ++p, sizeof(gamedirfile));
	}
	else
	{
		Q_strlcpy_err(gamedirfile, p, sizeof(gamedirfile));
	}
	Q_strlcpy_err(fs_gamedir, dir, sizeof(fs_gamedir));

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
#ifdef PLATFORM_UNIX
add_pakfile:
#endif
	for (i = 0; i < 10; i++)
	{
		if (been_here)
		{
			Q_snprintf_err(pakfile, sizeof(pakfile), "%s/pak%i.pak", fs_userdir, i);
		}
		else
		{
			Q_snprintf_err(pakfile, sizeof(pakfile), "%s/pak%i.pak", dir, i);
		}
		pak = FS_LoadPackFile (pakfile, i, base_fs);
		if (!pak)
			continue;
		search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
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
	search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
	if (been_here)
	{
		Q_strlcpy_err(search->filename, fs_userdir, MAX_OSPATH);
	}
	else
	{
		Q_strlcpy_err(search->filename, dir, MAX_OSPATH);
	}
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	if (been_here)
		return;
	been_here = true;

// add user's directory to the search path
// add any pak files in the user's directory
#ifdef PLATFORM_UNIX
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

	if (!Q_strcasecmp(gamedirfile, dir))
		return;		// still the same
	Q_strlcpy_err(gamedirfile, dir, sizeof(gamedirfile));

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
	Cache_Flush ();

// check for reserved gamedirs
	if (!Q_strcasecmp(dir, "hw"))
	{
#if !defined(H2W)
	// hw is reserved for hexenworld only. hexen2 shouldn't use it
		Sys_Printf ("WARNING: Gamedir not set to hw :\n"
			    "It is reserved for HexenWorld.\n");
#else
	// that we reached here means the hw server decided to abandon
	// whatever the previous mod it was running and went back to
	// pure hw. weird.. do as he wishes anyway and adjust our variables.
		Q_snprintf_err(fs_gamedir, sizeof(fs_gamedir), "%s/hw", fs_basedir);
#    ifdef PLATFORM_UNIX
		Q_snprintf_err(fs_userdir, sizeof(fs_userdir), "%s/hw", host_parms.userdir);
#    else
		Q_strlcpy_err (fs_userdir, fs_gamedir, sizeof(fs_userdir));
#    endif
#    if defined(SERVERONLY)
	// change the *gamedir serverinfo properly
		Info_SetValueForStarKey (svs.info, "*gamedir", "hw", MAX_SERVERINFO_STRING);
#    endif
		Q_strlcpy_err (fs_savedir, fs_userdir, sizeof(fs_savedir));
#endif
		return;
	}
	else if (!Q_strcasecmp(dir, "portals"))
	{
	// no hw server is supposed to set gamedir to portals
	// and hw must be above portals in hierarchy. this is
	// actually a hypothetical case.
	// as for hexen2, it cannot reach here.
		return;
	}
	else if (!Q_strcasecmp(dir, "data1"))
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
		Q_snprintf_err(fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir, dir);
	}

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
#ifdef PLATFORM_UNIX
add_pakfiles:
#endif
	for (i = 0; i < 10; i++)
	{
		if (been_here)
		{
			Q_snprintf_err(pakfile, sizeof(pakfile), "%s/pak%i.pak", fs_userdir, i);
		}
		else
		{
			Q_snprintf_err(pakfile, sizeof(pakfile), "%s/pak%i.pak", fs_gamedir, i);
		}
		pak = FS_LoadPackFile (pakfile, i, false);
		if (!pak)
			continue;
		search = Z_Malloc (sizeof(searchpath_t));
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;
	}

// add the directory to the search path
// O.S: this needs to be done ~after~ adding the pakfiles in
// this dir, so that the dir itself will be placed above the
// pakfiles in the search order
	search = Z_Malloc (sizeof(searchpath_t));
	if (been_here)
	{
		Q_strlcpy_err(search->filename, fs_userdir, MAX_OSPATH);
	}
	else
	{
		Q_strlcpy_err(search->filename, fs_gamedir, MAX_OSPATH);
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
#ifdef PLATFORM_UNIX
	Q_snprintf_err(fs_userdir, sizeof(fs_userdir), "%s/%s", host_parms.userdir, dir);
	Sys_mkdir_err (fs_userdir);
	Q_strlcpy_err (fs_savedir, fs_userdir, sizeof(fs_savedir));
// add any pak files in the user's directory
	if (strcmp(fs_gamedir, fs_userdir))
		goto add_pakfiles;
#else
	Q_strlcpy_err (fs_userdir, fs_gamedir, sizeof(fs_userdir));
	Q_strlcpy_err (fs_savedir, fs_userdir, sizeof(fs_savedir));
#endif
}

/*
============
MoveUserData
moves all <userdir>/userdata to <userdir>/data1/userdata

AoT and earlier versions of HoT didn't create <userdir>/data1
and kept all user the data in <userdir> instead. Starting with
HoT 1.4.1, we are creating and using <userdir>/data1 . This
procedure is intended to update the user direcory accordingly.
Call from FS_Init ~just after~ setting fs_userdir
to host_parms.userdir/data1
============
*/
#ifdef PLATFORM_UNIX
static void do_movedata (const char *path1, const char *path2, FILE *logfile)
{
	Sys_Printf ("%s -> %s : ", path1, path2);
	if (logfile)
		fprintf (logfile, "%s -> %s : ", path1, path2);
	if (rename (path1, path2) == 0)
	{
		Sys_Printf("OK\n");
		if (logfile)
			fprintf(logfile, "OK\n");
	}
	else
	{
		Sys_Printf("Failed (%s)\n", strerror(errno));
		if (logfile)
			fprintf(logfile, "Failed (%s)\n", strerror(errno));
	}
}

static void MoveUserData (void)
{
	int		i;
	FILE		*fh;
	struct stat	test;
	char	*tmp, tmp1[MAX_OSPATH], tmp2[MAX_OSPATH];
	char	*movefiles[] = 
	{
		"*.cfg",	// config files
		"*.rc",		// config files
		"*.dem",	// pre-recorded demos
		"pak?.pak"	// pak files
	};
	char	*movedirs[] = 
	{
		"quick",	// quick saves
		"shots",	// screenshots
		".midi",	// midi cache
		"glhexen",	// model mesh cache
		/* these are highly unlikely, but just in case.. */
		"maps",
		"midi",
		"sound",
		"models",
		"gfx"
	};
#	define NUM_MOVEFILES	(sizeof(movefiles)/sizeof(movefiles[0]))
#	define NUM_MOVEDIRS	(sizeof(movedirs)/sizeof(movedirs[0]))

	Q_snprintf_err(tmp1, sizeof(tmp1), "%s/userdata.moved", fs_userdir);
	if (stat(tmp1, &test) == 0)
	{
		// the data should have already been moved in earlier runs.
		if ((test.st_mode & S_IFREG) == S_IFREG)
			return;
	}
	fh = fopen(tmp1, "wb");

	Sys_Printf ("Moving user data from root of userdir to userdir/data1\n");

	for (i = 0; i < NUM_MOVEFILES; i++)
	{
		tmp = Sys_FindFirstFile (host_parms.userdir, movefiles[i]);
		while (tmp)
		{
			Q_snprintf_err(tmp1, sizeof(tmp1), "%s/%s", host_parms.userdir, tmp);
			Q_snprintf_err(tmp2, sizeof(tmp2), "%s/%s", fs_userdir, tmp);
			do_movedata (tmp1, tmp2, fh);
			tmp = Sys_FindNextFile ();
		}
		Sys_FindClose ();
	}

	// move the savegames
	for (i = 0; i < MAX_SAVEGAMES; i++)
	{
		Q_snprintf_err(tmp1, sizeof(tmp1), "%s/s%d", host_parms.userdir, i);
		if (stat(tmp1, &test) == 0)
		{
			if ((test.st_mode & S_IFDIR) == S_IFDIR)
			{
				Q_snprintf_err(tmp2, sizeof(tmp2), "%s/s%d", fs_userdir, i);
				do_movedata (tmp1, tmp2, fh);
			}
		}
	}

	// move the savegames (multiplayer)
	for (i = 0; i < MAX_SAVEGAMES; i++)
	{
		Q_snprintf_err(tmp1, sizeof(tmp1), "%s/ms%d", host_parms.userdir, i);
		if (stat(tmp1, &test) == 0)
		{
			if ((test.st_mode & S_IFDIR) == S_IFDIR)
			{
				Q_snprintf_err(tmp2, sizeof(tmp2), "%s/ms%d", fs_userdir, i);
				do_movedata (tmp1, tmp2, fh);
			}
		}
	}

	// other dirs
	for (i = 0; i < NUM_MOVEDIRS; i++)
	{
		Q_snprintf_err(tmp1, sizeof(tmp1), "%s/%s", host_parms.userdir, movedirs[i]);
		if (stat(tmp1, &test) == 0)
		{
			if ((test.st_mode & S_IFDIR) == S_IFDIR)
			{
				Q_snprintf_err(tmp2, sizeof(tmp2), "%s/%s", fs_userdir, movedirs[i]);
				do_movedata (tmp1, tmp2, fh);
			}
		}
	}

	if (fh)
		fclose (fh);
}
#endif

/*
================
FS_Init
================
*/
void FS_Init (void)
{
	int		i;
	char		temp[12];
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

//
// -basedir <path>
// Overrides the system supplied base directory (under data1)
//
	i = COM_CheckParm ("-basedir");
	if (i && i < com_argc-1)
	{
		Q_strlcpy_err(fs_basedir, com_argv[i+1], sizeof(fs_basedir));
	}
	else
	{
		Q_strlcpy_err(fs_basedir, host_parms.basedir, sizeof(fs_basedir));
	}

	Q_strlcpy_err(fs_userdir, host_parms.userdir, sizeof(fs_userdir));

//
// start up with data1 by default
//
	Q_snprintf_err(fs_userdir, sizeof(fs_userdir), "%s/data1", host_parms.userdir);
#ifdef PLATFORM_UNIX
// properly move the user data from older versions in the user's directory
	Sys_mkdir_err (fs_userdir);
	MoveUserData ();
#endif
	FS_AddGameDirectory (va("%s/data1", fs_basedir), true);

	// check if we are playing the registered version
	CheckRegistered ();
	// check for mix'n'match screw-ups
	if ((gameflags & GAME_REGISTERED) && ((gameflags & GAME_DEMO) || (gameflags & GAME_OEM)))
		Sys_Error ("Bad Hexen II installation");
#if !( defined(H2W) && defined(SERVERONLY) )
	if ((gameflags & GAME_MODIFIED) && !(gameflags & GAME_REGISTERED))
		Sys_Error ("You must have the full version of Hexen II to play modified games");
#endif

#if defined(H2MP) || defined(H2W)
	if (! COM_CheckParm ("-noportals"))
		check_portals = true;
#else
// see if the user wants mission pack support
	check_portals = (COM_CheckParm ("-portals")) || (COM_CheckParm ("-missionpack")) || (COM_CheckParm ("-h2mp"));
	i = COM_CheckParm ("-game");
	if (i && i < com_argc-1)
	{
		if (!Q_strcasecmp(com_argv[i+1], "portals"))
			check_portals = true;
	}
#endif

//	if (check_portals && !(gameflags & GAME_REGISTERED))
//		Sys_Error ("Portal of Praevus requires registered version of Hexen II");
	if (check_portals && (gameflags & GAME_REGISTERED))
	{
		i = Hunk_LowMark ();
		search_tmp = fs_searchpaths;

		Q_snprintf_err(fs_userdir, sizeof(fs_userdir), "%s/portals", host_parms.userdir);
		Sys_mkdir_err (fs_userdir);
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
			snprintf (fs_gamedir, sizeof(fs_gamedir), "%s/data1", fs_basedir);
			snprintf (fs_userdir, sizeof(fs_userdir), "%s/data1", host_parms.userdir);
		}
	}

#if defined(H2W)
	Q_snprintf_err(fs_userdir, sizeof(fs_userdir), "%s/hw", host_parms.userdir);
	Sys_mkdir_err (fs_userdir);
	FS_AddGameDirectory (va("%s/hw", fs_basedir), true);
	// error out for H2W builds if GAME_HEXENWORLD isn't set
	if (!(gameflags & GAME_HEXENWORLD))
		Sys_Error ("You must have the HexenWorld data installed");
#endif

// this is the end of our base searchpath:
// any set gamedirs, such as those from -game commandline
// arguments, from exec'ed configs or the ones dictated by
// the server, will be freed up to here upon a new gamedir
// command
	fs_base_searchpaths = fs_searchpaths;

	Q_strlcpy_err(fs_savedir, fs_userdir, sizeof(fs_savedir));

	i = COM_CheckParm ("-game");
	if (i && !(gameflags & GAME_REGISTERED))
	{
	// only registered versions can do -game
		Sys_Error ("You must have the full version of Hexen II to play modified games");
	}
	else
	{
	// add basedir/gamedir as an override game
		if (i && i < com_argc-1)
			FS_Gamedir (com_argv[i+1]);
	}

// finish the filesystem setup
	oem.flags &= ~CVAR_ROM;
	registered.flags &= ~CVAR_ROM;
	if (gameflags & GAME_REGISTERED)
	{
		snprintf (temp, sizeof(temp), "registered");
		Cvar_Set ("registered", "1");
	}
	else if (gameflags & GAME_OEM)
	{
		snprintf (temp, sizeof(temp), "oem");
		Cvar_Set ("oem", "1");
	}
	else if (gameflags & GAME_DEMO)
	{
		snprintf (temp, sizeof(temp), "demo");
	}
	else
	{
	//	snprintf (temp, sizeof(temp), "unknown");
	// no proper Raven data: it's best to error out here
		Sys_Error ("Unable to find a proper Hexen II installation");
	}
	oem.flags |= CVAR_ROM;
	registered.flags |= CVAR_ROM;

	Sys_Printf ("Playing %s version.\n", temp);
}

