/*
	qwsvinc.h
	primary header for server

	FIXME:	kill this in the future and make each C
		file include only the necessary headers.

	$Id$
*/

#ifndef __HWSVINC_H
#define __HWSVINC_H

/* include the system stdc headers:			*/
#include "q_stdinc.h"

/* include the compiler specific stuff	*/
#include "compiler.h"
/* include the OS/arch definitions, etc	*/
#include "arch_def.h"

/* make sure to include our compile time options first	*/
#include "h2config.h"

/* include the quake headers				*/

#include "q_endian.h"
#include "sys.h"
#include "qsnprint.h"
#include "link_ops.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "printsys.h"
#include "common.h"
#include "quakefs.h"
#include "info_str.h"
#include "bspfile.h"
#include "zone.h"
#include "mathlib.h"
#include "cvar.h"

#include "protocol.h"
#include "net.h"

#include "cmd.h"
#include "crc.h"

#include "host.h"
#include "host_string.h"

#include "progs.h"
#include "effects.h"
#include "model.h"

#include "server.h"
#include "world.h"
#include "pmove.h"

#endif	/* __HWSVINC_H */

