//
//	wrapper header file including
//	the correct xbm icon file for
//	different Hexen II flavors
//

#if defined(H2W)
// hexenworld
#	include "../icons/h2w_ico.xbm"
#elif defined(H2MP)
// hexen2 with mission pack
#	include "icons/h2mp_ico.xbm"
#else
// plain hexen2
#	include "icons/h2_ico.xbm"
#endif

