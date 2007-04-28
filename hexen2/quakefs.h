/*
	quakefs.h
	Hexen II filesystem

	$Id: quakefs.h,v 1.9 2007-04-28 15:31:04 sezero Exp $
*/

#ifndef __QUAKEFS_H
#define __QUAKEFS_H

//
// game data flags
//
#define	GAME_DEMO		(1 << 0)
#define	GAME_OEM		(1 << 1)
#define	GAME_MODIFIED		(1 << 2)
#define	GAME_REGISTERED		(1 << 3)
#define	GAME_REGISTERED0	(1 << 4)
#define	GAME_REGISTERED1	(1 << 5)
#define	GAME_PORTALS		(1 << 6)
#define	GAME_HEXENWORLD		(1 << 7)

/* FIXME:  data for Raven's interim releases, such
   as 1.07, 1.08, 1.09 and 1.10 are not available.
   Similarly, more detailed data are needed for the
   oem (Matrox m3D bundle) version.		*/
#define	GAME_OLD_CDROM0		(1 << 8)
#define	GAME_OLD_CDROM1		(1 << 9)
#define	GAME_OLD_DEMO		(1 << 10)
#define	GAME_REGISTERED_OLD	(1 << 11)

extern	char	*fs_basedir;
extern	char	fs_gamedir[MAX_OSPATH];
extern	char	fs_gamedir_nopath[32];
extern	char	fs_userdir[MAX_OSPATH];

/* NOTE:  the savedir (fs_savedir, or the
   old com_savedir) is no longer available
   as a separate variable. the saves are
   always put in the current userdir now.
 */

extern	struct cvar_s	registered;
extern	struct cvar_s	oem;
extern	unsigned int	gameflags;

void FS_Init (void);

void FS_Gamedir (const char *dir);
// Sets the gamedir and path to a different directory.


//
// file i/o within qfs
//
extern	size_t	fs_filesize;	// size of the last file opened through QFS api
extern	char	*fs_filepath;	// path of the last file opened through QFS api
						// NULL for files in a pakfile.

extern	int	file_from_pak;	// global indicating that file came from a pak

int FS_CopyFile (const char *frompath, const char *topath);
// Copies the FROMPATH file as TOPATH file, creating any dirs needed.
// Used for saving the game. Returns 0 on success, non-zero on error.

int FS_WriteFile (const char *filename, const void *data, size_t len);
// Prefixes the filename by the current game directory and does an fwrite()
// Returns 0 on success, 1 on error.

int FS_CreatePath (char *path);
// Creates directory under user's path, making parent directories as needed.
// The path must either be a path to a file, or, if the full path is meant to
// be created, it must have the trailing path seperator. Returns 0 on success,
// non-zero on error.

int FS_FileInGamedir (const char *fname);
// Reports the existance of a file with read perms in fs_gamedir or fs_userdir.
// Returns -1 on failure. Files in pakfiles are NOT meant for this procedure!

size_t FS_OpenFile (const char *filename, FILE **file, qboolean override_pack);

byte *FS_LoadZoneFile (const char *path, int zone_id);
byte *FS_LoadStackFile (const char *path, void *buffer, size_t bufsize);
byte *FS_LoadBufFile (const char *path, void *buffer, size_t *bufsize);
byte *FS_LoadTempFile (const char *path);
byte *FS_LoadHunkFile (const char *path);
byte *FS_LoadMallocFile (const char *path);

struct cache_user_s;
void  FS_LoadCacheFile (const char *path, struct cache_user_s *cu);

#endif	/* __QUAKEFS_H */

