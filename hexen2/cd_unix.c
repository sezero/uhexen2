/*
	cd_unix.c
	$Id: cd_unix.c,v 1.2 2005-06-12 14:22:50 sezero Exp $

	Wrapper file to include correct cdaudio code

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#if defined (WITH_SDLCD)
// for this to happen, you need to
// edit the makefiles for USE_SDLCD
#include "cd_sdl.c"
#elif defined (__linux__)
#include "cd_linux.c"
#elif defined (__FreeBSD__) 
#include "cd_unix.c"
#else
#include "cd_sdl.c"
//#include "cd_null.c"
#endif

