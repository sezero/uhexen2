/*
 * qdir.h
 * $Id: qdir.h,v 1.5 2007-11-11 16:11:46 sezero Exp $
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

#ifndef __QDIRS_H__
#define __QDIRS_H__

extern	char		qdir[1024];
extern	char		gamedir[1024];
extern	qboolean	archive;
extern	char		archivedir[1024];

void	SetQdirFromPath (const char *path);
char	*ExpandArg (const char *path);	// from cmd line
char	*ExpandPath (const char *path);	// from scripts
char	*ExpandPathAndArchive (const char *path);

#endif	/* __QDIRS_H__ */

