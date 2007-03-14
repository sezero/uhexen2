/*
	net_wins.h
	winsock udp driver

	$Id: net_wins.h,v 1.4 2007-03-14 21:03:27 sezero Exp $
*/

#ifndef __NET_WINSOCK_H
#define __NET_WINSOCK_H

int  WINS_Init (void);
void WINS_Shutdown (void);
void WINS_Listen (qboolean state);
int  WINS_OpenSocket (int port);
int  WINS_CloseSocket (int mysocket);
int  WINS_Connect (int mysocket, struct qsockaddr *addr);
int  WINS_CheckNewConnections (void);
int  WINS_Read (int mysocket, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Write (int mysocket, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Broadcast (int mysocket, byte *buf, int len);
char *WINS_AddrToString (struct qsockaddr *addr);
int  WINS_StringToAddr (char *string, struct qsockaddr *addr);
int  WINS_GetSocketAddr (int mysocket, struct qsockaddr *addr);
int  WINS_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  WINS_GetAddrFromName (char *name, struct qsockaddr *addr);
int  WINS_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  WINS_GetSocketPort (struct qsockaddr *addr);
int  WINS_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __NET_WINSOCK_H */

