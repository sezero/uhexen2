/*
	qdir.c

	$Id: qdir.c,v 1.5 2007-02-17 07:56:17 sezero Exp $
*/


#include "util_inc.h"
#include "cmdlib.h"
#include "util_io.h"

/*
	qdir will hold the path up to the quake directory,
	including the slash:

	f:\quake\
	/raid/quake/

	gamedir will hold qdir + the game directory (id1, id2, etc)
*/

char		qdir[1024];
char		gamedir[1024];
qboolean	archive;
char		archivedir[1024];

#define BUILDDIR "h2mp"


void SetQdirFromPath (char *path)
{
	char	temp[1024];
	char	*c;

	if (!(path[0] == '/' || path[0] == '\\' || path[1] == ':'))
	{	// path is partial
		Q_getwd (temp);
		strcat (temp, path);
		path = temp;
	}

	// search for "quake" in path

	for (c = path ; *c ; c++)
	{
		if (!Q_strncasecmp (c, BUILDDIR, sizeof(BUILDDIR)-1))
		{
			strncpy (qdir, path, c+sizeof(BUILDDIR)-path);
			printf ("qdir: %s\n", qdir);
			c += sizeof(BUILDDIR);
			while (*c)
			{
				if (*c == '/' || *c == '\\')
				{
					strncpy (gamedir, path, c+1-path);
					printf ("gamedir: %s\n", gamedir);
					return;
				}
				c++;
			}
			Error ("No gamedir in %s", path);
			return;
		}
	}

	Error ("%s: no %s in %s", __FUNCTION__, BUILDDIR, path);
}

char *ExpandArg (const char *path)
{
	static char full[1024];

	if (path[0] != '/' && path[0] != '\\' && path[1] != ':')
	{
		Q_getwd (full);
		strcat (full, path);
	}
	else
		strcpy (full, path);

	return full;
}

char *ExpandPath (const char *path)
{
	static char full[1024];

	if (!qdir)
		Error ("%s called without qdir set", __FUNCTION__);

	if (path[0] == '/' || path[0] == '\\' || path[1] == ':')
	{
		sprintf (full, "%s", path);
	}
	else
	{
		sprintf (full, "%s%s", qdir, path);
	}

	return full;
}

char *ExpandPathAndArchive (const char *path)
{
	char	*expanded;
	char	archivename[1024];

	expanded = ExpandPath (path);

	if (archive)
	{
		sprintf (archivename, "%s/%s", archivedir, path);
		Q_CopyFile (expanded, archivename);
	}
	return expanded;
}

