/*
 * host_string.h --
 * internationalized string resource shared between client and server
 * $Id$
 *
 * Copyright (C) 1997-1998 Raven Software Corp.
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

#ifndef HOST_STRING_H
#define HOST_STRING_H

extern	int		host_string_count;

void Host_LoadStrings (void);
const char *Host_GetString (int idx);

#endif	/* HOST_STRING_H */

