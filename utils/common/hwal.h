/*
	hwal.h
	Hexen II, .WAL texture file format

	$Id: hwal.h,v 1.2 2007-09-14 14:11:24 sezero Exp $
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

