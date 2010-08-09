/*
	hwrcon.c
	$Id: hwrcon.c,v 1.25 2010-08-09 14:33:12 sezero Exp $

	HWRCON 1.2 HexenWorld Remote CONsole
	Idea based on RCon 1.1 by Michael Dwyer/N0ZAP (18-May-1998).
	Made to work with HexenWorld using code from the HexenWorld
	engine (C) Raven Software and ID Software.
	Copyright (C) 1998 Michael Dwyer <mdwyer@holly.colostate.edu>
	Copyright (C) 2006-2010 O. Sezer <sezero@users.sourceforge.net>

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


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "arch_def.h"
#include "compiler.h"

#define	COMPILE_TIME_ASSERT(name, x)	\
	typedef int dummy_ ## name[(x) * 2 - 1]

#include "net_sys.h"
#include "qsnprint.h"
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
#include "wsaerror.h"
static WSADATA		winsockdata;
#endif

static sys_socket_t	socketfd = INVALID_SOCKET;

void Sys_Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));

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

static void NET_Init (void)
{
#if defined(PLATFORM_WINDOWS)
	int err = WSAStartup(MAKEWORD(1,1), &winsockdata);
	if (err != 0)
		Sys_Error ("Winsock initialization failed (%s)", socketerror(err));
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

//=============================================================================

#define	VER_HWRCON_MAJ		1
#define	VER_HWRCON_MID		2
#define	VER_HWRCON_MIN		6

#define	PORT_SERVER		26950
#define	MAX_RCON_PACKET		256

#ifdef USE_HUFFMAN
static unsigned char huffbuff[65536];
// number of 255s to put on the header
#define	HEADER_SIZE	4
#else
// if we won't use HeffEncode, we need to put an additional 255
// at the beginning for our message to be read by server
#define	HEADER_SIZE	5
#endif

int main (int argc, char *argv[])
{
	int		len, size;
#ifdef USE_HUFFMAN
	int		hufflen;
#endif
	int		i, j, k;
	unsigned char	packet[MAX_RCON_PACKET];
	netadr_t		ipaddress;
	struct sockaddr_in	hostaddress;
	int		err;

	printf ("HWRCON %d.%d.%d\n", VER_HWRCON_MAJ, VER_HWRCON_MID, VER_HWRCON_MIN);

// Command Line Sanity Checking
	if (argc < 3)
	{
		printf ("Usage: %s <address>[:port] <password> commands ...\n", argv[0]);
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
// Concat all the excess command line stuff into a single string
	for (i = 2 ; i < argc ; i++) 
	{
		for (j = 0 ; j < strlen(argv[i]) ; j++)
		{
			packet[k] = argv[i][j];
			if (++k > sizeof(packet) - 1)
				Sys_Error ("Command too long");
		}
		if (i != argc - 1)
		{
			packet[k] = 0x20;	// add a space
			if (++k > sizeof(packet) - 1)
				Sys_Error ("Command too long");
		}
	}

	packet[k] = '\0';
	len = k + 1;

// Open the Socket
	socketfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketfd == INVALID_SOCKET)
	{
		err = SOCKETERRNO;
		NET_Shutdown ();
		Sys_Error ("Couldn't open socket: %s", socketerror(err));
	}

// Send the packet
#ifdef USE_HUFFMAN
// Init Huffman
	HuffInit ();
	HuffEncode (packet, huffbuff, len, &hufflen);
	size = sendto(socketfd, (char *)huffbuff, hufflen, 0,
#else
	size = sendto(socketfd, (char *)packet, len, 0,
#endif
			(struct sockaddr *)&hostaddress, sizeof(hostaddress));

// See if it worked
#ifdef USE_HUFFMAN
	len = hufflen;
#endif
	if (size != len)
	{
		err = SOCKETERRNO;
		printf ("Sendto failed: %s\n", socketerror(err));
		NET_Shutdown ();
		exit (1);
	}

// Clean Up
	NET_Shutdown ();
	exit (0);
}

