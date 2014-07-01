/*
 * sys.h: non-portable functions
 * relies on: arch_def.h
 * $Id$
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

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

void Sys_Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
void Sys_Quit (void) __attribute__((__noreturn__));
double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

/* disable user directories on platforms where they
 * aren't necessary or not possible. */
#if defined(PLATFORM_DOS) || defined(PLATFORM_AMIGA) || \
    defined(PLATFORM_WINDOWS)
#undef	DO_USERDIRS
#define	DO_USERDIRS	0
#endif	/* DO_USERDIRS  */

#endif	/* __HX2_SYS_H */

