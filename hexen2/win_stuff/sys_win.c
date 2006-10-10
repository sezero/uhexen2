// sys_win.c -- Win32 system interface code

#include "quakedef.h"
#include "winquake.h"
#include <errno.h>
#include "resource.h"
#include <io.h>
#include "conproc.h"


//#define CRC_A 59461 // "Who's Ridin' With Chaos?"
//#define CRC_B 54866 // "Santa needs a new sled!"

// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

#define CONSOLE_ERROR_TIMEOUT	60.0	// # of seconds to wait on Sys_Error running
						//  dedicated before exiting
#define PAUSE_SLEEP		50	// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP		20	// sleep time when not focus

qboolean	ActiveApp, Minimized;
qboolean	Win95, Win95old, WinNT;

static double		pfreq;
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static int			lowshift;
qboolean			isDedicated;
static qboolean		sc_return_on_enter = false;
static HANDLE		hinput, houtput;

//static char		*tracking_tag = "Sticky Buns";

static HANDLE	tevent;
static HANDLE	hFile;
static HANDLE	heventParent;
static HANDLE	heventChild;

static void Sys_InitFloatTime (void);

volatile int		sys_checksum;

//=============================================================================


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

	for (n=0 ; n<4 ; n++)
	{
		for (m=0 ; m<(size - 16 * 0x1000) ; m += 4)
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
	if (findhandle)
		Sys_Error ("Sys_FindFirst without FindClose");

	findhandle = FindFirstFile(va("%s/%s", path, pattern), &finddata);

	if (findhandle != INVALID_HANDLE_VALUE)
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
	char		text[MAXPRINTMSG], text2[MAXPRINTMSG];
	char		*text3 = "Press Enter to exit\n";
	char		*text4 = "***********************************\n";
	char		*text5 = "\n";
	DWORD		dummy;
	double		starttime;

	Host_Shutdown ();

	va_start (argptr, error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

	if (isDedicated)
	{
		snprintf (text2, sizeof (text2), "ERROR: %s\n", text);
		if (text2[sizeof(text2)-2] != '\0')
			text2[sizeof(text2)-2] = '\n';
		WriteFile (houtput, text5, strlen (text5), &dummy, NULL);
		WriteFile (houtput, text4, strlen (text4), &dummy, NULL);
		WriteFile (houtput, text2, strlen (text2), &dummy, NULL);
		WriteFile (houtput, text3, strlen (text3), &dummy, NULL);
		WriteFile (houtput, text4, strlen (text4), &dummy, NULL);

		starttime = Sys_DoubleTime ();
		sc_return_on_enter = true;	// so Enter will get us out of here
		while (!Sys_ConsoleInput () &&
			((Sys_DoubleTime () - starttime) < CONSOLE_ERROR_TIMEOUT))
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

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];
	DWORD		dummy;

	if (isDedicated)
	{
		va_start (argptr,fmt);
		vsnprintf (text, sizeof (text), fmt, argptr);
		va_end (argptr);

		WriteFile(houtput, text, strlen (text), &dummy, NULL);
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

	if (j && j < com_argc-1)
	{
		curtime = (double) (atof(com_argv[j+1]));
	}
	else
	{
		curtime = 0.0;
	}

	lastcurtime = curtime;
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
SleepUntilInput
==================
*/
static void SleepUntilInput (int time)
{
	MsgWaitForMultipleObjects(1, &tevent, FALSE, time, QS_ALLINPUT);
}


static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	Sys_Printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	Sys_Printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
	Sys_Printf ("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, VERSION_PLATFORM);
	Sys_Printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

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
HWND		hwnd_dialog;
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

	if (isDedicated)
		parms.memsize = MIN_MEM_ALLOC;

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

	if(COM_CheckParm("-nopagein") == 0)
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

	// give QHOST a chance to hook into the console
		t = COM_CheckParm ("-HFILE");
		if (t && t < com_argc-1)
		{
			hFile = (HANDLE)atoi (com_argv[t+1]);
		}

		t = COM_CheckParm ("-HPARENT");
		if (t && t < com_argc-1)
		{
			heventParent = (HANDLE)atoi (com_argv[t+1]);
		}

		t = COM_CheckParm ("-HCHILD");
		if (t && t < com_argc-1)
		{
			heventChild = (HANDLE)atoi (com_argv[t+1]);
		}

		InitConProc (hFile, heventParent, heventChild);

		PrintVersion();
	}

	Sys_Init ();

// because sound is off until we become active
	S_BlockSound ();

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

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
				Sys_Sleep();
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

	/* return success of application */
	return TRUE;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.37  2006/10/10 06:43:22  sezero
 * ensure the newline if trunctaion occurs in win32 version of Sys_Error
 *
 * Revision 1.36  2006/09/23 07:25:36  sezero
 * added missing com_argc checks (and fixed the incorrect ones)
 * after several COM_CheckParm calls.
 *
 * Revision 1.35  2006/06/25 10:21:04  sezero
 * misc clean-ups and prepare for merging a dedicated server
 *
 * Revision 1.34  2006/06/14 10:13:32  sezero
 * removed bogus retval check from win32 version of Sys_FindFirstFile
 *
 * Revision 1.33  2006/04/17 14:00:51  sezero
 * minor update to version display stuff
 *
 * Revision 1.32  2006/03/24 17:34:24  sezero
 * includes cleanup
 *
 * Revision 1.31  2006/02/19 16:14:16  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. (part 13: more sys_XXX.c, along with more variable name clean-up.)
 *
 * Revision 1.30  2006/02/19 14:19:39  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. (part 11: sys_win.c). also did some synchronization between hwcl and
 * h2 versions.
 *
 * Revision 1.29  2006/01/17 18:46:53  sezero
 * missing part of vid_win synchronization (block_drawing stuff)
 *
 * Revision 1.28  2006/01/12 12:34:38  sezero
 * added video modes enumeration via SDL. added on-the-fly video mode changing
 * partially based on the Pa3PyX hexen2 tree. TODO: make the game remember its
 * video settings, clean it up, fix it up...
 *
 * Revision 1.27  2006/01/06 12:41:42  sezero
 * increased minimum heapsize to 16 mb, otherwise hunk allocation errors occur
 * with old 8 mb minimum. added a -forcemem commandline switch which enables
 * forcing user memory args out of min/max limits when parsing the -heapsize
 * and -zone switches.
 *
 * Revision 1.26  2006/01/06 12:15:02  sezero
 * added a simplified findfirst/findnext implementation: When given a
 * directory and a wildcard match pattern, they will find/report the
 * matches one after other. Unix implementation uses readdir()/fnmatch(),
 * win32 uses FindFirstFile native functions.
 * Sys_FindFirstFile and Sys_FindNextFile return filenames only, not a
 * dirent struct, therefore there is no attribute matching. this is
 * enough for what we presently need in this engine. may be improved in
 * future.
 *
 * Revision 1.25  2005/12/11 12:00:59  sezero
 * win32 wheelmouse cleanup
 *
 * Revision 1.24  2005/12/04 11:14:38  sezero
 * the big vsnprintf patch
 *
 * Revision 1.23  2005/10/02 15:43:08  sezero
 * killed -Wshadow warnings
 *
 * Revision 1.22  2005/09/28 06:06:38  sezero
 * killed the cvar sys_delay
 *
 * Revision 1.21  2005/09/24 23:50:36  sezero
 * fixed a bunch of compiler warnings
 *
 * Revision 1.20  2005/09/20 21:19:45  sezero
 * Sys_Quit and Sys_Error clean-up: VID_SetDefaultMode, VID_ForceLockState and
 * VID_ForceUnlockedAndReturnState are history. Host_Shutdown is called before
 * quit/error messages. Placed SDL_UnlockSurface() and MGL_endDirectAccess()
 * to VID_Shutdown, just in case. Added the word "HexenWorld" to win32 version
 * of hexenworld error window label. Took care of some accidentally repeated
 * code. "Fatalized" and added extra linefeeds to the sys_error messages.
 *
 * Revision 1.19  2005/09/20 21:17:26  sezero
 * Moved VERSION_PLATFORM and id386 defines to sys.h, where they belong.
 *
 * Revision 1.18  2005/08/20 13:06:34  sezero
 * favored unlink() over DeleteFile() on win32. removed unnecessary
 * platform defines for directory path separators. removed a left-
 * over CL_RemoveGIPFiles() from sys_win.c. fixed temporary gip files
 * not being removed and probably causing "bad" savegames on win32.
 *
 * Revision 1.17  2005/08/10 23:19:26  sezero
 * slight tweaks
 *
 * Revision 1.16  2005/08/07 10:59:06  sezero
 * killed the Sys_FileTime crap. now using standart access() function.
 *
 * Revision 1.15  2005/07/23 22:22:10  sezero
 * unified the common funcntions for hexen2-hexenworld
 *
 * Revision 1.14  2005/06/19 11:23:23  sezero
 * added wheelmouse support and conwidth support to hexen2. changed
 * hexenworld's default behavior of default 640 conwidth to main width
 * unless specified otherwise by the user. disabled startup splash
 * screens for now. sycned hexen2 and hexnworld's GL_Init_Functions().
 * disabled InitCommonControls()in gl_vidnt. moved RegisterWindowMessage
 * for uMSG_MOUSEWHEEL to in_win where it belongs. bumped MAXIMUM_WIN_MEMORY
 * to 32 MB. killed useless Sys_ConsoleInput in hwcl. several other sycning
 * and clean-up
 *
 * Revision 1.13  2005/06/17 16:24:41  sezero
 * win32 fixes and clean-ups
 *
 * Revision 1.12  2005/06/15 22:45:10  sezero
 * killed silly opengl pop-up info
 *
 * Revision 1.11  2005/06/15 11:10:52  sezero
 * made Sys_mkdir int for win32 as well
 *
 * Revision 1.10  2005/06/15 10:45:42  sezero
 * unused stuff
 *
 * Revision 1.9  2005/06/15 10:35:40  sezero
 * added missing includes
 *
 * Revision 1.8  2005/06/15 10:30:10  sezero
 * fix a compile problem on win32 (filelength clash with io.h)
 * and keep the names consistent throughout the tree
 *
 * Revision 1.7  2005/06/15 09:45:01  sezero
 * more endianness stuff
 *
 * Revision 1.6  2005/05/29 08:38:16  sezero
 * get rid of the silly func name difference
 *
 * Revision 1.5  2005/05/20 15:26:33  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 * Revision 1.4  2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * Revision 1.3  2005/05/17 17:40:01  sezero
 * (Re-)added the parms.userdir to all sys_win.c. The platform conditionals
 * around some of the com_userdir code of late are now unnecessary.
 *
 * Revision 1.2  2005/05/17 06:44:08  sezero
 * restored some win32 files to their original state
 *
 * 
 * 8     4/13/98 1:01p Jmonroe
 * changed default to not use CDVolume
 * 
 * 7     4/01/98 6:43p Jmonroe
 * fixed boundschecker errors
 * 
 * 6     3/16/98 6:25p Jmonroe
 * set up euro secure checks
 * 
 * 5     3/06/98 10:35a Jmonroe
 * 
 * 4     3/05/98 5:13p Jmonroe
 * added pr build security shit
 * 
 * 3     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 25    9/30/97 4:26p Rjohnson
 * Updates
 * 
 * 24    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 23    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 22    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 21    9/02/97 12:24a Rjohnson
 * Checks for a cd
 * 
 * 20    8/31/97 3:44p Rjohnson
 * Memory change
 * 
 * 19    8/27/97 12:10p Rjohnson
 * Increased max memory
 * 
 * 18    8/21/97 10:12p Rjohnson
 * Demo Updates
 * 
 * 17    8/21/97 11:05a Rjohnson
 * Deleting gip files
 * 
 * 16    8/20/97 2:59p Rjohnson
 * Fix for crashes and saves
 * 
 * 15    8/16/97 10:53a Rjohnson
 * Comment
 * 
 * 14    8/01/97 4:30p Rjohnson
 * More encryption work
 * 
 * 13    7/17/97 2:00p Rjohnson
 * Added a security means to control the running of the game
 * 
 * 12    6/16/97 12:25p Rjohnson
 * Name changes from winquake to hexenii
 * 
 * 11    6/16/97 3:13a Rjohnson
 * Fixes for: allocating less memory, models clipping out, and plaques in
 * gl version
 * 
 * 10    5/13/97 4:54p Rjohnson
 * Increased the memory requirements for the gl version
 * 
 * 9     5/01/97 12:18a Bgokey
 * 
 * 8     3/21/97 11:05a Rjohnson
 * Increased minimum memory to 12 megs
 * 
 * 7     3/07/97 2:24p Rjohnson
 * Id Updates
 * 
 * 6     2/24/97 3:12p Bgokey
 * Added -nopagein.
 * 
 * 5     2/18/97 4:55p Rjohnson
 * Id Updates
 * 
 * 4     1/02/97 11:16a Rjohnson
 * Christmas work - added adaptive time, game delays, negative light,
 * particle effects, etc
 * 
 * 3     12/10/96 12:22p Rjohnson
 * Code cleanup and made it so that quake won't try to do anything when it
 * isn't the active app
 */
