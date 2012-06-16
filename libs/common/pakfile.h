/*
 * pakfile.h -- on-disk pak file structures
 *
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

#ifndef __PAKFILE_H
#define __PAKFILE_H

// Little-endian "PACK"
#define IDPAKHEADER		(('K'<<24)+('C'<<16)+('A'<<8)+'P')

#define	PAK_PATH_LENGTH		56

typedef struct
{
	char	name[PAK_PATH_LENGTH];
	int		filepos, filelen;
} dpackfile_t;

typedef struct
{
//	int		ident;	// == IDPAKHEADER
	char	id[4];
	int		dirofs;
	int		dirlen;
} dpackheader_t;

#define	packfile_t	dpackfile_t
#define	packheader_t	dpackheader_t
#define	MAX_FILES_IN_PACK	2048


#endif	/* __PAKFILE_H */

