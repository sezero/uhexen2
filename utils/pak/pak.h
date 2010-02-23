/*
	pak.h
	$Id: pak.h,v 1.1 2010-02-23 12:40:12 sezero Exp $
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

