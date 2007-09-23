/*
	qsnprint.h
	$Id: qsnprint.h,v 1.2 2007-09-23 18:45:09 sezero Exp $

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

/* snprintf, vsnprintf : always use our versions. */
/* platform dependant (v)snprintf function names: */
#if defined(PLATFORM_WINDOWS)
#define	snprintf_func		_snprintf
#define	vsnprintf_func		_vsnprintf
#else
#define	snprintf_func		snprintf
#define	vsnprintf_func		vsnprintf
#endif

extern int q_snprintf (char *str, size_t size, const char *format, ...) __attribute__((format(printf,3,4)));
extern int q_vsnprintf(char *str, size_t size, const char *format, va_list args);

#endif	/* __Q_SNPRINF_H */

