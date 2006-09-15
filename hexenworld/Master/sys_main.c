// sys_main.c

// whether to use the password file to determine
// the path to the home directory
#define USE_PASSWORD_FILE	0

#include "defs.h"
#if defined(_WIN32)
#include <windows.h>
#endif
#include <limits.h>

#if defined(PLATFORM_UNIX)
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#if USE_PASSWORD_FILE
#include <pwd.h>
#endif
#endif

#if defined(_WIN32)
#include <sys/timeb.h>
#include <time.h>
#include <io.h>
#include <conio.h>
#endif

#if defined(PLATFORM_UNIX)
static int	do_stdin = 1;
static qboolean	stdin_ready;
static char	userdir[256];
#endif

char		com_token[1024];
int		com_argc;
char	**com_argv;

static char	*largv[MAX_NUM_ARGVS + 1];
static char	*argvdummy = " ";

char		filters_file[256];


//=============================================================================

short ShortSwap (short l)
{
	byte	b1, b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

int LongSwap (int l)
{
	byte	b1, b2, b3, b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];

	return dat2.f;
}


//=============================================================================

static void COM_InitArgv (int argc, char **argv)
{
	for (com_argc=0 ; (com_argc<MAX_NUM_ARGVS) && (com_argc < argc) ; com_argc++)
	{
		largv[com_argc] = argv[com_argc];
	}

	largv[com_argc] = argvdummy;
	com_argv = largv;
}

int COM_CheckParm (char *parm)
{
	int		i;

	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP sometimes clears appkit vars.
		if (!strcmp (parm,com_argv[i]))
			return i;
	}

	return 0;
}

char *COM_Parse (char *data)
{
	int		c;
	int		len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;	// end of file;
		data++;
	}

// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	} while (c>32);

	com_token[len] = 0;
	return data;
}


//=============================================================================

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsnprintf (text, sizeof (text), error,argptr);
	va_end (argptr);

	printf ("\nFATAL ERROR: %s\n\n", text);

	exit (1);
}

void Sys_Quit (void)
{
	exit (0);
}


//=============================================================================

void SZ_Init (sizebuf_t *buf, byte *data, int length)
{
	memset (buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
	//buf->cursize = 0;
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void	*data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("SZ_GetSpace: overflow without allowoverflow set (%d)", buf->maxsize);

		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %i is > full buffer size", length);

		printf ("SZ_GetSpace: overflow\n");	// because Con_Printf may be redirected
		SZ_Clear (buf);
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write (sizebuf_t *buf, void *data, int length)
{
	memcpy (SZ_GetSpace(buf,length),data,length);
}

//=============================================================================

void SV_WriteFilterList(void);

void SV_Shutdown (void)
{
	NET_Shutdown ();

	//write filter list
	SV_WriteFilterList();
}


//=============================================================================

/* sys_dead_sleep: When set, the server gets NO cpu if no clients are connected
   and there's no other activity. *MIGHT* cause problems with some mods. */
static qboolean sys_dead_sleep	= 0;

static int Sys_CheckInput (int ns)
{
	fd_set		fdset;
	int		res;
	struct timeval	_timeout;
	struct timeval	*timeout = 0;

	_timeout.tv_sec = 0;
#ifdef _WIN32
	_timeout.tv_usec = ns < 0 ? 0 : 100;
#else
	_timeout.tv_usec = ns < 0 ? 0 : 10000;
#endif
	// select on the net socket and stdin
	// the only reason we have a timeout at all is so that if the last
	// connected client times out, the message would not otherwise
	// be printed until the next event.
	FD_ZERO (&fdset);

#ifndef _WIN32
	if (do_stdin)
		FD_SET (0, &fdset);
#endif
	if (ns >= 0)
		FD_SET (ns, &fdset);

	if (!sys_dead_sleep)
		timeout = &_timeout;

	res = select (max (ns, 0) + 1, &fdset, NULL, NULL, timeout);
	if (res == 0 || res == -1)
		return 0;

#ifndef _WIN32
	stdin_ready = FD_ISSET (0, &fdset);
#endif
	return 1;
}

static char *Sys_ConsoleInput (void)
{
	static char	con_text[256];
	static int		textlen;

#ifdef _WIN32
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

static void SV_GetConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}

double Sys_DoubleTime (void)
{
	static qboolean first = true;
#ifdef _WIN32
	static DWORD starttime;
	DWORD now;

	now = timeGetTime();

	if (first)
	{
		first = false;
		starttime = now;
		return 0.0;
	}

	if (now < starttime) // wrapped?
		return (now / 1000.0) + (LONG_MAX - starttime / 1000.0);

	if (now - starttime == 0)
		return 0.0;

	return (now - starttime) / 1000.0;
#else
	struct timeval tp;
	struct timezone tzp;
	double now;
	static double start_time;

	gettimeofday (&tp, &tzp);

	now = tp.tv_sec + tp.tv_usec / 1e6;

	if (first)
	{
		first = false;
		start_time = now;
	}

	return now - start_time;
#endif
}

#ifdef PLATFORM_UNIX

int Sys_mkdir (char *path)
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

	snprintf (buff, path_len, "%s/%s", home_dir, HWM_USERDIR);
	return Sys_mkdir(buff);
}
#endif


#define SV_TIMEOUT 450

static void SV_TimeOut(void)
{
	//Remove listed severs that havent sent a heartbeat for some time
	double t = Sys_DoubleTime();

	server_t *sv;
	server_t *next;

	if (sv_list == NULL)
		return;

	for (sv=sv_list ; sv ; )
	{
		if (sv->timeout + SV_TIMEOUT < t)
		{
			next = sv->next;
			printf("%s timed out\n",NET_AdrToString(sv->ip));
			SVL_Remove(sv);
			free(sv);
			sv = next;
		}
		else
		{
			sv = sv->next;
		}
	}
}

static void SV_Frame(void)
{
	Sys_CheckInput (net_socket);

	SV_GetConsoleCommands ();

	Cbuf_Execute ();

	SV_TimeOut();

	SV_ReadPackets();
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
				printf("\nHexenWorld master server %s\n\n", VER_HWMASTER);
				printf("Usage:     hwmaster [-port xxxxx]\n");
				printf("See the documentation for details\n\n");
				exit(0);
			}
			else if ( !(strcmp(argv[t], "-v")) || !(strcmp(argv[t], "-version")) ||
				  !(strcmp(argv[t], "--version")) )
			{
				printf("hwmaster %s\n", VER_HWMASTER);
				exit(0);
			}
		}
	}

	COM_InitArgv (argc, argv);

#ifdef PLATFORM_UNIX
// userdir stuff
	if (Sys_GetUserdir(userdir,sizeof(userdir)) != 0)
		Sys_Error ("Couldn't determine userspace directory");
	printf ("Userdir: %s\n", userdir);
	sprintf(filters_file, "%s/%s", userdir, "filters.ini");
#else
	sprintf(filters_file, "%s", "filters.ini");
#endif

#ifdef _WIN32
	timeBeginPeriod (1);
#endif

	Cbuf_Init();
	Cmd_Init ();

	SV_InitNet();

	printf ("Exe: "__TIME__" "__DATE__"\n");

	printf("======== HW master %s initialized ========\n\n", VER_HWMASTER);

	while (1)
	{
		SV_Frame();
	}

	return 0;
}

