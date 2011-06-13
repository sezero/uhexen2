/*
	vregset.c
	video register-setting interpreter.
	from quake1 source with minor adaptations for uhexen2.

	$Id$

	Copyright (C) 1996-1997  Id Software, Inc.

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

#include <dos.h>

#include "quakedef.h"
#include "vregset.h"


/*
================
VideoRegisterSet
================
*/
void VideoRegisterSet (const int *pregset)
{
	int		port, temp0, temp1, temp2;

	for ( ;; )
	{
		switch (*pregset++)
		{
		case VRS_END:
			return;

		case VRS_BYTE_OUT:
			port = *pregset++;
			outportb (port, *pregset++);
			break;

		case VRS_BYTE_RMW:
			port = *pregset++;
			temp0 = *pregset++;
			temp1 = *pregset++;
			temp2 = inportb (port);
			temp2 &= temp0;
			temp2 |= temp1;
			outportb (port, temp2);
			break;

		case VRS_WORD_OUT:
			port = *pregset++;
			outportb (port, *pregset & 0xFF);
			outportb (port+1, *pregset >> 8);
			pregset++;
			break;

		default:
			Sys_Error ("VideoRegisterSet: Invalid command\n");
		}
	}
}

