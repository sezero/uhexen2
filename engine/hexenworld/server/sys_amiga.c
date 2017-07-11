/*
 * sys_amiga.c -- Amiga system interface code
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2012  Szilard Biro <col.lawrence@gmail.com>
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

#include <proto/exec.h>
#include <proto/dos.h>

#include <proto/timer.h>
#include <time.h>

int __stack = 0x100000; /* 1 MB stack */
#ifdef __AROS__
#include "incstack.h"
/* The problem here is that our real main never returns: the exit
 * point of program is either Sys_Quit() or Sys_Error(). One way
 * of making the real main return to the incstack.h main wrapper
 * is setjmp()'ing in real main and longjmp()'ing from the actual
 * exit points, avoiding exit(). */
#include <setjmp.h>
static jmp_buf exit_buf;
static int my_rc = 0;
#define HAVE_AROS_MAIN_WRAPPER
#endif

// heapsize: minimum 8 mb, standart 16 mb, max is 32 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x0800000
#define STD_MEM_ALLOC	0x1000000
#define MAX_MEM_ALLOC	0x2000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};
int		devlog;	/* log the Con_DPrintf and Sys_DPrintf content when !developer.integer */

#ifdef PLATFORM_AMIGAOS3
#define USE_ECLOCK_TIMER
#define MY_TIMERUNIT	UNIT_ECLOCK
#else
#define MY_TIMERUNIT	UNIT_MICROHZ
static double		starttime;
#endif
static qboolean		first = true;

static BPTR		amiga_stdin, amiga_stdout;
#define	MODE_RAW	1
#define	MODE_NORMAL	0

struct timerequest	*timerio;
struct MsgPort		*timerport;
#if defined(__MORPHOS__) || defined(__VBCC__)
struct Library		*TimerBase;
#else
struct Device		*TimerBase;
#endif


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir(const char *path, qboolean crash)
{
	BPTR lock = CreateDir((const STRPTR) path);

	if (lock)
	{
		UnLock(lock);
		return 0;
	}

	if (IoErr() == ERROR_OBJECT_EXISTS)
		return 0;

	if (crash)
		Sys_Error("Unable to create directory %s", path);
	return -1;
}

int Sys_rmdir (const char *path)
{
	if (DeleteFile((const STRPTR) path) != 0)
		return 0;
	return -1;
}

int Sys_unlink (const char *path)
{
	if (DeleteFile((const STRPTR) path) != 0)
		return 0;
	return -1;
}

int Sys_rename (const char *oldp, const char *newp)
{
	if (Rename((const STRPTR) oldp, (const STRPTR) newp) != 0)
		return 0;
	return -1;
}

long Sys_filesize (const char *path)
{
	long size = -1;
	BPTR fh = Open((const STRPTR) path, MODE_OLDFILE);
	if (fh)
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

int Sys_FileType (const char *path)
{
	int type = FS_ENT_NONE;
	BPTR fh = Open((const STRPTR) path, MODE_OLDFILE);
	if (fh)
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

#define	COPY_READ_BUFSIZE		8192	/* BUFSIZ */
int Sys_CopyFile (const char *frompath, const char *topath)
{
	char buf[COPY_READ_BUFSIZE];
	BPTR in, out;
	struct FileInfoBlock *fib;
	struct DateStamp stamp;
	LONG remaining, count;

	in = Open((const STRPTR) frompath, MODE_OLDFILE);
	if (!in)
	{
		Con_Printf ("%s: unable to open %s\n", __thisfunc__, frompath);
		return 1;
	}
	fib = (struct FileInfoBlock*) AllocDosObject(DOS_FIB, NULL);
	if (fib != NULL)
	{
		if (ExamineFH(in, fib) == 0)
			remaining = -1;
		else
		{
			remaining = fib->fib_Size;
			stamp = fib->fib_Date;
		}
		FreeDosObject(DOS_FIB, fib);
		if (remaining < 0)
		{
			Con_Printf ("%s: can't determine size for %s\n", __thisfunc__, frompath);
			Close(in);
			return 1;
		}
	}
	else
	{
		Con_Printf ("%s: can't allocate FileInfoBlock for %s\n", __thisfunc__, frompath);
		Close(in);
		return 1;
	}
	out = Open((const STRPTR) topath, MODE_NEWFILE);
	if (!out)
	{
		Con_Printf ("%s: unable to open %s\n", __thisfunc__, topath);
		Close(in);
		return 1;
	}

	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else	count = sizeof(buf);

		if (Read(in, buf, count) == -1)
			break;
		if (Write(out, buf, count) == -1)
			break;

		remaining -= count;
	}

	Close(in);
	Close(out);

	if (remaining != 0)
		return 1;

	SetFileDate(topath, &stamp);

	return 0;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only, not a dirent struct. this is
what we presently need in this engine.
=================================================
*/
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
		pdup = Z_Strdup(pat);
	else
	{
	/* replace each "*" by "#?" */
		n += (int) strlen(pat) + 1;
		pdup = (char *) Z_Malloc(n, Z_MAINZONE);

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

const char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	BPTR newdir;

	if (apath)
		Sys_Error ("Sys_FindFirst without FindClose");

	apath = (struct AnchorPath *) AllocVec (sizeof(struct AnchorPath) + PATH_SIZE, MEMF_CLEAR);
	if (!apath)
		return NULL;

	apath->ap_Strlen = PATH_SIZE;
	apath->ap_BreakBits = 0;
	apath->ap_Flags = 0;  /* APF_DOWILD */

	newdir = Lock((const STRPTR) path, SHARED_LOCK);
	if (newdir)
		oldcurrentdir = CurrentDir(newdir);
	else
	{
		FreeVec(apath);
		apath = NULL;
		return NULL;
	}

	pattern_str = pattern_helper (pattern);

	if (MatchFirst((const STRPTR) pattern_str, apath) == 0)
	{
	    if (apath->ap_Info.fib_DirEntryType < 0)
		return (const char *) (apath->ap_Info.fib_FileName);
	}

	return Sys_FindNextFile();
}

const char *Sys_FindNextFile (void)
{
	if (!apath)
		return NULL;

	while (MatchNext(apath) == 0)
	{
	    if (apath->ap_Info.fib_DirEntryType < 0)
		return (const char *) (apath->ap_Info.fib_FileName);
	}

	return NULL;
}

void Sys_FindClose (void)
{
	if (!apath)
		return;
	MatchEnd(apath);
	FreeVec(apath);
	UnLock(CurrentDir(oldcurrentdir));
	oldcurrentdir = 0;
	apath = NULL;
	Z_Free(pattern_str);
	pattern_str = NULL;
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

/*
================
Sys_Init
================
*/
static void Sys_Init (void)
{
	if ((timerport = CreateMsgPort()))
	{
		if ((timerio = (struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest))))
		{
			if (OpenDevice((STRPTR) TIMERNAME, MY_TIMERUNIT,
					(struct IORequest *) timerio, 0) == 0)
			{
#if defined(__MORPHOS__) || defined(__VBCC__)
				TimerBase = (struct Library *)timerio->tr_node.io_Device;
#else
				TimerBase = timerio->tr_node.io_Device;
#endif
			}
			else
			{
				DeleteIORequest((struct IORequest *)timerio);
				DeleteMsgPort(timerport);
			}
		}
		else
		{
			DeleteMsgPort(timerport);
		}
	}

	if (!TimerBase)
		Sys_Error("Can't open timer.device");

	/* 1us wait, for timer cleanup success */
	timerio->tr_node.io_Command = TR_ADDREQUEST;
	timerio->tr_time.tv_secs = 0;
	timerio->tr_time.tv_micro = 1;
	SendIO((struct IORequest *) timerio);
	WaitIO((struct IORequest *) timerio);

	amiga_stdout = Output();
	amiga_stdin = Input();
	SetMode(amiga_stdin, MODE_RAW);
}

static void Sys_AtExit (void)
{
	if (amiga_stdin)
		SetMode(amiga_stdin, MODE_NORMAL);
	if (TimerBase)
	{
		/*
		if (!CheckIO((struct IORequest *) timerio)
		{
			AbortIO((struct IORequest *) timerio);
			WaitIO((struct IORequest *) timerio);
		}
		*/
		WaitIO((struct IORequest *) timerio);
		CloseDevice((struct IORequest *) timerio);
		DeleteIORequest((struct IORequest *) timerio);
		DeleteMsgPort(timerport);
		TimerBase = NULL;
	}
}

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

#ifdef HAVE_AROS_MAIN_WRAPPER
	Sys_AtExit();
	my_rc = 1;
	longjmp(exit_buf, 1);
#else
	exit (1);
#endif
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
#ifdef HAVE_AROS_MAIN_WRAPPER
	Sys_AtExit();
	longjmp(exit_buf, 1);
#else
	exit (0);
#endif
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
#if defined(USE_ECLOCK_TIMER)
	static ULONG old_lo;
	ULONG E_Freq;
	struct EClockVal eclock;

	E_Freq = ReadEClock(&eclock);

	if (first)
	{
		first = false;
		old_lo = eclock.ev_lo;
		return 0.0;
	}
	return (double)(eclock.ev_lo - old_lo) / (double)E_Freq;
#else
	struct timeval	tp;
	double		now;

	GetSysTime(&tp);

	now = tp.tv_secs + tp.tv_micro / 1e6;

	if (first)
	{
		first = false;
		starttime = now;
		return 0.0;
	}

	return now - starttime;
#endif
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

	while (WaitForChar(amiga_stdin,10))
	{
		Read (amiga_stdin, &c, 1);
		if (c == '\n' || c == '\r')
		{
			Write(amiga_stdout, "\n", 1);
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		else if (c == 8)
		{
			if (textlen)
			{
				Write(amiga_stdout, "\b \b", 3);
				textlen--;
				con_text[textlen] = '\0';
			}
			continue;
		}
		con_text[textlen] = c;
		textlen++;
		if (textlen < (int) sizeof(con_text))
		{
			Write(amiga_stdout, &c, 1);
			con_text[textlen] = '\0';
		}
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

static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
#if 1
	int len = q_strlcpy(dst, "PROGDIR:", dstsize);
	if (len < (int)dstsize)
		return 0;
	return -1;
#else
	if (NameFromLock(((struct Process *) FindTask(NULL))->pr_CurrentDir, (STRPTR) dst, dstsize) != 0)
		return 0;
	return -1;
#endif
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

#ifdef HAVE_AROS_MAIN_WRAPPER
	if (setjmp(exit_buf))
		return my_rc;
#endif

	PrintVersion();

	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if ( !(strcmp(argv[i], "-v")) || !(strcmp(argv[i], "-version" )) ||
				!(strcmp(argv[i], "--version")) )
			{
				return 0;
			}
			else if ( !(strcmp(argv[i], "-h")) || !(strcmp(argv[i], "-help" )) ||
				  !(strcmp(argv[i], "-?")) || !(strcmp(argv[i], "--help")) )
			{
				Sys_PrintTerm ("See the documentation for details\n");
				return 0;
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

#ifndef HAVE_AROS_MAIN_WRAPPER
	atexit (Sys_AtExit);
#endif
	Sys_Init ();

	SV_Init();

// report the filesystem to the user
	Sys_Printf("gamedir is: %s\n", FS_GetGamedir());
	Sys_Printf("userdir is: %s\n", FS_GetUserdir());

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

