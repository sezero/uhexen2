/*
	pathutil.c
	filename handling utilities

	$Id: pathutil.h,v 1.1 2007-02-13 13:34:36 sezero Exp $
*/

#ifndef __PATHUTIL_H
#define __PATHUTIL_H

#define PATHSEPERATOR	'/'

void	DefaultExtension (char *path, const char *extension);
void	DefaultPath (char *path, const char *basepath);
void	StripFilename (char *path);
void	StripExtension (char *path);

void	ExtractFilePath (const char *path, char *dest);
void	ExtractFileBase (const char *path, char *dest);
void	ExtractFileExtension (const char *path, char *dest);

#endif	/* __PATHUTIL_H */

