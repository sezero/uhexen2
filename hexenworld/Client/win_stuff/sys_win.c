/*
	sys_win.c
	Win32 system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/win_stuff/sys_win.c,v 1.45 2007-07-10 13:54:01 sezero Exp $
*/

#include "quakedef.h"
#include "debuglog.h"
#include "winquake.h"
#include <limits.h>
#include <errno.h>
#include "resource.h"
#include <io.h>
#include <direct.h>
#include <fcntl.h>


// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

#define PAUSE_SLEEP		50	/* sleep time on pause or minimization	*/
#define NOT_FOCUS_SLEEP		20	/* sleep time when not focus		*/

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};

qboolean	ActiveApp, Minimized;
qboolean	Win95, Win95old, WinNT;

/*
#define	TIME_WRAP_VALUE	(~(DWORD)0)
*/
#define	TIME_WRAP_VALUE	LONG_MAX
static DWORD		starttime;

static HANDLE	qwclsemaphore;


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path)
{
	int rc;

	rc = _mkdir (path);
	if (rc != 0 && errno == EEXIST)
		rc = 0;

	return rc;
}

/*
=================================================
simplified findfirst/findnext implementation:
Sys_FindFirstFile and Sys_FindNextFile return
filenames only, not a dirent struct. this is
what we presently need in this engine.
=================================================
*/
static HANDLE  findhandle;
static WIN32_FIND_DATA finddata;

char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	if (findhandle)
		Sys_Error ("Sys_FindFirst without FindClose");

	findhandle = FindFirstFile(va("%s/%s", path, pattern), &finddata);

	if (findhandle != INVALID_HANDLE_VALUE)
	{
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return Sys_FindNextFile();
		else
			return finddata.cFileName;
	}

	return NULL;
}

char *Sys_FindNextFile (void)
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

void Sys_FindClose (void)
{
	if (findhandle != INVALID_HANDLE_VALUE)
		FindClose(findhandle);
	findhandle = NULL;
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
	if (!VirtualProtect((LPVOID)startaddr, length, PAGE_READWRITE, &flOldProtect))
		Sys_Error("Protection change failed\n");
}
#endif	/* id386, !GLQUAKE */


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
		WinNT = true;
	else
		WinNT = false;

	if ((vinfo.dwMajorVersion == 4) && (vinfo.dwMinorVersion == 0))
	{
		Win95 = true;
		/* Win95-gold or Win95A can't switch bpp automatically */
		if (vinfo.szCSDVersion[1] != 'C' && vinfo.szCSDVersion[1] != 'B')
			Win95old = true;
	}

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
	vsnprintf (text, sizeof (text), error, argptr);
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


void Sys_Sleep (void)
{
	Sleep (1);
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
static char	*empty_string = "";
static char	cwd[1024];
static quakeparms_t	parms;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int				i;
	double			time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;
#if !defined(NO_SPLASHES)
	RECT			rect;
#endif	/* NO_SPLASHES */

	/* previous instances do not exist in Win32 */
	if (hPrevInstance)
		return 0;

	global_hInstance = hInstance;
	global_nCmdShow = nCmdShow;

	lpBuffer.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus (&lpBuffer);

	memset (cwd, 0, sizeof(cwd));
	if (!GetCurrentDirectory (sizeof(cwd), cwd))
		Sys_Error ("Couldn't determine current directory");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
	parms.userdir = cwd;	/* no userdir on win32 */

	parms.argc = 1;
	argv[0] = empty_string;

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

#if !defined(NO_SPLASHES)
	hwnd_dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);

	if (hwnd_dialog)
	{
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
	}
#endif	/* NO_SPLASHES */

// take the greater of all the available memory or half the total memory,
// but at least 16 Mb and no more than 32 Mb, unless they explicitly
// request otherwise
	parms.memsize = lpBuffer.dwAvailPhys;

	if (parms.memsize < MIN_MEM_ALLOC)
		parms.memsize = MIN_MEM_ALLOC;

	if (parms.memsize < (lpBuffer.dwTotalPhys >> 1))
		parms.memsize = lpBuffer.dwTotalPhys >> 1;

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

// because sound is off until we become active
	S_BlockSound ();

	Host_Init();

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
	// yield the CPU for a little while when paused, minimized, or not the focus
		if ((cl.paused && (!ActiveApp && !DDActive)) || Minimized || block_drawing)
		{
			Sleep (PAUSE_SLEEP);
			scr_skipupdate = 1;		// no point in bothering to draw
		}
		else if (!ActiveApp && !DDActive)
		{
			Sleep (NOT_FOCUS_SLEEP);
		}

		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);
		oldtime = newtime;
	}

	/* return success of application */
	return TRUE;
}

