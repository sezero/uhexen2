/*
 * util_io.c -- file and directory utilities
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


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include <errno.h>
#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <direct.h>
#include <windows.h>
#include "io_msvc.h"
#elif defined(PLATFORM_DOS)
#include <unistd.h>
#include <sys/stat.h>
#include <dos.h>
#include <io.h>
#include <dir.h>
#include <fcntl.h>
#elif defined(PLATFORM_AMIGA)
#include <proto/exec.h>
#include <proto/dos.h>
#include <time.h>
#else	/* Unix */
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#endif
#include "util_io.h"
#include "cmdlib.h"
#include "filenames.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

#ifdef PLATFORM_WINDOWS

static HANDLE findhandle = INVALID_HANDLE_VALUE;
static WIN32_FIND_DATA finddata;
static char		findstr[256];

const char *Q_FindFirstFile (const char *path, const char *pattern)
{
	if (findhandle != INVALID_HANDLE_VALUE)
		COM_Error ("FindFirst without FindClose");
	q_snprintf (findstr, sizeof(findstr), "%s/%s", path, pattern);
	findhandle = FindFirstFile(findstr, &finddata);
	if (findhandle == INVALID_HANDLE_VALUE)
		return NULL;
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return Q_FindNextFile();
	return finddata.cFileName;
}

const char *Q_FindNextFile (void)
{
	if (findhandle == INVALID_HANDLE_VALUE)
		return NULL;
	while (FindNextFile(findhandle, &finddata) != 0)
	{
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		return finddata.cFileName;
	}
	return NULL;
}

void Q_FindClose (void)
{
	if (findhandle != INVALID_HANDLE_VALUE)
	{
		FindClose(findhandle);
		findhandle = INVALID_HANDLE_VALUE;
	}
}

void Q_getwd (char *out, size_t size, qboolean trailing_dirsep)
{
	size_t sz;

	if (_getcwd(out, size) == NULL)
		COM_Error ("Couldn't determine current directory");
	if (!trailing_dirsep)
		return;
	sz = strlen(out);
	if (!sz || out[sz - 1] == '\\' || out[sz - 1] == '/')
		return;
	qerr_strlcat(__thisfunc__, __LINE__, out, "\\", size);
}

void Q_mkdir (const char *path)
{
	if (_mkdir (path) != -1)
		return;
	if (errno != EEXIST)
		COM_Error ("Unable to create directory %s", path);
}

int Q_rmdir (const char *path)
{
	return _rmdir(path);
}

int Q_unlink (const char *path)
{
	return _unlink(path);
}

int Q_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

long Q_filesize (const char *path)
{
	HANDLE fh;
	WIN32_FIND_DATA data;
	long size;

	fh = FindFirstFile(path, &data);
	if (fh == INVALID_HANDLE_VALUE)
		return -1;
	FindClose(fh);
	if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return -1;
//	we're not dealing with gigabytes of files.
//	size should normally smaller than INT_MAX.
//	size = (data.nFileSizeHigh * (MAXDWORD + 1)) + data.nFileSizeLow;
	size = (long) data.nFileSizeLow;
	return size;
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES	((DWORD)-1)
#endif
int Q_FileType (const char *path)
{
	DWORD result = GetFileAttributes(path);

	if (result == INVALID_FILE_ATTRIBUTES)
		return FS_ENT_NONE;
	if (result & FILE_ATTRIBUTE_DIRECTORY)
		return FS_ENT_DIRECTORY;

	return FS_ENT_FILE;
}


#elif defined(PLATFORM_DOS)

static struct ffblk	finddata;
static int		findhandle = -1;
static char		findstr[256];

const char *Q_FindFirstFile (const char *path, const char *pattern)
{
	if (findhandle == 0)
		COM_Error ("FindFirst without FindClose");

	q_snprintf (findstr, sizeof(findstr), "%s/%s", path, pattern);
	memset (&finddata, 0, sizeof(finddata));

	findhandle = findfirst(findstr, &finddata, FA_ARCH | FA_RDONLY);
	if (findhandle == 0)
		return finddata.ff_name;

	return NULL;
}

const char *Q_FindNextFile (void)
{
	if (findhandle != 0)
		return NULL;

	if (findnext(&finddata) == 0)
		return finddata.ff_name;

	return NULL;
}

void Q_FindClose (void)
{
	findhandle = -1;
}

void Q_getwd (char *out, size_t size, qboolean trailing_dirsep)
{
	size_t sz;

	if (getcwd(out, size) == NULL)
		COM_Error ("Couldn't determine current directory");
	if (!trailing_dirsep)
		return;
	sz = strlen(out);
	if (!sz || out[sz - 1] == '\\' || out[sz - 1] == '/')
		return;
	qerr_strlcat(__thisfunc__, __LINE__, out, "/", size);
}

void Q_mkdir (const char *path)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0)
		COM_Error ("Unable to create directory %s", path);
}

int Q_rmdir (const char *path)
{
	return rmdir(path);
}

int Q_unlink (const char *path)
{
	return remove(path);
}

int Q_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

long Q_filesize (const char *path)
{
	struct ffblk	f;

	if (findfirst(path, &f, FA_ARCH | FA_RDONLY) != 0)
		return -1;

	return (long) f.ff_fsize;
}

int Q_FileType (const char *path)
{
	int attr = _chmod(path, 0);
	/* Root directories on some non-local drives
	   (e.g. CD-ROM) as well as devices may fail
	   _chmod, but we are not interested in such
	   cases.  */
	if (attr == -1)
		return FS_ENT_NONE;
	if (attr & _A_SUBDIR)
		return FS_ENT_DIRECTORY;
	if (attr & _A_VOLID)	/* we shouldn't hit this! */
		return FS_ENT_DIRECTORY;

	return FS_ENT_FILE;
}

#elif defined(PLATFORM_AMIGA)

#define PATH_SIZE 1024
static struct AnchorPath *apath;
static BPTR oldcurrentdir;
static STRPTR pattern_str;

static STRPTR pattern_helper (const char *pat)
{
	char	*pdup;
	const char	*p;
	int	n;

	for (n = 0, p = pat; *p != '\0'; ++p, ++n)
	{
		if ((p = strchr (p, '*')) == NULL)
			break;
	}

	if (n == 0)
		pdup = SafeStrdup(pat);
	else
	{
	/* replace each "*" by "#?" */
		n += (int) strlen(pat) + 1;
		pdup = (char *) SafeMalloc (n);

		for (n = 0, p = pat; *p != '\0'; ++p, ++n)
		{
			if (*p != '*')
				pdup[n] = *p;
			else
			{
				pdup[n] = '#'; ++n;
				pdup[n] = '?';
			}
		}
		pdup[n] = '\0';
	}

	return (STRPTR) pdup;
}

const char *Q_FindFirstFile (const char *path, const char *pattern)
{
	BPTR newdir;

	if (apath)
		COM_Error ("Sys_FindFirst without FindClose");

	apath = AllocVec (sizeof(struct AnchorPath) + PATH_SIZE, MEMF_CLEAR);
	if (!apath)
		return NULL;

	apath->ap_Strlen = PATH_SIZE;
	apath->ap_BreakBits = 0;
	apath->ap_Flags = APB_DOWILD | !APB_DODIR;

	newdir = Lock((const STRPTR) path, SHARED_LOCK);
	if (newdir)
		oldcurrentdir = CurrentDir(newdir);
	else
	{
		FreeVec(apath);
		return NULL;
	}

	pattern_str = pattern_helper (pattern);

	if (MatchFirst((const STRPTR) pattern_str, apath) == 0)
		return (const char *) (apath->ap_Info.fib_FileName);

	return NULL;
}

const char *Q_FindNextFile (void)
{
	if (!apath)
		return NULL;

	if (MatchNext(apath) == 0)
		return (const char *) (apath->ap_Info.fib_FileName);

	return NULL;
}

void Q_FindClose (void)
{
	if (apath == NULL)
		return;
	MatchEnd(apath);
	FreeVec(apath);
	UnLock(CurrentDir(oldcurrentdir));
	oldcurrentdir = NULL;
	apath = NULL;
	free (pattern_str);
	pattern_str = NULL;
}

void Q_getwd (char *out, size_t size, qboolean trailing_dirsep)
{
#if 0
	qerr_strlcpy(__thisfunc__, __LINE__, out, "PROGDIR:", size);
#else
	size_t sz;
	if (NameFromLock(((struct Process *) FindTask(NULL))->pr_CurrentDir, (STRPTR) out, size) == 0)
		COM_Error ("Couldn't determine current directory");
	if (!trailing_dirsep)
		return;
	sz = strlen(out);
	if (!sz || out[sz - 1] == ':' || out[sz - 1] == '/')
		return;
	qerr_strlcat(__thisfunc__, __LINE__, out, "/", size);
#endif
}

void Q_mkdir (const char *path)
{
	BPTR lock = CreateDir((const STRPTR) path);

	if (lock)
	{
		UnLock(lock);
		return;
	}
	if (IoErr() == ERROR_OBJECT_EXISTS)
		return;

	COM_Error("Unable to create directory %s", path);
}

int Q_rmdir (const char *path)
{
	if (DeleteFile((const STRPTR) path) != 0)
		return 0;
	return -1;
}

int Q_unlink (const char *path)
{
	if (DeleteFile((const STRPTR) path) != 0)
		return 0;
	return -1;
}

int Q_rename (const char *oldp, const char *newp)
{
	if (Rename((const STRPTR) oldp, (const STRPTR) newp) != 0)
		return 0;
	return -1;
}

long Q_filesize (const char *path)
{
	long size = -1;
	BPTR fh = Open((const STRPTR) path, MODE_OLDFILE);
	if (fh != NULL)
	{
		struct FileInfoBlock *fib = (struct FileInfoBlock*)
					AllocDosObject(DOS_FIB, NULL);
		if (fib != NULL)
		{
			if (ExamineFH(fh, fib))
				size = fib->fib_Size;
			FreeDosObject(DOS_FIB, fib);
		}
		Close(fh);
	}
	return size;
}

int Q_FileType (const char *path)
{
	int type = FS_ENT_NONE;
	BPTR fh = Open((const STRPTR) path, MODE_OLDFILE);
	if (fh != NULL)
	{
		struct FileInfoBlock *fib = (struct FileInfoBlock*)
					AllocDosObject(DOS_FIB, NULL);
		if (fib != NULL)
		{
			if (ExamineFH(fh, fib))
			{
				if (fib->fib_DirEntryType >= 0)
					type = FS_ENT_DIRECTORY;
				else	type = FS_ENT_FILE;
			}
			FreeDosObject(DOS_FIB, fib);
		}
		Close(fh);
	}
	return type;
}

#else	/* Unix */

static DIR		*finddir;
static struct dirent	*finddata;
static char		*findpath, *findpattern;
static char		matchpath[256];

const char *Q_FindFirstFile (const char *path, const char *pattern)
{
	if (finddir)
		COM_Error ("FindFirst without FindClose");

	finddir = opendir (path);
	if (!finddir)
		return NULL;
	findpattern = SafeStrdup (pattern);
	findpath = SafeStrdup (path);

	if (*findpath != '\0')
	{
	/* searching under "/" won't be a good idea, for example.. */
		size_t siz = strlen(findpath) - 1;
		if (findpath[siz] == '/' || findpath[siz] == '\\')
			findpath[siz] = '\0';
	}

	return Q_FindNextFile();
}

const char *Q_FindNextFile (void)
{
	struct stat	test;

	if (!finddir)
		return NULL;

	while ((finddata = readdir(finddir)) != NULL)
	{
		if (!fnmatch (findpattern, finddata->d_name, FNM_PATHNAME))
		{
			q_snprintf(matchpath, sizeof(matchpath), "%s/%s", findpath, finddata->d_name);
			if ( (stat(matchpath, &test) == 0)
						&& S_ISREG(test.st_mode))
				return finddata->d_name;
		}
	}

	return NULL;
}

void Q_FindClose (void)
{
	if (finddir != NULL)
	{
		closedir(finddir);
		finddir = NULL;
	}
	if (findpath != NULL)
	{
		free (findpath);
		findpath = NULL;
	}
	if (findpattern != NULL)
	{
		free (findpattern);
		findpattern = NULL;
	}
}

void Q_getwd (char *out, size_t size, qboolean trailing_dirsep)
{
	size_t sz;

	if (getcwd(out, size) == NULL)
		COM_Error ("Couldn't determine current directory");
	if (!trailing_dirsep)
		return;
	sz = strlen(out);
	if (!sz || out[sz - 1] == '/')
		return;
	qerr_strlcat(__thisfunc__, __LINE__, out, "/", size);
}

void Q_mkdir (const char *path)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0)
	{
		rc = errno;
		COM_Error ("Unable to create directory %s: %s", path, strerror(rc));
	}
}

int Q_rmdir (const char *path)
{
	return rmdir(path);
}

int Q_unlink (const char *path)
{
	return unlink(path);
}

int Q_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

long Q_filesize (const char *path)
{
	struct stat	st;

	if (stat(path, &st) != 0)
		return -1;
	if (! S_ISREG(st.st_mode))
		return -1;

	return (long) st.st_size;
}

int Q_FileType (const char *path)
{
	/*
	if (access(path, R_OK) == -1)
		return 0;
	*/
	struct stat	st;

	if (stat(path, &st) != 0)
		return FS_ENT_NONE;
	if (S_ISDIR(st.st_mode))
		return FS_ENT_DIRECTORY;
	if (S_ISREG(st.st_mode))
		return FS_ENT_FILE;

	return FS_ENT_NONE;
}
#endif	/* End of platform-specifics */

/*
==============
Q_filelength

==============
*/
long Q_filelength (FILE *f)
{
	long	pos, end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);
	return end;
}

/*
==============
SafeOpenWrite

==============
*/
FILE *SafeOpenWrite (const char *filename)
{
	FILE	*f;

	f = fopen(filename, "wb");
	if (!f)
		COM_Error("Error opening %s: %s", filename, strerror(errno));

	return f;
}

/*
==============
SafeOpenRead

==============
*/
FILE *SafeOpenRead (const char *filename)
{
	FILE	*f;

	f = fopen(filename, "rb");
	if (!f)
		COM_Error("Error opening %s: %s", filename, strerror(errno));

	return f;
}

/*
==============
SafeRead

==============
*/
void SafeRead (FILE *f, void *buffer, int count)
{
	if (fread (buffer, 1, count, f) != (size_t)count)
		COM_Error("File read failure");
}

/*
==============
SafeWrite

==============
*/
void SafeWrite (FILE *f, const void *buffer, int count)
{
	if (fwrite(buffer, 1, count, f) != (size_t)count)
		COM_Error("File write failure");
}

/*
==============
LoadFile

==============
*/
int LoadFile (const char *filename, void **bufferptr)
{
	FILE	*f;
	size_t	length;
	void	*buffer;

	f = SafeOpenRead (filename);
	length = (size_t) Q_filelength (f);
	buffer = malloc (length + 1);
	if (!buffer)
		COM_Error ("%s failed for %lu bytes.", __thisfunc__, (unsigned long)length);
	((char *)buffer)[length] = 0;
	SafeRead(f, buffer, length);
	fclose (f);

	*bufferptr = buffer;
	return length;
}

/*
==============
SaveFile

==============
*/
void SaveFile (const char *filename, const void *buffer, int count)
{
	FILE	*f;

	f = SafeOpenWrite (filename);
	SafeWrite (f, buffer, count);
	fclose (f);
}

/*
============
CreatePath
============
*/
void CreatePath (char *path)
{
	char	*ofs, c;

	if (!path || !*path)
		return;

	ofs = path;
	if (HAS_DRIVE_SPEC(ofs))
		ofs = STRIP_DRIVE_SPEC(ofs);
	if (IS_DIR_SEPARATOR(*ofs))
		ofs++;

	for ( ; *ofs; ofs++)
	{
		c = *ofs;
		if (IS_DIR_SEPARATOR(c))
		{
			*ofs = 0;
			Q_mkdir (path);
			*ofs = c;
		}
	}
}

/*
============
Q_CopyFile

Used to archive source files
============
*/
#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int Q_CopyFile (const char *frompath, const char *topath)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*in, *out;
	char	temp[1024];
/*	off_t	remaining, count;*/
	size_t	remaining, count;

	strcpy (temp, topath);
	CreatePath (temp);

	in = fopen (frompath, "rb");
	if (!in)
		COM_Error ("Unable to open file %s", frompath);
	out = fopen (topath, "wb");
	if (!out)
		COM_Error ("Unable to create file %s", topath);

	remaining = Q_filelength (in);
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else	count = sizeof(buf);

		if (fread(buf, 1, count, in) != count)
			break;
		if (fwrite(buf, 1, count, out) != count)
			break;

		remaining -= count;
	}

	fclose (in);
	fclose (out);

	return (remaining == 0)? 0 : 1;
}

int Q_WriteFileFromHandle (FILE *fromfile, const char *topath, size_t size)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*out;
/*	off_t	remaining, count;*/
	size_t	remaining, count;
	char	temp[1024];

	strcpy (temp, topath);
	CreatePath (temp);

	out = fopen (topath, "wb");
	if (!out)
		COM_Error ("Unable to create file %s", topath);

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

