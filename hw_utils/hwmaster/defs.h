/*
	mst_defs.h
	common definitions for hexenworld master server

	$Id$
*/

#ifndef __HWMASTER_DEFS
#define __HWMASTER_DEFS

//=============================================================================
// Defines

#define __STRINGIFY(x) #x
#define STRINGIFY(x) __STRINGIFY(x)

#define VER_HWMASTER_MAJ	1
#define VER_HWMASTER_MID	2
#define VER_HWMASTER_MIN	7
#define VER_HWMASTER_STR	STRINGIFY(VER_HWMASTER_MAJ) "." STRINGIFY(VER_HWMASTER_MID) "." STRINGIFY(VER_HWMASTER_MIN)

#define HWM_USERDIR	".hwmaster"	/* user directory for unix	*/

#define	MAX_OSPATH	256		/* max length of a filesystem pathname	*/

#define MAX_NUM_ARGVS	50

#define FILTERS_FILE	"filters.ini"

#define	MAX_MSGLEN		1450	// max length of a reliable message
#define	MAX_DATAGRAM		1450	// max length of unreliable message

/* =====================================================================
   USE_PASSWORD_FILE, 0 or 1 (sys_main.c)
   On any sane unix system we shall get the home directory based on the
   real uid. If this fails (not finding the user in the password file
   isn't normal) or if you disable this, we will get it by reading the
   HOME environment variable, only. Also see sys.h where USE_PASSWORD_FILE
   may be disabled on purpose for some platforms.
   =================================================================== */
#define	USE_PASSWORD_FILE		1


//=============================================================================
// Includes

#include "sizebuf.h"
#include "msg_io.h"
#include "protocol.h"
#include "qsnprint.h"
#include "common.h"
#include "cmd.h"
#include "net.h"
#include "sys.h"
#include "server.h"


//=============================================================================
// Macros


//=============================================================================
// Globals

extern char		com_token[1024];
extern int		com_argc;
extern char	**com_argv;


#endif	/* __HWMASTER_DEFS */

