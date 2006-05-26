// sys.h -- non-portable functions

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

#if !defined(SERVERONLY)

void Sys_DebugLog(char *file, char *fmt, ...);

void Sys_Sleep (void);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging

void Sys_SendKeyEvents (void);
// Perform Key_Event () callbacks until the input que is empty

#endif

// platform specific definitions

#if defined (__APPLE__)
#	undef __MACOSX__
#	define __MACOSX__	1
#elif defined (macintosh)
#	undef __MACOS__
#	define __MACOS__	1
#endif

#if defined (PLATFORM_UNIX)
#	if defined (__linux__)
#		define VERSION_PLATFORM "Linux"
#	elif defined (__DragonFly__)
#		define VERSION_PLATFORM "DragonFly"
#	elif defined (__FreeBSD__)
#		define VERSION_PLATFORM "FreeBSD"
#	elif defined (__NetBSD__)
#		define VERSION_PLATFORM "NetBSD"
#	elif defined (__OpenBSD__)
#		define VERSION_PLATFORM "OpenBSD"
#	elif defined (__MORPHOS__)
#		define VERSION_PLATFORM "MorphOS"
#	elif defined (__MACOSX__)
#		define VERSION_PLATFORM "MacOSX"
#	elif defined (__MACOS__)
#		define VERSION_PLATFORM "MacOS"
#	else
#		define VERSION_PLATFORM "Unix"
#	endif
#elif defined (_WIN32)
#	define VERSION_PLATFORM "Windows"
#else
#	define VERSION_PLATFORM "Unknown"
#	warning "Platform is UNKNOWN"
#endif

/* From Dan Olson:
   The code isn't compilable on non-intel until all of the asm is
   taken out.  Don't worry about the id386 define *yet*, and even
   after all of the assembly is replaced  you may still need it
   defined for non-x86 compiles. The eventual goal should probably
   be to get rid of all x86 specific stuff.
*/

#if ( defined(_M_IX86) || defined(__i386__) ) && defined(USE_INTEL_ASM) && !defined(SERVERONLY) && !defined(WINDED)

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

