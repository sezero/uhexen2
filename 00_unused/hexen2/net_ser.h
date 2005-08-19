// net_ser.h

int			Serial_Init (void);
void		Serial_Listen (qboolean state);
void		Serial_SearchForHosts (qboolean xmit);
qsocket_t	*Serial_Connect (char *host);
qsocket_t 	*Serial_CheckNewConnections (void);
int			Serial_GetMessage (qsocket_t *sock);
int			Serial_SendMessage (qsocket_t *sock, sizebuf_t *data);
int			Serial_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
qboolean	Serial_CanSendMessage (qsocket_t *sock);
qboolean	Serial_CanSendUnreliableMessage (qsocket_t *sock);
void		Serial_Close (qsocket_t *sock);
void		Serial_Shutdown (void);
