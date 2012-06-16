/*
 * info_str.h -- Hexen2World info strings handling
 * $Id: info_str.h,v 1.4 2007-11-11 13:17:44 sezero Exp $
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

#ifndef __H2W_INFOSTR_H
#define __H2W_INFOSTR_H

#define	MAX_INFO_STRING			196
#define	MAX_SERVERINFO_STRING		512
#define	MAX_LOCALINFO_STRING		32768

// none of these functions accept NULL input

const char *Info_ValueForKey (const char *s, const char *key);	// may return an empty string, but never NULL
void Info_RemoveKey (char *s, const char *key);
void Info_RemovePrefixedKeys (char *start, char prefix);
void Info_SetValueForKey (char *s, const char *key, const char *value, size_t maxsize);
void Info_SetValueForStarKey (char *s, const char *key, const char *value, size_t maxsize);
void Info_Print (const char *s);

#endif	/* __H2W_INFOSTR_H */

