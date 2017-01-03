/*
 * qsnprint.h
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

#ifndef __Q_SNPRINF_H
#define __Q_SNPRINF_H

#ifdef __cplusplus
extern "C" {
#endif
extern int q_snprintf (char *str, size_t size, const char *format, ...) FUNC_PRINTF(3,4);
extern int q_vsnprintf(char *str, size_t size, const char *format, va_list args) FUNC_PRINTF(3,0);
#ifdef __cplusplus
}
#endif

#endif	/* __Q_SNPRINF_H */

