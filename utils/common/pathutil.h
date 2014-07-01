/*
 * pathutil.c -- filename handling utilities
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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
 */

#ifndef __PATHUTIL_H
#define __PATHUTIL_H

void	DefaultExtension (char *path, const char *extension, size_t len);
void	DefaultPath (char *path, const char *basepath, size_t len);
void	StripFilename (char *path);
void	StripExtension (char *path);

void	ExtractFilePath (const char *in, char *out, size_t outsize);
void	ExtractFileBase (const char *in, char *out, size_t outsize);
void	ExtractFileExtension (const char *in, char *out, size_t outsize);

const char *FileGetExtension (const char *in); /* doesn't return NULL */

#endif	/* __PATHUTIL_H */

