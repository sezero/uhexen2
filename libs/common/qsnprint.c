/*
 * qsnprint.c
 * $Id$
 *
 * (v)snprintf wrappers
 * Copyright (C) 2007 O. Sezer <sezero@users.sourceforge.net>
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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "arch_def.h"
#include "compiler.h"
#include "qsnprint.h"

/* platform dependant (v)snprintf function names: */
#if defined(PLATFORM_WINDOWS)
#define	snprintf_func		_snprintf
#define	vsnprintf_func		_vsnprintf
#else
#define	snprintf_func		snprintf
#define	vsnprintf_func		vsnprintf
#endif

#if defined(__DJGPP__) &&	\
  (!defined(__DJGPP_MINOR__) || __DJGPP_MINOR__ < 4)
/* DJGPP < v2.04 doesn't have [v]snprintf().  */
/* to ensure a proper version check, include stdio.h
 * or go32.h which includes sys/version.h since djgpp
 * versions >= 2.02 and defines __DJGPP_MINOR__ */
#include "djlib/vsnprntf.c"
#endif	/* __DJGPP_MINOR__ < 4 */

int q_vsnprintf(char *str, size_t size, const char *format, va_list args)
{
	int		ret;

	ret = vsnprintf_func (str, size, format, args);

	if (ret < 0)
		ret = (int)size;
	if (size == 0)	/* no buffer */
		return ret;
	if ((size_t)ret >= size)
		str[size - 1] = '\0';

	return ret;
}

int q_snprintf (char *str, size_t size, const char *format, ...)
{
	int		ret;
	va_list		argptr;

	va_start (argptr, format);
	ret = q_vsnprintf (str, size, format, argptr);
	va_end (argptr);

	return ret;
}

