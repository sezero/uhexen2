/*
	qsnprint.c
	$Id: qsnprint.c,v 1.1 2007-09-22 15:30:22 sezero Exp $

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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "arch_def.h"
#include "compiler.h"
#include "qsnprint.h"

#if defined(SNPRINTF_RETURNS_NEGATIVE) || defined(SNPRINTF_DOESNT_TERMINATE)
int q_vsnprintf(char *str, size_t size, const char *format, va_list args)
{
	int		ret;

	ret = vsnprintf_func (str, size, format, args);
# if defined(SNPRINTF_RETURNS_NEGATIVE)
	if (ret < 0)
		ret = (int)size;
# endif
# if defined(SNPRINTF_DOESNT_TERMINATE)
	if (ret >= (int)size)
		str[size - 1] = '\0';
# endif
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
#endif	/* SNPRINTF_RETURNS_NEGATIVE || SNPRINTF_DOESNT_TERMINATE */

