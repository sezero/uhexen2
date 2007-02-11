/*
	quakedef.h
	primary header for client

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/quakedef.h,v 1.105 2007-02-11 08:06:32 sezero Exp $
*/

#ifndef __QUAKEDEFS_H
#define __QUAKEDEFS_H

#define __STRINGIFY(x) #x
#define STRINGIFY(x) __STRINGIFY(x)

#include "compiler.h"
#include "arch_def.h"
#include "h2_opt.h"

#define	HOT_VERSION_MAJ		1
#define	HOT_VERSION_MID		4
#define	HOT_VERSION_MIN		1
#define	HOT_VERSION_REL_DATE	"2006-11-16"
#define	HOT_VERSION_BETA	0
#define	HOT_VERSION_BETA_STR	"Final"
#define	HOT_VERSION_STR		STRINGIFY(HOT_VERSION_MAJ) "." STRINGIFY(HOT_VERSION_MID) "." STRINGIFY(HOT_VERSION_MIN)
#define	GLQUAKE_VERSION		1.00
#define	ENGINE_VERSION		1.16
#define	ENGINE_NAME		"Hexen2"

#ifndef	DEMOBUILD
#ifdef __MACOSX__
#define	AOT_USERDIR		"Library/Application Support/Hexen2"
#else
#define	AOT_USERDIR		".hexen2"
#endif
#else
#ifdef __MACOSX__
#define	AOT_USERDIR		"Library/Application Support/Hexen2 Demo"
#else
#define	AOT_USERDIR		".hexen2demo"
#endif
#endif

#define	MAX_QPATH	64	// max length of a quake game pathname
#define	MAX_OSPATH	256	// max length of a filesystem pathname

#define	QUAKE_GAME		// as opposed to utilities

#include <sys/types.h>
#include <math.h>
#include <string.h>
#if !defined(_WIN32)
// for strcasecmp and strncasecmp
#include <strings.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define CACHE_SIZE	32	// used to align key data structures

#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings

#define	MINIMUM_MEMORY	0x550000
#define	MINIMUM_MEMORY_LEVELPAK	(MINIMUM_MEMORY + 0x100000)

#define MAX_NUM_ARGVS	50

// up / down
#define	PITCH		0

// left / right
#define	YAW		1

// fall over
#define	ROLL		2


//
// Timing macros
//
#define HX_FRAME_TIME	0.05
#define HX_FPS		20


//#ifdef DEMOBUILD
//#define	MAX_SCOREBOARD	8
//#endif
#define	MAX_SCOREBOARD	16
#define	MAX_SCOREBOARDNAME	32

#define	SOUND_CHANNELS	8


#define	ON_EPSILON	0.1		// point on plane side epsilon

//#define	MAX_MSGLEN	8000		// max length of a reliable message
//#define	MAX_MSGLEN	16000		// max length of a reliable message
#define	MAX_MSGLEN	20000		// for mission pack tibet2

//#define	MAX_DATAGRAM	2048		// max length of unreliable message  TEMP: This only for E3
#define	MAX_DATAGRAM	1024		// max length of unreliable message


//
// per-level limits
//
#define	MAX_EDICTS	600		// FIXME: ouch! ouch! ouch!
#define	MAX_LIGHTSTYLES	64
#define	MAX_MODELS	512		// Sent over the net as a word
#define	MAX_SOUNDS	512		// Sent over the net as a byte


#define	MAX_STYLESTRING		64


//
// stats are integers communicated to the client by the server
//
#define	MAX_CL_STATS		32
//#define	STAT_HEALTH		0
#define	STAT_FRAGS		1
#define	STAT_WEAPON		2
//#define	STAT_AMMO		3
#define	STAT_ARMOR		4
#define	STAT_WEAPONFRAME	5
//#define	STAT_SHELLS		6
//#define	STAT_NAILS		7
//#define	STAT_ROCKETS		8
//#define	STAT_CELLS		9
//#define	STAT_ACTIVEWEAPON	10
#define	STAT_TOTALSECRETS	11
#define	STAT_TOTALMONSTERS	12
#define	STAT_SECRETS		13	// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14	// bumped by svc_killedmonster
//#define	STAT_BLUEMANA		15
//#define	STAT_GREENMANA		16
//#define	STAT_EXPERIENCE		17


#define	MAX_INVENTORY		15	// Max inventory array size


#define	SAVEGAME_VERSION	5
#define	SAVEGAME_COMMENT_LENGTH	39	// 0-19: level name, 21-rest: save time
#define	MAX_SAVEGAMES		12	// Max number of savegames for the menu listing


//
// stock defines
//
#define	IT_SHOTGUN		1
#define	IT_SUPER_SHOTGUN	2
#define	IT_NAILGUN		4
#define	IT_SUPER_NAILGUN	8
#define	IT_GRENADE_LAUNCHER	16
#define	IT_ROCKET_LAUNCHER	32
#define	IT_LIGHTNING		64
#define	IT_SUPER_LIGHTNING	128
#define	IT_SHELLS		256
#define	IT_NAILS		512
#define	IT_ROCKETS		1024
#define	IT_CELLS		2048
#define	IT_AXE			4096
#define	IT_ARMOR1		8192
#define	IT_ARMOR2		16384
#define	IT_ARMOR3		32768
#define	IT_SUPERHEALTH		65536
#define	IT_KEY1			131072
#define	IT_KEY2			262144
#define	IT_INVISIBILITY		524288
#define	IT_INVULNERABILITY	1048576
#define	IT_SUIT			2097152
#define	IT_QUAD			4194304
#define	IT_SIGIL1		(1 << 28)
#define	IT_SIGIL2		(1 << 29)
#define	IT_SIGIL3		(1 << 30)
#define	IT_SIGIL4		(1 << 31)

#define	ART_HASTE			1
#define	ART_INVINCIBILITY		2
#define	ART_TOMEOFPOWER			4
#define	ART_INVISIBILITY		8
#define	ARTFLAG_FROZEN			128
#define	ARTFLAG_STONED			256
#define	ARTFLAG_DIVINE_INTERVENTION	512

//
// edict->drawflags
//
#define MLS_MASKIN			7	// MLS: Model Light Style
#define MLS_MASKOUT			248
#define MLS_NONE			0
#define MLS_FULLBRIGHT			1
#define MLS_POWERMODE			2
#define MLS_TORCH			3
#define MLS_TOTALDARK			4
#define MLS_ABSLIGHT			7
#define SCALE_TYPE_MASKIN		24
#define SCALE_TYPE_MASKOUT		231
#define SCALE_TYPE_UNIFORM		0	// Scale X, Y, and Z
#define SCALE_TYPE_XYONLY		8	// Scale X and Y
#define SCALE_TYPE_ZONLY		16	// Scale Z
#define SCALE_ORIGIN_MASKIN		96
#define SCALE_ORIGIN_MASKOUT		159
#define SCALE_ORIGIN_CENTER		0	// Scaling origin at object center
#define SCALE_ORIGIN_BOTTOM		32	// Scaling origin at object bottom
#define SCALE_ORIGIN_TOP		64	// Scaling origin at object top
#define DRF_TRANSLUCENT			128
#define DRF_ANIMATEONCE			256


//===========================================

//
// game data flags
//
#define	GAME_DEMO		1
#define	GAME_OEM		2
#define	GAME_MODIFIED		4
#define	GAME_REGISTERED		8
#define	GAME_REGISTERED0	16
#define	GAME_REGISTERED1	32
#define	GAME_PORTALS		64
#define	GAME_HEXENWORLD		128

//
// Player Classes
//
#define MAX_PLAYER_CLASS	5
#define	PORTALS_EXTRA_CLASSES	1
#define ABILITIES_STR_INDEX	400

#define CLASS_PALADIN		1
#define CLASS_CLERIC 		2
#define CLASS_NECROMANCER	3
#define CLASS_THEIF   		4
#define CLASS_DEMON		5

//#define BASE_ENT_ON		1
//#define BASE_ENT_SENT	2

//=============================================================================

#include "common.h"
#include "bspfile.h"
#include "sys.h"
#include "zone.h"
#include "mathlib.h"
#include "cvar.h"

#include "protocol.h"
#include "net.h"

#include "cmd.h"
#include "crc.h"

//=============================================================================

typedef struct
{
	vec3_t	origin;
	vec3_t	angles;
	short	modelindex;
	byte	frame;
	byte	colormap;
	byte	skin;
	byte	effects;
	byte	scale;
	byte	drawflags;
	byte	abslight;
	byte	ClearCount[32];
} entity_state_t;

typedef struct
{
	byte	flags;
	short	index;

	vec3_t	origin;
	vec3_t	angles;
	short	modelindex;
	byte	frame;
	byte	colormap;
	byte	skin;
	byte	effects;
	byte	scale;
	byte	drawflags;
	byte	abslight;
} entity_state2_t;

typedef struct
{
	byte	flags;

	vec3_t	origin;
	vec3_t	angles;
	short	modelindex;
	byte	frame;
	byte	colormap;
	byte	skin;
	byte	effects;
	byte	scale;
	byte	drawflags;
	byte	abslight;
} entity_state3_t;

#define MAX_CLIENT_STATES	150
#define MAX_FRAMES		5
#define MAX_CLIENTS		8
#define CLEAR_LIMIT		2

#define ENT_STATE_ON		1
#define ENT_CLEARED		2

typedef struct
{
	entity_state2_t	states[MAX_CLIENT_STATES];
//	unsigned long	frame;
//	unsigned long	flags;
	int		count;
} client_frames_t;

typedef struct
{
	entity_state2_t	states[MAX_CLIENT_STATES*2];
	int		count;
} client_frames2_t;

typedef struct
{
	client_frames_t	frames[MAX_FRAMES+2]; // 0 = base, 1-max = proposed, max+1 = too late
} client_state2_t;

//=============================================================================

#if !defined(SERVERONLY)
#include "console.h"
#include "vid.h"
#include "wad.h"
#include "draw.h"
#include "render.h"
#include "view.h"
#include "screen.h"
#include "sbar.h"
#include "sound.h"
#include "cdaudio.h"
#include "mididef.h"
#endif

#include "progs.h"
#include "pr_strng.h"
#include "cl_effect.h"
#if defined(SERVERONLY)
#include "h2ded.h"
#else
#include "client.h"
#endif
#include "server.h"

#if defined(SERVERONLY)
#include "server/model.h"
#elif defined(GLQUAKE)
// gl_model.h is included from within glquake.h
#include "glquake.h"
#else
#include "model.h"
#include "d_iface.h"
#endif
#include "world.h"

#if !defined(SERVERONLY)
#include "input.h"
#include "keys.h"
#include "menu.h"
#endif

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	char	*basedir;
	char	*userdir;		// userspace directory on UNIX platforms
	int	argc;
	char	**argv;
	void	*membase;
	int	memsize;
} quakeparms_t;


//=============================================================================

//
// host
//
extern	quakeparms_t	host_parms;

extern	cvar_t		sys_ticrate;
extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;

extern	qboolean	host_initialized;	// true if into command execution
extern	double		host_frametime;
extern	byte		*host_basepal;
extern	byte		*host_colormap;
extern	int		host_framecount;	// incremented every frame, never reset
extern	double		realtime;		// not bounded in any way, changed at
						// start of every frame, never reset

void Host_ClearMemory (void);
void Host_InitCommands (void);
void Host_Init (quakeparms_t *parms);
void Host_Shutdown(void);
void Host_Error (const char *error, ...) _FUNC_PRINTF(1);
void Host_EndGame (const char *message, ...) _FUNC_PRINTF(1);
void Host_Frame (float time);
void Host_Quit_f (void);
void Host_ClientCommands (const char *fmt, ...) _FUNC_PRINTF(1);
void Host_ShutdownServer (qboolean crash);
void Host_RemoveGIPFiles (const char *path);
qboolean Host_CopyFiles(const char *source, const char *pat, const char *dest);
qboolean SaveGamestate (qboolean ClientsOnly);


extern	qboolean	isDedicated;

extern	qboolean	msg_suppress_1;	// suppresses resolution and cache size console output
					//  a fullscreen DIB focus gain/loss
extern	int		current_skill;	// skill level for currently loaded level (in case
					//  the user changes the cvar while the level is
					//  running, this reflects the level actually in use)

extern	qboolean	noclip_anglehack;
extern	int		sv_kingofhill;
extern	qboolean	intro_playing;	// whether the mission pack intro is playing
extern	qboolean	skip_start;	// for the mission pack intro
extern	int		num_intro_msg;	// for the mission pack intro

//
// chase
//
extern	cvar_t		chase_active;

void Chase_Init (void);
void Chase_Reset (void);
void Chase_Update (void);

#endif	/* __QUAKEDEFS_H */

