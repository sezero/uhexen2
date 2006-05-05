/*
HWTERM 1.2 HexenWorld Remote Console Terminal
Idea based on QTerm 1.1 by Michael Dwyer/N0ZAP (18-May-1998).
Made to work with HexenWorld using code from the HexenWorld
engine (C) Raven Software and ID Software. Socket timeout code
taken from the XQF project.
Copyright (C) 1998 Michael Dwyer <mdwyer@holly.colostate.edu>
Copyright (C) 2006 O. Sezer <sezero@users.sourceforge.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef PLATFORM_UNIX
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "huffman.h"

#ifdef PLATFORM_UNIX
#define ioctlsocket(A,B,C) ioctl(A,B,C)
#define closesocket(A) close(A)
#endif
#ifdef _WIN32
static WSADATA	winsockdata;
// socklen_t: on win32, it seems to be a winsock2 thing
#if !( defined(_WS2TCPIP_H) || defined(_WS2TCPIP_H_) )
typedef int	socklen_t;
#endif
#endif


//=============================================================================

typedef struct
{
	unsigned char	ip[4];
	unsigned short	port;
	unsigned short	pad;
} netadr_t;

static int		socketfd = -1;

static void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a)
{
	*(int *)&a->ip = *(int *)&s->sin_addr;
	a->port = s->sin_port;
}

static void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
}

char *NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

static int NET_StringToAdr (char *s, netadr_t *a)
{
	struct hostent		*h;
	struct sockaddr_in	sadr;
	char	*colon;
	char	copy[128];

	memset (&sadr, 0, sizeof(sadr));
	sadr.sin_family = AF_INET;

	sadr.sin_port = 0;

	strcpy (copy, s);
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
		*(int *)&sadr.sin_addr = inet_addr(copy);
	}
	else
	{
		h = gethostbyname (copy);
		if (!h)
			return 0;
		*(int *)&sadr.sin_addr = *(int *)h->h_addr_list[0];
	}

	SockadrToNetadr (&sadr, a);

	return 1;
}

static int NET_WaitReadTimeout (int fd, long sec, long usec)
{
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	return select(fd+1, &rfds, NULL, NULL, &tv);
}

static void NET_Shutdown (void)
{
	if (socketfd != -1)
		closesocket (socketfd);
#ifdef _WIN32
	WSACleanup ();
#endif
}

//=============================================================================

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsnprintf (text, sizeof (text), error,argptr);
	va_end (argptr);

	printf ("\nERROR: %s\n\n", text);

	exit (1);
}

//=============================================================================

#define	VERSION_STR		"1.2.2"

#define	PORT_SERVER		26950
#define	MAX_RCON_PACKET		256
// number of 255s to put on the header
#define	HEADER_SIZE	4

static unsigned char huffbuff[65536];

int main (int argc, char *argv[])
{
	int		len, hufflen, size;
	int		i, k;
	socklen_t	fromlen;
//	unsigned char	packet[MAX_RCON_PACKET];
	char		packet[MAX_RCON_PACKET];
	char		response[MAX_RCON_PACKET*10];
	netadr_t		ipaddress;
	struct sockaddr_in	hostaddress;
	unsigned long	_true = 1;
#ifdef _WIN32
	WORD	wVersionRequested;
	int		err;
#endif

	printf ("HWTERM %s\n", VERSION_STR);

// Command Line Sanity Checking
	if (argc < 3)
	{
		printf ("Usage: %s <address>[:port] <password>\n", argv[0]);
		exit (1);
	}

#ifdef _WIN32
// Init winsock
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup (MAKEWORD(1, 1), &winsockdata);
	if (err)
		Sys_Error ("Winsock initialization failed.");
#endif

// Decode the address and port
	if (!NET_StringToAdr(argv[1], &ipaddress))
	{
		NET_Shutdown ();
		Sys_Error ("Unable to resolve address %s", argv[1]);
	}
	if (ipaddress.port == 0)
		ipaddress.port = htons(PORT_SERVER);
	NetadrToSockadr(&ipaddress, &hostaddress);
	printf ("Using address %s\n", NET_AdrToString(ipaddress));

// Open the Socket
	if ((socketfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		NET_Shutdown ();
		Sys_Error ("Couldn't open socket: %s", strerror(errno));
	}
// Set the socket to non-blocking mode
	if (ioctlsocket (socketfd, FIONBIO, &_true) == -1)
	{
		NET_Shutdown ();
		Sys_Error ("ioctl FIONBIO:", strerror(errno));
	}

	printf ("Use CTRL-C to exit\n");

/* For these next blocks, k denotes the end
   of string for the packet. DO NOT RESET IT.
   j denotes the beginning of the command
   portion. DO NOT RESET IT EITHER.	*/

// Prepare the header: \377\377\377\377rcon<space>
	for (k = 0 ; k < HEADER_SIZE ; k++)
	{
		packet[k] = 255;
	}
	packet[k] = 0;
	strcat (packet, "rcon ");
	k = strlen(packet);
// Put the password on the packet
	for (i = 0 ; i < strlen(argv[2]) ; i++)
	{
		packet[k] = argv[2][i];
		k++;
	}
// Add a space
	packet[k] = 0x20;
	k++;

// Init Huffman
	HuffInit ();

// Loop for user input
	while (1)
	{
		printf ("RCON> ");
		fgets (&packet[k], 224, stdin);
		len = strlen(packet);
		len++;

	// Send the packet
		HuffEncode ((unsigned char *)packet,huffbuff,len,&hufflen);
		size = sendto(socketfd, huffbuff, hufflen, 0, 
			(struct sockaddr *)&hostaddress, sizeof(hostaddress));

	// See if it worked
		if (size != hufflen)
		{
			perror ("Sendto failed");
			printf ("Tried to send %i, sent %i\n", hufflen, size);
			goto error_out;
		}

	// Read the response
		fromlen = sizeof(hostaddress);
		if (NET_WaitReadTimeout (socketfd, 5, 0) <= 0)
		{
			printf ("*** timeout waiting for reply\n");
		}
		else
		while (NET_WaitReadTimeout(socketfd, 0, 50000) > 0)
		{
			size = recvfrom(socketfd, huffbuff, MAX_RCON_PACKET*10, 0, 
				(struct sockaddr *)&hostaddress, &fromlen);
			if (size < 0)
			{
#	ifdef _WIN32
				err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
				{
					printf ("Recv failed: %s\n", strerror(err));
					goto error_out;
				}
#	else
				if (errno != EWOULDBLOCK)
				{
					perror("Recv failed");
					goto error_out;
				}
#	endif
			}
			else
			{
				HuffDecode(huffbuff, (unsigned char *)response, size, &size);
				printf (&response[5]);
			}
		}
	}

error_out:
	NET_Shutdown ();
	exit (1);
}

