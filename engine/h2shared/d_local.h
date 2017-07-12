/*
 * d_local.h -- private rasterization driver defs
 * $Id: d_local.h,v 1.18 2008-04-03 07:11:49 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
	unsigned int		width;
	unsigned int		height;		// DEBUG only needed for debug
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

// TODO: put in span spilling to shrink list size
// !!! if this is changed, it must be changed in d_polysa.s too !!!
#define	DPS_MAXSPANS	(MAXHEIGHT + 1)
// 1 extra for spanpackage that marks end

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	void		*pdest;
	short		*pz;
	int		count;
	byte		*ptex;
	int		sfrac, tfrac, light, zi;
} spanpackage_t;

typedef struct {
	int		isflattop;
	int		numleftedges;
	int		*pleftedgevert0;
	int		*pleftedgevert1;
	int		*pleftedgevert2;
	int		numrightedges;
	int		*prightedgevert0;
	int		*prightedgevert1;
	int		*prightedgevert2;
} edgetable_t;

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

__ASM_FUNCS_BEGIN

void D_DrawSpans8 (espan_t *pspans);
void D_DrawSpans8T(espan_t *pspans);
void D_DrawZSpans (espan_t *pspans);
void D_DrawSingleZSpans (espan_t *pspans);

#if id386
void D_DrawSpans16 (espan_t *pspans);
void D_DrawSpans16T (espan_t *pspans);
void D_SpriteDrawSpans (sspan_t *pspan);
void D_SpriteDrawSpansT (sspan_t *pspan);
void D_SpriteDrawSpansT2 (sspan_t *pspan);
void D_DrawTurbulent8Span (void);
void D_DrawTurbulent8TSpan (void);
void D_DrawTurbulent8TQuickSpan (void);

void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage);
void D_PolysetDrawSpans8T (spanpackage_t *pspanpackage);
void D_PolysetDrawSpans8T2 (spanpackage_t *pspanpackage);
void D_PolysetDrawSpans8T3 (spanpackage_t *pspanpackage);
void D_PolysetDrawSpans8T5 (spanpackage_t *pspanpackage);

void D_Draw16StartT (void);
void D_Draw16EndT (void);
void D_DrawTurbulent8TSpanEnd (void);
void D_PolysetAff8Start (void);
void D_PolysetAff8StartT (void);
void D_PolysetAff8StartT2 (void);
void D_PolysetAff8StartT3 (void);
void D_PolysetAff8StartT5 (void);
void D_PolysetAff8End (void);
void D_PolysetAff8EndT (void);
void D_PolysetAff8EndT2 (void);
void D_PolysetAff8EndT3 (void);
void D_PolysetAff8EndT5 (void);
void D_SpriteSpansStartT (void);
void D_SpriteSpansEndT (void);
void D_SpriteSpansStartT2 (void);
void D_SpriteSpansEndT2 (void);

void D_Aff8Patch (void *pcolormap);
void D_Aff8PatchT (void *pcolormap);
void D_Aff8PatchT2 (void *pcolormap);
void D_Aff8PatchT3 (void *pcolormap);
void D_Aff8PatchT5 (void *pcolormap);

void R_TranPatch1 (void);
void R_TranPatch2 (void);
void R_TranPatch3 (void);
void R_TranPatch4 (void);
void R_TranPatch5 (void);
void R_TranPatch6 (void);
void R_TranPatch7 (void);
#endif /* id386 */

/* C funcs called from asm code: */
void D_PolysetSetEdgeTable (void);
void D_RasterizeAliasPolySmooth (void);

__ASM_FUNCS_END


void Turbulent8 (surf_t *s);

void D_DrawSkyScans8 (espan_t *pspan);
void D_DrawSkyScans16 (espan_t *pspan);

surfcache_t *D_CacheSurface (msurface_t *surface, int miplevel);

void D_Patch (void);


extern short	*d_pzbuffer;
extern int	d_zrowbytes, d_zwidth;

extern int	*d_pscantable;
extern int	d_scantable[MAXHEIGHT];

extern int	d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

extern int	d_y_aspect_shift, d_y_aspect_rshift, d_pix_min, d_pix_max, d_pix_shift;

extern pixel_t	*d_viewbuffer;

extern short	*zspantable[MAXHEIGHT];

extern int	d_aflatcolor;
extern int	d_minmip;
extern float	d_scalemip[3];

#define	SCAN_SIZE		2048

extern byte	scanList[SCAN_SIZE];

#endif	/* __D_LOCAL_H */

