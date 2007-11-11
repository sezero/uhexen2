/*
	qdir.h

	$Id: qdir.h,v 1.5 2007-11-11 16:11:46 sezero Exp $
*/

#ifndef __QDIRS_H__
#define __QDIRS_H__

extern	char		qdir[1024];
extern	char		gamedir[1024];
extern	qboolean	archive;
extern	char		archivedir[1024];

void	SetQdirFromPath (const char *path);
char	*ExpandArg (const char *path);	// from cmd line
char	*ExpandPath (const char *path);	// from scripts
char	*ExpandPathAndArchive (const char *path);

#endif	/* __QDIRS_H__ */

