/*
	net_wipx.c
	winsock ipx driver

	$Id: net_wipx.c,v 1.19 2007-07-08 11:55:34 sezero Exp $
*/

#include "quakedef.h"
#include "winquake.h"

#ifdef __LCC__
// no wsipx.h in lcc
#define NSPROTO_IPX	1000
#define NSPROTO_SPX	1256
#define NSPROTO_SPXII	1257

typedef struct sockaddr_ipx {
	short sa_family;
	char sa_netnum[4];
	char sa_nodenum[6];
	unsigned short sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX, *LPSOCKADDR_IPX;
#else
// not LCC, include the header
#include <wsipx.h>
#endif

#include "net_wipx.h"

extern cvar_t hostname;

#define MAXHOSTNAMELEN		256

static int net_acceptsocket = -1;		// socket for fielding new connections
static int net_controlsocket;
static struct qsockaddr broadcastaddr;

extern qboolean winsock_initialized;
extern WSADATA		winsockdata;

#define IPXSOCKETS 18
static int ipxsocket[IPXSOCKETS];
static int sequence[IPXSOCKETS];

//=============================================================================

int WIPX_Init (void)
{
	int		i;
	char	buff[MAXHOSTNAMELEN];
	struct qsockaddr addr;
	char	*p;
	int		r;
	WORD	wVersionRequested;

	if (COM_CheckParm ("-noipx"))
		return -1;

	if (winsock_initialized == 0)
	{
		wVersionRequested = MAKEWORD(1, 1);

		r = WSAStartup (MAKEWORD(1, 1), &winsockdata);

		if (r)
		{
			Con_Printf ("Winsock initialization failed.\n");
			return -1;
		}
	}
	winsock_initialized++;

	for (i = 0; i < IPXSOCKETS; i++)
		ipxsocket[i] = 0;

	// determine my name & address
	if (gethostname(buff, MAXHOSTNAMELEN) == 0)
	{
		buff[MAXHOSTNAMELEN-1] = 0;

		// if the quake hostname isn't set, set it to the machine name
		if (strcmp(hostname.string, "UNNAMED") == 0)
		{
			// see if it's a text IP address (well, close enough)
			for (p = buff; *p; p++)
			{
				if ((*p < '0' || *p > '9') && *p != '.')
					break;
			}

			// if it is a real name, strip off the domain; we only want the host
			if (*p)
			{
				for (i = 0; i < 15; i++)
				{
					if (buff[i] == '.')
						break;
				}
				buff[i] = 0;
			}
			Cvar_Set ("hostname", buff);
		}
	}

	if ((net_controlsocket = WIPX_OpenSocket (0)) == -1)
	{
		Con_Printf("%s: Unable to open control socket\n", __thisfunc__);
		if (--winsock_initialized == 0)
			WSACleanup ();
		return -1;
	}

	((struct sockaddr_ipx *)&broadcastaddr)->sa_family = AF_IPX;
	memset(((struct sockaddr_ipx *)&broadcastaddr)->sa_netnum, 0, 4);
	memset(((struct sockaddr_ipx *)&broadcastaddr)->sa_nodenum, 0xff, 6);
	((struct sockaddr_ipx *)&broadcastaddr)->sa_socket = htons((unsigned short)net_hostport);

	WIPX_GetSocketAddr (net_controlsocket, &addr);
	strcpy(my_ipx_address,  WIPX_AddrToString (&addr));
	p = strrchr (my_ipx_address, ':');
	if (p)
		*p = 0;

	Con_Printf("Winsock IPX Initialized\n");
	ipxAvailable = true;

	return net_controlsocket;
}

//=============================================================================

void WIPX_Shutdown (void)
{
	WIPX_Listen (false);
	WIPX_CloseSocket (net_controlsocket);
	if (--winsock_initialized == 0)
		WSACleanup ();
}

//=============================================================================

void WIPX_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != -1)
			return;
		if ((net_acceptsocket = WIPX_OpenSocket (net_hostport)) == -1)
			Sys_Error ("%s: Unable to open accept socket", __thisfunc__);
		return;
	}

	// disable listening
	if (net_acceptsocket == -1)
		return;
	WIPX_CloseSocket (net_acceptsocket);
	net_acceptsocket = -1;
}

//=============================================================================

int WIPX_OpenSocket (int port)
{
	int		handle;
	int		newsocket;
	struct sockaddr_ipx address;
	u_long _true = 1;

	for (handle = 0; handle < IPXSOCKETS; handle++)
	{
		if (ipxsocket[handle] == 0)
			break;
	}
	if (handle == IPXSOCKETS)
		return -1;

	if ((newsocket = socket (AF_IPX, SOCK_DGRAM, NSPROTO_IPX)) == INVALID_SOCKET)
		return -1;

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		goto ErrorReturn;

	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof(_true)) < 0)
		goto ErrorReturn;

	address.sa_family = AF_IPX;
	memset(address.sa_netnum, 0, 4);
	memset(address.sa_nodenum, 0, 6);
	address.sa_socket = htons((unsigned short)port);
	if ( bind (newsocket, (struct sockaddr *)&address, sizeof(address)) == 0)
	{
		ipxsocket[handle] = newsocket;
		sequence[handle] = 0;
		return handle;
	}

	if (ipxAvailable)
		Sys_Error ("Winsock IPX bind failed");
	else // we are still in init phase, no need to error
		Con_Printf ("Winsock IPX bind failed\n");
ErrorReturn:
	closesocket (newsocket);
	return -1;
}

//=============================================================================

int WIPX_CloseSocket (int handle)
{
	int mysocket = ipxsocket[handle];
	int ret;

	ret =  closesocket (mysocket);
	ipxsocket[handle] = 0;
	return ret;
}

//=============================================================================

int WIPX_Connect (int handle, struct qsockaddr *addr)
{
	return 0;
}

//=============================================================================

int WIPX_CheckNewConnections (void)
{
	unsigned long	available;

	if (net_acceptsocket == -1)
		return -1;

	if (ioctlsocket (ipxsocket[net_acceptsocket], FIONREAD, &available) == -1)
		Sys_Error ("WIPX: ioctlsocket (FIONREAD) failed\n");
	if (available)
		return net_acceptsocket;
	return -1;
}

//=============================================================================

//static byte netpacketBuffer[NET_DATAGRAMSIZE + 4];
static byte netpacketBuffer[NET_MAXMESSAGE + 4];
//_Datagram_SearchForHosts calls this with net_message.maxsize as len!

int WIPX_Read (int handle, byte *buf, int len, struct qsockaddr *addr)
{
	int addrlen = sizeof (struct qsockaddr);
	int mysocket = ipxsocket[handle];
	int ret;

	ret = recvfrom (mysocket, (char *)netpacketBuffer, len+4, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == -1)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK || err == WSAECONNREFUSED)
			return 0;
	}

	if (ret < 4)
		return 0;

	// remove sequence number, it's only needed for DOS IPX
	ret -= 4;
	memcpy(buf, netpacketBuffer+4, ret);

	return ret;
}

//=============================================================================

int WIPX_Broadcast (int handle, byte *buf, int len)
{
	return WIPX_Write (handle, buf, len, &broadcastaddr);
}

//=============================================================================

int WIPX_Write (int handle, byte *buf, int len, struct qsockaddr *addr)
{
	int mysocket = ipxsocket[handle];
	int ret;

	// build packet with sequence number
	*(int *)(&netpacketBuffer[0]) = sequence[handle];
	sequence[handle]++;
	memcpy(&netpacketBuffer[4], buf, len);
	len += 4;

	ret = sendto (mysocket, (char *)netpacketBuffer, len, 0, (struct sockaddr *)addr, sizeof(struct qsockaddr));
	if (ret == -1)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			return 0;
	}

	return ret;
}

//=============================================================================

char *WIPX_AddrToString (struct qsockaddr *addr)
{
	static char buf[28];

	sprintf(buf, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%u",
		((struct sockaddr_ipx *)addr)->sa_netnum[0] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_netnum[1] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_netnum[2] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_netnum[3] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[0] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[1] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[2] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[3] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[4] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[5] & 0xff,
		ntohs(((struct sockaddr_ipx *)addr)->sa_socket)
		);
	return buf;
}

//=============================================================================

int WIPX_StringToAddr (const char *string, struct qsockaddr *addr)
{
	int		val;
	char	buf[3];

	buf[2] = 0;
	memset(addr, 0, sizeof(struct qsockaddr));
	addr->sa_family = AF_IPX;

#define DO(src,dest)	\
	buf[0] = string[src];	\
	buf[1] = string[src + 1];	\
	if (sscanf (buf, "%x", &val) != 1)	\
		return -1;	\
	((struct sockaddr_ipx *)addr)->dest = val

	DO(0, sa_netnum[0]);
	DO(2, sa_netnum[1]);
	DO(4, sa_netnum[2]);
	DO(6, sa_netnum[3]);
	DO(9, sa_nodenum[0]);
	DO(11, sa_nodenum[1]);
	DO(13, sa_nodenum[2]);
	DO(15, sa_nodenum[3]);
	DO(17, sa_nodenum[4]);
	DO(19, sa_nodenum[5]);
#undef DO

	sscanf (&string[22], "%u", &val);
	((struct sockaddr_ipx *)addr)->sa_socket = htons((unsigned short)val);

	return 0;
}

//=============================================================================

int WIPX_GetSocketAddr (int handle, struct qsockaddr *addr)
{
	int mysocket = ipxsocket[handle];
	int addrlen = sizeof(struct qsockaddr);

	memset(addr, 0, sizeof(struct qsockaddr));
	if (getsockname(mysocket, (struct sockaddr *)addr, &addrlen) != 0)
	{
		int err;

		err = WSAGetLastError();
		// FIXME: the oscar goes to ?...
	}

	return 0;
}

//=============================================================================

int WIPX_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	strcpy(name, WIPX_AddrToString(addr));
	return 0;
}

//=============================================================================

int WIPX_GetAddrFromName (const char *name, struct qsockaddr *addr)
{
	int		n;
	char	buf[32];

	n = strlen(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, net_hostport);
		return WIPX_StringToAddr (buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, net_hostport);
		return WIPX_StringToAddr (buf, addr);
	}
	if (n > 21 && n <= 27)
		return WIPX_StringToAddr (name, addr);

	return -1;
}

//=============================================================================

int WIPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (*((struct sockaddr_ipx *)addr1)->sa_netnum && *((struct sockaddr_ipx *)addr2)->sa_netnum)
		if (memcmp(((struct sockaddr_ipx *)addr1)->sa_netnum, ((struct sockaddr_ipx *)addr2)->sa_netnum, 4) != 0)
			return -1;
	if (memcmp(((struct sockaddr_ipx *)addr1)->sa_nodenum, ((struct sockaddr_ipx *)addr2)->sa_nodenum, 4) != 0)
		return -1;

	if (((struct sockaddr_ipx *)addr1)->sa_socket != ((struct sockaddr_ipx *)addr2)->sa_socket)
	{
		if (net_allowmultiple.integer)
			return -1;
		else
			return 1;
	}

	return 0;
}

//=============================================================================

int WIPX_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_ipx *)addr)->sa_socket);
}


int WIPX_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_ipx *)addr)->sa_socket = htons((unsigned short)port);
	return 0;
}

//=============================================================================

