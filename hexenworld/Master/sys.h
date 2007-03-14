/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/sys.h,v 1.2 2007-03-14 21:04:15 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

void Sys_Error (const char *error, ...) _FUNC_PRINTF(1);
void Sys_Quit (void);
double Sys_DoubleTime (void);

int Sys_CheckInput (int ns);
char *Sys_ConsoleInput (void);

#endif	/* __HX2_SYS_H */

