/*
	sys_main.c
	main loop and system interface

	$Id$
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"

#include "defs.h"

#include <windows.h>
#include <io.h>
#include <conio.h>
#include <mmsystem.h>
#include "io_msvc.h"

#define	TIME_WRAP_VALUE	(~(DWORD)0)
static DWORD		starttime;


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
	static int	textlen;
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
		if (textlen < (int) sizeof(con_text))
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
}

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

	timeBeginPeriod (1);	/* 1 ms timer precision */
	starttime = timeGetTime ();

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

