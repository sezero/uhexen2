/*
 * pak.h
 * $Id: pak.h,v 1.1 2010-02-23 12:40:12 sezero Exp $
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

#ifndef _QUAKE_PAK_H
#define _QUAKE_PAK_H

typedef struct
{
	char	name[MAX_OSPATH];
	int		filepos, filelen;
} pakfiles_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	unsigned short	crc;
	int		numfiles;
	pakfiles_t	*files;
} pack_t;

pack_t *LoadPackFile (const char *packfile);

#endif	/* _QUAKE_PAK_H */

