/*
	loki_patch/arch.h
	common arch definitions

	$Id: arch.h,v 1.2 2007-03-14 21:04:27 sezero Exp $
*/

#ifndef __ARCH_H__
#define __ARCH_H__

/* Make sure the BYTE_ORDER macro is defined */

#ifdef WIN32
# define BIG_ENDIAN  0
# define LIL_ENDIAN  1
# define BYTE_ORDER  LIL_ENDIAN
#else
# define HAVE_FTW
# ifdef __FreeBSD__
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
#endif /* WIN32 */

#endif /* __ARCH_H__ */
