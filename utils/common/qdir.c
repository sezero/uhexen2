/*
	qdir.c

	$Id: qdir.c,v 1.7 2007-05-13 11:59:44 sezero Exp $
*/


#include "util_inc.h"
#include "cmdlib.h"
#include "util_io.h"

/*
	qdir will hold the path up to the base hexen2 directory (as
	defined in BUILDDIR), including the slash:
		c:\h2mp\
		/usr/local/games/h2mp/
	gamedir will hold qdir + the game directory (data1, portals,
	etc). SetQdirFromPath requires an input containing both the
	basedir and the gamedir:
		c:\h2mp\data1\somefile.dat
		/usr/local/games/h2mp/data1/somefile.dat
	or similar partials:
		data1\somefile.dat
		h2mp/data1/somefile.dat
*/

#define	BUILDDIR	"h2mp"

char		qdir[1024];
char		gamedir[1024];
qboolean	archive;
char		archivedir[1024];


void SetQdirFromPath (char *path)
{
	char	temp[1024];
	char	*c, *mark;

	if (!(path[0] == '/' || path[0] == '\\' || path[1] == ':'))
	{	// path is partial
		Q_getwd (temp);
		strcat (temp, path);
		path = temp;
	}

	c = path;
	while (*c)
		++c;
	while (c > path && *c != '/' && *c != '\\')
		--c;
	if (c == path)
		goto end;
	mark = c + 1;
	--c;
	// search for the basedir (as defined in BUILDDIR) in path
	while (c != path)
	{
		if (!Q_strncasecmp (c, BUILDDIR, sizeof(BUILDDIR)-1))
		{
			strncpy (qdir, path, c+sizeof(BUILDDIR)-path);
			printf ("qdir: %s\n", qdir);
			// now search for a gamedir in path
			c += sizeof(BUILDDIR);
			while (c < mark)
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
		}
		--c;
	}
end:
	Error ("%s: no '%s' in %s", __thisfunc__, BUILDDIR, path);
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
		Error ("%s called without qdir set", __thisfunc__);

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

