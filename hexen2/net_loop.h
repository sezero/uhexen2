// net_loop.h

int			Loop_Init (void);
void		Loop_Listen (qboolean state);
void		Loop_SearchForHosts (qboolean xmit);
qsocket_t 	*Loop_Connect (char *host);
qsocket_t 	*Loop_CheckNewConnections (void);
int			Loop_GetMessage (qsocket_t *sock);
int			Loop_SendMessage (qsocket_t *sock, sizebuf_t *data);
int			Loop_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
qboolean	Loop_CanSendMessage (qsocket_t *sock);
qboolean	Loop_CanSendUnreliableMessage (qsocket_t *sock);
void		Loop_Close (qsocket_t *sock);
void		Loop_Shutdown (void);
