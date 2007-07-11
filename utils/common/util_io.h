/*
	util_io.h
	file and directory utilities

	$Id: util_io.h,v 1.4 2007-07-11 16:47:20 sezero Exp $
*/

#ifndef __UTILIO_H
#define __UTILIO_H

#if defined(PLATFORM_WINDOWS)
#include <io.h>
#else
#include <unistd.h>
#endif

#if defined(PLATFORM_WINDOWS) && !defined(F_OK)
// values for the second argument to access(). MS does not define them
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif

void	Q_mkdir (const char *path);
void	Q_getwd (char *out);
int	Q_filelength (FILE *f);
int	FileTime (const char *path);

char	*Q_FindFirstFile (const char *path, const char *pattern);
char	*Q_FindNextFile (void);
void	Q_FindClose (void);

FILE	*SafeOpenWrite (const char *filename);
FILE	*SafeOpenRead (const char *filename);
void	SafeRead (FILE *f, void *buffer, int count);
void	SafeWrite (FILE *f, const void *buffer, int count);

int	LoadFile (const char *filename, void **bufferptr);
void	SaveFile (const char *filename, const void *buffer, int count);

void	CreatePath (char *path);
void	Q_CopyFile (const char *from, const char *to);

#endif	/* __UTILIO_H */

