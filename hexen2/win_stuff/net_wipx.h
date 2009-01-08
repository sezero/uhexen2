/*
	net_wipx.h
	winsock ipx driver

	$Id: net_wipx.h,v 1.7 2009-01-08 12:01:51 sezero Exp $
*/

#ifndef __NET_WINIPX_H
#define __NET_WINIPX_H

int  WIPX_Init (void);
void WIPX_Shutdown (void);
void WIPX_Listen (qboolean state);
int  WIPX_OpenSocket (int port);
int  WIPX_CloseSocket (int socketid);
int  WIPX_Connect (int socketid, struct qsockaddr *addr);
int  WIPX_CheckNewConnections (void);
int  WIPX_Read (int socketid, byte *buf, int len, struct qsockaddr *addr);
int  WIPX_Write (int socketid, byte *buf, int len, struct qsockaddr *addr);
int  WIPX_Broadcast (int socketid, byte *buf, int len);
const char *WIPX_AddrToString (struct qsockaddr *addr);
int  WIPX_StringToAddr (const char *string, struct qsockaddr *addr);
int  WIPX_GetSocketAddr (int socketid, struct qsockaddr *addr);
int  WIPX_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  WIPX_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  WIPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  WIPX_GetSocketPort (struct qsockaddr *addr);
int  WIPX_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __NET_WINIPX_H */

