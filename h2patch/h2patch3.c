/*  h2patch3 - hexen2 pak patch application using xdelta3
 *  Copyright (C) 2007-2011.  O.Sezer <sezero@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <direct.h>
#elif defined(__DJGPP__)
#include <dos.h>
#include <dpmi.h>
#include <sys/time.h>
#include <io.h>
#include <unistd.h>
#include <dir.h>
#include <fcntl.h>
#include <conio.h>
#else /* POSIX */
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "compiler.h"
#include "qsnprint.h"


#if !defined(MAXPATHLEN)
#if defined(PATH_MAX)
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 256
#endif
#endif
#define MAX_OSPATH	MAXPATHLEN

#include "md5.h"
#include "xdelta3-mainopt.h"

#define NUM_PATCHES	2

static const struct
{
	const char	*dir_name;	/* where the file is	*/
	const char	*filename;	/* file to patch	*/
	const char	*deltaname;	/* delta file to use	*/
	const char	*old_md5;	/* unpatched md5sum	*/
	const char	*new_md5;	/* md5sum after patch	*/
	long	old_size, new_size;
} patch_data[NUM_PATCHES] =
{
	{  "data1", "pak0.pak",
	   "data1pk0.xd3",
	   "b53c9391d16134cb3baddc1085f18683",
	   "c9675191e75dd25a3b9ed81ee7e05eff",
	   21714275, 22704056
	},
	{  "data1", "pak1.pak",
	   "data1pk1.xd3",
	   "9a2010aafb9c0fe71c37d01292030270",
	   "c2ac5b0640773eed9ebe1cda2eca2ad0",
	   76958474, 75601170
	}
};

static	char	dst[MAX_OSPATH],
		pat[MAX_OSPATH],
		out[MAX_OSPATH];
static	char	csum[CHECKSUM_SIZE+1];

#define DELTA_DIR	"patchdat"
#define cdrom_path	"install/hexen2/data1"
#define patch_tmpname	"uh2patch.tmp"

#define	XPATCH_NONE		0
#define	XPATCH_APPLIED		1
#define	XPATCH_FAIL		2

#define	XD3_VERSION_MAJ		3
#define	XD3_VERSION_MIN		0
#define	XD3_VERSION_SVN		342
#define	HOT_VERSION_MAJ		1
#define	HOT_VERSION_MID		5
#define	HOT_VERSION_MIN		0

#define	H2PATCH_SRCWINSZ	(1<<23)	/* 8 MB is enough */

static xd3_progress_t h2patch_progress;

static void log_print (const char *fmt, ...) __attribute__((__format__(__printf__,1,2)));
static void progress_print (void);

static xd3_options_t h2patch_options =
{
	XD3_DEFAULT_IOPT_SIZE,	/* iopt_size */
	XD3_DEFAULT_WINSIZE,	/* winsize */
	H2PATCH_SRCWINSZ,	/* srcwinsz */
	XD3_DEFAULT_SPREVSZ,	/* sprevsz */

	1,			/* force overwrite */
	0,			/* verbose */
	1,			/* use_checksum */

	&h2patch_progress,	/* progress_data */
	log_print,		/* debug_print() */
	progress_print		/* progress_log() */
};

#define FS_ENT_NONE		(0)
#define FS_ENT_FILE		(1 << 0)
#define FS_ENT_DIRECTORY	(1 << 1)

#define ACCESS_FILEOK		(0)
#define ACCESS_NOFILE		(-1)
#define ACCESS_NOPERM		(-2)

#if defined(__DJGPP__)
static void ask_user_abort (const char *msg)
{
	char	c;

	fprintf (stdout, "%s\n", msg);
	fprintf (stdout, "Continue anyway? [Y/N] ");

	while (1)
	{
		c = getch ();
		if (c == 'y' || c == 'Y' || c == 'n' || c == 'N')
			break;
	}

	fprintf (stdout, "%c\n", c);

	if (c == 'n' || c == 'N')
	{
		fprintf (stderr, "Terminated by user!\n");
		exit (1);
	}
}

static int Sys_unlink (const char *path)
{
	return unlink(path);
}

static int Sys_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

static long Sys_filesize (const char *path)
{
	struct ffblk	f;

	if (findfirst(path, &f, FA_ARCH | FA_RDONLY) != 0)
		return -1;

	return (long) f.ff_fsize;
}

static int Sys_FileType (const char *path)
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

static int check_access (const char *name)
{
	if (Sys_FileType(name) != FS_ENT_FILE)
		return ACCESS_NOFILE;

	return ACCESS_FILEOK;
}

#elif defined(_WIN32)

static int Sys_unlink (const char *path)
{
	int	err;
	err = ! DeleteFile(path);
	return err;
}

static int Sys_rename (const char *oldp, const char *newp)
{
	int	err;
	err = ! MoveFile(oldp, newp);
	return err;
}

static long Sys_filesize (const char *path)
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
static int Sys_FileType (const char *path)
{
	DWORD result = GetFileAttributes(path);

	if (result == INVALID_FILE_ATTRIBUTES)
		return FS_ENT_NONE;
	if (result & FILE_ATTRIBUTE_DIRECTORY)
		return FS_ENT_DIRECTORY;

	return FS_ENT_FILE;
}

static int check_access (const char *name)
{
	if (Sys_FileType(name) != FS_ENT_FILE)
		return ACCESS_NOFILE;

	return ACCESS_FILEOK;
}

#else /* POSIX */

static int Sys_unlink (const char *path)
{
	return unlink(path);
}

static int Sys_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

static long Sys_filesize (const char *path)
{
	struct stat	st;

	if (stat(path, &st) != 0)
		return -1;
	if (! S_ISREG(st.st_mode))
		return -1;

	return (long) st.st_size;
}

static int Sys_FileType (const char *path)
{
	struct stat	st;

	if (stat(path, &st) != 0)
		return FS_ENT_NONE;
	if (S_ISDIR(st.st_mode))
		return FS_ENT_DIRECTORY;
	if (S_ISREG(st.st_mode))
		return FS_ENT_FILE;

	return FS_ENT_NONE;
}

static int check_access (const char *name)
{
	/* if pak files were copied off of a cdrom,
	 * some permissions may be missing and access()
	 * would fail the R_OK|W_OK check. */
	chmod (name, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (Sys_FileType(name) != FS_ENT_FILE)
		return ACCESS_NOFILE;

	if (access(dst, R_OK|W_OK) != 0)
		return ACCESS_NOPERM;

	return ACCESS_FILEOK;
}

#endif

static void print_version (void)
{
	fprintf (stdout, "Hexen II 1.11 pak patch / Hammer of Thyrion (uHexen2) %d.%d.%d\n",
			 HOT_VERSION_MAJ, HOT_VERSION_MID, HOT_VERSION_MIN);
	fprintf (stdout, "Xdelta version 3.0.0, Copyright (C) 2007-2011, Joshua MacDonald\n");
}

static void print_help (void)
{
	fprintf (stdout, "Options:\n");
	fprintf (stdout, "  -h | -help   show help\n");
	fprintf (stdout, "  -version     show version\n");
	fprintf (stdout, "  -verbose     be verbose\n");
}


static long get_millisecs (void)
{
#ifdef _WIN32
/* http://www.codeproject.com/KB/datetime/winapi_datetime_ops.aspx
 * It doesn't matter that the offset is Jan 1, 1601, result
 * is the number of 100 nanosecond units, 100ns * 10,000 = 1ms. */
	SYSTEMTIME st;
	FILETIME ft;
	ULARGE_INTEGER ul1;

	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	ul1.HighPart = ft.dwHighDateTime;
	ul1.LowPart = ft.dwLowDateTime;

	return (long)(ul1.QuadPart / 10000);
#else
/* POSIX. also OK with DJGPP. */
	struct timeval tv;

	gettimeofday (&tv, NULL);

	return (tv.tv_sec) * 1000L + (tv.tv_usec) / 1000;
#endif
}

/*  LOG PRINTING:  */

static char eol_char[4] = {0, 0, 0, 0};
static long starttime;

static void progress_print (void)
{
	long elapsed = get_millisecs () - starttime;

	if (elapsed < 10000)
	{
		fprintf(stderr, "%10lu bytes, %.2fs%s",
			h2patch_progress.current_file_written,
			elapsed / 1000.0, eol_char);
	}
	else
	{
		elapsed /= 1000;
		fprintf(stderr, "%10lu bytes, %3ldm:%02lds%s",
			h2patch_progress.current_file_written,
			elapsed / 60, elapsed % 60, eol_char);
	}
}

static void start_file_progress (long bytes)
{
	h2patch_progress.current_file_written = 0;
	h2patch_progress.current_file_total = bytes;
	eol_char[0] = '\r';
	starttime = get_millisecs ();
}

static void finish_file_progress (void)
{
	eol_char[0] = '\n';
	if (h2patch_progress.current_file_written != 0)
		progress_print ();
}

static void log_print (const char *fmt, ...)
{
	va_list		argptr;

	va_start (argptr, fmt);
	vfprintf (stderr, fmt, argptr);
	va_end (argptr);
}


int main (int argc, char **argv)
{
	int	i, num_patched, ret;
	long		len;
#if defined(__DJGPP__)
	unsigned long	bytes;
	unsigned int	drive;
	struct diskfree_t  df;
#endif	/* __DJGPP__ */

	print_version ();
	for (i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help"))
		{
			print_help ();
			return 0;
		}
		else if (!strcmp(argv[i], "-version"))
		{
			return 0;
		}
		else if (!strcmp(argv[i], "-verbose"))
		{
			h2patch_options.verbose = 1;
		}
		else
		{
			fprintf (stderr, "Unrecognized option \"%s\"\n", argv[i]);
			print_help ();
			return 1;
		}
	}

	memset (&h2patch_progress, 0, sizeof(xd3_progress_t));
	num_patched = 0;

	for (i = 0; i < NUM_PATCHES; i++)
	{
		h2patch_progress.total_bytes += patch_data[i].new_size;
		/* delete our temp files from possible previous runs */
		q_snprintf (out, sizeof(out), "%s/%s",
						patch_data[i].dir_name,
							 patch_tmpname);
		Sys_unlink (out);
	}

#if defined(__DJGPP__)
	/* get the free disk space */
	_dos_getdrive(&drive);
	if (_dos_getdiskfree(drive, &df) != 0)
	{
		fprintf (stderr, "Error: Unable to determine free diskspace.\n");
		return 2;
	}

	bytes = (unsigned long)df.avail_clusters *
		(unsigned long)df.bytes_per_sector *
		(unsigned long)df.sectors_per_cluster;
#endif	/* __DJGPP__ */

#ifdef _WIN32
	setvbuf(stderr, NULL, _IONBF, 0);  /* Do not buffer stderr */
#endif

	for (i = 0; i < NUM_PATCHES; i++)
	{
		q_snprintf (dst, sizeof(dst), "%s/%s",
						patch_data[i].dir_name,
						patch_data[i].filename);

		ret = check_access(dst);
		switch (ret)
		{
		case ACCESS_NOFILE:
			fprintf (stderr, "Error: File %s not found\n", dst);
			return 1;
		case ACCESS_NOPERM:
			fprintf (stderr, "Error: cannot access %s, check permissions!\n", dst);
			return 1;
		case ACCESS_FILEOK:
		default:
			break;
		}

		len = Sys_filesize (dst);
		if (len != patch_data[i].old_size &&
			len != patch_data[i].new_size)
		{
			fprintf (stderr, "Error: File %s is an incompatible version\n", dst);
			return 1;
		}

		fprintf (stdout, "Checksumming %s, please wait...\n", dst);
		memset (csum, 0, sizeof(csum));
		md5_compute(dst, csum);
		if (strcmp(csum, patch_data[i].new_md5) == 0)
		{
			fprintf (stdout, "File %s is already patched\n\n", dst);
			h2patch_progress.current_written += patch_data[i].new_size;
			continue;
		}
		if (strcmp(csum, patch_data[i].old_md5) != 0)
		{
			fprintf (stderr, "Error: File %s is an incompatible version\n", dst);
			return 1;
		}

		q_snprintf (pat, sizeof(pat), "%s/%s/%s",
					DELTA_DIR, patch_data[i].dir_name,
						   patch_data[i].deltaname);
		if (Sys_FileType(pat) != FS_ENT_FILE)
		{
			fprintf (stderr, "Error: File %s not found\n", pat);
			return 1;
		}

#if defined(__DJGPP__)
		if (bytes < patch_data[i].new_size)
		{
			ask_user_abort ("Not enough disk space for patching");
		}
#endif	/* __DJGPP__ */

		q_snprintf (out, sizeof(out), "%s/%s",
						patch_data[i].dir_name,
							 patch_tmpname);
		fprintf (stdout, "Patching %s, please wait...\n", dst);

		start_file_progress (patch_data[i].new_size);
		ret = xd3_main_patcher(&h2patch_options, dst, pat, out);
		finish_file_progress ();
		if (ret != 0)
		{
			Sys_unlink (out);
			fprintf (stderr, "Error: Failed patching %s\n", dst);
			return 2;
		}

		fprintf (stdout, "Checksumming %s, please wait...\n", out);
		memset (csum, 0, sizeof(csum));
		md5_compute(out, csum);
		if (strcmp(csum, patch_data[i].new_md5) != 0)
		{
			Sys_unlink (out);
			fprintf (stderr, "Error: File %s failed checksum after patching\n", dst);
			return 2;
		}

		Sys_unlink (dst);
		if (Sys_rename(out, dst) != 0)
		{
			Sys_unlink (out);
			fprintf (stderr, "Error: Failed renaming patched file to %s\n", patch_data[i].filename);
			return 2;
		}

		num_patched++;
		fprintf (stdout, "Patch successful for %s\n\n", dst);
	}

	fprintf (stdout, "%d file(s) patched.\n", num_patched);
	return 0;
}

