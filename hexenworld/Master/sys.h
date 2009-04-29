/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/sys.h,v 1.6 2009-04-29 07:49:28 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

void Sys_Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
void Sys_Quit (void) __attribute__((noreturn));
double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

/* disable use of password file on platforms where they
   aren't necessary or not possible. */
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_DOS)
#undef	USE_PASSWORD_FILE
#define	USE_PASSWORD_FILE	0
#endif	/* _PASSWORD_FILE */

#endif	/* __HX2_SYS_H */

