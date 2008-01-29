/*
	pathutil.c
	filename handling utilities

	$Id: pathutil.h,v 1.2 2008-01-29 12:03:10 sezero Exp $
*/

#ifndef __PATHUTIL_H
#define __PATHUTIL_H

#define PATHSEPERATOR	'/'

void	DefaultExtension (char *path, const char *extension, size_t len);
void	DefaultPath (char *path, const char *basepath, size_t len);
void	StripFilename (char *path);
void	StripExtension (char *path);

void	ExtractFilePath (const char *in, char *out, size_t outsize);
void	ExtractFileBase (const char *in, char *out, size_t outsize);
void	ExtractFileExtension (const char *in, char *out, size_t outsize);

#endif	/* __PATHUTIL_H */

