/*
	common.h
	common header for hexen2 launcher

	$Id: common.h,v 1.8 2007-08-09 06:08:22 sezero Exp $

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

#define MAX_OSPATH	256

// from main.c
extern char	basedir[MAX_OSPATH];
extern char	userdir[MAX_OSPATH];

// from launch_bin.c
void launch_hexen2_bin (void);

#endif	// LAUNCHER_COMMON_INC_H

