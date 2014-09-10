/*
 * Mac OS X specific functions needed by the common sys_unix.c.
 * $Id$
 *
 * Copyright (C) 2008-2012  O.Sezer <sezero@users.sourceforge.net>
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

#include <errno.h>	/* perror() */
#include <libgen.h>	/* dirname() and basename() */
#include <unistd.h>	/* getcwd() */
#import <Cocoa/Cocoa.h>	/* NSRunCriticalAlertPanel() */

static char *OSX_StripAppBundle (char *dir)
{ /* based on the ioquake3 project at icculus.org. */
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
		perror("realpath");
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


/* Display message from Sys_Error() on a window: */
void Cocoa_ErrorMessage (const char *errorMsg)
{
#if (MAC_OS_X_VERSION_MIN_REQUIRED < 1040)	/* ppc builds targeting 10.3 and older */
    NSString* msg = [NSString stringWithCString:errorMsg];
#else
    NSString* msg = [NSString stringWithCString:errorMsg encoding:NSASCIIStringEncoding];
#endif
    NSRunCriticalAlertPanel (@"Hexen II Error", msg, @"OK", nil, nil);
}


#define MAX_CLIPBOARDTXT	MAXCMDLINE	/* 256 */
char *Sys_GetClipboardData (void)
{
    char *data			= NULL;
    NSPasteboard* pasteboard	= [NSPasteboard generalPasteboard];
    NSArray* types		= [pasteboard types];

    if ([types containsObject: NSStringPboardType]) {
	NSString* clipboardString = [pasteboard stringForType: NSStringPboardType];
	if (clipboardString != NULL && [clipboardString length] > 0) {
		size_t sz = [clipboardString length] + 1;
		sz = q_min(MAX_CLIPBOARDTXT, sz);
		data = (char *) Z_Malloc(sz, Z_MAINZONE);
#if (MAC_OS_X_VERSION_MIN_REQUIRED < 1040)	/* for ppc builds targeting 10.3 and older */
		q_strlcpy (data, [clipboardString cString], sz);
#else
		q_strlcpy (data, [clipboardString cStringUsingEncoding: NSASCIIStringEncoding], sz);
#endif
	}
    }
    return data;
}

