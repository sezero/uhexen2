/*
	pakfile.h
	in-memory pak structures

	$Id: pak.h,v 1.1 2007-04-19 09:36:25 sezero Exp $
*/

#ifndef __PAK_H
#define __PAK_H


typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

#endif	/* __PAK_H */

