/*
	net_wins.h
	winsock udp driver

	$Id: net_wins.h,v 1.8 2009-04-28 14:00:34 sezero Exp $
*/

#ifndef __NET_WINSOCK_H
#define __NET_WINSOCK_H

sys_socket_t  WINS_Init (void);
void WINS_Shutdown (void);
void WINS_Listen (qboolean state);
sys_socket_t  WINS_OpenSocket (int port);
int  WINS_CloseSocket (sys_socket_t socketid);
int  WINS_Connect (sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t  WINS_CheckNewConnections (void);
int  WINS_Read (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Write (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int  WINS_Broadcast (sys_socket_t socketid, byte *buf, int len);
const char *WINS_AddrToString (struct qsockaddr *addr);
int  WINS_StringToAddr (const char *string, struct qsockaddr *addr);
int  WINS_GetSocketAddr (sys_socket_t socketid, struct qsockaddr *addr);
int  WINS_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  WINS_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  WINS_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  WINS_GetSocketPort (struct qsockaddr *addr);
int  WINS_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __NET_WINSOCK_H */

