/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/sys.h,v 1.33 2007-09-29 11:44:24 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

//
// file IO
//
int Sys_mkdir (const char *path);

#define	Sys_mkdir_err(p)	{				\
	if (Sys_mkdir((p)))					\
		Sys_Error("Unable to create directory %s",(p));	\
}

#if defined(PLATFORM_WINDOWS)
#define	Sys_rmdir	_rmdir
#elif defined(PLATFORM_UNIX)
#define	Sys_rmdir	rmdir
#else /* local implementation */
int Sys_rmdir (const char *path);
#endif

#if defined(PLATFORM_WINDOWS)
#define	Sys_unlink	_unlink
#elif defined(PLATFORM_UNIX)
#define	Sys_unlink	unlink
#else /* local implementation */
int Sys_unlink (const char *path);
#endif

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


#if defined(USE_INTEL_ASM) && !defined(SERVERONLY) && (defined(_M_IX86) || defined(__i386__))

#	define	id386		1
#	define	UNALIGNED_OK	1	/* set to 0 if unaligned accesses are not supported */
/* fpu stuff */
void	MaskExceptions (void);
void	Sys_SetFPCW (void);
void	Sys_LowFPPrecision (void);
void	Sys_HighFPPrecision (void);
void	Sys_PopFPCW (void);
void	Sys_PushFPCW_SetHigh (void);

#else	/* not i386 or no intel asm */

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

