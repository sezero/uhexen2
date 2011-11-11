/*
	net.h
	quake's interface to the networking layer

	$Header: /cvsroot/uhexen2/hw_utils/hwmaster/net.h,v 1.6 2009-04-29 07:49:28 sezero Exp $
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
void		NET_SendPacket (int length, void *data, netadr_t to);
int		NET_CheckReadTimeout (long sec, long usec);

qboolean	NET_CompareAdr (netadr_t a, netadr_t b);
qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b);	// without port
const char	*NET_AdrToString (netadr_t a);
const char	*NET_BaseAdrToString (netadr_t a);
qboolean	NET_StringToAdr (const char *s, netadr_t *a);

#endif	/* __H2W_NET_H */

