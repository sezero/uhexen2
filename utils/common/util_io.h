/*
	util_io.h
	file and directory utilities

	$Id$
*/

#ifndef __UTILIO_H
#define __UTILIO_H

#if defined(PLATFORM_WINDOWS)
#include <io.h>
#include "io_msvc.h"
#else
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
void	Q_getwd (char *out, size_t size);
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

