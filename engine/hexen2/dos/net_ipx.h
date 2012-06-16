/*
 * net_ipx.h -- IPX network driver for dosquake.
 * from quake1 source with minor adaptations for uhexen2.
 * $Id: net_ipx.h,v 1.2 2007-12-21 15:05:23 sezero Exp $
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

#ifndef __net_ipx_h
#define __net_ipx_h

int  IPX_Init (void);
void IPX_Shutdown (void);
void IPX_Listen (qboolean state);
int  IPX_OpenSocket (int port);
int  IPX_CloseSocket (int handle);
int  IPX_Connect (int handle, struct qsockaddr *addr);
int  IPX_CheckNewConnections (void);
int  IPX_Read (int handle, byte *buf, int len, struct qsockaddr *addr);
int  IPX_Write (int handle, byte *buf, int len, struct qsockaddr *addr);
int  IPX_Broadcast (int handle, byte *buf, int len);
const char *IPX_AddrToString (struct qsockaddr *addr);
int  IPX_StringToAddr (const char *string, struct qsockaddr *addr);
int  IPX_GetSocketAddr (int handle, struct qsockaddr *addr);
int  IPX_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  IPX_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  IPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  IPX_GetSocketPort (struct qsockaddr *addr);
int  IPX_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __net_ipx_h */

