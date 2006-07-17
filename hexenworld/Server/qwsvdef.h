/*
	quakedef.h
	primary header for server

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Server/qwsvdef.h,v 1.9 2006-07-17 14:48:41 sezero Exp $
*/

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

#include "bothdefs.h"

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

void SV_Error (char *error, ...);
void SV_Init (quakeparms_t *parms);

void Con_Printf (char *fmt, ...);
void Con_DPrintf (char *fmt, ...);

extern	unsigned int	defLosses;	// Defenders losses in Siege
extern	unsigned int	attLosses;	// Attackers Losses in Siege

