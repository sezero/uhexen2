/*
	quakedef.h
	primary header for client

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/quakedef.h,v 1.17 2005-01-13 10:47:01 sezero Exp $
*/

//#define	GLTEST			// experimental stuff

#if defined _WIN32
// This is ignored on GNU
#pragma warning(disable:4244)
#endif

#ifdef PLATFORM_UNIX
#include "linux_inc.h"
#include <ctype.h>
#define WITH_SDL	/* for the mouse2/3 hack in keys.c */
#endif

#define HOT_VERSION_MAJ 1
#define HOT_VERSION_MID 2
#define HOT_VERSION_MIN 3
#ifdef PLATFORM_UNIX
#define VERSION_PLATFORM "Linux"
#else
#define VERSION_PLATFORM "Windows"
#endif

#define	QUAKE_GAME		// as opposed to utilities

#define HEXEN2_VERSION		1.12

//define	PARANOID	// speed sapping error checking

#ifdef QUAKE2
#define	GAMENAME	"data1"	// directory to look in by default
#else
#define	GAMENAME	"data1"
#endif

#ifndef DEMOBUILD
#define AOT_USERDIR ".hexen2"
#else
#define AOT_USERDIR ".hexen2demo"
#endif

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#if defined(_WIN32) && !defined(WINDED) || defined(PLATFORM_UNIX)

#if defined(_M_IX86)
#define __i386__	1
#endif

void	VID_LockBuffer (void);
void	VID_UnlockBuffer (void);

#else

#define	VID_LockBuffer()
#define	VID_UnlockBuffer()

#endif

/* From Dan Olson:
   The code isn't compilable on non-intel until all of the asm is
   taken out.  Don't worry about the id386 define *yet*, and even
   after all of the assembly is replaced  you may still need it
   defined for non-x86 compiles. The eventual goal should probably
   be to get rid of all x86 specific stuff. */
#ifdef __i386__
#define id386	1
#else
#define id386	0
#endif

#if id386
#define UNALIGNED_OK	1	// set to 0 if unaligned accesses are not supported
#else
#define UNALIGNED_OK	0
#endif

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define CACHE_SIZE	32	// used to align key data structures

#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings

#define	MINIMUM_MEMORY	0x550000
#define	MINIMUM_MEMORY_LEVELPAK	(MINIMUM_MEMORY + 0x100000)

#define MAX_NUM_ARGVS	50

// up / down
#define	PITCH	0

// left / right
#define	YAW	1

// fall over
#define	ROLL	2


// Timing macros
#define HX_FRAME_TIME		0.05
#define HX_FPS			20


#define	MAX_QPATH		64			// max length of a quake game pathname
#define	MAX_OSPATH		128			// max length of a filesystem pathname

#define	ON_EPSILON		0.1			// point on plane side epsilon

//#define	MAX_MSGLEN		8000		// max length of a reliable message
//#define	MAX_MSGLEN		16000		// max length of a reliable message
#define	MAX_MSGLEN		20000		// for mission pack tibet2

#define	MAX_DATAGRAM		1024		// max length of unreliable message
//#define	MAX_DATAGRAM	2048		// max length of unreliable message  TEMP: This only for E3

//
// per-level limits
//
#define	MAX_EDICTS		600		// FIXME: ouch! ouch! ouch!
#define	MAX_LIGHTSTYLES	64

#define	MAX_MODELS		512		// Sent over the net as a word

#define	MAX_SOUNDS		512		// Sent over the net as a byte


#define	SAVEGAME_COMMENT_LENGTH	39

#define	MAX_STYLESTRING	64

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
#define	STAT_SECRETS		13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14		// bumped by svc_killedmonster
//#define	STAT_BLUEMANA		15
//#define	STAT_GREENMANA		16
//#define	STAT_EXPERIENCE		17


#define	MAX_INVENTORY		15		// Max inventory array size

// stock defines

#define	IT_SHOTGUN		1
#define	IT_SUPER_SHOTGUN	2
#define	IT_NAILGUN		4
#define	IT_SUPER_NAILGUN	8
#define	IT_GRENADE_LAUNCHER	16
#define	IT_ROCKET_LAUNCHER	32
#define	IT_LIGHTNING		64
#define IT_SUPER_LIGHTNING      128
#define IT_SHELLS               256
#define IT_NAILS                512
#define IT_ROCKETS              1024
#define IT_CELLS                2048
#define IT_AXE                  4096
#define IT_ARMOR1               8192
#define IT_ARMOR2               16384
#define IT_ARMOR3               32768
#define IT_SUPERHEALTH          65536
#define IT_KEY1                 131072
#define IT_KEY2                 262144
#define	IT_INVISIBILITY		524288
#define	IT_INVULNERABILITY	1048576
#define	IT_SUIT			2097152
#define	IT_QUAD			4194304
#define IT_SIGIL1               (1<<28)
#define IT_SIGIL2               (1<<29)
#define IT_SIGIL3               (1<<30)
#define IT_SIGIL4               (1<<31)

#define ART_HASTE			1
#define ART_INVINCIBILITY		2
#define ART_TOMEOFPOWER			4
#define ART_INVISIBILITY		8
#define ARTFLAG_FROZEN			128
#define ARTFLAG_STONED			256
#define ARTFLAG_DIVINE_INTERVENTION	512

//===========================================

#ifdef H2MP
#define NUM_CLASSES		5
#else
#define	NUM_CLASSES		4
#endif
#define ABILITIES_STR_INDEX	400

#ifdef DEMOBUILD
	#define	MAX_SCOREBOARD	8
#else
	#define	MAX_SCOREBOARD	16
#endif

#define	MAX_SCOREBOARDNAME	32

#define	SOUND_CHANNELS		8

// This makes anyone on id's net privileged
// Use for multiplayer testing only - VERY dangerous!!!
// #define IDGODS

#include "common.h"
#include "bspfile.h"
#include "vid.h"
#include "sys.h"
#include "zone.h"
#include "mathlib.h"

//#define BASE_ENT_ON		1
//#define BASE_ENT_SENT	2

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

#if RJNET
	byte	ClearCount[32];
#endif
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

#define MAX_CLIENT_STATES 150
#define MAX_FRAMES 5
#define MAX_CLIENTS 8
#define CLEAR_LIMIT 2

#define ENT_STATE_ON		1
#define ENT_CLEARED			2

typedef struct 
{
	entity_state2_t states[MAX_CLIENT_STATES];
//	unsigned long frame;
//	unsigned long flags;
	int count;
} client_frames_t;

typedef struct 
{
	entity_state2_t states[MAX_CLIENT_STATES*2];
	int count;
} client_frames2_t;

typedef struct
{
	client_frames_t frames[MAX_FRAMES+2]; // 0 = base, 1-max = proposed, max+1 = too late
} client_state2_t;


#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "screen.h"
#include "net.h"
#include "protocol.h"
#include "cmd.h"
#include "sbar.h"
#include "sound.h"
#include "render.h"
#include "cl_effect.h"
#include "progs.h"
#include "client.h"
#include "server.h"

#ifdef GLQUAKE
#include "gl_model.h"
#else
#include "model.h"
#include "d_iface.h"
#endif

#include "input.h"
#include "world.h"
#include "keys.h"
#include "console.h"
#include "view.h"
#include "menu.h"
#include "crc.h"
#include "cdaudio.h"

#ifdef GLQUAKE
#include "glquake.h"
#endif

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	char	*basedir;
        char    *userdir;               // userspace directory on UNIX platforms
	char	*cachedir;		// for development over ISDN lines
	int	argc;
	char	**argv;
	void	*membase;
	int	memsize;
} quakeparms_t;


//=============================================================================



extern qboolean noclip_anglehack;


//
// host
//
extern	quakeparms_t host_parms;

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
void Host_ServerFrame (void);
void Host_InitCommands (void);
void Host_Init (quakeparms_t *parms);
void Host_Shutdown(void);
void Host_Error (char *error, ...);
void Host_EndGame (char *message, ...);
void Host_Frame (float time);
void Host_Quit_f (void);
void Host_ClientCommands (char *fmt, ...);
void Host_ShutdownServer (qboolean crash);
void SaveGamestate (qboolean ClientsOnly);

extern qboolean		msg_suppress_1;		// suppresses resolution and cache size console output
										//  an fullscreen DIB focus gain/loss
extern int		current_skill;		// skill level for currently loaded level (in case
										//  the user changes the cvar while the level is
										//  running, this reflects the level actually in use)

extern qboolean		isDedicated;

extern int		minimum_memory;

extern int		sv_kingofhill;
extern qboolean		intro_playing;
extern qboolean		skip_start;
extern int		num_intro_msg;
extern qboolean		check_bottom;
//
// chase
//
extern	cvar_t	chase_active;

void Chase_Init (void);
void Chase_Reset (void);
void Chase_Update (void);


// Midi Stuff
// Put it here cuz I didn't want to make all the files
// include the window components
qboolean MIDI_Init(void);
void ReInitMusic(void);
void MIDI_Cleanup(void);
void MIDI_Play(char *Name);
void MIDI_Stop(void);
void MIDI_Pause(void);
void MIDI_Loop(int NewValue);

//
// Steve's windowed mouse tinkerings
//
void IN_ActivateMouseSA (void);
void IN_DeactivateMouseSA (void);

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.16  2005/01/02 15:08:53  sezero
 * Retire AoT, aot and .aot: Change userdir (AOT_USERDIR) to .hexen2
 *
 * Revision 1.15  2004/12/30 14:27:15  sezero
 * userdir (AOT_USERDIR) is ~/.hexen2demo for make DEMOBUILD=yes [...]
 *
 * Revision 1.14  2004/12/26 10:05:57  sezero
 * prepare for 1.2.3
 *
 * Revision 1.12  2004/12/22 21:50:16  sezero
 * play with version and help display
 *
 * Revision 1.11  2004/12/19 10:35:30  sezero
 * - Add "implicitly declared" functions as externs: Software rendering files.
 * - Add comments on the asm/non-Intel status of the code
 *
 * Revision 1.10  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.9  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.8  2004/12/18 13:54:43  sezero
 * Clean-up and kill warnings 6:
 * Include proper stock headers to avoid some "implicity declared" warnings
 *
 * Revision 1.7  2004/12/18 13:44:12  sezero
 * Clean-up and kill warnings 1:
 * Kill two pragmas that are ignored anyway.
 *
 * Revision 1.6  2004/12/18 13:20:37  sezero
 * make the music automatically restart when changed in the options menu
 *
 * Revision 1.5  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.4  2004/11/29 16:22:23  sezero
 * Grrrr......
 *
 * Revision 1.3  2004/11/29 16:02:07  sezero
 * bump version to 1.2.1a to reflect the latest changes
 *
 * Revision 1.2  2004/11/28 00:58:08  sezero
 *
 * Commit Steven's changes as of 2004.11.24:
 *
 * * Rewritten Help/Version message(s)
 * * Proper fullscreen mode(s) for OpenGL.
 * * Screen sizes are selectable with "-width" and "-height" options.
 * * Mouse grab in window modes , which is released when menus appear.
 * * Interactive video modes in software game disabled.
 * * Replaced Video Mode menu with a helpful message.
 * * New menu items for GL Glow, Chase mode, Draw Shadows.
 * * Changes to initial cvar_t variables:
 *      r_shadows, gl_other_glows, _windowed_mouse,
 *
 * Revision 1.1.1.1  2004/11/28 00:06:10  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.5  2002/01/02 15:16:37  phneutre
 * bumped version number to 1.2.0
 *
 * Revision 1.4  2001/12/13 16:06:15  phneutre
 * initial support for userspace directory (~/.aot) (parms.userdir)
 *
 * Revision 1.3  2001/12/03 22:45:09  theoddone33
 * Change version numbers
 *
 * Revision 1.2  2001/11/12 23:31:58  theoddone33
 * Some Loki-ish parameters and general cleanup/bugfixes.
 *
 * 8     3/19/98 12:53p Jmonroe
 * 
 * 7     3/13/98 5:01a Mgummelt
 * May have finally fixed that damn monster stair-stepping problem...
 * 
 * 6     3/12/98 6:31p Mgummelt
 * 
 * 5     3/06/98 4:55p Mgummelt
 * 
 * 4     3/03/98 1:41p Jmonroe
 * removed old mp stuff
 * 
 * 3     3/01/98 7:30p Jweier
 * 
 * 54    10/29/97 5:39p Jheitzman
 * 
 * 53    10/28/97 2:58p Jheitzman
 * 
 * 51    10/06/97 6:04p Rjohnson
 * Fix for save games and version update
 * 
 * 50    9/25/97 11:56p Rjohnson
 * Version update
 * 
 * 49    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 48    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 47    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 46    9/02/97 12:24a Rjohnson
 * Version Update
 * 
 * 45    8/30/97 6:17p Rjohnson
 * Network changes
 * 
 * 44    8/29/97 2:49p Rjohnson
 * Network updates
 * 
 * 43    8/28/97 3:36p Rjohnson
 * Version Update
 * 
 * 42    8/27/97 12:11p Rjohnson
 * Version Update
 * 
 * 41    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 40    8/21/97 10:12p Rjohnson
 * Version Update
 * 
 * 39    8/20/97 2:59p Rjohnson
 * Version Update
 * 
 * 38    8/16/97 10:25a Rjohnson
 * Version Update
 * 
 * 37    8/11/97 2:52p Rlove
 * 
 * 36    8/09/97 10:39a Rjohnson
 * Version Update
 * 
 * 35    8/09/97 1:13a Bgokey
 * 
 * 34    8/05/97 3:55p Rjohnson
 * Version # Update
 * 
 * 33    8/05/97 3:49p Rjohnson
 * Fix for ipx networking
 * 
 * 32    8/01/97 6:25p Rjohnson
 * 
 * 31    7/30/97 1:56p Rjohnson
 * Changed version
 * 
 * 30    7/21/97 9:25p Rjohnson
 * Reduces the amount of memory used by RJNET
 * 
 * 29    7/11/97 5:21p Rjohnson
 * RJNET Updates
 * 
 * 28    6/16/97 5:38p Rlove
 * Temporary E3 fix of the max packet size MAX_DATAGRAM
 * 
 * 27    6/16/97 3:46p Rjohnson
 * Increased the reliable packet size
 * 
 * 26    6/06/97 11:21a Bgokey
 * 
 * 25    5/31/97 1:17p Bgokey
 * 
 * 24    5/27/97 4:46p Rjohnson
 * Added the smoke puff effect
 * 
 * 23    5/20/97 11:32a Rjohnson
 * Revised Effects
 * 
 * 22    5/19/97 2:54p Rjohnson
 * Added new client effects
 * 
 * 21    5/09/97 3:52p Rjohnson
 * Change to allow more than 256 precache models
 * 
 * 20    5/03/97 2:13p Bgokey
 * 
 * 19    4/20/97 5:05p Rjohnson
 * Networking Update
 * 
 * 18    4/15/97 9:02p Bgokey
 * 
 * 17    4/04/97 3:06p Rjohnson
 * Networking updates and corrections
 * 
 * 16    4/02/97 1:03p Bgokey
 * 
 * 15    3/07/97 1:57p Rjohnson
 * Id Updates
 * 
 * 14    2/27/97 4:12p Rjohnson
 * Added Midi prags
 * 
 * 13    2/24/97 3:51p Bgokey
 * Added HX_FRAME_TIME and HX_FPS.
 * 
 * 12    2/19/97 11:44a Rjohnson
 * Id Updates
 * 
 * 11    2/13/97 1:53p Bgokey
 * 
 * 10    2/07/97 1:37p Rlove
 * Artifact of Invincibility
 * 
 * 9     1/28/97 10:28a Rjohnson
 * Added experience and level advancement
 */
