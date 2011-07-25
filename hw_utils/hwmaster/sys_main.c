/*
	sys_main.c
	main loop and system interface

	$Id: sys_main.c,v 1.50 2009-04-29 07:49:28 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"

#include "defs.h"

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
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <mmsystem.h>
#include "io_msvc.h"
#endif	/* WINDOWS */

#if defined(PLATFORM_WINDOWS)
#define	TIME_WRAP_VALUE	(~(DWORD)0)
static DWORD		starttime;
#endif	/* PLATFORM_WINDOWS */

#if defined(PLATFORM_UNIX)
static double		starttime;
static qboolean		first = true;

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

char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int		textlen;

#ifdef PLATFORM_WINDOWS
	int		c;

	// read a line out
	while (_kbhit())
	{
		c = _getch();
		_putch (c);
		if (c == '\r')
		{
			con_text[textlen] = '\0';
			_putch ('\n');
			textlen = 0;
			return con_text;
		}
		if (c == 8)
		{
			if (textlen)
			{
				_putch (' ');
				_putch (c);
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
			printf("\nConsole input too long!\n");
			break;
		}
	}

	return NULL;

#else	/* UNIX: */

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
			printf("\nConsole input too long!\n");
			break;
		}
	}

	return NULL;
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

int Sys_mkdir (const char *path, qboolean crash)
{
	int rc = mkdir (path, 0777);
	if (rc != 0 && errno == EEXIST)
		rc = 0;
	if (rc != 0 && crash)
		Sys_Error("Unable to create directory %s", path);
	return rc;
}

static int Sys_GetUserdir (char *dst, size_t dstsize)
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

	if (strlen(home_dir) + strlen(HWM_USERDIR) + 12 > dstsize)
		return 1;

	q_snprintf (dst, dstsize, "%s/%s", home_dir, HWM_USERDIR);
	return Sys_mkdir(dst, false);
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
	if (Sys_GetUserdir(userdir, sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");
	printf ("Userdir: %s\n", userdir);
	if (q_snprintf(filters_file, sizeof(filters_file), "%s/filters.ini", userdir) >= (int)sizeof(filters_file))
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

