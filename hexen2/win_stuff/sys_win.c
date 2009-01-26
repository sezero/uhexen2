/*
	sys_win.c
	Win32 system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/win_stuff/sys_win.c,v 1.76 2009-01-26 12:25:07 sezero Exp $
*/

#include "quakedef.h"
#include "winquake.h"
#include <mmsystem.h>
#include <limits.h>
#include <errno.h>
#include "resource.h"
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include "conproc.h"
#include "debuglog.h"
#include "io_msvc.h"


// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

#define CONSOLE_ERROR_TIMEOUT	60.0	/* # of seconds to wait on Sys_Error running dedicated before exiting */
#define PAUSE_SLEEP		50	/* sleep time on pause or minimization		*/
#define NOT_FOCUS_SLEEP		20	/* sleep time when not focus			*/

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};

qboolean	ActiveApp, Minimized;
qboolean	Win95, Win95old, WinNT;

qboolean		isDedicated;

/*
#define	TIME_WRAP_VALUE	LONG_MAX
*/
#define	TIME_WRAP_VALUE	(~(DWORD)0)
static DWORD		starttime;
static qboolean		sc_return_on_enter = false;
static HANDLE		hinput, houtput;

/*
#define	CRC_A 59461	// "Who's Ridin' With Chaos?"
#define	CRC_B 54866	// "Santa needs a new sled!"
static	char		*tracking_tag = "Sticky Buns";
*/

static HANDLE	tevent;
static HANDLE	hFile;
static HANDLE	heventParent;
static HANDLE	heventChild;

static volatile int	sys_checksum;


/*
================
Sys_PageIn
================
*/
static void Sys_PageIn (void *ptr, int size)
{
	byte	*x;
	int		m, n;

// touch all the memory to make sure it's there. The 16-page skip is to
// keep Win 95 from thinking we're trying to page ourselves in (we are
// doing that, of course, but there's no reason we shouldn't)
	x = (byte *)ptr;

	for (n = 0; n < 4; n++)
	{
		for (m = 0; m < (size - 16 * 0x1000); m += 4)
		{
			sys_checksum += *(int *)&x[m];
			sys_checksum += *(int *)&x[m + 16 * 0x1000];
		}
	}
}


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path, qboolean crash)
{
	int rc = _mkdir (path);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0 && crash)
		Sys_Error("Unable to create directory %s", path);
	return rc;
}

int Sys_rmdir (const char *path)
{
	return _rmdir(path);
}

int Sys_unlink (const char *path)
{
	return _unlink(path);
}

#define NO_OVERWRITING	FALSE /* allow overwriting files */
int Sys_CopyFile (const char *frompath, const char *topath)
{
	int	err;
	err = ! CopyFile (frompath, topath, NO_OVERWRITING);
	return err;
}
#undef  NO_OVERWRITING

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

/* do we really need these with opengl ?? */
	MaskExceptions ();
	Sys_SetFPCW ();
}


#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG], text2[MAX_PRINTMSG];
	const char	text3[] = "Press Enter to exit\n";
	const char	text4[] = "***********************************\n";
	const char	text5[] = "\n";
	DWORD		dummy;
	double		err_begin;

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

	if (isDedicated)
	{
		q_snprintf (text2, sizeof (text2), "ERROR: %s\n", text);
		if (text2[sizeof(text2)-2] != '\0')
			text2[sizeof(text2)-2] = '\n';
		WriteFile (houtput, text5, strlen (text5), &dummy, NULL);
		WriteFile (houtput, text4, strlen (text4), &dummy, NULL);
		WriteFile (houtput, text2, strlen (text2), &dummy, NULL);
		WriteFile (houtput, text3, strlen (text3), &dummy, NULL);
		WriteFile (houtput, text4, strlen (text4), &dummy, NULL);

		err_begin = Sys_DoubleTime ();
		sc_return_on_enter = true;	// so Enter will get us out of here
		while (!Sys_ConsoleInput () &&
			((Sys_DoubleTime () - err_begin) < CONSOLE_ERROR_TIMEOUT))
		{
		}
	}
	else
	{
		MessageBox(NULL, text, ENGINE_NAME " Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
	}

// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (1);
}

void Sys_PrintTerm (const char *msgtxt)
{
	DWORD		dummy;

	if (isDedicated)
	{
		if (sys_nostdout.integer)
			return;

		WriteFile(houtput, msgtxt, strlen(msgtxt), &dummy, NULL);
	}
}

void Sys_Quit (void)
{
	Host_Shutdown();

	if (tevent)
		CloseHandle (tevent);

	if (isDedicated)
		FreeConsole ();

// shut down QHOST hooks if necessary
	DeinitConProc ();

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


char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int		textlen;
	INPUT_RECORD	recs[1024];
	int		ch;
	DWORD		dummy, numread, numevents;

	if (!isDedicated)
		return NULL;

	for ( ;; )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
				case '\r':
					WriteFile(houtput, "\r\n", 2, &dummy, NULL);

					if (textlen)
					{
						con_text[textlen] = 0;
						textlen = 0;
						return con_text;
					}
					else if (sc_return_on_enter)
					{
					// special case to allow exiting from the error handler on Enter
						con_text[0] = '\r';
						textlen = 0;
						return con_text;
					}

					break;

				case '\b':
					WriteFile(houtput, "\b \b", 3, &dummy, NULL);
					if (textlen)
					{
						textlen--;
					}
					break;

				default:
					if (ch >= ' ')
					{
						WriteFile(houtput, &ch, 1, &dummy, NULL);
						con_text[textlen] = ch;
						textlen = (textlen + 1) & 0xff;
					}

					break;
				}
			}
		}
	}

	return NULL;
}

void Sys_Sleep (unsigned long msecs)
{
	if (!msecs)
		return;
	else if (msecs > 1000)
		msecs = 1000;

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
			if ((cliptext = GlobalLock(hClipboardData)) != NULL)
			{
				size_t size = GlobalSize(hClipboardData) + 1;
				/* this is intended for simple small text
				 * copies, such as ip addresses, etc:
				 * do chop the size here, otherwise we may
				 * experience Z_Malloc failures, or integer
				 * overflow crashes for worse. */
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
SleepUntilInput
==================
*/
static void SleepUntilInput (unsigned long msecs)
{
	MsgWaitForMultipleObjects(1, &tevent, FALSE, msecs, QS_ALLINPUT);
}


static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	Sys_Printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	Sys_Printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
	Sys_Printf ("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, PLATFORM_STRING);
	Sys_Printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

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
static char	empty_string[] = "";
static char	cwd[1024];
static quakeparms_t	parms;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int		i;
	double		time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;
#if !defined(NO_SPLASHES)
	RECT		rect;
#endif	/* NO_SPLASHES */

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

	COM_ValidateByteorder ();

	isDedicated = (COM_CheckParm ("-dedicated") != 0);

#if !defined(NO_SPLASHES)
	if (!isDedicated)
	{
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

	if (isDedicated)
		parms.memsize = MIN_MEM_ALLOC;

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

	if (COM_CheckParm("-nopagein") == 0)
	{
		Sys_PageIn (parms.membase, parms.memsize);
	}

	tevent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!tevent)
		Sys_Error ("Couldn't create event");

	if (isDedicated)
	{
		if (!AllocConsole ())
		{
			Sys_Error ("Couldn't create dedicated server console");
		}

		hinput = GetStdHandle (STD_INPUT_HANDLE);
		houtput = GetStdHandle (STD_OUTPUT_HANDLE);

#if !defined(_WIN64)
# define a_to_intptr						 atoi
#else
# define a_to_intptr						_atoi64
#endif	/* _WIN64 */
	// give QHOST a chance to hook into the console
		i = COM_CheckParm ("-HFILE");
		if (i && i < com_argc-1)
		{
			hFile = (HANDLE)a_to_intptr(com_argv[i+1]);
		}

		i = COM_CheckParm ("-HPARENT");
		if (i && i < com_argc-1)
		{
			heventParent = (HANDLE)a_to_intptr(com_argv[i+1]);
		}

		i = COM_CheckParm ("-HCHILD");
		if (i && i < com_argc-1)
		{
			heventChild = (HANDLE)a_to_intptr(com_argv[i+1]);
		}

		InitConProc (hFile, heventParent, heventChild);

		PrintVersion();
	}

	Sys_Init ();

// because sound is off until we become active
	S_BlockSound ();

	Host_Init();

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
		if (isDedicated)
		{
			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;

			while (time < sys_ticrate.value )
			{
				Sleep (1);
				newtime = Sys_DoubleTime ();
				time = newtime - oldtime;
			}
		}
		else
		{
		// yield the CPU for a little while when paused, minimized, or not the focus
			if ((cl.paused && (!ActiveApp && !DDActive)) || Minimized || block_drawing)
			{
				SleepUntilInput (PAUSE_SLEEP);
				scr_skipupdate = 1;		// no point in bothering to draw
			}
			else if (!ActiveApp && !DDActive)
			{
				SleepUntilInput (NOT_FOCUS_SLEEP);
				scr_skipupdate = 1;		// no point in bothering to draw
			}

			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;
		}

		Host_Frame (time);
		oldtime = newtime;
	}

	return 0;
}

