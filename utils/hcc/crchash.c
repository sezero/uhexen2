/*
 * crchash.c
 * hash functions for HCC
 *
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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

#include "crchash.h"
#include "crc.c"

/*
==============
COM_Hash

==============
*/
int COM_Hash (const char *key)
{
	int		i;
	int		length;
	const char	*keyBack;
	unsigned short	hash;

	length = strlen (key);
	keyBack = key + length - 1;
	hash = CRC_INIT_VALUE;

	if (length > 20)
	{
		length = 20;
	}

	for (i = 0; i < length; i++)
	{
		hash = (hash<<8)^crctable[(hash>>8)^*key++];
		if (++i >= length)
		{
			break;
		}
		hash = (hash<<8)^crctable[(hash>>8)^*keyBack--];
	}

	return hash % HASH_TABLE_SIZE;
}

