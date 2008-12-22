/*
	net_sys.h
	common network system header
	- depends on arch_def.h
	- may depend on q_stdinc.h

	$Id: net_sys.h,v 1.12 2008-12-22 12:24:11 sezero Exp $

	Copyright (C) 2007  O.Sezer <sezero@users.sourceforge.net>

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
		Boston, MA  02110-1301  USA
*/

#ifndef __NET_SYS_H__
#define __NET_SYS_H__

#include <sys/types.h>
#include <errno.h>

/* unix includes and compatibility macros */
#if defined(PLATFORM_UNIX) || defined(PLATFORM_AMIGA)

#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>

#if defined(__sun) || defined(sun)
#include <sys/filio.h>
#endif	/* __sunos__ */

#if defined(PLATFORM_AMIGA)
#include <proto/socket.h>
#endif
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#if defined(PLATFORM_AMIGA)
typedef int	socklen_t;
#define	SOCKETERRNO	Errno()
#define	ioctlsocket	IoctlSocket
#define	closesocket	CloseSocket
#else
#define	SOCKETERRNO	errno
#define	ioctlsocket	ioctl
#define	closesocket	close
#endif

#endif	/* end of unix stuff */


/* windows includes and compatibility macros */
#if defined(PLATFORM_WINDOWS)

#include <windows.h>
#if defined(_WIN64) && !defined(_USE_WINSOCK2)
#define _USE_WINSOCK2	1
#endif
#if !defined(_USE_WINSOCK2)
#include <winsock.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

/* there is no in_addr_t on windows: define it as
   the type of the S_addr of in_addr structure */
#if defined(__GNUC__)
typedef typeof(((struct in_addr *)0)->s_addr)	in_addr_t;
#else
typedef u_long	in_addr_t;
#endif	/* in_addr_t type */

/* on windows, socklen_t is to be a winsock2 thing */
#if !defined(IP_MSFILTER_SIZE)
typedef int	socklen_t;
#endif	/* socklen_t type */

#define	SOCKETERRNO	WSAGetLastError()
#define	EWOULDBLOCK	WSAEWOULDBLOCK
#define	ECONNREFUSED	WSAECONNREFUSED

#endif	/* end of windows stuff */


/* macros which may still be missing */

#if !defined(INADDR_NONE)
#define	INADDR_NONE	((in_addr_t) 0xffffffff)
#endif	/* INADDR_NONE */

#if !defined(INADDR_LOOPBACK)
#define	INADDR_LOOPBACK	((in_addr_t) 0x7f000001)	/* 127.0.0.1	*/
#endif	/* INADDR_LOOPBACK */


#if !defined(MAXHOSTNAMELEN)
/* SUSv2 guarantees that `Host names are limited to 255 bytes'.
   POSIX 1003.1-2001 guarantees that `Host names (not including
   the terminating NUL) are limited to HOST_NAME_MAX bytes'. */
#define	MAXHOSTNAMELEN		256
#endif	/* MAXHOSTNAMELEN */


#endif	/* __NET_SYS_H__ */

