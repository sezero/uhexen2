// sys_win.c -- Win32 system interface code

#include "quakedef.h"
#include <windows.h>
#include "quakeinc.h"
#include <errno.h>
#include "resource.h"
#include <io.h>
#include <fcntl.h>


// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

#define PAUSE_SLEEP		50	// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP		20	// sleep time when not focus

qboolean	ActiveApp, Minimized;
qboolean	Win95, Win95old, WinNT;

static double		pfreq;
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static int			lowshift;

static HANDLE	qwclsemaphore;

static void Sys_InitFloatTime (void);


//=============================================================================


void Sys_DebugLog(char *file, char *fmt, ...)
{
	va_list		argptr;
	static char	data[MAXPRINTMSG];
	int			fd;

	va_start(argptr, fmt);
	vsnprintf(data, sizeof (data), fmt, argptr);
	va_end(argptr);
	fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
	write(fd, data, strlen(data));
	close(fd);
};

/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (char *path)
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

char *Sys_FindFirstFile (char *path, char *pattern)
{
	BOOL	retval;

	if (findhandle)
		Sys_Error ("Sys_FindFirst without FindClose");

	findhandle = FindFirstFile(va("%s/%s", path, pattern), &finddata);
	retval = TRUE;

	if (findhandle != INVALID_HANDLE_VALUE && retval)
		return finddata.cFileName;

	return NULL;
}

char *Sys_FindNextFile (void)
{
	BOOL	retval;

	if (!findhandle || findhandle == INVALID_HANDLE_VALUE)
		return NULL;

	retval = FindNextFile(findhandle,&finddata);
	if (retval)
		return finddata.cFileName;

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
#ifndef GLQUAKE
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	DWORD	flOldProtect;

//@@@ copy on write or just read-write?
	if (!VirtualProtect((LPVOID)startaddr, length, PAGE_READWRITE, &flOldProtect))
		Sys_Error("Protection change failed\n");
}
#endif	// !GLQUAKE


/*
================
Sys_Init
================
*/
static void Sys_Init (void)
{
	LARGE_INTEGER	PerformanceFreq;
	unsigned int	lowpart, highpart;
	OSVERSIONINFO	vinfo;

#ifndef SERVERONLY
	// allocate a named semaphore on the client so the
	// front end can tell if it is alive

	// mutex will fail if semephore already exists
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
#endif

	MaskExceptions ();
	Sys_SetFPCW ();

	if (!QueryPerformanceFrequency (&PerformanceFreq))
		Sys_Error ("No hardware timer available");

// get 32 out of the 64 time bits such that we have around
// 1 microsecond resolution
	lowpart = (unsigned int)PerformanceFreq.LowPart;
	highpart = (unsigned int)PerformanceFreq.HighPart;
	lowshift = 0;

	while (highpart || (lowpart > 2000000.0))
	{
		lowshift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	pfreq = 1.0 / (double)lowpart;

	Sys_InitFloatTime ();

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
		// Win95-gold or Win95A can't switch bpp automatically
		if (vinfo.szCSDVersion[1] != 'C' && vinfo.szCSDVersion[1] != 'B')
			Win95old = true;
	}
}


void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];

	Host_Shutdown ();

	va_start (argptr, error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

	MessageBox(NULL, text, ENGINE_NAME " Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);

#ifndef SERVERONLY
	CloseHandle (qwclsemaphore);
#endif

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;

	va_start (argptr,fmt);
	vprintf (fmt, argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
	Host_Shutdown();
#ifndef SERVERONLY
	if (qwclsemaphore)
		CloseHandle (qwclsemaphore);
#endif

	exit (0);
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	Sys_PushFPCW_SetHigh ();

	QueryPerformanceCounter (&PerformanceCount);

	temp = ((unsigned int)PerformanceCount.LowPart >> lowshift) |
		   ((unsigned int)PerformanceCount.HighPart << (32 - lowshift));

	if (first)
	{
		oldtime = temp;
		first = 0;
	}
	else
	{
	// check for turnover or backward time
		if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			t2 = temp - oldtime;

			time = (double)t2 * pfreq;
			oldtime = temp;

			curtime += time;

			if (curtime == lastcurtime)
			{
				sametimecount++;

				if (sametimecount > 100000)
				{
					curtime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			lastcurtime = curtime;
		}
	}

	Sys_PopFPCW ();

	return curtime;
}


/*
================
Sys_InitFloatTime
================
*/
static void Sys_InitFloatTime (void)
{
	int		j;

	Sys_DoubleTime ();

	j = COM_CheckParm("-starttime");

	if (j)
	{
		curtime = (double) (atof(com_argv[j+1]));
	}
	else
	{
		curtime = 0.0;
	}

	lastcurtime = curtime;
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
char		*argv[MAX_NUM_ARGVS];
static char	*empty_string = "";
#if !defined(NO_SPLASHES)
HWND	hwnd_dialog;
#endif

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	quakeparms_t	parms;
	double			time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;
	static	char	cwd[1024];
	int				t;
#if !defined(NO_SPLASHES)
	RECT			rect;
#endif

	/* previous instances do not exist in Win32 */
	if (hPrevInstance)
		return 0;

	global_hInstance = hInstance;
	global_nCmdShow = nCmdShow;

	lpBuffer.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus (&lpBuffer);

	if (!GetCurrentDirectory (sizeof(cwd), cwd))
		Sys_Error ("Couldn't determine current directory");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	parms.basedir = cwd;
	parms.cachedir = NULL;
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

	COM_InitArgv (parms.argc, parms.argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

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
#endif

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

	t = COM_CheckParm ("-heapsize");
	if (t && t < com_argc-1)
	{
		parms.memsize = atoi (com_argv[t + 1]) * 1024;
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

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

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

