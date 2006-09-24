// net_vcr.h

#ifndef __NET_VCR_H
#define __NET_VCR_H

// VCR facility as a lan driver:
// The "-record" commandline argument creates a file named
// quake.vcr and records the game. This is NOT an ordinary
// demo recording, but a server only feature for debugging
// server bugs.

#define VCR_OP_CONNECT					1
#define VCR_OP_GETMESSAGE				2
#define VCR_OP_SENDMESSAGE				3
#define VCR_OP_CANSENDMESSAGE				4
#define VCR_MAX_MESSAGE					4

#if NET_USE_VCR

int			VCR_Init (void);
void		VCR_Listen (qboolean state);
void		VCR_SearchForHosts (qboolean xmit);
qsocket_t	*VCR_Connect (char *host);
qsocket_t	*VCR_CheckNewConnections (void);
int			VCR_GetMessage (qsocket_t *sock);
int			VCR_SendMessage (qsocket_t *sock, sizebuf_t *data);
qboolean	VCR_CanSendMessage (qsocket_t *sock);
void		VCR_Close (qsocket_t *sock);
void		VCR_Shutdown (void);

#endif	// NET_USE_VCR

#endif	/* __NET_VCR_H */

