/*
	view.h
	player eye positioning

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/view.h,v 1.8 2007-07-28 09:33:59 sezero Exp $
*/

#ifndef __HX2_VIEW_H
#define __HX2_VIEW_H

extern	cvar_t		v_gamma;
extern	cvar_t		crosshair;
extern	cvar_t		cl_crossx, cl_crossy;
extern	cvar_t		crosshaircolor;

#ifdef GLQUAKE
extern	float		v_blend[4];
#endif

#if 0
extern	byte		gammatable[256];	// palette is sent through this
extern	unsigned short	ramps[3][256];
#endif

void V_Init (void);
void V_RenderView (void);
void V_CalcBlend (void);
float V_CalcRoll (vec3_t angles, vec3_t velocity);
void V_UpdatePalette (void);

#endif	/* __HX2_VIEW_H */

