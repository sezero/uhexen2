
#ifndef __QDIRS_H__
#define __QDIRS_H__

extern	char		qdir[1024];
extern	char		gamedir[1024];
extern	qboolean	archive;
extern	char		archivedir[1024];

void	SetQdirFromPath (char *path);
char	*ExpandArg (char *path);	// from cmd line
char	*ExpandPath (char *path);	// from scripts
char	*ExpandPathAndArchive (char *path);

#endif	/* __QDIRS_H__ */

