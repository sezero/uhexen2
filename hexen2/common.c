/*
	common.c
	misc functions used in client and server

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/common.c,v 1.6 2005-02-05 16:17:29 sezero Exp $
*/

#include "quakedef.h"
#ifndef PLATFORM_UNIX
#include <windows.h>
#endif

#define NUM_SAFE_ARGVS  7

static char     *largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static char     *argvdummy = " ";

static char     *safeargvs[NUM_SAFE_ARGVS] =
	{"-nomidi", "-nolan", "-nosound", "-nocdaudio", "-nojoy", "-nomouse", "-dibonly"};

extern cvar_t	sys_nostdout;	// - DDOI	
cvar_t  registered = {"registered","0"};
#ifdef H2MP
cvar_t  sv_gamedir = {"*gamedir","portals",false,true};
#else
cvar_t	sv_gamedir = {"*gamedir","data1",false,true};
#endif
cvar_t  oem = {"oem","0"};
cvar_t  cmdline = {"cmdline","0", false, true};

qboolean        com_modified;   // set true if using non-id files

qboolean		proghack;

int             static_registered = 1;  // only for startup check, then set

qboolean		msg_suppress_1 = 0;

void COM_InitFilesystem (void);

// if a packfile directory differs from this, it is assumed to be hacked
// demo
//#define PAK0_COUNT              701
//#define PAK0_CRC                20870
// This count didn't match the demo - DDOI
#define PAK0DEM_COUNT		797
#define PAK0DEM_CRC                22780

// retail
#define PAK0_COUNT              697
#define PAK0_CRC                9787

#define PAK2_COUNT              183
#define PAK2_CRC                4807

char	com_token[1024];
int		com_argc;
char	**com_argv;

#define CMDLINE_LENGTH	256
char	com_cmdline[CMDLINE_LENGTH];

#ifdef ACTIVISION
	byte *EncryptKey;
	int KeySize = 0;
#endif

// Build Version
//    BUILD_RAVEN      
//       Internal version we build everyday
//    BUILD_ACTIVISION_1            2/6/1997 
//       Version was sent to activision
//    BUILD_ID_1					3/21/1997 
//       Version was sent to activision
//    BUILD_ID_2					4/14/1997 
//       Version was sent to activision
//    BUILD_ACTIVISION_2            6/17/1997 
//       Version was sent to activision for e3
//    BUILD_RAVEN_2					6/19/1997 
//       Post E3 Raven build
//    BUILD_ACTIVISION_3            7/23/1997 
//       Alpha 2 sent to activision
//    BUILD_RAVEN_3					7/23/1997 
//       Post Alpha 2 Raven build
//    BUILD_ACTIVISION_4            7/30/1997 
//       Network fix for activision
//    BUILD_RAVEN_4					7/30/1997 
//       Post Network fix
//    BUILD_ACTIVISION_5            8/1/1997 
//       "Beta" version
//    BUILD_RAVEN_5					8/1/1997 
//       Post "Beta"
//    BUILD_ACTIVISION_6            8/5/1997 
//       IPX Networking Update
//    BUILD_RAVEN_6					8/6/1997 
//       Post Networking fix
//    BUILD_ACTIVISION_7            8/9/1997 
//       Beta 2
//    BUILD_RAVEN_7					8/11/1997 
//       Post Networking fix
//    BUILD_ACTIVISION_8            8/16/1997 
//       Beta 3
//    BUILD_RAVEN_8					8/16/1997 
//       Post Beta 3
//    BUILD_ACTIVISION_9            8/18/1997 
//       Demo Beta 2
//    BUILD_ACTIVISION_10           8/19/1997 
//       Demo Beta 3
//    BUILD_RAVEN_9					8/20/1997 
//       Post Beta 3
//    BUILD_ACTIVISION_11           8/21/1997 
//       Demo Beta 4
//    BUILD_ACTIVISION_12           8/21/1997 
//       Beta 4
//    BUILD_ACTIVISION_13           8/25/1997 
//       Demo Beta 5
//    BUILD_ACTIVISION_14           3/5/1998
//       Press release MP

char H3_VERSION[] = "BUILD_ACTIVISION_14";

// Build Number
//    0 - BUILD_RAVEN
//    1 - BUILD_ACTIVISION_1
//    2 - BUILD_ID_1
//    3 - BUILD_ACTIVISION_2
//    4 - BUILD_RAVEN_2
//    5 - BUILD_ACTIVISION_3
//    6 - BUILD_RAVEN_3
//    7 - BUILD_ACTIVISION_4
//    8 - BUILD_RAVEN_4
//    9 - BUILD_ACTIVISION_5
//   10 - BUILD_RAVEN_5
//   11 - BUILD_ACTIVISION_6
//   12 - BUILD_RAVEN_6
//   13 - BUILD_ACTIVISION_7
//   14 - BUILD_RAVEN_7
//   15 - BUILD_ACTIVISION_8
//   16 - BUILD_RAVEN_8
//   17 - BUILD_ACTIVISION_9
//   18 - BUILD_ACTIVISION_10
//   19 - BUILD_RAVEN_9
//   20 - BUILD_ACTIVISION_11
//   21 - BUILD_ACTIVISION_12
//   22 - BUILD_ACTIVISION_13
//   22 - BUILD_ACTIVISION_14
int H3_BUILD = 23;

// this graphic needs to be in the pak file to use registered features
unsigned short pop[] =
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

#ifdef ACTIVISION
	void DecryptFile(char *FileName, byte *Buffer, int BufSize);
#endif

/*
All of Quake's data access is through a hierchal file system, but the contents of the 
file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game 
directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This 
can be overridden with the "-basedir" command line parm to allow code debugging in a 
different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated 
files (savegames, screenshots, demos, config files) will be saved to.  This can be 
overridden with the "-game" command line parameter.  The game directory can never be 
changed while quake is executing.  This is a precacution against having a malicious 
server instruct clients to write files over areas they shouldn't.

The "cache directory" is only used during development to save network bandwidth, 
especially over ISDN / T1 lines.  If there is a cache directory
specified, when a file is found by the normal search path, it will be mirrored
into the cache directory, then opened there.


FIXME:
The file "parms.txt" will be read out of the game directory and appended to the current 
command line arguments to allow different games to initialize startup parms differently.  
This could be used to add a "-sspeed 22050" for the high quality sound edition.  Because 
they are added at the end, they will not override an explicit setting on the original 
command line.	
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

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

/*
void Q_memset (void *dest, int fill, int count)
{
	int             i;
	
	if ( (((long)dest | count) & 3) == 0)
	{
		count >>= 2;
		fill = fill | (fill<<8) | (fill<<16) | (fill<<24);
		for (i=0 ; i<count ; i++)
			((int *)dest)[i] = fill;
	}
	else
		for (i=0 ; i<count ; i++)
			((byte *)dest)[i] = fill;
}

void Q_memcpy (void *dest, void *src, int count)
{
	int             i;
	
	if (( ( (long)dest | (long)src | count) & 3) == 0 )
	{
		count>>=2;
		for (i=0 ; i<count ; i++)
			((int *)dest)[i] = ((int *)src)[i];
	}
	else
		for (i=0 ; i<count ; i++)
			((byte *)dest)[i] = ((byte *)src)[i];
}

int Q_memcmp (void *m1, void *m2, int count)
{
	while(count)
	{
		count--;
		if (((byte *)m1)[count] != ((byte *)m2)[count])
			return -1;
	}
	return 0;
}

void Q_strcpy (char *dest, char *src)
{
	while (*src)
	{
		*dest++ = *src++;
	}
	*dest++ = 0;
}

void Q_strncpy (char *dest, char *src, int count)
{
	while (*src && count--)
	{
		*dest++ = *src++;
	}
	if (count)
		*dest++ = 0;
}

int Q_strlen (char *str)
{
	int             count;
	
	count = 0;
	while (str[count])
		count++;

	return count;
}

char *Q_strrchr(char *s, char c)
{
    int len = Q_strlen(s);
    s += len;
    while (len--)
	if (*--s == c) return s;
    return 0;
}

void Q_strcat (char *dest, char *src)
{
	dest += Q_strlen(dest);
	Q_strcpy (dest, src);
}

int Q_strcmp (char *s1, char *s2)
{
	while (1)
	{
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 0;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}

int Q_strncmp (char *s1, char *s2, int count)
{
	while (1)
	{
		if (!count--)
			return 0;
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 0;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}

int Q_strncasecmp (char *s1, char *s2, int n)
{
	int             c1, c2;
	
	while (1)
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;               // strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;              // strings not equal
		}
		if (!c1)
			return 0;               // strings are equal
//              s1++;
//              s2++;
	}
	
	return -1;
}

int Q_strcasecmp (char *s1, char *s2)
{
	return Q_strncasecmp (s1, s2, 99999);
}

int Q_atoi (char *str)
{
	int             val;
	int             sign;
	int             c;
	
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

//
// check for hex
//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val<<4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val<<4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val<<4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
//
// check for character
//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}
	
//
// assume decimal
//
	while (1)
	{
		c = *str++;
		if (c <'0' || c > '9')
			return val*sign;
		val = val*10 + c - '0';
	}
	
	return 0;
}


float Q_atof (char *str)
{
	double			val;
	int             sign;
	int             c;
	int             decimal, total;
	
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

//
// check for hex
//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val*16) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val*16) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val*16) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
//
// check for character
//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}
	
//
// assume decimal
//
	decimal = -1;
	total = 0;
	while (1)
	{
		c = *str++;
		if (c == '.')
		{
			decimal = total;
			continue;
		}
		if (c <'0' || c > '9')
			break;
		val = val*10 + c - '0';
		total++;
	}

	if (decimal == -1)
		return val*sign;
	while (total > decimal)
	{
		val /= 10;
		total--;
	}
	
	return val*sign;
}
*/

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

qboolean        bigendien;

short   (*BigShort) (short l);
short   (*LittleShort) (short l);
int     (*BigLong) (int l);
int     (*LittleLong) (int l);
float   (*BigFloat) (float l);
float   (*LittleFloat) (float l);

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short   ShortNoSwap (short l)
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

int     LongNoSwap (int l)
{
	return l;
}

float FloatSwap (float f)
{
	union
	{
		float   f;
		byte    b[4];
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
	byte    *buf;
	
#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte    *buf;
	
#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte    *buf;
	
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
	byte    *buf;
	
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
		float   f;
		int     l;
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
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, ((int)f*256/360) & 255);
}

//
// reading functions
//
int                     msg_readcount;
qboolean        msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int     c;
	
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
	int     c;
	
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
	int     c;
	
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
	int     c;
	
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
		byte    b[4];
		float   f;
		int     l;
	} dat;
	
	dat.b[0] =      net_message.data[msg_readcount];
	dat.b[1] =      net_message.data[msg_readcount+1];
	dat.b[2] =      net_message.data[msg_readcount+2];
	dat.b[3] =      net_message.data[msg_readcount+3];
	msg_readcount += 4;
	
	dat.l = LittleLong (dat.l);

	return dat.f;   
}

char *MSG_ReadString (void)
{
	static char     string[2048];
	int             l,c;
	
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

float MSG_ReadCoord (void)
{
	return MSG_ReadShort() * (1.0/8);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0/256);
}



//===========================================================================

void SZ_Alloc (sizebuf_t *buf, int startsize)
{
	if (startsize < 256)
		startsize = 256;
	buf->data = Hunk_AllocName (startsize, "sizebuf");
	buf->maxsize = startsize;
	buf->cursize = 0;
	buf->effectsize = 0;
	buf->effectdata = false;
}


void SZ_Free (sizebuf_t *buf)
{
//      Z_Free (buf->data);
//      buf->data = NULL;
//      buf->maxsize = 0;
	buf->cursize = 0;
	buf->effectsize = 0;
	buf->effectdata = false;
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->effectsize = 0;
	buf->effectdata = false;
}

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void    *data;
	
	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("SZ_GetSpace: overflow without allowoverflow set\n");
		
		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %i is > full buffer size\n", length);
			
		buf->overflowed = true;
		Con_Printf ("SZ_GetSpace: overflow\nCurrently %d of %d, requested %d\n",buf->cursize,buf->maxsize,length);
		SZ_Clear (buf); 
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;
	if (buf->effectdata)
		buf->effectsize += length;
	
	return data;
}

void SZ_Write (sizebuf_t *buf, void *data, int length)
{
	memcpy (SZ_GetSpace(buf,length),data,length);         
}

void SZ_Print (sizebuf_t *buf, char *data)
{
	int             len;
	
	len = strlen(data)+1;

// byte * cast to keep VC++ happy
	if (buf->data[buf->cursize-1])
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
	char    *last;
	
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
	int             i;

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
	char *s, *s2;
	
	s = in + strlen(in) - 1;
	
	while (s != in && *s != '.')
		s--;
	
	/* Pa3PyX: no range checking -- used to trash the stack and crash the
		   game randomly upon loading progs, for instance (or in any
		   other instance where one would supply a filename witout
		   a path */
//	for (s2 = s ; *s2 && *s2 != '/' ; s2--);
	for (s2 = s; *s2 && *s2 != '/' && s2 >= in; s2--);
	
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
	char    *src;
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


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	int             c;
	int             len;
	
	len = 0;
	com_token[0] = 0;
	
	if (!data)
		return NULL;
		
// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;                    // end of file;
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
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c==':')
			break;
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
	int             i;
	
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;               // NEXTSTEP sometimes clears appkit vars.
		if (!strcmp (parm,com_argv[i]))
			return i;
	}
		
	return 0;
}

/*
================
COM_CheckRegistered

Looks for the pop.txt file and verifies it.
Sets the "registered" cvar.
Immediately exits out if an alternate game was attempted to be started without
being registered.
================
*/
void COM_CheckRegistered (void)
{
	int             h;
	unsigned short  check[128];
	int                     i;

	COM_OpenFile("gfx/pop.lmp", &h);
	static_registered = 0;

//-basedir c:\h3\test -game data1 +exec rick.cfg
	if (h == -1)
	{
#if WINDED
	Sys_Error ("This dedicated server requires a full registered copy of Hexen II");
#endif
		Con_Printf ("Playing demo version.\n");

		if (com_modified)
			Sys_Error ("You must have the full version to use modified games");
		return;
	}

	Sys_FileRead (h, check, sizeof(check));
	COM_CloseFile (h);

#ifdef ACTIVISION
	DecryptFile("gfx/pop.lmp", (byte *)check, sizeof(check));
#endif
	
	for (i=0 ; i<128 ; i++)
		if (pop[i] != (unsigned short)BigShort (check[i]))
			Sys_Error ("Corrupted data file.");
	
	static_registered = 1;
	Cvar_Set ("cmdline", com_cmdline);

	Cvar_Set ("registered", "1");
	Con_Printf ("Playing retail version.\n");
}


void COM_Path_f (void);


/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, char **argv)
{
	qboolean        safe;
	int             i, j, n;

// reconstitute the command line for the cmdline externally visible cvar
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

extern qboolean	LegitCopy;

#ifdef ACTIVISION

qboolean ReadEncryption(char *Position)
{
	FILE *FH;
	char temp[MAX_PATH];

#ifdef PLATFORM_UNIX
	sprintf(temp,"%sinstall/data2.cab",Position);
#else
	sprintf(temp,"%sinstall\\data2.cab",Position);
#endif
	
	FH = fopen(temp,"rb");
	if (!FH)
	{
		KeySize = 0;
		return false;
	}
	fseek(FH,0,SEEK_END);
	KeySize = ftell(FH);
	fseek(FH,0,SEEK_SET);

	EncryptKey = (byte *)malloc(KeySize);
	if (!EncryptKey)
	{
		Con_Printf("Could not alloc %d bytes\n",KeySize);
		KeySize = 0;
		fclose(FH);

		return false;
	}

	fread(EncryptKey,1,KeySize,FH);
	fclose(FH);

	return true;
}

void FindEncryption(void)
{
	char Drives[2048],*Position;
	char VolumeName[MAX_PATH],FileSystemName[MAX_PATH];
	DWORD VolumeSerialNumber,MaximumComponentLength,FileSystemFlags;
	int DriveType;
	BOOL Result;

	GetLogicalDriveStrings(sizeof(Drives),Drives);

	Position = Drives;
	while(strlen(Position) != 0)
	{
		DriveType = GetDriveType(Position);
		if (DriveType == DRIVE_CDROM)
		{
			Result = GetVolumeInformation(Position,VolumeName,sizeof(VolumeName),&VolumeSerialNumber,
				&MaximumComponentLength,&FileSystemFlags,FileSystemName,sizeof(FileSystemName));

			if (Result && strcmpi(VolumeName,"Hexen II") == 0)
			    //&& VolumeSerialNumber == 0x24214a8b)
			{
				if (ReadEncryption(Position))
					return;
			}
		}
		Position += strlen(Position)+1;
	}

	KeySize = 1024;
	EncryptKey = (byte *)malloc(1024);

	LegitCopy = false;
}
#endif

#ifdef SECURE
void FindCD(void)
{
	char Drives[2048],*Position;
	char VolumeName[MAX_PATH],FileSystemName[MAX_PATH];
	DWORD VolumeSerialNumber,MaximumComponentLength,FileSystemFlags;
	int DriveType;
	BOOL Result;


	GetLogicalDriveStrings(sizeof(Drives),Drives);

	Position = Drives;
	while(strlen(Position) != 0)
	{
		DriveType = GetDriveType(Position);
		if (DriveType == DRIVE_CDROM)
		{
			Result = GetVolumeInformation(Position,VolumeName,sizeof(VolumeName),&VolumeSerialNumber,
				&MaximumComponentLength,&FileSystemFlags,FileSystemName,sizeof(FileSystemName));

//			if (Result && strcmpi(VolumeName,"Hexen II") == 0)
//			if (Result && strcmpi(VolumeName,"m3D_2") == 0)
//			if (Result && strcmpi(VolumeName,"dkreign") == 0)
			if (Result && strcmpi(VolumeName,"H2MP") == 0)
			    //&& VolumeSerialNumber == 0x24214a8b)
			{
				FILE *FH;
				char temp[MAX_PATH];
#ifdef PLATFORM_UNIX
				sprintf(temp,"%sInstall/Cdrom.spd",Position);
#else
				sprintf(temp,"%sInstall\\Cdrom.spd",Position);
#endif
				
				FH = fopen(temp,"rb");
				if (FH)
				{
					fclose(FH);
					return;
				}
			}
		}
		Position += strlen(Position)+1;
	}

	errormessage = "You need to have the Hexen 2 Mission Pack CD in order to play!";

	LegitCopy = false;
}
#endif

/*
================
COM_Init
================
*/
void COM_Init (char *basedir)
{
	byte    swaptest[2] = {1,0};

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

	Cvar_RegisterVariable (&registered);
	Cvar_RegisterVariable (&oem);
	Cvar_RegisterVariable (&cmdline);
	Cvar_RegisterVariable (&sv_gamedir);
	Cmd_AddCommand ("path", COM_Path_f);

	COM_InitFilesystem ();
	COM_CheckRegistered ();
}


/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char    *va(char *format, ...)
{
	va_list         argptr;
	static char             string[1024];
	
	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

	return string;  
}


/// just for debugging
int     memsearch (byte *start, int count, int search)
{
	int             i;
	
	for (i=0 ; i<count ; i++)
		if (start[i] == search)
			return i;
	return -1;
}

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/

int     com_filesize;


//
// in memory
//

typedef struct
{
	char    name[MAX_QPATH];
	int             filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char    filename[MAX_OSPATH];
	int             handle;
	int             numfiles;
	packfile_t      *files;
} pack_t;

//
// on disk
//
typedef struct
{
	char    name[56];
	int             filepos, filelen;
} dpackfile_t;

typedef struct
{
	char    id[4];
	int             dirofs;
	int             dirlen;
} dpackheader_t;

#define MAX_FILES_IN_PACK       2048

char    com_cachedir[MAX_OSPATH];
char    com_gamedir[MAX_OSPATH];
char    com_userdir[MAX_OSPATH];
char    com_savedir[MAX_OSPATH];

typedef struct searchpath_s
{
	char    filename[MAX_OSPATH];
	pack_t  *pack;          // only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t    *com_searchpaths;

/*
============
COM_Path_f

============
*/
void COM_Path_f (void)
{
	searchpath_t    *s;
	
	Con_Printf ("Current search path:\n");
	for (s=com_searchpaths ; s ; s=s->next)
	{
		if (s->pack)
		{
			Con_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		}
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
	int             handle;
	char    name[MAX_OSPATH];
	
	sprintf (name, "%s/%s", com_userdir, filename);

	handle = Sys_FileOpenWrite (name);
	if (handle == -1)
	{
		Sys_Printf ("COM_WriteFile: failed on %s\n", name);
		return;
	}
	
	Sys_Printf ("COM_WriteFile: %s\n", name);
	Sys_FileWrite (handle, data, len);
	Sys_FileClose (handle);
}

/*
============
COM_CreatePath

Only used for CopyFile
============
*/
void    COM_CreatePath (char *path)
{
	char    *ofs;
	
	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{       // create the directory
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
needed.  This is for the convenience of developers using ISDN from home.
===========
*/
void COM_CopyFile (char *netpath, char *cachepath)
{
	int             in, out;
	int             remaining, count;
	char    buf[4096];
	
	remaining = Sys_FileOpenRead (netpath, &in);            
	COM_CreatePath (cachepath);     // create directories up to the cache file
	out = Sys_FileOpenWrite (cachepath);
	
	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);
		Sys_FileRead (in, buf, count);
		Sys_FileWrite (out, buf, count);
		remaining -= count;
	}

	Sys_FileClose (in);
	Sys_FileClose (out);    
}

/*
===========
COM_FindFile

Finds the file in the search path.
Sets com_filesize and one of handle or file
===========
*/
int COM_FindFile (char *filename, int *handle, FILE **file, qboolean override_pack)
{
	searchpath_t    *search;
	char            netpath[MAX_OSPATH];
	char            cachepath[MAX_OSPATH];
	pack_t          *pak;
	int                     i;
	int                     findtime, cachetime;

	if (file && handle)
		Sys_Error ("COM_FindFile: both handle and file set");
	if (!file && !handle)
		Sys_Error ("COM_FindFile: neither handle or file set");
		
//
// search through the path, one element at a time
//
	search = com_searchpaths;
	if (proghack)
	{	// gross hack to use quake 1 progs with quake 2 maps
		if (!strcmp(filename, "progs.dat"))
			search = search->next;
	}

	for ( ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
				if (!strcmp (pak->files[i].name, filename))
				{       // found it!
					if(!sys_nostdout.value) Sys_Printf ("PackFile: %s : %s\n",pak->filename, filename);
					if (handle)
					{
						*handle = pak->handle;
						Sys_FileSeek (pak->handle, pak->files[i].filepos);
					}
					else
					{       // open a new file on the pakfile
						*file = fopen (pak->filename, "rb");
						if (*file)
							fseek (*file, pak->files[i].filepos, SEEK_SET);
					}
					com_filesize = pak->files[i].filelen;
					return com_filesize;
				}
		}
		else
		{               
	// check a file in the directory tree
			if (!static_registered && !override_pack)
			{       // if not a registered version, don't ever go beyond base
#ifdef PLATFORM_UNIX
				if ( strchr (filename, '/'))
#else
				if ( strchr (filename, '/') || strchr (filename,'\\'))
#endif
					continue;
			}
			
			sprintf (netpath, "%s/%s",search->filename, filename);
			
			findtime = Sys_FileTime (netpath);
			if (findtime == -1)
				continue;
				
		// see if the file needs to be updated in the cache
			if (!com_cachedir[0])
				strcpy (cachepath, netpath);
			else
			{	
#if defined(_WIN32)
				if ((strlen(netpath) < 2) || (netpath[1] != ':'))
					sprintf (cachepath,"%s%s", com_cachedir, netpath);
				else
					sprintf (cachepath,"%s%s", com_cachedir, netpath+2);
#else
				sprintf (cachepath,"%s%s", com_cachedir, netpath);
#endif

				cachetime = Sys_FileTime (cachepath);
			
				if (cachetime < findtime)
					COM_CopyFile (netpath, cachepath);
				strcpy (netpath, cachepath);
			}	

			if (!sys_nostdout.value) Sys_Printf ("FindFile: %s\n",netpath);
			com_filesize = Sys_FileOpenRead (netpath, &i);
			if (handle)
				*handle = i;
			else
			{
				Sys_FileClose (i);
				*file = fopen (netpath, "rb");
			}
			return com_filesize;
		}
		
	}
	
	Sys_Printf ("FindFile: can't find %s\n", filename);
	
	if (handle)
		*handle = -1;
	else
		*file = NULL;
	com_filesize = -1;
	return -1;
}


/*
===========
COM_OpenFile

filename never has a leading slash, but may contain directory walks
returns a handle and a length
it may actually be inside a pak file
===========
*/
int COM_OpenFile (char *filename, int *handle)
{
	return COM_FindFile (filename, handle, NULL, false);
}

/*
===========
COM_FOpenFile

If the requested file is inside a packfile, a new FILE * will be opened
into the file.
===========
*/
int COM_FOpenFile (char *filename, FILE **file, qboolean override_pack)
{
	return COM_FindFile (filename, NULL, file, override_pack);
}

/*
============
COM_CloseFile

If it is a pak file handle, don't really close it
============
*/
void COM_CloseFile (int h)
{
	searchpath_t    *s;
	
	for (s = com_searchpaths ; s ; s=s->next)
		if (s->pack && s->pack->handle == h)
			return;
			
	Sys_FileClose (h);
}

#ifdef ACTIVISION

#define MAX_LIST 6

char *FileList[MAX_LIST] =
{
	".mdl",
	".bsp",
	".spr",
	".lmp",
	".wad",
	".wav"
};

void DecryptFile(char *FileName, byte *Buffer, int BufSize)
{
	int Length,i,Value,Previous;

	Length = strlen(FileName);
	if (Length < 4) return;
	
	for(i=0;i<MAX_LIST;i++)
		if (strcmpi(&FileName[Length-4],FileList[i]) == 0)
			break;

	if (i >= MAX_LIST)
		return;

	Previous = 0;
	for(i=0;i<BufSize;i++)
	{
		Value = Buffer[i];

		Value = Value - Previous - EncryptKey[i % KeySize] - BufSize;

		Buffer[i] = Value & 255;
		Previous = Buffer[i];
	}
}
#endif

/*
============
COM_LoadFile

Filename are reletive to the quake directory.
Allways appends a 0 byte.
============
*/
cache_user_t *loadcache;
byte    *loadbuf;
int             loadsize;
byte *COM_LoadFile (char *path, int usehunk, int *size)
{
	int             h;
	byte    *buf;
	char    base[32];
	int             len;

	buf = NULL;     // quiet compiler warning

// look for it in the filesystem or pack files
	len = COM_OpenFile (path, &h);
	if (size) *size = len;
	if (h == -1)
		return NULL;
	
// extract the filename base name for hunk tag
	COM_FileBase (path, base);
	
	if (usehunk == 1)
		buf = Hunk_AllocName (len+1, path);
	else if (usehunk == 2)
		buf = Hunk_TempAlloc (len+1);
	else if (usehunk == 0)
		buf = Z_Malloc (len+1);
	else if (usehunk == 3)
		buf = Cache_Alloc (loadcache, len+1, path);
	else if (usehunk == 4)
	{
		if (len+1 > loadsize)
			buf = Hunk_TempAlloc (len+1);
		else
			buf = loadbuf;
	}
	else
		Sys_Error ("COM_LoadFile: bad usehunk");

	if (!buf)
		Sys_Error ("COM_LoadFile: not enough space for %s", path);
		
	((byte *)buf)[len] = 0;

	Draw_BeginDisc ();
	Sys_FileRead (h, buf, len);                     
	COM_CloseFile (h);
#ifdef ACTIVISION
	DecryptFile(path, buf, len);
#endif
	Draw_EndDisc ();

	return buf;
}

byte *COM_LoadHunkFile (char *path)
{
	return COM_LoadFile (path, 1, NULL);
}

byte *COM_LoadHunkFile2 (char *path, int *size)
{
	return COM_LoadFile (path, 1, size);
}

byte *COM_LoadTempFile (char *path)
{
	return COM_LoadFile (path, 2, NULL);
}

void COM_LoadCacheFile (char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	COM_LoadFile (path, 3, NULL);
}

// uses temp hunk if larger than bufsize
byte *COM_LoadStackFile (char *path, void *buffer, int bufsize)
{
	byte    *buf;
	
	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = COM_LoadFile (path, 4, NULL);
	
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
pack_t *COM_LoadPackFile (char *packfile)
{
	dpackheader_t   header;
	int                             i;
	packfile_t              *newfiles;
	int                             numpackfiles;
	pack_t                  *pack;
	int                             packhandle;
	dpackfile_t             info[MAX_FILES_IN_PACK];
	unsigned short          crc;

	if (Sys_FileOpenRead (packfile, &packhandle) == -1)
	{
//              Con_Printf ("Couldn't open %s\n", packfile);
		return NULL;
	}
	Sys_FileRead (packhandle, (void *)&header, sizeof(header));
	if (header.id[0] != 'P' || header.id[1] != 'A'
	|| header.id[2] != 'C' || header.id[3] != 'K')
		Sys_Error ("%s is not a packfile", packfile);
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error ("%s has %i files", packfile, numpackfiles);

	if (numpackfiles != PAK0_COUNT && numpackfiles != PAK0DEM_COUNT && numpackfiles != PAK2_COUNT)
		com_modified = true;    // not the original file

	Sys_FileSeek (packhandle, header.dirofs);
	Sys_FileRead (packhandle, (void *)info, header.dirlen);

// crc the directory to check for modifications
	CRC_Init (&crc);
	for (i=0 ; i<header.dirlen ; i++)
		CRC_ProcessByte (&crc, ((byte *)info)[i]);
	if (crc != PAK0_CRC && crc != PAK2_CRC && crc != PAK0DEM_CRC)
		com_modified = true;

	newfiles = Hunk_AllocName (numpackfiles * sizeof(packfile_t), "packfile");

// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Hunk_Alloc (sizeof (pack_t));
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
void COM_AddGameDirectory (char *dir, qboolean adduser)
{
	int				i;
	searchpath_t    *search;
	pack_t			*pak;
	char			pakfile[MAX_OSPATH];

	strcpy (com_gamedir, dir);

//
// add any pak files in the format pak0.pak pak1.pak, ...
//
	for (i=0 ; i < 10; i++)
	{
		sprintf (pakfile, "%s/pak%i.pak", dir, i);
		pak = COM_LoadPackFile (pakfile);
		if (!pak)
			continue;
		if (i == 2)
			Cvar_Set ("oem", "1");
		search = Hunk_Alloc (sizeof(searchpath_t));
		search->pack = pak;
		search->next = com_searchpaths;
		com_searchpaths = search;               
	}

//
// add the directory to the search path
//
	search = Hunk_Alloc (sizeof(searchpath_t));
	strcpy (search->filename, dir);
	search->next = com_searchpaths;
	com_searchpaths = search;

//
// add user's directory to the search path
// we don't need to set it on win32 platforms since it's exactly com_gamedir
//
#ifdef PLATFORM_UNIX
if (adduser) {
	search = Hunk_Alloc (sizeof(searchpath_t));
	strcpy (search->filename, com_userdir);
	search->next = com_searchpaths;
	com_searchpaths = search;
}
#endif
//
// add the contents of the parms.txt file to the end of the command line
//

}

/*
================
COM_InitFilesystem
================
*/
void COM_InitFilesystem (void)
{
	int             i, j;
	char    basedir[MAX_OSPATH];
	searchpath_t    *search;

//
// -basedir <path>
// Overrides the system supplied base directory (under GAMENAME)
//
	i = COM_CheckParm ("-basedir");
	if (i && i < com_argc-1)
		strcpy (basedir, com_argv[i+1]);
	else
		strcpy (basedir, host_parms.basedir);

	strcpy (com_userdir, host_parms.userdir);

	j = strlen (basedir);

	if (j > 0)
	{
#ifdef PLATFORM_UNIX
		if ((basedir[j-1] == '/'))
#else
		if ((basedir[j-1] == '\\') || (basedir[j-1] == '/'))
#endif
			basedir[j-1] = 0;
	}

//
// -cachedir <path>
// Overrides the system supplied cache directory (NULL or /qcache)
// -cachedir - will disable caching.
//
	i = COM_CheckParm ("-cachedir");
	if (i && i < com_argc-1)
	{
		if (com_argv[i+1][0] == '-')
			com_cachedir[0] = 0;
		else
			strcpy (com_cachedir, com_argv[i+1]);
	}
	else if (host_parms.cachedir)
		strcpy (com_cachedir, host_parms.cachedir);
	else
		com_cachedir[0] = 0;

//
// start up with GAMENAME by default (data1)
//
#ifndef H2MP
	COM_AddGameDirectory (va("%s/"GAMENAME, basedir), true);
#else
	COM_AddGameDirectory (va("%s/"GAMENAME, basedir), false);
	sprintf (com_userdir, "%s/portals", host_parms.userdir);
	Sys_mkdir (com_userdir);
	COM_AddGameDirectory (va("%s/portals", basedir), true);
#endif

// -game <gamedir>
// Adds basedir/gamedir as an override game
//
	i = COM_CheckParm ("-game");
	if (i && i < com_argc-1)
	{
		com_modified = true;
		sprintf (com_userdir, "%s/%s", host_parms.userdir, com_argv[i+1]);
		Sys_mkdir (com_userdir);
		COM_AddGameDirectory (va("%s/%s", basedir, com_argv[i+1]), true);
	}

// -path <dir or packfile> [<dir or packfile>] ...
// Fully specifies the exact serach path, overriding the generated one
//
	i = COM_CheckParm ("-path");
	if (i)
	{
		com_modified = true;
		com_searchpaths = NULL;
		while (++i < com_argc)
		{
			if (!com_argv[i] || com_argv[i][0] == '+' || com_argv[i][0] == '-')
				break;
			
			search = Hunk_Alloc (sizeof(searchpath_t));
			if ( !strcmp(COM_FileExtension(com_argv[i]), "pak") )
			{
				search->pack = COM_LoadPackFile (com_argv[i]);
				if (!search->pack)
					Sys_Error ("Couldn't load packfile: %s", com_argv[i]);
			}
			else
				strcpy (search->filename, com_argv[i]);
			search->next = com_searchpaths;
			com_searchpaths = search;
		}
	}

	if (COM_CheckParm ("-proghack"))
		proghack = true;

	i = COM_CheckParm ("-savedir");
	if (i)
	{
		sprintf(com_savedir,"%s/"GAMENAME, basedir);
	}
	else
	{
		strcpy(com_savedir,com_userdir);
	}

//Mimic qw style cvar to help gamespy, do we really need a way to change it?
//	Cvar_Set ("*gamedir", com_gamedir);	//removed to prevent full path
}


/*
 * $Log: not supported by cvs2svn $
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
