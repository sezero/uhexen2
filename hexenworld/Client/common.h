// comndef.h  -- general definitions

#ifndef __HX2_COMMON_H
#define __HX2_COMMON_H

#ifdef PLATFORM_UNIX
#include "linux_inc.h"
#endif

typedef unsigned char 		byte;
#define _DEF_BYTE_

// KJB Undefined true and false defined in SciTech's DEBUG.H header
#undef true
#undef false

typedef enum {false, true}	qboolean;

#define	MAX_INFO_STRING	196
#define	MAX_SERVERINFO_STRING	512
#define	MAX_LOCALINFO_STRING	32768

#ifdef SUNOS
#define model_t hx2_model_t // Workaround conflict with /usr/include/sys/model.h
#endif

//============================================================================

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte		*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

void SZ_Init (sizebuf_t *buf, byte *data, int length);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf

//============================================================================

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (int)&(((t *)0)->m)))

//============================================================================

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)

#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)

//============================================================================

extern char *Q_strlwr (char *str);
extern char *Q_strupr (char *str);

extern void *Q_malloc(unsigned int size);
extern void Q_free(void *mem);


//============================================================================

// endianness stuff: <sys/types.h> is supposed
// to succeed in locating the correct endian.h
// this BSD style may not work everywhere, eg. on WIN32
#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)
#undef BYTE_ORDER
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#endif
// assumptions in case we don't have endianness info
#ifndef BYTE_ORDER
#if defined(_WIN32)
#define BYTE_ORDER LITTLE_ENDIAN
#define GUESSED_WIN32_ENDIANNESS
#else
#if defined(SUNOS)
// these bits from darkplaces project
#define GUESSED_SUNOS_ENDIANNESS
#if defined(__i386) || defined(__amd64)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#else
#define ASSUMED_LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif
#endif

short	ShortSwap (short);
int	LongSwap (int);
float	FloatSwap (float);

#if BYTE_ORDER == BIG_ENDIAN
#define BigShort(s) (s)
#define LittleShort(s) ShortSwap((s))
#define BigLong(l) (l)
#define LittleLong(l) LongSwap((l))
#define BigFloat(f) (f)
#define LittleFloat(f) FloatSwap((f))
#else
// BYTE_ORDER == LITTLE_ENDIAN
#define BigShort(s) ShortSwap((s))
#define LittleShort(s) (s)
#define BigLong(l) LongSwap((l))
#define LittleLong(l) (l)
#define BigFloat(f) FloatSwap((f))
#define LittleFloat(f) (f)
#endif

// end of endianness stuff


//============================================================================
#ifdef H2W
struct usercmd_s;
#endif
void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);
#ifdef H2W
void MSG_WriteAngle16 (sizebuf_t *sb, float f);
void MSG_WriteUsercmd (sizebuf_t *sb, struct usercmd_s *cmd, qboolean long_msg);
#endif

extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading (void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);
#ifdef H2W
char *MSG_ReadStringLine (void);
#endif

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);
#ifdef H2W
float MSG_ReadAngle16 (void);
void MSG_ReadUsercmd (struct usercmd_s *cmd, qboolean long_msg);
#endif

//============================================================================

#ifdef _WIN32
#define Q_strncasecmp	strnicmp
#define Q_strcasecmp	stricmp
#else
#define Q_strncasecmp	strncasecmp
#define Q_strcasecmp	strcasecmp
#endif

// strlcpy and strlcat :
#include "strl_fn.h"

#if HAVE_STRLCAT && HAVE_STRLCPY

// use native library functions
#define Q_strlcpy strlcpy
#define Q_strlcat strlcat

#else

// use our own copies of strlcpy and strlcat taken from OpenBSD :
extern size_t Q_strlcpy (char *dst, const char *src, size_t size);
extern size_t Q_strlcat (char *dst, const char *src, size_t size);

#endif

#define Q_strlcat_err(DST,SRC,SIZE) {							\
	if (Q_strlcat((DST),(SRC),(SIZE)) >= (SIZE))					\
		Sys_Error("%s: %d: string buffer overflow!",__FUNCTION__,__LINE__);	\
}
#define Q_strlcpy_err(DST,SRC,SIZE) {							\
	if (Q_strlcpy((DST),(SRC),(SIZE)) >= (SIZE))					\
		Sys_Error("%s: %d: string buffer overflow!",__FUNCTION__,__LINE__);	\
}
#define Q_snprintf_err(DST,SIZE,fmt,args...) {						\
	if (snprintf((DST),(SIZE),fmt,##args) >= (SIZE))				\
		Sys_Error("%s: %d: string buffer overflow!",__FUNCTION__,__LINE__);	\
}


//============================================================================

extern	char		com_token[1024];
extern	qboolean	com_eof;

char *COM_Parse (char *data);


extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (char *parm);
#if 0
void COM_AddParm (char *parm);
#endif
void COM_Init (void);
void COM_InitArgv (int argc, char **argv);

char *COM_SkipPath (char *pathname);
void COM_StripExtension (char *in, char *out);
void COM_FileBase (char *in, char *out);
void COM_DefaultExtension (char *path, char *extension, size_t len);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer

int COM_StrCompare (const void *arg1, const void *arg2);
// quick'n'dirty string comparison function for use with qsort


//============================================================================

extern	size_t	com_filesize;
struct cache_user_s;

extern	char	com_basedir[MAX_OSPATH];
extern	char	com_gamedir[MAX_OSPATH];
extern	char	com_savedir[MAX_OSPATH];	// temporary path for saving gip files
extern	char	com_userdir[MAX_OSPATH];

int COM_WriteFile (char *filename, void *data, size_t len);
size_t COM_FOpenFile (char *filename, FILE **file, qboolean override_pack);
int COM_CopyFile (char *netpath, char *cachepath);
void COM_CloseFile (FILE *h);

byte *COM_LoadStackFile (char *path, void *buffer, size_t bufsize);
byte *COM_LoadBufFile (char *path, void *buffer, size_t *bufsize);
byte *COM_LoadTempFile (char *path);
byte *COM_LoadHunkFile (char *path);
byte *COM_LoadZoneFile (char *path);
byte *COM_LoadMallocFile (char *path);
void COM_LoadCacheFile (char *path, struct cache_user_s *cu);
int COM_CreatePath (char *path);
void COM_Gamedir (char *dir);
int COM_FileInGamedir (char *fname);

extern	struct cvar_s	registered;
extern	struct cvar_s	oem;
extern unsigned int	gameflags;

#ifdef H2W
char *Info_ValueForKey (char *s, char *key);
void Info_RemoveKey (char *s, char *key);
void Info_RemovePrefixedKeys (char *start, char prefix);
void Info_SetValueForKey (char *s, char *key, char *value, int maxsize);
void Info_SetValueForStarKey (char *s, char *key, char *value, int maxsize);
void Info_Print (char *s);

// global vars for siege.
extern qboolean cl_siege;
extern byte cl_fraglimit;
extern float cl_timelimit;
extern float cl_server_time_offset;
#endif

#endif	/* __HX2_COMMON_H */

