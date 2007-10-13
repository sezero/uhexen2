/*
	mst_defs.h
	common definitions for hexenworld master server

	$Id: defs.h,v 1.32 2007-10-13 07:05:28 sezero Exp $
*/

#ifndef __HWMASTER_DEFS
#define __HWMASTER_DEFS

//=============================================================================
// Defines

#define __STRINGIFY(x) #x
#define STRINGIFY(x) __STRINGIFY(x)

#define VER_HWMASTER_MAJ	1
#define VER_HWMASTER_MID	2
#define VER_HWMASTER_MIN	4
#define VER_HWMASTER_STR	STRINGIFY(VER_HWMASTER_MAJ) "." STRINGIFY(VER_HWMASTER_MID) "." STRINGIFY(VER_HWMASTER_MIN)

#define HWM_USERDIR	".hwmaster"	/* user directory for unix	*/

#define	MAX_OSPATH	256		/* max length of a filesystem pathname	*/

#define MAX_NUM_ARGVS	50

#define	MAX_MSGLEN		1450	// max length of a reliable message
#define	MAX_DATAGRAM		1450	// max length of unreliable message

/* =====================================================================
   USE_PASSWORD_FILE, 0 or 1 (sys_main.c)
   On any sane unix system we shall get the home directory based on the
   real uid. If this fails (not finding the user in the password file
   isn't normal) or if you disable this, we will get it by reading the
   HOME environment variable, only.
   =================================================================== */
#define	USE_PASSWORD_FILE		1


//=============================================================================
// Includes

#include "compiler.h"
#include "arch_def.h"
#include "q_types.h"
#include <stdio.h>
#include <string.h>
#if !defined(PLATFORM_WINDOWS)
#include <strings.h>	/* strcasecmp and strncasecmp	*/
#endif	/* ! PLATFORM_WINDOWS */
#include <stdarg.h>
#include <stdlib.h>
#include "q_endian.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "protocol.h"
#include "common.h"
#include "cmd.h"
#include "net.h"
#include "sys.h"
#include "server.h"


//=============================================================================
// Macros

#undef	min
#undef	max
#define	q_min(a, b)		(((a) < (b)) ? (a) : (b))
#define	q_max(a, b)		(((a) > (b)) ? (a) : (b))


//=============================================================================
// Globals

extern char		com_token[1024];
extern int		com_argc;
extern char	**com_argv;


#endif	/* __HWMASTER_DEFS */

