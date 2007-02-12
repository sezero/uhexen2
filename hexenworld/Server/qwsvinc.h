/*
	qwsvinc.h
	primary header for server

	$Id: qwsvinc.h,v 1.1 2007-02-12 16:54:51 sezero Exp $
*/

#ifndef __HWSVINC_H
#define __HWSVINC_H

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
#include "info_str.h"
#include "bspfile.h"
#include "sys.h"
#include "zone.h"
#include "mathlib.h"
#include "cvar.h"

#include "protocol.h"
#include "net.h"

#include "cmd.h"
#include "crc.h"

#include "progs.h"
#include "pr_strng.h"
#include "cl_effect.h"
#include "model.h"

#include "server.h"
#include "world.h"
#include "pmove.h"

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

extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;

extern	qboolean	host_initialized;	// true if into command execution
extern	double		host_frametime;
extern	double		realtime;		// not bounded in any way, changed at
						// start of every frame, never reset

void SV_Error (const char *error, ...) _FUNC_PRINTF(1);
void SV_Init (quakeparms_t *parms);

void Con_Printf (const char *fmt, ...) _FUNC_PRINTF(1);
void Con_DPrintf (const char *fmt, ...) _FUNC_PRINTF(1);

#endif	/* __HWSVINC_H */

