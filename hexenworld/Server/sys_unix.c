/*
	sys_unix.c
	$Id: sys_unix.c,v 1.8 2005-09-20 21:19:45 sezero Exp $

	Unix system interface code
*/

#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>

#include "qwsvdef.h"


cvar_t	sys_nostdout = {"sys_nostdout","0"};

/*
================
Sys_GetUserdir
================
*/
int Sys_GetUserdir (char *buff, unsigned int len)
{
	if (getenv("HOME") == NULL)
		return 1;

	if (strlen(getenv("HOME")) + strlen(AOT_USERDIR) + 5 > len)
		return 1;

	sprintf (buff, "%s/%s", getenv("HOME"), AOT_USERDIR);
	return Sys_mkdir(buff);
}

/*
================
Sys_mkdir
================
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
================
Sys_Error
================
*/
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

	printf ("\nFATAL ERROR: %s\n\n", text);

	exit (1);
}


/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	struct timeval  tp;
	struct timezone tzp;
	static int	secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase) {
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
	static char	text[256];
	static int	len;
	char	c;
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
=============
Sys_Init

Quake calls this so the system can register variables before host_hunklevel
is marked
=============
*/
void Sys_Init (void)
{
	Cvar_RegisterVariable (&sys_nostdout);
}

/*
==================
main

==================
*/
char	*newargv[256];

int main (int argc, char **argv)
{
	quakeparms_t	parms;
	double		newtime, time, oldtime;
	struct timeval	timeout;
	fd_set		fdset;
	int		t;
	static char	userdir[MAX_OSPATH];

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	if (COM_CheckParm ("-help") || COM_CheckParm ("--help") ||
	    COM_CheckParm ("-h")    || COM_CheckParm ("-?"))
	{
		printf ("HexenWorld server %4.2f (%s)\n", VERSION, VERSION_PLATFORM);
		printf ("(Hammer of Thyrion, release %d.%d.%d)\n",
			 HOT_VERSION_MAJ, HOT_VERSION_MID, HOT_VERSION_MIN);
		printf ("See the documentation for details\n");
		exit (0);
	}

	if (COM_CheckParm ("-v") || COM_CheckParm ("-version") || COM_CheckParm ("--version"))
	{
		printf ("hwsv %4.2f (%s)\n", VERSION, VERSION_PLATFORM);
		exit (0);
	}

	// Client uses 32 Mb minimum but this server-only situation
	// should go well with 16 Mb. We can always use -heapsize..
	parms.memsize = 16*1024*1024;

	if (((t = COM_CheckParm ("-heapsize")) != 0) && (t + 1 < com_argc))
	{
		parms.memsize = atoi (com_argv[t + 1]) * 1024;

		if (parms.memsize > 64*1024*1024) { // no bigger than 64 MB
			Sys_Printf ("Requested memory (%d Mb) too large.\n", parms.memsize/(1024*1024));
			Sys_Printf ("Will try going with a saner 64 Mb..\n");
			parms.memsize = 64*1024*1024;
		} else if (parms.memsize < 8*1024*1024) { // no less than 8 MB
			Sys_Printf ("Requested memory (%d Mb) too little.\n", parms.memsize/(1024*1024));
			Sys_Printf ("Will try going with a humble 8 Mb..\n");
			parms.memsize = 8*1024*1024;
		}
	}

	parms.membase = malloc (parms.memsize);

	if (!parms.membase)
		Sys_Error("Insufficient memory.\n");

	parms.basedir = ".";
	parms.cachedir = NULL;

	if (Sys_GetUserdir(userdir,sizeof(userdir)))
		Sys_Error ("Couldn't determine userspace directory");
	parms.userdir = userdir;

	SV_Init (&parms);

// run one frame immediately for first heartbeat
	SV_Frame (HX_FRAME_TIME);		

// report the filesystem to the user
	Sys_Printf("userdir is: %s\n",userdir);
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

	return true;
}
