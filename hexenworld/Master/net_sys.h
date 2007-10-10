/*
	net_sys.h
	common network system header

	$Id: net_sys.h,v 1.8 2007-10-10 14:30:38 sezero Exp $
*/

#ifndef __NET_SYS_H__
#define __NET_SYS_H__

#include <sys/types.h>
#include <errno.h>
#include "arch_def.h"

/* unix includes and compatibility macros */
#if defined(PLATFORM_UNIX)

#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>

#if defined(__SOLARIS__)
#include <sys/filio.h>
#endif

#if defined(__MORPHOS__)
#include <proto/socket.h>
#endif
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef INADDR_NONE
#define	INADDR_NONE	((in_addr_t) 0xffffffff)
#endif
#ifndef INADDR_LOOPBACK
#define	INADDR_LOOPBACK	((in_addr_t) 0x7f000001)	/* 127.0.0.1	*/
#endif

#define	SOCKETERRNO	errno

#if defined(__MORPHOS__)
/*#if !defined(ixemul)*/
#undef	SOCKETERRNO
#define	SOCKETERRNO	Errno()
/*#endif*/
#define	socklen_t	int
#define	ioctlsocket	IoctlSocket
#define	closesocket	CloseSocket
#else
#define	ioctlsocket	ioctl
#define	closesocket	close
#endif

#endif	/* end of unix stuff */


/* windows includes and compatibility macros */
#if defined(PLATFORM_WINDOWS)

#include <windows.h>
#include <winsock.h>

/* there is no in_addr_t on win32: define it as
   the type of the S_addr of in_addr structure */
typedef u_long	in_addr_t;

#if !( defined(_WS2TCPIP_H) || defined(_WS2TCPIP_H_) )
/* on win32, socklen_t seems to be a winsock2 thing */
typedef int	socklen_t;
#endif

#define	SOCKETERRNO	WSAGetLastError()
#define	EWOULDBLOCK	WSAEWOULDBLOCK
#define	ECONNREFUSED	WSAECONNREFUSED

#endif	/* end of windows stuff */


#if !defined(MAXHOSTNAMELEN)
/* SUSv2 guarantees that `Host names are limited to 255 bytes'.
   POSIX 1003.1-2001 guarantees that `Host names (not including
   the terminating NUL) are limited to HOST_NAME_MAX bytes'. */
#define	MAXHOSTNAMELEN		256
#endif	/* MAXHOSTNAMELEN */


#endif	/* __NET_SYS_H__ */

