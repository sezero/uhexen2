/*
 * util_io.h -- file and directory utilities
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

#ifndef __UTILIO_H
#define __UTILIO_H

#if defined(PLATFORM_WINDOWS)
#include <io.h>
#include "io_msvc.h"
#elif !defined(PLATFORM_AMIGA)
#include <unistd.h>
#endif

#if defined(PLATFORM_WINDOWS) && !defined(F_OK)
/* constants for access() mode argument. MS does not define them.
 * Note that X_OK (0x01) must not be used in windows code.  */
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif

#define	FS_ENT_NONE		(0)
#define	FS_ENT_FILE		(1 << 0)
#define	FS_ENT_DIRECTORY	(1 << 1)

void	Q_mkdir (const char *path);
int	Q_rmdir (const char *path);
int	Q_unlink (const char *path);
int	Q_rename (const char *oldp, const char *newp);
void	Q_getwd (char *out, size_t size, qboolean trailing_dirsep);
long	Q_filesize (const char *path);
int	Q_FileType (const char *path);
const char *Q_FindFirstFile (const char *path, const char *pattern);
const char *Q_FindNextFile (void);
void	Q_FindClose (void);

long	Q_filelength (FILE *f);

FILE	*SafeOpenWrite (const char *filename);
FILE	*SafeOpenRead (const char *filename);
void	SafeRead (FILE *f, void *buffer, int count);
void	SafeWrite (FILE *f, const void *buffer, int count);

int	LoadFile (const char *filename, void **bufferptr);
void	SaveFile (const char *filename, const void *buffer, int count);

void	CreatePath (char *path);
int	Q_CopyFile (const char *frompath, const char *topath);
int	Q_WriteFileFromHandle (FILE *fromfile, const char *topath, size_t size);

#endif	/* __UTILIO_H */

