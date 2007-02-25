/*
	quakefs.h
	Hexen II filesystem

	$Id: quakefs.h,v 1.5 2007-02-25 19:02:25 sezero Exp $
*/

#ifndef __QUAKEFS_H
#define __QUAKEFS_H

#if defined(_NEED_SEARCHPATH_T)
typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	struct	pack_s	*pack;		// only one of filename / pack will be used
	struct	searchpath_s *next;
} searchpath_t;

extern searchpath_t		*fs_searchpaths;
#endif	/* _NEED_SEARCHPATH_T */

extern	char	fs_basedir[MAX_OSPATH];
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

#endif	/* __QUAKEFS_H */

