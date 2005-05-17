// comndef.h  -- general definitions

typedef unsigned char 		byte;
#define _DEF_BYTE_

#undef true
#undef false

typedef enum {false, true}	qboolean;

//============================================================================

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte		*data;
	int			maxsize;
	int			cursize;
	int			effectsize;
	qboolean	effectdata;
} sizebuf_t;

void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
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

// endianness stuff
#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN

#define BigShort(s) (s)
#define LittleShort(s) (ShortSwap(s))
#define BigLong(l) (l)
#define LittleLong(l) (LongSwap(l))
#define BigFloat(f) (f)
#define LittleFloat(f) (FloatSwap(f))

#elif __BYTE_ORDER == __LITTLE_ENDIAN

#define BigShort(s) (ShortSwap(s))
#define LittleShort(s) (s)
#define BigLong(l) (LongSwap(l))
#define LittleLong(l) (l)
#define BigFloat(f) (FloatSwap(f))
#define LittleFloat(f) (f)

#else
#error __BYTE_ORDER unset. I expect __LITTLE_ENDIAN or __BIG_ENDIAN
#endif

short	ShortSwap (short);
int	LongSwap (int);
float	FloatSwap (float);

//============================================================================

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);

extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading (void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);

//============================================================================

#ifndef PLATFORM_UNIX
#define Q_strncasecmp(s1,s2,n) strnicmp((s1),(s2),(n))
#define Q_strcasecmp(s1,s2) stricmp((s1),(s2))
#else
// assuming __GNUC__
#define Q_strncasecmp(s1,s2,n) strncasecmp((s1),(s2),(n))
#define Q_strcasecmp(s1,s2) strcasecmp((s1),(s2))
#endif

//============================================================================

extern	char		com_token[1024];
extern	qboolean	com_eof;

char *COM_Parse (char *data);


extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (char *parm);
void COM_Init (char *path);
void COM_InitArgv (int argc, char **argv);

char *COM_SkipPath (char *pathname);
void COM_StripExtension (char *in, char *out);
void COM_FileBase (char *in, char *out);
void COM_DefaultExtension (char *path, char *extension);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer


//============================================================================

extern int com_filesize;
struct cache_user_s;

extern	char	com_gamedir[MAX_OSPATH];
extern	char	com_savedir[MAX_OSPATH];
extern	char	com_userdir[MAX_OSPATH];

void COM_WriteFile (char *filename, void *data, int len);
int COM_OpenFile (char *filename, int *hndl);
int COM_FOpenFile (char *filename, FILE **file, qboolean override_pack);
void COM_CloseFile (int h);
void COM_CopyFile (char *netpath, char *cachepath);

byte *COM_LoadStackFile (char *path, void *buffer, int bufsize);
byte *COM_LoadTempFile (char *path);
byte *COM_LoadHunkFile (char *path);
void COM_LoadCacheFile (char *path, struct cache_user_s *cu);


extern	struct cvar_s	registered;
extern	struct cvar_s	oem;

