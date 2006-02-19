/*
	sys_unix.c
	Unix system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/sys_unix.c,v 1.44 2006-02-19 16:14:16 sezero Exp $
*/

#include "quakedef.h"

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <dirent.h>
#include <fnmatch.h>
#include "sdl_inc.h"


#ifdef ASSUMED_LITTLE_ENDIAN
#warning "Unable to determine CPU endianess. Defaulting to little endian"
#endif

// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

#define CONSOLE_ERROR_TIMEOUT	60.0	// # of seconds to wait on Sys_Error
					// before exiting

static Uint8		appState;
extern qboolean		draw_reinit;


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

	rc = mkdir (path, 0777);
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
static DIR		*finddir;
static struct dirent	*finddata;
static char		*findpattern;

char *Sys_FindFirstFile (char *path, char *pattern)
{
	int	pattern_len;

	if (finddir)
		Sys_Error ("Sys_FindFirst without FindClose");

	finddir = opendir (path);
	if (!finddir)
		return NULL;

	pattern_len = strlen (pattern);
	findpattern = malloc (pattern_len + 1);
	if (!findpattern)
		return NULL;
	strcpy (findpattern, pattern);

	do {
		finddata = readdir(finddir);
		if (finddata != NULL)
		{
			if (!fnmatch (findpattern, finddata->d_name, FNM_PATHNAME))
			{
				return finddata->d_name;
			}
		}
	} while (finddata != NULL);

	return NULL;
}

char *Sys_FindNextFile (void)
{
	if (!finddir)
		return NULL;

	do {
		finddata = readdir(finddir);
		if (finddata != NULL)
		{
			if (!fnmatch (findpattern, finddata->d_name, FNM_PATHNAME))
			{
				return finddata->d_name;
			}
		}
	} while (finddata != NULL);

	return NULL;
}

void Sys_FindClose (void)
{
	if (finddir != NULL)
		closedir(finddir);
	finddir = NULL;
	if (findpattern != NULL)
		free (findpattern);
	findpattern = NULL;
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
	int		r;
	unsigned long	addr;
	int		psize = getpagesize();

	addr = (startaddr & ~(psize-1)) - psize;

//	fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//		addr, startaddr+length, length);

	r = mprotect ((char *) addr, length + startaddr - addr + psize, 7);

	if (r < 0)
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
	Sys_SetFPCW();
}


void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];
//	double		starttime;

	// re-enable SDL_QUIT event which maybe disabled
	// during a video mode change
	draw_reinit = false;
	Host_Shutdown ();

	va_start (argptr, error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

	fprintf(stderr, "\nFATAL ERROR: %s\n\n", text);

/*	starttime = Sys_DoubleTime ();
	while (!Sys_ConsoleInput () &&
		((Sys_DoubleTime () - starttime) < CONSOLE_ERROR_TIMEOUT))
	{
	}
*/
	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsnprintf (text, sizeof (text), fmt, argptr);
	va_end (argptr);

	fprintf(stderr, "%s", text);
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
	// This is Sys_DoubleTime from Quake, since Hexen 2's Sys_DoubleTime
	// is inherently un-portable - DDOI
	struct timeval	tp;
	struct timezone	tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}


void Sys_Sleep (void)
{
	usleep(1);
}

void Sys_SendKeyEvents (void)
{
	IN_SendKeyEvents();
}

static int Sys_GetUserdir (char *buff, unsigned int path_len)
{
	if (getenv("HOME") == NULL)
		return 1;

/* O.S:	We keep the userdir (and host_parms.userdir) as ~/.hexen2
	here.  We'll change com_userdir in COM_InitFilesystem()
	depending on H2MP, H2W and/or -game cmdline arg, instead.
   S.A:	Now using $HOME istead of the passwd struct */

	if (strlen(getenv("HOME")) + strlen(AOT_USERDIR) + 5 > path_len)
		return 1;

	sprintf (buff, "%s/%s", getenv("HOME"), AOT_USERDIR);
	return Sys_mkdir(buff);
}

static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_BETA_DATE);
#else
	printf ("Hammer of Thyrion, release %s\n", HOT_VERSION_STR);
#endif
	printf ("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, VERSION_PLATFORM);
}

static char *help_strings[] = {
	"     [-v | --version]        Display the game version",
	"     [-w | --window ]        Run the game windowed",
	"     [-f | --fullscreen]     Run the game fullscreen",
	"     [-width X [-height Y]]  Select screen size",
#ifdef GLQUAKE
	"     [-bpp]                  Depth for GL fullscreen mode",
	"     [-vsync]                Enable sync with monitor refresh",
	"     [-g | --gllibrary]      Select 3D rendering library",
	"     [-fsaa N]               Enable N sample antialiasing",
	"     [-paltex]               Enable 8-bit textures",
	"     [-nomtex]               Disable multitexture detection/usage",
#endif
	"     [-s | --nosound]        Run the game without sound",
#if defined(__linux__) && !defined(NO_ALSA)
	"     [-sndalsa]              Use ALSA sound (alsa > 1.0.1)",
#endif
	"     [-sndsdl]               Use SDL sound",
	"     [-nomouse]              Disable mouse usage",
	"     [-heapsize Bytes]       Heapsize (memory to allocate)",
	"\n"
	"More info / sending bug reports:  http://uhexen2.sourceforge.net\n",
	NULL
};

static void PrintHelp(char *name)
{
	int i = 0;

	printf ("\nUsage: %s [options]\n", name);
	while (help_strings[i])
	{
		printf (help_strings[i]);
		printf ("\n");
		i++;
	}
}

int main(int argc, char *argv[])
{
	quakeparms_t	parms;
	double	time, oldtime, newtime;
	char	cwd[MAX_OSPATH];
	char	userdir[MAX_OSPATH];
	int	t;
	const SDL_version *sdl_version;

	PrintVersion();

	if (argc > 1)
	{
		for (t = 1; t < argc; t++)
		{
			if ( !(strcmp(argv[t], "-v")) || !(strcmp(argv[t], "-version" )) ||
				!(strcmp(argv[t], "--version")) )
			{
				exit(0);
			}
			else if ( !(strcmp(argv[t], "-h")) || !(strcmp(argv[t], "-help" )) ||
				  !(strcmp(argv[t], "-?")) || !(strcmp(argv[t], "--help")) )
			{
				PrintHelp(argv[0]);
				exit (0);
			}
		}
	}

	if (!(getcwd (cwd, sizeof(cwd))))
		Sys_Error ("Couldn't determine current directory");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	if (Sys_GetUserdir(userdir,sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");

	parms.basedir = cwd;
	parms.cachedir = NULL;
	parms.userdir = userdir;

	parms.argc = 1;
	argv[0] = "";

	parms.argv = argv;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	Sys_Printf("userdir is: %s\n",userdir);

	sdl_version = SDL_Linked_Version();
	Sys_Printf("Found SDL version %i.%i.%i\n",sdl_version->major,sdl_version->minor,sdl_version->patch);
	if (SDL_VERSIONNUM(sdl_version->major,sdl_version->minor,sdl_version->patch) < SDL_REQUIREDVERSION)
	{	//reject running under SDL versions older than what is stated in sdl_inc.h
		printf("You need at least v%d.%d.%d of SDL to run this game\n",SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z);
		exit(0);
	}

	parms.memsize = STD_MEM_ALLOC;

	t = COM_CheckParm ("-heapsize");
	if (t && t < com_argc-1)
	{
		parms.memsize = atoi (com_argv[t + 1]) * 1024;

		if ((parms.memsize > MAX_MEM_ALLOC) && !(COM_CheckParm ("-forcemem")))
		{
			Sys_Printf ("Requested memory (%d Mb) too large, using the default\n", parms.memsize/(1024*1024));
			Sys_Printf ("maximum. If you are sure, use the -forcemem switch\n");
			parms.memsize = MAX_MEM_ALLOC;
		}
		else if ((parms.memsize < MIN_MEM_ALLOC) && !(COM_CheckParm ("-forcemem")))
		{
			Sys_Printf ("Requested memory (%d Mb) too little, using the default\n", parms.memsize/(1024*1024));
			Sys_Printf ("minimum. If you are sure, use the -forcemem switch\n");
			parms.memsize = MIN_MEM_ALLOC;
		}
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error ("Insufficient memory.\n");

	Sys_Init ();

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
		appState = SDL_GetAppState();

		// If we have no input focus at all, sleep a bit
		if ( !(appState & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) || cl.paused)
		{
			usleep (16000);
		}
		// If we're minimised, sleep a bit more
		if ( !(appState & SDL_APPACTIVE) )
		{
			scr_skipupdate = 1;
			usleep (32000);
		}
		else
		{
			scr_skipupdate = 0;
		}

		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);
		oldtime = newtime;
	}

	return 0;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.43  2006/01/23 20:22:53  sezero
 * tidied up the version and help display stuff. bumped the HoT version to
 * 1.4.0-pre1. added conditionals to properly display beta version strings.
 *
 * Revision 1.42  2006/01/23 20:20:55  sezero
 * sys_unix.c: added code to sleep a bit when we have no focus or paused, and
 * sleep more when we are minimized. inspired from the q3 project at icculus.
 *
 * Revision 1.41  2006/01/12 12:43:49  sezero
 * Created an sdl_inc.h with all sdl version requirements and replaced all
 * SDL.h and SDL_mixer.h includes with it. Made the source to compile against
 * SDL versions older than 1.2.6 without disabling multisampling. Multisampling
 * (fsaa) option is now decided at runtime. Minimum required SDL and SDL_mixer
 * versions are now 1.2.4. If compiled without midi, minimum SDL required is
 * 1.2.0. Added SDL_mixer version checking to sdl-midi with measures to prevent
 * relocation errors.
 *
 * Revision 1.40  2006/01/12 12:34:39  sezero
 * added video modes enumeration via SDL. added on-the-fly video mode changing
 * partially based on the Pa3PyX hexen2 tree. TODO: make the game remember its
 * video settings, clean it up, fix it up...
 *
 * Revision 1.39  2006/01/06 12:41:42  sezero
 * increased minimum heapsize to 16 mb, otherwise hunk allocation errors occur
 * with old 8 mb minimum. added a -forcemem commandline switch which enables
 * forcing user memory args out of min/max limits when parsing the -heapsize
 * and -zone switches.
 *
 * Revision 1.38  2006/01/06 12:15:02  sezero
 * added a simplified findfirst/findnext implementation: When given a
 * directory and a wildcard match pattern, they will find/report the
 * matches one after other. Unix implementation uses readdir()/fnmatch(),
 * win32 uses FindFirstFile native functions.
 * Sys_FindFirstFile and Sys_FindNextFile return filenames only, not a
 * dirent struct, therefore there is no attribute matching. this is
 * enough for what we presently need in this engine. may be improved in
 * future.
 *
 * Revision 1.37  2005/12/04 11:14:38  sezero
 * the big vsnprintf patch
 *
 * Revision 1.36  2005/10/25 17:14:23  sezero
 * added a STRINGIFY macro. unified version macros. simplified version
 * printing. simplified and enhanced version watermark print onto console
 * background. added HoT lines to the quit menu (shameless plug)
 *
 * Revision 1.35  2005/09/28 06:06:38  sezero
 * killed the cvar sys_delay
 *
 * Revision 1.34  2005/09/20 21:19:45  sezero
 * Sys_Quit and Sys_Error clean-up: VID_SetDefaultMode, VID_ForceLockState and
 * VID_ForceUnlockedAndReturnState are history. Host_Shutdown is called before
 * quit/error messages. Placed SDL_UnlockSurface() and MGL_endDirectAccess()
 * to VID_Shutdown, just in case. Added the word "HexenWorld" to win32 version
 * of hexenworld error window label. Took care of some accidentally repeated
 * code. "Fatalized" and added extra linefeeds to the sys_error messages.
 *
 * Revision 1.33  2005/09/20 21:17:26  sezero
 * Moved VERSION_PLATFORM and id386 defines to sys.h, where they belong.
 *
 * Revision 1.32  2005/08/12 09:21:09  sezero
 * loosened SDL version restrictions depending on the SDL version
 * on the build system. will issue a warning if less than 1.2.6
 *
 * Revision 1.31  2005/08/10 23:19:26  sezero
 * slight tweaks
 *
 * Revision 1.30  2005/08/07 10:59:18  sezero
 * killed the Sys_FileTime crap. now using standart access() function.
 *
 * Revision 1.29  2005/07/30 15:19:13  sezero
 * insignificant cosmetics
 *
 * Revision 1.28  2005/07/23 22:22:10  sezero
 * unified the common funcntions for hexen2-hexenworld
 *
 * Revision 1.27  2005/07/22 17:06:43  sezero
 * whitespace cleanup
 *
 * Revision 1.26  2005/07/09 11:53:40  sezero
 * moved the local unix version of strlwr to zone.c, its only user.
 *
 * Revision 1.25  2005/07/09 07:00:03  sezero
 * SDL version refusal should not happen for dedicated servers
 *
 * Revision 1.24  2005/06/15 10:30:10  sezero
 * fix a compile problem on win32 (filelength clash with io.h)
 * and keep the names consistent throughout the tree
 *
 * Revision 1.23  2005/06/15 09:45:01  sezero
 * more endianness stuff
 *
 * Revision 1.22  2005/06/12 07:28:54  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.21  2005/06/01 14:13:22  sezero
 * SDL version >= 1.2.6 is now required. Updated the READMEs
 * and added a version check at startup.
 *
 * Revision 1.20  2005/05/30 09:44:20  sezero
 * updated commandline help display
 *
 * Revision 1.19  2005/05/26 09:38:11  sezero
 * updated help display
 *
 * Revision 1.18  2005/05/20 12:34:46  sezero
 * removed some windows left-overs from sys_unix
 *
 * Revision 1.17  2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * Revision 1.16  2005/04/30 08:34:56  sezero
 * removed pointless global decleration of starttime in sys_unix.c which
 * kills a shadowed decleration warning, as well.
 *
 * Revision 1.15  2005/04/15 20:23:17  sezero
 * no dedicated server possible in hexenworld client
 *
 * Revision 1.14  2005/04/14 07:36:15  sezero
 * -? arg is back (Steve likes it ;)
 *
 * Revision 1.13  2005/04/13 12:21:34  sezero
 * - Removed useless -minmemory cmdline argument
 * - Removed useless parms->memsize < minimum_memory check in Host_Init
 * - Added lower/upper boundaries (8mb/96mb) for -heapsize argument
 * - Added lower (48kb for hexen2, 256kb for hw)/upper (1mb) boundaries
 *   for -zone argument (DYNAMIC_SIZE definitions/zonesize requirements
 *   are different for hexen2 and hexenworld)
 * - We won't die if no size is specified after -zone, but will ignore
 * - Added null string terminations to hexen2 zone.c, so as to prevent
 *   garbage on sys_memory console command (found this in Pa3PyX)
 *
 * Revision 1.12  2005/03/03 19:48:46  sezero
 * More bits from Steven:
 * - increase MAX_OSPATH to 256
 * - Sys_Mkdir is now int. Its return code should be checked in other places, too.
 * - Sys_GetUserdir now uses $HOME instead of the passwd struct
 *
 * Revision 1.11  2005/03/03 17:03:39  sezero
 * - sys_unix.c cleanup: remove dead and/or win32 code
 * - remove unreached return from main (it should have been 0, btw. from Steve)
 *
 * Revision 1.10  2005/02/20 13:39:22  sezero
 * add the new sound options to the help messages
 *
 * Revision 1.9  2005/02/09 14:37:34  sezero
 * make compiler happy (unused warnings)
 *
 * Revision 1.8  2005/02/06 15:03:15  sezero
 * move resource.h to ./win_stuff/
 *
 * Revision 1.7  2005/01/18 11:29:18  sezero
 * - Fix userdirs to work correctly against the -game arg
 * - Added a "qboolean adduser" arg to COM_AddGameDir()  (useful esp. in
 *   hexenworld):  com_userdir evolves gradually. Every search path added
 *   has a consequence of additon of (multiple) unnecessary userpaths.
 *   Avoid it where we can. (original aot also added the very same userdir
 *   multiple times)
 * - Changed all instances of "id1" to "data1"
 *
 * Revision 1.6  2005/01/01 21:50:49  sezero
 * warnings cleanup: unused stuff
 *
 * Revision 1.5  2004/12/22 21:51:17  sezero
 * play with version and help display
 *
 * Revision 1.4  2004/12/12 19:01:02  sezero
 * port Steven's 2004-12-12 mouse changes form hexen2. hopefully correct...
 *
 * Revision 1.3  2004/12/05 12:25:58  sezero
 * Sync with Steven's changes to hexen2, 2004-12-04
 *
 * Revision 1.2  2004/12/04 02:10:33  sezero
 * porting from hexen2 :  version stuff.
 *
 * Revision 1.1.1.1  2004/11/28 08:57:00  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.5  2002/01/03 14:15:47  phneutre
 * ~/.aot/hw support
 *
 * Revision 1.4  2002/01/03 12:56:37  phneutre
 * changed "h2mp" [options] in help by the real name of the binary
 *
 * Revision 1.3  2001/12/06 14:39:41  theoddone33
 * Die Nvidia bug. Die die die.
 *
 * Revision 1.2  2001/12/03 21:11:28  theoddone33
 * Indicate Hexenworld
 *
 * Revision 1.1  2001/12/02 00:23:32  theoddone33
 * Add files for client
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
