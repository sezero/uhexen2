/*
	net_bw.h
	Beame & Whiteside TCP/IP for dosquake.
	from quake1 source with minor adaptations for uhexen2.

	$Id: net_bw.h,v 1.2 2007-12-21 15:05:23 sezero Exp $

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

#ifndef __net_bw_h
#define __net_bw_h

int  BW_Init (void);
void BW_Shutdown (void);
void BW_Listen (qboolean state);
int  BW_OpenSocket (int port);
int  BW_CloseSocket (int mysocket);
int  BW_Connect (int mysocket, struct qsockaddr *addr);
int  BW_CheckNewConnections (void);
int  BW_Read (int mysocket, byte *buf, int len, struct qsockaddr *addr);
int  BW_Write (int mysocket, byte *buf, int len, struct qsockaddr *addr);
int  BW_Broadcast (int mysocket, byte *buf, int len);
const char *BW_AddrToString (struct qsockaddr *addr);
int  BW_StringToAddr (const char *string, struct qsockaddr *addr);
int  BW_GetSocketAddr (int mysocket, struct qsockaddr *addr);
int  BW_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  BW_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  BW_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  BW_GetSocketPort (struct qsockaddr *addr);
int  BW_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __net_bw_h */

