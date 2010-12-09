/*
	r_shared.h:
	general refresh-related stuff shared between the refresh
	and the driver

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/r_shared.h,v 1.10 2008-03-21 14:45:16 sezero Exp $
*/

#ifndef __R_SHARED_H
#define __R_SHARED_H

#ifndef GLQUAKE

// FIXME: clean up and move into d_iface.h

#define	MAXVERTS	16		// max points in a surface polygon

#define MAXWORKINGVERTS	(MAXVERTS+4)	// max points in an intermediate
					// polygon (while processing)

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define	MAXHEIGHT	1024
#define	MAXWIDTH	1280

#define	INFINITE_DISTANCE	0x10000
				// distance that's always guaranteed to be
				// farther away than anything in the scene

//=============================================================================


extern void	R_DrawLine (polyvert_t *polyvert0, polyvert_t *polyvert1);


extern	int	cachewidth;
extern	pixel_t	*cacheblock;
extern	int	screenwidth;

extern	float	pixelAspect;

extern	int	r_drawnpolycount;

extern	cvar_t	r_clearcolor;

extern	int	sintable[1280];
extern	int	intsintable[1280];

extern	vec3_t	vup, base_vup;
extern	vec3_t	vpn, base_vpn;
extern	vec3_t	vright, base_vright;
extern	entity_t	*currententity;


#define NUMSTACKEDGES		2400
#define	MINEDGES		NUMSTACKEDGES

#define NUMSTACKSURFACES	1000
#define MINSURFACES		NUMSTACKSURFACES

#define	MAXSPANS		3000


// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct espan_s
{
	int			u, v, count;
	struct espan_s	*pnext;
} espan_t;


// FIXME: compress, make a union if that will help
// insubmodel is only 1, flags is fewer than 32, spanstate could be a byte
typedef struct surf_s
{
	struct surf_s	*next;		// active surface stack in r_edge.c
	struct surf_s	*prev;		// used in r_edge.c for active surf stack
	struct espan_s	*spans;		// pointer to linked list of spans to draw
	int		key;		// sorting key (BSP order)
	int		last_u;		// set during tracing
	int		spanstate;	// 0 = not in span
					// 1 = in span
					// -1 = in inverted span (end before
					//  start)

	int		flags;		// currentface flags
	void		*data;		// associated data like msurface_t
	entity_t	*entity;
	float		nearzi;		// nearest 1/z on surface, for mipmapping
	qboolean	insubmodel;
	float		d_ziorigin, d_zistepu, d_zistepv;

	int		pad[2];		// to 64 bytes
} surf_t;

extern	surf_t	*surfaces, *surface_p, *surf_max;

// surfaces are generated in back to front order by the bsp, so if a surf
// pointer is greater than another one, it should be drawn in front
// surfaces[1] is the background, and is used as the active surface stack.
// surfaces[0] is a dummy, because index 0 is used to indicate no surface
//  attached to an edge_t

//===================================================================

extern	vec3_t	sxformaxis[4];	// s axis transformed into viewspace
extern	vec3_t	txformaxis[4];	// t axis transformed into viewspac

extern	vec3_t	modelorg, base_modelorg;

extern	float	xcenter, ycenter;
extern	float	xscale, yscale;
extern	float	xscaleinv, yscaleinv;
extern	float	xscaleshrink, yscaleshrink;

extern	int	d_lightstylevalue[256];	// 8.8 frac of base light value

extern	int	ubasestep, errorterm, erroradjustup, erroradjustdown;

extern	int	r_skymade;

__ASM_FUNCS_BEGIN
void TransformVector (vec3_t in, vec3_t out);
__ASM_FUNCS_END

void R_MakeSky (void);


// flags in finalvert_t.flags
#define ALIAS_LEFT_CLIP			0x0001
#define ALIAS_TOP_CLIP			0x0002
#define ALIAS_RIGHT_CLIP		0x0004
#define ALIAS_BOTTOM_CLIP		0x0008
#define ALIAS_Z_CLIP			0x0010
// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define ALIAS_ONSEAM			0x0020	// also defined in genmodel.h
						// must be kept in sync
#define ALIAS_XY_CLIP_MASK		0x000F

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct edge_s
{
	fixed16_t	u;
	fixed16_t	u_step;
	struct edge_s	*prev, *next;
	unsigned short	surfs[2];
	struct edge_s	*nextremove;
	float		nearzi;
	medge_t		*owner;
} edge_t;

extern	byte	*mainTransTable;
extern	byte	*transTable;	/* the particle table */
extern	byte	*playerTranslation;
extern	const int	color_offsets[MAX_PLAYER_CLASS];

#endif	/*  !GLQUAKE	*/

#endif	/* __R_SHARED_H	*/

