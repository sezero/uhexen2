/*
	hwterm.c
	$Id: hwterm.c,v 1.28 2009-04-27 12:50:43 sezero Exp $

	HWTERM 1.2 HexenWorld Remote Console Terminal
	Idea based on QTerm 1.1 by Michael Dwyer/N0ZAP (18-May-1998).
	Made to work with HexenWorld using code from the HexenWorld
	engine (C) Raven Software and ID Software. Socket timeout code
	taken from the XQF project.
	Copyright (C) 1998 Michael Dwyer <mdwyer@holly.colostate.edu>
	Copyright (C) 2006-2009 O. Sezer <sezero@users.sourceforge.net>

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
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/


#undef	USE_HUFFMAN
#define	USE_HUFFMAN	1

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "arch_def.h"
#include "compiler.h"
#include "net_sys.h"
#include "qsnprint.h"
#if defined(PLATFORM_UNIX)
#include <sys/time.h>
#endif
#if defined(USE_HUFFMAN)
#include "huffman.h"
#endif


//=============================================================================

typedef struct
{
	unsigned char	ip[4];
	unsigned short	port;
	unsigned short	pad;
} netadr_t;

//=============================================================================

#if defined(PLATFORM_WINDOWS)
static WSADATA		winsockdata;
#endif

static sys_socket_t	socketfd = INVALID_SOCKET;

void Sys_Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
static void Sys_Quit (int error_state) __attribute__((noreturn));

//=============================================================================

void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	q_vsnprintf (text, sizeof (text), error,argptr);
	va_end (argptr);

	printf ("\nERROR: %s\n\n", text);

	exit (1);
}

//=============================================================================

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

const char *NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	sprintf (s, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3],
							ntohs(a.port));

	return s;
}

static int NET_StringToAdr (const char *s, netadr_t *a)
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
			return 0;
		sadr.sin_addr.s_addr = *(in_addr_t *)h->h_addr_list[0];
	}

	SockadrToNetadr (&sadr, a);

	return 1;
}

static int NET_WaitReadTimeout (sys_socket_t fd, long sec, long usec)
{
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	return select(fd+1, &rfds, NULL, NULL, &tv);
}

static void NET_Init (void)
{
#if defined(PLATFORM_WINDOWS)
	if (WSAStartup(MAKEWORD(1,1), &winsockdata) != 0)
		Sys_Error ("Winsock initialization failed.");
#endif	/* PLATFORM_WINDOWS */
}

static void NET_Shutdown (void)
{
	if (socketfd != INVALID_SOCKET)
	{
		closesocket (socketfd);
		socketfd = INVALID_SOCKET;
	}
#if defined(PLATFORM_WINDOWS)
	WSACleanup ();
#endif
}

static void Sys_Quit (int error_state)
{
	NET_Shutdown ();
	exit (error_state);
}

//=============================================================================

#define	VER_HWTERM_MAJ		1
#define	VER_HWTERM_MID		2
#define	VER_HWTERM_MIN		6

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
	unsigned char	packet[MAX_RCON_PACKET];
	unsigned char	response[MAX_RCON_PACKET*10];
	netadr_t		ipaddress;
	struct sockaddr_in	hostaddress;
	unsigned long	_true = 1;

	printf ("HWTERM %d.%d.%d\n", VER_HWTERM_MAJ, VER_HWTERM_MID, VER_HWTERM_MIN);

// Command Line Sanity Checking
	if (argc < 3)
	{
		printf ("Usage: %s <address>[:port] <password>\n", argv[0]);
		exit (1);
	}

// Init OS-specific network stuff
	NET_Init ();

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
	socketfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketfd == INVALID_SOCKET)
	{
		NET_Shutdown ();
		Sys_Error ("Couldn't open socket: %s", strerror(errno));
	}
// Set the socket to non-blocking mode
	if (ioctlsocket (socketfd, FIONBIO, &_true) == SOCKET_ERROR)
	{
		NET_Shutdown ();
		Sys_Error ("ioctl FIONBIO: %s", strerror(errno));
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
	packet[k]	= 'r';
	packet[++k]	= 'c';
	packet[++k]	= 'o';
	packet[++k]	= 'n';
	packet[++k]	= ' ';
	packet[++k]	= 0;
// Put the password on the packet
	for (i = 0 ; i < strlen(argv[2]) ; i++)
	{
		packet[k] = argv[2][i];
		k++;
	}
// Add a space
	packet[k] = 0x20;
	packet[++k] = 0;

// Init Huffman
	HuffInit ();

// Loop for user input
	while (1)
	{
		printf ("RCON> ");
		fgets ((char *)(packet+k), sizeof(packet)-k, stdin);
		len = strlen((char *)packet) + 1;

	// Send the packet
		HuffEncode (packet, huffbuff, len, &hufflen);
		size = sendto(socketfd, (char *)huffbuff, hufflen, 0,
			(struct sockaddr *)&hostaddress, sizeof(hostaddress));

	// See if it worked
		if (size != hufflen)
		{
			perror ("Sendto failed");
			Sys_Quit (1);
		}

	// Read the response
		memset (response, 0, sizeof(response));
		fromlen = sizeof(hostaddress);
		if (NET_WaitReadTimeout (socketfd, 5, 0) <= 0)
		{
			printf ("*** timeout waiting for reply\n");
		}
		else
		while (NET_WaitReadTimeout(socketfd, 0, 50000) > 0)
		{
			size = recvfrom(socketfd, (char *)huffbuff, sizeof(response), 0,
				(struct sockaddr *)&hostaddress, &fromlen);
			if (size == SOCKET_ERROR)
			{
#if defined(PLATFORM_WINDOWS)
				int err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
				{
					printf ("Recv failed: %s\n", strerror(err));
					Sys_Quit (1);
				}
#else
				if (errno != EWOULDBLOCK)
				{
					perror("Recv failed");
					Sys_Quit (1);
				}
#endif
			}
			else if (size == sizeof(response))
			{
				printf ("Received oversized packet!\n");
				Sys_Quit (1);
			}
			else
			{
				HuffDecode(huffbuff, response, size, &size, sizeof(response));
				if (size > sizeof(response))
				{
					printf ("Received oversized compressed data!\n");
					Sys_Quit (1);
				}
				for (i = 0 ; i < HEADER_SIZE ; i++)
				{
					if (response[i] != 255)
					{
						printf ("Invalid response received\n");
						Sys_Quit (1);
					}
				}

				printf ("%s\n", (char *)(response+HEADER_SIZE+1));
			}
		}
	}

	NET_Shutdown ();
	exit (0);
}

