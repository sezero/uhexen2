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
#if defined(SDLQUAKE)
#include "sys_sdl.h"	/* alternative implementations using SDL. */
#endif
#include "debuglog.h"

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <fcntl.h>
#include <io.h>
#ifdef __WATCOMC__
#include <direct.h>
#endif
#include <conio.h>
#if defined(SDLQUAKE)
#include "sdl_inc.h"
#endif	/* SDLQUAKE */


// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};
cvar_t		sys_throttle = {"sys_throttle", "0.02", CVAR_ARCHIVE};


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path, qboolean crash)
{
	HDIR findhnd = HDIR_CREATE;
	FILEFINDBUF3 findbuf = {0};
	ULONG count = 1;
	APIRET rc = DosCreateDir(path, NULL);
	if (rc == 0) return 0;
	if (DosFindFirst(path, &findhnd, MUST_HAVE_DIRECTORY, &findbuf,
			 sizeof(findbuf), &count, FIL_STANDARD) == NO_ERROR)
	{
		DosFindClose(findhnd);
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
	HDIR findhnd = HDIR_CREATE;
	FILEFINDBUF3 findbuf = {0};
	ULONG cnt = 1;
	APIRET rc = DosFindFirst(path, &findhnd, FILE_NORMAL, &findbuf,
				 sizeof(findbuf), &cnt, FIL_STANDARD);

	if (rc != NO_ERROR) return -1;
	DosFindClose(findhnd);
	if (findbuf.attrFile & FILE_DIRECTORY)
		return -1;
	return (long)findbuf.cbFile;
}

int Sys_FileType (const char *path)
{
	HDIR findhnd = HDIR_CREATE;
	FILEFINDBUF3 findbuf = {0};
	ULONG cnt = 1;
	APIRET rc = DosFindFirst(path, &findhnd, FILE_NORMAL, &findbuf,
				 sizeof(findbuf), &cnt, FIL_STANDARD);

	if (rc != NO_ERROR) return FS_ENT_NONE;
	DosFindClose(findhnd);
	if (findbuf.attrFile & FILE_DIRECTORY)
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

/*
================
Sys_MakeCodeWriteable
================
*/
#if id386 && !defined(GLQUAKE)
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	APIRET rc = DosSetMem((void *)startaddr, length, PAG_READ | PAG_WRITE | PAG_EXECUTE);
	if (rc != NO_ERROR)
		Sys_Error("Protection change failed (ERR: %lu)", rc);
}
#endif	/* id386, !GLQUAKE */


/*
================
Sys_Init
================
*/
static void Sys_Init (void)
{
/* do we really need these with opengl ?? */
	Sys_SetFPCW();
#if defined(SDLQUAKE)
	if (SDL_Init(0) < 0)
		Sys_Error("SDL failed to initialize.");
#endif
}

static void Sys_AtExit (void)
{
#if defined(SDLQUAKE)
	SDL_Quit();
#endif
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
#if 0
	WinMessageBox (HWND_DESKTOP, HWND_DESKTOP, text, ENGINE_NAME " Error", 0, MB_OK | MB_MOVEABLE | MB_ERROR);
#endif

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


void Sys_Sleep (unsigned long msecs)
{
	DosSleep (msecs);
}

void Sys_SendKeyEvents (void)
{
	IN_SendKeyEvents();
}

#if !defined(Sys_GetClipboardData)
#define Sys_GetClipboardData Sys_GetClipboardData /* */
char *Sys_GetClipboardData (void)
{
	return NULL;
}
#endif

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
	"     [-width X [-height Y]]  Select screen size",
#ifdef GLQUAKE
	"     [-bpp]                  Depth for GL fullscreen mode",
	"     [-vsync]                Enable sync with monitor refresh",
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

	/* initialize the host params */
	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
	parms.userdir = cwd;
	parms.argc = argc;
	parms.argv = argv;
	host_parms = &parms;

	LOG_Init (&parms);

	Sys_Printf("basedir is: %s\n", parms.basedir);

	COM_ValidateByteorder ();

	Sys_CheckSDL ();

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

	Sys_Init ();
	atexit (Sys_AtExit);

	Host_Init();

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
#if defined(SDLQUAKE)
		appState = SDL_GetAppState();
		/* If we have no input focus at all, sleep a bit */
		if ( !(appState & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) || cl.paused)
		{
			DosSleep (16);
		}
		/* If we're minimised, sleep a bit more */
		if ( !(appState & SDL_APPACTIVE))
		{
			scr_skipupdate = 1;
			DosSleep (32);
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
			DosSleep (1);

		oldtime = newtime;
	}

	return 0;
}

