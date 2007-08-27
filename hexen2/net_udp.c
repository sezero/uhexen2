/*
	net_udp.c
	$Id: net_udp.c,v 1.32 2007-08-27 09:21:24 sezero Exp $

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


#include "net_sys.h"
#include <net/if.h>
#include "quakedef.h"

static int net_acceptsocket = -1;	// socket for fielding new connections
static int net_controlsocket;
static int net_broadcastsocket = 0;
static struct qsockaddr broadcastaddr;

static char		ifname[IFNAMSIZ];
static struct in_addr	myAddr,		// the local address returned by the OS.
			localAddr,	// address to advertise by embedding in
					// CCREP_SERVER_INFO and CCREP_ACCEPT
					// response packets instead of the default
					// returned by the OS. from command line
					// argument -localip <ip_address>, used
					// by GetSocketAddr()
			bindAddr;	// the address that we bind to instead of
					// INADDR_ANY. from the command line args
					// -ip <ip_address>

#include "net_udp.h"

//=============================================================================

static int UDP_GetLocalAddress (int sock)
{
	struct ifconf	ifc;
	struct ifreq	*ifr;
	char		buf[8192];
	int		i, n;
	struct sockaddr_in	*iaddr;
	struct in_addr		addr;

	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;

	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
		return -1;

	ifr = ifc.ifc_req;
	n = ifc.ifc_len / sizeof(struct ifreq);

	for (i = 0; i < n; i++)
	{
		if (ioctl(sock, SIOCGIFADDR, &ifr[i]) == -1)
			continue;
		iaddr = (struct sockaddr_in *)&ifr[i].ifr_addr;
		Con_SafeDPrintf("%s: %s\n", ifr[i].ifr_name, inet_ntoa(iaddr->sin_addr));
		addr.s_addr = iaddr->sin_addr.s_addr;
		if (addr.s_addr != htonl(INADDR_LOOPBACK))
		{
			myAddr.s_addr = addr.s_addr;
			strcpy (ifname, ifr[i].ifr_name);
			return 0;
		}
	}

	return -1;
}

int UDP_Init (void)
{
	int		i;
	char	*colon;
	char	buff[MAXHOSTNAMELEN];
	struct hostent		*local;
	struct qsockaddr	addr;

	if (COM_CheckParm ("-noudp"))
		return -1;

	// determine my name & address
	if (gethostname(buff, MAXHOSTNAMELEN) != 0)
	{
		Con_SafePrintf("%s: gethostname failed, UDP disabled (errno: %i)\n", __thisfunc__, errno);
		return -1;
	}
	buff[MAXHOSTNAMELEN-1] = 0;

	local = gethostbyname(buff);

	if (local == NULL)
	{
		Con_SafePrintf("%s: gethostbyname failed, UDP disabled (errno: %i)\n", __thisfunc__, h_errno);
		return -1;
	}

	// if the quake hostname isn't set, set it to the machine name
	if (strcmp(hostname.string, "UNNAMED") == 0)
	{
		buff[15] = 0;
		Cvar_Set("hostname", buff);
	}

	myAddr = *(struct in_addr *)local->h_addr_list[0];

	// check for interface binding option
	i = COM_CheckParm("-ip");
	if (!i)
		i = COM_CheckParm("-bindip");
	if (i && i < com_argc-1)
	{
		bindAddr.s_addr = inet_addr(com_argv[i+1]);
		if (bindAddr.s_addr == INADDR_NONE)
			Sys_Error("%s: %s is not a valid IP address", __thisfunc__, com_argv[i+1]);
		Con_SafePrintf("Binding to IP Interface Address of %s\n", com_argv[i+1]);
	}
	else
	{
		bindAddr.s_addr = INADDR_NONE;
	}

	// check for ip advertise option
	i = COM_CheckParm("-localip");
	if (i && i < com_argc-1)
	{
		localAddr.s_addr = inet_addr(com_argv[i+1]);
		if (localAddr.s_addr == INADDR_NONE)
			Sys_Error("%s: %s is not a valid IP address", __thisfunc__, com_argv[i+1]);
		Con_SafePrintf("Advertising %s as the local IP in response packets\n", com_argv[i+1]);
	}
	else
	{
		localAddr.s_addr = INADDR_NONE;
	}

	if ((net_controlsocket = UDP_OpenSocket (0)) == -1)
	{
		Con_SafePrintf("%s: Unable to open control socket, UDP disabled\n", __thisfunc__);
		return -1;
	}

	memset (ifname, 0, sizeof(ifname));
	if (bindAddr.s_addr == INADDR_NONE && localAddr.s_addr == INADDR_NONE)
	{
	// myAddr may resolve to 127.0.0.1, see if we can do any better
		if (UDP_GetLocalAddress(net_controlsocket) == 0)
			Con_SafePrintf ("Local address: %s (%s)\n", inet_ntoa(myAddr), ifname);
	}

	((struct sockaddr_in *)&broadcastaddr)->sin_family = AF_INET;
	((struct sockaddr_in *)&broadcastaddr)->sin_addr.s_addr = INADDR_BROADCAST;
	((struct sockaddr_in *)&broadcastaddr)->sin_port = htons((unsigned short)net_hostport);

	UDP_GetSocketAddr (net_controlsocket, &addr);
	strcpy(my_tcpip_address,  UDP_AddrToString (&addr));
	colon = strrchr (my_tcpip_address, ':');
	if (colon)
		*colon = 0;

	Con_SafePrintf("UDP Initialized\n");
	tcpipAvailable = true;

	return net_controlsocket;
}

//=============================================================================

void UDP_Shutdown (void)
{
	UDP_Listen (false);
	UDP_CloseSocket (net_controlsocket);
}

//=============================================================================

void UDP_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != -1)
			return;
		if ((net_acceptsocket = UDP_OpenSocket (net_hostport)) == -1)
			Sys_Error ("%s: Unable to open accept socket", __thisfunc__);
		return;
	}

	// disable listening
	if (net_acceptsocket == -1)
		return;
	UDP_CloseSocket (net_acceptsocket);
	net_acceptsocket = -1;
}

//=============================================================================

int UDP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	int _true = 1;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return -1;

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		goto ErrorReturn;

	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	if (bindAddr.s_addr != INADDR_NONE)
		address.sin_addr.s_addr = bindAddr.s_addr;
	else
		address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)port);
	if ( bind (newsocket, (struct sockaddr *)&address, sizeof(address)) == -1)
		goto ErrorReturn;

	return newsocket;

ErrorReturn:
	UDP_CloseSocket (newsocket);
	return -1;
}

//=============================================================================

int UDP_CloseSocket (int mysocket)
{
	if (mysocket == net_broadcastsocket)
		net_broadcastsocket = 0;
	return closesocket (mysocket);
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

	hostaddr->sa_family = AF_INET;
	((struct sockaddr_in *)hostaddr)->sin_port = htons((unsigned short)port);
	((struct sockaddr_in *)hostaddr)->sin_addr.s_addr = (myAddr.s_addr & htonl(mask)) | htonl(addr);

	return 0;
}

//=============================================================================

int UDP_Connect (int mysocket, struct qsockaddr *addr)
{
	return 0;
}

//=============================================================================

int UDP_CheckNewConnections (void)
{
#if defined(__MORPHOS__)
	char		buf[4096];

	if (net_acceptsocket == -1)
		return -1;

	if (recvfrom (net_acceptsocket, buf, sizeof(buf), MSG_PEEK, NULL, NULL) >= 0)
	{
		return net_acceptsocket;
	}
#else
	unsigned long	available;
	struct sockaddr_in	from;
	socklen_t	fromlen;
	char		buff[1];

	if (net_acceptsocket == -1)
		return -1;

	if (ioctl (net_acceptsocket, FIONREAD, &available) == -1)
		Sys_Error ("UDP: ioctlsocket (FIONREAD) failed");
	if (available)
		return net_acceptsocket;
	// quietly absorb empty packets
	recvfrom (net_acceptsocket, buff, 0, 0, (struct sockaddr *) &from, &fromlen);
#endif
	return -1;
}

//=============================================================================

int UDP_Read (int mysocket, byte *buf, int len, struct qsockaddr *addr)
{
	socklen_t addrlen = sizeof (struct qsockaddr);
	int ret;

	ret = recvfrom (mysocket, buf, len, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == -1 && (errno == EWOULDBLOCK || errno == ECONNREFUSED))
		return 0;
	return ret;
}

//=============================================================================

static int UDP_MakeSocketBroadcastCapable (int mysocket)
{
	int	i = 1;

	// make this socket broadcast capable
	if (setsockopt(mysocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) < 0)
		return -1;
	net_broadcastsocket = mysocket;

	return 0;
}

//=============================================================================

int UDP_Broadcast (int mysocket, byte *buf, int len)
{
	int	ret;

	if (mysocket != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets");
		ret = UDP_MakeSocketBroadcastCapable (mysocket);
		if (ret == -1)
		{
			Con_Printf("Unable to make socket broadcast capable\n");
			return ret;
		}
	}

	return UDP_Write (mysocket, buf, len, &broadcastaddr);
}

//=============================================================================

int UDP_Write (int mysocket, byte *buf, int len, struct qsockaddr *addr)
{
	int	ret;

	ret = sendto (mysocket, buf, len, 0, (struct sockaddr *)addr, sizeof(struct qsockaddr));
	if (ret == -1 && errno == EWOULDBLOCK)
		return 0;
	return ret;
}

//=============================================================================

char *UDP_AddrToString (struct qsockaddr *addr)
{
	static char buffer[22];
	int		haddr;

	haddr = ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
	snprintf (buffer, sizeof(buffer), "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff, (haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff, ntohs(((struct sockaddr_in *)addr)->sin_port));
	return buffer;
}

//=============================================================================

int UDP_StringToAddr (const char *string, struct qsockaddr *addr)
{
	int	ha1, ha2, ha3, ha4, hp, ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)hp);
	return 0;
}

//=============================================================================

int UDP_GetSocketAddr (int mysocket, struct qsockaddr *addr)
{
	socklen_t addrlen = sizeof(struct qsockaddr);
	struct sockaddr_in *address = (struct sockaddr_in *)addr;
	struct in_addr	a;

	memset(addr, 0, sizeof(struct qsockaddr));
	getsockname(mysocket, (struct sockaddr *)addr, &addrlen);

	/*
	 * The returned IP is embedded in our repsonse to a broadcast
	 * request for server info from clients.  If the server admin
	 * wishes to advertise a specific IP, then allow the "default"
	 * address returned by the OS to be overridden.
	 */
	if (localAddr.s_addr != INADDR_NONE)
		address->sin_addr.s_addr = localAddr.s_addr;
	else
	{
		a = address->sin_addr;
		if (a.s_addr == 0 || a.s_addr == htonl(INADDR_LOOPBACK))
			address->sin_addr.s_addr = myAddr.s_addr;
	}

	return 0;
}

//=============================================================================

int UDP_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	struct hostent *hostentry;

	hostentry = gethostbyaddr ((char *)&((struct sockaddr_in *)addr)->sin_addr, sizeof(struct in_addr), AF_INET);
	if (hostentry)
	{
		strncpy (name, (char *)hostentry->h_name, NET_NAMELEN - 1);
		return 0;
	}

	strcpy (name, UDP_AddrToString (addr));
	return 0;
}

//=============================================================================

int UDP_GetAddrFromName (const char *name, struct qsockaddr *addr)
{
	struct hostent *hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress (name, addr);

	hostentry = gethostbyname (name);
	if (!hostentry)
		return -1;

	addr->sa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)net_hostport);
	((struct sockaddr_in *)addr)->sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];

	return 0;
}

//=============================================================================

int UDP_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_addr.s_addr != ((struct sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_port != ((struct sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
}

//=============================================================================

int UDP_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_in *)addr)->sin_port);
}


int UDP_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)port);
	return 0;
}

//=============================================================================

