/*
	sys_unix.c
	Unix system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sys_unix.c,v 1.98 2007-10-13 07:55:32 sezero Exp $
*/

#include "quakedef.h"
#include "userdir.h"
#include "debuglog.h"

#include <errno.h>
#include <unistd.h>
#ifdef __MACOSX__
#include <libgen.h>	/* for dirname and basename */
#endif
#if USE_PASSWORD_FILE && DO_USERDIRS
#include <pwd.h>
#endif	/* USE_PASSWORD_FILE */
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <dirent.h>
#include <fnmatch.h>
#include "sdl_inc.h"


// heapsize: minimum 16mb, standart 32 mb, max is 96 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x1000000
#define STD_MEM_ALLOC	0x2000000
#define MAX_MEM_ALLOC	0x6000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};

qboolean		isDedicated;
static double		starttime;
static qboolean		first = true;


/*
===============================================================================

FILE IO

===============================================================================
*/

int Sys_mkdir (const char *path)
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
static char		*findpath, *findpattern;

char *Sys_FindFirstFile (const char *path, const char *pattern)
{
	size_t	tmp_len;

	if (finddir)
		Sys_Error ("Sys_FindFirst without FindClose");

	finddir = opendir (path);
	if (!finddir)
		return NULL;

	tmp_len = strlen (pattern);
	findpattern = (char *) Z_Malloc (tmp_len + 1, Z_MAINZONE);
//	if (!findpattern)
//		return NULL;
	strcpy (findpattern, pattern);
	findpattern[tmp_len] = '\0';
	tmp_len = strlen (path);
	findpath = (char *) Z_Malloc (tmp_len + 1, Z_MAINZONE);
//	if (!findpath)
//		return NULL;
	strcpy (findpath, path);
	findpath[tmp_len] = '\0';

	return Sys_FindNextFile();
}

char *Sys_FindNextFile (void)
{
	struct stat	test;

	if (!finddir)
		return NULL;

	do {
		finddata = readdir(finddir);
		if (finddata != NULL)
		{
			if (!fnmatch (findpattern, finddata->d_name, FNM_PATHNAME))
			{
				if ( (stat(va("%s/%s", findpath, finddata->d_name), &test) == 0)
							&& S_ISREG(test.st_mode) )
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
	if (findpath != NULL)
		Z_Free (findpath);
	if (findpattern != NULL)
		Z_Free (findpattern);
	finddir = NULL;
	findpath = NULL;
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
#if id386 && !defined(GLQUAKE)
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	int		r;
	unsigned long	endaddr = startaddr + length;
#if !defined(__QNX__)
# if	1
	long		psize = sysconf (_SC_PAGESIZE);
# else
	int		psize = getpagesize();
# endif
	startaddr &= ~(psize - 1);
	endaddr = (endaddr + psize - 1) & ~(psize - 1);
#endif
	// systems with mprotect but not getpagesize (or similar) probably don't
	// need to page align the arguments to mprotect (eg, QNX)
	r = mprotect ((char *) startaddr, endaddr - startaddr, PROT_WRITE | PROT_READ | PROT_EXEC);

	if (r < 0)
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
/* do we really need these with opengl ?? */
	Sys_SetFPCW();
}


#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];

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

	fprintf(stderr, ERROR_PREFIX "%s\n\n", text);

	exit (1);
}

void Sys_PrintTerm (const char *msgtxt)
{
	unsigned char		*p;

	if (sys_nostdout.integer)
		return;

	for (p = (unsigned char *) msgtxt; *p; p++)
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
	struct timeval	tp;
	double		now;

	gettimeofday (&tp, NULL);

	now = tp.tv_sec + tp.tv_usec / 1e6;

	if (first)
	{
		first = false;
		starttime = now;
		return 0.0;
	}

	return now - starttime;
}


/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int	textlen;
	char		c;
	fd_set		set;
	struct timeval	timeout;

	FD_ZERO (&set);
	FD_SET (0, &set);	// stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	while (select (1, &set, NULL, NULL, &timeout))
	{
		read (0, &c, 1);
		if (c == '\n' || c == '\r')
		{
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		else if (c == 8)
		{
			if (textlen)
			{
				textlen--;
				con_text[textlen] = '\0';
			}
			continue;
		}
		con_text[textlen] = c;
		textlen++;
		if (textlen < sizeof(con_text))
			con_text[textlen] = '\0';
		else
		{
		// buffer is full
			textlen = 0;
			con_text[0] = '\0';
			Sys_PrintTerm("\nConsole input too long!\n");
			break;
		}
	}

	return NULL;
}

void Sys_Sleep (void)
{
	usleep(1);
}

void Sys_SendKeyEvents (void)
{
	IN_SendKeyEvents();
}

#if DO_USERDIRS
static int Sys_GetUserdir (char *buff, size_t path_len)
{
	char		*home_dir = NULL;
#if USE_PASSWORD_FILE
	struct passwd	*pwent;

	pwent = getpwuid( getuid() );
	if (pwent == NULL)
		perror("getpwuid");
	else
		home_dir = pwent->pw_dir;
#endif
	if (home_dir == NULL)
		home_dir = getenv("HOME");
	if (home_dir == NULL)
		return 1;

//	what would be a maximum path for a file in the user's directory...
//	$HOME/AOT_USERDIR/game_dir/dirname1/dirname2/dirname3/filename.ext
//	still fits in the MAX_OSPATH == 256 definition, but just in case :
	if (strlen(home_dir) + strlen(AOT_USERDIR) + 50 > path_len)
		return 1;

	q_snprintf (buff, path_len, "%s/%s", home_dir, AOT_USERDIR);
	return Sys_mkdir(buff);
}
#endif	/* DO_USERDIRS */

static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
	printf ("running on %s engine %4.2f (%s)\n", ENGINE_NAME, ENGINE_VERSION, PLATFORM_STRING);
	printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

#define _SND_SYS_MACROS_ONLY
#include "snd_sys.h"
static char *help_strings[] = {
	"     [-v | --version]        Display version information",
#ifndef DEMOBUILD
#   if defined(H2MP)
	"     [-noportals]            Disable the mission pack support",
#   else
	"     [-portals | -h2mp ]     Run the Portal of Praevus mission pack",
#   endif
#endif
	"     [-w | -window ]         Run the game windowed",
	"     [-f | -fullscreen]      Run the game fullscreen",
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
#if HAVE_OSS_SOUND
	"     [-sndoss]               Use OSS sound",
#endif
#if HAVE_ALSA_SOUND
	"     [-sndalsa]              Use ALSA sound (alsa > 1.0.1)",
#endif
#if HAVE_SUN_SOUND
	"     [-sndsun | -sndbsd]     Use SUN / BSD sound",
#endif
#if HAVE_SDL_SOUND
	"     [-sndsdl]               Use SDL sound",
#endif
#endif	//  SOUND_NUMDRIVERS
#endif	// _NO_SOUND
	"     [-nomouse]              Disable mouse usage",
	"     [-listen N]             Enable multiplayer with max N players",
	"     [-heapsize Bytes]       Heapsize (memory to allocate)",
	NULL
};

static void PrintHelp(char *name)
{
	int i = 0;

	printf ("Usage: %s [options]\n", name);
	while (help_strings[i])
	{
		printf (help_strings[i]);
		printf ("\n");
		i++;
	}
	printf ("\n");
}

#ifdef __MACOSX__
/*
=================
Sys_StripAppBundle

If passed dir is suffixed with the directory structure of a Mac OS X
.app bundle, the .app directory structure is stripped off the end and
the result is returned. If not, dir is returned untouched. Taken from
the quake3 project at icculus.org.

For Mac OS X, we package the game like this:

	Hexen II	( --> the holder directory)
	|
	 - Hexen II gl.app (bundle dir for the opengl application)
	|  |
	|   - Contents
	|  |  |
	|  |   - MacOS	(the actual binary resides here)
	|  |
	|   - Resources (icons here)
	|
	 - data1	( --> game data directories)
	|
	 - portals	( ditto)

=================
*/
static char *Sys_StripAppBundle (char *dir)
{
	static char	osx_path[MAX_OSPATH];

	q_strlcpy (osx_path, dir, sizeof(osx_path));
	if (strcmp(basename(osx_path), "MacOS"))
		return dir;
	q_strlcpy (osx_path, dirname(osx_path), sizeof(osx_path));
	if (strcmp(basename(osx_path), "Contents"))
		return dir;
	q_strlcpy (osx_path, dirname(osx_path), sizeof(osx_path));
	if (!strstr(basename(osx_path), ".app"))
		return dir;
	q_strlcpy (osx_path, dirname(osx_path), sizeof(osx_path));
	return osx_path;
}
#endif	/* __MACOSX__ */

/*
===============================================================================

MAIN

===============================================================================
*/
static Uint8		appState;
static quakeparms_t	parms;
static char	cwd[MAX_OSPATH];
#if DO_USERDIRS
static char	userdir[MAX_OSPATH];
#endif

int main(int argc, char *argv[])
{
	int			i;
	double		time, oldtime, newtime;
	char		*tmp;
	const SDL_version *sdl_version;

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
#ifdef __MACOSX__
	if ( realpath(argv[0], cwd) == NULL )
	{
		if ( getcwd (cwd, sizeof(cwd)-1) == NULL )
			Sys_Error ("Couldn't determine current directory");
	}
	else
	{
		// strip off the binary name
		q_strlcpy (cwd, dirname(cwd), sizeof(cwd));
	}

	tmp = Sys_StripAppBundle(cwd);
	q_strlcpy (cwd, tmp, sizeof(cwd));
#else
	if ( getcwd (cwd, sizeof(cwd)-1) == NULL )
		Sys_Error ("Couldn't determine current directory");

	tmp = cwd;
	while (*tmp != 0)
		tmp++;
	while (*tmp == 0)
	{
		--tmp;
		if (*tmp == '/')
			*tmp = 0;
	}
#endif

#if DO_USERDIRS
	memset (userdir, 0, sizeof(userdir));
	if (Sys_GetUserdir(userdir,sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");
#endif

	/* initialize the host params */
	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
#if DO_USERDIRS
	parms.userdir = userdir;
#else
	parms.userdir = cwd;
#endif
	parms.argc = argc;
	parms.argv = argv;
	host_parms = &parms;

	LOG_Init (&parms);

	Sys_Printf("basedir is: %s\n", parms.basedir);
	Sys_Printf("userdir is: %s\n", parms.userdir);

	isDedicated = (COM_CheckParm ("-dedicated") != 0);

	sdl_version = SDL_Linked_Version();
	Sys_Printf("Found SDL version %i.%i.%i\n",sdl_version->major,sdl_version->minor,sdl_version->patch);
	if (!isDedicated &&
	    SDL_VERSIONNUM(sdl_version->major,sdl_version->minor,sdl_version->patch) < SDL_REQUIREDVERSION)
	{	//reject running under SDL versions older than what is stated in sdl_inc.h
		Sys_Printf("You need at least v%d.%d.%d of SDL to run this game\n",SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z);
		exit (1);
	}

	if (isDedicated)
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

	Sys_Init ();

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
				Sys_Sleep();
				newtime = Sys_DoubleTime ();
				time = newtime - oldtime;
			}
		}
		else
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
		}

		Host_Frame (time);
		oldtime = newtime;
	}

	return 0;
}

