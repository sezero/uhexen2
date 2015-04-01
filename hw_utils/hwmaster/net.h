/*
 * net.h
 * quake's interface to the networking layer
 *
 * $Id: net.h,v 1.6 2009-04-29 07:49:28 sezero Exp $
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

#ifndef __H2W_NET_H
#define __H2W_NET_H

#define	PORT_ANY	-1

typedef struct
{
	byte	ip[4];
	unsigned short	port;
	unsigned short	pad;
} netadr_t;

extern	netadr_t	net_local_adr;
extern	netadr_t	net_loopback_adr;
extern	netadr_t	net_from;	// address of who sent the packet
extern	sizebuf_t	net_message;

void		NET_Init (int port);
void		NET_Shutdown (void);
int		NET_GetPacket (void);
void		NET_SendPacket (int length, void *data, const netadr_t *to);
int		NET_CheckReadTimeout (long sec, long usec);

qboolean	NET_CompareAdr (const netadr_t *a, const netadr_t *b);
qboolean	NET_CompareBaseAdr (const netadr_t *a, const netadr_t *b);	// without port
const char	*NET_AdrToString (const netadr_t *a);
const char	*NET_BaseAdrToString (const netadr_t *a);
qboolean	NET_StringToAdr (const char *s, netadr_t *a);

#endif	/* __H2W_NET_H */

