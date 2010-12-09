/*
	xbm_icon.h
	wrapper header file including
	the correct xbm icon file for
	different Hexen II flavors

	$Id: xbm_icon.h,v 1.4 2007-09-07 20:06:03 sezero Exp $
*/

#if defined(H2W)
// hexenworld
#	include "../../resource/h2w_ico.xbm"
#elif defined(H2MP)
// hexen2 with mission pack
#	include "../resource/h2mp_ico.xbm"
#else
// plain hexen2
#	include "../resource/h2_ico.xbm"
#endif

