/*
	common.h
	common header for hexen2 launcher

	$Id: common.h,v 1.6 2007-03-15 18:18:15 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#ifndef	LAUNCHER_COMMON_INC_H
#define	LAUNCHER_COMMON_INC_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>

#define MAX_OSPATH	256

// from launch_bin.c
void CheckExe (void);
void launch_hexen2_bin (void);

#endif	// LAUNCHER_COMMON_INC_H

