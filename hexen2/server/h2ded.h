/*
	h2ded.h
	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/server/h2ded.h,v 1.1 2006-06-25 12:57:06 sezero Exp $
*/


#if defined(GLQUAKE)
#error SERVERONLY and GLQUAKE must not be defined at the same time
#endif

//=============================================================================

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
extern void Con_DPrintf (char *fmt, ...);

