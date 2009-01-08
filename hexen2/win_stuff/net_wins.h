/*
	net_wins.h
	winsock udp driver

	$Id: net_wins.h,v 1.7 2009-01-08 12:01:51 sezero Exp $
*/

#ifndef __NET_WINSOCK_H
#define __NET_WINSOCK_H

int  WINS_Init (void);
void WINS_Shutdown (void);
void WINS_Listen (qboolean state);
int  WINS_OpenSocket (int port);
int  WINS_CloseSocket (int socketid);
int  WINS_Connect (int socketid, struct qsockaddr *addr);
int  WINS_CheckNewConnections (void);
int  WINS_Read (int socketid, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Write (int socketid, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Broadcast (int socketid, byte *buf, int len);
const char *WINS_AddrToString (struct qsockaddr *addr);
int  WINS_StringToAddr (const char *string, struct qsockaddr *addr);
int  WINS_GetSocketAddr (int socketid, struct qsockaddr *addr);
int  WINS_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  WINS_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  WINS_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  WINS_GetSocketPort (struct qsockaddr *addr);
int  WINS_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __NET_WINSOCK_H */

