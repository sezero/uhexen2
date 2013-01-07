/*
 * userdir.h -- arch specific user directory definitions
 * $Id$
 *
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

#ifndef __USERDIR_H
#define __USERDIR_H

#if defined(DEMOBUILD)
/* user a different user directory for the demo version
 * so that the demo and retail versions can co-exist on
 * the same machine peacefully */
#define	SYS_USERDIR_OSX		"Library/Application Support/Hexen2 Demo"
#define	SYS_USERDIR_UNIX	".hexen2demo"

#else	/* for retail version: */

#define	SYS_USERDIR_OSX		"Library/Application Support/Hexen2"
#define	SYS_USERDIR_UNIX	".hexen2"

#endif


#if defined(PLATFORM_OSX)
#define	AOT_USERDIR		SYS_USERDIR_OSX
#else	/* unix: */
#define	AOT_USERDIR		SYS_USERDIR_UNIX
#endif

#endif	/* __USERDIR_H */

