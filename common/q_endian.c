/*
 * q_endian.c -- byte order functions
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2007-2016  O.Sezer <sezero@users.sourceforge.net>
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

#include "q_stdinc.h"
#include "q_endian.h"

#if ENDIAN_RUNTIME_DETECT
#define __byteswap_func static
#else
#define __byteswap_func
#endif

int host_byteorder;
int host_bigendian; /* qboolean */

int DetectByteorder (void)
{
	union
	{
		uint32_t i;
		char c[4];
	} bint = {0x12345678};
		/*    U N I X */

	/*
	BE_ORDER:  12 34 56 78
		   U  N  I  X

	LE_ORDER:  78 56 34 12
		   X  I  N  U

	PDP_ORDER: 34 12 78 56
		   N  U  X  I
	*/

	if ( bint.c[0] == 0x12 )
		return BIG_ENDIAN;
	else if ( bint.c[0] == 0x78 )
		return LITTLE_ENDIAN;
	else if ( bint.c[0] == 0x34 )
		return PDP_ENDIAN;

	return -1;
}

#if ENDIAN_RUNTIME_DETECT || !(defined(__VBCC__) && defined(__M68K__))
__byteswap_func
short ShortSwap (short l)
{
	unsigned char	b1, b2;

	b1 = l & 255;
	b2 = (l>>8) & 255;

	return (b1<<8) + b2;
}

__byteswap_func
int LongSwap (int l)
{
	unsigned char	b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l>>8 ) & 255;
	b3 = (l>>16) & 255;
	b4 = (l>>24) & 255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

__byteswap_func
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
#endif

#if ENDIAN_RUNTIME_DETECT

__byteswap_func
short ShortNoSwap (short l)
{
	return l;
}

__byteswap_func
int LongNoSwap (int l)
{
	return l;
}

__byteswap_func
float FloatNoSwap (float f)
{
	return f;
}

short (*BigShort) (short);
short (*LittleShort) (short);
int   (*BigLong) (int);
int   (*LittleLong) (int);
float (*BigFloat) (float);
float (*LittleFloat) (float);

#endif /* ENDIAN_RUNTIME_DETECT */

void ByteOrder_Init (void)
{
	host_byteorder = DetectByteorder ();
	host_bigendian = (host_byteorder == BIG_ENDIAN);

#if ENDIAN_RUNTIME_DETECT
	switch (host_byteorder)
	{
	case BIG_ENDIAN:
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
		break;

	case LITTLE_ENDIAN:
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
		break;

	default:
		break;
	}
#endif /* ENDIAN_RUNTIME_DETECT */
}

