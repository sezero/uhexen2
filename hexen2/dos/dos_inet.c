/*
	dos_inet.c
	from quake1 source with minor adaptations for uhexen2.

	$Id: dos_inet.c,v 1.1 2007-10-22 18:07:51 sezero Exp $

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

#include <stdio.h>
#include <stdlib.h>
#include <dpmi.h>

#include "dos_inet.h"


in_addr_t inet_addr (const char *cp)
{
	int	ret;
	unsigned int	ha1, ha2, ha3, ha4;
	in_addr_t	ipaddr;

	ret = sscanf(cp, "%u.%u.%u.%u", &ha1, &ha2, &ha3, &ha4);
	if (ret != 4)
		return INADDR_NONE;
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;
	return ipaddr;
}

char *inet_ntoa (struct in_addr in)
{
	static char buf[32];

	sprintf(buf, "%u.%u.%u.%u", in.S_un.S_un_b.s_b1, in.S_un.S_un_b.s_b2, in.S_un.S_un_b.s_b3, in.S_un.S_un_b.s_b4);
	return buf;
}

