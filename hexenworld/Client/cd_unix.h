/*
	cd_unix.h
	$Id: cd_unix.h,v 1.4 2007-12-12 10:51:08 sezero Exp $

	Unix include file to compile the correct cdaudio code

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


#ifndef __CD_UNIX_H
#define __CD_UNIX_H

#undef	__USE_BSD_CDROM__
#undef	__USE_LINUX_CDROM__
#undef	__USE_SDL_CDROM__


#if defined (WITH_SDLCD)

// for this to happen, you need to
// edit the makefiles for USE_SDLCD
#	define	__USE_SDL_CDROM__

#elif defined (__linux) || defined (__linux__)

#	define	__USE_LINUX_CDROM__

#elif defined (__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__)

#	define	__USE_BSD_CDROM__

#else

// default to SDL_cdrom
#	define	__USE_SDL_CDROM__

#endif

#endif	/* __CD_UNIX_H */

