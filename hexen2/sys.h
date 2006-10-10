// sys.h -- non-portable functions

#ifndef __HX2_SYS_H
#define __HX2_SYS_H

// maximum allowed print message length
#define MAXPRINTMSG	4096

//
// file IO
//
#if defined(_WIN32) && !defined(F_OK)
// values for the second argument to access(). MS does not define them
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif
int Sys_mkdir (char *path);

#define Sys_mkdir_err(Dir_Path) {					\
        if (Sys_mkdir((Dir_Path)))					\
		Sys_Error("Unable to create directory %s",(Dir_Path));	\
}

// simplified findfirst/findnext implementation
char *Sys_FindFirstFile (char *path, char *pattern);
char *Sys_FindNextFile (void);
void Sys_FindClose (void);

//
// memory protection
//
#if !defined(GLQUAKE) && !defined(SERVERONLY)
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);
#endif

//
// system IO
//
void Sys_Error (char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (char *fmt, ...);
// send text to the console

void Sys_Quit (void);

double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

void Sys_DebugLog(char *file, char *fmt, ...);

void Sys_Sleep (void);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging

#if !defined(SERVERONLY)

void Sys_SendKeyEvents (void);
// Perform Key_Event () callbacks until the input que is empty

#endif


/* From Dan Olson:
   The code isn't compilable on non-intel until all of the asm is
   taken out.  Don't worry about the id386 define *yet*, and even
   after all of the assembly is replaced  you may still need it
   defined for non-x86 compiles. The eventual goal should probably
   be to get rid of all x86 specific stuff.
*/

#if ( defined(_M_IX86) || defined(__i386__) ) && defined(USE_INTEL_ASM) && !defined(SERVERONLY)

#	define	id386		1
#	define	UNALIGNED_OK	1	// set to 0 if unaligned accesses are not supported
// fpu stuff
void	MaskExceptions (void);
void	Sys_SetFPCW (void);
void	Sys_LowFPPrecision (void);
void	Sys_HighFPPrecision (void);
void	Sys_PopFPCW (void);
void	Sys_PushFPCW_SetHigh (void);

#else	// not i386 on no intel asm

#	define	id386		0
#	define	UNALIGNED_OK	0
#   if !defined(SERVERONLY)
#	define	MaskExceptions()
#	define	Sys_SetFPCW()
#	define	Sys_LowFPPrecision()
#	define	Sys_HighFPPrecision()
#	define	Sys_PopFPCW()
#	define	Sys_PushFPCW_SetHigh()
#   endif
#endif

#endif	/* __HX2_SYS_H */

