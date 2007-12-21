/*
	net.h
	quake's interface to the networking layer

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Master/net.h,v 1.4 2007-12-21 15:05:23 sezero Exp $
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
extern	netadr_t	net_from;	// address of who sent the packet
extern	sizebuf_t	net_message;

extern	int		net_socket;

void		NET_Init (int port);
void		NET_Shutdown (void);
qboolean	NET_GetPacket (void);
void		NET_SendPacket (int length, void *data, netadr_t to);

qboolean	NET_CompareAdr (netadr_t a, netadr_t b);
qboolean	NET_CompareAdrNoPort (netadr_t a, netadr_t b);
void		NET_CopyAdr (netadr_t *a, netadr_t *b);
const char	*NET_AdrToString (netadr_t a);
const char	*NET_BaseAdrToString (netadr_t a);
qboolean	NET_StringToAdr (const char *s, netadr_t *a);

#endif	/* __H2W_NET_H */

