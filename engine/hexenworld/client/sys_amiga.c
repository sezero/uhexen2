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
#include "debuglog.h"

#include <proto/exec.h>
#include <proto/dos.h>

#include <proto/timer.h>
#include <time.h>

#include <proto/intuition.h>
#include <proto/iffparse.h>
#include <datatypes/textclass.h>

#if defined(SDLQUAKE)
#include "sdl_inc.h"
#endif

#define MIN_STACK_SIZE 0x100000 /* 1 MB stack */
#ifdef __CLIB2__
int __stack_size = MIN_STACK_SIZE;
#else
int __stack = MIN_STACK_SIZE;
#endif
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

#include <SDI/SDI_compiler.h> /* IPTR */

// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};
cvar_t		sys_throttle = {"sys_throttle", "0.02", CVAR_ARCHIVE};

static double		starttime;
static qboolean		first = true;

struct timerequest	*timerio;
struct MsgPort		*timerport;
#if defined(__MORPHOS__) || defined(__VBCC__)
struct Library		*TimerBase;
#else
struct Device		*TimerBase;
#endif
#ifdef __CLIB2__
struct IntuitionBase *IntuitionBase;
struct Library *IFFParseBase;
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

/* File existence check with the "Please insert volume XXX"
 * system requester disabled.  */
qboolean Sys_PathExistsQuiet(const char *name)
{
	struct Process *self = (struct Process *) FindTask(NULL);
	APTR oldwinptr; BPTR lock;

	oldwinptr = self->pr_WindowPtr;
	self->pr_WindowPtr = (APTR) -1;
	lock = Lock((const STRPTR) name, ACCESS_READ);
	self->pr_WindowPtr = oldwinptr;
	if (lock) {
		UnLock(lock);
		return true;
	}
	return false;
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

/*
================
Sys_MakeCodeWriteable
================
*/
#if id386 && !defined(GLQUAKE)
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
/* Not needed on Amiga. */
}
#endif	/* id386, !GLQUAKE */


/*
================
Sys_Init
================
*/
static void Sys_Init (void)
{
	/*MaskExceptions ();*/
	Sys_SetFPCW ();

#ifdef __CLIB2__
	IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 0);
	if (!IntuitionBase)
		Sys_Error ("Cannot open intuition.library!");

	IFFParseBase = OpenLibrary("iffparse.library", 0);
#endif

	if ((timerport = CreateMsgPort()))
	{
		if ((timerio = (struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest))))
		{
			if (OpenDevice((STRPTR) TIMERNAME, UNIT_MICROHZ,
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

#if defined(SDLQUAKE)
	if (SDL_Init(0) < 0)
		Sys_Error("SDL failed to initialize.");
#endif
}

static void Sys_AtExit (void)
{
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
#ifdef __CLIB2__
	if (IntuitionBase)
	{
		CloseLibrary((struct Library *)IntuitionBase);
		IntuitionBase = NULL;
	}
	if (IFFParseBase)
	{
		CloseLibrary(IFFParseBase);
		IFFParseBase = NULL;
	}
#endif
#if defined(SDLQUAKE)
	SDL_Quit();
#endif
}

void Sys_ErrorMessage(const char *string)
{
	struct EasyStruct es;

	if (!IntuitionBase) return;

	es.es_StructSize = sizeof(es);
	es.es_Flags = 0;
	es.es_Title = (STRPTR) ENGINE_NAME " error";
	es.es_TextFormat = (STRPTR) string;
	es.es_GadgetFormat = (STRPTR) "Quit";

	EasyRequest(0, &es, 0, 0);
}

#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];
	const char	text2[] = ERROR_PREFIX;
	const unsigned char	*p;

	host_parms->errstate++;

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
	Sys_ErrorMessage (text);

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
	Host_Shutdown();

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


void Sys_Sleep (unsigned long msecs)
{
	timerio->tr_node.io_Command = TR_ADDREQUEST;
	timerio->tr_time.tv_secs = msecs / 1000;
	timerio->tr_time.tv_micro = (msecs * 1000) % 1000000;
	SendIO((struct IORequest *) timerio);
	WaitIO((struct IORequest *) timerio);
}

void Sys_SendKeyEvents (void)
{
	IN_SendKeyEvents();
}

#define MAX_CLIPBOARDTXT	MAXCMDLINE	/* 256 */
char *Sys_GetClipboardData (void)
{
	struct IFFHandle *IFFHandle;
	struct ContextNode *cn;
	LONG readbytes;
	char *chunk_buffer = NULL;

	if (!IFFParseBase)
		return NULL;

	if ((IFFHandle = AllocIFF())) {
	    if ((IFFHandle->iff_Stream = (IPTR) OpenClipboard(0))) {
		InitIFFasClip(IFFHandle);
		if (!OpenIFF(IFFHandle, IFFF_READ)) {
		    if (!StopChunk(IFFHandle, ID_FTXT, ID_CHRS)) {
			if (!ParseIFF(IFFHandle, IFFPARSE_SCAN)) {
			    cn = CurrentChunk(IFFHandle);
			    if (cn && (cn->cn_Type == ID_FTXT) &&
					(cn->cn_ID == ID_CHRS)) {
				chunk_buffer = (char *)
					  Z_Malloc(MAX_CLIPBOARDTXT, Z_MAINZONE);
				readbytes = ReadChunkBytes(IFFHandle,
							   chunk_buffer,
							   MAX_CLIPBOARDTXT - 1);
				if (readbytes < 0)
				    readbytes = 0;
				chunk_buffer[readbytes] = '\0';
			    }
			}
		    }
		    CloseIFF(IFFHandle);
		}
		CloseClipboard((struct ClipboardHandle *) IFFHandle->iff_Stream);
	    }
	    FreeIFF(IFFHandle);
	}

	return chunk_buffer;
}

static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
#if 1
	int len = q_strlcpy(dst, "PROGDIR:", dstsize);
	if (len < (int)dstsize)
		return 0;
	return -1;
#else
	if (NameFromLock(GetProgramDir(), (STRPTR) dst, dstsize) != 0)
		return 0;
	return -1;
#endif
}

static void Sys_CheckSDL (void)
{
#if defined(SDLQUAKE)
	const SDL_version *sdl_version;

	sdl_version = SDL_Linked_Version();
	Sys_Printf("Found SDL version %i.%i.%i\n",sdl_version->major,sdl_version->minor,sdl_version->patch);
	if (SDL_VERSIONNUM(sdl_version->major,sdl_version->minor,sdl_version->patch) < SDL_REQUIREDVERSION)
	{	/*reject running under SDL versions older than what is stated in sdl_inc.h */
		Sys_Error("You need at least v%d.%d.%d of SDL to run this game.", SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z);
	}
# if defined(SDL_NEW_VERSION_REJECT)
	if (SDL_VERSIONNUM(sdl_version->major,sdl_version->minor,sdl_version->patch) >= SDL_NEW_VERSION_REJECT)
	{	/*reject running under SDL versions newer than what is stated in sdl_inc.h */
		Sys_Error("Your version of SDL library is incompatible with me.\n"
			  "You need a library version in the line of %d.%d.%d\n", SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z);
	}
# endif /* SDL_NEW_VERSION_REJECT */
#endif	/* SDLQUAKE */
}

static void PrintVersion (void)
{
	Sys_Printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
	Sys_Printf ("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, PLATFORM_STRING);
	Sys_Printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

#include "snd_sys.h"
static const char *help_strings[] = {
	"     [-v | --version]        Display version information",
	"     [-noportals]            Disable the mission pack support",
	"     [-w | -window ]         Run the game windowed",
	"     [-f | -fullscreen]      Run the game fullscreen",
	"     [-width X [-height Y]]  Select screen size",
#ifdef GLQUAKE
	"     [-bpp]                  Depth for GL fullscreen mode",
	"     [-g | -gllibrary]       Select 3D rendering library",
	"     [-fsaa N]               Enable N sample antialiasing",
	"     [-paltex]               Enable 8-bit textures",
	"     [-nomtex]               Disable multitexture detection/usage",
#endif
#if !defined(_NO_SOUND)
#if SOUND_NUMDRIVERS
	"     [-s | -nosound]         Run the game without sound",
#endif
#if (SOUND_NUMDRIVERS > 1)
#if HAVE_SDL_SOUND
	"     [-sndsdl]               Use SDL sound",
#endif
#if HAVE_AHI_SOUND
	"     [-sndahi]               Use AHI audio system",
#endif
#endif	/*  SOUND_NUMDRIVERS */
#endif	/* _NO_SOUND */
	"     [-nomouse]              Disable mouse usage",
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
#if defined(SDLQUAKE)
static Uint8		appState;
#endif

int main (int argc, char **argv)
{
	int			i;
	double		time, oldtime, newtime;
	ULONG		availMem;

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
				PrintHelp(argv[0]);
				return 0;
			}
		}
	}

	/* initialize the host params */
	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
	parms.userdir = cwd;
	parms.argc = argc;
	parms.argv = argv;
	parms.errstate = 0;
	host_parms = &parms;

	memset (cwd, 0, sizeof(cwd));
	if (Sys_GetBasedir(argv[0], cwd, sizeof(cwd)) != 0)
		Sys_Error ("Couldn't determine current directory");

	LOG_Init (&parms);

	Sys_Printf("basedir is: %s\n", parms.basedir);
	Sys_Printf("userdir is: %s\n", parms.userdir);

	COM_ValidateByteorder ();

	Sys_CheckSDL ();

	availMem = AvailMem(MEMF_ANY|MEMF_LARGEST);
	if (availMem < STD_MEM_ALLOC)
		parms.memsize = MIN_MEM_ALLOC;
	else
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

	Host_Init();

	/* running from Workbench */
	if (argc == 0)
		Cvar_SetQuick(&sys_nostdout, "1");

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
#if defined(SDLQUAKE)
		appState = SDL_GetAppState();
		/* If we have no input focus at all, sleep a bit */
		if ( !(appState & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) || cl.paused)
		{
			Sys_Sleep(16);
		}
		/* If we're minimised, sleep a bit more */
		if ( !(appState & SDL_APPACTIVE))
		{
			scr_skipupdate = 1;
			Sys_Sleep(32);
		}
		else
		{
			scr_skipupdate = 0;
		}
#endif	/* SDLQUAKE */
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);

		if (time < sys_throttle.value)
			Sys_Sleep(1);

		oldtime = newtime;
	}

	return 0;
}

