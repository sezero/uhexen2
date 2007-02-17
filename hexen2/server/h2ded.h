/*
	h2ded.h
	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/h2ded.h,v 1.4 2007-02-17 11:06:02 sezero Exp $
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

