/*
	pakfile.h
	on-disk pak file structures

	$Id: pakfile.h,v 1.2 2007-04-19 09:36:29 sezero Exp $
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

