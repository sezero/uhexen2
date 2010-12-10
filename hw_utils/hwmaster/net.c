/*
	net_udp.c
	network UDP driver

	$Header: /cvsroot/uhexen2/hw_utils/hwmaster/net.c,v 1.44 2009-04-30 07:06:13 sezero Exp $
*/

#include "q_stdinc.h"
#include "arch_def.h"
#include "net_sys.h"
#include "compiler.h"
#include "defs.h"

//=============================================================================

//
// net driver vars
//

netadr_t	net_local_adr;
netadr_t	net_loopback_adr;
netadr_t	net_from;
sizebuf_t	net_message;

static sys_socket_t	net_socket = INVALID_SOCKET;

#ifdef PLATFORM_WINDOWS
#include "wsaerror.h"
static WSADATA	winsockdata;
#endif

#define	MAX_UDP_PACKET	(MAX_MSGLEN + 9)	/* one more than msg + header */
static byte	net_message_buffer[MAX_UDP_PACKET];


//=============================================================================

//
// net driver functions
//

static void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	memcpy (&s->sin_addr, a->ip, 4);
	s->sin_port = a->port;
}

static void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a)
{
	memcpy (a->ip, &s->sin_addr, 4);
	a->port = s->sin_port;
}

qboolean NET_CompareBaseAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] &&
	    a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3])
	{
		return true;
	}
	return false;
}

qboolean NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] &&
	    a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] &&
	    a.port == b.port)
	{
		return true;
	}
	return false;
}

const char *NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3],
							ntohs(a.port));

	return s;
}

const char *NET_BaseAdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);

	return s;
}

/*
=============
NET_StringToAdr
=============
*/
qboolean NET_StringToAdr (const char *s, netadr_t *a)
{
	struct hostent		*h;
	struct sockaddr_in	sadr;
	char	*colon;
	char	copy[128];

	memset (&sadr, 0, sizeof(sadr));
	sadr.sin_family = AF_INET;
	sadr.sin_port = 0;

	strncpy (copy, s, sizeof(copy) - 1);
	copy[sizeof(copy) - 1] = '\0';
	// strip off a trailing :port if present
	for (colon = copy ; *colon ; colon++)
	{
		if (*colon == ':')
		{
			*colon = 0;
			sadr.sin_port = htons((short)atoi(colon+1));
		}
	}

	if (copy[0] >= '0' && copy[0] <= '9')
	{
		sadr.sin_addr.s_addr = inet_addr(copy);
	}
	else
	{
		h = gethostbyname (copy);
		if (!h)
			return false;
		sadr.sin_addr.s_addr = *(in_addr_t *)h->h_addr_list[0];
	}

	SockadrToNetadr (&sadr, a);

	return true;
}


//=============================================================================

int NET_GetPacket (void)
{
	int	ret;
	struct sockaddr_in	from;
	socklen_t		fromlen;

	fromlen = sizeof(from);
	ret = recvfrom(net_socket, (char *)net_message_buffer, sizeof(net_message_buffer), 0,
			(struct sockaddr *)&from, &fromlen);
	SockadrToNetadr (&from, &net_from);

	if (ret == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		if (err == EWOULDBLOCK)
			return 0;
# ifdef PLATFORM_WINDOWS
		if (err == WSAEMSGSIZE)
		{
			printf ("Oversize packet from %s\n",
					NET_AdrToString (net_from));
			return 0;
		}
		if (err == WSAECONNRESET)
		{
			printf ("Connection reset by peer %s\n",
					NET_AdrToString (net_from));
			return 0;
		}
# endif	/* _WINDOWS */
	//	Sys_Error ("%s: %s", __thisfunc__, socketerror(err));
		printf ("%s warning: %s\n", __thisfunc__, socketerror(err));
		return 0;
	}

	net_message.cursize = ret;
	if (ret == (int) sizeof(net_message_buffer))
	{
		printf ("Oversize packet from %s\n",
					NET_AdrToString (net_from));
		return 0;
	}

	return ret;
}


//=============================================================================

void NET_SendPacket (int length, void *data, netadr_t to)
{
	int	ret;
	struct sockaddr_in	addr;

	NetadrToSockadr (&to, &addr);

	ret = sendto (net_socket, (char *)data, length, 0,
				(struct sockaddr *)&addr, sizeof(addr) );
	if (ret == SOCKET_ERROR)
	{
		int err = SOCKETERRNO;
		if (err == EWOULDBLOCK)
			return;
		printf ("%s ERROR: %s\n", __thisfunc__, socketerror(err));
	}
}


//=============================================================================

int NET_CheckSockets (void)
{
	fd_set		fdset;
	struct timeval	timeout;
	int	ret;

	FD_ZERO (&fdset);
	FD_SET (net_socket, &fdset);
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;

	ret = select (net_socket + 1, &fdset, NULL, NULL, &timeout);
	if (ret == SOCKET_ERROR)
		return -1;
	return ret;
}

//=============================================================================

static sys_socket_t UDP_OpenSocket (int port)
{
	int			i;
	sys_socket_t	newsocket;
	struct sockaddr_in	address;
	unsigned long _true = 1;
	int		err;

	newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (newsocket == INVALID_SOCKET)
	{
		err = SOCKETERRNO;
		Sys_Error ("%s: socket: %s", __thisfunc__, socketerror(err));
	}

	if (ioctlsocket (newsocket, FIONBIO, &_true) == SOCKET_ERROR)
	{
		err = SOCKETERRNO;
		Sys_Error ("%s: ioctl FIONBIO: %s", __thisfunc__, socketerror(err));
	}

	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	//ZOID -- check for interface binding option
	i = COM_CheckParm("-ip");
	if (!i)
		i = COM_CheckParm("-bindip");
	if (i && i < com_argc-1)
	{
		address.sin_addr.s_addr = inet_addr(com_argv[i+1]);
		if (address.sin_addr.s_addr == INADDR_NONE)
			Sys_Error ("%s is not a valid IP address", com_argv[i+1]);
		printf("Binding to IP Interface Address of %s\n", inet_ntoa(address.sin_addr));
	}
	else
	{
		address.sin_addr.s_addr = INADDR_ANY;
	}

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	if (bind(newsocket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
	{
		err = SOCKETERRNO;
		Sys_Error ("%s: bind: %s", __thisfunc__, socketerror(err));
	}

	return newsocket;
}

static void NET_GetLocalAddress (void)
{
	char	buff[MAXHOSTNAMELEN];
	struct sockaddr_in	address;
	socklen_t		namelen;
	int		err;

	if (gethostname(buff, MAXHOSTNAMELEN) == SOCKET_ERROR)
	{
		err = SOCKETERRNO;
		Sys_Error ("%s: gethostname: %s", __thisfunc__, socketerror(err));
	}
	buff[MAXHOSTNAMELEN-1] = 0;

	NET_StringToAdr (buff, &net_local_adr);

	namelen = sizeof(address);
	if (getsockname (net_socket, (struct sockaddr *)&address, &namelen) == SOCKET_ERROR)
	{
		err = SOCKETERRNO;
		Sys_Error ("%s: getsockname: %s", __thisfunc__, socketerror(err));
	}
	net_local_adr.port = address.sin_port;

	printf("IP address %s\n", NET_AdrToString(net_local_adr));
}

/*
====================
NET_Init
====================
*/
void NET_Init (int port)
{
	in_addr_t a = htonl(INADDR_LOOPBACK);
#ifdef PLATFORM_WINDOWS
	int err = WSAStartup(MAKEWORD(1,1), &winsockdata);
	if (err != 0)
		Sys_Error ("Winsock initialization failed (%s)", socketerror(err));
#endif

	//
	// open the single socket to be used for all communications
	//
	net_socket = UDP_OpenSocket (port);

	//
	// init the message buffer
	//
	SZ_Init (&net_message, net_message_buffer, sizeof(net_message_buffer));

	//
	// determine my name & address
	//
	NET_GetLocalAddress ();

	memset (&net_loopback_adr, 0, sizeof(netadr_t));
	memcpy (net_loopback_adr.ip, &a, 4);

	printf("UDP Initialized\n");
}

/*
====================
NET_Shutdown
====================
*/
void	NET_Shutdown (void)
{
	if (net_socket != INVALID_SOCKET)
	{
		closesocket (net_socket);
		net_socket = INVALID_SOCKET;
	}
#ifdef PLATFORM_WINDOWS
	WSACleanup ();
#endif
}

