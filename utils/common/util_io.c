/*
	util_io.c
	file and directory utilities

	$Id: util_io.c,v 1.16 2009-02-20 18:55:04 sezero Exp $
*/


// HEADER FILES ------------------------------------------------------------

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include <sys/stat.h>
#include <errno.h>
#ifdef PLATFORM_WINDOWS
#include <conio.h>
#include <io.h>
#include <direct.h>
#include <windows.h>
#include "io_msvc.h"
#else	/* Unix */
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

#else	/* FindFile for Unix */

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
#endif	/* End of FindFile */

void Q_getwd (char *out, size_t size)
{
#ifdef PLATFORM_WINDOWS
	_getcwd (out, size);
	qerr_strlcat(__thisfunc__, __LINE__, out, "\\", size);
#else
	getcwd (out, size);
	qerr_strlcat(__thisfunc__, __LINE__, out, "/", size);
#endif
}

void Q_mkdir (const char *path)
{
#ifdef PLATFORM_WINDOWS
	if (_mkdir (path) != -1)
		return;
#else
	if (mkdir (path, 0777) != -1)
		return;
#endif
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

/*
==============
Q_filelength

==============
*/
int Q_filelength (FILE *f)
{
	int		pos;
	int		end;

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
	int	length;
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
void Q_CopyFile (const char *from, const char *to)
{
	char	temp[1024];
	void	*buffer;
	int	length;

	length = LoadFile (from, &buffer);
	q_strlcpy (temp, to, sizeof(temp));
	CreatePath (temp);
	SaveFile (to, buffer, length);
	free (buffer);
}

