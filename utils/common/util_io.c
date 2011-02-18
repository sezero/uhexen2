/*
	util_io.c
	file and directory utilities

	$Id$
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
#else	/* Unix */
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#endif
#include "util_io.h"
#include "cmdlib.h"

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

static HANDLE  findhandle;
static WIN32_FIND_DATA finddata;

char *Q_FindNextFile (void)
{
	BOOL	retval;

	if (!findhandle || findhandle == INVALID_HANDLE_VALUE)
		return NULL;

	retval = FindNextFile(findhandle,&finddata);
	while (retval)
	{
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			retval = FindNextFile(findhandle,&finddata);
			continue;
		}

		return finddata.cFileName;
	}

	return NULL;
}

char *Q_FindFirstFile (const char *path, const char *pattern)
{
	char	tmp_buf[256];

	if (findhandle)
		Error ("FindFirst without FindClose");

	q_snprintf (tmp_buf, sizeof(tmp_buf), "%s/%s", path, pattern);
	findhandle = FindFirstFile(tmp_buf, &finddata);

	if (findhandle != INVALID_HANDLE_VALUE)
	{
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return Q_FindNextFile();
		else
			return finddata.cFileName;
	}

	return NULL;
}

void Q_FindClose (void)
{
	if (findhandle != INVALID_HANDLE_VALUE)
		FindClose(findhandle);
	findhandle = NULL;
}

void Q_getwd (char *out, size_t size)
{
	_getcwd (out, size);
	qerr_strlcat(__thisfunc__, __LINE__, out, "\\", size);
}

void Q_mkdir (const char *path)
{
	if (_mkdir (path) != -1)
		return;
	if (errno != EEXIST)
		Error ("Unable to create directory %s", path);
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

char *Q_FindFirstFile (const char *path, const char *pattern)
{
	char	tmp_buf[256];

	if (findhandle == 0)
		Error ("FindFirst without FindClose");

	q_snprintf (tmp_buf, sizeof(tmp_buf), "%s/%s", path, pattern);
	memset (&finddata, 0, sizeof(finddata));

	findhandle = findfirst(tmp_buf, &finddata, FA_ARCH | FA_RDONLY);
	if (findhandle == 0)
		return finddata.ff_name;

	return NULL;
}

char *Q_FindNextFile (void)
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

void Q_getwd (char *out, size_t size)
{
	getcwd (out, size);
	qerr_strlcat(__thisfunc__, __LINE__, out, "/", size);
}

void Q_mkdir (const char *path)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0)
		Error ("Unable to create directory %s", path);
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

#else	/* Unix */

static DIR		*finddir;
static struct dirent	*finddata;
static char		*findpath, *findpattern;
static char		matchpath[256];

void Q_FindClose (void)
{
	if (finddir != NULL)
		closedir(finddir);
	if (findpath != NULL)
		free (findpath);
	if (findpattern != NULL)
		free (findpattern);
	finddir = NULL;
	findpath = NULL;
	findpattern = NULL;
}

char *Q_FindNextFile (void)
{
	struct stat	test;

	if (!finddir)
		return NULL;

	do {
		finddata = readdir(finddir);
		if (finddata != NULL)
		{
			if (!fnmatch (findpattern, finddata->d_name, FNM_PATHNAME))
			{
				q_snprintf(matchpath, sizeof(matchpath), "%s/%s", findpath, finddata->d_name);
				if ( (stat(matchpath, &test) == 0)
							&& S_ISREG(test.st_mode) )
					return finddata->d_name;
			}
		}
	} while (finddata != NULL);

	return NULL;
}

char *Q_FindFirstFile (const char *path, const char *pattern)
{
	size_t	tmp_len;

	if (finddir)
		Error ("FindFirst without FindClose");

	finddir = opendir (path);
	if (!finddir)
		return NULL;

	tmp_len = strlen (pattern);
	findpattern = (char *) calloc (tmp_len + 1, sizeof(char));
	if (!findpattern)
	{
		Q_FindClose();
		return NULL;
	}
	strcpy (findpattern, pattern);
	tmp_len = strlen (path);
	findpath = (char *) calloc (tmp_len + 1, sizeof(char));
	if (!findpath)
	{
		Q_FindClose();
		return NULL;
	}
	strcpy (findpath, path);
	if (tmp_len)
	{
		--tmp_len;
		/* searching / won't be a good idea, for example.. */
		if (findpath[tmp_len] == '/' || findpath[tmp_len] == '\\')
			findpath[tmp_len] = '\0';
	}

	return Q_FindNextFile();
}

void Q_getwd (char *out, size_t size)
{
	getcwd (out, size);
	qerr_strlcat(__thisfunc__, __LINE__, out, "/", size);
}

void Q_mkdir (const char *path)
{
	if (mkdir (path, 0777) != -1)
		return;
	if (errno != EEXIST)
		Error ("Unable to create directory %s", path);
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
		Error("Error opening %s: %s", filename, strerror(errno));

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
		Error("Error opening %s: %s", filename, strerror(errno));

	return f;
}

/*
==============
SafeRead

==============
*/
void SafeRead (FILE *f, void *buffer, int count)
{
	if ( fread (buffer, 1, count, f) != (size_t)count)
		Error("File read failure");
}

/*
==============
SafeWrite

==============
*/
void SafeWrite (FILE *f, const void *buffer, int count)
{
	if (fwrite(buffer, 1, count, f) != (size_t)count)
		Error("File write failure");
}

/*
==============
LoadFile

==============
*/
int LoadFile (const char *filename, void **bufferptr)
{
	FILE	*f;
	long	length;
	void	*buffer;

	f = SafeOpenRead (filename);
	length = Q_filelength (f);
	buffer = malloc (length+1);
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
	if (path[1] == ':')
		path += 2;

	for (ofs = path + 1; *ofs; ofs++)
	{
		c = *ofs;
		if (c == '/' || c == '\\')
		{	// create the directory
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
#if 0
void Q_CopyFile (const char *from, const char *to)
{
	char	temp[1024];
	void	*buffer;
	int	length;

	if (!from || !*from)
		Error ("%s: null source", __thisfunc__);
	if (!to || !*to)
		Error ("%s: null destination", __thisfunc__);
	length = LoadFile (from, &buffer);
	q_strlcpy (temp, to, sizeof(temp));
	CreatePath (temp);
	SaveFile (to, buffer, length);
	free (buffer);
}
#endif

#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int Q_CopyFile (const char *frompath, const char *topath)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*in, *out;
	char		temp[1024];
	int		err = 0;
//	off_t		remaining, count;
	size_t		remaining, count;

	strcpy (temp, topath);
	CreatePath (temp);

	in = fopen (frompath, "rb");
	if (!in)
		Error ("Unable to open file %s", frompath);
	out = fopen (topath, "wb");
	if (!out)
		Error ("Unable to create file %s", topath);

	remaining = Q_filelength (in);
	memset (buf, 0, sizeof(buf));
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);

		fread (buf, 1, count, in);
		err = ferror (in);
		if (err)
			break;

		fwrite (buf, 1, count, out);
		err = ferror (out);
		if (err)
			break;

		remaining -= count;
	}

	fclose (in);
	fclose (out);

	return err;
}

int Q_WriteFileFromHandle (FILE *fromfile, const char *topath, size_t size)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*out;
//	off_t		remaining, count;
	size_t		remaining, count;
	char		temp[1024];
	int		err = 0;

	strcpy (temp, topath);
	CreatePath (temp);

	out = fopen (topath, "wb");
	if (!out)
		Error ("Unable to create file %s", topath);

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

