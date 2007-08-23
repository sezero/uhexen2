/*
	sys_win.c
	Win32 system interface code

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/sys_win.c,v 1.24 2007-08-23 19:45:30 sezero Exp $
*/

#include "quakedef.h"
#include "debuglog.h"
#include <sys/types.h>
#include <limits.h>
#include <winsock.h>
#include <mmsystem.h>
#include <errno.h>
#include <io.h>
#include <direct.h>
#include <conio.h>


// heapsize: minimum 8 mb, standart 16 mb, max is 32 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x0800000
#define STD_MEM_ALLOC	0x1000000
#define MAX_MEM_ALLOC	0x2000000

cvar_t		sys_nostdout = {"sys_nostdout", "0", CVAR_NONE};

qboolean		isDedicated = true;	/* compatibility */

/*
#define	TIME_WRAP_VALUE	(~(DWORD)0)
*/
#define	TIME_WRAP_VALUE	LONG_MAX
static DWORD		starttime;


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

#define ERROR_PREFIX	"\nFATAL ERROR: "
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[MAX_PRINTMSG];

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (con_debuglog)
	{
		LOG_Print (ERROR_PREFIX);
		LOG_Print (text);
		LOG_Print ("\n\n");
	}

	Host_Shutdown ();

	printf (ERROR_PREFIX "%s\n\n", text);

#ifdef DEBUG_BUILD
	getch();
#endif

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
	int		c;

	// read a line out
	while (_kbhit())
	{
		c = _getch();
		putch (c);
		if (c == '\r')
		{
			con_text[textlen] = 0;
			putch ('\n');
			textlen = 0;
			return con_text;
		}
		if (c == 8)
		{
			if (textlen)
			{
				putch (' ');
				putch (c);
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
	Sleep (1);
}


static void PrintVersion (void)
{
#if HOT_VERSION_BETA
	printf ("Hammer of Thyrion, %s-%s (%s) pre-release\n", HOT_VERSION_STR, HOT_VERSION_BETA_STR, HOT_VERSION_REL_DATE);
#else
	printf ("Hammer of Thyrion, release %s (%s)\n", HOT_VERSION_STR, HOT_VERSION_REL_DATE);
#endif
	printf ("Hexen II dedicated server %4.2f (%s)\n", ENGINE_VERSION, PLATFORM_STRING);
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

/*
===============================================================================

MAIN

===============================================================================
*/
static quakeparms_t	parms;
static char	cwd[MAX_OSPATH];

int main (int argc, char **argv)
{
	int			i;
	double		time, oldtime;
	char		*tmp;

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
				  !(strcmp(argv[i], "--help")) || !(strcmp(argv[i], "-?")) )
			{
				PrintHelp(argv[0]);
				exit (0);
			}
		}
	}

	memset (cwd, 0, sizeof(cwd));
	if ( _getcwd (cwd, sizeof(cwd)-1) == NULL )
		Sys_Error ("Couldn't determine current directory");

	tmp = cwd;
	while (*tmp != 0)
		tmp++;
	while (*tmp == 0)
	{
		--tmp;
		if (*tmp == '\\' || *tmp == '/')
			*tmp = 0;
	}

	/* initialize the host params */
	memset (&parms, 0, sizeof(parms));
	parms.basedir = cwd;
	parms.userdir = cwd;	/* no userdir on win32 */
	parms.argc = argc;
	parms.argv = argv;
	host_parms = &parms;

	LOG_Init (&parms);

	Sys_Printf("basedir is: %s\n", parms.basedir);
	Sys_Printf("userdir is: %s\n", parms.userdir);

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

	timeBeginPeriod (1);	/* 1 ms timer precision */
	starttime = timeGetTime ();

	Host_Init();

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

