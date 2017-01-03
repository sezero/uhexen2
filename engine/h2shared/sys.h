/*
 * sys.h: non-portable functions
 * relies on: arch_def.h
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef HX2_SYS_H
#define HX2_SYS_H

/* file IO */

int Sys_mkdir (const char *path, qboolean crash);
int Sys_rmdir (const char *path);
int Sys_unlink (const char *path);
int Sys_rename (const char *oldp, const char *newp);

int Sys_FileType (const char *path);
/* returns an FS entity type, i.e. FS_ENT_FILE or FS_ENT_DIRECTORY.
 * returns FS_ENT_NONE (0) if no such file or directory is present. */

long Sys_filesize (const char *path);

int Sys_CopyFile (const char *frompath, const char *topath);

const char *Sys_FindFirstFile (const char *path, const char *pattern);
const char *Sys_FindNextFile (void);
void Sys_FindClose (void);

#if defined(PLATFORM_AMIGA) && !defined(SERVERONLY)
qboolean Sys_PathExistsQuiet (const char *p);
/* File existence check with the "Please insert volume XXX"
 * system requester disabled.  */
#endif


/* memory protection */

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);


/* user directories */

/* disable user directories on platforms where they
 * are not necessary or not possible. */
#if defined(PLATFORM_DOS) || defined(PLATFORM_AMIGA) || \
    defined(PLATFORM_WINDOWS) || defined(PLATFORM_OS2)
#undef	DO_USERDIRS
#define	DO_USERDIRS	0
#endif	/* DO_USERDIRS  */


/* system IO */

FUNC_NORETURN void Sys_Quit (void);
FUNC_NORETURN void Sys_Error (const char *error, ...) FUNC_PRINTF(1,2);
	/* cause the entire program to exit */
#ifdef __WATCOMC__
#pragma aux Sys_Error aborts;
#pragma aux Sys_Quit aborts;
#endif

void Sys_PrintTerm (const char *msgtxt);
	/* print the given string to the terminal */

double Sys_DoubleTime (void);

char *Sys_DateTimeString (char *buf);
	/* returns date + time string equivalent to the combination
	 * of following calls:	char buf[20]; t = time(NULL);
	 * strftime (buf, sizeof(buf), "%m/%d/%Y %H:%M:%S", localtime(&t)); */

const char *Sys_ConsoleInput (void);

void Sys_Sleep (unsigned long msecs);
	/* yield for about 'msecs' milliseconds */

void Sys_SendKeyEvents (void);
	/* perform Key_Event () callbacks until the input que is empty */

char *Sys_GetClipboardData (void);


#if defined(USE_INTEL_ASM) && \
   (defined(__i386) || defined(__i386__) || defined(__386__) || defined(_M_IX86) || defined(__I386__))

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

#endif	/* HX2_SYS_H */
