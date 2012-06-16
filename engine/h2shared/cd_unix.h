/*
 * cd_unix.h -- header for Unix builds to pick the correct cdaudio code
 * $Id: cd_unix.h,v 1.5 2008-04-02 20:37:36 sezero Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef __CD_UNIX_H
#define __CD_UNIX_H

#undef	__USE_BSD_CDROM__
#undef	__USE_LINUX_CDROM__
#undef	__USE_SDL_CDROM__

#if defined (WITH_SDLCD)
/* This means that the makefile is edited for USE_SDLCD */
# if defined(SDLQUAKE)
#  define	__USE_SDL_CDROM__	1
# else
#  error "cd_unix.h: WITH_SDLCD is defined but SDLQUAKE is not."
# endif
#elif defined (__linux) || defined (__linux__)
#  define	__USE_LINUX_CDROM__	1
#elif defined (__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__)
#  define	__USE_BSD_CDROM__	1
#elif defined (SDLQUAKE)
#  define	__USE_SDL_CDROM__	1
#else
#  error "no cdaudio module defined. edit cd_unix.h or your makefile.."
#endif

#endif	/* __CD_UNIX_H */

