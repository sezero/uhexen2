#include "defs.h"
#include <limits.h>


int sv_mode;

sizebuf_t	cmd_text;
byte		cmd_text_buf[8192];

char		com_token[1024];
int		com_argc;
char	**com_argv;

static char	*largv[MAX_NUM_ARGVS + 1];
static char	*argvdummy = " ";

bool	bigendien;

short	(*BigShort) (short l);
short	(*LittleShort) (short l);
int	(*BigLong) (int l);
int	(*LittleLong) (int l);
float	(*BigFloat) (float l);
float	(*LittleFloat) (float l);

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	ShortNoSwap (short l)
{
	return l;
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

int	LongNoSwap (int l)
{
	return l;
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

float FloatNoSwap (float f)
{
	return f;
}

////////////////////////////////

void COM_Init (void)
{
	byte	swaptest[2] = {1,0};

// set the byte swapping variables in a portable manner	
	if ( *(short *)swaptest == 1)
	{
		bigendien = false;
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
	}
	else
	{
 		bigendien = true;
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
	}
}

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
/////////////////////////////////////////////////////////////
void SV_WriteFilterList();

void SV_Shutdown (void)
{
	NET_Shutdown ();

	//write filter list
	SV_WriteFilterList();
}

char *Sys_ConsoleInput (void)
{
	static char	text[256];
	static int		len;
	char		c;
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
	static bool first = true;
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
	double time = Sys_DoubleTime();

	server_t *sv;
	server_t *next;

	if(sv_list==NULL)
		return;

	for(sv = sv_list;sv;)
	{
		if(sv->timeout + SV_TIMEOUT < time)
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

	SV_GetConsoleCommands ();
	
	Cbuf_Execute ();

	SV_TimeOut();

	SV_ReadPackets();	
}

int main (int argc, char **argv)
{
	COM_Init();
	COM_InitArgv (argc, argv);

	Cbuf_Init();
	Cmd_Init ();	

	SV_InitNet();

	printf ("Exe: "__TIME__" "__DATE__"\n");

	printf("======== HW master initialized ========\n\n");

	while(1)
	{
		SV_Frame();
	}

	return 0;
}
