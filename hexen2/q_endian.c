/*
	q_endian.c
	byte order functions

	$Id: q_endian.c,v 1.7 2007-12-14 16:41:10 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2007  O.Sezer <sezero@users.sourceforge.net>

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

#include "q_stdinc.h"
#include "q_endian.h"

#if defined(ENDIAN_ASSUMED_UNSAFE)
# if (ENDIAN_ASSUMED_UNSAFE == LITTLE_ENDIAN)
#    warning "Cannot determine endianess. Using LIL endian as an UNSAFE default"
# elif (ENDIAN_ASSUMED_UNSAFE == PDP_ENDIAN)
#    warning "Cannot determine endianess. Using PDP (NUXI) as an UNSAFE default."
# elif (ENDIAN_ASSUMED_UNSAFE == BIG_ENDIAN)
#    warning "Cannot determine endianess. Using BIG endian as an UNSAFE default."
# endif
#endif	/* ENDIAN_ASSUMED_UNSAFE */


int DetectByteorder (void)
{
	int	i = 0x12345678;
		/*    U N I X */

	/*
	BE_ORDER:  12 34 56 78
		   U  N  I  X

	LE_ORDER:  78 56 34 12
		   X  I  N  U

	PDP_ORDER: 34 12 78 56
		   N  U  X  I
	*/

	if ( *(char *)&i == 0x12 )
		return BIG_ENDIAN;
	else if ( *(char *)&i == 0x78 )
		return LITTLE_ENDIAN;
	else if ( *(char *)&i == 0x34 )
		return PDP_ENDIAN;

	return -1;
}

short ShortSwap (short l)
{
	unsigned char	b1, b2;

	b1 = l & 255;
	b2 = (l>>8) & 255;

	return (b1<<8) + b2;
}

int LongSwap (int l)
{
	unsigned char	b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l>>8 ) & 255;
	b3 = (l>>16) & 255;
	b4 = (l>>24) & 255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		unsigned char	b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

int LongSwapPDP2BE (int l)
{
	union
	{
		int	l;
		unsigned char	b[4];
	} dat1, dat2;

	dat1.l = l;
	dat2.b[0] = dat1.b[1];
	dat2.b[1] = dat1.b[0];
	dat2.b[2] = dat1.b[3];
	dat2.b[3] = dat1.b[2];

	return dat2.l;
}

int LongSwapPDP2LE (int l)
{
	union
	{
		int	l;
		short	s[2];
	} dat1, dat2;

	dat1.l = l;
	dat2.s[0] = dat1.s[1];
	dat2.s[1] = dat1.s[0];

	return dat2.l;
}

float FloatSwapPDP2BE (float f)
{
	union
	{
		float	f;
		unsigned char	b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[1];
	dat2.b[1] = dat1.b[0];
	dat2.b[2] = dat1.b[3];
	dat2.b[3] = dat1.b[2];

	return dat2.f;
}

float FloatSwapPDP2LE (float f)
{
	union
	{
		float	f;
		short	s[2];
	} dat1, dat2;

	dat1.f = f;
	dat2.s[0] = dat1.s[1];
	dat2.s[1] = dat1.s[0];

	return dat2.f;
}

