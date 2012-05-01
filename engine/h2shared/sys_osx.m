/*
 * Mac OS X specific functions needed by the common sys_unix.c.
 * $Id$
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
 *
 */

#include "quakedef.h"
#include "sys_osx.h"
#include "userdir.h"

#include <libgen.h>	/* dirname() and basename() */
#import <Cocoa/Cocoa.h>	/* NSRunCriticalAlertPanel() */

/*
OSX_StripAppBundle:  If passed dir is suffixed with the directory
structure of a Mac OS X .app bundle, the .app directory structure
is stripped off the end and the result is returned. If not, dir is
returned untouched.  Based on the ioquake3 project at icculus.org.

For Mac OS X, we package uHexen2 like this:

Hexen II ( --> the holder directory.)
|
 - Hexen II gl.app (the bundle dir for the opengl application)
|  |
|   - Contents
|  |  |
|  |   - MacOS	(the actual binary resides here)
|  |
|   - Resources (icons here)
|
 - data1	( --> main game data directory)
|
 - portals	( --> expansion pack game data directory)
*/
static char *OSX_StripAppBundle (char *dir)
{
	static char	osx_path[MAX_OSPATH];

	q_strlcpy (osx_path, dir, sizeof(osx_path));
	if (strcmp(basename(osx_path), "MacOS"))
		return dir;
	q_strlcpy (osx_path, dirname(osx_path), sizeof(osx_path));
	if (strcmp(basename(osx_path), "Contents"))
		return dir;
	q_strlcpy (osx_path, dirname(osx_path), sizeof(osx_path));
	if (!strstr(basename(osx_path), ".app"))
		return dir;
	q_strlcpy (osx_path, dirname(osx_path), sizeof(osx_path));
	return osx_path;
}

int OSX_GetBasedir (char *argv0, char *dst, size_t dstsize)
{
	char	*tmp;

	if (realpath(argv0, dst) == NULL)
	{
		if (getcwd(dst, dstsize - 1) == NULL)
			return -1;
	}
	else
	{
		/* strip off the binary name */
		tmp = strdup (dst);
		if (!tmp)
			return -1;
		q_strlcpy (dst, dirname(tmp), dstsize);
		free (tmp);
	}

	tmp = OSX_StripAppBundle(dst);
	if (tmp != dst)
		q_strlcpy (dst, tmp, dstsize);

	return 0;
}


/* Display the message from Sys_Error() on a window */

void Cocoa_ErrorMessage (const char *errorMsg)
{
	NSRunCriticalAlertPanel(@"Hexen II Error",
				[NSString stringWithUTF8String:errorMsg],
				@"OK", nil, nil);
}

