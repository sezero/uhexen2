/*
	sys.h
	non-portable functions

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/sys.h,v 1.40 2008-03-07 08:10:58 sezero Exp $
*/

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

//
// file IO
//
int Sys_mkdir (const char *path, qboolean crash);

int Sys_rmdir (const char *path);
int Sys_unlink (const char *path);

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
#   if defined(__cplusplus)
#	define	__ASM_FUNCS_BEGIN	extern "C" {
#	define	__ASM_FUNCS_END			}
#   else
#	define	__ASM_FUNCS_BEGIN
#	define	__ASM_FUNCS_END
#   endif
/* fpu stuff */
__ASM_FUNCS_BEGIN
void	MaskExceptions (void);
void	Sys_SetFPCW (void);
void	Sys_LowFPPrecision (void);
void	Sys_HighFPPrecision (void);
void	Sys_PopFPCW (void);
void	Sys_PushFPCW_SetHigh (void);
__ASM_FUNCS_END

#else	/* not i386 or no intel asm */

#	define	id386		0
#	define	UNALIGNED_OK	0
#	define	__ASM_FUNCS_BEGIN
#	define	__ASM_FUNCS_END
#	define	MaskExceptions()	do {} while (0)
#	define	Sys_SetFPCW()		do {} while (0)
#	define	Sys_LowFPPrecision()	do {} while (0)
#	define	Sys_HighFPPrecision()	do {} while (0)
#	define	Sys_PopFPCW()		do {} while (0)
#	define	Sys_PushFPCW_SetHigh()	do {} while (0)
#endif

#endif	/* __HX2_SYS_H */

