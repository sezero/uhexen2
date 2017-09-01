/*
 * sys_os2.c -- OS/2 system interface code
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2016  O.Sezer <sezero@users.sourceforge.net>
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

#define INCL_DOS
#define INCL_DOSERRORS
#ifdef __EMX__
#define INCL_KBD
#define INCL_VIO
#endif
#include <os2.h>

#include <fcntl.h>
#include <io.h>
#ifdef __WATCOMC__
#include <direct.h>
#endif
#include <conio.h>


// heapsize: minimum 8 mb, standart 16 mb, max is 32 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x0800000
#define STD_MEM_ALLOC	0x1000000
#define MAX_MEM_ALLOC	0x2000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};
int		devlog;	/* log the Con_DPrintf and Sys_DPrintf content when !developer.integer */


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path, qboolean crash)
{
	FILESTATUS3 fs;
	APIRET rc = DosCreateDir(path, NULL);
	if (rc == NO_ERROR) return 0;
	if ((DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs)) == NO_ERROR) &&
						  (fs.attrFile & FILE_DIRECTORY)) {
		return 0; /* dir exists */
	}
	if (crash)
		Sys_Error("Unable to create directory %s (ERR: %lu)", path, rc);
	return -1;
}

int Sys_rmdir (const char *path)
{
	APIRET rc = DosDeleteDir(path);
	return (rc == NO_ERROR)? 0 : -1;
}

int Sys_unlink (const char *path)
{
	APIRET rc = DosDelete(path);
	return (rc == NO_ERROR)? 0 : -1;
}

int Sys_rename (const char *oldp, const char *newp)
{
	APIRET rc = DosMove(oldp, newp);
	return (rc == NO_ERROR)? 0 : -1;
}

long Sys_filesize (const char *path)
{
	FILESTATUS3 fs;
	APIRET rc = DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs));
	if (rc != NO_ERROR) return -1;
	if (fs.attrFile & FILE_DIRECTORY)
		return -1;
	return (long)fs.cbFile;
}

int Sys_FileType (const char *path)
{
	FILESTATUS3 fs;
	APIRET rc = DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs));
	if (rc != NO_ERROR) return FS_ENT_NONE;
	if (fs.attrFile & FILE_DIRECTORY)
		return FS_ENT_DIRECTORY;
	return FS_ENT_FILE;
}

int Sys_CopyFile (const char *frompath, const char *topath)
{
	APIRET rc = DosCopy(frompath, topath, DCPY_EXISTING);
	return (rc == NO_ERROR)? 0 : -1;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only, not a dirent struct. this is
what we presently need in this engine.
=================================================
*/
static HDIR findhandle = HDIR_CREATE;
static FILEFINDBUF3 findbuffer;
static char	findstr[MAX_OSPATH];

const char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	ULONG cnt = 1;
	APIRET rc;
	if (findhandle != HDIR_CREATE)
		Sys_Error ("Sys_FindFirst without FindClose");
	q_snprintf (findstr, sizeof(findstr), "%s/%s", path, pattern);
	findbuffer.oNextEntryOffset = 0;
	rc = DosFindFirst(findstr, &findhandle, FILE_NORMAL, &findbuffer,
				 sizeof(findbuffer), &cnt, FIL_STANDARD);
	if (rc != NO_ERROR) {
		findhandle = HDIR_CREATE;
		findbuffer.oNextEntryOffset = 0;
		return NULL;
	}
	if (findbuffer.attrFile & FILE_DIRECTORY)
		return Sys_FindNextFile();
	return findbuffer.achName;
}

const char *Sys_FindNextFile (void)
{
	APIRET rc;
	ULONG cnt;
	if (findhandle == HDIR_CREATE)
		return NULL;
	while (1) {
		cnt = 1;
		rc = DosFindNext(findhandle, &findbuffer, sizeof(findbuffer), &cnt);
		if (rc != NO_ERROR)
			return NULL;
		if (!(findbuffer.attrFile & FILE_DIRECTORY))
			return findbuffer.achName;
	}
	return NULL;
}

void Sys_FindClose (void)
{
	if (findhandle != HDIR_CREATE) {
		DosFindClose(findhandle);
		findhandle = HDIR_CREATE;
		findbuffer.oNextEntryOffset = 0;
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

	if (sv_logfile)
	{
		fprintf (sv_logfile, ERROR_PREFIX "%s\n\n", text);
		fflush (sv_logfile);
	}

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
	exit (0);
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	union i64 { QWORD qw; long long ll; };
	static qboolean		first = true;
	static ULONG		ticks_per_sec;
	static union i64	start;
	union i64		now;

	if (first)
	{
		first = false;
		DosTmrQueryFreq(&ticks_per_sec);
		DosTmrQueryTime(&start.qw);
		return 0.0;
	}

	DosTmrQueryTime(&now.qw);
	return (double)(now.ll - start.ll) / (double)ticks_per_sec;
}

char *Sys_DateTimeString (char *buf)
{
	static char strbuf[24];
	DATETIME dt;
	unsigned int val;

	if (!buf) buf = strbuf;

	DosGetDateTime (&dt);

	val = dt.month;
	buf[0] = val / 10 + '0';
	buf[1] = val % 10 + '0';
	buf[2] = '/';
	val = dt.day;
	buf[3] = val / 10 + '0';
	buf[4] = val % 10 + '0';
	buf[5] = '/';
	val = dt.year / 100;
	buf[6] = val / 10 + '0';
	buf[7] = val % 10 + '0';
	val = dt.year % 100;
	buf[8] = val / 10 + '0';
	buf[9] = val % 10 + '0';

	buf[10] = ' ';

	val = dt.hours;
	buf[11] = val / 10 + '0';
	buf[12] = val % 10 + '0';
	buf[13] = ':';
	val = dt.minutes;
	buf[14] = val / 10 + '0';
	buf[15] = val % 10 + '0';
	buf[16] = ':';
	val = dt.seconds;
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
#ifdef __EMX__
int putch (int c) {
	char ch = c;
	VioWrtTTY(&ch, 1, 0);
	return c;
}
int kbhit (void) {
	KBDKEYINFO k;
	if (KbdPeek(&k, 0))
		return 0;
	return (k.fbStatus & KBDTRF_FINAL_CHAR_IN);
}
#endif

const char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int	textlen = 0;
	char		ch;

	if (! kbhit())
		return NULL;

	ch = getche();

	switch (ch)
	{
	case '\r':
		putch('\n');
		if (textlen)
		{
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		break;

	case '\b':
		putch(' ');
		if (textlen)
		{
			textlen--;
			putch('\b');
		}
		break;

	default:
		con_text[textlen] = ch;
		textlen = (textlen + 1) & 0xff;
		break;
	}

	return NULL;
}

static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
	ULONG l, drv;

	if (dstsize < 8) return -1;
	l = dstsize - 3;
	if (DosQueryCurrentDir(0, (PBYTE) dst + 3, &l) != NO_ERROR)
		return -1;
	DosQueryCurrentDisk(&drv, &l);
	dst[0] = drv + 'A' - 1;
	dst[1] = ':';
	dst[2] = '\\';

	return 0;
}

static void PrintVersion (void)
{
	Sys_Printf ("HexenWorld server %4.2f (%s)\n", ENGINE_VERSION, PLATFORM_STRING);
	Sys_Printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
}

/*
===============================================================================

MAIN

===============================================================================
*/
static quakeparms_t	parms;
static char	cwd[MAX_OSPATH];

int main (int argc, char **argv)
{
	int			i;
	double		newtime, time, oldtime;

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
				Sys_PrintTerm ("See the documentation for details\n");
				exit (0);
			}
		}
	}

	memset (cwd, 0, sizeof(cwd));
	if (Sys_GetBasedir(argv[0], cwd, sizeof(cwd)) != 0)
		Sys_Error ("Couldn't determine current directory");

	/* initialize the host params */
	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
	parms.userdir = cwd;
	parms.argc = argc;
	parms.argv = argv;
	host_parms = &parms;

	devlog = COM_CheckParm("-devlog");

	Sys_Printf("basedir is: %s\n", parms.basedir);

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

	SV_Init();

// report the filesystem to the user
	Sys_Printf("gamedir is: %s\n", FS_GetGamedir());

// run one frame immediately for first heartbeat
	SV_Frame (HX_FRAME_TIME);

//
// main loop
//
	oldtime = Sys_DoubleTime () - HX_FRAME_TIME;
	while (1)
	{
		if (NET_CheckReadTimeout(0, 10000) == -1)
			continue;

		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;
		oldtime = newtime;

		SV_Frame (time);
	}

	return 0;
}

