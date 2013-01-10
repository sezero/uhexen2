/* h2patch3 -- hexen2 pak patch application using xdelta3
 * Copyright (C) 2007-2012  O.Sezer <sezero@users.sourceforge.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include "q_stdint.h"

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
#else /* POSIX */
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "compiler.h"
#include "qsnprint.h"
#include "filenames.h"


#if !defined(MAXPATHLEN)
#if defined(PATH_MAX)
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 256
#endif
#endif
#define MAX_OSPATH	MAXPATHLEN

#include "xdelta3-mainopt.h"

struct other_pak
{
	long			size;
	unsigned long		sum;	/* adler32 */
	const char		*desc;
	struct other_pak const	*next;
};

static const struct other_pak pak0_oem1 = {
	22720659,	0xE9D25D16,
	"Continent of Blackmarsh (m3D, v1.10)",
	NULL
};

static const struct other_pak pak0_oem0 = {
	/* don't have this myself, therefore no patch. */
	22719295,	/**/ 0x01,
	"Continent of Blackmarsh (m3D, v1.08)",
	&pak0_oem1
};

static const struct other_pak pak0_demo1 = {
	27750257,	0xED96172E,
	"Demo (Nov. 1997, v1.11)",
	&pak0_oem0
};

static const struct other_pak pak0_demo0 = {
	23537707,	0x88A46443,
	"Demo (Aug. 1997, v0.42)",
	&pak0_demo1
};

#if 0
static const struct other_pak pak2_oem1 = {
	17742721,	0x5595110E,
	"Continent of Blackmarsh (m3D, v1.10)",
	NULL
};

static const struct other_pak pak2_oem0 = {
	/* don't have this myself, therefore no patch. */
	17739969,	/**/ 0x01,
	"Continent of Blackmarsh (m3D, v1.08)",
	&pak2_oem1
};
#endif

#define NUM_PATCHES	2

struct patch_pak
{
	const char	*dir_name;	/* where the file is	*/
	const char	*filename;	/* file to patch	*/
	const char	*deltaname;	/* delta file to use	*/
	const char	*old_desc;
	const char	*new_desc;

	struct other_pak const	*other_data;
			/* possible descriptions of same-named pak
			 * versions not supported by this program. */

	unsigned long	old_sum, new_sum;	/* adler32	*/
	long	old_size, new_size;
};

static const struct patch_pak patch_data[NUM_PATCHES] =
{
	{  "data1", "pak0.pak",
	   "data1pk0.xd3",
	   "retail, from Hexen II cdrom (v1.03)",
	   "retail, already patched (v1.11)",
	   &pak0_demo0,
	   0xCFF397B9, 0xDCF2218F,
	   21714275, 22704056
	},
	{  "data1", "pak1.pak",
	   "data1pk1.xd3",
	   "retail, from Hexen II cdrom (v1.03)",
	   "retail, already patched (v1.11)",
	   NULL,
	   0x8C787960, 0xD56A2FE8,
	   76958474, 75601170
	}
};

static	char	dst[MAX_OSPATH],
		pat[MAX_OSPATH],
		out[MAX_OSPATH];

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
#define	HOT_VERSION_MIN		6

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
static int Sys_unlink (const char *path)
{
	return remove(path);
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

static long get_millisecs (void)
{
/* UCLOCKS_PER_SEC is defined as 1193180 therefore
 * dividing it by 1000 causes minor precision loss
 * which we don't care much about. */
	return uclock() / (UCLOCKS_PER_SEC / 1000);
}

#elif defined(_WIN32)

static int Sys_unlink (const char *path)
{
	if (DeleteFile(path) != 0)
		return 0;
	return -1;
}

static int Sys_rename (const char *oldp, const char *newp)
{
	if (MoveFile(oldp, newp) != 0)
		return 0;
	return -1;
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

static long get_millisecs (void)
{
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
	/* paks copied off of a cdrom might fail R_OK|W_OK */
	chmod (name, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (Sys_FileType(name) != FS_ENT_FILE)
		return ACCESS_NOFILE;

	if (access(dst, R_OK|W_OK) != 0)
		return ACCESS_NOPERM;

	return ACCESS_FILEOK;
}

static long get_millisecs (void)
{
	struct timeval tv;

	gettimeofday (&tv, NULL);

	return (tv.tv_sec) * 1000L + (tv.tv_usec) / 1000;
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

static const char *other_pak_desc (int num, long len)
{
	const struct other_pak *p = patch_data[num].other_data;

	for ( ; p != NULL; p = p->next)
	{
		if (len == p->size)
			return p->desc;
	}

	return "an unknown pak file";
}


int main (int argc, char **argv)
{
	int	i, num_patched, ret;
	long		len;
	unsigned long	csum;

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
		q_snprintf (out, sizeof(out), "%s%c%s", patch_data[i].dir_name,
						DIR_SEPARATOR_CHAR, patch_tmpname);
		Sys_unlink (out);
	}

#ifdef _WIN32
	setvbuf(stderr, NULL, _IONBF, 0);  /* Do not buffer stderr */
#endif

	for (i = 0; i < NUM_PATCHES; i++)
	{
		q_snprintf (dst, sizeof(dst), "%s%c%s", patch_data[i].dir_name,
					DIR_SEPARATOR_CHAR, patch_data[i].filename);
		fprintf (stdout, "File %s :\n", dst);

		ret = check_access(dst);
		switch (ret)
		{
		case ACCESS_NOFILE:
			fprintf (stderr, "... Error: cannot find!\n");
			return 1;
		case ACCESS_NOPERM:
			fprintf (stderr, "... Error: cannot access, check permissions!\n");
			return 1;
		case ACCESS_FILEOK:
		default:
			break;
		}

		len = Sys_filesize (dst);

		if (len == patch_data[i].new_size)
		{
			fprintf (stdout, "... looks like %s\n", patch_data[i].new_desc);
			fprintf (stdout, "... checksumming...");
			fflush (stdout);
			csum = xd3_calc_adler32(dst);
			if (csum == patch_data[i].new_sum)
				fprintf (stdout, " OK ");
			else	fprintf (stdout, "\n... WARNING: checksum mismatch! file corrupted?\n");
			fprintf (stdout, "... skipped.\n\n");
			h2patch_progress.current_written += patch_data[i].new_size;
			continue;
		}
		if (len != patch_data[i].old_size)
		{
			fprintf (stderr, "... looks like %s\n", other_pak_desc(i, len));
			fprintf (stderr, "... Error: not supported by h2patch!\n");
			return 1;
		}

		fprintf (stdout, "... looks like %s\n", patch_data[i].old_desc);

		q_snprintf (pat, sizeof(pat), "%s%c%s%c%s", DELTA_DIR, DIR_SEPARATOR_CHAR,
						patch_data[i].dir_name, DIR_SEPARATOR_CHAR,
								patch_data[i].deltaname);
		if (Sys_FileType(pat) != FS_ENT_FILE)
		{
			fprintf (stderr, "... Error: delta file not found!\n");
			return 1;
		}

		q_snprintf (out, sizeof(out), "%s%c%s", patch_data[i].dir_name,
						DIR_SEPARATOR_CHAR, patch_tmpname);
		fprintf (stdout, "... applying patch...\n");

		start_file_progress (patch_data[i].new_size);
		ret = xd3_main_patcher(&h2patch_options, dst, pat, out);
		finish_file_progress ();
		if (ret != 0)
		{
			Sys_unlink (out);
			fprintf (stderr, "... Error: patch failed! file corrupted?\n");
			return 2;
		}

		Sys_unlink (dst);
		if (Sys_rename(out, dst) != 0)
		{
			Sys_unlink (out);
			fprintf (stderr, "... Error: failed renaming patched file\n");
			return 2;
		}

		num_patched++;
		fprintf (stdout, "... OK. Patch successful.\n\n");
	}

	fprintf (stdout, "%d file(s) patched.\n", num_patched);
	return 0;
}

