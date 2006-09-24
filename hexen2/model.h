/*
	model.h
	header for model loading and caching

	$Id: model.h,v 1.7 2006-09-24 17:28:42 sezero Exp $
*/

#ifndef __HX2_MODEL_H
#define __HX2_MODEL_H

#include "genmodel.h"
#include "spritegn.h"

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vec3_t		position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// plane_t structure
// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct mplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

typedef struct texture_s
{
	char		name[16];
	unsigned	width, height;
	int		anim_total;		// total tenths in sequence ( 0 = no)
	int		anim_min, anim_max;	// time for this frame min <=time< max
	struct texture_s *anim_next;		// in the animation sequence
	struct texture_s *alternate_anims;	// bmodels in frmae 1 use these
	unsigned	offsets[MIPLEVELS];	// four mip maps stored
} texture_t;


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_TRANSLUCENT	0x80
#define SURF_DRAWBLACK		0x200

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct
{
	float		vecs[2][4];
	float		mipadjust;
	texture_t	*texture;
	int		flags;
} mtexinfo_t;

typedef struct msurface_s
{
	int		visframe;	// should be drawn when node is crossed

	int		dlightframe;
	int		dlightbits;

	mplane_t	*plane;
	int		flags;

	int		firstedge;	// look up in model->surfedges[], negative numbers
	int		numedges;	// are backwards edges
	
// surface generation data
	struct surfcache_s	*cachespots[MIPLEVELS];

	short		texturemins[2];
	short		extents[2];

	mtexinfo_t	*texinfo;
	
// lighting info
	byte		styles[MAXLIGHTMAPS];
	byte		*samples;		// [numstyles*surfsize]
} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int		contents;		// 0, to differentiate from leafs
	int		visframe;		// node needs to be traversed if current
	
	short		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];	

	unsigned short	firstsurface;
	unsigned short	numsurfaces;
} mnode_t;



typedef struct mleaf_s
{
// common with node
	int		contents;		// wil be a negative contents number
	int		visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
	efrag_t		*efrags;

	msurface_t	**firstmarksurface;
	int		nummarksurfaces;
	int		key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	dclipnode_t	*clipnodes;
	mplane_t	*planes;
	int		firstclipnode;
	int		lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/


// FIXME: shorten these?
typedef struct mspriteframe_s
{
	short		width;
	short		height;
	//void		*pcachespot;	// remove?
	float		up, down, left, right;
	byte		pixels[4];
} mspriteframe_t;

typedef struct
{
	short		numframes;
	float		*intervals;
	mspriteframe_t	*frames[1];
} mspritegroup_t;

typedef struct
{
	spriteframetype_t	type;
	mspriteframe_t		*frameptr;
} mspriteframedesc_t;

typedef struct
{
	short			type;
	short			maxwidth;
	short			maxheight;
	short			numframes;
	float			beamlength;		// remove?
	//void			*cachespot;		// remove?
	mspriteframedesc_t	frames[1];
} msprite_t;


/*
==============================================================================

ALIAS MODELS

Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

typedef struct
{
	aliasframetype_t	type;
	trivertx_t		bboxmin;
	trivertx_t		bboxmax;
	int			frame;
	char			name[16];
} maliasframedesc_t;

typedef struct
{
	aliasskintype_t		type;
	void			*pcachespot;
	int			skin;
} maliasskindesc_t;

typedef struct
{
	trivertx_t		bboxmin;
	trivertx_t		bboxmax;
	int			frame;
} maliasgroupframedesc_t;

typedef struct
{
	int			numframes;
	int			intervals;
	maliasgroupframedesc_t	frames[1];
} maliasgroup_t;

typedef struct
{
	int			numskins;
	int			intervals;
	maliasskindesc_t	skindescs[1];
} maliasskingroup_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct mtriangle_s {
	int			facesfront;
	unsigned short		vertindex[3];
	unsigned short		stindex[3];
} mtriangle_t;

typedef struct {
	int			model;
	int			stverts;
	int			skindesc;
	int			triangles;
	maliasframedesc_t	frames[1];
} aliashdr_t;

//===================================================================

//
// entity effects
//
#define	EF_BRIGHTFIELD			0x00000001
#define	EF_MUZZLEFLASH			0x00000002
#define	EF_BRIGHTLIGHT			0x00000004
#define	EF_DIMLIGHT			0x00000008
#define	EF_DARKLIGHT			0x00000010
#define	EF_DARKFIELD			0x00000020
#define	EF_LIGHT			0x00000040
#define	EF_NODRAW			0x00000080

//===================================================================

//
// Whole model
//

typedef enum {mod_brush, mod_sprite, mod_alias} modtype_t;

// EF_ changes must also be made in gl_model.h

#define	EF_ROCKET	       1	// leave a trail
#define	EF_GRENADE	       2	// leave a trail
#define	EF_GIB		       4	// leave a trail
#define	EF_ROTATE	       8	// rotate (bonus items)
#define	EF_TRACER	      16	// green split trail
#define	EF_ZOMGIB	      32	// small blood trail
#define	EF_TRACER2	      64	// orange split trail + rotate
#define	EF_TRACER3	     128	// purple trail
#define	EF_FIREBALL	     256	// Yellow transparent trail in all directions
#define	EF_ICE		     512	// Blue-white transparent trail, with gravity
#define	EF_MIP_MAP	    1024	// This model has mip-maps
#define	EF_SPIT		    2048	// Black transparent trail with negative light
#define	EF_TRANSPARENT	    4096	// Transparent sprite
#define	EF_SPELL	    8192	// Vertical spray of particles
#define	EF_HOLEY	   16384	// Solid model with color 0
#define	EF_SPECIAL_TRANS   32768	// Translucency through the particle table
#define	EF_FACE_VIEW	   65536	// Poly Model always faces you
#define	EF_VORP_MISSILE	  131072	// leave a trail at top and bottom of model
#define	EF_SET_STAFF	  262144	// slowly move up and left/right
#define	EF_MAGICMISSILE	  524288	// a trickle of blue/white particles with gravity
#define	EF_BONESHARD	 1048576	// a trickle of brown particles with gravity
#define	EF_SCARAB	 2097152	// white transparent particles with little gravity
#define	EF_ACIDBALL	 4194304	// Green drippy acid shit
#define	EF_BLOODSHOT	 8388608	// Blood rain shot trail

#define	EF_MIP_MAP_FAR	0x1000000	// Set per frame, this model will use the far mip map

typedef struct model_s
{
	char		name[MAX_QPATH];
	qboolean	needload;		// bmodels and sprites don't cache normally

	modtype_t	type;
	int		numframes;
	synctype_t	synctype;
	
	int		flags;

//
// volume occupied by the model graphics
//		
	vec3_t		mins, maxs;
	float		radius;

//
// brush model
//
	int		firstmodelsurface, nummodelsurfaces;

	int		numsubmodels;
	dmodel_t	*submodels;

	int		numplanes;
	mplane_t	*planes;

	int		numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int		numvertexes;
	mvertex_t	*vertexes;

	int		numedges;
	medge_t		*edges;

	int		numnodes;
	mnode_t		*nodes;

	int		numtexinfo;
	mtexinfo_t	*texinfo;

	int		numsurfaces;
	msurface_t	*surfaces;

	int		numsurfedges;
	int		*surfedges;

	int		numclipnodes;
	dclipnode_t	*clipnodes;

	int		nummarksurfaces;
	msurface_t	**marksurfaces;

	hull_t		hulls[MAX_MAP_HULLS];

	int		numtextures;
	texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;

//
// additional model data
//
	cache_user_t	cache;		// only access through Mod_Extradata

} model_t;

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
model_t *Mod_ForName (char *name, qboolean crash);
model_t *Mod_FindName (char *name);
void	*Mod_Extradata (model_t *mod);	// handles caching
void	Mod_TouchModel (char *name);

mleaf_t *Mod_PointInLeaf (float *p, model_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, model_t *model);

#endif	/* __HX2_MODEL_H */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2006/04/06 22:08:22  sezero
 * more tidy-ups (model.h, gl_model.h)
 *
 * Revision 1.5  2005/09/19 20:10:17  sezero
 * startings of model code unification. mostly
 * cosmetic for now, more will follow.
 *
 * Revision 1.4  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.3  2004/12/18 13:59:25  sezero
 * Clean-up and kill warnings 8:
 * Missing prototypes.
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:05:23  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 7     3/12/98 1:12p Jmonroe
 * removed strcmp from render code
 * 
 * 6     3/10/98 6:42p Jmonroe
 * first pass of new model fmt, fully in view render needs work
 * 
 * 5     2/26/98 9:19p Jmonroe
 * shortened memory struct for sprites, added sprite orientation code in
 * gl (need to test)
 * 
 * 4     2/20/98 12:18p Jmonroe
 * changed modelgen.h to genmodel.h
 * 
 * 20    9/18/97 2:34p Rlove
 * 
 * 19    9/17/97 1:27p Rlove
 * 
 * 18    9/17/97 11:11a Rlove
 * 
 * 17    7/15/97 4:09p Rjohnson
 * New particle effect
 * 
 * 16    6/12/97 6:42a Rlove
 * Smoke entities are being deleted (before they weren't)
 * 
 * 15    5/28/97 3:54p Rjohnson
 * Effect to make a model always face you
 * 
 * 14    5/22/97 5:56p Rjohnson
 * New translucency effect
 * 
 * 13    4/03/97 10:40a Rjohnson
 * Added a translucent flag for surfaces
 * 
 * 12    3/28/97 5:28p Rjohnson
 * Updates to the transparency for the models
 * 
 * 11    3/07/97 1:50p Rjohnson
 * Id Updates
 * 
 * 10    3/07/97 12:06p Rjohnson
 * Added new spell particle effect
 * 
 * 9     2/20/97 12:13p Rjohnson
 * Code fixes for id update
 * 
 * 8     2/11/97 1:15p Rjohnson
 * Added transparent sprites
 * 
 * 7     1/02/97 11:16a Rjohnson
 * Christmas work - added adaptive time, game delays, negative light,
 * particle effects, etc
 * 
 * 6     12/12/96 12:09p Rjohnson
 * Removed model distance define
 * 
 * 5     12/12/96 9:42a Ggribb
 * added mip mapped monsters
 * 
 * 4     12/11/96 3:54p Rjohnson
 * Negated mean of mip map flag
 * 
 * 3     12/11/96 10:44a Rjohnson
 * Added the ice and no mip-mapping defines
 * 
 * 2     12/06/96 2:00p Rjohnson
 * New particle type for the fireball
 */
