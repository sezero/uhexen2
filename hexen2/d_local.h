/*
	d_local.h
	private rasterization driver defs

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/d_local.h,v 1.14 2007-08-09 06:12:45 sezero Exp $
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


extern void (*d_drawspans) (espan_t *pspan);

extern void D_DrawSpans8 (espan_t *pspans);
extern void D_DrawSpans8T(espan_t *pspans);
extern void D_DrawZSpans (espan_t *pspans);
extern void D_DrawSingleZSpans (espan_t *pspans);
extern void Turbulent8 (surf_t *s);

extern void D_DrawSkyScans8 (espan_t *pspan);
extern void D_DrawSkyScans16 (espan_t *pspan);

#if id386
extern void D_DrawSpans16 (espan_t *pspans);
extern void D_DrawSpans16T (espan_t *pspans);
extern void D_SpriteDrawSpans (sspan_t *pspan);
extern void D_SpriteDrawSpansT (sspan_t *pspan);
extern void D_SpriteDrawSpansT2 (sspan_t *pspan);
extern void D_DrawTurbulent8Span (void);
extern void D_DrawTurbulent8TSpan (void);
extern void D_DrawTurbulent8TQuickSpan (void);
#endif

extern void D_Patch (void);

#if id386
extern void D_Draw16StartT (void);
extern void D_Draw16EndT (void);
extern void D_DrawTurbulent8TSpanEnd (void);
extern void D_PolysetAff8Start (void);
extern void D_PolysetAff8StartT (void);
extern void D_PolysetAff8StartT2 (void);
extern void D_PolysetAff8StartT3 (void);
extern void D_PolysetAff8StartT5 (void);
extern void D_PolysetAff8End (void);
extern void D_PolysetAff8EndT (void);
extern void D_PolysetAff8EndT2 (void);
extern void D_PolysetAff8EndT3 (void);
extern void D_PolysetAff8EndT5 (void);
extern void D_SpriteSpansStartT (void);
extern void D_SpriteSpansEndT (void);
extern void D_SpriteSpansStartT2 (void);
extern void D_SpriteSpansEndT2 (void);

extern void D_Aff8Patch (void *pcolormap);
extern void D_Aff8PatchT (void *pcolormap);
extern void D_Aff8PatchT2 (void *pcolormap);
extern void D_Aff8PatchT3 (void *pcolormap);
extern void D_Aff8PatchT5 (void *pcolormap);

extern void R_TranPatch1 (void);
extern void R_TranPatch2 (void);
extern void R_TranPatch3 (void);
extern void R_TranPatch4 (void);
extern void R_TranPatch5 (void);
extern void R_TranPatch6 (void);
extern void R_TranPatch7 (void);
#endif

extern surfcache_t *D_CacheSurface (msurface_t *surface, int miplevel);


extern short	*d_pzbuffer;
extern unsigned int d_zrowbytes, d_zwidth;

extern int	*d_pscantable;
extern int	d_scantable[MAXHEIGHT];

extern int	d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

extern int	d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;

extern pixel_t	*d_viewbuffer;

extern short	*zspantable[MAXHEIGHT];

extern int	d_aflatcolor;
extern int	d_minmip;
extern float	d_scalemip[3];

#define	SCAN_SIZE		2048

extern byte	scanList[SCAN_SIZE];

#endif	/* __D_LOCAL_H */

