/*
	qdir.c

	$Id: qdir.c,v 1.12 2008-01-29 12:03:10 sezero Exp $
*/


#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
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

static const char BUILDDIR[] = "h2mp";

char		qdir[1024];
char		gamedir[1024];
qboolean	archive;
char		archivedir[1024];


void SetQdirFromPath (const char *path)
{
	char	temp[1024];
	const char	*c, *mark;

	if (!(path[0] == '/' || path[0] == '\\' || path[1] == ':'))
	{	// path is partial
		Q_getwd (temp, sizeof(temp));
		qerr_strlcat(__thisfunc__, __LINE__, temp, path, sizeof(temp));
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
		if (!q_strncasecmp (c, BUILDDIR, sizeof(BUILDDIR) - 1))
		{
			strncpy (qdir, path, c + sizeof(BUILDDIR) - path);
			printf ("qdir: %s\n", qdir);
			// now search for a gamedir in path
			c += sizeof(BUILDDIR);
			while (c < mark)
			{
				if (*c == '/' || *c == '\\')
				{
					strncpy (gamedir, path, c + 1 - path);
					printf ("gamedir: %s\n", gamedir);
					return;
				}
				c++;
			}
			COM_Error ("No gamedir in %s", path);
		}
		--c;
	}
end:
	COM_Error ("%s: no '%s' in %s", __thisfunc__, BUILDDIR, path);
}

char *ExpandArg (const char *path)
{
	static char full[1024];

	if (path[0] != '/' && path[0] != '\\' && path[1] != ':')
	{
		Q_getwd (full, sizeof(full));
		qerr_strlcat(__thisfunc__, __LINE__, full, path, sizeof(full));
	}
	else
	{
		qerr_strlcpy(__thisfunc__, __LINE__, full, path, sizeof(full));
	}

	return full;
}

char *ExpandPath (const char *path)
{
	static char full[1024];

	if (!qdir[0])
		COM_Error ("%s called without qdir set", __thisfunc__);

	if (path[0] == '/' || path[0] == '\\' || path[1] == ':')
	{
		qerr_snprintf(__thisfunc__, __LINE__, full, sizeof(full), "%s", path);
	}
	else
	{
		qerr_snprintf(__thisfunc__, __LINE__, full, sizeof(full), "%s%s", qdir, path);
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
		qerr_snprintf(__thisfunc__, __LINE__, archivename, sizeof(archivename), "%s/%s", archivedir, path);
		Q_CopyFile (expanded, archivename);
	}
	return expanded;
}

