// net_vcr.h

// VCR facility as a lan driver:
// The "-record" commandline argument creates a file named
// quake.vcr and records the game. This is NOT an ordinary
// demo recording, but a server only feature for debugging
// server bugs.

// COMPILE TIME OPTION: If you want to use the vcr feature
// regardless of building a debug version, change the below
// definition to 1. Four debug builds, it will be set to 1
// down below.
#define	NET_USE_VCR					0

#ifdef	DEBUG_BUILD
#undef	NET_USE_VCR
#define	NET_USE_VCR					1
#endif

#define VCR_OP_CONNECT					1
#define VCR_OP_GETMESSAGE				2
#define VCR_OP_SENDMESSAGE				3
#define VCR_OP_CANSENDMESSAGE				4
#define VCR_MAX_MESSAGE					4

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

