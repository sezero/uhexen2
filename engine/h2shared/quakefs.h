/*
 * quakefs.h -- Hexen II filesystem
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

#ifndef __QUAKEFS_H
#define __QUAKEFS_H

/* game data flags */
#define	GAME_DEMO		(1 << 0)
#define	GAME_OEM		(1 << 1)
#define	GAME_OEM0		(1 << 2)
#define	GAME_OEM2		(1 << 3)
#define	GAME_REGISTERED		(1 << 4)
#define	GAME_REGISTERED0	(1 << 5)
#define	GAME_REGISTERED1	(1 << 6)
#define	GAME_PORTALS		(1 << 7)
#define	GAME_HEXENWORLD		(1 << 8)

/* FIXME: more detailed data needed for the oem
 * (Matrox m3D bundle) original 1.08 version.  */
#define	GAME_OLD_CDROM0		(1 << 9)
#define	GAME_OLD_CDROM1		(1 << 10)
#define	GAME_OLD_DEMO		(1 << 11)
#define	GAME_REGISTERED_OLD	(1 << 12)
#define	GAME_OLD_OEM		(1 << 13)
#define	GAME_OLD_OEM0		(1 << 14)
#define	GAME_OLD_OEM2		(1 << 15)

#define	GAME_MODIFIED		(1 << 16)

extern	char	fs_gamedir_nopath[MAX_QPATH];

const char *FS_GetBasedir (void);
const char *FS_GetUserbase (void);
const char *FS_GetGamedir (void);
const char *FS_GetUserdir (void);

extern	struct cvar_s	registered;
extern	struct cvar_s	oem;
extern	unsigned int	gameflags;

void FS_Init (void);

void FS_Gamedir (const char *dir);
	/* Sets the gamedir and path to a different directory. */


/* file i/o within qfs */
extern	size_t	fs_filesize;	/* size of the last file opened through QFS api */

extern	int	file_from_pak;	/* global indicating that file came from a pak. */

#define	FS_ENT_NONE		(0)
#define	FS_ENT_FILE		(1 << 0)
#define	FS_ENT_DIRECTORY	(1 << 1)

int FS_CopyFile (const char *frompath, const char *topath);
	/* Copies the FROMPATH file as TOPATH file, creating any dirs needed.
	 * Used for saving the game. Returns 0 on success, non-zero on error. */

int FS_WriteFileFromHandle (FILE *fromfile, const char *topath, size_t size);
	/* takes an open file as its source, writes a new file copying `size' bytes */

int FS_WriteFile (const char *filename, const void *data, size_t len);
	/* Prefixes the filename by the current game directory and does an fwrite()
	 * Returns 0 on success, 1 on error.  */

int FS_CreatePath (char *path);
	/* Creates directory under user's path, making parent directories as needed.
	 * The path must either be a path to a file, or, if the full path is meant to
	 * be created, it must have the trailing path seperator. Returns 0 on success,
	 * non-zero on error. */

size_t FS_OpenFile (const char *filename, FILE **file, unsigned int *path_id);
	/* Opens a file (a standalone file or a file in pak) in the hexen2 filesystem,
	 * returns fs_filesize on success or (size_t)-1 on failure.  if path_id is not
	 * NULL, the id number of the opened file's gamedir is stored in path_id.  */

qboolean FS_FileExists (const char *filename, unsigned int *path_id);
	/* Returns whether the file is found in the hexen2 filesystem.  if path_id is
	 * not NULL, the id number of the found file's gamedir is stored in path_id. */

qboolean FS_FileInGamedir (const char *filename);
	/* Reports the existance of a file with read permissions in
	 * fs_gamedir or fs_userdir. *NOT* for files in pakfiles!  */

/* these procedures open a file using FS_OpenFile and loads it into a proper
 * buffer. the buffer is allocated with a total size of fs_filesize + 1. the
 * procedures differ by their buffer allocation method.  */
byte *FS_LoadZoneFile (const char *path, int zone_id, unsigned int *path_id);
	/* allocates the buffer on the zone. zone_id: which zone to use.  */
byte *FS_LoadTempFile (const char *path, unsigned int *path_id);
	/* allocates the buffer on the temp hunk.  */
byte *FS_LoadHunkFile (const char *path, unsigned int *path_id);
	/* allocates the buffer on the hunk.  */
byte *FS_LoadMallocFile (const char *path, unsigned int *path_id);
	/* allocates the buffer on the system mem (malloc).  */
byte *FS_LoadStackFile (const char *path, void *buffer, size_t bufsize,
							unsigned int *path_id);
	/* uses the specified stack stack buffer with the specified size
	 * of bufsize. if bufsize is too short, uses temp hunk. the bufsize
	 * must include the +1  */

struct cache_user_s;
void  FS_LoadCacheFile (const char *path, struct cache_user_s *cu,
							unsigned int *path_id);
	/* uses cache mem for allocating the buffer.  */

#define	FS_BASEDIR	0	/* host_parms->basedir (i.e.:  fs_basedir) */
#define	FS_USERBASE	1	/* host_parms->userdir */
#define	FS_GAMEDIR	2	/* host_parms->basedir/gamedir (fs_gamedir) */
#define	FS_USERDIR	3	/* host_parms->userdir/gamedir (fs_userdir) */

char *FS_MakePath (int base, int *error, const char *path);
	/* strcat to an FS path basename. returns a static buffer.  */
char *FS_MakePath_VA (int base, int *error, const char *format, ...)
				__attribute__((__format__(__printf__,3,4)));
				/* like FS_MakePath(), but does varargs.  */
char *FS_MakePath_BUF (int base, int *error, char *buf, size_t siz, const char *path);
char *FS_MakePath_VABUF (int base, int *error, char *buf, size_t siz, const char *format, ...)
				__attribute__((__format__(__printf__,5,6)));
		/* like the two above, but with a user-provided buffer.  */

/* The following FS_*() stdio replacements are necessary if one is
 * to perform non-sequential reads on files reopened on pak files
 * because we need the bookkeeping about file start/end positions.
 * Allocating and filling in the fshandle_t structure is the users'
 * responsibility when the file is initially opened. */

typedef struct _fshandle_t
{
	FILE *file;
	qboolean pak;	/* is the file read from a pak */
	long start;	/* file or data start position */
	long length;	/* file or data size */
	long pos;	/* current position relative to start */
} fshandle_t;

size_t FS_fread(void *ptr, size_t size, size_t nmemb, fshandle_t *fh);
int FS_fseek(fshandle_t *fh, long offset, int whence);
long FS_ftell(fshandle_t *fh);
void FS_rewind(fshandle_t *fh);
int FS_feof(fshandle_t *fh);
int FS_ferror(fshandle_t *fh);
int FS_fclose(fshandle_t *fh);
int FS_fgetc(fshandle_t *fh);
char *FS_fgets(char *s, int size, fshandle_t *fh);
long FS_filelength (fshandle_t *fh);

#endif	/* __QUAKEFS_H */

