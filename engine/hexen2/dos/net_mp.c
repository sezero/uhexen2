/*
	net_mpath.c
	MPATH TCP driver: the (Winsock) UDP driver for dosquake
        for use when run from within win95.
	from quake1 source with minor adaptations for uhexen2.

	$Id: net_mp.c,v 1.14 2009-04-28 14:00:34 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#include <dpmi.h>
#include "quakedef.h"

/* these we get from djgpp: */
#define WSLIB_ntohl	ntohl
#define WSLIB_ntohs	ntohs
#define WSLIB_htonl	htonl
#define WSLIB_htons	htons
/* these we have implemented locally: */
#define WSLIB_inet_ntoa	inet_ntoa
#define WSLIB_inet_addr	inet_addr
#include "mpdosock.h"

#define sys_socket_t int	/* messy... */

#include "net_defs.h"

short	flat_selector;

int WSLIB_WSAGetLastError (void);
void WSLIB_sockets_flush (void);

#define MAXHOSTNAMELEN		256

static int	net_acceptsocket = -1;		// socket for fielding new connections
static int	net_controlsocket;
static int	net_broadcastsocket = 0;
//static qboolean ifbcastinit = false;
static struct sockaddr_in broadcastaddr;

static struct in_addr	myAddr;

#include "net_mp.h"


//=============================================================================

int MPATH_Init (void)
{
	struct hostent	*local;
	char	buff[MAXHOSTNAMELEN];
	struct qsockaddr	addr;
	char	*colon;

	if (COM_CheckParm ("-mpath") == 0)
		return -1;

	flat_selector = __dpmi_allocate_ldt_descriptors(1);
	if (flat_selector == -1)
	{
		Con_Printf("MPATH_Init: Can't get flat selector\n");
		return -1;
	}
	if (__dpmi_set_segment_base_address(flat_selector, 0) == -1)
	{
		Con_Printf("MPATH_Init: Can't seg flat base!\n");
		return -1;
	}
	if (__dpmi_set_segment_limit(flat_selector, 0xffffffff) == -1)
	{
		Con_Printf("MPATH_Init: Can't set segment limit\n");
		return -1;
	}

	// determine my name & address
	myAddr.s_addr = WSLIB_htonl(INADDR_LOOPBACK);
	if (WSLIB_gethostname(buff, MAXHOSTNAMELEN) != 0)
	{
		Con_Printf("MPATH_Init: WARNING: gethostname failed.\n");
	}
	else
	{
		buff[MAXHOSTNAMELEN - 1] = 0;
		local = WSLIB_gethostbyname(buff);
		if (local == NULL)
		{
			Con_Printf("MPATH_Init: WARNING: gethostbyname failed.\n");
		}
		else if (local->h_addrtype != AF_INET)
		{
			Con_Printf("MPATH_Init: address from gethostbyname not IPv4\n");
		}
		else
		{
			myAddr = *(struct in_addr *)local->h_addr_list[0];
		}
	}
	Con_Printf("UDP, Local address: %s\n", WSLIB_inet_ntoa(myAddr));

	if ((net_controlsocket = MPATH_OpenSocket(0)) == -1)
		Sys_Error("MPATH_Init: Unable to open control socket\n");

	broadcastaddr.sin_family = AF_INET;
	broadcastaddr.sin_addr.s_addr = INADDR_BROADCAST;
	broadcastaddr.sin_port = WSLIB_htons((unsigned short)net_hostport);

	MPATH_GetSocketAddr (net_controlsocket, &addr);
	strcpy(my_tcpip_address,  MPATH_AddrToString (&addr));
	colon = strrchr (my_tcpip_address, ':');
	if (colon)
		*colon = 0;

	Con_Printf("MPath Initialized\n");
	tcpipAvailable = true;

	return net_controlsocket;
}

//=============================================================================

void MPATH_Shutdown (void)
{
	MPATH_Listen (false);
	MPATH_CloseSocket (net_controlsocket);
}

//=============================================================================

void MPATH_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != -1)
			return;
		if ((net_acceptsocket = MPATH_OpenSocket (net_hostport)) == -1)
			Sys_Error ("MPATH_Listen: Unable to open accept socket\n");
		return;
	}

	// disable listening
	if (net_acceptsocket == -1)
		return;
	MPATH_CloseSocket (net_acceptsocket);
	net_acceptsocket = -1;
}

//=============================================================================

int MPATH_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	u_long _true = 1;

	if ((newsocket = WSLIB_socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return -1;

	if (WSLIB_ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		goto ErrorReturn;

	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = WSLIB_htons((unsigned short)port);
	if ( WSLIB_bind (newsocket, (struct sockaddr *)&address, sizeof(address)) == -1)
		goto ErrorReturn;

	return newsocket;

ErrorReturn:
	WSLIB_closesocket (newsocket);
	return -1;
}

//=============================================================================

int MPATH_CloseSocket (int socketid)
{
	if (socketid == net_broadcastsocket)
		net_broadcastsocket = 0;
	return WSLIB_closesocket (socketid);
}

//=============================================================================

/*
============
PartialIPAddress

this lets you type only as much of the net address as required, using
the local network components to fill in the rest
============
*/
static int PartialIPAddress (const char *in, struct qsockaddr *hostaddr)
{
	char	buff[256];
	char	*b;
	int	addr, mask, num, port, run;

	buff[0] = '.';
	b = buff;
	strcpy(buff+1, in);
	if (buff[1] == '.')
		b++;

	addr = 0;
	mask = -1;
	while (*b == '.')
	{
		b++;
		num = 0;
		run = 0;
		while (!( *b < '0' || *b > '9'))
		{
			num = num*10 + *b++ - '0';
			if (++run > 3)
				return -1;
		}
		if ((*b < '0' || *b > '9') && *b != '.' && *b != ':' && *b != 0)
			return -1;
		if (num < 0 || num > 255)
			return -1;
		mask <<= 8;
		addr = (addr<<8) + num;
	}

	if (*b++ == ':')
		port = atoi(b);
	else
		port = net_hostport;

	hostaddr->qsa_family = AF_INET;
	((struct sockaddr_in *)hostaddr)->sin_port = WSLIB_htons((unsigned short)port);
	((struct sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr.s_addr & WSLIB_htonl(mask)) | WSLIB_htonl(addr);

	return 0;
}

//=============================================================================

int MPATH_Connect (int socketid, struct qsockaddr *addr)
{
	return 0;
}

//=============================================================================

int MPATH_CheckNewConnections (void)
{
	char		buf[4];

	if (net_acceptsocket == -1)
		return -1;

	if (WSLIB_recvfrom (net_acceptsocket, buf, 4, MSG_PEEK, NULL, NULL) >= 0)
		return net_acceptsocket;
	return -1;
}

//=============================================================================

int MPATH_Read (int socketid, byte *buf, int len, struct qsockaddr *addr)
{
	int addrlen = sizeof(struct qsockaddr);
	int ret;

	ret = WSLIB_recvfrom (socketid, (char *)buf, len, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == -1)
	{
		int err = WSLIB_WSAGetLastError();

		if (err == WSAEWOULDBLOCK || err == WSAECONNREFUSED)
			return 0;
	}
	return ret;
}

//=============================================================================

int MPATH_MakeSocketBroadcastCapable (int socketid)
{
	int	i = 1;

	// make this socket broadcast capable
	if (WSLIB_setsockopt(socketid, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) < 0)
		return -1;
	net_broadcastsocket = socketid;

	return 0;
}

//=============================================================================

int MPATH_Broadcast (int socketid, byte *buf, int len)
{
	int ret;

	if (socketid != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets\n");
		ret = MPATH_MakeSocketBroadcastCapable (socketid);
		if (ret == -1)
		{
			Con_Printf("Unable to make socket broadcast capable\n");
			return ret;
		}
	}

	return MPATH_Write (socketid, buf, len, (struct qsockaddr *)&broadcastaddr);
}

//=============================================================================

int MPATH_Write (int socketid, byte *buf, int len, struct qsockaddr *addr)
{
	int	ret;

	ret = WSLIB_sendto (socketid, (char *)buf, len, 0, (struct sockaddr *)addr, sizeof(struct qsockaddr));
	if (ret == -1)
	{
		if (WSLIB_WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;
	}

	WSLIB_sockets_flush();

	return ret;
}

//=============================================================================

const char *MPATH_AddrToString (struct qsockaddr *addr)
{
	static char buffer[22];
	int		haddr;

	haddr = WSLIB_ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
	q_snprintf (buffer, sizeof(buffer), "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
			  (haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff,
			  WSLIB_ntohs(((struct sockaddr_in *)addr)->sin_port));
	return buffer;
}

//=============================================================================

int MPATH_StringToAddr (const char *string, struct qsockaddr *addr)
{
	int	ha1, ha2, ha3, ha4, hp, ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->qsa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_addr.s_addr = WSLIB_htonl(ipaddr);
	((struct sockaddr_in *)addr)->sin_port = WSLIB_htons((unsigned short)hp);
	return 0;
}

//=============================================================================

int MPATH_GetSocketAddr (int socketid, struct qsockaddr *addr)
{
	int addrlen = sizeof(struct qsockaddr);
	struct sockaddr_in *address = (struct sockaddr_in *)addr;
	struct in_addr	a;

	memset(addr, 0, sizeof(struct qsockaddr));
	WSLIB_getsockname(socketid, (struct sockaddr *)addr, &addrlen);
	a = address->sin_addr;
	if (a.s_addr == 0 || a.s_addr == WSLIB_htonl(INADDR_LOOPBACK))
		address->sin_addr.s_addr = myAddr.s_addr;

	return 0;
}

//=============================================================================

int MPATH_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	struct hostent *hostentry;

	hostentry = WSLIB_gethostbyaddr ((char *)&((struct sockaddr_in *)addr)->sin_addr, sizeof(struct in_addr), AF_INET);
	if (hostentry)
	{
		strncpy (name, (char *)hostentry->h_name, NET_NAMELEN - 1);
		return 0;
	}

	strcpy (name, MPATH_AddrToString (addr));
	return 0;
}

//=============================================================================

int MPATH_GetAddrFromName (const char *name, struct qsockaddr *addr)
{
	struct hostent *hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress (name, addr);

	hostentry = WSLIB_gethostbyname (name);
	if (!hostentry)
		return -1;

	addr->qsa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_port = WSLIB_htons((unsigned short)net_hostport);
	((struct sockaddr_in *)addr)->sin_addr.s_addr = *(u_long *)hostentry->h_addr_list[0];

	return 0;
}

//=============================================================================

int MPATH_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->qsa_family != addr2->qsa_family)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_addr.s_addr != ((struct sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_port != ((struct sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
}

//=============================================================================

int MPATH_GetSocketPort (struct qsockaddr *addr)
{
	return WSLIB_ntohs(((struct sockaddr_in *)addr)->sin_port);
}


int MPATH_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_in *)addr)->sin_port = WSLIB_htons((unsigned short)port);
	return 0;
}

//=============================================================================

