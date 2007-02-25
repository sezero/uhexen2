/*
	quakefs.c
	quake file io

	$Id: quakeio.c,v 1.4 2007-02-25 19:02:26 sezero Exp $
*/

#define _NEED_SEARCHPATH_T

#include "quakedef.h"
#include "pakfile.h"
#ifdef _WIN32
#include <io.h>
#endif
#ifdef PLATFORM_UNIX
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <time.h>
#include <utime.h>


size_t		qio_filesize;	// size of the last file opened through QIO
int		file_from_pak;	// ZOID: global indicating that file came from a pak


/*
================
QIO_filelength
================
*/
static size_t QIO_filelength (FILE *f)
{
	long		pos, end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return (size_t)end;
}

/*
============
QIO_WriteFile

The filename will be prefixed by the current game directory
Returns 0 on success, 1 on error.
============
*/
int QIO_WriteFile (const char *filename, const void *data, size_t len)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	size_t	size;

	if (snprintf(name, sizeof(name), "%s/%s", fs_userdir, filename) >= sizeof(name))
	{
		Con_Printf ("%s: string buffer overflow!\n", __FUNCTION__);
		return 1;
	}

	f = fopen (name, "wb");
	if (!f)
	{
		Con_Printf ("Error opening %s\n", filename);
		return 1;
	}

	Sys_Printf ("%s: %s\n", __FUNCTION__, name);
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
QIO_CreatePath

Creates directory under user's path, making parent directories
as needed. The path must either be a path to a file, or, if the
full path is meant to be created, it must have the trailing path
seperator. Only used for CopyFile and download. Returns 0 on
success, non-zero on error.
============
*/
int QIO_CreatePath (char *path)
{
	char	*ofs;
	int		error_state = 0;
	size_t		offset;

	if (!path || !path[0])
	{
		Con_Printf ("%s: no path!\n", __FUNCTION__);
		return 1;
	}

	if (strstr(path, ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return 1;
	}

	ofs = host_parms.userdir;
	if (strstr(path, ofs) != path)
	{
		Sys_Error ("Attempted to create a directory out of user's path");
		return 1;
	}

	offset = strlen(ofs);
	for (ofs = path+offset ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			error_state = Sys_mkdir (path);
			*ofs = '/';
			if (error_state)
				break;
		}
	}

	return error_state;
}


/*
===========
QIO_CopyFile

Copies the FROMPATH file as TOPATH file, creating any dirs needed.
Used for saving the game. Returns 0 on success, non-zero on error.
===========
*/
#define	COPY_READ_BUFSIZE		16384
int QIO_CopyFile (const char *frompath, const char *topath)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*in, *out;
	struct stat	st;
	int		err = 0;
//	off_t		remaining, count;
	size_t		remaining, count;

	if ( !frompath || !topath )
	{
		Con_Printf ("%s: null input\n", __FUNCTION__);
		return 1;
	}

	if ( stat (frompath, &st) != 0 )
	{
		Con_Printf ("%s: unable to stat %s\n", frompath, __FUNCTION__);
		return 1;
	}
//	remaining = st.st_size;

	in = fopen (frompath, "rb");
	if (!in)
	{
		Con_Printf ("%s: unable to open %s\n", frompath, __FUNCTION__);
		return 1;
	}
	remaining = QIO_filelength(in);

	// create directories up to the cache file
	Q_strlcpy (buf, topath, sizeof(buf));
	if (QIO_CreatePath (buf))
	{
		Con_Printf ("%s: unable to create directory\n", __FUNCTION__);
		fclose (in);
		return 1;
	}

	out = fopen(topath, "wb");
	if (!out)
	{
		Con_Printf ("%s: unable to create %s\n", topath, __FUNCTION__);
		fclose (in);
		return 1;
	}

	memset (buf, 0, sizeof(buf));
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);

		fread (buf, 1, count, in);
		err = ferror(in);
		if (err)
			break;

		fwrite (buf, 1, count, out);
		err = ferror(out);
		if (err)
			break;

		remaining -= count;
	}

	fclose (in);
	fclose (out);

	if (!err)
	{
	// restore the file's timestamp
		struct utimbuf		tm;
		tm.actime = time (NULL);
		tm.modtime = st.st_mtime;
		utime (topath, &tm);
	}

	return err;
}

/*
===========
QIO_FOpenFile

Finds the file in the search path.
Sets qio_filesize and one of handle or file
===========
*/
size_t QIO_FOpenFile (const char *filename, FILE **file, qboolean override_pack)
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
				if (!strcmp (pak->files[i].name, filename))
				{	// found it!
					// open a new file on the pakfile
					*file = fopen (pak->filename, "rb");
					if (!*file)
						Sys_Error ("Couldn't reopen %s", pak->filename);
					fseek (*file, pak->files[i].filepos, SEEK_SET);
					qio_filesize = (size_t) pak->files[i].filelen;
					file_from_pak = 1;
					return qio_filesize;
				}
		}
		else
		{
	// check a file in the directory tree
#ifndef H2W
			if (!(gameflags & GAME_REGISTERED) && !override_pack)
			{	// if not a registered version, don't ever go beyond base
				if ( strchr (filename, '/') || strchr (filename,'\\'))
					continue;
			}
#endif	// !H2W

			snprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
			if (access(netpath, R_OK) == -1)
				continue;

			*file = fopen (netpath, "rb");
			if (!*file)
				Sys_Error ("Couldn't reopen %s", netpath);
			return QIO_filelength (*file);
		}
	}

	Sys_Printf ("%s: can't find %s\n", __FUNCTION__, filename);

	*file = NULL;
	qio_filesize = (size_t)-1;
	return qio_filesize;
}

/*
============
QFS_FileInGamedir

Reports the existance of a file with read permissions in
fs_gamedir or fs_userdir. -1 is returned on failure,
ie. the return value of the access() function Files in
pakfiles are NOT meant for this procedure!
============
*/
int QFS_FileInGamedir (const char *fname)
{
	int	ret;

	ret = access (va("%s/%s", fs_userdir, fname), R_OK);
	if (ret == -1)
		ret = access (va("%s/%s", fs_gamedir, fname), R_OK);

	return ret;
}

/*
============
QIO_LoadFile

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
static cache_user_t *loadcache;
static byte	*loadbuf;
static size_t		loadsize;

static byte *QIO_LoadFile (const char *path, int usehunk)
{
	FILE	*h;
	byte	*buf;
	char	base[32];
	size_t		len;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = qio_filesize = QIO_FOpenFile (path, &h, false);
	if (!h)
		return NULL;

// extract the filename base name for hunk tag
	COM_FileBase (path, base);

	switch (usehunk)
	{
	case LOADFILE_HUNK:
		buf = Hunk_AllocName (len+1, base);
		break;
	case LOADFILE_TEMPHUNK:
		buf = Hunk_TempAlloc (len+1);
		break;
	case LOADFILE_ZONE:
		buf = Z_Malloc (len+1);
		break;
	case LOADFILE_CACHE:
		buf = Cache_Alloc (loadcache, len+1, base);
		break;
	case LOADFILE_STACK:
		if (len + 1 > loadsize)
			buf = Hunk_TempAlloc (len+1);
		else
			buf = loadbuf;
		break;
	case LOADFILE_BUF:
		// Pa3PyX: like 4, except uses hunk (not temp) if no space
		if (len + 1 > loadsize)
			buf = Hunk_AllocName(len + 1, path);
		else
			buf = loadbuf;
		break;
	case LOADFILE_MALLOC:
		buf = Q_malloc (len+1);
		break;
	default:
		Sys_Error ("%s: bad usehunk", __FUNCTION__);
	}

	if (!buf)
		Sys_Error ("%s: not enough space for %s", __FUNCTION__, path);

	((byte *)buf)[len] = 0;

#if !defined(SERVERONLY) && !defined(GLQUAKE)
	Draw_BeginDisc ();
#endif
	fread (buf, 1, len, h);
	fclose (h);
#if !defined(SERVERONLY) && !defined(GLQUAKE)
	Draw_EndDisc ();
#endif
	return buf;
}

byte *QIO_LoadHunkFile (const char *path)
{
	return QIO_LoadFile (path, LOADFILE_HUNK);
}

byte *QIO_LoadZoneFile (const char *path)
{
	return QIO_LoadFile (path, LOADFILE_ZONE);
}

byte *QIO_LoadTempFile (const char *path)
{
	return QIO_LoadFile (path, LOADFILE_TEMPHUNK);
}

void QIO_LoadCacheFile (const char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	QIO_LoadFile (path, LOADFILE_CACHE);
}

// uses temp hunk if larger than bufsize
byte *QIO_LoadStackFile (const char *path, void *buffer, size_t bufsize)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = QIO_LoadFile (path, LOADFILE_STACK);

	return buf;
}

// Pa3PyX: Like QIO_LoadStackFile, excepts loads onto
// the hunk (instead of temp) if there is no space
byte *QIO_LoadBufFile (const char *path, void *buffer, size_t *bufsize)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = (*bufsize) + 1;
	buf = QIO_LoadFile (path, LOADFILE_BUF);
	if (buf && !(*bufsize))
		*bufsize = qio_filesize;

	return buf;
}

// LordHavoc: returns malloc'd memory
byte *QIO_LoadMallocFile (const char *path)
{
	return QIO_LoadFile (path, LOADFILE_MALLOC);
}

