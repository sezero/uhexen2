/*
	qwsvinc.h
	primary header for server

	FIXME:	kill this in the future and make each C
		file include only the necessary headers.

	$Id: qwsvinc.h,v 1.10 2007-07-11 16:47:16 sezero Exp $
*/

#ifndef __HWSVINC_H
#define __HWSVINC_H

/* include the compiler specific stuff	*/
#include "compiler.h"
/* include the OS/arch definitions, etc	*/
#include "arch_def.h"

/* make sure to include our compile time options first	*/
#include "h2option.h"

/* include the system headers: make sure to use q_types.h
   instead of <sys/types.h>				*/
#include "q_types.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#if !defined(PLATFORM_WINDOWS)
#include <strings.h>	/* strcasecmp and strncasecmp	*/
#endif	/* ! PLATFORM_WINDOWS */
#include <stdarg.h>
#include <stdlib.h>

/* include the quake headers				*/

#include "q_endian.h"
#include "link_ops.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "printsys.h"
#include "common.h"
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

#include "host.h"

#include "progs.h"
#include "pr_strng.h"
#include "cl_effect.h"
#include "model.h"

#include "server.h"
#include "world.h"
#include "pmove.h"

#endif	/* __HWSVINC_H */

