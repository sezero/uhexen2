/*
	conproc.c
	Quake terminal for Win32

	$Id: conproc.h,v 1.3 2007-03-14 21:03:26 sezero Exp $
*/

#ifndef __CONPROC_H
#define __CONPROC_H

#define CCOM_WRITE_TEXT		0x2
// Param1 : Text

#define CCOM_GET_TEXT		0x3
// Param1 : Begin line
// Param2 : End line

#define CCOM_GET_SCR_LINES	0x4
// No params

#define CCOM_SET_SCR_LINES	0x5
// Param1 : Number of lines

void InitConProc (HANDLE hFile, HANDLE heventParent, HANDLE heventChild);
void DeinitConProc (void);

#endif	/* __CONPROC_H */

