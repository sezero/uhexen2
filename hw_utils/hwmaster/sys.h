/*
 * sys.h: non-portable functions
 * relies on: arch_def.h
 * $Id$
 */

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

void Sys_Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
void Sys_Quit (void) __attribute__((__noreturn__));
double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

/* disable use of password file on platforms where they
   aren't necessary or not possible. */
#if defined(PLATFORM_DOS) || defined(PLATFORM_AMIGA) || \
    defined(PLATFORM_WINDOWS)
#undef	USE_PASSWORD_FILE
#define	USE_PASSWORD_FILE	0
#endif	/* _PASSWORD_FILE */

/* disable user directories on platforms where they
 * aren't necessary or not possible. */
#if defined(PLATFORM_DOS) || defined(PLATFORM_AMIGA) || \
    defined(PLATFORM_WINDOWS)
#undef	DO_USERDIRS
#define	DO_USERDIRS	0
#endif	/* DO_USERDIRS  */

#endif	/* __HX2_SYS_H */

