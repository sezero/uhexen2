/*
	genmodel.h
	model structures

	$Id: genmodel.h,v 1.4 2007-03-14 21:03:07 sezero Exp $

	This file must be identical in the genmodel project and in the H3
	project.
*/

#ifndef __GENMODEL_H
#define __GENMODEL_H

#define ALIAS_VERSION		6
#define ALIAS_NEWVERSION	50

#define ALIAS_ONSEAM		0x0020

// Little-endian "IDPO"
#define IDPOLYHEADER (('O'<<24)+('P'<<16)+('D'<<8)+'I')
#define RAPOLYHEADER (('O'<<24)+('P'<<16)+('A'<<8)+'R')

// must match definition in spritegn.h
#ifndef SYNCTYPE_T
#define SYNCTYPE_T
typedef enum
{
	ST_SYNC = 0,
	ST_RAND
} synctype_t;
#endif

typedef enum
{
	ALIAS_SINGLE = 0,
	ALIAS_GROUP
} aliasframetype_t;

typedef enum
{
	ALIAS_SKIN_SINGLE = 0,
	ALIAS_SKIN_GROUP
} aliasskintype_t;

typedef struct
{
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;
} mdl_t;

typedef struct
{
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;
	int			num_st_verts;
} newmdl_t;

typedef struct
{
	// TODO: could be shorts
	int		onseam;
	int		s;
	int		t;
} stvert_t;

typedef struct dtriangle_s
{
	int	facesfront;
	int	vertindex[3];
} dtriangle_t;

typedef struct dnewtriangle_s
{
	int	facesfront;
	unsigned short	vertindex[3];
	unsigned short	stindex[3];
} dnewtriangle_t;

#define DT_FACES_FRONT	0x0010

typedef struct
{
	byte	v[3];
	byte	lightnormalindex;
} trivertx_t;

typedef struct
{
	trivertx_t	bboxmin;	// lightnormal isn't used
	trivertx_t	bboxmax;	// lightnormal isn't used
	char	name[16];	// frame name from grabbing
} daliasframe_t;

typedef struct
{
	int		numframes;
	trivertx_t	bboxmin;	// lightnormal isn't used
	trivertx_t	bboxmax;	// lightnormal isn't used
} daliasgroup_t;

typedef struct
{
	int		numskins;
} daliasskingroup_t;

typedef struct
{
	float	interval;
} daliasinterval_t;

typedef struct
{
	float	interval;
} daliasskininterval_t;

typedef struct
{
	aliasframetype_t	type;
} daliasframetype_t;

typedef struct
{
	aliasskintype_t		type;
} daliasskintype_t;

#endif	/* __GENMODEL_H */

