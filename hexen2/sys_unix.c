// sys_unix.c -- Unix system interface code

/*
 * $Header: /home/ozzie/Download/0000/uhexen2/hexen2/sys_unix.c,v 1.2 2004-11-28 00:58:08 sezero Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>

#include "quakedef.h"
#include "errno.h"
#include "resource.h"

#define CRC_A 59461 // "Who's Ridin' With Chaos?"
#define CRC_B 54866 // "Santa needs a new sled!"

#ifdef GLQUAKE
	#define MINIMUM_WIN_MEMORY		0x1000000
	#define MAXIMUM_WIN_MEMORY		0x1800000
#else
	#define MINIMUM_WIN_MEMORY		0x0C00000
	#define MAXIMUM_WIN_MEMORY		0x1600000
#endif

#define CONSOLE_ERROR_TIMEOUT	60.0	// # of seconds to wait on Sys_Error running
										//  dedicated before exiting
#define PAUSE_SLEEP		50				// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP	20				// sleep time when not focus

#define MAXPRINTMSG	4096

int			starttime;
qboolean	ActiveApp, Minimized;
qboolean	Win32AtLeastV4, WinNT;
qboolean	LegitCopy = true;

static double		pfreq;
static double		curtime = 0.0;
static double		lastcurtime = 0.0;
static int			lowshift;
qboolean			isDedicated;
static qboolean		sc_return_on_enter = false;
HANDLE				hinput, houtput;

static char			*tracking_tag = "Sticky Buns";

static HANDLE	tevent;
static HANDLE	hFile;
static HANDLE	heventParent;
static HANDLE	heventChild;

void MaskExceptions (void);
void Sys_InitFloatTime (void);

cvar_t		sys_delay = {"sys_delay","0", true};

volatile int					sys_checksum;


/*
================
Sys_PageIn
================
*/
void Sys_PageIn (void *ptr, int size)
{
	byte	*x;
	int		j, m, n;

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

#define	MAX_HANDLES		10
FILE	*sys_handles[MAX_HANDLES];

int		findhandle (void)
{
	int		i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int		pos;
	int		end;
	int		t;

	t = VID_ForceUnlockedAndReturnState ();

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	VID_ForceLockState (t);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE	*f;
	int		i, retval;
	int		t;

	t = VID_ForceUnlockedAndReturnState ();

	i = findhandle ();

	f = fopen(path, "rb");

	if (!f)
	{
		*hndl = -1;
		retval = -1;
	}
	else
	{
		sys_handles[i] = f;
		*hndl = i;
		retval = filelength(f);
	}

	VID_ForceLockState (t);

	return retval;
}

int Sys_FileOpenWrite (char *path)
{
	FILE	*f;
	int		i;
	int		t;

	t = VID_ForceUnlockedAndReturnState ();
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	VID_ForceLockState (t);

	return i;
}

void Sys_FileClose (int handle)
{
	int		t;

	t = VID_ForceUnlockedAndReturnState ();
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
	VID_ForceLockState (t);
}

void Sys_FileSeek (int handle, int position)
{
	int		t;

	t = VID_ForceUnlockedAndReturnState ();
	fseek (sys_handles[handle], position, SEEK_SET);
	VID_ForceLockState (t);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	int		t, x;

	t = VID_ForceUnlockedAndReturnState ();
	x = fread (dest, 1, count, sys_handles[handle]);
	VID_ForceLockState (t);
	return x;
}

int Sys_FileWrite (int handle, void *data, int count)
{
	int		t, x;

	t = VID_ForceUnlockedAndReturnState ();
	x = fwrite (data, 1, count, sys_handles[handle]);
	VID_ForceLockState (t);
	return x;
}

int	Sys_FileTime (char *path)
{
	FILE	*f;
	int		t, retval;

	t = VID_ForceUnlockedAndReturnState ();
	
	f = fopen(path, "rb");

	if (f)
	{
		fclose(f);
		retval = 1;
	}
	else
	{
		retval = -1;
	}
	
	VID_ForceLockState (t);
	return retval;
}

void Sys_mkdir (char *path)
{
	mkdir (path, 0777);
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
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
        int                     r;
        unsigned long   addr;
        int                     psize = getpagesize();

        addr = (startaddr & ~(psize-1)) - psize;

//      fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//                      addr, startaddr+length, length);

        r = mprotect ((char *) addr, length + startaddr - addr + psize, 7);

        if (r < 0)
                Sys_Error("Protection change failed\n");
#if 0
	DWORD  flOldProtect;

	if (!VirtualProtect((LPVOID)startaddr, length, PAGE_READWRITE, &flOldProtect))
   		Sys_Error("Protection change failed\n");
#endif
}

#ifdef SECURE_TIME
#include <time.h>
#define BuildTime 889138800
void CheckTime()
{
	time_t now;
	int    days;

	time( &now );
	if ((now < BuildTime) || (now> BuildTime+26*24*60*60))
	{
		errormessage = "This version has expired.@Please contact Activision for a new version";
		LegitCopy = false;
	}
}

#endif

/*
================
Sys_Init
================
*/
void Sys_Init (void)
{
	Sys_SetFPCW();
#if 0
	LARGE_INTEGER	PerformanceFreq;
	unsigned int	lowpart, highpart;
	OSVERSIONINFO	vinfo;
static	char temp[MAX_PATH+1];
	int value,i;
	HKEY hKey;
    DWORD dwSize,dwType;
	unsigned short crc;

	MaskExceptions ();
	Sys_SetFPCW ();

#if 0
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
#endif
	pfreq = 1.0 / (double)lowpart;

	Sys_InitFloatTime ();

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx (&vinfo))
		Sys_Error ("Couldn't get OS info");

	if (vinfo.dwMajorVersion < 4)
		Win32AtLeastV4 = false;
	else
		Win32AtLeastV4 = true;

	if (vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error ("Hexen2 requires at least Win95 or NT 4.0");
	
	if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		WinNT = true;
	else
		WinNT = false;

#ifdef SECURE
	dwSize = sizeof(temp);
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 "SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName",
                 0, KEY_READ, &hKey);
    value = RegQueryValueEx(hKey,"RAID2",NULL,&dwType,temp,&dwSize);

	if (value == ERROR_SUCCESS && dwType == REG_SZ)
	{
		CRC_Init(&crc);
		dwSize = strlen(temp);
		for(i=0;i<dwSize;i++)
		{
			CRC_ProcessByte(&crc,temp[i]);
		}
		i = CRC_Value(crc);
//		Con_Printf("CRC is %d\n",i);

		if (i != CRC_A && i != CRC_B)
			LegitCopy = false;
	}
	else LegitCopy = false;

	if (!LegitCopy)
		errormessage = "You need to re-install Hexen 2 on this computer!";

#else
	LegitCopy = true;
#endif

#ifdef ACTIVISION
	FindEncryption();
#endif

#ifdef SECURE
	FindCD();
#endif
#ifdef SECURE_TIME
	CheckTime();
#endif
#endif // 0
}


void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG], text2[MAXPRINTMSG];
	char		*text3 = "Press Enter to exit\n";
	char		*text4 = "***********************************\n";
	char		*text5 = "\n";
//	DWORD		dummy;
	double		starttime;

	VID_ForceUnlockedAndReturnState ();

	va_start (argptr, error);
	vsnprintf (text, MAXPRINTMSG, error, argptr);
	va_end (argptr);

	if (isDedicated)
	{
		va_start (argptr, error);
		vsnprintf (text, MAXPRINTMSG, error, argptr);
		va_end (argptr);

#if 0
		sprintf (text2, "ERROR: %s\n", text);
		WriteFile (houtput, text5, strlen (text5), &dummy, NULL);
		WriteFile (houtput, text4, strlen (text4), &dummy, NULL);
		WriteFile (houtput, text2, strlen (text2), &dummy, NULL);
		WriteFile (houtput, text3, strlen (text3), &dummy, NULL);
		WriteFile (houtput, text4, strlen (text4), &dummy, NULL);
#endif
		fprintf(stderr, "ERROR: %s\n", text);


		starttime = Sys_FloatTime ();
		sc_return_on_enter = true;	// so Enter will get us out of here

		while (!Sys_ConsoleInput () &&
				((Sys_FloatTime () - starttime) < CONSOLE_ERROR_TIMEOUT))
		{
		}
	}
	else
	{
	// switch to windowed so the message box is visible
		VID_SetDefaultMode ();
//		MessageBox(NULL, text, "Hexen II Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		fprintf(stderr, "ERROR: %s\n", text);
	}

	Host_Shutdown ();

#if 0
// shut down QHOST hooks if necessary
	DeinitConProc ();
#endif

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];
	//DWORD		dummy;
	
	va_start (argptr,fmt);
	vsnprintf (text, MAXPRINTMSG, fmt, argptr);
	va_end (argptr);

	if (isDedicated)
	{
		va_start (argptr,fmt);
		vsnprintf (text, MAXPRINTMSG, fmt, argptr);
		va_end (argptr);

//		WriteFile(houtput, text, strlen (text), &dummy, NULL);	
	}
	fprintf(stderr, "%s", text);
}

void Sys_Quit (void)
{

	VID_ForceUnlockedAndReturnState ();

	Host_Shutdown();

#if 0
	if (tevent)
		CloseHandle (tevent);

	if (isDedicated)
		FreeConsole ();

// shut down QHOST hooks if necessary
	DeinitConProc ();
#endif

	exit (0);
}


/*
================
Sys_FloatTime
================
*/
double Sys_FloatTime (void)
{
	// This is Sys_DoubleTime from Quake, since Hexen 2's Sys_FloatTime
	// is inherently un-portable - DDOI
        struct timeval  tp;
        struct timezone tzp;
        static int              secbase;

        gettimeofday(&tp, &tzp);

        if (!secbase) {
                secbase = tp.tv_sec;
                return tp.tv_usec/1000000.0;
        }

        return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
#if 0
	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	Sys_PushFPCW_SetHigh ();

	QueryPerformanceCounter (&PerformanceCount);

	temp = ((unsigned int)PerformanceCount.part.LowPart >> lowshift) |
		   ((unsigned int)PerformanceCount.part.HighPart << (32 - lowshift));

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
#endif
    return 1;
}


/*
================
Sys_InitFloatTime
================
*/
void Sys_InitFloatTime (void)
{
	int		j;

	Sys_FloatTime ();

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


char *Sys_ConsoleInput (void)
{
#warning FIXME Soon - DDOI
#if 0
	static char	text[256];
	static int		len;
	INPUT_RECORD	recs[1024];
	int		count;
	int		i, dummy;
	int		ch, numread, numevents;

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

						if (len)
						{
							text[len] = 0;
							len = 0;
							return text;
						}
						else if (sc_return_on_enter)
						{
						// special case to allow exiting from the error handler on Enter
							text[0] = '\r';
							len = 0;
							return text;
						}

						break;

					case '\b':
						WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						if (len)
						{
							len--;
						}
						break;

					default:
						if (ch >= ' ')
						{
							WriteFile(houtput, &ch, 1, &dummy, NULL);	
							text[len] = ch;
							len = (len + 1) & 0xff;
						}

						break;

				}
			}
		}
	}

#endif
	return NULL;
}

void Sys_Sleep (void)
{
	//Sleep (1);
	usleep(1);
}

void Sys_SendKeyEvents (void)
{
	IN_SendKeyEvents();
#if 0
    MSG        msg;

	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
	// we always update if there are any event, even if we're paused
		scr_skipupdate = 0;

		if (!GetMessage (&msg, NULL, 0, 0))
			Sys_Quit ();
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}
#endif
}

#if 0
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
void SleepUntilInput (int time)
{

	MsgWaitForMultipleObjects(1, &tevent, FALSE, time, QS_ALLINPUT);
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
HWND		hwnd_dialog;


#ifdef GLQUAKE

#define H2_PARAM_KEY      "Software\\Hexen2"
#define H2_FLAG_VALUE     "Flag"

#endif

#endif

int Sys_GetUserdir(char *buff, unsigned int len)
{
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 0;
    }

#ifdef H2MP
    if ( strlen( pwent->pw_dir ) + strlen( AOT_USERDIR) + 10 > (unsigned)len ) {
	return 0;
    }

    sprintf( buff, "%s/%s/portals", pwent->pw_dir, AOT_USERDIR );
#else
    if ( strlen( pwent->pw_dir ) + strlen( AOT_USERDIR) + 2 > (unsigned)len ) {
	return 0;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, AOT_USERDIR );
#endif
    Sys_mkdir(buff);

    return 1;
}

void PrintHelp(char *name)
{
	printf ("\n");
	PrintVersion();
	printf ("http://sourceforge.net/somewhere\n");
	printf ("\n");
	printf ("Please send bug reports or patches to:\n");
//	printf ("     Dan Olson          <theoddone33@linuxgames.com>\n");
//	printf ("     Clement Bourdarias <phneutre@mangoquest.org>\n");
	printf ("     Ozkan Sezer        <sezeroz@ttnet.net.tr>\n");
	printf ("     Steven Atkinson    <stevenaaus@yahoo.com>\n");
	printf ("\n");
	printf ("Usage: %s [options]\n", name);
	printf ("     [-h | --help]                   Display this help message\n");
	printf ("     [-v | --version]                Display the game version\n");
	printf ("     [-f | --fullscreen]             Run the game fullscreen\n");
	printf ("     [-w | --windowed]               Run the game windowed\n");
	printf ("     [-s | --nosound]                Run the game without sound\n");
	printf ("     [-g | --gllibrary]              Select 3D rendering library\n");
	printf ("     [-width Width [-height Height]] Select screen size\n");
	printf ("     [-bpp]                          Depth for GL fullscreen mode\n");
	printf ("     [-heapsize Bytes]               Heapsize\n");
	printf ("\n");
}

void PrintVersion (void)
{
	printf ("Hammer of Thyrion (%d.%d.%d)\n",
	HOT_VERSION_MAJ,HOT_VERSION_MID,HOT_VERSION_MIN);
	printf ("Anvil  of Thyrion 1.12 (%s %d.%d.%d)\n",
	VERSION_PLATFORM,VERSION_MAJ,VERSION_MID,VERSION_MIN);
	printf ("\tPatched by Steven A. and Ozkan Sezer with some\n");
	printf ("\tcode borrowed from the pa3pyx and js mods\n");

}

int main(int argc, char *argv[])
{
	quakeparms_t	parms;
	double			time, oldtime, newtime;
	static	char	cwd[1024];
	static  char    userdir[1024];
	int				t;
	char binary_name[1024];

	if (!(getcwd (cwd, sizeof(cwd))))
		Sys_Error ("Couldn't determine current directory");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	if (!(Sys_GetUserdir(userdir,sizeof(userdir))))
	        Sys_Error ("Couldn't determine userspace directory");

	else printf("userdir is: %s\n",userdir);

	parms.basedir = cwd;
	parms.cachedir = NULL;
	parms.userdir = userdir;

	memset(binary_name,0,1024);
	strncpy(binary_name,argv[0],1024);

	parms.argc = 1;
	argv[0] = "";

	parms.argv = argv;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	if (COM_CheckParm("-h")||COM_CheckParm("--help")||COM_CheckParm("-?"))
	{
		PrintHelp(binary_name);
		exit (0);
	}

	if (COM_CheckParm ("-v") || COM_CheckParm ("--version"))
	{
		PrintVersion();
		exit (0);
	}
			
	isDedicated = (COM_CheckParm ("-dedicated") != 0);

// take the greater of all the available memory or half the total memory,
// but at least 8 Mb and no more than 16 Mb, unless they explicitly
// request otherwise
	parms.memsize = 16*1024*1024;

	if (COM_CheckParm ("-heapsize"))
	{
		t = COM_CheckParm("-heapsize") + 1;

		if (t < com_argc)
			parms.memsize = atoi (com_argv[t]) * 1024;
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error ("Not enough memory free; check disk space\n");

	if(COM_CheckParm("-nopagein") == 0)
	{
		Sys_PageIn (parms.membase, parms.memsize);
	}

#if 0
	if (!tevent)
		Sys_Error ("Couldn't create event");

	if (isDedicated)
	{
		if (!AllocConsole ())
		{
			Sys_Error ("Couldn't create dedicated server console");
		}

	// give QHOST a chance to hook into the console
		// FIXME - DDOI
	//InitConProc (hFile, heventParent, heventChild);
	}
#endif

	Sys_Init ();

// because sound is off until we become active
	//S_BlockSound ();

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

	oldtime = Sys_FloatTime ();

	Cvar_RegisterVariable (&sys_delay);

    /* main window message loop */
	while (1)
	{
		if (isDedicated)
		{
			newtime = Sys_FloatTime ();
			time = newtime - oldtime;

			while (time < sys_ticrate.value )
			{
				Sys_Sleep();
				newtime = Sys_FloatTime ();
				time = newtime - oldtime;
			}
		}
		else
		{
			newtime = Sys_FloatTime ();
			time = newtime - oldtime;
		}

		if (sys_delay.value) 
			usleep(sys_delay.value);

		Host_Frame (time);
		oldtime = newtime;
	}

    /* return success of application */
    return 1;
}

void strlwr (char * str)
{
	while (*str) {*str = tolower (*str); str++; }
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2004/11/28 00:07:58  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.10  2002/01/06 03:15:48  theoddone33
 * Separate portals dir so that hexen 2 will not list PoP saves.
 *
 * Revision 1.9  2002/01/02 17:13:12  phneutre
 * added myself to authors
 *
 * Revision 1.8  2002/01/02 15:15:44  phneutre
 * [-g | --gllibrary] command line support
 *
 * Revision 1.7  2001/12/14 16:31:06  phneutre
 * moved CL_RemoveGIPFiles(NULL) grom main() to Host_Init because of NULL paths
 *
 * Revision 1.6  2001/12/13 22:21:08  phneutre
 * there is something strange, see comment in main()
 *
 * Revision 1.5  2001/12/13 16:06:15  phneutre
 * initial support for userspace directory (~/.aot) (parms.userdir)
 *
 * Revision 1.4  2001/12/06 14:39:38  theoddone33
 * Die Nvidia bug. Die die die.
 *
 * Revision 1.3  2001/12/02 04:59:43  theoddone33
 * Fix nvidia extention problem and a whole bunch of other stuff too apparently
 *
 * Revision 1.2  2001/11/12 23:31:58  theoddone33
 * Some Loki-ish parameters and general cleanup/bugfixes.
 *
 * Revision 1.1.1.1  2001/11/09 17:04:10  theoddone33
 * Inital import
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
