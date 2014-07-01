/*
 * mst_defs.h
 * common definitions for hexenworld master server
 *
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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

#ifndef __HWMASTER_DEFS
#define __HWMASTER_DEFS


#define __STRINGIFY(x) #x
#define STRINGIFY(x) __STRINGIFY(x)

#define VER_HWMASTER_MAJ	1
#define VER_HWMASTER_MID	2
#define VER_HWMASTER_MIN	7
#define VER_HWMASTER_STR	STRINGIFY(VER_HWMASTER_MAJ) "." STRINGIFY(VER_HWMASTER_MID) "." STRINGIFY(VER_HWMASTER_MIN)

/* =====================================================================
   DO_USERDIRS: 0 or 1
   Allows separating user directories on multi-user systems. We HIGHLY
   recommend keeping it as 1. Also see "sys.h" where DO_USERDIRS may be
   disabled on purpose for some platforms.  Only userdir functionality
   in hwmaster is where the filters.ini is stored: under the userdir if
   configured for userdirs, or under the currentdir otherwise.
   =================================================================== */
#define DO_USERDIRS	1
#define HWM_USERDIR	".hwmaster"	/* user directory for unix	*/

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


/* Includes */

#include "sys.h"
#include "sizebuf.h"
#include "msg_io.h"
#include "protocol.h"
#include "qsnprint.h"
#include "common.h"
#include "cmd.h"
#include "net.h"
#include "server.h"


/* Globals */

extern char	com_token[1024];
extern int	com_argc;
extern char	**com_argv;


#endif	/* __HWMASTER_DEFS */

