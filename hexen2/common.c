/*
	common.c
	misc functions used in client and server

	$Id: common.c,v 1.15 2005-07-09 09:07:55 sezero Exp $
*/

#include "quakedef.h"
#ifndef PLATFORM_UNIX
#include <windows.h>
#endif

#define NUM_SAFE_ARGVS	7

static char	*largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static char	*argvdummy = " ";

static char	*safeargvs[NUM_SAFE_ARGVS] =
	{"-nomidi", "-nolan", "-nosound", "-nocdaudio", "-nojoy", "-nomouse", "-dibonly"};

cvar_t	registered = {"registered","0"};
#ifdef H2MP
cvar_t	sv_gamedir = {"*gamedir","portals",false,true};
#else
cvar_t	sv_gamedir = {"*gamedir","data1",false,true};
#endif
cvar_t	oem = {"oem","0"};
cvar_t	cmdline = {"cmdline","0", false, true};

qboolean	com_modified;	// set true if using non-id files

int		static_registered = 1;	// only for startup check, then set

qboolean		msg_suppress_1 = 0;

void COM_InitFilesystem (void);
void COM_Path_f (void);

// if a packfile directory differs from this, it is assumed to be hacked
#define MAX_PAKDATA	6
const int pakdata[MAX_PAKDATA][2] = {
	{ 696,	34289 },
	{ 523,	2995  },
	{ 183,	4807  },
	{ 245,	1478  },
	{ 102,	41062 },
	{ 797,	22780 }
};

const char *dirdata[MAX_PAKDATA] = {
	"data1",
	"data1",
	"data1",
	"portals",
	"hw",
	"data1"
};

char	gamedirfile[MAX_OSPATH];

#define CMDLINE_LENGTH	256
char	com_cmdline[CMDLINE_LENGTH];

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
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, (int)(f*256/360) & 255);
}

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
	void	*data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("SZ_GetSpace: overflow without allowoverflow set");
		
		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %i is > full buffer size", length);
			
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
	int		len;

	len = strlen(data)+1;

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
	char *s, *s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
		s--;

	/* Pa3PyX: no range checking -- used to trash the stack and crash the
	   game randomly upon loading progs, for instance (or in any other
	   instance where one would supply a filename witout a path */
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
Sets the "registered" cvar.
Immediately exits out if an alternate game was attempted to be started without
being registered.
================
*/
void COM_CheckRegistered (void)
{
	int		h;
	unsigned short	check[128];
	int			i;

	COM_OpenFile("gfx/pop.lmp", &h);
	static_registered = 0;

//-basedir c:\h3\test -game data1 +exec rick.cfg
	if (h == -1)
	{
		Con_Printf ("Playing demo version.\n");

		if (com_modified)
			Sys_Error ("You must have the full version to use modified games");
		return;
	}

	Sys_FileRead (h, check, sizeof(check));
	COM_CloseFile (h);

	for (i=0 ; i<128 ; i++)
		if (pop[i] != (unsigned short)BigShort (check[i]))
			Sys_Error ("Corrupted data file.");

	Cvar_Set ("cmdline", com_cmdline);

	Cvar_Set ("registered", "1");
	static_registered = 1;
	Con_Printf ("Playing registered version.\n");
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

/*
================
COM_Init
================
*/
void COM_Init (char *basedir)
{
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
char	*va(char *format, ...)
{
	va_list		argptr;
	static char		string[1024];

	va_start (argptr, format);
	vsprintf (string, format,argptr);
	va_end (argptr);

	return string;
}


/// just for debugging
int	memsearch (byte *start, int count, int search)
{
	int		i;

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
	int		handle;
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
char	com_userdir[MAX_OSPATH];
char	com_savedir[MAX_OSPATH];

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct	searchpath_s *next;
} searchpath_t;

searchpath_t	*com_searchpaths;

/*
============
COM_Path_f

============
*/
void COM_Path_f (void)
{
	searchpath_t	*s;

	Con_Printf ("Current search path:\n");
	for (s=com_searchpaths ; s ; s=s->next)
	{
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
	int		handle;
	char	name[MAX_OSPATH];

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
void	COM_CreatePath (char *path)
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
needed.  This is for the convenience of developers using ISDN from home.
===========
*/
void COM_CopyFile (char *netpath, char *cachepath)
{
	int		in, out;
	int		remaining, count;
	char	buf[4096];

	remaining = Sys_FileOpenRead (netpath, &in);            
	COM_CreatePath (cachepath);	// create directories up to the cache file
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
	searchpath_t	*search;
	char		netpath[MAX_OSPATH];
	pack_t		*pak;
	int			i;
	int			findtime;

	if (file && handle)
		Sys_Error ("COM_FindFile: both handle and file set");
	if (!file && !handle)
		Sys_Error ("COM_FindFile: neither handle or file set");

//
// search through the path, one element at a time
//
	search = com_searchpaths;
	for ( ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
				if (!strcmp (pak->files[i].name, filename))
				{	// found it!
					if (handle)
					{
						*handle = pak->handle;
						Sys_FileSeek (pak->handle, pak->files[i].filepos);
					}
					else
					{
					// open a new file on the pakfile
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

			/* Sys_Printf ("FindFile: %s\n",netpath); */
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
	searchpath_t	*s;

	for (s = com_searchpaths ; s ; s=s->next)
		if (s->pack && s->pack->handle == h)
			return;

	Sys_FileClose (h);
}

/*
============
COM_LoadFile

Filename are reletive to the quake directory.
Allways appends a 0 byte.
============
*/
cache_user_t *loadcache;
byte	*loadbuf;
int		loadsize;
byte *COM_LoadFile (char *path, int usehunk, int *size)
{
	int		h;
	byte	*buf;
	char	base[32];
	int		len;

	buf = NULL;	// quiet compiler warning

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

#ifndef GLQUAKE
	Draw_BeginDisc ();
#endif
	Sys_FileRead (h, buf, len);                     
	COM_CloseFile (h);
#ifndef GLQUAKE
	Draw_EndDisc ();
#endif
	return buf;
}

byte *COM_LoadHunkFile (char *path)
{
	return COM_LoadFile (path, 1, NULL);
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
	byte	*buf;

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
pack_t *COM_LoadPackFile (char *packfile, int paknum)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	int				packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned short		crc;

	if (Sys_FileOpenRead (packfile, &packhandle) == -1)
	{
//		Con_Printf ("Couldn't open %s\n", packfile);
		return NULL;
	}

	Sys_FileRead (packhandle, (void *)&header, sizeof(header));

	if (header.id[0] != 'P' || header.id[1] != 'A' ||
	    header.id[2] != 'C' || header.id[3] != 'K')
		Sys_Error ("%s is not a packfile", packfile);

	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error ("%s has %i files", packfile, numpackfiles);

	Sys_FileSeek (packhandle, header.dirofs);
	Sys_FileRead (packhandle, (void *)info, header.dirlen);

// crc the directory
	CRC_Init (&crc);
	for (i=0 ; i<header.dirlen ; i++)
		CRC_ProcessByte (&crc, ((byte *)info)[i]);

// check for modifications
	if (paknum <= MAX_PAKDATA-2) {
		if (strcmp(gamedirfile, dirdata[paknum]) != 0) {
			com_modified = true;    // raven didnt ship like that
		} else if (numpackfiles != pakdata[paknum][0]) {
			if (paknum == 0) { // demo ??
				if (numpackfiles != pakdata[MAX_PAKDATA-1][0]) {
					com_modified = true;    // not original
				} else if (crc != pakdata[MAX_PAKDATA-1][1]) {
					com_modified = true;    // not original
				}
				// both crc and numfiles matched the demo
			} else {
				com_modified = true;    // not original
			}
		} else if (crc != pakdata[paknum][1]) {
			com_modified = true;    // not original
		}
		// both crc and numfiles are good, we are still original
	} else {
		com_modified = true;
	}

// parse the directory
	newfiles = Hunk_AllocName (numpackfiles * sizeof(packfile_t), "packfile");
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}

	pack = Hunk_AllocName (sizeof (pack_t), "pack");
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
void COM_AddGameDirectory (char *dir)
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
		if (i == 2)
			Cvar_Set ("oem", "1");
		search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
		search->pack = pak;
		search->next = com_searchpaths;
		com_searchpaths = search;
	}

//
// add the directory to the search path
//
	search = Hunk_AllocName (sizeof(searchpath_t), "searchpath");
	strcpy (search->filename, dir);
	search->next = com_searchpaths;
	com_searchpaths = search;

//
// add user's directory to the search path
// we don't need to set it on win32 platforms since it's exactly com_gamedir
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
COM_InitFilesystem
================
*/
void COM_InitFilesystem (void)
{
	int		i, j;
	char	basedir[MAX_OSPATH];

//
// -basedir <path>
// Overrides the system supplied base directory (under data1)
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
// start up with data1 by default
//
#ifdef _WIN32
	// Let's keep the game's old win32 behavior
	sprintf (com_userdir, "%s/data1", host_parms.userdir);
#endif
	COM_AddGameDirectory (va("%s/data1", basedir));
#ifdef H2MP
	sprintf (com_userdir, "%s/portals", host_parms.userdir);
	Sys_mkdir (com_userdir);
	COM_AddGameDirectory (va("%s/portals", basedir));
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
		COM_AddGameDirectory (va("%s/%s", basedir, com_argv[i+1]));
	}

	strcpy(com_savedir,com_userdir);

//Mimic qw style cvar to help gamespy, do we really need a way to change it?
//	Cvar_Set ("*gamedir", com_gamedir);	//removed to prevent full path
}


/*
 * $Log: not supported by cvs2svn $
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
