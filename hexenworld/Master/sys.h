/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/sys.h,v 1.3 2007-04-19 14:07:18 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

void Sys_Error (const char *error, ...) __attribute__((format(printf,1,2)));
void Sys_Quit (void);
double Sys_DoubleTime (void);

int Sys_CheckInput (int ns);
char *Sys_ConsoleInput (void);

#endif	/* __HX2_SYS_H */

