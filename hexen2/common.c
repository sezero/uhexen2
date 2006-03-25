/*
	common.c
	misc functions used in client and server

	$Id: common.c,v 1.41 2006-03-25 09:33:01 sezero Exp $
*/

#if defined(H2W) && defined(SERVERONLY)
#include "qwsvdef.h"
#else
#include "quakedef.h"
#endif
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#include <ctype.h>
#endif

#define NUM_SAFE_ARGVS	6

static char	*largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static char	*argvdummy = " ";

static char	*safeargvs[NUM_SAFE_ARGVS] =
	{"-nomidi", "-nolan", "-nosound", "-nocdaudio", "-nojoy", "-nomouse"};

cvar_t	registered = {"registered", "0", CVAR_ROM};
cvar_t	oem = {"oem", "0", CVAR_ROM};

unsigned int	gameflags;

qboolean		msg_suppress_1 = 0;

static void COM_InitFilesystem (void);
static void COM_Path_f (void);
#ifndef SERVERONLY
static void COM_Maplist_f (void);
#endif

// look-up table of pak filenames: { numfiles, crc }
// if a packfile directory differs from this, it is assumed to be hacked
#define MAX_PAKDATA	6
static const int pakdata[MAX_PAKDATA][2] = {
	{ 696,	34289 },	/* pak0.pak, registered	*/
	{ 523,	2995  },	/* pak1.pak, registered	*/
	{ 183,	4807  },	/* pak2.pak, oem, data needs verification */
	{ 245,	1478  },	/* pak3.pak, portals	*/
	{ 102,	41062 },	/* pak4.pak, hexenworld	*/
	{ 797,	22780 }		/* pak0.pak, demo	*/
};
// loacations of pak filenames as shipped by raven
static const char *dirdata[MAX_PAKDATA] = {
	"data1",	/* pak0.pak, registered	*/
	"data1",	/* pak1.pak, registered	*/
	"data1",	/* pak2.pak, oem	*/
	"portals",	/* pak3.pak, portals	*/
	"hw",		/* pak4.pak, hexenworld	*/
	"data1"		/* pak0.pak, demo	*/
};

char	gamedirfile[MAX_OSPATH];

#define CMDLINE_LENGTH	256
static char	com_cmdline[CMDLINE_LENGTH];

// this graphic needs to be in the pak file to use registered features
static const unsigned short pop[] =
{
 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
,0x0000,0x0000,0x6600,0x0000,0x0000,0x0000,0x6600,0x0000
,0x0000,0x0066,0x0000,0x0000,0x0000,0x0000,0x0067,0x0000
,0x0000,0x6665,0x0000,0x0000,0x0000,0x0000,0x0065,0x6600
,0x0063,0x6561,0x0000,0x0000,0x0000,0x0000,0x0061,0x6563
,0x0064,0x6561,0x0000,0x0000,0x0000,0x0000,0x0061,0x6564
,0x0064,0x6564,0x0000,0x6469,0x6969,0x6400,0x0064,0x6564
,0x0063,0x6568,0x6200,0x0064,0x6864,0x0000,0x6268,0x6563
,0x0000,0x6567,0x6963,0x0064,0x6764,0x0063,0x6967,0x6500
,0x0000,0x6266,0x6769,0x6a68,0x6768,0x6a69,0x6766,0x6200
,0x0000,0x0062,0x6566,0x6666,0x6666,0x6666,0x6562,0x0000
,0x0000,0x0000,0x0062,0x6364,0x6664,0x6362,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0062,0x6662,0x0000,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0061,0x6661,0x0000,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0000,0x6500,0x0000,0x0000,0x0000
,0x0000,0x0000,0x0000,0x0000,0x6400,0x0000,0x0000,0x0000
};


//============================================================================

/*
All of Quake's data access is through a hierchal file system, but the contents
of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the exe and all game
directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.
This can be overridden with the "-basedir" command line parm to allow code
debugging in a different directory.  The base directory is only used during
filesystem initialization.

The "game directory" is the first tree on the search path and directory that
all generated files (savegames, screenshots, demos, config files) will be saved
to.  This can be overridden with the "-game" command line parameter.  The game
directory can never be changed while quake is executing.  This is a precacution
against having a malicious server instruct clients to write files over areas
they shouldn't.

The "cache directory" is only used during development to save network bandwidth
especially over ISDN / T1 lines.  If there is a cache directory specified, when
a file is found by the normal search path, it will be mirrored into the cache
directory, then opened there.
*/


//============================================================================

// ClearLink is used for new headnodes
void ClearLink (link_t *l)
{
	l->prev = l->next = l;
}

void RemoveLink (link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

void InsertLinkBefore (link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}

void InsertLinkAfter (link_t *l, link_t *after)
{
	l->next = after->next;
	l->prev = after;
	l->prev->next = l;
	l->next->prev = l;
}

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

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


/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

//
// writing functions
//

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

	buf = SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte	*buf;

	buf = SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
{
	union
	{
		float	f;
		int	l;
	} dat;

	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, strlen(s)+1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
//	MSG_WriteShort (sb, (int)(f*8));
	if (f >= 0)
		MSG_WriteShort (sb, (int)(f * 8.0 + 0.5));
	else
		MSG_WriteShort (sb, (int)(f * 8.0 - 0.5));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
//	MSG_WriteByte (sb, (int)(f*256/360) & 255);
//	LordHavoc: round to nearest value, rather than rounding toward zero
	if (f >= 0)
		MSG_WriteByte (sb, (int)(f*(256.0/360.0) + 0.5) & 255);
	else
		MSG_WriteByte (sb, (int)(f*(256.0/360.0) - 0.5) & 255);
}

#ifdef H2W
void MSG_WriteAngle16 (sizebuf_t *sb, float f)
{
//	MSG_WriteShort (sb, (int)(f*65536/360) & 65535);
	if (f >= 0)
		MSG_WriteShort (sb, (int)(f*(65536.0/360.0) + 0.5) & 65535);
	else
		MSG_WriteShort (sb, (int)(f*(65536.0/360.0) - 0.5) & 65535);
}

void MSG_WriteUsercmd (sizebuf_t *buf, usercmd_t *cmd, qboolean long_msg)
{
	int		bits;

//
// send the movement message
//
	bits = 0;
	if (cmd->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove)
		bits |= CM_UP;
	if (cmd->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse)
		bits |= CM_IMPULSE;
	if (cmd->msec)
		bits |= CM_MSEC;

	MSG_WriteByte (buf, bits);
	if (long_msg)
	{
		MSG_WriteByte (buf, cmd->light_level);
	}

	if (bits & CM_ANGLE1)
		MSG_WriteAngle16 (buf, cmd->angles[0]);
	MSG_WriteAngle16 (buf, cmd->angles[1]);
	if (bits & CM_ANGLE3)
		MSG_WriteAngle16 (buf, cmd->angles[2]);

	if (bits & CM_FORWARD)
		MSG_WriteChar (buf, (int)(cmd->forwardmove*0.25));
	if (bits & CM_SIDE)
	  	MSG_WriteChar (buf, (int)(cmd->sidemove*0.25));
	if (bits & CM_UP)
		MSG_WriteChar (buf, (int)(cmd->upmove*0.25));

	if (bits & CM_BUTTONS)
	  	MSG_WriteByte (buf, cmd->buttons);
	if (bits & CM_IMPULSE)
		MSG_WriteByte (buf, cmd->impulse);
	if (bits & CM_MSEC)
		MSG_WriteByte (buf, cmd->msec);
}
#endif	// H2W


//
// reading functions
//
int		msg_readcount;
qboolean	msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int	c;

	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (signed char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadByte (void)
{
	int	c;

	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (unsigned char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadShort (void)
{
	int	c;

	if (msg_readcount+2 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (short)(net_message.data[msg_readcount]
			+ (net_message.data[msg_readcount+1]<<8));

	msg_readcount += 2;

	return c;
}

int MSG_ReadLong (void)
{
	int	c;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = net_message.data[msg_readcount]
			+ (net_message.data[msg_readcount+1]<<8)
			+ (net_message.data[msg_readcount+2]<<16)
			+ (net_message.data[msg_readcount+3]<<24);

	msg_readcount += 4;

	return c;
}

float MSG_ReadFloat (void)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;

	dat.b[0] =	net_message.data[msg_readcount];
	dat.b[1] =	net_message.data[msg_readcount+1];
	dat.b[2] =	net_message.data[msg_readcount+2];
	dat.b[3] =	net_message.data[msg_readcount+3];
	msg_readcount += 4;

	dat.l = LittleLong (dat.l);

	return dat.f;
}

char *MSG_ReadString (void)
{
	static char	string[2048];
	int		l,c;

	l = 0;
	do
	{
		c = MSG_ReadChar ();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);

	string[l] = 0;

	return string;
}

#ifdef H2W
char *MSG_ReadStringLine (void)
{
	static char	string[2048];
	int		l,c;

	l = 0;
	do
	{
		c = MSG_ReadChar ();
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);

	string[l] = 0;

	return string;
}
#endif	// H2W

float MSG_ReadCoord (void)
{
	return MSG_ReadShort() * (1.0/8.0);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0/256.0);
}

#ifdef H2W
float MSG_ReadAngle16 (void)
{
	return MSG_ReadShort() * (360.0/65536.0);
}

void MSG_ReadUsercmd (usercmd_t *move, qboolean long_msg)
{
	int bits;

	memset (move, 0, sizeof(*move));

	bits = MSG_ReadByte ();
	if (long_msg)
	{
		move->light_level = MSG_ReadByte();
	}
	else
	{
		move->light_level = 0;
	}

// read current angles
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadAngle16 ();
	else
		move->angles[0] = 0;
	move->angles[1] = MSG_ReadAngle16 ();
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadAngle16 ();
	else
		move->angles[2] = 0;

// read movement
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadChar () * 4;
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadChar () * 4;
	if (bits & CM_UP)
		move->upmove = MSG_ReadChar () * 4;

// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte ();
	else
		move->buttons = 0;

	if (bits & CM_IMPULSE)
		move->impulse = MSG_ReadByte ();
	else
		move->impulse = 0;

// read time to run command
	if (bits & CM_MSEC)
		move->msec = MSG_ReadByte ();
	else
		move->msec = 0;
}
#endif	// H2W


//===========================================================================

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
			Sys_Error ("SZ_GetSpace: overflow without allowoverflow set");

		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %i is > full buffer size", length);

		Sys_Printf ("SZ_GetSpace: overflow\nCurrently %d of %d, requested %d\n",buf->cursize,buf->maxsize,length);
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

void SZ_Print (sizebuf_t *buf, char *data)
{
	int		len;

	len = strlen(data)+1;

	if (!buf->cursize || buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}


//============================================================================


/*
============
COM_SkipPath
============
*/
char *COM_SkipPath (char *pathname)
{
	char	*last;

	last = pathname;
	while (*pathname)
	{
		if (*pathname=='/')
			last = pathname+1;
		pathname++;
	}
	return last;
}

/*
============
COM_StripExtension
============
*/
void COM_StripExtension (char *in, char *out)
{
	while (*in && *in != '.')
		*out++ = *in++;
	*out = 0;
}

/*
============
COM_FileExtension
============
*/
char *COM_FileExtension (char *in)
{
	static char exten[8];
	int		i;

	while (*in && *in != '.')
		in++;
	if (!*in)
		return "";
	in++;
	for (i=0 ; i<7 && *in ; i++,in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase (char *in, char *out)
{
	char	*s, *s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
		s--;

	/* Pa3PyX: no range checking -- used to trash the stack and crash the
	   game randomly upon loading progs, for instance (or in any other
	   instance where one would supply a filename witout a path */
//	for (s2 = s ; *s2 && *s2 != '/' ; s2--);
	for (s2 = s; *s2 && *s2 != '/' && s2 >= in; s2--)
		;

	if (s-s2 < 2)
		strcpy (out,"?model?");
	else
	{
		s--;
		strncpy (out,s2+1, s-s2);
		out[s-s2] = 0;
	}
}


/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension (char *path, char *extension)
{
	char	*src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	strcat (path, extension);
}


// quick'n'dirty string comparison function for use with qsort
int COM_StrCompare (const void *arg1, const void *arg2)
{
	return Q_strcasecmp ( *(char **) arg1, *(char **) arg2);
}

//============================================================================

char		com_token[1024];
int		com_argc;
char	**com_argv;


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
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

// parse single characters
/*	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}
*/
// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;

//		if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
//			break;
	} while (c>32);

	com_token[len] = 0;
	return data;
}


/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
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

/*
================
COM_CheckRegistered

Looks for the pop.txt file and verifies it.
Sets the registered flag.
================
*/
void COM_CheckRegistered (void)
{
	FILE		*h;
	unsigned short	check[128];
	int			i;

	COM_FOpenFile("gfx/pop.lmp", &h, false);

	if (!h)
		return;

	fread (check, 1, sizeof(check), h);
	fclose (h);

	for (i=0 ; i<128 ; i++)
		if (pop[i] != (unsigned short)BigShort (check[i]))
			Sys_Error ("Corrupted data file.");

	// check if we have 1.11 versions of pak0.pak and pak1.pak
	if (!(gameflags & GAME_REGISTERED0) || !(gameflags & GAME_REGISTERED1))
		Sys_Error ("You must patch your installation with Raven's 1.11 update");

	gameflags |= GAME_REGISTERED;
}


/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, char **argv)
{
	qboolean	safe;
	int		i, j, n;

// reconstitute the command line for the cmdline console command
	n = 0;

	for (j=0 ; (j<MAX_NUM_ARGVS) && (j< argc) ; j++)
	{
		i = 0;

		while ((n < (CMDLINE_LENGTH - 1)) && argv[j][i])
		{
			com_cmdline[n++] = argv[j][i++];
		}

		if (n < (CMDLINE_LENGTH - 1))
			com_cmdline[n++] = ' ';
		else
			break;
	}

	com_cmdline[n] = 0;

	safe = false;

	for (com_argc=0 ; (com_argc<MAX_NUM_ARGVS) && (com_argc < argc) ;
		 com_argc++)
	{
		largv[com_argc] = argv[com_argc];
		if (!strcmp ("-safe", argv[com_argc]))
			safe = true;
	}

	if (safe)
	{
	// force all the safe-mode switches. Note that we reserved extra space in
	// case we need to add these, so we don't need an overflow check
		for (i=0 ; i<NUM_SAFE_ARGVS ; i++)
		{
			largv[com_argc] = safeargvs[i];
			com_argc++;
		}
	}

	largv[com_argc] = argvdummy;
	com_argv = largv;
}

#if 0
/*
================
COM_AddParm

Adds the given string at the end of the current argument list
================
*/
void COM_AddParm (char *parm)
{
	largv[com_argc++] = parm;
}
#endif

static void COM_Cmdline_f (void)
{
	Con_Printf ("cmdline is: \"%s\"\n", com_cmdline);
}

/*
================
COM_Init
================
*/
void COM_Init (void)
{
	Cvar_RegisterVariable (&registered);
	Cvar_RegisterVariable (&oem);
	Cmd_AddCommand ("path", COM_Path_f);
	Cmd_AddCommand ("cmdline", COM_Cmdline_f);
#ifndef SERVERONLY
	Cmd_AddCommand ("maps", COM_Maplist_f);
#endif

	COM_InitFilesystem ();
}


/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char *va(char *format, ...)
{
	va_list		argptr;
	static char		string[1024];

	va_start (argptr, format);
	vsnprintf (string, sizeof (string), format, argptr);
	va_end (argptr);

	return string;
}

#if 0
/// just for debugging
int memsearch (byte *start, int count, int search)
{
	int		i;

	for (i=0 ; i<count ; i++)
		if (start[i] == search)
			return i;
	return -1;
}
#endif

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/

int	com_filesize;


//
// in memory
//

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

//
// on disk
//
typedef struct
{
	char	name[56];
	int		filepos, filelen;
} dpackfile_t;

typedef struct
{
	char	id[4];
	int		dirofs;
	int		dirlen;
} dpackheader_t;

#define	MAX_FILES_IN_PACK	2048

char	com_gamedir[MAX_OSPATH];
char	com_basedir[MAX_OSPATH];
char	com_userdir[MAX_OSPATH];
char	com_savedir[MAX_OSPATH];	// temporary path for saving gip files

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct	searchpath_s *next;
} searchpath_t;

static searchpath_t	*com_searchpaths;
static searchpath_t	*com_base_searchpaths;	// without gamedirs

/*
================
COM_filelength
================
*/
static int COM_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

static int COM_FileOpenRead (char *path, FILE **hndl)
{
	FILE	*f;

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = NULL;
		return -1;
	}
	*hndl = f;

	return COM_filelength(f);
}

/*
============
COM_Path_f

============
*/
static void COM_Path_f (void)
{
	searchpath_t	*s;

	Con_Printf ("Current search path:\n");
	for (s=com_searchpaths ; s ; s=s->next)
	{
		if (s == com_base_searchpaths)
			Con_Printf ("----------\n");
		if (s->pack)
			Con_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Con_Printf ("%s\n", s->filename);
	}
}

/*
============
COM_WriteFile

The filename will be prefixed by the current game directory
============
*/
void COM_WriteFile (char *filename, void *data, int len)
{
	FILE	*f;
	char	name[MAX_OSPATH];

	sprintf (name, "%s/%s", com_userdir, filename);

	f = fopen (name, "wb");
	if (!f)
		Sys_Error ("Error opening %s", filename);

	Sys_Printf ("COM_WriteFile: %s\n", name);
	fwrite (data, 1, len, f);
	fclose (f);
}


/*
============
COM_CreatePath

Only used for CopyFile and download
============
*/
void COM_CreatePath (char *path)
{
	char	*ofs;

	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			Sys_mkdir (path);
			*ofs = '/';
		}
	}
}


/*
===========
COM_CopyFile

Copies a file over from the net to the local cache, creating any directories
needed. Used for saving the game. Returns 0 on success, non-zero on error.
===========
*/
int COM_CopyFile (char *netpath, char *cachepath)
{
	FILE	*in, *out;
	int		err = 0, remaining, count;
	char	buf[4096];

	remaining = COM_FileOpenRead (netpath, &in);
	if (remaining == -1)
		return 1;
	COM_CreatePath (cachepath);	// create directories up to the cache file
	out = fopen(cachepath, "wb");
	if (!out)
		return 1;

	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);
		fread (buf, 1, count, in);
		err = ferror(in);
		if (err)
			break;
		fwrite (buf, 1, count, out);
		err = ferror(out);
		if (err)
			break;
		remaining -= count;
	}

	fclose (in);
	fclose (out);
	return err;
}

/*
===========
COM_Maplist_f

Prints map filenames to the console
===========
*/
#ifndef SERVERONLY
static void COM_Maplist_f (void)
{
	int			i, len, run, cnt;
	pack_t		*pak;
	searchpath_t	*search;
	char		**maplist = NULL, mappath[MAX_OSPATH];
	char	*findname;

	// do two runs - first count the number of maps
	// then collect their names into maplist
	for (run = 1 ; run <= 2; run ++)
	{
		cnt = 0;

		// search through the path, one element at a time
		for (search = com_searchpaths; search; search = search->next)
		{
			// either "search->filename" or "search->pak" is defined
			if (search->pack)
			{
				pak = search->pack;

				for (i = 0; i < pak->numfiles; i++)
				{
					if (strncmp ("maps/", pak->files[i].name, 5) == 0  && 
					    strstr(pak->files[i].name, ".bsp"))
					{
						// S.A.: remove those b_**** maps
						// side effect: real maps named b_**** will be ignored :<
						// O.S.: this is an issue with quake1 not hexen2.
						// disabling this check.
					//	if (strncmp ("b_", pak->files[i].name + 5, 2) == 0)
					//		continue;

						if (run == 2)
						{
							len = strlen (pak->files[i].name + 5) - 4 + 1;
							// - ".bsp" (-4) +  "\0" (+1)

							maplist[cnt] = malloc (len);
							strncpy ((char *)maplist[cnt] , pak->files[i].name + 5, len);
							// null terminate new string
							maplist[cnt][len - 1] = 0;
						}
						cnt++;
					}
				}
			}
			else
			{	// element is a filename, look for maps therein using scandir
				snprintf (mappath, MAX_OSPATH, search->filename);
				strcat (mappath, "/maps");
				findname = Sys_FindFirstFile (mappath, "*.bsp");
				while (findname)
				{
					if (run == 2)
					{
						// add to our maplist (the same as above)
						len = strlen(findname) - 4 + 1;
						maplist[cnt] = malloc (len);
						strncpy (maplist[cnt], findname, len);
						maplist[cnt][len - 1] = 0;
					}
					findname = Sys_FindNextFile ();
					cnt++;
				}
				Sys_FindClose ();
			}
		}

		if (run == 1)
		{
			// after first run, we know how many maps we have
			// should I use malloc or something else
			maplist = malloc(cnt * sizeof (char *));
		}
	}

	// sort the list
	qsort (maplist, cnt, sizeof(char *), COM_StrCompare);
	Con_Printf ("Found %d maps:\n\n", cnt);
	Con_ShowList (cnt, (const char**)maplist);
	Con_Printf ("\n");

	// Free memory
	for (i = 0; i < cnt; i++)
		free (maplist[i]);

	free (maplist);
}
#endif

/*
===========
COM_FindFile

Finds the file in the search path.
Sets com_filesize and one of handle or file
===========
*/
int file_from_pak;	// global indicating file came from pack file ZOID
int COM_FOpenFile (char *filename, FILE **file, qboolean override_pack)
{
	searchpath_t	*search;
	char		netpath[MAX_OSPATH];
	pack_t		*pak;
	int			i;

	file_from_pak = 0;

//
// search through the path, one element at a time
//
	for (search = com_searchpaths ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
				if (!strcmp (pak->files[i].name, filename))
				{	// found it!
					// open a new file on the pakfile
					*file = fopen (pak->filename, "rb");
					if (!*file)
						Sys_Error ("Couldn't reopen %s", pak->filename);	
					fseek (*file, pak->files[i].filepos, SEEK_SET);
					com_filesize = pak->files[i].filelen;
					file_from_pak = 1;
					return com_filesize;
				}
		}
		else
		{
	// check a file in the directory tree
#ifndef H2W
			if (!(gameflags & GAME_REGISTERED) && !override_pack)
			{	// if not a registered version, don't ever go beyond base
				if ( strchr (filename, '/') || strchr (filename,'\\'))
					continue;
			}
#endif	// !H2W

			sprintf (netpath, "%s/%s",search->filename, filename);
			if (access(netpath, R_OK) == -1)
				continue;

			*file = fopen (netpath, "rb");
			return COM_filelength (*file);
		}
	}

	Sys_Printf ("FindFile: can't find %s\n", filename);

	*file = NULL;
	com_filesize = -1;
	return -1;
}

/*
============
COM_LoadFile

Filename are reletive to the quake directory.
Allways appends a 0 byte to the loaded data.
============
*/
static cache_user_t *loadcache;
static byte	*loadbuf;
static int		loadsize;
static byte *COM_LoadFile (char *path, int usehunk)
{
	FILE	*h;
	byte	*buf;
	char	base[32];
	int		len;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = com_filesize = COM_FOpenFile (path, &h, false);
	if (!h)
		return NULL;

// extract the filename base name for hunk tag
	COM_FileBase (path, base);

	if (usehunk == 1)
		buf = Hunk_AllocName (len+1, base);
	else if (usehunk == 2)
		buf = Hunk_TempAlloc (len+1);
	else if (usehunk == 0)
		buf = Z_Malloc (len+1);
	else if (usehunk == 3)
		buf = Cache_Alloc (loadcache, len+1, base);
	else if (usehunk == 4)
	{
		if (len+1 > loadsize)
			buf = Hunk_TempAlloc (len+1);
		else
			buf = loadbuf;
	}
	else if (usehunk == 5)
	{	// Pa3PyX: like 4, except uses hunk (not temp) if no space
		if (len + 1 > loadsize)
			buf = Hunk_AllocName(len + 1, path);
		else
			buf = loadbuf;
	}
	else
		Sys_Error ("COM_LoadFile: bad usehunk");

	if (!buf)
		Sys_Error ("COM_LoadFile: not enough space for %s", path);

	((byte *)buf)[len] = 0;

#if !defined(SERVERONLY) && !defined(GLQUAKE)
	Draw_BeginDisc ();
#endif
	fread (buf, 1, len, h);
	fclose (h);
#if !defined(SERVERONLY) && !defined(GLQUAKE)
	Draw_EndDisc ();
#endif
	return buf;
}

byte *COM_LoadHunkFile (char *path)
{
	return COM_LoadFile (path, 1);
}

byte *COM_LoadTempFile (char *path)
{
	return COM_LoadFile (path, 2);
}

void COM_LoadCacheFile (char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	COM_LoadFile (path, 3);
}

// uses temp hunk if larger than bufsize
byte *COM_LoadStackFile (char *path, void *buffer, int bufsize)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = COM_LoadFile (path, 4);

	return buf;
}

// Pa3PyX: Like COM_LoadStackFile, excepts loads onto
// the hunk (instead of temp) if there is no space
byte *COM_LoadBufFile (char *path, void *buffer, int *bufsize)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = (*bufsize) + 1;
	buf = COM_LoadFile (path, 5);
	if (buf && !(*bufsize))
		*bufsize = com_filesize;

	return buf;
}

/*
=================
COM_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
static pack_t *COM_LoadPackFile (char *packfile, int paknum)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned short		crc;

	if (COM_FileOpenRead (packfile, &packhandle) == -1)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (header.id[0] != 'P' || header.id[1] != 'A' ||
	    header.id[2] != 'C' || header.id[3] != 'K')
		Sys_Error ("%s is not a packfile", packfile);

	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error ("%s has %i files", packfile, numpackfiles);

	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (&info, 1, header.dirlen, packhandle);

// crc the directory
	CRC_Init (&crc);
	for (i=0 ; i<header.dirlen ; i++)
		CRC_ProcessByte (&crc, ((byte *)info)[i]);

// check for modifications
	if (paknum <= MAX_PAKDATA-2)
	{
		if (strcmp(gamedirfile, dirdata[paknum]) != 0)
		{
			// raven didnt ship like that
			gameflags |= GAME_MODIFIED;
		}
		else if (numpackfiles != pakdata[paknum][0])
		{
			if (paknum == 0)
			{
				// demo ??
				if (numpackfiles != pakdata[MAX_PAKDATA-1][0])
				{
				// not original
					gameflags |= GAME_MODIFIED;
				}
				else if (crc != pakdata[MAX_PAKDATA-1][1])
				{
				// not original
					gameflags |= GAME_MODIFIED;
				}
				else
				{
				// both crc and numfiles matched the demo
					gameflags |= GAME_DEMO;
				}
			}
			else
			{
			// not original
				gameflags |= GAME_MODIFIED;
			}
		}
		else if (crc != pakdata[paknum][1])
		{
		// not original
			gameflags |= GAME_MODIFIED;
		}
		else
		{
			switch (paknum)
			{
			case 0:	// pak0 of full version 1.11
				gameflags |= GAME_REGISTERED0;
				break;
			case 1:	// pak1 of full version 1.11
				gameflags |= GAME_REGISTERED1;
				break;
			case 2:	// bundle version
				gameflags |= GAME_OEM;
				break;
			case 3:	// mission pack
				gameflags |= GAME_PORTALS;
				break;
			case 4:	// hexenworld
				gameflags |= GAME_HEXENWORLD;
				break;
			default:// we shouldn't reach here
				break;
			}
		}
		// both crc and numfiles are good, we are still original
	}
	else
	{
		gameflags |= GAME_MODIFIED;
	}

// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	Con_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}


/*
================
COM_AddGameDirectory

Sets com_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
================
*/
static void COM_AddGameDirectory (char *dir)
{
	int				i;
	searchpath_t		*search;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];
	char			*p;

	if ((p = strrchr(dir, '/')) != NULL)
		strcpy(gamedirfile, ++p);
	else
		strcpy(gamedirfile, p);
	strcpy (com_gamedir, dir);

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
	for (i=0 ; i < 10; i++)
	{
		sprintf (pakfile, "%s/pak%i.pak", dir, i);
		pak = COM_LoadPackFile (pakfile, i);
		if (!pak)
			continue;
		search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
		search->pack = pak;
		search->next = com_searchpaths;
		com_searchpaths = search;
	}

//
// add the directory to the search path
// O.S: this needs to be done ~after~ adding the pakfiles in this dir, so
// that the dir itself will be placed above the pakfiles in the search order
// which, in turn, will allow override files: this way, data1/default.cfg
// will be opened instead of data1/pak0.pak:/default.cfg
//
	search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
	strcpy (search->filename, dir);
	search->next = com_searchpaths;
	com_searchpaths = search;

//
// add user's directory to the search path
// we don't need to set it on win32 platforms since it's exactly com_gamedir
// FIXME: how about pak files in user's directory??
//
#ifdef PLATFORM_UNIX
	search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
	strcpy (search->filename, com_userdir);
	search->next = com_searchpaths;
	com_searchpaths = search;
#endif
}

/*
================
COM_Gamedir

Sets the gamedir and path to a different directory.
================
*/
void COM_Gamedir (char *dir)
{
	searchpath_t	*search, *next;
	int				i;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];

	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":") )
	{
		Con_Printf ("Gamedir should be a single filename, not a path\n");
		return;
	}

	if (!strcmp(gamedirfile, dir))
		return;		// still the same
	strcpy (gamedirfile, dir);

	//
	// free up any current game dir info
	//
	while (com_searchpaths != com_base_searchpaths)
	{
		if (com_searchpaths->pack)
		{
			fclose (com_searchpaths->pack->handle);
			Z_Free (com_searchpaths->pack->files);
			Z_Free (com_searchpaths->pack);
		}
		next = com_searchpaths->next;
		Z_Free (com_searchpaths);
		com_searchpaths = next;
	}

	//
	// flush all data, so it will be forced to reload
	//
	Cache_Flush ();

	sprintf (com_gamedir, "%s/%s", com_basedir, dir);

	if (!strcmp(dir,"data1") || ((gameflags & GAME_HEXENWORLD) && !strcmp(dir, "hw")))
		return;

	//
	// add the directory to the search path
	//
	search = Z_Malloc (sizeof(searchpath_t));
	strcpy (search->filename, com_gamedir);
	search->next = com_searchpaths;
	com_searchpaths = search;

	//
	// add any pak files in the format pak0.pak pak1.pak, ...
	//
	for (i=0 ; i < 10 ; i++)
	{
		sprintf (pakfile, "%s/pak%i.pak", com_gamedir, i);
		pak = COM_LoadPackFile (pakfile, i);
		if (!pak)
			continue;
		search = Z_Malloc (sizeof(searchpath_t));
		search->pack = pak;
		search->next = com_searchpaths;
		com_searchpaths = search;
	}
}

/*
================
COM_InitFilesystem
================
*/
static void COM_InitFilesystem (void)
{
	int		i;
	char		temp[12];

//
// -basedir <path>
// Overrides the system supplied base directory (under data1)
//
	i = COM_CheckParm ("-basedir");
	if (i && i < com_argc-1)
		strcpy (com_basedir, com_argv[i+1]);
	else
		strcpy (com_basedir, host_parms.basedir);

	strcpy (com_userdir, host_parms.userdir);

//
// start up with data1 by default
//
#ifdef _WIN32
	// Let's keep the game's old win32 behavior
	sprintf (com_userdir, "%s/data1", host_parms.userdir);
#endif
	COM_AddGameDirectory (va("%s/data1", com_basedir));

	// check if we are playing the registered version
	COM_CheckRegistered ();
	// check for mix'n'match screw-ups
	if ((gameflags & GAME_REGISTERED) && ((gameflags & GAME_DEMO) || (gameflags & GAME_OEM)))
		Sys_Error ("Bad Hexen II installation");
#ifndef SERVERONLY
	if ((gameflags & GAME_MODIFIED) && !(gameflags & GAME_REGISTERED))
		Sys_Error ("You must have the full version of Hexen II to play modified games");
#endif

#if defined(H2MP)
	// mission pack requires the registered version
	if (!(gameflags & GAME_REGISTERED) || (gameflags & GAME_DEMO) || (gameflags & GAME_OEM))
		Sys_Error ("Portal of Praevus requires registered version of Hexen II");
#endif

#if defined(H2MP) || defined(H2W)
	if (gameflags & GAME_REGISTERED)
	{
		sprintf (com_userdir, "%s/portals", host_parms.userdir);
		Sys_mkdir (com_userdir);
		COM_AddGameDirectory (va("%s/portals", com_basedir));
	}
#   if defined(H2MP)
	// error out for H2MP builds if GAME_PORTALS isn't set
	if (!(gameflags & GAME_PORTALS))
		Sys_Error ("Portal of Praevus game data not found");
#   endif
#endif

#if defined(H2W)
	sprintf (com_userdir, "%s/hw", host_parms.userdir);
	Sys_mkdir (com_userdir);
	COM_AddGameDirectory (va("%s/hw", com_basedir));
	// error out for H2W builds if GAME_HEXENWORLD isn't set
	if (!(gameflags & GAME_HEXENWORLD))
		Sys_Error ("You must have the HexenWorld data installed");
#endif

// -game <gamedir>
// Adds basedir/gamedir as an override game
//
	if (gameflags & GAME_REGISTERED)
	{
		i = COM_CheckParm ("-game");
		if (i && i < com_argc-1)
		{
			gameflags |= GAME_MODIFIED;
			sprintf (com_userdir, "%s/%s", host_parms.userdir, com_argv[i+1]);
			Sys_mkdir (com_userdir);
			COM_AddGameDirectory (va("%s/%s", com_basedir, com_argv[i+1]));
		}

	}

	strcpy(com_savedir,com_userdir);

	// any set gamedirs will be freed up to here
	com_base_searchpaths = com_searchpaths;

// finish the filesystem setup
	oem.flags &= ~CVAR_ROM;
	registered.flags &= ~CVAR_ROM;
	if (gameflags & GAME_REGISTERED)
	{
		sprintf (temp, "registered");
		Cvar_Set ("registered", "1");
	}
	else if (gameflags & GAME_OEM)
	{
		sprintf (temp, "oem");
		Cvar_Set ("oem", "1");
	}
	else if (gameflags & GAME_DEMO)
	{
		sprintf (temp, "demo");
	}
	else
	{	// Umm??
		sprintf (temp, "unknown");
	}
	oem.flags |= CVAR_ROM;
	registered.flags |= CVAR_ROM;

	Con_Printf ("Playing %s version.\n", temp);
}

#ifdef H2W
/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
char *Info_ValueForKey (char *s, char *key)
{
	char	pkey[512];
	static	char value[4][512];	// use two buffers so compares
								// work without stomping on each other
	static	int	valueindex;
	char	*o;
	
	valueindex = (valueindex + 1) % 4;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

void Info_RemoveKey (char *s, char *key)
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if (strstr (key, "\\"))
	{
		Con_Printf ("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_RemovePrefixedKeys (char *start, char prefix)
{
	char	*s;
	char	pkey[512];
	char	value[512];
	char	*o;

	s = start;

	while (1)
	{
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (pkey[0] == prefix)
		{
			Info_RemoveKey (start, pkey);
			s = start;
		}

		if (!*s)
			return;
	}

}

void Info_SetValueForStarKey (char *s, char *key, char *value, int maxsize)
{
	char	new[1024], *v;
	int		c;
#ifdef SERVERONLY
	extern cvar_t sv_highchars;
#endif

	if (strstr (key, "\\") || strstr (value, "\\") )
	{
		Con_Printf ("Can't use keys or values with a \\\n");
		return;
	}

	if (strstr (key, "\"") || strstr (value, "\"") )
	{
		Con_Printf ("Can't use keys or values with a \"\n");
		return;
	}

	if (strlen(key) > 63 || strlen(value) > 63)
	{
		Con_Printf ("Keys and values must be < 64 characters.\n");
		return;
	}
	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	sprintf (new, "\\%s\\%s", key, value);

	if (strlen(new) + strlen(s) > maxsize)
	{
		Con_Printf ("Info string length exceeded\n");
		return;
	}

	// only copy ascii values
	s += strlen(s);
	v = new;
	while (*v)
	{
		c = (unsigned char)*v++;
#ifndef SERVERONLY
		// client only allows highbits on name
		if (Q_strcasecmp(key, "name") != 0) {
			c &= 127;
			if (c < 32 || c > 127)
				continue;
			// auto lowercase team
			if (Q_strcasecmp(key, "team") == 0)
				c = tolower(c);
		}
#else
		if (!sv_highchars.value) {
			c &= 127;
			if (c < 32 || c > 127)
				continue;
		}
#endif
//		c &= 127;		// strip high bits
		if (c > 13) // && c < 127)
			*s++ = c;
	}
	*s = 0;
}

void Info_SetValueForKey (char *s, char *key, char *value, int maxsize)
{
	if (key[0] == '*')
	{
		Con_Printf ("Can't set * keys\n");
		return;
	}

	Info_SetValueForStarKey (s, key, value, maxsize);
}

void Info_Print (char *s)
{
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Con_Printf ("%s", key);

		if (!*s)
		{
			Con_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Con_Printf ("%s\n", value);
	}
}
#endif	// H2W

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.40  2006/03/25 09:25:38  sezero
 * small tidy-ups
 *
 * Revision 1.39  2006/03/25 09:16:29  sezero
 * Fixed COM_Gamedir() to properly update com_gamedir in case of directory
 * names already in com_base_searchpaths.
 *
 * Revision 1.38  2006/03/24 15:05:39  sezero
 * killed the archive, server and info members of the cvar structure.
 * the new flags member is now employed for all those purposes. also
 * made all non-globally used cvars static.
 *
 * Revision 1.37  2006/03/23 19:45:09  sezero
 * Marked the cvars oem and registered as read-only in COM_InitFilesystem
 *
 * Revision 1.36  2006/03/17 20:23:17  sezero
 * Updates to COM_LoadPackFile, COM_CheckRegistered and COM_InitFilesystem:
 * killed variables com_modified, com_portals and static_registered. Combined
 * all such info into an unsigned gameflags var. If correct versions of pak
 * files are installed at correct places, the state will be set: whether we
 * have the demo, oem or registered pakfiles, whether the registered game pak
 * files are version 1.11, whether it is a modified game (extra pak files in
 * the game directory or -game among the commandline switches), and whether
 * we have correct versions portals and hexnworld pakfiles. The game will
 * refuse to run on incorrect instances, h2mp will refuse to run if portals
 * isn't set and hexenworld will refuse to run if hexenworld isn't set.
 * Also activated the hexenworld progs crc check which was comment out (who
 * knows why): Server will refuse to run if it detects an incorrect crc for
 * its hwprogs.dat.
 *
 * Revision 1.35  2006/01/12 12:34:37  sezero
 * added video modes enumeration via SDL. added on-the-fly video mode changing
 * partially based on the Pa3PyX hexen2 tree. TODO: make the game remember its
 * video settings, clean it up, fix it up...
 *
 * Revision 1.34  2006/01/06 12:19:08  sezero
 * put the new Sys_FindFirstFile/Sys_FindNextFile stuff into action. also killed
 * the tempdir and trailing slash funnies in host_cmd.c when calling CL_CopyFiles
 * and CL_RemoveGIPFiles in saving and loading games.
 *
 * Revision 1.33  2005/12/29 07:12:01  sezero
 * cleaned up COM_CopyFile goto ugliness
 *
 * Revision 1.32  2005/12/28 22:17:04  sezero
 * fixed hexenworld server compilation
 *
 * Revision 1.31  2005/12/28 20:32:08  sezero
 * fixed a typo in the maplist function
 *
 * Revision 1.30  2005/12/28 20:00:15  sezero
 * added "maps" command to list all the maps in the searchpath.
 * from Steven. shortened and modified by O.S. to use fnmatch
 * instead of scandir and to make win32 to list maps not residing
 * in a pakfile.
 *
 * Revision 1.29  2005/12/28 14:20:23  sezero
 * made COM_CopyFile return int and added ferror() calls after every fread()
 * and fwrite() calls, so that CL_CopyFiles can behave correctly under unix.
 * made SaveGamestate return qboolean, replaced the silly "ERROR: couldn't
 * open" message by goto retry_message calls. made Host_Savegame_f to return
 * immediately upon SaveGamestate failure.
 *
 * Revision 1.28  2005/12/28 12:07:02  sezero
 * added COM_StrCompare as a quick'n'dirty string comparison function for use with qsort
 *
 * Revision 1.27  2005/12/04 11:20:57  sezero
 * init stuff cleanup
 *
 * Revision 1.26  2005/12/04 11:14:38  sezero
 * the big vsnprintf patch
 *
 * Revision 1.25  2005/10/25 20:08:41  sezero
 * coding style and whitespace cleanup.
 *
 * Revision 1.24  2005/10/25 20:04:17  sezero
 * static functions part-1: started making local functions static,
 * killing nested externs, const vars clean-up.
 *
 * Revision 1.23  2005/10/24 21:22:15  sezero
 * round to nearest value, rather than rounding toward zero while
 * sending angles and coords. (from the darkplaces project where
 * it fixes the crosshair problem.)
 *
 * Revision 1.22  2005/09/24 23:50:36  sezero
 * fixed a bunch of compiler warnings
 *
 * Revision 1.21  2005/08/20 13:06:33  sezero
 * favored unlink() over DeleteFile() on win32. removed unnecessary
 * platform defines for directory path separators. removed a left-
 * over CL_RemoveGIPFiles() from sys_win.c. fixed temporary gip files
 * not being removed and probably causing "bad" savegames on win32.
 *
 * Revision 1.20  2005/08/09 15:39:28  sezero
 * Prioritized a game directory over its pakfiles in the search order which is
 * the behavior of HoT-1.2.4 and older. This bug, carried over from hexenworld,
 * disallowed override files to be used. The ID guys probably had their concerns
 * while doing this, which Raven did change in hexen2 but missed in hexenworld.
 *
 * Revision 1.19  2005/08/07 10:59:04  sezero
 * killed the Sys_FileTime crap. now using standart access() function.
 *
 * Revision 1.18  2005/07/31 00:45:11  sezero
 * platform defines cleanup
 *
 * Revision 1.17  2005/07/23 22:22:08  sezero
 * unified the common funcntions for hexen2-hexenworld
 *
 * Revision 1.16  2005/07/15 16:55:31  sezero
 * documented the modified/unmodified detection
 *
 * Revision 1.15  2005/07/09 09:07:55  sezero
 * changed all Hunk_Alloc() usage to Hunk_AllocName() for easier memory usage tracing.
 *
 * Revision 1.14  2005/06/15 09:53:57  sezero
 * keep the game's old userdir behavior on win32
 *
 * Revision 1.13  2005/05/21 17:32:02  sezero
 * disabled the rotating skull annoyance in GL mode (used to
 * cause problems with voodoo1/mesa6 when using gamma tricks)
 *
 * Revision 1.12  2005/05/20 12:01:27  sezero
 * large initial syncing of common.c between h2 and hw:
 * a lot of whitespace clean-up, removal of activision
 * secure stuff, demoval of -path, -proghack, -savedir
 * and -cachedir support, better registered version
 * detection and crc checks, etc. There are some leftovers
 * after this, next patches will clean that up.
 *
 * Revision 1.11  2005/05/17 22:56:19  sezero
 * cleanup the "stricmp, strcmpi, strnicmp, Q_strcasecmp, Q_strncasecmp" mess:
 * Q_strXcasecmp will now be used throughout the code which are implementation
 * dependant defines for __GNUC__ (strXcasecmp) and _WIN32 (strXicmp)
 *
 * Revision 1.10  2005/04/13 12:18:30  sezero
 * tiny clean-up in COM_InitFilesystem ...
 *
 * Revision 1.9  2005/04/05 19:44:14  sezero
 * removed my previous restrictions about adding
 * userpaths to the searchpath (was added in 1.2.3)
 *
 * Revision 1.8  2005/04/05 19:28:40  sezero
 * clean-ups in endianness which now is decided at compile time
 *
 * Revision 1.7  2005/02/05 16:21:13  sezero
 * killed Com_LoadHunkFile2()  [from HexenWorld]
 *
 * Revision 1.6  2005/02/05 16:17:29  sezero
 * - Midi file paths cleanup. these should be leftovers
 *   from times when gamedir and userdir were the same.
 * - Killed Com_WriteFileFullPath(), not used anymore.
 * - Replaced some Con_Printf() with Sys_Printf().
 *
 * Revision 1.5  2005/01/18 11:29:18  sezero
 * - Fix userdirs to work correctly against the -game arg
 * - Added a "qboolean adduser" arg to COM_AddGameDir()  (useful esp. in
 *   hexenworld):  com_userdir evolves gradually. Every search path added
 *   has a consequence of additon of (multiple) unnecessary userpaths.
 *   Avoid it where we can. (original aot also added the very same userdir
 *   multiple times)
 * - Changed all instances of "id1" to "data1"
 *
 * Revision 1.4  2004/12/18 14:15:34  sezero
 * Clean-up and kill warnings 10:
 * Remove some already commented-out functions and code fragments.
 * They seem to be of no-future use. Also remove some unused functions.
 *
 * Revision 1.3  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.2  2004/11/28 00:34:59  sezero
 * no range checking. used to trash the stack and crash the game randomly upon loading progs. code borrowed from Pa3PyX sources.
 *
 * Revision 1.1.1.1  2004/11/28 00:01:25  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.6  2002/01/04 14:50:55  phneutre
 * write files in ~/.aot/ directory by default (in COM_WriteFile)
 *
 * Revision 1.5  2001/12/13 20:04:43  phneutre
 * search, load and write savegames in ~/.aot/s* instead of data1/s*
 *
 * Revision 1.4  2001/12/13 17:26:20  phneutre
 * implemented COM_WriteFileFullPath to copy files in ~/.aot
 *
 * Revision 1.3  2001/12/13 17:08:41  phneutre
 * add com_userdir to the search path
 *
 * Revision 1.2  2001/12/13 16:19:07  phneutre
 * new com_userdir variable, please use this one to access user's ~/.aot directory.
 *
 * Revision 1.1.1.1  2001/11/09 17:03:57  theoddone33
 * Inital import
 *
 * 
 * 15    3/19/98 4:28p Jmonroe
 * 
 * 14    3/19/98 1:04p Jmonroe
 * changed gamedir to be a server var for gamespy
 * 
 * 13    3/19/98 12:53p Jmonroe
 * 
 * 12    3/17/98 6:07p Jmonroe
 * 
 * 11    3/17/98 6:06p Jmonroe
 * 
 * 10    3/17/98 1:24p Jmonroe
 * added a qw cvar for gamespy
 * 
 * 9     3/16/98 6:25p Jmonroe
 * set up euro secure checks
 * 
 * 8     3/05/98 5:13p Jmonroe
 * added pr build security shit
 * 
 * 7     3/05/98 12:11p Jmonroe
 * 
 * 6     3/04/98 11:49a Jmonroe
 * 
 * 5     3/03/98 1:40p Jmonroe
 * removed old mp stuff
 * 
 * 4     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 3     1/18/98 8:05p Jmonroe
 * all of rick's patch code is in now
 * 
 * 36    10/03/97 4:07p Rjohnson
 * CRC Update
 * 
 * 35    9/30/97 4:26p Rjohnson
 * Updates
 * 
 * 34    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 33    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 32    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 31    9/02/97 12:24a Rjohnson
 * Version Update
 * 
 * 30    8/29/97 2:49p Rjohnson
 * Network updates
 * 
 * 29    8/27/97 12:13p Rjohnson
 * Version Updates
 * 
 * 28    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 27    8/21/97 10:13p Rjohnson
 * Version Update
 * 
 * 26    8/20/97 2:59p Rjohnson
 * Version Update
 * 
 * 25    8/16/97 10:53a Rjohnson
 * Version Update
 * 
 * 24    8/14/97 3:52p Rjohnson
 * Update for demo
 * 
 * 23    8/11/97 9:50a Rjohnson
 * Version Update
 * 
 * 22    8/06/97 10:58a Rjohnson
 * Version control updates
 * 
 * 21    8/02/97 3:47p Rjohnson
 * Post Beta
 * 
 * 20    8/02/97 11:20a Rjohnson
 * Encryption Update
 * 
 * 19    8/01/97 4:30p Rjohnson
 * More encryption work
 * 
 * 18    7/30/97 2:05p Rjohnson
 * Version Update
 * 
 * 17    7/30/97 1:54p Rjohnson
 * Added encryption
 * 
 * 16    7/24/97 12:37a Rjohnson
 * Raven Build
 * 
 * 15    7/24/97 12:10a Rjohnson
 * Activision Update
 * 
 * 14    6/19/97 10:13a Rjohnson
 * Post E3
 * 
 * 13    6/19/97 10:12a Rjohnson
 * Activision H3 Demo
 * 
 * 12    5/01/97 12:29a Bgokey
 * 
 * 11    4/14/97 5:11p Rjohnson
 * Id Updates
 * 
 * 10    3/22/97 2:41p Rjohnson
 * Back to raven build
 * 
 * 9     3/21/97 6:02p Rjohnson
 * Updated internal stamp
 * 
 * 8     3/07/97 12:30p Rjohnson
 * Id Updates
 * 
 * 7     3/06/97 10:29a Rjohnson
 * Added memory statistics
 * 
 * 6     2/27/97 4:04p Rjohnson
 * Added a new type of load function to also give you the size
 * 
 * 5     2/17/97 3:14p Rjohnson
 * Id Updates
 * 
 * 4     2/07/97 8:23a Rlove
 * Resetting Version stuff from Activision send to normal Raven build
 * 
 * 3     2/06/97 4:05p Rjohnson
 * Added a cheap serialization
 */
