/*
	d_local.h
	private rasterization driver defs

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/d_local.h,v 1.10 2006-09-24 17:28:42 sezero Exp $
*/

#ifndef __D_LOCAL_H
#define __D_LOCAL_H

#include "r_shared.h"

//
// TODO: fine-tune this; it's based on providing some overage even if there
// is a 2k-wide scan, with subdivision every 8, for 256 spans of 12 bytes each
//
#define SCANBUFFERPAD		0x1000

#define R_SKY_SMASK	0x007F0000
#define R_SKY_TMASK	0x007F0000

#define DS_SPAN_LIST_END	-128

//#define SURFCACHE_SIZE_AT_320X200	600*1024
#define SURFCACHE_SIZE_AT_320X200	768*1024

typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int			lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int			dlight;
	int			size;		// including header
	unsigned		width;
	unsigned		height;		// DEBUG only needed for debug
	float			mipscale;
	struct texture_s	*texture;	// checked for animating textures
	int			drawflags;
	int			abslight;
	byte			data[4];	// width*height elements
} surfcache_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct sspan_s
{
	int			u, v, count;
} sspan_t;

extern float	scale_for_mip;

extern qboolean		d_roverwrapped;
extern surfcache_t	*sc_rover;
extern surfcache_t	*d_initial_rover;

extern float	d_sdivzstepu, d_tdivzstepu, d_zistepu;
extern float	d_sdivzstepv, d_tdivzstepv, d_zistepv;
extern float	d_sdivzorigin, d_tdivzorigin, d_ziorigin;

extern fixed16_t	sadjust, tadjust;
extern fixed16_t	bbextents, bbextentt;


void D_DrawSpans8 (espan_t *pspans);
void D_DrawZSpans (espan_t *pspans);
void Turbulent8 (surf_t *s);

void D_DrawSkyScans8 (espan_t *pspan);
void D_DrawSkyScans16 (espan_t *pspan);

void D_Patch (void);

void R_ShowSubDiv (void);
surfcache_t	*D_CacheSurface (msurface_t *surface, int miplevel);

#if id386
extern void D_PolysetAff8Start (void);
extern void D_PolysetAff8End (void);
extern void D_PolysetAff8StartT (void);
extern void D_PolysetAff8EndT (void);
extern void D_PolysetAff8StartT2 (void);
extern void D_PolysetAff8EndT2 (void);
extern void D_PolysetAff8StartT3 (void);
extern void D_PolysetAff8EndT3 (void);
extern void D_PolysetAff8StartT5 (void);
extern void D_PolysetAff8EndT5 (void);
extern void D_Draw16StartT (void);
extern void D_Draw16EndT (void);
extern void D_SpriteDrawSpans (sspan_t *pspan);
extern void D_SpriteDrawSpansT (sspan_t *pspan);
extern void D_SpriteDrawSpansT2 (sspan_t *pspan);
extern void D_SpriteSpansStartT (void);
extern void D_SpriteSpansEndT (void);
extern void D_SpriteSpansStartT2 (void);
extern void D_SpriteSpansEndT2 (void);
extern void D_DrawSpans16 (espan_t *pspans);
extern void D_DrawSpans16T (espan_t *pspans);
extern void D_DrawSingleZSpans (espan_t *pspans);
extern void D_DrawTurbulent8Span (void);
extern void D_DrawTurbulent8TSpan (void);
extern void D_DrawTurbulent8TQuickSpan (void);
extern void D_DrawTurbulent8TSpanEnd (void);
extern void D_Aff8Patch (void *pcolormap);
extern void D_Aff8PatchT(void *pcolormap);
extern void D_Aff8PatchT2(void *pcolormap);
extern void D_Aff8PatchT3(void *pcolormap);
extern void D_Aff8PatchT5(void *pcolormap);
extern void R_TranPatch1(void);
extern void R_TranPatch2(void);
extern void R_TranPatch3(void);
extern void R_TranPatch4(void);
extern void R_TranPatch5(void);
extern void R_TranPatch6(void);
extern void R_TranPatch7(void);
#endif

extern short *d_pzbuffer;
extern unsigned int d_zrowbytes, d_zwidth;

extern int	*d_pscantable;
extern int	d_scantable[MAXHEIGHT];

extern int	d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

extern int	d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;

extern pixel_t	*d_viewbuffer;

extern short	*zspantable[MAXHEIGHT];

extern int		d_aflatcolor;
extern int		d_minmip;
extern float	d_scalemip[3];

extern void (*d_drawspans) (espan_t *pspan);

#define SCAN_SIZE 2048

extern byte			scanList[SCAN_SIZE];

#endif	/* __D_LOCAL_H */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2006/07/18 08:35:39  sezero
 * made d_aflatcolor global through d_local.h
 *
 * Revision 1.8  2006/07/15 22:15:17  sezero
 * removed declaration of non-existant prealspandrawer function pointer
 *
 * Revision 1.7  2006/03/28 19:53:23  sezero
 * continue making static functions and vars static. whitespace and coding
 * style cleanup. part 47: software renderer: r_aclip.c, r_alias.c, r_bsp.c,
 * r_draw.c, r_edge.c.  also removed the #warning directives for the non-x86
 * non-compilability.  added a few comments about the non-Intel issue.
 *
 * Revision 1.6  2006/03/24 15:05:39  sezero
 * killed the archive, server and info members of the cvar structure.
 * the new flags member is now employed for all those purposes. also
 * made all non-globally used cvars static.
 *
 * Revision 1.5  2005/10/25 20:08:41  sezero
 * coding style and whitespace cleanup.
 *
 * Revision 1.4  2005/10/25 20:04:17  sezero
 * static functions part-1: started making local functions static,
 * killing nested externs, const vars clean-up.
 *
 * Revision 1.3  2004/12/19 10:35:30  sezero
 * - Add "implicitly declared" functions as externs: Software rendering files.
 * - Add comments on the asm/non-Intel status of the code
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:01:47  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 13    8/30/97 6:17p Rjohnson
 * Reduced texture cache
 * 
 * 12    6/12/97 11:08a Rjohnson
 * Only water is translucent now
 * 
 * 11    6/03/97 5:50p Rjohnson
 * Added translucent water
 * 
 * 10    5/22/97 5:56p Rjohnson
 * New translucency effect
 * 
 * 9     5/18/97 1:37p Rjohnson
 * Added new mixed mode sprite assembly
 * 
 * 8     5/18/97 12:50p Rjohnson
 * Sprite translucency is now in assembly
 * 
 * 7     5/15/97 4:42p Rjohnson
 * Minor assembly optimization
 * 
 * 6     4/24/97 11:21p Rjohnson
 * You can now set the overall light level of a bmodel
 * 
 * 5     4/21/97 11:35a Rjohnson
 * Translucency update (drawn in right order) and translucent bmodels
 * 
 * 4     3/28/97 5:28p Rjohnson
 * Updates to the transparency for the models
 * 
 * 3     3/28/97 10:08a Rjohnson
 * Added transparent models
 * 
 * 2     1/07/97 10:03a Rjohnson
 * Increased surface cache size
 */
