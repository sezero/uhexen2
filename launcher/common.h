/*
 * common.h
 * common header for hexen2 launcher
 *
 * $Id$
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

#ifndef	LAUNCHER_COMMON_INC_H
#define	LAUNCHER_COMMON_INC_H

#include <sys/types.h>
#include <stddef.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>

#if !defined(PATH_MAX)
#if defined(MAXPATHLEN)
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#define MAX_OSPATH PATH_MAX

/* =====================================================================
   USE_PASSWORD_FILE, 0 or 1 (main.c)
   On any sane unix system we shall get the home directory based on the
   real uid. If this fails (not finding the user in the password file
   isn't normal) or if you disable this, we will get it by reading the
   HOME environment variable, only.
   =================================================================== */
#define	USE_PASSWORD_FILE		1

/* from main.c */
extern char	basedir[MAX_OSPATH];
extern char	userdir[MAX_OSPATH];

/* from launch_bin.c */
void launch_hexen2_bin (void);

#endif	/* LAUNCHER_COMMON_INC_H */

