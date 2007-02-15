/*
	quakefs.h
	Hexen II filesystem

	$Id: quakefs.h,v 1.2 2007-02-15 07:19:33 sezero Exp $
*/

#ifndef __QUAKEFS_H
#define __QUAKEFS_H

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	struct	pack_s	*pack;		// only one of filename / pack will be used
	struct	searchpath_s *next;
} searchpath_t;

extern searchpath_t		*fs_searchpaths;

extern	char	fs_basedir[MAX_OSPATH];
extern	char	fs_gamedir[MAX_OSPATH];
extern	char	fs_gamedir_nopath[MAX_OSPATH];
extern	char	fs_userdir[MAX_OSPATH];
extern	char	fs_savedir[MAX_OSPATH];	// temporary path for saving gip files

extern	struct cvar_s	registered;
extern	struct cvar_s	oem;
extern	unsigned int	gameflags;

void FS_Init (void);

void FS_Gamedir (const char *dir);
// Sets the gamedir and path to a different directory.

#endif	/* __QUAKEFS_H */

