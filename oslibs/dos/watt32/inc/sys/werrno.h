/*!\file sys/werrno.h
 *
 * sys_errlist[] and errno's for compilers with limited errnos.
 * For WIN32, we do NOT use the <winsock.h> WSAE* codes.
 *
 * G. Vanem <giva@bgnett.no> 1998 - 2004
 */

#ifndef __SYS_WERRNO_H
#define __SYS_WERRNO_H

/* When doing "gcc -MM" with gcc 3.0 we must include <sys/version.h>
 * (via stdio.h) in order for __DJGPP__ to be defined
 */
#include <stdio.h>
#include <errno.h>

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

/* Hack: fix for compiling with djgpp 2.04, but
 *       ./util/dj_err.exe was compiled with 2.03
 */
#if defined(MIXING_DJGPP_203_AND_204)
#undef ELOOP
#endif

#if defined(__MINGW32__)
  #include <sys/mingw32.err>

#elif defined(__HIGHC__)
  #undef EDEADLK
  #undef EDEADLOCK
  #include <sys/highc.err>

#elif defined(__BORLANDC__)
  #ifdef __FLAT__
  #undef ENAMETOOLONG  /* bcc32 4.0 */
  #endif
  #ifdef _WIN32
  #undef ENOTEMPTY
  #endif
  #include <sys/borlandc.err>

#elif defined(__TURBOC__)
  #include <sys/turboc.err>

#elif defined(__WATCOMC__)
  #include <sys/watcom.err>

#elif defined(__DJGPP__)
  #include <sys/djgpp.err>

#elif defined(__DMC__)                        /* Digital Mars Compiler */
  #include <sys/digmars.err>

#elif defined(_MSC_VER) && (_MSC_VER <= 800)  /* MSC 8.0 or older */
  #include <sys/quickc.err>

#elif defined(_MSC_VER) && (_MSC_VER > 800)   /* Visual C on Windows */
  #undef EDEADLOCK
  #include <sys/visualc.err>

#elif defined(__CCDL__)                       /* LadSoft's cc386.exe */
  #include <sys/ladsoft.err>

#elif defined(__LCC__)
  #include <sys/lcc.err>

#elif defined(__POCC__)
  #include <sys/pellesc.err>

#else
  #error Unknown target in <sys/werrno.h>.
#endif

/*
 * Ugly hack ahead. Someone tell me a better way, but
 * errno and friends are macros on Windows. Redefine them
 * to point to our variables.
 *
 * On Windows, the usual 'errno' is a macro "(*_errno)()" that
 * is problematic to use as a lvalue.
 * On other platforms we modify the global 'errno' variable directly.
 * (see SOCK_ERRNO() in misc.h). So no need to redefine it in any way.
 */

W32_DATA int   _w32_errno;
W32_DATA int   _w32_sys_nerr;
W32_DATA char *_w32_sys_errlist[];

#if 0
  #undef  sys_nerr
  #define sys_nerr      _w32_sys_nerr

  #undef  _sys_nerr
  #define _sys_nerr     _w32_sys_nerr

  #undef  __sys_nerr
  #define __sys_nerr    _w32_sys_nerr

  #undef  sys_errlist
  #define sys_errlist   _w32_sys_errlist

  #undef  _sys_errlist
  #define _sys_errlist  _w32_sys_errlist

  #undef  __sys_errlist
  #define __sys_errlist _w32_sys_errlist
#endif

/*
 * Incase you have trouble with duplicate defined symbols,
 * make sure the "*_s()" versions are referenced before normal
 * perror() and strerror() in your C-library.
 */
W32_FUNC void  perror_s   (const char *str);
W32_FUNC char *strerror_s (int errnum);

#if defined(__cplusplus)  /* Damn C++ */
  #include <stdlib.h>
  #include <string.h>

#elif !defined(_MSC_VER) && !defined(WIN32)
  #if !defined(_INC_STDLIB) && !defined(_STDLIB_H_) && !defined(_STDIO_H)
  W32_FUNC void perror (const char *str);
  #endif

  #if !defined(_INC_STRING) && !defined(_STRING_H) && !defined(_STRING_H_)
  W32_FUNC char *strerror (int errnum);
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  W32_FUNC int  __stdcall WSAGetLastError (void);
  W32_FUNC void __stdcall WSASetLastError (int err);
#endif

#endif  /* __SYS_WERRNO_H */

