/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sys.h,v 1.32 2007-06-16 14:41:35 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

//
// file IO
//
int Sys_mkdir (const char *path);

#define Sys_mkdir_err(Dir_Path) {					\
        if (Sys_mkdir((Dir_Path)))					\
		Sys_Error("Unable to create directory %s",(Dir_Path));	\
}

// simplified findfirst/findnext implementation
char *Sys_FindFirstFile (const char *path, const char *pattern);
char *Sys_FindNextFile (void);
void Sys_FindClose (void);

//
// memory protection
//
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);

//
// system IO
//
void Sys_Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
// an error will cause the entire program to exit

void Sys_PrintTerm (const char *msgtxt);
// prints the given string to the terminal

void Sys_Quit (void) __attribute__((noreturn));

double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

void Sys_Sleep (void);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging

void Sys_SendKeyEvents (void);
// Perform Key_Event () callbacks until the input que is empty


/* From Dan Olson:
   The code isn't compilable on non-intel until all of the asm is
   taken out.  Don't worry about the id386 define *yet*, and even
   after all of the assembly is replaced  you may still need it
   defined for non-x86 compiles. The eventual goal should probably
   be to get rid of all x86 specific stuff.
*/

#if defined(USE_INTEL_ASM) && !defined(SERVERONLY) && (defined(_M_IX86) || defined(__i386__))

#	define	id386		1
#	define	UNALIGNED_OK	1	// set to 0 if unaligned accesses are not supported
// fpu stuff
void	MaskExceptions (void);
void	Sys_SetFPCW (void);
void	Sys_LowFPPrecision (void);
void	Sys_HighFPPrecision (void);
void	Sys_PopFPCW (void);
void	Sys_PushFPCW_SetHigh (void);

#else	// not i386 or no intel asm

#	define	id386		0
#	define	UNALIGNED_OK	0
#	define	MaskExceptions()	do {} while (0)
#	define	Sys_SetFPCW()		do {} while (0)
#	define	Sys_LowFPPrecision()	do {} while (0)
#	define	Sys_HighFPPrecision()	do {} while (0)
#	define	Sys_PopFPCW()		do {} while (0)
#	define	Sys_PushFPCW_SetHigh()	do {} while (0)
#endif

#endif	/* __HX2_SYS_H */

