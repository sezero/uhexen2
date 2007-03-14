/*
	qdir.h

	$Id: qdir.h,v 1.4 2007-03-14 21:04:31 sezero Exp $
*/

#ifndef __QDIRS_H__
#define __QDIRS_H__

extern	char		qdir[1024];
extern	char		gamedir[1024];
extern	qboolean	archive;
extern	char		archivedir[1024];

void	SetQdirFromPath (char *path);
char	*ExpandArg (const char *path);	// from cmd line
char	*ExpandPath (const char *path);	// from scripts
char	*ExpandPathAndArchive (const char *path);

#endif	/* __QDIRS_H__ */

