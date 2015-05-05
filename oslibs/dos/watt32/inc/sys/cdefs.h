/*!\file sys/cdefs.h
 *
 * C-compiler definitions.
 */

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Berkeley Software Design, Inc.
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
 *	@(#)cdefs.h	8.7 (Berkeley) 1/21/94
 */

#ifndef __SYS_CDEFS_H
#define __SYS_CDEFS_H

#if defined(__DJGPP__)
  #include_next <sys/cdefs.h>
#endif

#ifdef __cplusplus
  #define __BEGIN_DECLS  extern "C" {
  #define __END_DECLS    }
#else
  #define __BEGIN_DECLS
  #define __END_DECLS
#endif

#ifndef W32_NAMESPACE
  #define W32_NAMESPACE(x) _w32_ ## x
#endif

#if defined(__CCDL__)
  #define cdecl _cdecl

#elif defined(__MINGW32__) && !defined(cdecl)
  #define cdecl __attribute__((__cdecl__))

#elif defined(_MSC_VER) || defined(__POCC__)
  #undef  cdecl
  #if (_MSC_VER <= 600)
    #define cdecl _cdecl
    #undef  __STDC__
    #define __STDC__ 1
  #else
    #define cdecl __cdecl
  #endif
  #define NO_ANSI_KEYWORDS

#elif defined(__DMC__)
  #define NO_UNDERSCORE __syscall
  /* e.g. int NO_UNDERSCORE foo (void); */
#endif

#ifndef cdecl
#define cdecl
#endif

#if defined(_MSC_VER) && !defined(WIN32) && !defined(_WIN32)
  /*
   * MS's Quick-C/Visual-C (for DOS) insists that signal-handlers, atexit
   * functions and var-arg functions must be defined cdecl.
   * This is only an issue if a program is using 'fastcall' globally
   * (cl option /Gr).
   * On Win32 this isn't an issue since WATT-32.DLL cannot be built
   * as 'fastcall' anyway.
   */
  #undef  MS_CDECL
  #define MS_CDECL   cdecl
#else
  #define MS_CDECL
#endif


struct mbuf {
       struct mbuf  *next;    /* Links mbufs belonging to single packets */
       struct mbuf  *anext;   /* Links packets on queues */
       unsigned      size;    /* Size of associated data buffer */
       int           refcnt;  /* Reference count */
       struct mbuf  *dup;     /* Pointer to duplicated mbuf */
       char         *data;    /* Active working pointers */
       unsigned      cnt;
     };


/*
 * The __CONCAT macro is used to concatenate parts of symbol names, e.g.
 * with "#define OLD(foo) __CONCAT(old,foo)", OLD(foo) produces oldfoo.
 * The __CONCAT macro is a bit tricky -- make sure you don't put spaces
 * in between its arguments.  __CONCAT can also concatenate double-quoted
 * strings produced by the __STRING macro, but this only works with ANSI C.
 */
#if (defined(__STDC__) && __STDC__) || defined(__cplusplus) || defined(__TURBOC__)
  #undef  __P
  #define __P(protos)     protos        /* full-blown ANSI C */
  #define __CONCAT(x,y)   x ## y
  #define __STRING(x)     #x

  #define __const         const         /* define reserved names to standard */
  #define __signed        signed
  #define __volatile      volatile
  #if defined(__cplusplus)
    #define __inline      inline        /* convert to C++ keyword */
  #elif !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__WATCOMC__)
    #define __inline                    /* delete GCC/MSC/Watcom keyword */
  #endif

#else
  #define __P(protos)     ()            /* traditional C preprocessor */
  #define __CONCAT(x,y)   x/**/y
  #define __STRING(x)     "x"

  #if !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__WATCOMC__)
    #define __inline
  #endif

  #if !defined(__GNUC__)
    #define __const                     /* delete pseudo-ANSI C keywords */
    #define __signed
    #define __volatile

    /*
     * In non-ANSI C environments, new programs will want ANSI-only C keywords
     * deleted from the program and old programs will want them left alone.
     * When using a compiler other than gcc, programs using the ANSI C keywords
     * const, inline etc. as normal identifiers should define -DNO_ANSI_KEYWORDS.
     * When using "gcc -traditional", we assume that this is the intent; if
     * __GNUC__ is defined but __STDC__ is not, we leave the new keywords alone.
     */
    #if !defined(NO_ANSI_KEYWORDS)
      #define const                     /* delete ANSI C keywords */
      #define inline
      #define signed
      #define volatile
    #endif
  #endif  /* !__GNUC__ */
#endif  /* !C++ */

/*
 * GCC1 and some versions of GCC2 declare dead (non-returning) and
 * pure (no side effects) functions using "volatile" and "const";
 * unfortunately, these then cause warnings under "-ansi -pedantic".
 * GCC2 uses a new, peculiar __attribute__((attrs)) style.  All of
 * these work for GNU C++ (modulo a slight glitch in the C++ grammar
 * in the distribution version of 2.5.5).
 */
#if !defined(__GNUC__) || (__GNUC__ < 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ < 5))
  #undef  __attribute__
  #define __attribute__(x)   /* delete __attribute__ if non-gcc or older than gcc 2.5 */
  #if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    #undef __dead
    #undef __pure
    #define __dead  __volatile
    #define __pure  __const
    #undef  __dead2
    #define __dead2
  #endif
#endif

#if defined(__GNUC__)  /* from djgpp-2.04 sys/cdefs.h : */
/* Ensure that always traditional GNU extern inline semantics are used
   (aka -fgnu89-inline) even if ISO C99 semantics have been specified.  */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
# define _W32_EXTERN_INLINE  extern __inline__ __attribute__ ((__gnu_inline__))
#else
# define _W32_EXTERN_INLINE  extern __inline__
#endif
#endif /* _W32_EXTERN_INLINE */

/*
 * Delete pseudo-keywords wherever they are not available or needed.
 * This seems to break MingW in mysterious ways, so leave it,
 */
#ifndef __dead
#define __dead
#endif

#ifndef __dead2
#define __dead2
#endif

#ifndef __pure
#define __pure
#endif

/*
 * min() & max() macros
 */
#if defined(__HIGHC__)
  #undef  min
  #undef  max
  #define min(a,b) _min(a,b)  /* intrinsic functions */
  #define max(a,b) _max(a,b)
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

/*
 * from NetBSD's <sys/cdefs_aout.h>
 *
 * Written by J.T. Conklin <jtc@wimsey.com> 01/17/95.
 * Public domain.
 */

/* #define _C_LABEL(x)	__CONCAT(_,x) */

#if defined(__GNUC__)
  #define __indr_reference(sym,alias)                \
          __asm__(".stabs \"_" #alias "\",11,0,0,0");\
          __asm__(".stabs \"_" #sym "\",1,0,0,0");

  #define __warn_references(sym,msg)                 \
          __asm__(".stabs \"" msg "\",30,0,0,0");    \
          __asm__(".stabs \"_" #sym "\",1,0,0,0");

  #define __IDSTRING(name,string)  \
          static const char name[] __attribute__((__unused__)) = string
#else
  #define __indr_reference(sym,alias)
  #define __warn_references(sym,msg)
  #define __IDSTRING(name,string)  static const char name[] = string
#endif

#define __RCSID(_s)                __IDSTRING(rcsid,_s)
#define __COPYRIGHT(_s)            __IDSTRING(copyright,_s)

#define __KERNEL_RCSID(_n,_s)      __IDSTRING(__CONCAT(rcsid,_n),_s)
#define __KERNEL_COPYRIGHT(_n,_s)  __IDSTRING(__CONCAT(copyright,_n),_s)

#endif
