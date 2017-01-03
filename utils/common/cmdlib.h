/*
 * cmdlib.h --
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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

#ifndef __CMDLIB_H__
#define __CMDLIB_H__

// HEADER FILES ------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#undef	min
#undef	max
#define	q_min(a, b)	(((a) < (b)) ? (a) : (b))
#define	q_max(a, b)	(((a) > (b)) ? (a) : (b))

/* easier to include these here */
#include "strl_fn.h"
#include "qsnprint.h"

/* these qerr_ versions of functions error out if they detect, well, an error.
 * their first two arguments must the name of the caller function (see compiler.h
 * for the __thisfunc__ macro) and the line number, which should be __LINE__ .
 */
extern size_t qerr_strlcat (const char *caller, int linenum, char *dst, const char *src, size_t size);
extern size_t qerr_strlcpy (const char *caller, int linenum, char *dst, const char *src, size_t size);
extern int qerr_snprintf (const char *caller, int linenum, char *str, size_t size, const char *format, ...) FUNC_PRINTF(5,6);

// PUBLIC DATA DECLARATIONS ------------------------------------------------

/* set myargc and myargv properly
 * before calling CheckParm()  */
extern int		myargc;
extern char		**myargv;

extern char	com_token[1024];


// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

/* locale-insensitive strcasecmp replacement functions: */
extern int q_strcasecmp (const char * s1, const char * s2);
extern int q_strncasecmp (const char *s1, const char *s2, size_t n);

/* locale-insensitive strlwr/upr replacement functions: */
char	*q_strlwr (char *str);
char	*q_strupr (char *str);

double	COM_GetTime (void);

void	*SafeMalloc (size_t size);
char	*SafeStrdup (const char *str);

#ifdef __WATCOMC__
#pragma aux COM_Error aborts;
#endif
FUNC_NORETURN
void	COM_Error (const char *error, ...) FUNC_PRINTF(1,2);

int	CheckParm (const char *check);

const char	*COM_Parse (const char *data);

int	ParseHex (const char *hex);
int	ParseNum (const char *str);

#endif	/* __CMDLIB_H__	*/

