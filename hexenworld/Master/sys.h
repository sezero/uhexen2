/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/sys.h,v 1.4 2007-04-19 17:45:15 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

void Sys_Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
void Sys_Quit (void) __attribute__((noreturn));
double Sys_DoubleTime (void);

int Sys_CheckInput (int ns);
char *Sys_ConsoleInput (void);

#endif	/* __HX2_SYS_H */

