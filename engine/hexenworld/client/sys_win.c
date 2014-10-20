/*
 * sys_win.c -- Windows system interface code
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

#include "quakedef.h"
#include "debuglog.h"
#include "winquake.h"
#include <mmsystem.h>
#include "resource.h"


// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

#define PAUSE_SLEEP		50	/* sleep time on pause or minimization	*/
#define NOT_FOCUS_SLEEP		20	/* sleep time when not focus		*/

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};
cvar_t		sys_throttle = {"sys_throttle", "0.02", CVAR_ARCHIVE};

qboolean	ActiveApp, Minimized;
qboolean	Win95, Win95old, WinNT, WinVista;

#define	TIME_WRAP_VALUE	(~(DWORD)0)
static DWORD		starttime;

static HANDLE	qwclsemaphore;


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path, qboolean crash)
{
	if (CreateDirectory(path, NULL) != 0)
		return 0;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;
	if (crash)
		Sys_Error("Unable to create directory %s", path);
	return -1;
}

int Sys_rmdir (const char *path)
{
	if (RemoveDirectory(path) != 0)
		return 0;
	return -1;
}

int Sys_unlink (const char *path)
{
	if (DeleteFile(path) != 0)
		return 0;
	return -1;
}

int Sys_rename (const char *oldp, const char *newp)
{
	if (MoveFile(oldp, newp) != 0)
		return 0;
	return -1;
}

long Sys_filesize (const char *path)
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
int Sys_FileType (const char *path)
{
	DWORD result = GetFileAttributes(path);

	if (result == INVALID_FILE_ATTRIBUTES)
		return FS_ENT_NONE;
	if (result & FILE_ATTRIBUTE_DIRECTORY)
		return FS_ENT_DIRECTORY;

	return FS_ENT_FILE;
}

int Sys_CopyFile (const char *frompath, const char *topath)
{
/* 3rd param: whether to fail if 'topath' already exists */
	if (CopyFile(frompath, topath, FALSE) != 0)
		return 0;
	return -1;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only, not a dirent struct. this is
what we presently need in this engine.
=================================================
*/
static HANDLE findhandle = INVALID_HANDLE_VALUE;
static WIN32_FIND_DATA finddata;
static char	findstr[MAX_OSPATH];

const char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	if (findhandle != INVALID_HANDLE_VALUE)
		Sys_Error ("Sys_FindFirst without FindClose");
	q_snprintf (findstr, sizeof(findstr), "%s/%s", path, pattern);
	findhandle = FindFirstFile(findstr, &finddata);
	if (findhandle == INVALID_HANDLE_VALUE)
		return NULL;
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return Sys_FindNextFile();
	return finddata.cFileName;
}

const char *Sys_FindNextFile (void)
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

void Sys_FindClose (void)
{
	if (findhandle != INVALID_HANDLE_VALUE)
	{
		FindClose(findhandle);
		findhandle = INVALID_HANDLE_VALUE;
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
	DWORD	flOldProtect;
//@@@ copy on write or just read-write?
	if (!VirtualProtect((LPVOID)startaddr, length, PAGE_EXECUTE_READWRITE, &flOldProtect))
		Sys_Error("Protection change failed\n");
}
#endif	/* id386, !GLQUAKE */


/*
================
Sys_SetDPIAware
================
*/
typedef enum { dpi_unaware = 0, dpi_system_aware = 1, dpi_monitor_aware = 2 } dpi_awareness;
typedef BOOL(*SetProcessDPIAwareFunc)();
typedef HRESULT(*SetProcessDPIAwarenessFunc)(dpi_awareness value);

static void Sys_SetDPIAware (void)
{
	HMODULE hUser32, hShcore;
	SetProcessDPIAwarenessFunc setDPIAwareness;
	SetProcessDPIAwareFunc setDPIAware;

	/* We do not handle the OS scaling our window. Call
	 * SetProcessDpiAwareness() or SetProcessDPIAware()
	 * to opt out of scaling.
	 */
	hShcore = LoadLibraryA ("Shcore.dll");
	hUser32 = LoadLibraryA ("user32.dll");
	setDPIAwareness = (SetProcessDPIAwarenessFunc) (hShcore ? GetProcAddress (hShcore, "SetProcessDpiAwareness") : NULL);
	setDPIAware = (SetProcessDPIAwareFunc) (hUser32 ? GetProcAddress (hUser32, "SetProcessDPIAware") : NULL);

	if (setDPIAwareness) /* Windows 8.1+ */
		setDPIAwareness (dpi_monitor_aware);
	else if (setDPIAware) /* Vista, Win7 or 8.0 */
		setDPIAware ();

	if (hShcore)
		FreeLibrary (hShcore);
	if (hUser32)
		FreeLibrary (hUser32);
}

/*
================
Sys_Init
================
*/
static void Sys_Init (void)
{
	OSVERSIONINFO	vinfo;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx (&vinfo))
		Sys_Error ("Couldn't get OS info");

	if ((vinfo.dwMajorVersion < 4) ||
		(vinfo.dwPlatformId == VER_PLATFORM_WIN32s))
	{
		Sys_Error ("%s requires at least Win95 or NT 4.0", ENGINE_NAME);
	}

	if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		WinNT = true;
		if (vinfo.dwMajorVersion >= 6)
			WinVista = true;
	}
	else
	{
		WinNT = false; /* Win9x or WinME */
		if ((vinfo.dwMajorVersion == 4) && (vinfo.dwMinorVersion == 0))
		{
			Win95 = true;
			/* Win95-gold or Win95A can't switch bpp automatically */
			if (vinfo.szCSDVersion[1] != 'C' && vinfo.szCSDVersion[1] != 'B')
				Win95old = true;
		}
	}

	Sys_SetDPIAware ();

	timeBeginPeriod (1);	/* 1 ms timer precision */
	starttime = timeGetTime ();

#if !defined(SERVERONLY)
/* allocate a named semaphore on the client so
   the front end can tell if it is alive
   mutex will fail if semephore already exists */
	qwclsemaphore = CreateMutex(
					NULL,	/* Security attributes	*/
					0,	/* owner		*/
					"hwcl");/* Semaphore name	*/
	if (!qwclsemaphore)
		Sys_Error ("HWCL is already running on this system");

	CloseHandle (qwclsemaphore);
	qwclsemaphore = CreateSemaphore(
					NULL,	/* Security attributes	*/
					0,	/* Initial count	*/
					1,	/* Maximum count	*/
					"hwcl");/* Semaphore name	*/
#endif	/* ! SERVERONLY */

/* do we really need these with opengl ?? */
	MaskExceptions ();
	Sys_SetFPCW ();
}


#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];

	va_start (argptr, error);
	q_vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

	if (con_debuglog)
	{
		LOG_Print (ERROR_PREFIX);
		LOG_Print (text);
		LOG_Print ("\n\n");
	}

	Host_Shutdown ();

	MessageBox(NULL, text, ENGINE_NAME " Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);

#if !defined(SERVERONLY)
	if (qwclsemaphore)
		CloseHandle (qwclsemaphore);
#endif	/* ! SERVERONLY */

	exit (1);
}

void Sys_PrintTerm (const char *msgtxt)
{
	/* no stdout for win32 graphical mode */
}

void Sys_Quit (void)
{
	Host_Shutdown();
#if !defined(SERVERONLY)
	if (qwclsemaphore)
		CloseHandle (qwclsemaphore);
#endif	/* ! SERVERONLY */

	exit (0);
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	DWORD	now, passed;

	now = timeGetTime();
	if (now < starttime)	/* wrapped? */
	{
		passed = TIME_WRAP_VALUE - starttime;
		passed += now;
	}
	else
	{
		passed = now - starttime;
	}

	return (passed == 0) ? 0.0 : (passed / 1000.0);
}

char *Sys_DateTimeString (char *buf)
{
	static char strbuf[24];
	SYSTEMTIME st;
	int val;

	if (!buf) buf = strbuf;

	GetLocalTime(&st);

	val = st.wMonth;
	buf[0] = val / 10 + '0';
	buf[1] = val % 10 + '0';
	buf[2] = '/';
	val = st.wDay;
	buf[3] = val / 10 + '0';
	buf[4] = val % 10 + '0';
	buf[5] = '/';
	val = st.wYear / 100;
	buf[6] = val / 10 + '0';
	buf[7] = val % 10 + '0';
	val = st.wYear % 100;
	buf[8] = val / 10 + '0';
	buf[9] = val % 10 + '0';

	buf[10] = ' ';

	val = st.wHour;
	buf[11] = val / 10 + '0';
	buf[12] = val % 10 + '0';
	buf[13] = ':';
	val = st.wMinute;
	buf[14] = val / 10 + '0';
	buf[15] = val % 10 + '0';
	buf[16] = ':';
	val = st.wSecond;
	buf[17] = val / 10 + '0';
	buf[18] = val % 10 + '0';

	buf[19] = '\0';

	return buf;
}


void Sys_Sleep (unsigned long msecs)
{
	Sleep (msecs);
}


void Sys_SendKeyEvents (void)
{
	MSG	msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
	// we always update if there are any event, even if we're paused
		scr_skipupdate = 0;

		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

#define MAX_CLIPBOARDTXT	MAXCMDLINE	/* 256 */
char *Sys_GetClipboardData (void)
{
	char *data = NULL;
	char *cliptext;

	if (OpenClipboard(NULL) != 0)
	{
		HANDLE hClipboardData;

		if ((hClipboardData = GetClipboardData(CF_TEXT)) != NULL)
		{
			cliptext = (char *) GlobalLock(hClipboardData);
			if (cliptext != NULL)
			{
				size_t size = GlobalSize(hClipboardData) + 1;
			/* this is intended for simple small text copies
			 * such as an ip address, etc:  do chop the size
			 * here, otherwise we may experience Z_Malloc()
			 * failures and all other not-oh-so-fun stuff. */
				size = q_min(MAX_CLIPBOARDTXT, size);
				data = (char *) Z_Malloc(size, Z_MAINZONE);
				q_strlcpy (data, cliptext, size);
				GlobalUnlock (hClipboardData);
			}
		}
		CloseClipboard ();
	}
	return data;
}


static int Sys_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
	char		*tmp;

	if (GetCurrentDirectory(dstsize, dst) == 0)
		return -1;

	tmp = dst;
	while (*tmp != 0)
		tmp++;
	while (*tmp == 0 && tmp != dst)
	{
		--tmp;
		if (tmp != dst && (*tmp == '/' || *tmp == '\\'))
			*tmp = 0;
	}

	return 0;
}

/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/


/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;
int			global_nCmdShow;
#if !defined(NO_SPLASHES)
HWND		hwnd_dialog;
#endif	/* NO_SPLASHES */
static char	*argv[MAX_NUM_ARGVS];
static char	cwd[1024];
static char	prog[MAX_PATH];
static quakeparms_t	parms;

static void Sys_CreateInitSplash (HINSTANCE hInstance)
{
#if !defined(NO_SPLASHES)
	RECT		rect;

	hwnd_dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);
	if (!hwnd_dialog)
		return;

	if (GetWindowRect (hwnd_dialog, &rect))
	{
		if (rect.left > (rect.top * 2))
		{
			SetWindowPos (hwnd_dialog, 0,
					(rect.left / 2) - ((rect.right - rect.left) / 2),
					rect.top, 0, 0,
					SWP_NOZORDER | SWP_NOSIZE);
		}
	}

	ShowWindow (hwnd_dialog, SW_SHOWDEFAULT);
	UpdateWindow (hwnd_dialog);
	SetForegroundWindow (hwnd_dialog);
#endif	/* NO_SPLASHES */
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int		i;
	double		time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;

	/* previous instances do not exist in Win32 */
	if (hPrevInstance)
		return 0;

	global_hInstance = hInstance;
	global_nCmdShow = nCmdShow;

	lpBuffer.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus (&lpBuffer);
	/* Maximum of 2GiB to work around signed int */
	if (lpBuffer.dwAvailPhys > 0x7FFFFFFF)
		lpBuffer.dwAvailPhys = 0x7FFFFFFF;
	if (lpBuffer.dwTotalPhys > 0x7FFFFFFF)
		lpBuffer.dwTotalPhys = 0x7FFFFFFF;

	memset (cwd, 0, sizeof(cwd));
	if (Sys_GetBasedir(NULL, cwd, sizeof(cwd)) != 0)
		Sys_Error ("Couldn't determine current directory");

	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
	parms.userdir = cwd;	/* no userdir on win32 */

	parms.argc = 1;
	argv[0] = prog;
	if (GetModuleFileName(NULL, prog, sizeof(prog)) == 0)
		prog[0] = '\0';
	else	prog[MAX_PATH - 1] = '\0';

	while (*lpCmdLine && (parms.argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[parms.argc] = lpCmdLine;
			parms.argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
		}
	}

	parms.argv = argv;
	host_parms = &parms;	/* initialize the host params */

	LOG_Init (&parms);

	Sys_Printf("basedir is: %s\n", parms.basedir);
	Sys_Printf("userdir is: %s\n", parms.userdir);

	COM_ValidateByteorder ();

	Sys_CreateInitSplash (global_hInstance);

// take the greater of all the available memory or half the total memory,
// but at least 16 Mb and no more than 32 Mb, unless they explicitly
// request otherwise
	parms.memsize = lpBuffer.dwAvailPhys;

	if (parms.memsize < MIN_MEM_ALLOC)
		parms.memsize = MIN_MEM_ALLOC;

	if (parms.memsize < (int) (lpBuffer.dwTotalPhys >> 1))
		parms.memsize = (int) (lpBuffer.dwTotalPhys >> 1);

	if (parms.memsize > STD_MEM_ALLOC)
		parms.memsize = STD_MEM_ALLOC;

	i = COM_CheckParm ("-heapsize");
	if (i && i < com_argc-1)
	{
		parms.memsize = atoi (com_argv[i+1]) * 1024;

		if ((parms.memsize > MAX_MEM_ALLOC) && !(COM_CheckParm ("-forcemem")))
			parms.memsize = MAX_MEM_ALLOC;
		else if ((parms.memsize < MIN_MEM_ALLOC) && !(COM_CheckParm ("-forcemem")))
			parms.memsize = MIN_MEM_ALLOC;
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error ("Insufficient memory.\n");

	Sys_Init ();

	Host_Init();

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
		/* yield the CPU for a little while when paused, minimized or not focused */
		if ((cl.paused && (!ActiveApp && !DDActive)) || Minimized || block_drawing)
		{
			Sleep (PAUSE_SLEEP);
			scr_skipupdate = 1;		/* no point in bothering to draw */
		}
		else if (!ActiveApp && !DDActive)
		{
			Sleep (NOT_FOCUS_SLEEP);
		}

		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);

		if (time < sys_throttle.value)
			Sleep (1);

		oldtime = newtime;
	}

	return 0;
}

