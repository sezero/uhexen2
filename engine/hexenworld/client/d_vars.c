/*
	d_vars.c
	rasterization driver global variables

	$Id: d_vars.c,v 1.3 2008-03-07 08:10:57 sezero Exp $
*/

#include	"quakedef.h"

#if	!id386

// all global and static refresh variables are collected in a contiguous block
// to avoid cache conflicts.

//-------------------------------------------------------
// global refresh variables
//-------------------------------------------------------

// FIXME: make into one big structure, like cl or sv
// FIXME: do separately for refresh engine and driver

float	d_sdivzstepu, d_tdivzstepu, d_zistepu;
float	d_sdivzstepv, d_tdivzstepv, d_zistepv;
float	d_sdivzorigin, d_tdivzorigin, d_ziorigin;

fixed16_t	sadjust, tadjust, bbextents, bbextentt;

pixel_t		*cacheblock;
int		cachewidth;
pixel_t		*d_viewbuffer;
short		*d_pzbuffer;
unsigned int	d_zrowbytes;
unsigned int	d_zwidth;

#endif	/* !id386 */

