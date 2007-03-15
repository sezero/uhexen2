/*
	h2ded.h
	header for hexen2 dedicated server

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/h2ded.h,v 1.6 2007-03-15 13:36:56 sezero Exp $
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

/* there is no redirection of Con_Printf in Hexen II: just terminal printing */
#undef	Con_Printf
#define	Con_Printf	Sys_Printf
#undef	Con_DPrintf
#define	Con_DPrintf	Sys_DPrintf

//=============================================================================

#endif	/* __HX2_DED_H */

