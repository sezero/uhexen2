/*
 * printsys.h -- console printing
 * $Id: printsys.h,v 1.7 2010-01-11 18:48:17 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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

#ifndef __PRINTSYS_H
#define __PRINTSYS_H

/*
 * CON_Printf: Prints to the in-game console and manages other jobs, such
 * as echoing to the terminal and log.  In case of HexenWorld this may be
 * redirected. In that case, the message is printed to the relevant client.
 *
 * Location:	Graphical client : console.c
 *		HexenWorld server: sv_send.c
 *	Hexen II dedicated server: host.c
 */
void CON_Printf (unsigned int flags, const char *fmt, ...) FUNC_PRINTF(2,3);

/* common print flags */
#define	_PRINT_NORMAL			0	/* print to both terminal and to the in-game console */
#define	_PRINT_TERMONLY			1	/* print to the terminal only: formerly Sys_Printf */
#define	_PRINT_DEVEL			2	/* print only if the developer cvar is set */
#define	_PRINT_SAFE			4	/* okay to call even when the screen can't be updated */

/* macros for compatibility with quake api */
#if defined(__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define Con_Printf(fmt, args...)	CON_Printf(_PRINT_NORMAL, fmt, ##args)
#define Con_DPrintf(fmt, args...)	CON_Printf(_PRINT_DEVEL, fmt, ##args)
#define Con_SafePrintf(fmt, args...)	CON_Printf(_PRINT_SAFE, fmt, ##args)
#define Con_SafeDPrintf(fmt, args...)	CON_Printf(_PRINT_DEVEL|_PRINT_SAFE, fmt, ##args)
#else
#define Con_Printf(...)			CON_Printf(_PRINT_NORMAL, __VA_ARGS__)
#define Con_DPrintf(...)		CON_Printf(_PRINT_DEVEL, __VA_ARGS__)
#define Con_SafePrintf(...)		CON_Printf(_PRINT_SAFE, __VA_ARGS__)
#define Con_SafeDPrintf(...)		CON_Printf(_PRINT_DEVEL|_PRINT_SAFE, __VA_ARGS__)
#endif

/* these macros print to the terminal only */
#if defined(__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define Sys_Printf(fmt, args...)	CON_Printf(_PRINT_TERMONLY, fmt, ##args)
#define Sys_DPrintf(fmt, args...)	CON_Printf(_PRINT_TERMONLY|_PRINT_DEVEL, fmt, ##args)
#else
#define Sys_Printf(...)			CON_Printf(_PRINT_TERMONLY, __VA_ARGS__)
#define Sys_DPrintf(...)		CON_Printf(_PRINT_TERMONLY|_PRINT_DEVEL, __VA_ARGS__)
#endif

#if defined(DEBUG_BUILD)
#define DEBUG_Printf			Sys_DPrintf
#else	/* not debug : */
#if defined (__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define DEBUG_Printf(fmt, args...)	do {} while (0)
#else
#define DEBUG_Printf(...)		do {} while (0)
#endif
#endif

#endif	/* __PRINTSYS_H */

