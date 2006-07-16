
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif


//=============================================================================

//Defines
#define VER_HWMASTER	"1.2.3"		/* our version string		*/

#define HWM_USERDIR	".hwmaster"	/* user directory for unix	*/

#undef true
#undef false
typedef enum {false, true}	qboolean;

#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define	MAX_ARGS	80
#define MAX_NUM_ARGVS	50

#define	MAX_MASTERS	8		// max recipients for heartbeat packets

#define	PORT_ANY	-1

#define	PORT_MASTER	26900
#define	PORT_SERVER	26950

//=========================================

// out of band message id bytes

// M = master, S = server, C = client, A = any
// the second character will always be \n if the message isn't a single
// byte long (?? not true anymore?)

#define	S2C_CHALLENGE		'c'
#define	S2C_CONNECTION		'j'
#define	A2A_PING		'k'	// respond with an A2A_ACK
#define	A2A_ACK			'l'	// general acknowledgement without info
#define	A2A_NACK		'm'	// [+ comment] general failure
#define A2A_ECHO		'e'	// for echoing
#define	A2C_PRINT		'n'	// print a message on client

#define	S2M_HEARTBEAT		'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN		'C'
#define	M2C_MASTER_REPLY	'd'	// + \n + hw server port list
//KS:
#define A2M_LIST		'o'
#define M2A_SENDLIST		'p'

#define	MAX_MSGLEN		1450	// max length of a reliable message
#define	MAX_DATAGRAM		1450	// max length of unreliable message
#define	MAX_UDP_PACKET	(MAX_MSGLEN*2)	// one more than msg + header


//=============================================================================
//Typedefs

typedef unsigned char byte;

typedef struct
{
	byte	ip[4];
	unsigned short	port;
	unsigned short	pad;
} netadr_t;

typedef struct server_s
{
	netadr_t	ip;
	struct	server_s *next;
	struct	server_s *previous;
	double	timeout;
} server_t;

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;	// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

typedef void (*xcommand_t) (void);

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char		*name;
	xcommand_t	function;
} cmd_function_t;


//=============================================================================
//Function prototypes

//sys_main.c
int COM_CheckParm (char *parm);
char *COM_Parse (char *data);
void Sys_Error (char *error, ...);
void Sys_Quit (void);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);

void SV_Shutdown();
double Sys_DoubleTime (void);

//cmds.c
int Cmd_Argc (void);
char *Cmd_Argv (int arg);
void Cmd_TokenizeString (char *text);
void Cmd_AddCommand (char *cmd_name, xcommand_t function);
void Cmd_Init();
void Cmd_ExecuteString (char *text);
void Cbuf_Init (void);
void Cbuf_AddText (char *text);
void Cbuf_InsertText (char *text);
void Cbuf_Execute (void);
qboolean Cmd_Exists (char *cmd_name);

//net.c
void NET_Shutdown (void);
char *NET_AdrToString (netadr_t a);
void SV_InitNet (void);
void SV_ReadPackets (void);
void SVL_Remove(server_t *sv);


//=============================================================================
//Globals

extern char		com_token[1024];
extern int		com_argc;
extern char	**com_argv;

extern int		net_socket;

extern server_t	*sv_list;

extern netadr_t		master_adr[MAX_MASTERS];	// address of group servers
extern int		num_masters;


//=============================================================================
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
#else
#if defined(SUNOS)
// these bits from darkplaces project
#if defined(__i386) || defined(__amd64)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#else
#warning "Unable to determine CPU endianess. Defaulting to little endian"
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

// end of endiannes stuff

