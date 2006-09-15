/*
	sys_unix.c
	Unix system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/sys_unix.c,v 1.3 2006-09-15 09:20:12 sezero Exp $
*/

#include "quakedef.h"

// whether to use the password file to determine
// the path to the home directory
#define USE_PASSWORD_FILE	0

#include <errno.h>
#include <unistd.h>
#if USE_PASSWORD_FILE
#include <pwd.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <dirent.h>
#include <fnmatch.h>


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
	size_t	pattern_len;

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
			con_text[textlen] = 0;
			textlen = 0;
			return con_text;
		}
		else if (c == 8)
		{
			if (textlen)
			{
				textlen--;
				con_text[textlen] = 0;
			}
			continue;
		}
		con_text[textlen] = c;
		textlen++;
		con_text[textlen] = 0;
		if (textlen == sizeof(con_text))
			textlen = 0;
	}

	return NULL;
}

void Sys_Sleep (void)
{
	usleep(1);
}

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

	snprintf (buff, path_len, "%s/%s", home_dir, AOT_USERDIR);
	return Sys_mkdir(buff);
}

static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
	printf ("Hexen II dedicated server %4.2f (%s)\n", ENGINE_VERSION, VERSION_PLATFORM);
	printf ("More info / sending bug reports:  http://uhexen2.sourceforge.net\n");
}

static char *help_strings[] = {
	"     [-v | --version]        Display version information",
#ifndef DEMOBUILD
#   if defined(H2MP)
	"     [-noportals]            Disable the mission pack support",
#   else
	"     [-portals | -h2mp ]     Run the Portal of Praevus mission pack",
#   endif
#endif
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

int main(int argc, char *argv[])
{
	quakeparms_t	parms;
	double	time, oldtime;
	char	cwd[MAX_OSPATH];
	char	userdir[MAX_OSPATH];
	int	t;

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
	parms.userdir = userdir;

	parms.argc = 1;
	argv[0] = "";

	parms.argv = argv;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	Sys_Printf("userdir is: %s\n",userdir);

	parms.memsize = MIN_MEM_ALLOC;

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

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
		time = Sys_DoubleTime ();

		if (time - oldtime < sys_ticrate.value )
		{
			Sys_Sleep();
			continue;
		}

		Host_Frame (time - oldtime);
		oldtime = time;
	}

	return 0;
}

