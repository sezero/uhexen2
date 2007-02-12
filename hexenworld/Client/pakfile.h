/*
	pakfile.h
	the pak file structure

	$Id: pakfile.h,v 1.1 2007-02-12 16:53:12 sezero Exp $
*/

#ifndef __PAKFILE_H
#define __PAKFILE_H

//
// on disk
//

#define	MAX_FILES_IN_PACK	2048

#define	PAK_PATH_LENGTH		56

typedef struct
{
	char	name[PAK_PATH_LENGTH];
	int		filepos, filelen;
} dpackfile_t;

typedef struct
{
	char	id[4];
	int		dirofs;
	int		dirlen;
} dpackheader_t;

//
// in memory
//

#if !defined(MAX_QPATH)
#define	MAX_QPATH		64	/* max length of a quake game pathname	*/
#endif	/* MAX_QPATH */

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

#if !defined(MAX_OSPATH)
#define	MAX_OSPATH		256	/* max length of a filesystem pathname	*/
#endif	/* MAX_OSPATH */

typedef struct pack_s
{
	char	filename[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
} pack_t;

#endif	/* __PAKFILE_H */

