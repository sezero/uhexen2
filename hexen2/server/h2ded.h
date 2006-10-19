/*
	h2ded.h
	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/h2ded.h,v 1.3 2006-10-19 06:32:29 sezero Exp $
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

#define	Con_Printf	Sys_Printf
#define	Con_SafePrintf	Sys_Printf
#define	Con_DPrintf	Sys_DPrintf

#endif	/* __HX2_DED_H */

