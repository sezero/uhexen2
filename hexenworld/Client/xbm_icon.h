/*
	xbm_icon.h
	wrapper header file including
	the correct xbm icon file for
	different Hexen II flavors

	$Id: xbm_icon.h,v 1.3 2007-03-14 21:03:43 sezero Exp $
*/

#if defined(H2W)
// hexenworld
#	include "../icons/h2w_ico.xbm"
//#elif defined(H2MP)
// hexen2 with mission pack
//#	include "icons/h2mp_ico.xbm"
#else
// plain hexen2
#	include "icons/h2_ico.xbm"
#endif

