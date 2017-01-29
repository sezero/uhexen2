/* sys_os2.c -- main loop and system interface
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"

#include "defs.h"

#define INCL_DOS
#define INCL_DOSERRORS
#ifdef __EMX__
#define INCL_KBD
#define INCL_VIO
#endif
#include <os2.h>
#include <conio.h>


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

#ifdef __EMX__
int putch (int c) {
	char ch = c;
	VioWrtTTY(&ch, 1, 0);
	return c;
}
int kbhit (void) {
	KBDKEYINFO k;
	if (KbdPeek(&k, 0))
		return 0;
	return (k.fbStatus & KBDTRF_FINAL_CHAR_IN);
}
#endif

char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int	textlen = 0;
	char		ch;

	if (! kbhit())
		return NULL;

	ch = getche();

	switch (ch)
	{
	case '\r':
		putch('\n');
		if (textlen)
		{
			con_text[textlen] = '\0';
			textlen = 0;
			return con_text;
		}
		break;

	case '\b':
		putch(' ');
		if (textlen)
		{
			textlen--;
			putch('\b');
		}
		break;

	default:
		con_text[textlen] = ch;
		textlen = (textlen + 1) & 0xff;
		break;
	}

	return NULL;
}

double Sys_DoubleTime (void)
{
	union i64 { QWORD qw; long long ll; };
	static qboolean		first = true;
	static ULONG		ticks_per_sec;
	static union i64	start;
	union i64		now;

	if (first)
	{
		first = false;
		DosTmrQueryFreq(&ticks_per_sec);
		DosTmrQueryTime(&start.qw);
		return 0.0;
	}

	DosTmrQueryTime(&now.qw);
	return (double)(now.ll - start.ll) / (double)ticks_per_sec;
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

	Cbuf_Init();
	Cmd_Init ();
	SV_InitNet();

	printf ("Exe: " __TIME__ " " __DATE__ "\n");
	printf("======== HW master %d.%d.%d initialized ========\n\n",
		VER_HWMASTER_MAJ, VER_HWMASTER_MID, VER_HWMASTER_MIN);

	while (1)
	{
		SV_Frame();
	}

	return 0;
}

