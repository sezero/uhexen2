/*
	loki_patch/arch.h
	common arch definitions

	$Id: arch.h,v 1.7 2008-12-27 19:33:24 sezero Exp $

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
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#ifndef __ARCH_H__
#define __ARCH_H__

/* Make sure the BYTE_ORDER macro is defined */

#if defined(_WIN32) || defined(_WIN64)
# define BIG_ENDIAN  0
# define LIL_ENDIAN  1
# define BYTE_ORDER  LIL_ENDIAN
#else
# define HAVE_FTW
# ifdef __FreeBSD__
#   include <machine/endian.h>
# elif defined(__DJGPP__)
#   include <machine/endian.h>
# elif defined(sgi)
#   include <sys/endian.h>
# elif defined(sun)
#   include <sys/isa_defs.h>
#   ifndef BYTE_ORDER
#     define BIG_ENDIAN 0
#     define LIL_ENDIAN 1
#     ifdef _LITTLE_ENDIAN
#       define BYTE_ORDER LIL_ENDIAN
#     else
#       define BYTE_ORDER BIG_ENDIAN
#     endif
#   endif
# elif defined(__svr4__)
#   include <sys/byteorder.h>
# elif defined(hpux)
#   include <arpa/nameser.h>
# elif defined(_AIX)
#   include <sys/machine.h>
# elif defined(darwin)
#   include<ppc/endian.h>
# else
#  include <endian.h>
# endif
#endif	/* ! WINDOWS */

#endif	/* __ARCH_H__ */

