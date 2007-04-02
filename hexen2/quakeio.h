/*
	quakeio.h
	quake file io

	$Id: quakeio.h,v 1.3 2007-04-02 21:06:00 sezero Exp $
*/

#ifndef __QUAKEIO_H
#define __QUAKEIO_H

#if defined(_WIN32) && !defined(F_OK)
// values for the mode argument of access(). MS does not define them
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif

extern	size_t	qio_filesize;	// size of the last file opened through QIO api
extern	char	*qio_filepath;	// path of the last file opened through QIO api
						// NULL for files in a pakfile.

extern	int	file_from_pak;	// global indicating that file came from a pak

int QIO_WriteFile (const char *filename, const void *data, size_t len);
// Prefixes the filename by the current game directory and does an fwrite()
// Returns 0 on success, 1 on error.

int QIO_CopyFile (const char *frompath, const char *topath);
// Copies the FROMPATH file as TOPATH file, creating any dirs needed.
// Used for saving the game. Returns 0 on success, non-zero on error.

int QIO_CreatePath (char *path);
// Creates directory under user's path, making parent directories as needed.
// The path must either be a path to a file, or, if the full path is meant to
// be created, it must have the trailing path seperator. Returns 0 on success,
// non-zero on error.

int QFS_FileInGamedir (const char *fname);
// Reports the existance of a file with read perms in fs_gamedir or fs_userdir.
// Returns -1 on failure. Files in pakfiles are NOT meant for this procedure!

size_t QIO_FOpenFile (const char *filename, FILE **file, qboolean override_pack);

byte *QIO_LoadStackFile (const char *path, void *buffer, size_t bufsize);
byte *QIO_LoadBufFile (const char *path, void *buffer, size_t *bufsize);
byte *QIO_LoadTempFile (const char *path);
byte *QIO_LoadHunkFile (const char *path);
byte *QIO_LoadZoneFile (const char *path);
byte *QIO_LoadMallocFile (const char *path);

struct cache_user_s;
void  QIO_LoadCacheFile (const char *path, struct cache_user_s *cu);

#endif	/* __QUAKEIO_H */

