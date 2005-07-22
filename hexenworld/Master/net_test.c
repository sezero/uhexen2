#include "defs.h"
#include <limits.h>

#ifndef _WIN32
int		do_stdin = 1;
qboolean	stdin_ready;
#endif

int		sv_mode;

sizebuf_t	cmd_text;
byte		cmd_text_buf[8192];

char		com_token[1024];
int		com_argc;
char	**com_argv;

static char	*largv[MAX_NUM_ARGVS + 1];
static char	*argvdummy = " ";

#ifdef PLATFORM_UNIX
char		userdir[240];
#endif
char		filters_file[256];

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

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


////////////////////////////////

void COM_InitArgv (int argc, char **argv)
{

	for (com_argc=0 ; (com_argc<MAX_NUM_ARGVS) && (com_argc < argc) ;
		 com_argc++)
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
			return NULL;			// end of file;
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

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

//    MessageBox(NULL, text, "Error", 0 /* MB_OK */ );
	printf ("ERROR: %s\n", text);

	exit (1);
}

void Sys_Quit (void)
{
	exit (0);
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

/////////////////////////////////////////////////////////////

void SV_WriteFilterList();

void SV_Shutdown (void)
{
	NET_Shutdown ();

	//write filter list
	SV_WriteFilterList();
}

/* sys_dead_sleep: When set, the server gets NO cpu if no clients are connected
   and there's no other activity. *MIGHT* cause problems with some mods. */
qboolean sys_dead_sleep	= 0;

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

int Sys_CheckInput (int ns)
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

char *Sys_ConsoleInput (void)
{
	static char	text[256];
	static int		len;

#ifdef _WIN32
	int		c;

	// read a line out
	while (_kbhit ()) {
		c = _getch ();
		putch (c);
		if (c == '\r') {
			text[len] = 0;
			putch ('\n');
			len = 0;
			return text;
		}
		if (c == 8) {
			if (len) {
				putch (' ');
				putch (c);
				len--;
				text[len] = 0;
			}
			continue;
		}
		text[len] = c;
		len++;
		text[len] = 0;
		if (len == sizeof (text))
			len = 0;
	}

	return NULL;
#else
	if (!stdin_ready || !do_stdin)
		return NULL;	// the select didn't say it was ready
	stdin_ready = false;

	len = read (0, text, sizeof (text));
	if (len == 0) {
		// end of file
		do_stdin = 0;
		return NULL;
	}
	if (len < 1)
		return NULL;
	text[len - 1] = 0;	// rip off the \n and terminate

	return text;
#endif
}

void SV_GetConsoleCommands (void)
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

	if (first) {
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

	if (first) {
		first = false;
		start_time = now;
	}

	return now - start_time;
#endif
}

#define SV_TIMEOUT 450

void SV_TimeOut()
{
	//Remove listed severs that havnt sent a heartbeat for some time
	double t = Sys_DoubleTime();

	server_t *sv;
	server_t *next;

	if(sv_list==NULL)
		return;

	for(sv = sv_list;sv;)
	{
		if(sv->timeout + SV_TIMEOUT < t)
		{
			next = sv->next;
			printf("%s timed out\n",NET_AdrToString(sv->ip));
			SVL_Remove(sv);
			free(sv);
			sv = next;
		}
		else
			sv = sv->next;
	}
	

}

void SV_Frame()
{

	Sys_CheckInput (net_socket);

	SV_GetConsoleCommands ();

	Cbuf_Execute ();

	SV_TimeOut();

	SV_ReadPackets();	
}

int main (int argc, char **argv)
{
	int t;

	if (argc>=1) {
	    for (t=1;t<argc;t++) {
		if ((strcmp(argv[t], "-h"    ) == 0 ||
		     strcmp(argv[t], "-help" ) == 0 ||
		     strcmp(argv[t], "--help") == 0 ||
		     strcmp(argv[t], "-?") == 0 ))
		{
			printf("\nHexenWorld master server %s\n\n", VER_HWMASTER);
			printf("Usage:     hwmaster [-port xxxxx]\n");
			printf("See the documentation for details\n\n");
			exit(0);
		}
		else if ((strcmp(argv[t], "-v"  ) == 0 ||
			  strcmp(argv[t], "-version" ) == 0 ||
			  strcmp(argv[t], "--version") == 0 ))
		{
			printf("hwmaster %s\n", VER_HWMASTER);
			exit(0);
		}
	    }
	}

	COM_InitArgv (argc, argv);

#ifdef PLATFORM_UNIX
	if (getenv("HOME") == NULL)
		Sys_Error ("Couldn't determine userspace directory");
	sprintf(userdir, "%s/%s", getenv("HOME"), ".hwmaster");
	t = mkdir (userdir, 0755);
	if (t != 0 && errno != EEXIST)
		Sys_Error ("Couldn't create user directory");
	sprintf(filters_file, "%s/%s", userdir, "filters.ini");
#else
	sprintf(filters_file, "%s", "filters.ini");
#endif

	Cbuf_Init();
	Cmd_Init ();	

	SV_InitNet();

	printf ("Exe: "__TIME__" "__DATE__"\n");

	printf("======== HW master %s initialized ========\n\n", VER_HWMASTER);

	while(1)
	{
		SV_Frame();
	}

	return 0;
}
