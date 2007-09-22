/*
	qsnprint.h
	$Id: qsnprint.h,v 1.1 2007-09-22 15:30:21 sezero Exp $

	(v)snprintf wrappers
	Copyright (C) 2007 O. Sezer <sezero@users.sourceforge.net>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#ifndef __Q_SNPRINF_H
#define __Q_SNPRINF_H

/* snprintf and vsnprintf : */
#undef	SNPRINTF_RETURNS_NEGATIVE
#undef	SNPRINTF_DOESNT_TERMINATE

/* platforms where (v)snprintf implementations return
   a negative value upon error: DOS (DJGPP v2.0.4) and
   Windows does that. Add more here. */
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_DOS)
#define	SNPRINTF_RETURNS_NEGATIVE	1
#endif

/* platform where (v)snprintf implementations may not
   null-terminate the given buffer upon truncation :
   Windows does that. Add more here. */
#if defined(PLATFORM_WINDOWS)
#define	SNPRINTF_DOESNT_TERMINATE	1
#endif

/* platform dependant (v)snprintf function names: */
#if defined(PLATFORM_WINDOWS)
#define	snprintf_func		_snprintf
#define	vsnprintf_func		_vsnprintf
#else
#define	snprintf_func		snprintf
#define	vsnprintf_func		vsnprintf
#endif

#if defined(SNPRINTF_RETURNS_NEGATIVE) || defined(SNPRINTF_DOESNT_TERMINATE)
extern int q_snprintf (char *str, size_t size, const char *format, ...) __attribute__((format(printf,3,4)));
extern int q_vsnprintf(char *str, size_t size, const char *format, va_list args);
#else
#define	q_snprintf		snprintf_func
#define	q_vsnprintf		vsnprintf_func
#endif

#endif	/* __Q_SNPRINF_H */

