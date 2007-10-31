/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sys.h,v 1.38 2007-10-31 19:55:46 sezero Exp $
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
#elif defined(PLATFORM_DOS)
#define	Sys_rmdir	rmdir
#elif defined(PLATFORM_UNIX)
#define	Sys_rmdir	rmdir
#else /* local implementation */
int Sys_rmdir (const char *path);
#endif

#if defined(PLATFORM_WINDOWS)
#define	Sys_unlink	_unlink
#elif defined(PLATFORM_DOS)
#define	Sys_unlink	unlink
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
// UID detection
//

/* disable use of password file on platforms where they
   aren't necessary or not possible. */
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_DOS)
#undef	USE_PASSWORD_FILE
#define	USE_PASSWORD_FILE	0
#endif	/* _PASSWORD_FILE */

//
// system IO
//
void Sys_Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
// an error will cause the entire program to exit

void Sys_PrintTerm (const char *msgtxt);
// prints the given string to the terminal

#if defined(PLATFORM_DOS)
// under DOS, we stop the terminal output when we
// will init the graphics
void Sys_EnableTerm (void);
void Sys_DisableTerm (void);
#endif	/* PLATFORM_DOS */

void Sys_Quit (void) __attribute__((noreturn));

double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

void Sys_Sleep (unsigned long msecs);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging
// 1000 milliseconds most.

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

