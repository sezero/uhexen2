/*
	view.h
	player eye positioning

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/view.h,v 1.6 2007-03-14 21:03:42 sezero Exp $
*/

#ifndef __HX2_VIEW_H
#define __HX2_VIEW_H

extern	cvar_t		v_gamma;

#ifdef GLQUAKE
extern	float	v_blend[4];
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

