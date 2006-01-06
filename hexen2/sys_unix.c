/*
	sys_unix.c
	Unix system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sys_unix.c,v 1.45 2006-01-06 12:41:41 sezero Exp $
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
//#include "SDL.h"
#include "SDL_version.h"


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

// minimum required SDL version
#define	SDL_MIN_X	1
#define	SDL_MIN_Y	2
#if defined(GLQUAKE) && SDL_PATCHLEVEL > 5
// for SDL_GL_MULTISAMPLESAMPLES
#define	SDL_MIN_Z	6
//#else if defined(SDL_BUTTON_WHEELUP)
//#define	SDL_MIN_Z	5
#else
#define	SDL_MIN_Z	0
#endif

static double		curtime = 0.0;
static double		lastcurtime = 0.0;
qboolean		isDedicated;

void Sys_InitFloatTime (void);

//=============================================================================


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
	int	len;

	if (finddir)
		Sys_Error ("Sys_FindFirst without FindClose");

	finddir = opendir (path);
	if (!finddir)
		return NULL;

	len = strlen (pattern);
	findpattern = malloc (len + 1);
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
void Sys_Init (void)
{
	Sys_SetFPCW();
}


void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];
//	double		starttime;

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
        struct timeval  tp;
        struct timezone tzp;
        static int              secbase;

        gettimeofday(&tp, &tzp);

        if (!secbase) {
                secbase = tp.tv_sec;
                return tp.tv_usec/1000000.0;
        }

        return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}


/*
================
Sys_InitFloatTime
================
*/
void Sys_InitFloatTime (void)
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


char *Sys_ConsoleInput (void)
{
	static char	text[256];
	static int	len;
	char		c;
	fd_set		set;
	struct timeval	timeout;

	FD_ZERO (&set);
	FD_SET (0, &set);	/* 0 is stdin?? */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	while (select (1, &set, NULL, NULL, &timeout))
	{
		read (0, &c, 1);
		if (c == '\n' || c == '\r')
		{
			text[len] = 0;
			len = 0;
			return text;
		}
		else if (c == 8)
		{
			if (len)
			{
				len--;
				text[len] = 0;
			}
			continue;
		}
		text[len] = c;
		len++;
		text[len] = 0;
		if (len == sizeof(text))
			len = 0;
	}

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
}

int Sys_GetUserdir (char *buff, unsigned int len)
{
	if (getenv("HOME") == NULL)
		return 1;

/* O.S:	We keep the userdir (and host_parms.userdir) as ~/.hexen2
	here.  We'll change com_userdir in COM_InitFilesystem()
	depending on H2MP, H2W and/or -game cmdline arg, instead.
   S.A:	Now using $HOME istead of the passwd struct */

	if (strlen(getenv("HOME")) + strlen(AOT_USERDIR) + 2 > len)
		return 1;

	sprintf (buff, "%s/%s", getenv("HOME"), AOT_USERDIR);
	return Sys_mkdir(buff);
}

void PrintVersion (void)
{
	printf ("Hammer of Thyrion, release " HOT_VERSION_STR "\n");
	printf ("running on " ENGINE_NAME " engine %4.2f (" VERSION_PLATFORM ")\n",
		ENGINE_VERSION);
}

void PrintHelp(char *name)
{
	PrintVersion();
	printf ("\n");
	printf ("Please send bug reports or patches to:\n");
	printf ("     Steven Atkinson  <stevenaaus@users.sourceforge.net>\n");
	printf ("     Ozkan Sezer      <sezero@users.sourceforge.net>\n");
	printf ("Visit http://sf.net/projects/uhexen2/ for more info.\n");
	printf ("\n");
	printf ("Usage: %s [options]\n", name);
	printf ("     [-v | --version]        Display the game version\n");
	printf ("     [-f | --fullscreen]     Run the game fullscreen\n");
	printf ("     [-width X [-height Y]]  Select screen size\n");
#ifdef GLQUAKE
	printf ("     [-bpp]                  Depth for GL fullscreen mode\n");
	printf ("     [-vsync]                Enable sync with monitor refresh\n");
	printf ("     [-g | --gllibrary]      Select 3D rendering library\n");
	printf ("     [-fsaa N]               Enable N sample antialiasing\n");
	printf ("     [-paltex]               Enable 8-bit GL extensions\n");
	printf ("     [-nomtex]               Disable multitexture detection/usage\n");
#endif
	printf ("     [-s | --nosound]        Run the game without sound\n");
	printf ("     [-sndalsa]              Use ALSA sound (alsa > 1.0.1)\n");
	printf ("     [-sndsdl]               Use SDL sound\n");
	printf ("     [-nomouse]              Disable mouse usage\n");
	printf ("     [-listen N]             Enable multiplayer with max. N players\n");
	printf ("     [-heapsize Bytes]       Heapsize (memory to allocate)\n");
}

int main(int argc, char *argv[])
{
	quakeparms_t	parms;
	double	time, oldtime, newtime;
	char	cwd[MAX_OSPATH];
	char	userdir[MAX_OSPATH];
	char	binary_name[MAX_OSPATH];
	int	t;
	const SDL_version *sdl_version;

	if (!(getcwd (cwd, sizeof(cwd))))
		Sys_Error ("Couldn't determine current directory");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	if (Sys_GetUserdir(userdir,sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");

	parms.basedir = cwd;
	parms.cachedir = NULL;
	parms.userdir = userdir;

	memset(binary_name,0,sizeof(binary_name));
	strncpy(binary_name,argv[0],sizeof(binary_name));

	parms.argc = 1;
	argv[0] = "";

	parms.argv = argv;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	if (COM_CheckParm ("-help") || COM_CheckParm ("--help") ||
	    COM_CheckParm ("-h")    || COM_CheckParm ("-?"))
	{
		PrintHelp(binary_name);
		exit (0);
	}

	if (COM_CheckParm ("-v") || COM_CheckParm ("-version") || COM_CheckParm ("--version"))
	{
		PrintVersion();
		exit (0);
	}

	Sys_Printf("userdir is: %s\n",userdir);

	isDedicated = (COM_CheckParm ("-dedicated") != 0);

	sdl_version = SDL_Linked_Version();
	Sys_Printf("Found SDL version %i.%i.%i\n",sdl_version->major,sdl_version->minor,sdl_version->patch);
	if (!isDedicated &&
	    SDL_VERSIONNUM(sdl_version->major,sdl_version->minor,sdl_version->patch) < SDL_VERSIONNUM(SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z))
	{	//reject running under SDL versions < 1.2.6
		printf("You need at least version %i.%i.%i of SDL to run this game\n",SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z);
		exit(0);
	}

	if (isDedicated)
		parms.memsize = MIN_MEM_ALLOC;
	else
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

// because sound is off until we become active
	//S_BlockSound ();

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

	oldtime = Sys_DoubleTime ();

    /* main window message loop */
	while (1)
	{
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		if (isDedicated)
		{
			while (time < sys_ticrate.value )
			{
				Sys_Sleep();
				newtime = Sys_DoubleTime ();
				time = newtime - oldtime;
			}
		}

		Host_Frame (time);
		oldtime = newtime;
	}

}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.44  2006/01/06 12:15:01  sezero
 * added a simplified findfirst/findnext implementation: When given a
 * directory and a wildcard match pattern, they will find/report the
 * matches one after other. Unix implementation uses readdir()/fnmatch(),
 * win32 uses FindFirstFile native functions.
 * Sys_FindFirstFile and Sys_FindNextFile return filenames only, not a
 * dirent struct, therefore there is no attribute matching. this is
 * enough for what we presently need in this engine. may be improved in
 * future.
 *
 * Revision 1.43  2005/12/04 11:14:38  sezero
 * the big vsnprintf patch
 *
 * Revision 1.42  2005/10/25 17:14:23  sezero
 * added a STRINGIFY macro. unified version macros. simplified version
 * printing. simplified and enhanced version watermark print onto console
 * background. added HoT lines to the quit menu (shameless plug)
 *
 * Revision 1.41  2005/09/28 06:06:38  sezero
 * killed the cvar sys_delay
 *
 * Revision 1.40  2005/09/20 21:19:45  sezero
 * Sys_Quit and Sys_Error clean-up: VID_SetDefaultMode, VID_ForceLockState and
 * VID_ForceUnlockedAndReturnState are history. Host_Shutdown is called before
 * quit/error messages. Placed SDL_UnlockSurface() and MGL_endDirectAccess()
 * to VID_Shutdown, just in case. Added the word "HexenWorld" to win32 version
 * of hexenworld error window label. Took care of some accidentally repeated
 * code. "Fatalized" and added extra linefeeds to the sys_error messages.
 *
 * Revision 1.39  2005/09/20 21:17:25  sezero
 * Moved VERSION_PLATFORM and id386 defines to sys.h, where they belong.
 *
 * Revision 1.38  2005/08/12 09:21:08  sezero
 * loosened SDL version restrictions depending on the SDL version
 * on the build system. will issue a warning if less than 1.2.6
 *
 * Revision 1.37  2005/08/10 23:19:26  sezero
 * slight tweaks
 *
 * Revision 1.36  2005/08/07 10:59:06  sezero
 * killed the Sys_FileTime crap. now using standart access() function.
 *
 * Revision 1.35  2005/07/30 15:19:13  sezero
 * insignificant cosmetics
 *
 * Revision 1.34  2005/07/23 22:22:09  sezero
 * unified the common funcntions for hexen2-hexenworld
 *
 * Revision 1.33  2005/07/22 17:06:20  sezero
 * whitespace cleanup
 *
 * Revision 1.32  2005/07/09 11:53:40  sezero
 * moved the local unix version of strlwr to zone.c, its only user.
 *
 * Revision 1.31  2005/07/09 07:02:09  sezero
 * tiny clean-up
 *
 * Revision 1.30  2005/07/09 07:00:03  sezero
 * SDL version refusal should not happen for dedicated servers
 *
 * Revision 1.29  2005/06/15 10:30:02  sezero
 * fix a compile problem on win32 (filelength clash with io.h)
 * and keep the names consistent throughout the tree
 *
 * Revision 1.28  2005/06/15 09:44:54  sezero
 * more endianness stuff
 *
 * Revision 1.27  2005/06/12 07:28:51  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.26  2005/06/01 14:13:21  sezero
 * SDL version >= 1.2.6 is now required. Updated the READMEs
 * and added a version check at startup.
 *
 * Revision 1.25  2005/05/30 09:44:19  sezero
 * updated commandline help display
 *
 * Revision 1.24  2005/05/29 08:38:12  sezero
 * get rid of the silly func name difference
 *
 * Revision 1.23  2005/05/26 09:38:06  sezero
 * updated help display
 *
 * Revision 1.22  2005/05/20 12:34:44  sezero
 * removed some windows left-overs from sys_unix
 *
 * Revision 1.21  2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * Revision 1.20  2005/04/30 08:34:52  sezero
 * removed pointless global decleration of starttime in sys_unix.c which
 * kills a shadowed decleration warning, as well.
 *
 * Revision 1.19  2005/04/15 20:24:21  sezero
 * added working console input (from hexenworld server).
 * disabled console input in non-dedicated mode.
 *
 * Revision 1.18  2005/04/14 07:36:15  sezero
 * -? arg is back (Steve likes it ;)
 *
 * Revision 1.17  2005/04/13 12:22:41  sezero
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
 * Revision 1.16  2005/03/03 19:48:40  sezero
 * More bits from Steven:
 * - increase MAX_OSPATH to 256
 * - Sys_Mkdir is now int. Its return code should be checked in other places, too.
 * - Sys_GetUserdir now uses $HOME instead of the passwd struct
 *
 * Revision 1.15  2005/03/03 17:04:59  sezero
 * remove h2mp dependency from length check, seems to serve no purpose (from Steve)
 *
 * Revision 1.14  2005/03/03 17:03:39  sezero
 * - sys_unix.c cleanup: remove dead and/or win32 code
 * - remove unreached return from main (it should have been 0, btw. from Steve)
 *
 * Revision 1.13  2005/02/20 13:38:29  sezero
 * add the new sound options to the help messages
 *
 * Revision 1.12  2005/02/06 15:03:10  sezero
 * move resource.h to ./win_stuff/
 *
 * Revision 1.11  2005/01/18 11:29:18  sezero
 * - Fix userdirs to work correctly against the -game arg
 * - Added a "qboolean adduser" arg to COM_AddGameDir()  (useful esp. in
 *   hexenworld):  com_userdir evolves gradually. Every search path added
 *   has a consequence of additon of (multiple) unnecessary userpaths.
 *   Avoid it where we can. (original aot also added the very same userdir
 *   multiple times)
 * - Changed all instances of "id1" to "data1"
 *
 * Revision 1.10  2004/12/22 21:50:16  sezero
 * play with version and help display
 *
 * Revision 1.9  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.8  2004/12/18 14:08:08  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.7  2004/12/12 14:38:18  sezero
 * steven fixed the mouse again ;)
 *
 * Revision 1.6  2004/12/12 14:14:43  sezero
 * style changes to our liking
 *
 * Revision 1.5  2004/12/05 10:52:18  sezero
 * Sync with Steven, 2004-12-04 :
 *  Fix the "Old Mission" menu PoP
 *  Also release the windowed mouse on pause
 *  Heapsize is now 32768 default
 *  The final splash screens now centre the messages properly
 *  Add more mods to the video mods table
 *  Add the docs folder and update it
 *
 * Revision 1.4  2004/11/29 16:31:58  sezero
 * Updated
 *
 * Revision 1.3  2004/11/29 16:22:23  sezero
 * Grrrr......
 *
 * Revision 1.2  2004/11/28 00:58:08  sezero
 *
 * Commit Steven's changes as of 2004.11.24:
 *
 * * Rewritten Help/Version message(s)
 * * Proper fullscreen mode(s) for OpenGL.
 * * Screen sizes are selectable with "-width" and "-height" options.
 * * Mouse grab in window modes , which is released when menus appear.
 * * Interactive video modes in software game disabled.
 * * Replaced Video Mode menu with a helpful message.
 * * New menu items for GL Glow, Chase mode, Draw Shadows.
 * * Changes to initial cvar_t variables:
 *      r_shadows, gl_other_glows, _windowed_mouse,
 *
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
