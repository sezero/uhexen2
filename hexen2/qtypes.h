/*
	qtypes.h
	$Id: qtypes.h,v 1.2 2004-12-12 14:14:42 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 1999,2000  contributors of the QuakeForge project
	Please see the file "AUTHORS" for a list of contributors

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
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#ifndef __qtypes
#define __qtypes

#define MAX_QPATH	64

#ifndef _DEF_BYTE_
# define _DEF_BYTE_
typedef unsigned char byte;
#endif

// KJB Undefined true and false defined in SciTech's DEBUG.H header
#undef true
#undef false
//typedef	enum	{false, true} qboolean;

// From mathlib...
typedef float	vec_t;
typedef vec_t	vec3_t[3];
typedef vec_t	vec5_t[5];
typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;


typedef	int	func_t;
typedef	int	string_t;
typedef	byte	pixel_t;

#endif // __qtypes
