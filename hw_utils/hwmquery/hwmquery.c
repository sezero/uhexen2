/*
	hwmquery.c
	$Id: hwmquery.c,v 1.19 2007-10-02 13:39:19 sezero Exp $

	HWMQUERY 0.2 HexenWorld Master Server Query
	Copyright (C) 2006-2007 O. Sezer <sezero@users.sourceforge.net>

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


#include "net_sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if defined(PLATFORM_UNIX)
#include <sys/time.h>
#endif
#include "compiler.h"
#include "qsnprint.h"


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

static int		socketfd = -1;

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

static void NET_Init (void)
{
#if defined(PLATFORM_WINDOWS)
	if (WSAStartup(MAKEWORD(1,1), &winsockdata) != 0)
		Sys_Error ("Winsock initialization failed.");
#endif	/* PLATFORM_WINDOWS */
}

static void NET_Shutdown (void)
{
	if (socketfd != -1)
		closesocket (socketfd);
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

#define	VER_HWMQUERY_MAJ	0
#define	VER_HWMQUERY_MID	2
#define	VER_HWMQUERY_MIN	0

#define	PORT_MASTER		26900
#define	PORT_SERVER		26950
#define	MAX_PACKET		2048
// number of 255s to put on the header
// qwmaster uses 4, but hwmaster uses 5
#define	HEADER_SIZE	5

#define	S2C_CHALLENGE		'c'
#define	M2C_MASTER_REPLY	'd'

static const unsigned char query_msg[] =
		{ 255, S2C_CHALLENGE, '\0' };

int main (int argc, char **argv)
{
	ssize_t		size;
	size_t		pos;
	socklen_t	fromlen;
	unsigned char	response[MAX_PACKET];
	netadr_t		ipaddress;
	struct sockaddr_in	hostaddress;
	unsigned long	_true = 1;

	printf ("HWMASTER QUERY %d.%d.%d\n", VER_HWMQUERY_MAJ, VER_HWMQUERY_MID, VER_HWMQUERY_MIN);

// Command Line Sanity Checking
	if (argc < 2)
	{
		printf ("Usage: %s <address>[:port]\n", argv[0]);
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
		ipaddress.port = htons(PORT_MASTER);
	NetadrToSockadr(&ipaddress, &hostaddress);

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
		Sys_Error ("ioctl FIONBIO: %s", strerror(errno));
	}

	// Send the query packet
	printf ("Querying master server at %s\n", NET_AdrToString(ipaddress));
	size = sendto(socketfd, (const char *)query_msg, sizeof(query_msg), 0,
			(struct sockaddr *)&hostaddress, sizeof(hostaddress));

	// See if it worked
	if (size != sizeof(query_msg))
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
		Sys_Quit (2);
	}
	else
	while (NET_WaitReadTimeout(socketfd, 0, 50000) > 0)
	{
		size = recvfrom(socketfd, (char *)response, sizeof(response), 0,
				(struct sockaddr *)&hostaddress, &fromlen);
		if (size < 0)
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
		else if (response[0] != 255 || response[1] != 255 ||
			 response[2] != 255 || response[3] != 255 ||
#if (HEADER_SIZE == 5)
			 response[4] != 255 ||
#endif
			 response[HEADER_SIZE] != M2C_MASTER_REPLY ||
			 response[HEADER_SIZE+1] != '\n')
		{
			printf ("Invalid response received\n");
			Sys_Quit (1);
		}
		else
		{
			char	*tmp = (char *)(response + HEADER_SIZE + 2);

			printf ("H2W Servers registered at %s:", NET_AdrToString(ipaddress));

			if (!*tmp)
				printf ("  NONE\n");
			else
			{
				struct in_addr	*addr;

				printf ("\n");
				// each address is 4 bytes (ip) + 2 bytes (port) == 6 bytes
				for (pos = 0; pos < strlen(tmp); pos = pos + 6)
				{
					addr = (struct in_addr*)(tmp + pos);
					printf ("%s:%u\n", inet_ntoa(*addr), htons( *((unsigned short *)(tmp + pos + 4)) ) );
				}
			}
		}
	}

	NET_Shutdown ();
	exit (0);
}

