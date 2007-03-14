/*
	h2ded.h
	header for hexen2 dedicated server

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/h2ded.h,v 1.5 2007-03-14 21:03:25 sezero Exp $
*/

#ifndef __HX2_DED_H
#define __HX2_DED_H


typedef struct
{
	vec3_t		viewangles;

// intended velocities
	float		forwardmove;
	float		sidemove;
	float		upmove;
	byte		lightlevel;
} usercmd_t;

//=============================================================================

extern	cvar_t	cl_warncmd;

//=============================================================================

#define	Con_Printf	Sys_Printf
#define	Con_SafePrintf	Sys_Printf
#define	Con_DPrintf	Sys_DPrintf

//=============================================================================

#endif	/* __HX2_DED_H */

