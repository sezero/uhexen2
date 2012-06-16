/*
 * net_wins.c -- winsock udp driver
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "q_stdinc.h"
#include "arch_def.h"
#include "net_sys.h"
#include "quakedef.h"
#include "net_defs.h"

static sys_socket_t net_acceptsocket = INVALID_SOCKET;	// socket for fielding new connections
static sys_socket_t net_controlsocket;
static sys_socket_t net_broadcastsocket = 0;
static struct sockaddr_in broadcastaddr;

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

#include "net_wins.h"

int winsock_initialized = 0;
WSADATA		winsockdata;
#define __wsaerr_static			/* not static: used by net_wipx.c too */
#include "wsaerror.h"

//=============================================================================

#if !defined(_USE_WINSOCK2)
static double	blocktime;

static INT_PTR PASCAL FAR BlockingHook (void)
{
	MSG	msg;
	BOOL	ret;

	if ((Sys_DoubleTime() - blocktime) > 2.0)
	{
		WSACancelBlockingCall();
		return FALSE;
	}

	/* get the next message, if any */
	ret = (BOOL) PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

	/* if we got one, process it */
	if (ret)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* TRUE if we got a message */
	return ret;
}
#endif	/* ! _USE_WINSOCK2 */

sys_socket_t WINS_Init (void)
{
	int	i, err;
	char	*colon;
	char	buff[MAXHOSTNAMELEN];
	struct hostent		*local;
	struct qsockaddr	addr;

	if (COM_CheckParm ("-noudp") || (winsock_initialized == -1))
		return INVALID_SOCKET;

	if (winsock_initialized == 0)
	{
		err = WSAStartup(MAKEWORD(1,1), &winsockdata);
		if (err != 0)
		{
			winsock_initialized = -1;
			Con_SafePrintf("Winsock initialization failed (%s)\n",
					socketerror(err));
			return INVALID_SOCKET;
		}
	}
	winsock_initialized++;

	// determine my name & address
	myAddr.s_addr = htonl(INADDR_LOOPBACK);
	if (gethostname(buff, MAXHOSTNAMELEN) != 0)
	{
		err = SOCKETERRNO;
		Con_SafePrintf("%s: WARNING: gethostname failed (%s)\n",
					__thisfunc__, socketerror(err));
	}
	else
	{
		buff[MAXHOSTNAMELEN - 1] = 0;
#if !defined(_USE_WINSOCK2)
		blocktime = Sys_DoubleTime();
		WSASetBlockingHook(BlockingHook);
#endif	/* ! _USE_WINSOCK2 */
		local = gethostbyname(buff);
		err = WSAGetLastError();
#if !defined(_USE_WINSOCK2)
		WSAUnhookBlockingHook();
#endif	/* ! _USE_WINSOCK2 */
		if (local == NULL)
		{
			Con_SafePrintf("%s: WARNING: gethostbyname failed (%s)\n",
					__thisfunc__, __WSAE_StrError(err));
		}
		else if (local->h_addrtype != AF_INET)
		{
			Con_SafePrintf("%s: address from gethostbyname not IPv4\n",
					__thisfunc__);
		}
		else
		{
			myAddr = *(struct in_addr *)local->h_addr_list[0];
		}
	}
	Con_SafePrintf("UDP, Local address: %s\n", inet_ntoa(myAddr));

	// check for interface binding option
	i = COM_CheckParm("-ip");
	if (i == 0)
		i = COM_CheckParm("-bindip");
	if (i && i < com_argc - 1)
	{
		bindAddr.s_addr = inet_addr(com_argv[i + 1]);
		if (bindAddr.s_addr == INADDR_NONE)
		{
			Sys_Error("%s: %s is not a valid IP address",
					__thisfunc__, com_argv[i + 1]);
		}
		Con_SafePrintf("Binding to IP Interface Address of %s\n",
							com_argv[i + 1]);
	}
	else
	{
		bindAddr.s_addr = INADDR_NONE;
	}

	// check for ip advertise option
	i = COM_CheckParm("-localip");
	if (i && i < com_argc - 1)
	{
		localAddr.s_addr = inet_addr(com_argv[i + 1]);
		if (localAddr.s_addr == INADDR_NONE)
		{
			Sys_Error("%s: %s is not a valid IP address",
					__thisfunc__, com_argv[i + 1]);
		}
		Con_SafePrintf("Advertising %s as the local IP in response packets\n",
							com_argv[i + 1]);
	}
	else
	{
		localAddr.s_addr = INADDR_NONE;
	}

	if ((net_controlsocket = WINS_OpenSocket(0)) == INVALID_SOCKET)
	{
		Con_SafePrintf("%s: Unable to open control socket, UDP disabled\n",
				__thisfunc__);
		if (--winsock_initialized == 0)
			WSACleanup ();
		return INVALID_SOCKET;
	}

	broadcastaddr.sin_family = AF_INET;
	broadcastaddr.sin_addr.s_addr = INADDR_BROADCAST;
	broadcastaddr.sin_port = htons((unsigned short)net_hostport);

	WINS_GetSocketAddr (net_controlsocket, &addr);
	strcpy(my_tcpip_address, WINS_AddrToString (&addr));
	colon = strrchr (my_tcpip_address, ':');
	if (colon)
		*colon = 0;

	Con_SafePrintf("UDP Initialized\n");
	tcpipAvailable = true;

	return net_controlsocket;
}

//=============================================================================

void WINS_Shutdown (void)
{
	WINS_Listen (false);
	WINS_CloseSocket (net_controlsocket);
	if (--winsock_initialized == 0)
		WSACleanup ();
}

//=============================================================================

void WINS_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != INVALID_SOCKET)
			return;
		if ((net_acceptsocket = WINS_OpenSocket (net_hostport)) == INVALID_SOCKET)
			Sys_Error ("%s: Unable to open accept socket", __thisfunc__);
		return;
	}

	// disable listening
	if (net_acceptsocket == INVALID_SOCKET)
		return;
	WINS_CloseSocket (net_acceptsocket);
	net_acceptsocket = INVALID_SOCKET;
}

//=============================================================================

sys_socket_t WINS_OpenSocket (int port)
{
	sys_socket_t newsocket;
	struct sockaddr_in address;
	u_long _true = 1;
	int err;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		err = SOCKETERRNO;
		Con_SafePrintf("%s: %s\n", __thisfunc__, socketerror(err));
		return INVALID_SOCKET;
	}

	if (ioctlsocket (newsocket, FIONBIO, &_true) == SOCKET_ERROR)
		goto ErrorReturn;

	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	if (bindAddr.s_addr != INADDR_NONE)
		address.sin_addr.s_addr = bindAddr.s_addr;
	else
		address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)port);
	if (bind (newsocket, (struct sockaddr *)&address, sizeof(address)) == 0)
		return newsocket;

	if (tcpipAvailable)
	{
		err = SOCKETERRNO;
		Sys_Error ("Unable to bind to %s (%s)",
				WINS_AddrToString ((struct qsockaddr *) &address),
				socketerror(err));
		return INVALID_SOCKET;	/* not reached */
	}
	/* else: we are still in init phase, no need to error */

ErrorReturn:
	err = SOCKETERRNO;
	Con_SafePrintf("%s: %s\n", __thisfunc__, socketerror(err));
	closesocket (newsocket);
	return INVALID_SOCKET;
}

//=============================================================================

int WINS_CloseSocket (sys_socket_t socketid)
{
	if (socketid == net_broadcastsocket)
		net_broadcastsocket = 0;
	return closesocket (socketid);
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
	((struct sockaddr_in *)hostaddr)->sin_port = htons((unsigned short)port);
	((struct sockaddr_in *)hostaddr)->sin_addr.s_addr =
					(myAddr.s_addr & htonl(mask)) | htonl(addr);

	return 0;
}

//=============================================================================

int WINS_Connect (sys_socket_t socketid, struct qsockaddr *addr)
{
	return 0;
}

//=============================================================================

sys_socket_t WINS_CheckNewConnections (void)
{
	char		buf[4096];

	if (net_acceptsocket == INVALID_SOCKET)
		return INVALID_SOCKET;

	if (recvfrom (net_acceptsocket, buf, sizeof(buf), MSG_PEEK, NULL, NULL)
								!= SOCKET_ERROR)
	{
		return net_acceptsocket;
	}
	return INVALID_SOCKET;
}

//=============================================================================

int WINS_Read (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr)
{
	socklen_t addrlen = sizeof(struct qsockaddr);
	int ret;

	ret = recvfrom (socketid, (char *)buf, len, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		if (err == NET_EWOULDBLOCK || err == NET_ECONNREFUSED)
			return 0;
		Con_SafeDPrintf ("%s, recvfrom: %s\n", __thisfunc__, socketerror(err));
	}
	return ret;
}

//=============================================================================

static int WINS_MakeSocketBroadcastCapable (sys_socket_t socketid)
{
	int	i = 1;

	// make this socket broadcast capable
	if (setsockopt(socketid, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i))
								 == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		Con_SafePrintf ("%s, setsockopt: %s\n", __thisfunc__, socketerror(err));
		return -1;
	}
	net_broadcastsocket = socketid;

	return 0;
}

//=============================================================================

int WINS_Broadcast (sys_socket_t socketid, byte *buf, int len)
{
	int	ret;

	if (socketid != net_broadcastsocket)
	{
		if (net_broadcastsocket != 0)
			Sys_Error("Attempted to use multiple broadcasts sockets");
		ret = WINS_MakeSocketBroadcastCapable (socketid);
		if (ret == -1)
		{
			Con_Printf("Unable to make socket broadcast capable\n");
			return ret;
		}
	}

	return WINS_Write (socketid, buf, len, (struct qsockaddr *)&broadcastaddr);
}

//=============================================================================

int WINS_Write (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr)
{
	int	ret;

	ret = sendto (socketid, (char *)buf, len, 0, (struct sockaddr *)addr,
							sizeof(struct qsockaddr));
	if (ret == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		if (err == NET_EWOULDBLOCK)
			return 0;
		Con_SafeDPrintf ("%s, sendto: %s\n", __thisfunc__, socketerror(err));
	}
	return ret;
}

//=============================================================================

const char *WINS_AddrToString (struct qsockaddr *addr)
{
	static char buffer[22];
	int		haddr;

	haddr = ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
	q_snprintf (buffer, sizeof(buffer), "%d.%d.%d.%d:%d", (haddr >> 24) & 0xff,
			  (haddr >> 16) & 0xff, (haddr >> 8) & 0xff, haddr & 0xff,
			  ntohs(((struct sockaddr_in *)addr)->sin_port));
	return buffer;
}

//=============================================================================

int WINS_StringToAddr (const char *string, struct qsockaddr *addr)
{
	int	ha1, ha2, ha3, ha4, hp, ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->qsa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)hp);
	return 0;
}

//=============================================================================

int WINS_GetSocketAddr (sys_socket_t socketid, struct qsockaddr *addr)
{
	socklen_t addrlen = sizeof(struct qsockaddr);
	struct sockaddr_in *address = (struct sockaddr_in *)addr;
	struct in_addr	a;

	memset(addr, 0, sizeof(struct qsockaddr));
	getsockname(socketid, (struct sockaddr *)addr, &addrlen);

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

int WINS_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	struct hostent *hostentry;

	hostentry = gethostbyaddr ((char *)&((struct sockaddr_in *)addr)->sin_addr,
						sizeof(struct in_addr), AF_INET);
	if (hostentry)
	{
		strncpy (name, (char *)hostentry->h_name, NET_NAMELEN - 1);
		return 0;
	}

	strcpy (name, WINS_AddrToString (addr));
	return 0;
}

//=============================================================================

int WINS_GetAddrFromName (const char *name, struct qsockaddr *addr)
{
	struct hostent *hostentry;

	if (name[0] >= '0' && name[0] <= '9')
		return PartialIPAddress (name, addr);

	hostentry = gethostbyname (name);
	if (!hostentry)
		return -1;

	addr->qsa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)net_hostport);
	((struct sockaddr_in *)addr)->sin_addr.s_addr =
						*(in_addr_t *)hostentry->h_addr_list[0];

	return 0;
}

//=============================================================================

int WINS_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->qsa_family != addr2->qsa_family)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_addr.s_addr !=
	    ((struct sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_port !=
	    ((struct sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
}

//=============================================================================

int WINS_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_in *)addr)->sin_port);
}


int WINS_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_in *)addr)->sin_port = htons((unsigned short)port);
	return 0;
}

//=============================================================================

