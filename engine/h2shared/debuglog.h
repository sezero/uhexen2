/*
 * debuglog.h -- logging console output to a file.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2008-2010  O.Sezer <sezero@users.sourceforge.net>
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

#ifndef __DEBUGLOG_H
#define __DEBUGLOG_H

#define DEBUG_PREFIX		"DEBUG: "

/* log filenames: */
#if defined(H2W)
#define	DEBUGLOG_FILENAME	"debug_hw.log"
#elif defined(SERVERONLY)
#define	DEBUGLOG_FILENAME	"debugded.log"
#else
#define	DEBUGLOG_FILENAME	"debug_h2.log"
#endif

/* log level:	*/
#define	LOG_NONE		0	/* no logging	*/
#define	LOG_NORMAL		1	/* normal logging: what you see on the game console and terminal */
#define	LOG_DEVEL		2	/* log the _DPrintf content even if the developer cvar isn't set */

extern	unsigned int		con_debuglog;

void LOG_Print (const char *logdata);
void LOG_Printf(const char *fmt, ...) FUNC_PRINTF(1,2);

struct quakeparms_s;
void LOG_Init (struct quakeparms_s *parms);
void LOG_Close (void);

#endif	/* __DEBUGLOG_H */

