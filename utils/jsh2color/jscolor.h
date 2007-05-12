/*
	jscolor.h
	$Id: jscolor.h,v 1.1 2007-05-12 09:56:35 sezero Exp $

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

#ifndef __JSCOLOR_H
#define __JSCOLOR_H

extern	miptex_t	miptex[512];
extern	int		numlighttex;
extern	int		faces_ltoffset[MAX_MAP_FACES];
extern	byte		newdlightdata[MAX_MAP_LIGHTING];
extern	int		newlightdatasize;

void Init_JSColor (void);

#endif	/* __JSCOLOR_H */

