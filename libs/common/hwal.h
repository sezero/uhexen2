/*
 * hwal.h
 * Hexen II, .WAL texture file format
 *
 * $Id$
 *
 * Copyright (C) 1996-2001  Id Software, Inc.
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

#ifndef __HWAL_H
#define __HWAL_H

// Little-endian "HWAL"
#define IDWALHEADER	(('L'<<24)+('A'<<16)+('W'<<8)+'H')

#define WALVERSION	1

#if !defined (MIPLEVELS)
#define	MIPLEVELS	4
#endif	/* MIPLEVELS */

// this format, based on a quake2 WAL structure, was put together
// by Jacques 'Korax' Krige.  compared to miptex_t, the miptex_wal_t
// structure has two extra int fields at the beginning and the name
// field is 32 chars long instead of 16. the rest, ie. the offsets,
// are the same.
typedef struct miptex_wal_s
{
	int			ident;
	int			version;
	char		name[32];
	unsigned int	width, height;
	unsigned int	offsets[MIPLEVELS];	// four mip maps stored
} miptex_wal_t;


#define	WAL_EXT_DIRNAME		"textures"
#define	WAL_REPLACE_ASTERIX		'_'
			/* character to replace '*' in texture names. */


#endif	/* __HWAL_H */

