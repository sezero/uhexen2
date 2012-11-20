/*
 * quakeinc.h -- primary header for client and server
 * FIXME:	kill this in the future and make each C
 *		file include only the necessary headers.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __QUAKEINC_H
#define __QUAKEINC_H

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
#include "strl_fn.h"
#include "link_ops.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "printsys.h"
#include "common.h"
#include "quakefs.h"
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
#include "server.h"
#if defined(SERVERONLY)
#include "sv_model.h"
#include "world.h"

#else	/* client */
#include "console.h"
#include "wad.h"
#include "vid.h"
#include "screen.h"
#include "draw.h"
#include "render.h"
#include "view.h"
#include "sbar.h"
#include "q_sound.h"
#include "client.h"

#if defined(GLQUAKE)
#include "glheader.h"
#include "gl_model.h"
#include "glquake.h"
#else	/* sw client */
#include "model.h"
#include "d_iface.h"
#endif
#include "world.h"

#include "r_part.h"
#include "input.h"
#include "keys.h"
#include "menu.h"
#endif	/* !SERVERONLY */

#endif	/* __QUAKEINC_H */

