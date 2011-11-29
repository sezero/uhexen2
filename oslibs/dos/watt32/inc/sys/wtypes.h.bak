/*!\file sys/wtypes.h
 * Watt-32 type definitions.
 */

/*-
 * Copyright (c) 1982, 1986, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)types.h	7.17 (Berkeley) 5/6/91
 *      @(#)wtypes.h    Waterloo TCP/IP
 */

/*
 * the naming <sys/wtypes.h> is required for those compilers that
 * have <sys/types.h> in the usual place but doesn't define
 * the following types. This file is included from <sys/socket.h>,
 * <tcp.h> etc.
 */

#ifndef __SYS_WTYPES_H
#define __SYS_WTYPES_H

#if 0                     /* No, that causes trouble */
  #undef  FD_SETSIZE
  #define FD_SETSIZE 512  /* use same FD_SETSIZE for all targets */
#endif

#if defined(__DJGPP__) || defined(__DMC__) || defined(__MINGW32__) || defined(__POCC__)
  #include <sys/types.h>
#endif

#if defined(__DJGPP__)
  #include <machine/endian.h>
#endif

#if defined(__MINGW32__) || (defined(__DJGPP__) && DJGPP_MINOR >= 4) || \
    (defined(__WATCOMC__) && __WATCOMC__ >= 1230) ||  /* OW 1.3+ */     \
    defined(__POCC__)                                 /* PellesC */
  #undef  HAVE_STDINT_H
  #define HAVE_STDINT_H
  #include <stdint.h>   /* doesn't define 'u_char' etc. */
#endif


#if !defined(HAVE_U_INT64_T) && !defined(u_int64_t)
  #if defined(__HIGHC__) || defined(__GNUC__)
    typedef unsigned long long  u_int64_t;
    #define HAVE_U_INT64_T

  #elif defined(__DMC__) && (__INTSIZE == 4)
    typedef unsigned long long  u_int64_t;
    #define HAVE_U_INT64_T

  #elif defined(__WATCOMC__) && defined(__WATCOM_INT64__)
    typedef unsigned __int64  u_int64_t;
    #define HAVE_U_INT64_T

  #elif (defined(_MSC_VER) && (_MSC_VER >= 900)) || defined(__POCC__)
    typedef unsigned __int64  u_int64_t;
    #define HAVE_U_INT64_T
  #endif
#endif

#if !defined(__GLIBC__)
  #if !defined(HAVE_U_CHAR) && !defined(u_char)
    typedef unsigned char  u_char;
    #define HAVE_U_CHAR
  #endif

  #if !defined(HAVE_U_SHORT) && !defined(u_short)
    typedef unsigned short  u_short;
    #define HAVE_U_SHORT
  #endif

  #if !defined(HAVE_USHORT) && !defined(ushort)
    typedef unsigned short  ushort;   /* SysV compatibility */
    #define HAVE_USHORT
  #endif

  #if !defined(HAVE_U_LONG) && !defined(u_long)
    typedef unsigned long  u_long;
    #define HAVE_U_LONG
  #endif

  #if !defined(HAVE_U_INT) && !defined(u_int)
    #if defined(__SMALL__) || defined(__LARGE__)
      typedef unsigned long u_int;  /* too many headers assumes u_int is >=32-bit */
    #else
      typedef unsigned int  u_int;
    #endif
    #define HAVE_U_INT
  #endif

  #if !defined(HAVE_CADDR_T) && !defined(caddr_t)
    typedef unsigned long  caddr_t;
    #define HAVE_CADDR_T
  #endif
#endif

#if !defined(HAVE_U_INT8_T) && !defined(u_int8_t)
  typedef unsigned char  u_int8_t;
#endif

#if !defined(HAVE_U_INT16_T) && !defined(u_int16_t)
  typedef unsigned short  u_int16_t;
#endif

#if !defined(HAVE_U_INT32_T) && !defined(u_int32_t)
  typedef unsigned long  u_int32_t;
#endif

#if !defined(HAVE_STDINT_H)
  #if !defined(HAVE_INT16_T) && !defined(int16_t)
    typedef short  int16_t;
    #define HAVE_INT16_T
  #endif

  #if !defined(HAVE_INT32_T) && !defined(int32_t)
    typedef long  int32_t;
    #define HAVE_INT32_T
  #endif
#endif

#if !defined(HAVE_U_QUAD_T) && !defined(u_quad_t)
  #define HAVE_U_QUAD_T
  #ifdef HAVE_U_INT64_T
    #define u_quad_t  u_int64_t
  #else
    #define u_quad_t  unsigned long
  #endif
#endif

#if !defined(IOVEC_DEFINED)
  #define IOVEC_DEFINED
  struct iovec {
         void *iov_base;
         int   iov_len;
       };
#endif


#define __BIT_TYPES_DEFINED__

#if !defined(FD_SET)  /* not djgpp */

  #undef  FD_SETSIZE
  #define FD_SETSIZE    512
  #define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
  #define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
  #define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
  #define FD_ZERO(p)    memset ((void*)(p), 0, sizeof(*(p)))

  typedef struct fd_set {
          unsigned char fd_bits [(FD_SETSIZE+7)/8];
        } fd_set;
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN    4321
#endif

#if defined(BIG_ENDIAN_MACHINE) || defined(USE_BIGENDIAN)
  #define BYTE_ORDER  BIG_ENDIAN
#else
  #define BYTE_ORDER  LITTLE_ENDIAN
#endif

#endif
