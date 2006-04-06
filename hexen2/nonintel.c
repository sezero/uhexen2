/*
	nonintel.c
	code for non-Intel processors only

	$Id: nonintel.c,v 1.2 2006-04-06 06:14:42 sezero Exp $
*/


//
//	TODO: Move other non-Intel code scattered around into here
//

#include "quakedef.h"

#if	!id386

/*
================
R_Surf8Patch
================
*/
void R_Surf8Patch ()
{
	// we only patch code on Intel
}


/*
================
R_Surf16Patch
================
*/
void R_Surf16Patch ()
{
	// we only patch code on Intel
}


/*
================
R_SurfacePatch
================
*/
void R_SurfacePatch (void)
{
	// we only patch code on Intel
}


#endif	// !id386

