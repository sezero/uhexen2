#include <sys/types.h>
#include <sys/timeb.h>
#include <winsock.h>
#include "qwsvdef.h"
#include <errno.h>
#include <io.h>
#include <conio.h>


// heapsize: minimum 8 mb, standart 16 mb, max is 32 mb.
// -heapsize argument will abide by these min/max settings
// unless the -forcemem argument is used
#define MIN_MEM_ALLOC	0x0800000
#define STD_MEM_ALLOC	0x1000000
#define MAX_MEM_ALLOC	0x2000000

cvar_t	sys_nostdout = {"sys_nostdout","0"};

/*
================
Sys_mkdir
================
*/
int Sys_mkdir (char *path)
{
	int rc;

	rc = _mkdir (path);
	if (rc != 0 && errno == EEXIST)
		rc = 0;

	return rc;
}


/*
================
Sys_Error
================
*/
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];

	va_start (argptr,error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

	printf ("\nFATAL ERROR: %s\n\n", text);

#ifdef DEBUG_BUILD
	getch();
#endif

	exit (1);
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	double t;
	struct _timeb	tstruct;
	static int	starttime;

	_ftime( &tstruct );

	if (!starttime)
		starttime = tstruct.time;
	t = (tstruct.time-starttime) + tstruct.millitm*0.001;

	return t;
}


/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int		len;
	int		c;

	// read a line out
	while (_kbhit())
	{
		c = _getch();
		putch (c);
		if (c == '\r')
		{
			con_text[len] = 0;
			putch ('\n');
			len = 0;
			return con_text;
		}
		if (c == 8)
		{
			if (len)
			{
				putch (' ');
				putch (c);
				len--;
				con_text[len] = 0;
			}
			continue;
		}
		con_text[len] = c;
		len++;
		con_text[len] = 0;
		if (len == sizeof(con_text))
			len = 0;
	}

	return NULL;
}


/*
================
Sys_Printf
================
*/
void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;

	if (sys_nostdout.value)
		return;

	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

/*
================
Sys_Quit
================
*/
void Sys_Quit (void)
{
	exit (0);
}


/*
==================
main

==================
*/
int main (int argc, char **argv)
{
	quakeparms_t	parms;
	double		newtime, time, oldtime;
	//static	char	cwd[1024];
	struct timeval	timeout;
	fd_set		fdset;
	int		t;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

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
		Sys_Error("Insufficient memory.\n");

	parms.basedir = ".";
	parms.cachedir = NULL;
	parms.userdir = parms.basedir;	// no userdir on win32

	SV_Init (&parms);

// run one frame immediately for first heartbeat
	SV_Frame (HX_FRAME_TIME);

// report the filesystem to the user
	Sys_Printf("com_gamedir is: %s\n",com_gamedir);
	Sys_Printf("com_userdir is: %s\n",com_userdir);

//
// main loop
//
	oldtime = Sys_DoubleTime () - HX_FRAME_TIME;
	while (1)
	{
	// select on the net socket and stdin
	// the only reason we have a timeout at all is so that if the last
	// connected client times out, the message would not otherwise
	// be printed until the next event.
		FD_ZERO(&fdset);
		FD_SET(net_socket, &fdset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		if (select (net_socket+1, &fdset, NULL, NULL, &timeout) == -1)
			continue;

	// find time passed since last cycle
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;
		oldtime = newtime;

		SV_Frame (time);
	}

	return 0;
}

