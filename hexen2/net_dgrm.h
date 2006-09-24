// net_dgrm.h

#ifndef __NET_DATAGRAM_H
#define __NET_DATAGRAM_H

int			Datagram_Init (void);
void		Datagram_Listen (qboolean state);
void		Datagram_SearchForHosts (qboolean xmit);
qsocket_t	*Datagram_Connect (char *host);
qsocket_t	*Datagram_CheckNewConnections (void);
int			Datagram_GetMessage (qsocket_t *sock);
int			Datagram_SendMessage (qsocket_t *sock, sizebuf_t *data);
int			Datagram_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
qboolean	Datagram_CanSendMessage (qsocket_t *sock);
qboolean	Datagram_CanSendUnreliableMessage (qsocket_t *sock);
void		Datagram_Close (qsocket_t *sock);
void		Datagram_Shutdown (void);

#endif	/* __NET_DATAGRAM_H */

