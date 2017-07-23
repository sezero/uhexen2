/*
 * d_polyset.c - routines for drawing sets of polygons sharing the same
 * texture (used for Alias models.)
 *
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * C versions of several asm functions:  Juraj Styk <jurajstyk@host.sk>
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

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

int		r_p0[6], r_p1[6], r_p2[6];

byte		*d_pcolormap;

int		d_aflatcolor;
int		d_xdenom;

static edgetable_t	*pedgetable;

static edgetable_t	edgetables[12] =
{
	{ 0, 1, r_p0, r_p2, NULL, 2, r_p0, r_p1, r_p2 },
	{ 0, 2, r_p1, r_p0, r_p2, 1, r_p1, r_p2, NULL },
	{ 1, 1, r_p0, r_p2, NULL, 1, r_p1, r_p2, NULL },
	{ 0, 1, r_p1, r_p0, NULL, 2, r_p1, r_p2, r_p0 },
	{ 0, 2, r_p0, r_p2, r_p1, 1, r_p0, r_p1, NULL },
	{ 0, 1, r_p2, r_p1, NULL, 1, r_p2, r_p0, NULL },
	{ 0, 1, r_p2, r_p1, NULL, 2, r_p2, r_p0, r_p1 },
	{ 0, 2, r_p2, r_p1, r_p0, 1, r_p2, r_p0, NULL },
	{ 0, 1, r_p1, r_p0, NULL, 1, r_p1, r_p2, NULL },
	{ 1, 1, r_p2, r_p1, NULL, 1, r_p0, r_p1, NULL },
	{ 1, 1, r_p1, r_p0, NULL, 1, r_p2, r_p0, NULL },
	{ 0, 1, r_p0, r_p2, NULL, 1, r_p0, r_p1, NULL }
};

// FIXME: some of these can become statics
int		a_sstepxfrac, a_tstepxfrac, r_lstepx, a_ststepxwhole;
int		r_sstepx, r_tstepx, r_lstepy, r_sstepy, r_tstepy;
int		r_zistepx, r_zistepy;
int		d_aspancount, d_countextrastep;

spanpackage_t	*a_spans;
spanpackage_t	*d_pedgespanpackage;
static int	ystart;
byte		*d_pdest, *d_ptex;
short		*d_pz;
int		d_sfrac, d_tfrac, d_light, d_zi;
int		d_ptexextrastep, d_sfracextrastep;
int		d_tfracextrastep, d_lightextrastep, d_pdestextrastep;
int		d_lightbasestep, d_pdestbasestep, d_ptexbasestep;
int		d_sfracbasestep, d_tfracbasestep;
int		d_ziextrastep, d_zibasestep;
int		d_pzextrastep, d_pzbasestep;

typedef struct {
	int		quotient;
	int		remainder;
} adivtab_t;

static adivtab_t	adivtab[32*32] =
{
#include "adivtab.h"
};

byte	*skintable[MAX_SKIN_HEIGHT];
int		skinwidth;
byte	*skinstart;

#if !id386
static spanpackage_t	spans[DPS_MAXSPANS + 1 + ((CACHE_SIZE - 1) / sizeof(spanpackage_t)) + 1];
						/* one extra because of cache line pretouching */
#ifndef id68k
static void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage);
#endif
static void D_PolysetDrawSpans8T (spanpackage_t *pspanpackage);
static void D_PolysetDrawSpans8T2 (spanpackage_t *pspanpackage);
static void D_PolysetDrawSpans8T3 (spanpackage_t *pspanpackage);
static void D_PolysetDrawSpans8T5 (spanpackage_t *pspanpackage);
#endif


#if	!id386

/*
================
D_PolysetDrawFinalVerts
================
*/
#ifndef id68k
static inline void do_PolysetDrawFinalVerts (finalvert_t *pv)
{
	int		z;
	short		*zbuf;

	// valid triangle coordinates for filling can include the bottom and
	// right clip edges, due to the fill rule; these shouldn't be drawn
	if (pv->v[0] < r_refdef.vrectright && pv->v[1] < r_refdef.vrectbottom)
	{
		z = pv->v[5]>>16;
		zbuf = zspantable[pv->v[1]] + pv->v[0];
		if (z >= *zbuf)
		{
			unsigned int	pix;

			*zbuf = z;
			pix = skintable[pv->v[3]>>16][pv->v[2]>>16];
			pix = ((byte *)acolormap)[pix + (pv->v[4] & 0xFF00)];
			d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]] = pix;
		}
	}
}
#endif

static inline void do_PolysetDrawFinalVertsT (finalvert_t *pv)
{
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	// valid triangle coordinates for filling can include the bottom and
	// right clip edges, due to the fill rule; these shouldn't be drawn
	if (pv->v[0] < r_refdef.vrectright && pv->v[1] < r_refdef.vrectbottom)
	{
		z = pv->v[5]>>16;
		zbuf = zspantable[pv->v[1]] + pv->v[0];
		if (z >= *zbuf)
		{
			color_map_idx = skintable[pv->v[3]>>16][pv->v[2]>>16];

			if (color_map_idx != 0)
			{
				unsigned int	pix, pix2;

				*zbuf = z;
				pix = ((byte *)acolormap)[color_map_idx + (pv->v[4] & 0xFF00)];
				pix2 = d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]];
				pix = mainTransTable[(pix<<8) + pix2];
				d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]] = pix;
			}
		}
	}
}

static inline void do_PolysetDrawFinalVertsT2 (finalvert_t *pv)
{
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	// valid triangle coordinates for filling can include the bottom and
	// right clip edges, due to the fill rule; these shouldn't be drawn
	if (pv->v[0] < r_refdef.vrectright && pv->v[1] < r_refdef.vrectbottom)
	{
		z = pv->v[5]>>16;
		zbuf = zspantable[pv->v[1]] + pv->v[0];
		if (z >= *zbuf)
		{
			color_map_idx = skintable[pv->v[3]>>16][pv->v[2]>>16];

			if (color_map_idx != 0)
			{
				unsigned int	pix, pix2;

				*zbuf = z;
				pix = ((byte *)acolormap)[color_map_idx + (pv->v[4] & 0xFF00)];
				if (color_map_idx % 2 == 0)
				{
					d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]] = pix;
				}
				else
				{
					pix2 = d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]];
					pix = mainTransTable[(pix<<8) + pix2];
					d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]] = pix;
				}
			}
		}
	}
}

static inline void do_PolysetDrawFinalVertsT3 (finalvert_t *pv)
{
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	// valid triangle coordinates for filling can include the bottom and
	// right clip edges, due to the fill rule; these shouldn't be drawn
	if (pv->v[0] < r_refdef.vrectright && pv->v[1] < r_refdef.vrectbottom)
	{
		z = pv->v[5]>>16;
		zbuf = zspantable[pv->v[1]] + pv->v[0];
		if (z >= *zbuf)
		{
			color_map_idx = skintable[pv->v[3]>>16][pv->v[2]>>16];

			if (color_map_idx != 0)
			{
				unsigned int	pix;

				*zbuf = z;
				pix = ((byte *)acolormap)[color_map_idx + (pv->v[4] & 0xFF00)];
				d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]] = pix;
			}
		}
	}
}

static inline void do_PolysetDrawFinalVertsT5 (finalvert_t *pv)
{
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	// valid triangle coordinates for filling can include the bottom and
	// right clip edges, due to the fill rule; these shouldn't be drawn
	if (pv->v[0] < r_refdef.vrectright && pv->v[1] < r_refdef.vrectbottom)
	{
		z = pv->v[5]>>16;
		zbuf = zspantable[pv->v[1]] + pv->v[0];
		if (z >= *zbuf)
		{
			color_map_idx = skintable[pv->v[3]>>16][pv->v[2]>>16];

			if (color_map_idx != 0)
			{
				unsigned int	pix, pix2;

				*zbuf = z;
				pix = color_map_idx;
				pix2 = d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]];
				pix = transTable[(pix<<8) + pix2];
				d_viewbuffer[d_scantable[pv->v[1]] + pv->v[0]] = pix;
			}
		}
	}
}

#ifndef id68k
void D_PolysetDrawFinalVerts (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
{
	do_PolysetDrawFinalVerts (pv1);
	do_PolysetDrawFinalVerts (pv2);
	do_PolysetDrawFinalVerts (pv3);
}
#endif

void D_PolysetDrawFinalVertsT (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
{
	do_PolysetDrawFinalVertsT (pv1);
	do_PolysetDrawFinalVertsT (pv2);
	do_PolysetDrawFinalVertsT (pv3);
}

void D_PolysetDrawFinalVertsT2 (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
{
	do_PolysetDrawFinalVertsT2 (pv1);
	do_PolysetDrawFinalVertsT2 (pv2);
	do_PolysetDrawFinalVertsT2 (pv3);
}

void D_PolysetDrawFinalVertsT3 (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
{
	do_PolysetDrawFinalVertsT3 (pv1);
	do_PolysetDrawFinalVertsT3 (pv2);
	do_PolysetDrawFinalVertsT3 (pv3);
}

void D_PolysetDrawFinalVertsT5 (finalvert_t *pv1, finalvert_t *pv2, finalvert_t *pv3)
{
	do_PolysetDrawFinalVertsT5 (pv1);
	do_PolysetDrawFinalVertsT5 (pv2);
	do_PolysetDrawFinalVertsT5 (pv3);
}


/*
================
D_PolysetRecursiveTriangle
================
*/
#ifndef id68k
static void D_PolysetRecursiveTriangle (int *lp1, int *lp2, int *lp3)
{
	int		*temp;
	int		d;
	int		new_p[6];
	int		z;
	short		*zbuf;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	d = lp3[0] - lp2[0];
	if (d < -1 || d > 1)
		goto split2;
	d = lp3[1] - lp2[1];
	if (d < -1 || d > 1)
		goto split2;

	d = lp1[0] - lp3[0];
	if (d < -1 || d > 1)
		goto split3;
	d = lp1[1] - lp3[1];
	if (d < -1 || d > 1)
	{
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return;			// entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;

// draw the point if splitting a leading edge
	if (lp2[1] > lp1[1])
		goto nodraw;
	if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
		goto nodraw;

	z = new_p[5]>>16;
	zbuf = zspantable[new_p[1]] + new_p[0];
	if (z >= *zbuf)
	{
		unsigned int	pix;

		*zbuf = z;
		pix = d_pcolormap[skintable[new_p[3]>>16][new_p[2]>>16]];
		d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
	}

nodraw:
// recursively continue
	D_PolysetRecursiveTriangle (lp3, lp1, new_p);
	D_PolysetRecursiveTriangle (lp3, new_p, lp2);
}
#endif

static void D_PolysetRecursiveTriangleT (int *lp1, int *lp2, int *lp3)
{
	int		*temp;
	int		d;
	int		new_p[6];
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	d = lp3[0] - lp2[0];
	if (d < -1 || d > 1)
		goto split2;
	d = lp3[1] - lp2[1];
	if (d < -1 || d > 1)
		goto split2;

	d = lp1[0] - lp3[0];
	if (d < -1 || d > 1)
		goto split3;
	d = lp1[1] - lp3[1];
	if (d < -1 || d > 1)
	{
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return;			// entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;

// draw the point if splitting a leading edge
	if (lp2[1] > lp1[1])
		goto nodraw;
	if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
		goto nodraw;

	z = new_p[5]>>16;
	zbuf = zspantable[new_p[1]] + new_p[0];
	if (z >= *zbuf)
	{
		color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];

		if (color_map_idx != 0)
		{
			unsigned int	pix, pix2;

			*zbuf = z;
			pix = d_pcolormap[color_map_idx];
			pix2 = d_viewbuffer[d_scantable[new_p[1]] + new_p[0]];
			pix = mainTransTable[(pix<<8) + pix2];
			d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
		}
	}

nodraw:
// recursively continue
	D_PolysetRecursiveTriangleT (lp3, lp1, new_p);
	D_PolysetRecursiveTriangleT (lp3, new_p, lp2);
}

static void D_PolysetRecursiveTriangleT2 (int *lp1, int *lp2, int *lp3)
{
	int		*temp;
	int		d;
	int		new_p[6];
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	d = lp3[0] - lp2[0];
	if (d < -1 || d > 1)
		goto split2;
	d = lp3[1] - lp2[1];
	if (d < -1 || d > 1)
		goto split2;

	d = lp1[0] - lp3[0];
	if (d < -1 || d > 1)
		goto split3;
	d = lp1[1] - lp3[1];
	if (d < -1 || d > 1)
	{
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return;			// entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;

// draw the point if splitting a leading edge
	if (lp2[1] > lp1[1])
		goto nodraw;
	if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
		goto nodraw;

	z = new_p[5]>>16;
	zbuf = zspantable[new_p[1]] + new_p[0];
	if (z >= *zbuf)
	{
		color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];

		if (color_map_idx != 0)
		{
			unsigned int	pix, pix2;

			*zbuf = z;
			pix = d_pcolormap[color_map_idx];

			if (color_map_idx % 2 == 0)
			{
				d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
			}
			else
			{
				pix2 = d_viewbuffer[d_scantable[new_p[1]] + new_p[0]];
				pix = mainTransTable[(pix<<8) + pix2];
				d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
			}
		}
	}

nodraw:
// recursively continue
	D_PolysetRecursiveTriangleT2 (lp3, lp1, new_p);
	D_PolysetRecursiveTriangleT2 (lp3, new_p, lp2);
}

static void D_PolysetRecursiveTriangleT3 (int *lp1, int *lp2, int *lp3)
{
	int		*temp;
	int		d;
	int		new_p[6];
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	d = lp3[0] - lp2[0];
	if (d < -1 || d > 1)
		goto split2;
	d = lp3[1] - lp2[1];
	if (d < -1 || d > 1)
		goto split2;

	d = lp1[0] - lp3[0];
	if (d < -1 || d > 1)
		goto split3;
	d = lp1[1] - lp3[1];
	if (d < -1 || d > 1)
	{
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return;			// entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;

// draw the point if splitting a leading edge
	if (lp2[1] > lp1[1])
		goto nodraw;
	if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
		goto nodraw;

	z = new_p[5]>>16;
	zbuf = zspantable[new_p[1]] + new_p[0];
	if (z >= *zbuf)
	{
		color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];

		if (color_map_idx != 0)
		{
			unsigned int	pix;

			*zbuf = z;
			pix = d_pcolormap[color_map_idx];
			d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
		}
	}

nodraw:
// recursively continue
	D_PolysetRecursiveTriangleT3 (lp3, lp1, new_p);
	D_PolysetRecursiveTriangleT3 (lp3, new_p, lp2);
}

static void D_PolysetRecursiveTriangleT5 (int *lp1, int *lp2, int *lp3)
{
	int		*temp;
	int		d;
	int		new_p[6];
	int		z;
	short		*zbuf;
	unsigned int	color_map_idx;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	d = lp3[0] - lp2[0];
	if (d < -1 || d > 1)
		goto split2;
	d = lp3[1] - lp2[1];
	if (d < -1 || d > 1)
		goto split2;

	d = lp1[0] - lp3[0];
	if (d < -1 || d > 1)
		goto split3;
	d = lp1[1] - lp3[1];
	if (d < -1 || d > 1)
	{
split3:
		temp = lp1;
		lp1 = lp3;
		lp3 = lp2;
		lp2 = temp;

		goto split;
	}

	return;			// entire tri is filled

split2:
	temp = lp1;
	lp1 = lp2;
	lp2 = lp3;
	lp3 = temp;

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;

// draw the point if splitting a leading edge
	if (lp2[1] > lp1[1])
		goto nodraw;
	if ((lp2[1] == lp1[1]) && (lp2[0] < lp1[0]))
		goto nodraw;

	z = new_p[5]>>16;
	zbuf = zspantable[new_p[1]] + new_p[0];
	if (z >= *zbuf)
	{
		color_map_idx = skintable[new_p[3]>>16][new_p[2]>>16];

		if (color_map_idx != 0)
		{
			unsigned int	pix, pix2;

			*zbuf = z;
			pix = color_map_idx;
			pix2 = d_viewbuffer[d_scantable[new_p[1]] + new_p[0]];
			pix = transTable[(pix<<8) + pix2];
			d_viewbuffer[d_scantable[new_p[1]] + new_p[0]] = pix;
		}
	}

nodraw:
// recursively continue
	D_PolysetRecursiveTriangleT5 (lp3, lp1, new_p);
	D_PolysetRecursiveTriangleT5 (lp3, new_p, lp2);
}

/*
================
D_DrawSubdiv
================
*/
static void D_DrawSubdiv (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int			i, lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++)
	{
		index0 = pfv + ptri[i].vertindex[0];
		index1 = pfv + ptri[i].vertindex[1];
		index2 = pfv + ptri[i].vertindex[2];

		if (((index0->v[1]-index1->v[1]) * (index0->v[0]-index2->v[0]) -
			 (index0->v[0]-index1->v[0]) * (index0->v[1]-index2->v[1])) >= 0)
		{
			continue;
		}

		d_pcolormap = &((byte *)acolormap)[index0->v[4] & 0xFF00];

		if (ptri[i].facesfront)
		{
			D_PolysetRecursiveTriangle(index0->v, index1->v, index2->v);
		}
		else
		{
			int		s0, s1, s2;

			s0 = index0->v[2];
			s1 = index1->v[2];
			s2 = index2->v[2];

			if (index0->flags & ALIAS_ONSEAM)
				index0->v[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				index1->v[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				index2->v[2] += r_affinetridesc.seamfixupX16;

			D_PolysetRecursiveTriangle(index0->v, index1->v, index2->v);

			index0->v[2] = s0;
			index1->v[2] = s1;
			index2->v[2] = s2;
		}
	}
}

static void D_DrawSubdivT (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int			i, lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++)
	{
		index0 = pfv + ptri[i].vertindex[0];
		index1 = pfv + ptri[i].vertindex[1];
		index2 = pfv + ptri[i].vertindex[2];

		if (((index0->v[1]-index1->v[1]) * (index0->v[0]-index2->v[0]) -
			 (index0->v[0]-index1->v[0]) * (index0->v[1]-index2->v[1])) >= 0)
		{
			continue;
		}

		d_pcolormap = &((byte *)acolormap)[index0->v[4] & 0xFF00];

		if (ptri[i].facesfront)
		{
			D_PolysetRecursiveTriangleT(index0->v, index1->v, index2->v);
		}
		else
		{
			int		s0, s1, s2;

			s0 = index0->v[2];
			s1 = index1->v[2];
			s2 = index2->v[2];

			if (index0->flags & ALIAS_ONSEAM)
				index0->v[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				index1->v[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				index2->v[2] += r_affinetridesc.seamfixupX16;

			D_PolysetRecursiveTriangleT(index0->v, index1->v, index2->v);

			index0->v[2] = s0;
			index1->v[2] = s1;
			index2->v[2] = s2;
		}
	}
}

static void D_DrawSubdivT2 (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int			i, lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++)
	{
		index0 = pfv + ptri[i].vertindex[0];
		index1 = pfv + ptri[i].vertindex[1];
		index2 = pfv + ptri[i].vertindex[2];

		if (((index0->v[1]-index1->v[1]) * (index0->v[0]-index2->v[0]) -
			 (index0->v[0]-index1->v[0]) * (index0->v[1]-index2->v[1])) >= 0)
		{
			continue;
		}

		d_pcolormap = &((byte *)acolormap)[index0->v[4] & 0xFF00];

		if (ptri[i].facesfront)
		{
			D_PolysetRecursiveTriangleT2(index0->v, index1->v, index2->v);
		}
		else
		{
			int		s0, s1, s2;

			s0 = index0->v[2];
			s1 = index1->v[2];
			s2 = index2->v[2];

			if (index0->flags & ALIAS_ONSEAM)
				index0->v[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				index1->v[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				index2->v[2] += r_affinetridesc.seamfixupX16;

			D_PolysetRecursiveTriangleT2(index0->v, index1->v, index2->v);

			index0->v[2] = s0;
			index1->v[2] = s1;
			index2->v[2] = s2;
		}
	}
}

static void D_DrawSubdivT3 (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int			i, lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++)
	{
		index0 = pfv + ptri[i].vertindex[0];
		index1 = pfv + ptri[i].vertindex[1];
		index2 = pfv + ptri[i].vertindex[2];

		if (((index0->v[1]-index1->v[1]) * (index0->v[0]-index2->v[0]) -
			 (index0->v[0]-index1->v[0]) * (index0->v[1]-index2->v[1])) >= 0)
		{
			continue;
		}

		d_pcolormap = &((byte *)acolormap)[index0->v[4] & 0xFF00];

		if (ptri[i].facesfront)
		{
			D_PolysetRecursiveTriangleT3(index0->v, index1->v, index2->v);
		}
		else
		{
			int		s0, s1, s2;

			s0 = index0->v[2];
			s1 = index1->v[2];
			s2 = index2->v[2];

			if (index0->flags & ALIAS_ONSEAM)
				index0->v[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				index1->v[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				index2->v[2] += r_affinetridesc.seamfixupX16;

			D_PolysetRecursiveTriangleT3(index0->v, index1->v, index2->v);

			index0->v[2] = s0;
			index1->v[2] = s1;
			index2->v[2] = s2;
		}
	}
}

static void D_DrawSubdivT5 (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int			i, lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++)
	{
		index0 = pfv + ptri[i].vertindex[0];
		index1 = pfv + ptri[i].vertindex[1];
		index2 = pfv + ptri[i].vertindex[2];

		if (((index0->v[1]-index1->v[1]) * (index0->v[0]-index2->v[0]) -
			 (index0->v[0]-index1->v[0]) * (index0->v[1]-index2->v[1])) >= 0)
		{
			continue;
		}

		if (ptri[i].facesfront)
		{
			D_PolysetRecursiveTriangleT5(index0->v, index1->v, index2->v);
		}
		else
		{
			int		s0, s1, s2;

			s0 = index0->v[2];
			s1 = index1->v[2];
			s2 = index2->v[2];

			if (index0->flags & ALIAS_ONSEAM)
				index0->v[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				index1->v[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				index2->v[2] += r_affinetridesc.seamfixupX16;

			D_PolysetRecursiveTriangleT5(index0->v, index1->v, index2->v);

			index0->v[2] = s0;
			index1->v[2] = s1;
			index2->v[2] = s2;
		}
	}
}


/*
================
D_DrawNonSubdiv
================
*/
static void D_DrawNonSubdiv (void)
{
	mtriangle_t		*ptri;
	finalvert_t		*pfv, *index0, *index1, *index2;
	int			i, lnumtriangles;

	pfv = r_affinetridesc.pfinalverts;
	ptri = r_affinetridesc.ptriangles;
	lnumtriangles = r_affinetridesc.numtriangles;

	for (i = 0; i < lnumtriangles; i++, ptri++)
	{
		index0 = pfv + ptri->vertindex[0];
		index1 = pfv + ptri->vertindex[1];
		index2 = pfv + ptri->vertindex[2];

		d_xdenom = (index0->v[1]-index1->v[1]) * (index0->v[0]-index2->v[0]) -
				(index0->v[0]-index1->v[0])*(index0->v[1]-index2->v[1]);

		if (d_xdenom >= 0)
		{
			continue;
		}

		r_p0[0] = index0->v[0];		// u
		r_p0[1] = index0->v[1];		// v
		r_p0[2] = index0->v[2];		// s
		r_p0[3] = index0->v[3];		// t
		r_p0[4] = index0->v[4];		// light
		r_p0[5] = index0->v[5];		// iz

		r_p1[0] = index1->v[0];
		r_p1[1] = index1->v[1];
		r_p1[2] = index1->v[2];
		r_p1[3] = index1->v[3];
		r_p1[4] = index1->v[4];
		r_p1[5] = index1->v[5];

		r_p2[0] = index2->v[0];
		r_p2[1] = index2->v[1];
		r_p2[2] = index2->v[2];
		r_p2[3] = index2->v[3];
		r_p2[4] = index2->v[4];
		r_p2[5] = index2->v[5];

		if (!ptri->facesfront)
		{
			if (index0->flags & ALIAS_ONSEAM)
				r_p0[2] += r_affinetridesc.seamfixupX16;
			if (index1->flags & ALIAS_ONSEAM)
				r_p1[2] += r_affinetridesc.seamfixupX16;
			if (index2->flags & ALIAS_ONSEAM)
				r_p2[2] += r_affinetridesc.seamfixupX16;
		}

		D_PolysetSetEdgeTable ();
		D_RasterizeAliasPolySmooth ();
	}
}


/*
================
D_PolysetDraw
================
*/
void D_PolysetDraw (void)
{
	a_spans = (spanpackage_t *)
			(((intptr_t)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	if (r_affinetridesc.drawtype)
	{
		D_DrawSubdiv ();
	}
	else
	{
		D_DrawNonSubdiv ();
	}
}

void D_PolysetDrawT (void)
{
	a_spans = (spanpackage_t *)
			(((intptr_t)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	if (r_affinetridesc.drawtype)
	{
		D_DrawSubdivT ();
	}
	else
	{
		D_DrawNonSubdiv ();
	}
}

void D_PolysetDrawT2 (void)
{
	a_spans = (spanpackage_t *)
			(((intptr_t)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	if (r_affinetridesc.drawtype)
	{
		D_DrawSubdivT2 ();
	}
	else
	{
		D_DrawNonSubdiv ();
	}
}

void D_PolysetDrawT3 (void)
{
	a_spans = (spanpackage_t *)
			(((intptr_t)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	if (r_affinetridesc.drawtype)
	{
		D_DrawSubdivT3 ();
	}
	else
	{
		D_DrawNonSubdiv ();
	}
}

void D_PolysetDrawT5 (void)
{
	a_spans = (spanpackage_t *)
			(((intptr_t)&spans[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));

	if (r_affinetridesc.drawtype)
	{
		D_DrawSubdivT5 ();
	}
	else
	{
		D_DrawNonSubdiv ();
	}
}


#endif	/* !id386 */


/*
================
D_PolysetUpdateTables
================
*/
void D_PolysetUpdateTables (void)
{
	int		i;
	byte		*s;

	if (r_affinetridesc.skinwidth != skinwidth ||
		r_affinetridesc.pskin != skinstart)
	{
		skinwidth = r_affinetridesc.skinwidth;
		skinstart = (byte *) r_affinetridesc.pskin;
		s = skinstart;
		for (i = 0; i < MAX_SKIN_HEIGHT; i++, s += skinwidth)
			skintable[i] = s;
	}
}


#if	!id386

#define D_PolysetScanLeftEdgeT		D_PolysetScanLeftEdge
#define D_PolysetScanLeftEdgeT2		D_PolysetScanLeftEdge
#define D_PolysetScanLeftEdgeT3		D_PolysetScanLeftEdge
#define D_PolysetScanLeftEdgeT5		D_PolysetScanLeftEdge
/*
===================
D_PolysetScanLeftEdge
====================
*/
static void D_PolysetScanLeftEdge (int height)
{
	do
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
		d_pedgespanpackage->light = d_light;
		d_pedgespanpackage->zi = d_zi;

		d_pedgespanpackage++;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_pdest += d_pdestextrastep;
			d_pz += d_pzextrastep;
			d_aspancount += d_countextrastep;
			d_ptex += d_ptexextrastep;
			d_sfrac += d_sfracextrastep;
			d_ptex += d_sfrac >> 16;

			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracextrastep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			d_light += d_lightextrastep;
			d_zi += d_ziextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_pdest += d_pdestbasestep;
			d_pz += d_pzbasestep;
			d_aspancount += ubasestep;
			d_ptex += d_ptexbasestep;
			d_sfrac += d_sfracbasestep;
			d_ptex += d_sfrac >> 16;
			d_sfrac &= 0xFFFF;
			d_tfrac += d_tfracbasestep;
			if (d_tfrac & 0x10000)
			{
				d_ptex += r_affinetridesc.skinwidth;
				d_tfrac &= 0xFFFF;
			}
			d_light += d_lightbasestep;
			d_zi += d_zibasestep;
		}
	} while (--height);
}

#endif	/* !id386 */


/*
===================
D_PolysetSetUpForLineScan
====================
*/
static void D_PolysetSetUpForLineScan(fixed8_t startvertu, fixed8_t startvertv,
					fixed8_t endvertu, fixed8_t endvertv)
{
	double		dm, dn;
	int		tm, tn;
	adivtab_t	*ptemp;

// TODO: implement x86 version

	errorterm = -1;

	tm = endvertu - startvertu;
	tn = endvertv - startvertv;

	if (((tm <= 16) && (tm >= -15)) &&
		((tn <= 16) && (tn >= -15)))
	{
		ptemp = &adivtab[((tm+15) << 5) + (tn+15)];
		ubasestep = ptemp->quotient;
		erroradjustup = ptemp->remainder;
		erroradjustdown = tn;
	}
	else
	{
		dm = (double)tm;
		dn = (double)tn;

		FloorDivMod (dm, dn, &ubasestep, &erroradjustup);

		erroradjustdown = dn;
	}
}


#if	!id386

#define D_PolysetCalcGradientsT		D_PolysetCalcGradients
#define D_PolysetCalcGradientsT2	D_PolysetCalcGradients
#define D_PolysetCalcGradientsT3	D_PolysetCalcGradients
#define D_PolysetCalcGradientsT5	D_PolysetCalcGradients
/*
================
D_PolysetCalcGradients
================
*/
#ifndef id68k
static void D_PolysetCalcGradients (int skin_width)
{
	float	xstepdenominv, ystepdenominv, t0, t1;
	float	p01_minus_p21, p11_minus_p21, p00_minus_p20, p10_minus_p20;

	p00_minus_p20 = r_p0[0] - r_p2[0];
	p01_minus_p21 = r_p0[1] - r_p2[1];
	p10_minus_p20 = r_p1[0] - r_p2[0];
	p11_minus_p21 = r_p1[1] - r_p2[1];

	xstepdenominv = 1.0 / (float)d_xdenom;

	ystepdenominv = -xstepdenominv;

// ceil () for light so positive steps are exaggerated, negative steps
// diminished,  pushing us away from underflow toward overflow. Underflow is
// very visible, overflow is very unlikely, because of ambient lighting
	t0 = r_p0[4] - r_p2[4];
	t1 = r_p1[4] - r_p2[4];
	r_lstepx = (int)ceil((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_lstepy = (int)ceil((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

	t0 = r_p0[2] - r_p2[2];
	t1 = r_p1[2] - r_p2[2];
	r_sstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_sstepy = (int)((t1 * p00_minus_p20 - t0* p10_minus_p20) * ystepdenominv);

	t0 = r_p0[3] - r_p2[3];
	t1 = r_p1[3] - r_p2[3];
	r_tstepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_tstepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

	t0 = r_p0[5] - r_p2[5];
	t1 = r_p1[5] - r_p2[5];
	r_zistepx = (int)((t1 * p01_minus_p21 - t0 * p11_minus_p21) * xstepdenominv);
	r_zistepy = (int)((t1 * p00_minus_p20 - t0 * p10_minus_p20) * ystepdenominv);

//	a_sstepxfrac = r_sstepx << 16;	// was #if id386 code
//	a_tstepxfrac = r_tstepx << 16;	// was #if id386 code
	a_sstepxfrac = r_sstepx & 0xFFFF;
	a_tstepxfrac = r_tstepx & 0xFFFF;

	a_ststepxwhole = skin_width * (r_tstepx >> 16) + (r_sstepx >> 16);
}
#endif

#endif	/* !id386 */


#if 0
byte gelmap[256];
void InitGel (byte *palette)
{
	int		i;
	int		r;

	for (i = 0; i < 256; i++)
	{
//		r = (palette[i*3]>>4);
		r = (palette[i*3] + palette[i*3+1] + palette[i*3+2])/(16*3);
		gelmap[i] = /* 64 */ 0 + r;
	}
}
#endif


#if	!id386

/*
================
D_PolysetDrawSpans8
================
*/
#ifndef id68k
static void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte		*lpdest;
	byte		*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short		*lpz;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte *) pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				if ((lzi >> 16) >= *lpz)
				{
					*lpdest = ((byte *)acolormap)[*lptex + (llight & 0xFF00)];
				// gel mapping
				//	*lpdest = gelmap[*lpdest];
					*lpz = lzi >> 16;
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}
#endif

static void D_PolysetDrawSpans8T (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte		*lpdest;
	byte		*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short		*lpz;
	unsigned int	btemp, color_map_idx;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte *) pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				color_map_idx = lptex[0];
				if (color_map_idx != 0)
				{
					if ((lzi >> 16) >= *lpz)
					{
						btemp = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
						*lpdest = mainTransTable[(btemp<<8) + (*lpdest)];
						*lpz = lzi >> 16;
					}
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

static void D_PolysetDrawSpans8T2 (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte		*lpdest;
	byte		*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short		*lpz;
	unsigned int	btemp, color_map_idx;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte *) pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				color_map_idx = lptex[0];
				if (color_map_idx != 0)
				{
					if ((lzi >> 16) >= *lpz)
					{
						if (color_map_idx % 2 == 0)
						{
							btemp = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
							*lpdest = (byte) btemp;
							*lpz = lzi >> 16;
						}
						else
						{
							btemp = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
							*lpdest = mainTransTable[(btemp<<8) + (*lpdest)];
							*lpz = lzi >> 16;
						}
					}
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

static void D_PolysetDrawSpans8T3 (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte		*lpdest;
	byte		*lptex;
	int		lsfrac, ltfrac;
	int		llight;
	int		lzi;
	short		*lpz;
	unsigned int	color_map_idx;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte *) pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			llight = pspanpackage->light;
			lzi = pspanpackage->zi;

			do
			{
				color_map_idx = lptex[0];
				if (color_map_idx != 0)
				{
					if ((lzi >> 16) >= *lpz)
					{
						*lpdest = ((byte *) acolormap)[*lptex + (llight & 0xFF00)];
						*lpz = lzi >> 16;
					}
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				llight += r_lstepx;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

static void D_PolysetDrawSpans8T5 (spanpackage_t *pspanpackage)
{
	int		lcount;
	byte		*lpdest;
	byte		*lptex;
	int		lsfrac, ltfrac;
	int		lzi;
	short		*lpz;
	unsigned int	color_map_idx;

	do
	{
		lcount = d_aspancount - pspanpackage->count;

		errorterm += erroradjustup;
		if (errorterm >= 0)
		{
			d_aspancount += d_countextrastep;
			errorterm -= erroradjustdown;
		}
		else
		{
			d_aspancount += ubasestep;
		}

		if (lcount)
		{
			lpdest = (byte *) pspanpackage->pdest;
			lptex = pspanpackage->ptex;
			lpz = pspanpackage->pz;
			lsfrac = pspanpackage->sfrac;
			ltfrac = pspanpackage->tfrac;
			lzi = pspanpackage->zi;

			do
			{
				color_map_idx = lptex[0];
				if (color_map_idx != 0)
				{
					if ((lzi >> 16) >= *lpz)
					{
						*lpdest = transTable[(color_map_idx<<8) + (*lpdest)];
						*lpz = lzi >> 16;
					}
				}
				lpdest++;
				lzi += r_zistepx;
				lpz++;
				lptex += a_ststepxwhole;
				lsfrac += a_sstepxfrac;
				lptex += lsfrac >> 16;
				lsfrac &= 0xFFFF;
				ltfrac += a_tstepxfrac;
				if (ltfrac & 0x10000)
				{
					lptex += r_affinetridesc.skinwidth;
					ltfrac &= 0xFFFF;
				}
			} while (--lcount);
		}

		pspanpackage++;
	} while (pspanpackage->count != -999999);
}

#endif	/* !id386 */


/*
================
D_PolysetFillSpans8
================
*/
void D_PolysetFillSpans8 (spanpackage_t *pspanpackage)
{
	int		color;

// FIXME: do z buffering

	color = d_aflatcolor++;

	while (1)
	{
		int		lcount;
		byte	*lpdest;

		lcount = pspanpackage->count;

		if (lcount == -1)
			return;

		if (lcount)
		{
			lpdest = (byte *) pspanpackage->pdest;

			do
			{
				*lpdest++ = color;
			} while (--lcount);
		}

		pspanpackage++;
	}
}

/*
================
D_RasterizeAliasPolySmooth
================
*/
void D_RasterizeAliasPolySmooth (void)
{
	int		initialleftheight, initialrightheight;
	int		*plefttop, *prighttop, *pleftbottom, *prightbottom;
	int		working_lstepx, originalcount;

	plefttop = pedgetable->pleftedgevert0;
	prighttop = pedgetable->prightedgevert0;

	pleftbottom = pedgetable->pleftedgevert1;
	prightbottom = pedgetable->prightedgevert1;

	initialleftheight = pleftbottom[1] - plefttop[1];
	initialrightheight = prightbottom[1] - prighttop[1];

//
// set the s, t, and light gradients, which are consistent across the triangle
// because being a triangle, things are affine
//
	if ((currententity->model->flags & EF_SPECIAL_TRANS))
		D_PolysetCalcGradientsT5 (r_affinetridesc.skinwidth);
	else if (currententity->drawflags & DRF_TRANSLUCENT)
		D_PolysetCalcGradientsT (r_affinetridesc.skinwidth);
	else if ((currententity->model->flags & EF_TRANSPARENT))
		D_PolysetCalcGradientsT2 (r_affinetridesc.skinwidth);
	else if ((currententity->model->flags & EF_HOLEY))
		D_PolysetCalcGradientsT3 (r_affinetridesc.skinwidth);
	else
		D_PolysetCalcGradients (r_affinetridesc.skinwidth);

//
// rasterize the polygon
//

//
// scan out the top (and possibly only) part of the left edge
//
	d_pedgespanpackage = a_spans;

	ystart = plefttop[1];
	d_aspancount = plefttop[0] - prighttop[0];

	d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
			(plefttop[3] >> 16) * r_affinetridesc.skinwidth;
#if	id386
	d_sfrac = (plefttop[2] & 0xFFFF) << 16;
	d_tfrac = (plefttop[3] & 0xFFFF) << 16;
#else
	d_sfrac = plefttop[2] & 0xFFFF;
	d_tfrac = plefttop[3] & 0xFFFF;
#endif
	d_light = plefttop[4];
	d_zi = plefttop[5];

	d_pdest = (byte *)d_viewbuffer + ystart * screenwidth + plefttop[0];
	d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

	if (initialleftheight == 1)
	{
		d_pedgespanpackage->pdest = d_pdest;
		d_pedgespanpackage->pz = d_pz;
		d_pedgespanpackage->count = d_aspancount;
		d_pedgespanpackage->ptex = d_ptex;

		d_pedgespanpackage->sfrac = d_sfrac;
		d_pedgespanpackage->tfrac = d_tfrac;

	// FIXME: need to clamp l, s, t, at both ends?
		d_pedgespanpackage->light = d_light;
		d_pedgespanpackage->zi = d_zi;

		d_pedgespanpackage++;
	}
	else
	{
		D_PolysetSetUpForLineScan(plefttop[0], plefttop[1], pleftbottom[0], pleftbottom[1]);

#if	id386
		d_pzbasestep = (d_zwidth + ubasestep) << 1;
		d_pzextrastep = d_pzbasestep + 2;
#else
		d_pzbasestep = d_zwidth + ubasestep;
		d_pzextrastep = d_pzbasestep + 1;
#endif

		d_pdestbasestep = screenwidth + ubasestep;
		d_pdestextrastep = d_pdestbasestep + 1;

	// TODO: can reuse partial expressions here

	// for negative steps in x along left edge, bias toward overflow rather than
	// underflow (sort of turning the floor () we did in the gradient calcs into
	// ceil (), but plus a little bit)
		if (ubasestep < 0)
			working_lstepx = r_lstepx - 1;
		else
			working_lstepx = r_lstepx;

		d_countextrastep = ubasestep + 1;
		d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
				((r_tstepy + r_tstepx * ubasestep) >> 16) * r_affinetridesc.skinwidth;
#if	id386
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) << 16;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) << 16;
#else
		d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
		d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
#endif
		d_lightbasestep = r_lstepy + working_lstepx * ubasestep;
		d_zibasestep = r_zistepy + r_zistepx * ubasestep;

		d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
				((r_tstepy + r_tstepx * d_countextrastep) >> 16) * r_affinetridesc.skinwidth;
#if	id386
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) << 16;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) << 16;
#else
		d_sfracextrastep = (r_sstepy + r_sstepx*d_countextrastep) & 0xFFFF;
		d_tfracextrastep = (r_tstepy + r_tstepx*d_countextrastep) & 0xFFFF;
#endif
		d_lightextrastep = d_lightbasestep + working_lstepx;
		d_ziextrastep = d_zibasestep + r_zistepx;

		if ((currententity->model->flags & EF_SPECIAL_TRANS))
			D_PolysetScanLeftEdgeT5 (initialleftheight);
		else if (currententity->drawflags & DRF_TRANSLUCENT)
			D_PolysetScanLeftEdgeT (initialleftheight);
		else if ((currententity->model->flags & EF_TRANSPARENT))
			D_PolysetScanLeftEdgeT2 (initialleftheight);
		else if ((currententity->model->flags & EF_HOLEY))
			D_PolysetScanLeftEdgeT3 (initialleftheight);
		else
			D_PolysetScanLeftEdge (initialleftheight);
	}

//
// scan out the bottom part of the left edge, if it exists
//
	if (pedgetable->numleftedges == 2)
	{
		int		height;

		plefttop = pleftbottom;
		pleftbottom = pedgetable->pleftedgevert2;

		height = pleftbottom[1] - plefttop[1];

// TODO: make this a function; modularize this function in general

		ystart = plefttop[1];
		d_aspancount = plefttop[0] - prighttop[0];
		d_ptex = (byte *)r_affinetridesc.pskin + (plefttop[2] >> 16) +
				(plefttop[3] >> 16) * r_affinetridesc.skinwidth;
		d_sfrac = 0;
		d_tfrac = 0;
		d_light = plefttop[4];
		d_zi = plefttop[5];

		d_pdest = (byte *)d_viewbuffer + ystart * screenwidth + plefttop[0];
		d_pz = d_pzbuffer + ystart * d_zwidth + plefttop[0];

		if (height == 1)
		{
			d_pedgespanpackage->pdest = d_pdest;
			d_pedgespanpackage->pz = d_pz;
			d_pedgespanpackage->count = d_aspancount;
			d_pedgespanpackage->ptex = d_ptex;

			d_pedgespanpackage->sfrac = d_sfrac;
			d_pedgespanpackage->tfrac = d_tfrac;

		// FIXME: need to clamp l, s, t, at both ends?
			d_pedgespanpackage->light = d_light;
			d_pedgespanpackage->zi = d_zi;

			d_pedgespanpackage++;
		}
		else
		{
			D_PolysetSetUpForLineScan(plefttop[0], plefttop[1], pleftbottom[0], pleftbottom[1]);

			d_pdestbasestep = screenwidth + ubasestep;
			d_pdestextrastep = d_pdestbasestep + 1;

#if	id386
			d_pzbasestep = (d_zwidth + ubasestep) << 1;
			d_pzextrastep = d_pzbasestep + 2;
#else
			d_pzbasestep = d_zwidth + ubasestep;
			d_pzextrastep = d_pzbasestep + 1;
#endif

			if (ubasestep < 0)
				working_lstepx = r_lstepx - 1;
			else
				working_lstepx = r_lstepx;

			d_countextrastep = ubasestep + 1;
			d_ptexbasestep = ((r_sstepy + r_sstepx * ubasestep) >> 16) +
					((r_tstepy + r_tstepx * ubasestep) >> 16) * r_affinetridesc.skinwidth;
#if	id386
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) << 16;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) << 16;
#else
			d_sfracbasestep = (r_sstepy + r_sstepx * ubasestep) & 0xFFFF;
			d_tfracbasestep = (r_tstepy + r_tstepx * ubasestep) & 0xFFFF;
#endif
			d_lightbasestep = r_lstepy + working_lstepx * ubasestep;
			d_zibasestep = r_zistepy + r_zistepx * ubasestep;

			d_ptexextrastep = ((r_sstepy + r_sstepx * d_countextrastep) >> 16) +
					((r_tstepy + r_tstepx * d_countextrastep) >> 16) * r_affinetridesc.skinwidth;
#if	id386
			d_sfracextrastep = ((r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF)<<16;
			d_tfracextrastep = ((r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF)<<16;
#else
			d_sfracextrastep = (r_sstepy+r_sstepx*d_countextrastep) & 0xFFFF;
			d_tfracextrastep = (r_tstepy+r_tstepx*d_countextrastep) & 0xFFFF;
#endif
			d_lightextrastep = d_lightbasestep + working_lstepx;
			d_ziextrastep = d_zibasestep + r_zistepx;

			if ((currententity->model->flags & EF_SPECIAL_TRANS))
				D_PolysetScanLeftEdgeT5 (height);
			else if (currententity->drawflags & DRF_TRANSLUCENT)
				D_PolysetScanLeftEdgeT (height);
			else if ((currententity->model->flags & EF_TRANSPARENT))
				D_PolysetScanLeftEdgeT2 (height);
			else if ((currententity->model->flags & EF_HOLEY))
				D_PolysetScanLeftEdgeT3 (height);
			else
				D_PolysetScanLeftEdge (height);
		}
	}

// scan out the top (and possibly only) part of the right edge, updating the
// count field
	d_pedgespanpackage = a_spans;

	D_PolysetSetUpForLineScan(prighttop[0], prighttop[1], prightbottom[0], prightbottom[1]);
	d_aspancount = 0;
	d_countextrastep = ubasestep + 1;
	originalcount = a_spans[initialrightheight].count;
	a_spans[initialrightheight].count = -999999; // mark end of the spanpackages

	if ((currententity->model->flags & EF_SPECIAL_TRANS))
		D_PolysetDrawSpans8T5 (a_spans);
	else if (currententity->drawflags & DRF_TRANSLUCENT)
		D_PolysetDrawSpans8T (a_spans);
	else if ((currententity->model->flags & EF_TRANSPARENT))
		D_PolysetDrawSpans8T2 (a_spans);
	else if ((currententity->model->flags & EF_HOLEY))
		D_PolysetDrawSpans8T3 (a_spans);
	else
		D_PolysetDrawSpans8 (a_spans);

// scan out the bottom part of the right edge, if it exists
	if (pedgetable->numrightedges == 2)
	{
		int				height;
		spanpackage_t	*pstart;

		pstart = a_spans + initialrightheight;
		pstart->count = originalcount;

		d_aspancount = prightbottom[0] - prighttop[0];

		prighttop = prightbottom;
		prightbottom = pedgetable->prightedgevert2;

		height = prightbottom[1] - prighttop[1];

		D_PolysetSetUpForLineScan(prighttop[0], prighttop[1], prightbottom[0], prightbottom[1]);

		d_countextrastep = ubasestep + 1;
		a_spans[initialrightheight + height].count = -999999;
											// mark end of the spanpackages
		if ((currententity->model->flags & EF_SPECIAL_TRANS))
			D_PolysetDrawSpans8T5 (pstart);
		else if (currententity->drawflags & DRF_TRANSLUCENT)
			D_PolysetDrawSpans8T (pstart);
		else if ((currententity->model->flags & EF_TRANSPARENT))
			D_PolysetDrawSpans8T2 (pstart);
		else if ((currententity->model->flags & EF_HOLEY))
			D_PolysetDrawSpans8T3 (pstart);
		else
			D_PolysetDrawSpans8 (pstart);
	}
}


/*
================
D_PolysetSetEdgeTable
================
*/
void D_PolysetSetEdgeTable (void)
{
	int			edgetableindex;

	edgetableindex = 0;	// assume the vertices are already in
						//  top to bottom order

//
// determine which edges are right & left, and the order in which
// to rasterize them
//
	if (r_p0[1] >= r_p1[1])
	{
		if (r_p0[1] == r_p1[1])
		{
			if (r_p0[1] < r_p2[1])
				pedgetable = &edgetables[2];
			else
				pedgetable = &edgetables[5];

			return;
		}
		else
		{
			edgetableindex = 1;
		}
	}

	if (r_p0[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[8];
		else
			pedgetable = &edgetables[9];

		return;
	}
	else if (r_p1[1] == r_p2[1])
	{
		if (edgetableindex)
			pedgetable = &edgetables[10];
		else
			pedgetable = &edgetables[11];

		return;
	}

	if (r_p0[1] > r_p2[1])
		edgetableindex += 2;

	if (r_p1[1] > r_p2[1])
		edgetableindex += 4;

	pedgetable = &edgetables[edgetableindex];
}


#if 0

void D_PolysetRecursiveDrawLine (int *lp1, int *lp2)
{
	int		d;
	int		new_p[6];
	int		ofs;

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;

	return;	// line is completed

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[4] = (lp1[4] + lp2[4]) >> 1;

// draw the point
	ofs = d_scantable[new_p[1]] + new_p[0];
	if (new_p[5] > d_pzbuffer[ofs])
	{
		unsigned int	pix;

		d_pzbuffer[ofs] = new_p[5];
		pix = skintable[new_p[3]>>16][new_p[2]>>16];
//		pix = ((byte *)acolormap)[pix + (new_p[4] & 0xFF00)];
		d_viewbuffer[ofs] = pix;
	}

// recursively continue
	D_PolysetRecursiveDrawLine (lp1, new_p);
	D_PolysetRecursiveDrawLine (new_p, lp2);
}

void D_PolysetRecursiveTriangle2 (int *lp1, int *lp2, int *lp3)
{
	int		d;
	int		new_p[6];

	d = lp2[0] - lp1[0];
	if (d < -1 || d > 1)
		goto split;
	d = lp2[1] - lp1[1];
	if (d < -1 || d > 1)
		goto split;
	return;

split:
// split this edge
	new_p[0] = (lp1[0] + lp2[0]) >> 1;
	new_p[1] = (lp1[1] + lp2[1]) >> 1;
	new_p[5] = (lp1[5] + lp2[5]) >> 1;
	new_p[2] = (lp1[2] + lp2[2]) >> 1;
	new_p[3] = (lp1[3] + lp2[3]) >> 1;
	new_p[4] = (lp1[4] + lp2[4]) >> 1;

	D_PolysetRecursiveDrawLine (new, lp3);

// recursively continue
	D_PolysetRecursiveTriangle (lp1, new, lp3);
	D_PolysetRecursiveTriangle (new, lp2, lp3);
}

#endif

