/*
	quakeinc.h
	primary header for client and server

	$Id: quakeinc.h,v 1.4 2007-02-13 16:30:28 sezero Exp $
*/

#ifndef __QUAKEINC_H
#define __QUAKEINC_H

#include "q_types.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#if !defined(_WIN32)
// for strcasecmp and strncasecmp
#include <strings.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <setjmp.h>

#include "q_endian.h"
#include "link_ops.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "common.h"
#include "quakeio.h"
#include "quakefs.h"
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
#endif	/* !SERVERONLY */

#include "progs.h"
#include "pr_strng.h"
#include "cl_effect.h"

#if defined(SERVERONLY)
#include "h2ded.h"
#else
#include "client.h"
#endif	/* SERVERONLY */
#include "server.h"

#if defined(GLQUAKE)
#include "glheader.h"
#include "gl_model.h"
#include "glquake.h"
#elif defined(SERVERONLY)
#include "server/model.h"
#else
#include "model.h"
#include "d_iface.h"
#endif

#include "world.h"

#if !defined(SERVERONLY)
#include "input.h"
#include "keys.h"
#include "menu.h"
#endif	/* !SERVERONLY */

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

#endif	/* __QUAKEINC_H */

