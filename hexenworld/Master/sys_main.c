/*
	sys_main.c
	main loop and system interface

	$Id: sys_main.c,v 1.40 2007-10-13 06:33:52 sezero Exp $
*/

// whether to use the password file to determine
// the path to the home directory
#define USE_PASSWORD_FILE	1

#include "defs.h"
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif
#include <limits.h>

#if defined(PLATFORM_UNIX)
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#if USE_PASSWORD_FILE
#include <pwd.h>
#endif
#endif

#if defined(PLATFORM_WINDOWS)
#include <io.h>
#include <conio.h>
#include <winsock.h>
#include <mmsystem.h>
#endif

#if defined(PLATFORM_WINDOWS)
/*
#define	TIME_WRAP_VALUE	LONG_MAX
*/
#define	TIME_WRAP_VALUE	(~(DWORD)0)
static DWORD		starttime;
#endif	/* PLATFORM_WINDOWS */

#if defined(PLATFORM_UNIX)
static double		starttime;
static qboolean		first = true;

static int	do_stdin = 1;
static qboolean	stdin_ready;
static char	userdir[MAX_OSPATH];

extern char	filters_file[MAX_OSPATH];
#endif	/* PLATFORM_UNIX */


//=============================================================================

void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	q_vsnprintf (text, sizeof (text), error,argptr);
	va_end (argptr);

	printf ("\nFATAL ERROR: %s\n\n", text);

	exit (1);
}

void Sys_Quit (void)
{
	exit (0);
}


//=============================================================================

/* sys_dead_sleep: When set, the server gets NO cpu if no clients are connected
   and there's no other activity. *MIGHT* cause problems with some mods. */
static qboolean sys_dead_sleep	= 0;

int Sys_CheckInput (int ns)
{
	fd_set		fdset;
	int		res;
	struct timeval	_timeout;
	struct timeval	*timeout = 0;

	_timeout.tv_sec = 0;
#ifdef PLATFORM_WINDOWS
	_timeout.tv_usec = ns < 0 ? 0 : 100;
#else
	_timeout.tv_usec = ns < 0 ? 0 : 10000;
#endif
	// select on the net socket and stdin
	// the only reason we have a timeout at all is so that if the last
	// connected client times out, the message would not otherwise
	// be printed until the next event.
	FD_ZERO (&fdset);

#ifndef PLATFORM_WINDOWS
	if (do_stdin)
		FD_SET (0, &fdset);
#endif
	if (ns >= 0)
		FD_SET (ns, &fdset);

	if (!sys_dead_sleep)
		timeout = &_timeout;

	res = select (q_max(ns, 0) + 1, &fdset, NULL, NULL, timeout);
	if (res == 0 || res == -1)
		return 0;

#ifndef PLATFORM_WINDOWS
	stdin_ready = FD_ISSET (0, &fdset);
#endif
	return 1;
}

char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int		textlen;

#ifdef PLATFORM_WINDOWS
	int		c;

	// read a line out
	while (_kbhit ())
	{
		c = _getch ();
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
		if (textlen == sizeof (con_text))
			textlen = 0;
	}

	return NULL;
#else
	if (!stdin_ready || !do_stdin)
		return NULL;	// the select didn't say it was ready
	stdin_ready = false;

	textlen = read (0, con_text, sizeof (con_text));
	if (textlen == 0)
	{	// end of file
		do_stdin = 0;
		return NULL;
	}
	if (textlen < 1)
		return NULL;
	con_text[textlen - 1] = 0;	// rip off the \n and terminate

	return con_text;
#endif
}

double Sys_DoubleTime (void)
{
#ifdef PLATFORM_WINDOWS
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
#else
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
#endif
}

#ifdef PLATFORM_UNIX

int Sys_mkdir (const char *path)
{
	int rc;

	rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;

	return rc;
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

	if (strlen(home_dir) + strlen(HWM_USERDIR) + 12 > path_len)
		return 1;

	q_snprintf (buff, path_len, "%s/%s", home_dir, HWM_USERDIR);
	return Sys_mkdir(buff);
}
#endif


//=============================================================================

int main (int argc, char **argv)
{
	int t;

	if (argc > 1)
	{
		for (t = 1; t < argc; t++)
		{
			if ( !(strcmp(argv[t], "-h")) || !(strcmp(argv[t], "-help")) ||
			     !(strcmp(argv[t], "--help")) || !(strcmp(argv[t], "-?")) )
			{
				printf("HexenWorld master server %s\n\n", VER_HWMASTER_STR);
				printf("Usage:     hwmaster [-port xxxxx]\n");
				printf("See the documentation for details\n\n");
				exit(0);
			}
			else if ( !(strcmp(argv[t], "-v")) || !(strcmp(argv[t], "-version")) ||
				  !(strcmp(argv[t], "--version")) )
			{
				printf("hwmaster %d.%d.%d\n", VER_HWMASTER_MAJ, VER_HWMASTER_MID, VER_HWMASTER_MIN);
				exit(0);
			}
		}
	}

	com_argv = argv;
	com_argc = argc;

#ifdef PLATFORM_UNIX
// userdir stuff
	if (Sys_GetUserdir(userdir,sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");
	printf ("Userdir: %s\n", userdir);
	if (q_snprintf(filters_file, sizeof(filters_file), "%s/filters.ini", userdir) >= sizeof(filters_file))
		Sys_Error ("Insufficient string buffer size");
#endif

#ifdef PLATFORM_WINDOWS
	timeBeginPeriod (1);	/* 1 ms timer precision */
	starttime = timeGetTime ();
#endif

	Cbuf_Init();
	Cmd_Init ();

	SV_InitNet();

	printf ("Exe: "__TIME__" "__DATE__"\n");

	printf("======== HW master %d.%d.%d initialized ========\n\n",
		VER_HWMASTER_MAJ, VER_HWMASTER_MID, VER_HWMASTER_MIN);

	while (1)
	{
		SV_Frame();
	}

	return 0;
}

