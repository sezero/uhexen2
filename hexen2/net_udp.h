/*
	net_udp.h
	$Id: net_udp.h,v 1.8 2009-01-08 12:01:50 sezero Exp $

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

#ifndef __net_udp_h
#define __net_udp_h

int  UDP_Init (void);
void UDP_Shutdown (void);
void UDP_Listen (qboolean state);
int  UDP_OpenSocket (int port);
int  UDP_CloseSocket (int socketid);
int  UDP_Connect (int socketid, struct qsockaddr *addr);
int  UDP_CheckNewConnections (void);
int  UDP_Read (int socketid, byte *buf, int len, struct qsockaddr *addr);
int  UDP_Write (int socketid, byte *buf, int len, struct qsockaddr *addr);
int  UDP_Broadcast (int socketid, byte *buf, int len);
const char *UDP_AddrToString (struct qsockaddr *addr);
int  UDP_StringToAddr (const char *string, struct qsockaddr *addr);
int  UDP_GetSocketAddr (int socketid, struct qsockaddr *addr);
int  UDP_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  UDP_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  UDP_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  UDP_GetSocketPort (struct qsockaddr *addr);
int  UDP_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __net_udp_h */

