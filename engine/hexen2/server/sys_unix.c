/*
 * sys_unix.c -- Unix system interface code
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
 * Copyright (C) 2004-2005  Steven Atkinson <stevenaaus@yahoo.com>
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
#include "userdir.h"
#include "debuglog.h"

#include <errno.h>
#include <unistd.h>
#if DO_USERDIRS
#include <pwd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <dirent.h>
#include <fnmatch.h>
#include <time.h>
#include <utime.h>


// heapsize: minimum 8 mb, standart 16 mb, max is 32 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x0800000
#define STD_MEM_ALLOC	0x1000000
#define MAX_MEM_ALLOC	0x2000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};

qboolean		isDedicated = true;	/* compatibility */
static double		starttime;
static qboolean		first = true;


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path, qboolean crash)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
	{
		struct stat st;
		if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
			rc = 0;
	}
	if (rc != 0 && crash)
	{
		rc = errno;
		Sys_Error("Unable to create directory %s: %s", path, strerror(rc));
	}
	return rc;
}

int Sys_rmdir (const char *path)
{
	return rmdir(path);
}

int Sys_unlink (const char *path)
{
	return unlink(path);
}

int Sys_rename (const char *oldp, const char *newp)
{
	return rename(oldp, newp);
}

long Sys_filesize (const char *path)
{
	struct stat	st;

	if (stat(path, &st) != 0)
		return -1;
	if (! S_ISREG(st.st_mode))
		return -1;

	return (long) st.st_size;
}

int Sys_FileType (const char *path)
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

#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int Sys_CopyFile (const char *frompath, const char *topath)
{
	char	buf[COPY_READ_BUFSIZE];
	FILE	*in, *out;
	struct stat	st;
	struct utimbuf	tm;
/*	off_t	remaining, count;*/
	size_t	remaining, count;

	if (stat(frompath, &st) != 0)
	{
		Con_Printf ("%s: unable to stat %s\n", __thisfunc__, frompath);
		return 1;
	}
	in = fopen (frompath, "rb");
	if (!in)
	{
		Con_Printf ("%s: unable to open %s\n", __thisfunc__, frompath);
		return 1;
	}
	out = fopen (topath, "wb");
	if (!out)
	{
		Con_Printf ("%s: unable to create %s\n", __thisfunc__, topath);
		fclose (in);
		return 1;
	}

	remaining = st.st_size;
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

	if (remaining == 0) {
	/* restore the file's timestamp */
		tm.actime = time (NULL);
		tm.modtime = st.st_mtime;
		utime (topath, &tm);
		return 0;
	}

	return 1;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only, not a dirent struct. this is
what we presently need in this engine.
=================================================
*/
static DIR		*finddir;
static struct dirent	*finddata;
static char		*findpath, *findpattern;

const char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	if (finddir)
		Sys_Error ("Sys_FindFirst without FindClose");

	finddir = opendir (path);
	if (!finddir)
		return NULL;

	findpattern = Z_Strdup (pattern);
	findpath = Z_Strdup (path);

	return Sys_FindNextFile();
}

const char *Sys_FindNextFile (void)
{
	struct stat	test;

	if (!finddir)
		return NULL;

	while ((finddata = readdir(finddir)) != NULL)
	{
		if (!fnmatch (findpattern, finddata->d_name, FNM_PATHNAME))
		{
			if ( (stat(va("%s/%s", findpath, finddata->d_name), &test) == 0)
						&& S_ISREG(test.st_mode))
				return finddata->d_name;
		}
	}

	return NULL;
}

void Sys_FindClose (void)
{
	if (finddir != NULL)
	{
		closedir(finddir);
		finddir = NULL;
	}
	if (findpath != NULL)
	{
		Z_Free (findpath);
		findpath = NULL;
	}
	if (findpattern != NULL)
	{
		Z_Free (findpattern);
		findpattern = NULL;
	}
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];
	const char	text2[] = ERROR_PREFIX;
	const unsigned char	*p;

	va_start (argptr, error);
	q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (con_debuglog)
	{
		LOG_Print (ERROR_PREFIX);
		LOG_Print (text);
		LOG_Print ("\n\n");
	}

	Host_Shutdown ();

	for (p = (const unsigned char *) text2; *p; p++)
		putc (*p, stderr);
	for (p = (const unsigned char *) text ; *p; p++)
		putc (*p, stderr);
	putc ('\n', stderr);
	putc ('\n', stderr);

	exit (1);
}

void Sys_PrintTerm (const char *msgtxt)
{
	const unsigned char	*p;

	if (sys_nostdout.integer)
		return;

	for (p = (const unsigned char *) msgtxt; *p; p++)
		putc (*p, stdout);
}

void Sys_Quit (void)
{
	Host_Shutdown();

	exit (0);
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	struct timeval	tp;
	double		now;

	gettimeofday (&tp, NULL);

	now = tp.tv_sec + tp.tv_usec / 1e6;

	if (first)
	{
		first = false;
		starttime = now;
		return 0.0;
	}

	return now - starttime;
}

char *Sys_DateTimeString (char *buf)
{
	static char strbuf[24];
	time_t t;
	struct tm *l;
	int val;

	if (!buf) buf = strbuf;

	t = time(NULL);
	l = localtime(&t);

	val = l->tm_mon + 1;	/* tm_mon: months since January [0,11] */
	buf[0] = val / 10 + '0';
	buf[1] = val % 10 + '0';
	buf[2] = '/';
	val = l->tm_mday;
	buf[3] = val / 10 + '0';
	buf[4] = val % 10 + '0';
	buf[5] = '/';
	val = l->tm_year / 100 + 19;	/* tm_year: #years since 1900. */
	buf[6] = val / 10 + '0';
	buf[7] = val % 10 + '0';
	val = l->tm_year % 100;
	buf[8] = val / 10 + '0';
	buf[9] = val % 10 + '0';

	buf[10] = ' ';

	val = l->tm_hour;
	buf[11] = val / 10 + '0';
	buf[12] = val % 10 + '0';
	buf[13] = ':';
	val = l->tm_min;
	buf[14] = val / 10 + '0';
	buf[15] = val % 10 + '0';
	buf[16] = ':';
	val = l->tm_sec;
	buf[17] = val / 10 + '0';
	buf[18] = val % 10 + '0';

	buf[19] = '\0';

	return buf;
}


/*
================
Sys_ConsoleInput
================
*/
const char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int	textlen;
	char		c;
	fd_set		set;
	struct timeval	timeout;

	FD_ZERO (&set);
	FD_SET (0, &set);	// stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	while (select (1, &set, NULL, NULL, &timeout))
	{
		read (0, &c, 1);
		if (c == '\n' || c == '\r')
		{
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		else if (c == 8)
		{
			if (textlen)
			{
				textlen--;
				con_text[textlen] = '\0';
			}
			continue;
		}
		con_text[textlen] = c;
		textlen++;
		if (textlen < (int) sizeof(con_text))
			con_text[textlen] = '\0';
		else
		{
		// buffer is full
			textlen = 0;
			con_text[0] = '\0';
			Sys_PrintTerm("\nConsole input too long!\n");
			break;
		}
	}

	return NULL;
}

void Sys_Sleep (unsigned long msecs)
{
	usleep (msecs * 1000);
}

static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
	char	*tmp;

	if (getcwd(dst, dstsize - 1) == NULL)
		return -1;

	tmp = dst;
	while (*tmp != 0)
		tmp++;
	while (*tmp == 0 && tmp != dst)
	{
		--tmp;
		if (tmp != dst && *tmp == '/')
			*tmp = 0;
	}

	return 0;
}

#if DO_USERDIRS
static int Sys_GetUserdir (char *dst, size_t dstsize)
{
	size_t		n;
	const char	*home_dir = NULL;
	struct passwd	*pwent;

	pwent = getpwuid(getuid());
	if (pwent == NULL)
		perror("getpwuid");
	else	home_dir = pwent->pw_dir;
	if (home_dir == NULL)
		home_dir = getenv("HOME");
	if (home_dir == NULL)
		return 1;

/* what would be a maximum path for a file in the user's directory...
 * $HOME/AOT_USERDIR/game_dir/dirname1/dirname2/dirname3/filename.ext
 * still fits in the MAX_OSPATH == 256 definition, but just in case.
 */
	n = strlen(home_dir) + strlen(AOT_USERDIR) + 50;
	if (n >= dstsize)
	{
		Sys_Error ("%s: Insufficient bufsize %d. Need at least %d.",
					__thisfunc__, (int)dstsize, (int)n);
	}

	q_snprintf (dst, dstsize, "%s/%s", home_dir, AOT_USERDIR);
	return 0;
}
#endif	/* DO_USERDIRS */

static void PrintVersion (void)
{
	Sys_Printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
	Sys_Printf ("Hexen II dedicated server %4.2f (%s)\n", ENGINE_VERSION, PLATFORM_STRING);
	Sys_Printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

static const char *help_strings[] = {
	"     [-v | --version]        Display version information",
#ifndef DEMOBUILD
#   if defined(H2MP)
	"     [-noportals]            Disable the mission pack support",
#   else
	"     [-portals | -h2mp ]     Run the Portal of Praevus mission pack",
#   endif
#endif
	"     [-heapsize Bytes]       Heapsize (memory to allocate)",
	NULL
};

static void PrintHelp (const char *name)
{
	int i = 0;

	Sys_Printf ("Usage: %s [options]\n", name);
	while (help_strings[i])
	{
		Sys_PrintTerm (help_strings[i]);
		Sys_PrintTerm ("\n");
		i++;
	}
	Sys_PrintTerm ("\n");
}

/*
===============================================================================

MAIN

===============================================================================
*/
static quakeparms_t	parms;
static char	cwd[MAX_OSPATH];
#if DO_USERDIRS
static char	userdir[MAX_OSPATH];
#endif

int main (int argc, char **argv)
{
	int			i;
	double		time, oldtime;

	PrintVersion();

	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if ( !(strcmp(argv[i], "-v")) || !(strcmp(argv[i], "-version" )) ||
				!(strcmp(argv[i], "--version")) )
			{
				exit(0);
			}
			else if ( !(strcmp(argv[i], "-h")) || !(strcmp(argv[i], "-help" )) ||
				  !(strcmp(argv[i], "-?")) || !(strcmp(argv[i], "--help")) )
			{
				PrintHelp(argv[0]);
				exit (0);
			}
		}
	}

	memset (cwd, 0, sizeof(cwd));
	if (Sys_GetBasedir(argv[0], cwd, sizeof(cwd)) != 0)
		Sys_Error ("Couldn't determine current directory");

#if DO_USERDIRS
	memset (userdir, 0, sizeof(userdir));
	if (Sys_GetUserdir(userdir, sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");
	Sys_mkdir(userdir, true);
#endif

	/* initialize the host params */
	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
#if DO_USERDIRS
	parms.userdir = userdir;
#else
	parms.userdir = cwd;
#endif
	parms.argc = argc;
	parms.argv = argv;
	host_parms = &parms;

	LOG_Init (&parms);

	Sys_Printf("basedir is: %s\n", parms.basedir);
	Sys_Printf("userdir is: %s\n", parms.userdir);

	COM_ValidateByteorder ();

	parms.memsize = STD_MEM_ALLOC;

	i = COM_CheckParm ("-heapsize");
	if (i && i < com_argc-1)
	{
		parms.memsize = atoi (com_argv[i+1]) * 1024;

		if ((parms.memsize > MAX_MEM_ALLOC) && !(COM_CheckParm ("-forcemem")))
		{
			Sys_Printf ("Requested memory (%d Mb) too large, using the default maximum.\n", parms.memsize/(1024*1024));
			Sys_Printf ("If you are sure, use the -forcemem switch.\n");
			parms.memsize = MAX_MEM_ALLOC;
		}
		else if ((parms.memsize < MIN_MEM_ALLOC) && !(COM_CheckParm ("-forcemem")))
		{
			Sys_Printf ("Requested memory (%d Mb) too little, using the default minimum.\n", parms.memsize/(1024*1024));
			Sys_Printf ("If you are sure, use the -forcemem switch.\n");
			parms.memsize = MIN_MEM_ALLOC;
		}
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error ("Insufficient memory.\n");

	Host_Init();

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
		time = Sys_DoubleTime ();

		if (time - oldtime < sys_ticrate.value )
		{
			usleep (1000);
			continue;
		}

		Host_Frame (time - oldtime);
		oldtime = time;
	}

	return 0;
}

