/*!\file sys/w32api.h
 *
 * Watt-32 API decoration for Win32 targets.
 */
#ifndef __SYS_W32API_H
#define __SYS_W32API_H

#if !defined(_WATT32_FAKE_WINSOCK_H) && (defined(_WINSOCK_H) || defined(_WINSOCKAPI_))
  #error Never include the real <winsock.h> in Watt-32 programs.
  #error Change your include-path so the fake <winsock.h> gets included first.
#endif

#if !defined(_WATT32_FAKE_WINSOCK2_H) && (defined(_WINSOCK2_H) || defined(_WINSOCK2API_))
  #error Never include the real <winsock2.h> in Watt-32 programs.
  #error Change your include-path so the fake <winsock2.h> gets included first.
#endif

#if !defined(_WATT32_FAKE_WS2TCPIP_H) && defined(_WS2TCPIP_H)
  #error Never include the real <ws2tcpip.h> in Watt-32 programs.
  #error Change your include-path so the fake <ws2tcpip.h> gets included first.
#endif

#if defined(WIN32) || defined(_WIN32)
  /* Don't include the real <winsock*.h> */
  #define _WINSOCKAPI_
  #define _WINSOCK2API_
  #define _WINSOCK_H
  #define _WINSOCK2_H
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#endif

/*
 * For non-Win32 targets the .\util\mkimp program (a small C-preprocessor)
 * is meant to search all headers for W32_FUNC/W32_DATA prefixes. All
 * functions with a W32_FUNC prefix will produce an export stub function.
 * See dj_dxe.mak. Very experimental at the moment.
 *
 * Note: only a small subset of the Winsock extensions are implemented in
 *       watt-32.dll (hardly any WSA*() functions yet).
 */
#if (defined(WIN32) || defined(_WIN32)) && !defined(WATT32_STATIC)
  #if defined(WATT32_BUILD)
    #define W32_FUNC  extern __declspec(dllexport)
    #define W32_DATA  extern __declspec(dllexport)
  #else
    #define W32_FUNC  extern __declspec(dllimport)
    #define W32_DATA  extern __declspec(dllimport)
  #endif
#else
  #define W32_FUNC  extern
  #define W32_DATA  extern
#endif

/*
 * W32_CALL is *not* defined to `stdcall' due to a bug in MingW's
 * linker. This bug prevents a MingW generated WATT-32.DLL from
 * being used by e.g. a MSVC program.
 * Ref. http://sources.redhat.com/bugzilla/show_bug.cgi?id=351
 * (last paragraph)
 */
#if (defined(WIN32) || defined(_WIN32)) && 0
  #define W32_CALL  __stdcall /* maybe __fastcall instead for MSVC? */
#else
  #define W32_CALL
#endif

#endif

