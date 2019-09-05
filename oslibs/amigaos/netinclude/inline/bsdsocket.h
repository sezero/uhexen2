/*
 * $Id$
 *
 * :ts=8
 *
 * 'Roadshow' -- Amiga TCP/IP stack
 * Copyright © 2001-2019 by Olaf Barthel.
 * All Rights Reserved.
 *
 * Amiga specific TCP/IP 'C' header files;
 * Freely Distributable
 */

/*
 * This file was created with fd2pragma V2.171 using the following options:
 *
 * fd2pragma bsdsocket_lib.sfd to RAM:inline special 47
 *
 * The 'struct timeval' was replaced by 'struct __timeval'.
 */

#ifndef _INLINE_BSDSOCKET_H
#define _INLINE_BSDSOCKET_H

#ifndef CLIB_BSDSOCKET_PROTOS_H
#define CLIB_BSDSOCKET_PROTOS_H
#endif

#ifndef  EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef  DEVICES_TIMER_H
#include <devices/timer.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef  NETINET_IN_H
#include <netinet/in.h>
#endif
#ifndef  SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifndef  SYS_MBUF_H
#include <sys/mbuf.h>
#endif
#ifndef  NET_ROUTE_H
#include <net/route.h>
#endif
#ifndef  NETDB_H
#include <netdb.h>
#endif
#ifndef  LIBRARIES_BSDSOCKET_H
#include <libraries/bsdsocket.h>
#endif
#ifndef  DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif

#ifndef BSDSOCKET_BASE_NAME
#define BSDSOCKET_BASE_NAME SocketBase
#endif

#define socket(domain, type, protocol) ({ \
  LONG _socket_domain = (domain); \
  LONG _socket_type = (type); \
  LONG _socket_protocol = (protocol); \
  ({ \
  register char * _socket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"))) \
  (_socket__bn - 30))(_socket__bn, _socket_domain, _socket_type, _socket_protocol); \
});})

#define bind(sock, name, namelen) ({ \
  LONG _bind_sock = (sock); \
  struct sockaddr * _bind_name = (name); \
  LONG _bind_namelen = (namelen); \
  ({ \
  register char * _bind__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct sockaddr * __asm("a0"), LONG __asm("d1"))) \
  (_bind__bn - 36))(_bind__bn, _bind_sock, _bind_name, _bind_namelen); \
});})

#define listen(sock, backlog) ({ \
  LONG _listen_sock = (sock); \
  LONG _listen_backlog = (backlog); \
  ({ \
  register char * _listen__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_listen__bn - 42))(_listen__bn, _listen_sock, _listen_backlog); \
});})

#define accept(sock, addr, addrlen) ({ \
  LONG _accept_sock = (sock); \
  struct sockaddr * _accept_addr = (addr); \
  socklen_t * _accept_addrlen = (addrlen); \
  ({ \
  register char * _accept__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct sockaddr * __asm("a0"), socklen_t * __asm("a1"))) \
  (_accept__bn - 48))(_accept__bn, _accept_sock, _accept_addr, _accept_addrlen); \
});})

#define connect(sock, name, namelen) ({ \
  LONG _connect_sock = (sock); \
  struct sockaddr * _connect_name = (name); \
  LONG _connect_namelen = (namelen); \
  ({ \
  register char * _connect__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct sockaddr * __asm("a0"), LONG __asm("d1"))) \
  (_connect__bn - 54))(_connect__bn, _connect_sock, _connect_name, _connect_namelen); \
});})

#define sendto(sock, buf, len, flags, to, tolen) ({ \
  LONG _sendto_sock = (sock); \
  APTR _sendto_buf = (buf); \
  LONG _sendto_len = (len); \
  LONG _sendto_flags = (flags); \
  struct sockaddr * _sendto_to = (to); \
  LONG _sendto_tolen = (tolen); \
  ({ \
  register char * _sendto__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), LONG __asm("d1"), LONG __asm("d2"), struct sockaddr * __asm("a1"), LONG __asm("d3"))) \
  (_sendto__bn - 60))(_sendto__bn, _sendto_sock, _sendto_buf, _sendto_len, _sendto_flags, _sendto_to, _sendto_tolen); \
});})

#define send(sock, buf, len, flags) ({ \
  LONG _send_sock = (sock); \
  APTR _send_buf = (buf); \
  LONG _send_len = (len); \
  LONG _send_flags = (flags); \
  ({ \
  register char * _send__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), LONG __asm("d1"), LONG __asm("d2"))) \
  (_send__bn - 66))(_send__bn, _send_sock, _send_buf, _send_len, _send_flags); \
});})

#define recvfrom(sock, buf, len, flags, addr, addrlen) ({ \
  LONG _recvfrom_sock = (sock); \
  APTR _recvfrom_buf = (buf); \
  LONG _recvfrom_len = (len); \
  LONG _recvfrom_flags = (flags); \
  struct sockaddr * _recvfrom_addr = (addr); \
  socklen_t * _recvfrom_addrlen = (addrlen); \
  ({ \
  register char * _recvfrom__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), LONG __asm("d1"), LONG __asm("d2"), struct sockaddr * __asm("a1"), socklen_t * __asm("a2"))) \
  (_recvfrom__bn - 72))(_recvfrom__bn, _recvfrom_sock, _recvfrom_buf, _recvfrom_len, _recvfrom_flags, _recvfrom_addr, _recvfrom_addrlen); \
});})

#define recv(sock, buf, len, flags) ({ \
  LONG _recv_sock = (sock); \
  APTR _recv_buf = (buf); \
  LONG _recv_len = (len); \
  LONG _recv_flags = (flags); \
  ({ \
  register char * _recv__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), LONG __asm("d1"), LONG __asm("d2"))) \
  (_recv__bn - 78))(_recv__bn, _recv_sock, _recv_buf, _recv_len, _recv_flags); \
});})

#define shutdown(sock, how) ({ \
  LONG _shutdown_sock = (sock); \
  LONG _shutdown_how = (how); \
  ({ \
  register char * _shutdown__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_shutdown__bn - 84))(_shutdown__bn, _shutdown_sock, _shutdown_how); \
});})

#define setsockopt(sock, level, optname, optval, optlen) ({ \
  LONG _setsockopt_sock = (sock); \
  LONG _setsockopt_level = (level); \
  LONG _setsockopt_optname = (optname); \
  APTR _setsockopt_optval = (optval); \
  LONG _setsockopt_optlen = (optlen); \
  ({ \
  register char * _setsockopt__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"), APTR __asm("a0"), LONG __asm("d3"))) \
  (_setsockopt__bn - 90))(_setsockopt__bn, _setsockopt_sock, _setsockopt_level, _setsockopt_optname, _setsockopt_optval, _setsockopt_optlen); \
});})

#define getsockopt(sock, level, optname, optval, optlen) ({ \
  LONG _getsockopt_sock = (sock); \
  LONG _getsockopt_level = (level); \
  LONG _getsockopt_optname = (optname); \
  APTR _getsockopt_optval = (optval); \
  socklen_t * _getsockopt_optlen = (optlen); \
  ({ \
  register char * _getsockopt__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"), APTR __asm("a0"), socklen_t * __asm("a1"))) \
  (_getsockopt__bn - 96))(_getsockopt__bn, _getsockopt_sock, _getsockopt_level, _getsockopt_optname, _getsockopt_optval, _getsockopt_optlen); \
});})

#define getsockname(sock, name, namelen) ({ \
  LONG _getsockname_sock = (sock); \
  struct sockaddr * _getsockname_name = (name); \
  socklen_t * _getsockname_namelen = (namelen); \
  ({ \
  register char * _getsockname__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct sockaddr * __asm("a0"), socklen_t * __asm("a1"))) \
  (_getsockname__bn - 102))(_getsockname__bn, _getsockname_sock, _getsockname_name, _getsockname_namelen); \
});})

#define getpeername(sock, name, namelen) ({ \
  LONG _getpeername_sock = (sock); \
  struct sockaddr * _getpeername_name = (name); \
  socklen_t * _getpeername_namelen = (namelen); \
  ({ \
  register char * _getpeername__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct sockaddr * __asm("a0"), socklen_t * __asm("a1"))) \
  (_getpeername__bn - 108))(_getpeername__bn, _getpeername_sock, _getpeername_name, _getpeername_namelen); \
});})

#define IoctlSocket(sock, req, argp) ({ \
  LONG _IoctlSocket_sock = (sock); \
  ULONG _IoctlSocket_req = (req); \
  APTR _IoctlSocket_argp = (argp); \
  ({ \
  register char * _IoctlSocket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), ULONG __asm("d1"), APTR __asm("a0"))) \
  (_IoctlSocket__bn - 114))(_IoctlSocket__bn, _IoctlSocket_sock, _IoctlSocket_req, _IoctlSocket_argp); \
});})

#define CloseSocket(sock) ({ \
  LONG _CloseSocket_sock = (sock); \
  ({ \
  register char * _CloseSocket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_CloseSocket__bn - 120))(_CloseSocket__bn, _CloseSocket_sock); \
});})

#define WaitSelect(nfds, read_fds, write_fds, except_fds, _timeout, signals) ({ \
  LONG _WaitSelect_nfds = (nfds); \
  APTR _WaitSelect_read_fds = (read_fds); \
  APTR _WaitSelect_write_fds = (write_fds); \
  APTR _WaitSelect_except_fds = (except_fds); \
  struct __timeval * _WaitSelect__timeout = (_timeout); \
  ULONG * _WaitSelect_signals = (signals); \
  ({ \
  register char * _WaitSelect__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), APTR __asm("a1"), APTR __asm("a2"), struct __timeval * __asm("a3"), ULONG * __asm("d1"))) \
  (_WaitSelect__bn - 126))(_WaitSelect__bn, _WaitSelect_nfds, _WaitSelect_read_fds, _WaitSelect_write_fds, _WaitSelect_except_fds, _WaitSelect__timeout, _WaitSelect_signals); \
});})

#define SetSocketSignals(int_mask, io_mask, urgent_mask) ({ \
  ULONG _SetSocketSignals_int_mask = (int_mask); \
  ULONG _SetSocketSignals_io_mask = (io_mask); \
  ULONG _SetSocketSignals_urgent_mask = (urgent_mask); \
  ({ \
  register char * _SetSocketSignals__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), ULONG __asm("d0"), ULONG __asm("d1"), ULONG __asm("d2"))) \
  (_SetSocketSignals__bn - 132))(_SetSocketSignals__bn, _SetSocketSignals_int_mask, _SetSocketSignals_io_mask, _SetSocketSignals_urgent_mask); \
});})

#define getdtablesize() ({ \
  register char * _getdtablesize__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_getdtablesize__bn - 138))(_getdtablesize__bn); \
})

#define ObtainSocket(id, domain, type, protocol) ({ \
  LONG _ObtainSocket_id = (id); \
  LONG _ObtainSocket_domain = (domain); \
  LONG _ObtainSocket_type = (type); \
  LONG _ObtainSocket_protocol = (protocol); \
  ({ \
  register char * _ObtainSocket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"), LONG __asm("d2"), LONG __asm("d3"))) \
  (_ObtainSocket__bn - 144))(_ObtainSocket__bn, _ObtainSocket_id, _ObtainSocket_domain, _ObtainSocket_type, _ObtainSocket_protocol); \
});})

#define ReleaseSocket(sock, id) ({ \
  LONG _ReleaseSocket_sock = (sock); \
  LONG _ReleaseSocket_id = (id); \
  ({ \
  register char * _ReleaseSocket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_ReleaseSocket__bn - 150))(_ReleaseSocket__bn, _ReleaseSocket_sock, _ReleaseSocket_id); \
});})

#define ReleaseCopyOfSocket(sock, id) ({ \
  LONG _ReleaseCopyOfSocket_sock = (sock); \
  LONG _ReleaseCopyOfSocket_id = (id); \
  ({ \
  register char * _ReleaseCopyOfSocket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_ReleaseCopyOfSocket__bn - 156))(_ReleaseCopyOfSocket__bn, _ReleaseCopyOfSocket_sock, _ReleaseCopyOfSocket_id); \
});})

#define Errno() ({ \
  register char * _Errno__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_Errno__bn - 162))(_Errno__bn); \
})

#define SetErrnoPtr(errno_ptr, size) ({ \
  APTR _SetErrnoPtr_errno_ptr = (errno_ptr); \
  LONG _SetErrnoPtr_size = (size); \
  ({ \
  register char * _SetErrnoPtr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), APTR __asm("a0"), LONG __asm("d0"))) \
  (_SetErrnoPtr__bn - 168))(_SetErrnoPtr__bn, _SetErrnoPtr_errno_ptr, _SetErrnoPtr_size); \
});})

#define Inet_NtoA(ip) ({ \
  LONG _Inet_NtoA_ip = (ip); \
  ({ \
  register char * _Inet_NtoA__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_Inet_NtoA__bn - 174))(_Inet_NtoA__bn, _Inet_NtoA_ip); \
});})

#define inet_addr(cp) ({ \
  STRPTR _inet_addr_cp = (cp); \
  ({ \
  register char * _inet_addr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((in_addr_t (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_inet_addr__bn - 180))(_inet_addr__bn, _inet_addr_cp); \
});})

#define Inet_LnaOf(in) ({ \
  LONG _Inet_LnaOf_in = (in); \
  ({ \
  register char * _Inet_LnaOf__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((in_addr_t (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_Inet_LnaOf__bn - 186))(_Inet_LnaOf__bn, _Inet_LnaOf_in); \
});})

#define Inet_NetOf(in) ({ \
  LONG _Inet_NetOf_in = (in); \
  ({ \
  register char * _Inet_NetOf__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((in_addr_t (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_Inet_NetOf__bn - 192))(_Inet_NetOf__bn, _Inet_NetOf_in); \
});})

#define Inet_MakeAddr(net, host) ({ \
  LONG _Inet_MakeAddr_net = (net); \
  LONG _Inet_MakeAddr_host = (host); \
  ({ \
  register char * _Inet_MakeAddr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((in_addr_t (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_Inet_MakeAddr__bn - 198))(_Inet_MakeAddr__bn, _Inet_MakeAddr_net, _Inet_MakeAddr_host); \
});})

#define inet_network(cp) ({ \
  STRPTR _inet_network_cp = (cp); \
  ({ \
  register char * _inet_network__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((in_addr_t (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_inet_network__bn - 204))(_inet_network__bn, _inet_network_cp); \
});})

#define gethostbyname(name) ({ \
  STRPTR _gethostbyname_name = (name); \
  ({ \
  register char * _gethostbyname__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct hostent * (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_gethostbyname__bn - 210))(_gethostbyname__bn, _gethostbyname_name); \
});})

#define gethostbyaddr(addr, len, type) ({ \
  STRPTR _gethostbyaddr_addr = (addr); \
  LONG _gethostbyaddr_len = (len); \
  LONG _gethostbyaddr_type = (type); \
  ({ \
  register char * _gethostbyaddr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct hostent * (*)(char * __asm("a6"), STRPTR __asm("a0"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_gethostbyaddr__bn - 216))(_gethostbyaddr__bn, _gethostbyaddr_addr, _gethostbyaddr_len, _gethostbyaddr_type); \
});})

#define getnetbyname(name) ({ \
  STRPTR _getnetbyname_name = (name); \
  ({ \
  register char * _getnetbyname__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct netent * (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_getnetbyname__bn - 222))(_getnetbyname__bn, _getnetbyname_name); \
});})

#define getnetbyaddr(net, type) ({ \
  LONG _getnetbyaddr_net = (net); \
  LONG _getnetbyaddr_type = (type); \
  ({ \
  register char * _getnetbyaddr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct netent * (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_getnetbyaddr__bn - 228))(_getnetbyaddr__bn, _getnetbyaddr_net, _getnetbyaddr_type); \
});})

#define getservbyname(name, proto) ({ \
  STRPTR _getservbyname_name = (name); \
  STRPTR _getservbyname_proto = (proto); \
  ({ \
  register char * _getservbyname__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct servent * (*)(char * __asm("a6"), STRPTR __asm("a0"), STRPTR __asm("a1"))) \
  (_getservbyname__bn - 234))(_getservbyname__bn, _getservbyname_name, _getservbyname_proto); \
});})

#define getservbyport(port, proto) ({ \
  LONG _getservbyport_port = (port); \
  STRPTR _getservbyport_proto = (proto); \
  ({ \
  register char * _getservbyport__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct servent * (*)(char * __asm("a6"), LONG __asm("d0"), STRPTR __asm("a0"))) \
  (_getservbyport__bn - 240))(_getservbyport__bn, _getservbyport_port, _getservbyport_proto); \
});})

#define getprotobyname(name) ({ \
  STRPTR _getprotobyname_name = (name); \
  ({ \
  register char * _getprotobyname__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct protoent * (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_getprotobyname__bn - 246))(_getprotobyname__bn, _getprotobyname_name); \
});})

#define getprotobynumber(proto) ({ \
  LONG _getprotobynumber_proto = (proto); \
  ({ \
  register char * _getprotobynumber__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct protoent * (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_getprotobynumber__bn - 252))(_getprotobynumber__bn, _getprotobynumber_proto); \
});})

#define vsyslog(pri, msg, args) ({ \
  LONG _vsyslog_pri = (pri); \
  STRPTR _vsyslog_msg = (msg); \
  APTR _vsyslog_args = (args); \
  ({ \
  register char * _vsyslog__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), LONG __asm("d0"), STRPTR __asm("a0"), APTR __asm("a1"))) \
  (_vsyslog__bn - 258))(_vsyslog__bn, _vsyslog_pri, _vsyslog_msg, _vsyslog_args); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ VOID ___syslog(struct Library * SocketBase, LONG pri, STRPTR msg, LONG args, ...)
{
  vsyslog(pri, msg, (APTR) &args);
}

#define syslog(pri, msg...) ___syslog(BSDSOCKET_BASE_NAME, pri, msg)
#endif

#define Dup2Socket(old_socket, new_socket) ({ \
  LONG _Dup2Socket_old_socket = (old_socket); \
  LONG _Dup2Socket_new_socket = (new_socket); \
  ({ \
  register char * _Dup2Socket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_Dup2Socket__bn - 264))(_Dup2Socket__bn, _Dup2Socket_old_socket, _Dup2Socket_new_socket); \
});})

#define sendmsg(sock, msg, flags) ({ \
  LONG _sendmsg_sock = (sock); \
  struct msghdr * _sendmsg_msg = (msg); \
  LONG _sendmsg_flags = (flags); \
  ({ \
  register char * _sendmsg__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct msghdr * __asm("a0"), LONG __asm("d1"))) \
  (_sendmsg__bn - 270))(_sendmsg__bn, _sendmsg_sock, _sendmsg_msg, _sendmsg_flags); \
});})

#define recvmsg(sock, msg, flags) ({ \
  LONG _recvmsg_sock = (sock); \
  struct msghdr * _recvmsg_msg = (msg); \
  LONG _recvmsg_flags = (flags); \
  ({ \
  register char * _recvmsg__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct msghdr * __asm("a0"), LONG __asm("d1"))) \
  (_recvmsg__bn - 276))(_recvmsg__bn, _recvmsg_sock, _recvmsg_msg, _recvmsg_flags); \
});})

#define gethostname(name, namelen) ({ \
  STRPTR _gethostname_name = (name); \
  LONG _gethostname_namelen = (namelen); \
  ({ \
  register char * _gethostname__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), LONG __asm("d0"))) \
  (_gethostname__bn - 282))(_gethostname__bn, _gethostname_name, _gethostname_namelen); \
});})

#define gethostid() ({ \
  register char * _gethostid__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((in_addr_t (*)(char * __asm("a6"))) \
  (_gethostid__bn - 288))(_gethostid__bn); \
})

#define SocketBaseTagList(tags) ({ \
  struct TagItem * _SocketBaseTagList_tags = (tags); \
  ({ \
  register char * _SocketBaseTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct TagItem * __asm("a0"))) \
  (_SocketBaseTagList__bn - 294))(_SocketBaseTagList__bn, _SocketBaseTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___SocketBaseTags(struct Library * SocketBase, Tag tags, ...)
{
  return SocketBaseTagList((struct TagItem *) &tags);
}

#define SocketBaseTags(tags...) ___SocketBaseTags(BSDSOCKET_BASE_NAME, tags)
#endif

#define GetSocketEvents(event_ptr) ({ \
  ULONG * _GetSocketEvents_event_ptr = (event_ptr); \
  ({ \
  register char * _GetSocketEvents__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), ULONG * __asm("a0"))) \
  (_GetSocketEvents__bn - 300))(_GetSocketEvents__bn, _GetSocketEvents_event_ptr); \
});})

#define bpf_open(channel) ({ \
  LONG _bpf_open_channel = (channel); \
  ({ \
  register char * _bpf_open__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_bpf_open__bn - 366))(_bpf_open__bn, _bpf_open_channel); \
});})

#define bpf_close(channel) ({ \
  LONG _bpf_close_channel = (channel); \
  ({ \
  register char * _bpf_close__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_bpf_close__bn - 372))(_bpf_close__bn, _bpf_close_channel); \
});})

#define bpf_read(channel, buffer, len) ({ \
  LONG _bpf_read_channel = (channel); \
  APTR _bpf_read_buffer = (buffer); \
  LONG _bpf_read_len = (len); \
  ({ \
  register char * _bpf_read__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), LONG __asm("d1"))) \
  (_bpf_read__bn - 378))(_bpf_read__bn, _bpf_read_channel, _bpf_read_buffer, _bpf_read_len); \
});})

#define bpf_write(channel, buffer, len) ({ \
  LONG _bpf_write_channel = (channel); \
  APTR _bpf_write_buffer = (buffer); \
  LONG _bpf_write_len = (len); \
  ({ \
  register char * _bpf_write__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), LONG __asm("d1"))) \
  (_bpf_write__bn - 384))(_bpf_write__bn, _bpf_write_channel, _bpf_write_buffer, _bpf_write_len); \
});})

#define bpf_set_notify_mask(channel, signal_mask) ({ \
  LONG _bpf_set_notify_mask_channel = (channel); \
  ULONG _bpf_set_notify_mask_signal_mask = (signal_mask); \
  ({ \
  register char * _bpf_set_notify_mask__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d1"), ULONG __asm("d0"))) \
  (_bpf_set_notify_mask__bn - 390))(_bpf_set_notify_mask__bn, _bpf_set_notify_mask_channel, _bpf_set_notify_mask_signal_mask); \
});})

#define bpf_set_interrupt_mask(channel, signal_mask) ({ \
  LONG _bpf_set_interrupt_mask_channel = (channel); \
  ULONG _bpf_set_interrupt_mask_signal_mask = (signal_mask); \
  ({ \
  register char * _bpf_set_interrupt_mask__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), ULONG __asm("d1"))) \
  (_bpf_set_interrupt_mask__bn - 396))(_bpf_set_interrupt_mask__bn, _bpf_set_interrupt_mask_channel, _bpf_set_interrupt_mask_signal_mask); \
});})

#define bpf_ioctl(channel, command, buffer) ({ \
  LONG _bpf_ioctl_channel = (channel); \
  ULONG _bpf_ioctl_command = (command); \
  APTR _bpf_ioctl_buffer = (buffer); \
  ({ \
  register char * _bpf_ioctl__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), ULONG __asm("d1"), APTR __asm("a0"))) \
  (_bpf_ioctl__bn - 402))(_bpf_ioctl__bn, _bpf_ioctl_channel, _bpf_ioctl_command, _bpf_ioctl_buffer); \
});})

#define bpf_data_waiting(channel) ({ \
  LONG _bpf_data_waiting_channel = (channel); \
  ({ \
  register char * _bpf_data_waiting__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_bpf_data_waiting__bn - 408))(_bpf_data_waiting__bn, _bpf_data_waiting_channel); \
});})

#define AddRouteTagList(tags) ({ \
  struct TagItem * _AddRouteTagList_tags = (tags); \
  ({ \
  register char * _AddRouteTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct TagItem * __asm("a0"))) \
  (_AddRouteTagList__bn - 414))(_AddRouteTagList__bn, _AddRouteTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___AddRouteTags(struct Library * SocketBase, Tag tags, ...)
{
  return AddRouteTagList((struct TagItem *) &tags);
}

#define AddRouteTags(tags...) ___AddRouteTags(BSDSOCKET_BASE_NAME, tags)
#endif

#define DeleteRouteTagList(tags) ({ \
  struct TagItem * _DeleteRouteTagList_tags = (tags); \
  ({ \
  register char * _DeleteRouteTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct TagItem * __asm("a0"))) \
  (_DeleteRouteTagList__bn - 420))(_DeleteRouteTagList__bn, _DeleteRouteTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___DeleteRouteTags(struct Library * SocketBase, Tag tags, ...)
{
  return DeleteRouteTagList((struct TagItem *) &tags);
}

#define DeleteRouteTags(tags...) ___DeleteRouteTags(BSDSOCKET_BASE_NAME, tags)
#endif

#define FreeRouteInfo(buf) ({ \
  struct rt_msghdr * _FreeRouteInfo_buf = (buf); \
  ({ \
  register char * _FreeRouteInfo__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct rt_msghdr * __asm("a0"))) \
  (_FreeRouteInfo__bn - 432))(_FreeRouteInfo__bn, _FreeRouteInfo_buf); \
});})

#define GetRouteInfo(address_family, flags) ({ \
  LONG _GetRouteInfo_address_family = (address_family); \
  LONG _GetRouteInfo_flags = (flags); \
  ({ \
  register char * _GetRouteInfo__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct rt_msghdr * (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_GetRouteInfo__bn - 438))(_GetRouteInfo__bn, _GetRouteInfo_address_family, _GetRouteInfo_flags); \
});})

#define AddInterfaceTagList(interface_name, device_name, unit, tags) ({ \
  STRPTR _AddInterfaceTagList_interface_name = (interface_name); \
  STRPTR _AddInterfaceTagList_device_name = (device_name); \
  LONG _AddInterfaceTagList_unit = (unit); \
  struct TagItem * _AddInterfaceTagList_tags = (tags); \
  ({ \
  register char * _AddInterfaceTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), STRPTR __asm("a1"), LONG __asm("d0"), struct TagItem * __asm("a2"))) \
  (_AddInterfaceTagList__bn - 444))(_AddInterfaceTagList__bn, _AddInterfaceTagList_interface_name, _AddInterfaceTagList_device_name, _AddInterfaceTagList_unit, _AddInterfaceTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___AddInterfaceTags(struct Library * SocketBase, STRPTR interface_name, STRPTR device_name, LONG unit, Tag tags, ...)
{
  return AddInterfaceTagList(interface_name, device_name, unit, (struct TagItem *) &tags);
}

#define AddInterfaceTags(interface_name, device_name, unit...) ___AddInterfaceTags(BSDSOCKET_BASE_NAME, interface_name, device_name, unit)
#endif

#define ConfigureInterfaceTagList(interface_name, tags) ({ \
  STRPTR _ConfigureInterfaceTagList_interface_name = (interface_name); \
  struct TagItem * _ConfigureInterfaceTagList_tags = (tags); \
  ({ \
  register char * _ConfigureInterfaceTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), struct TagItem * __asm("a1"))) \
  (_ConfigureInterfaceTagList__bn - 450))(_ConfigureInterfaceTagList__bn, _ConfigureInterfaceTagList_interface_name, _ConfigureInterfaceTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___ConfigureInterfaceTags(struct Library * SocketBase, STRPTR interface_name, Tag tags, ...)
{
  return ConfigureInterfaceTagList(interface_name, (struct TagItem *) &tags);
}

#define ConfigureInterfaceTags(interface_name...) ___ConfigureInterfaceTags(BSDSOCKET_BASE_NAME, interface_name)
#endif

#define ReleaseInterfaceList(list) ({ \
  struct List * _ReleaseInterfaceList_list = (list); \
  ({ \
  register char * _ReleaseInterfaceList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct List * __asm("a0"))) \
  (_ReleaseInterfaceList__bn - 456))(_ReleaseInterfaceList__bn, _ReleaseInterfaceList_list); \
});})

#define ObtainInterfaceList() ({ \
  register char * _ObtainInterfaceList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct List * (*)(char * __asm("a6"))) \
  (_ObtainInterfaceList__bn - 462))(_ObtainInterfaceList__bn); \
})

#define QueryInterfaceTagList(interface_name, tags) ({ \
  STRPTR _QueryInterfaceTagList_interface_name = (interface_name); \
  struct TagItem * _QueryInterfaceTagList_tags = (tags); \
  ({ \
  register char * _QueryInterfaceTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), struct TagItem * __asm("a1"))) \
  (_QueryInterfaceTagList__bn - 468))(_QueryInterfaceTagList__bn, _QueryInterfaceTagList_interface_name, _QueryInterfaceTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___QueryInterfaceTags(struct Library * SocketBase, STRPTR interface_name, Tag tags, ...)
{
  return QueryInterfaceTagList(interface_name, (struct TagItem *) &tags);
}

#define QueryInterfaceTags(interface_name...) ___QueryInterfaceTags(BSDSOCKET_BASE_NAME, interface_name)
#endif

#define CreateAddrAllocMessageA(version, protocol, interface_name, result_ptr, tags) ({ \
  LONG _CreateAddrAllocMessageA_version = (version); \
  LONG _CreateAddrAllocMessageA_protocol = (protocol); \
  STRPTR _CreateAddrAllocMessageA_interface_name = (interface_name); \
  struct AddressAllocationMessage ** _CreateAddrAllocMessageA_result_ptr = (result_ptr); \
  struct TagItem * _CreateAddrAllocMessageA_tags = (tags); \
  ({ \
  register char * _CreateAddrAllocMessageA__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"), STRPTR __asm("a0"), struct AddressAllocationMessage ** __asm("a1"), struct TagItem * __asm("a2"))) \
  (_CreateAddrAllocMessageA__bn - 474))(_CreateAddrAllocMessageA__bn, _CreateAddrAllocMessageA_version, _CreateAddrAllocMessageA_protocol, _CreateAddrAllocMessageA_interface_name, _CreateAddrAllocMessageA_result_ptr, _CreateAddrAllocMessageA_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___CreateAddrAllocMessage(struct Library * SocketBase, LONG version, LONG protocol, STRPTR interface_name, struct AddressAllocationMessage ** result_ptr, Tag tags, ...)
{
  return CreateAddrAllocMessageA(version, protocol, interface_name, result_ptr, (struct TagItem *) &tags);
}

#define CreateAddrAllocMessage(version, protocol, interface_name, result_ptr...) ___CreateAddrAllocMessage(BSDSOCKET_BASE_NAME, version, protocol, interface_name, result_ptr)
#endif

#define DeleteAddrAllocMessage(aam) ({ \
  struct AddressAllocationMessage * _DeleteAddrAllocMessage_aam = (aam); \
  ({ \
  register char * _DeleteAddrAllocMessage__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct AddressAllocationMessage * __asm("a0"))) \
  (_DeleteAddrAllocMessage__bn - 480))(_DeleteAddrAllocMessage__bn, _DeleteAddrAllocMessage_aam); \
});})

#define BeginInterfaceConfig(message) ({ \
  struct AddressAllocationMessage * _BeginInterfaceConfig_message = (message); \
  ({ \
  register char * _BeginInterfaceConfig__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct AddressAllocationMessage * __asm("a0"))) \
  (_BeginInterfaceConfig__bn - 486))(_BeginInterfaceConfig__bn, _BeginInterfaceConfig_message); \
});})

#define AbortInterfaceConfig(message) ({ \
  struct AddressAllocationMessage * _AbortInterfaceConfig_message = (message); \
  ({ \
  register char * _AbortInterfaceConfig__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct AddressAllocationMessage * __asm("a0"))) \
  (_AbortInterfaceConfig__bn - 492))(_AbortInterfaceConfig__bn, _AbortInterfaceConfig_message); \
});})

#define AddNetMonitorHookTagList(type, hook, tags) ({ \
  LONG _AddNetMonitorHookTagList_type = (type); \
  struct Hook * _AddNetMonitorHookTagList_hook = (hook); \
  struct TagItem * _AddNetMonitorHookTagList_tags = (tags); \
  ({ \
  register char * _AddNetMonitorHookTagList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), struct Hook * __asm("a0"), struct TagItem * __asm("a1"))) \
  (_AddNetMonitorHookTagList__bn - 498))(_AddNetMonitorHookTagList__bn, _AddNetMonitorHookTagList_type, _AddNetMonitorHookTagList_hook, _AddNetMonitorHookTagList_tags); \
});})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___AddNetMonitorHookTags(struct Library * SocketBase, LONG type, struct Hook * hook, Tag tags, ...)
{
  return AddNetMonitorHookTagList(type, hook, (struct TagItem *) &tags);
}

#define AddNetMonitorHookTags(type, hook...) ___AddNetMonitorHookTags(BSDSOCKET_BASE_NAME, type, hook)
#endif

#define RemoveNetMonitorHook(hook) ({ \
  struct Hook * _RemoveNetMonitorHook_hook = (hook); \
  ({ \
  register char * _RemoveNetMonitorHook__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct Hook * __asm("a0"))) \
  (_RemoveNetMonitorHook__bn - 504))(_RemoveNetMonitorHook__bn, _RemoveNetMonitorHook_hook); \
});})

#define GetNetworkStatistics(type, version, destination, size) ({ \
  LONG _GetNetworkStatistics_type = (type); \
  LONG _GetNetworkStatistics_version = (version); \
  APTR _GetNetworkStatistics_destination = (destination); \
  LONG _GetNetworkStatistics_size = (size); \
  ({ \
  register char * _GetNetworkStatistics__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), LONG __asm("d1"), APTR __asm("a0"), LONG __asm("d2"))) \
  (_GetNetworkStatistics__bn - 510))(_GetNetworkStatistics__bn, _GetNetworkStatistics_type, _GetNetworkStatistics_version, _GetNetworkStatistics_destination, _GetNetworkStatistics_size); \
});})

#define AddDomainNameServer(address) ({ \
  STRPTR _AddDomainNameServer_address = (address); \
  ({ \
  register char * _AddDomainNameServer__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_AddDomainNameServer__bn - 516))(_AddDomainNameServer__bn, _AddDomainNameServer_address); \
});})

#define RemoveDomainNameServer(address) ({ \
  STRPTR _RemoveDomainNameServer_address = (address); \
  ({ \
  register char * _RemoveDomainNameServer__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_RemoveDomainNameServer__bn - 522))(_RemoveDomainNameServer__bn, _RemoveDomainNameServer_address); \
});})

#define ReleaseDomainNameServerList(list) ({ \
  struct List * _ReleaseDomainNameServerList_list = (list); \
  ({ \
  register char * _ReleaseDomainNameServerList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct List * __asm("a0"))) \
  (_ReleaseDomainNameServerList__bn - 528))(_ReleaseDomainNameServerList__bn, _ReleaseDomainNameServerList_list); \
});})

#define ObtainDomainNameServerList() ({ \
  register char * _ObtainDomainNameServerList__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct List * (*)(char * __asm("a6"))) \
  (_ObtainDomainNameServerList__bn - 534))(_ObtainDomainNameServerList__bn); \
})

#define setnetent(stay_open) ({ \
  LONG _setnetent_stay_open = (stay_open); \
  ({ \
  register char * _setnetent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_setnetent__bn - 540))(_setnetent__bn, _setnetent_stay_open); \
});})

#define endnetent() ({ \
  register char * _endnetent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_endnetent__bn - 546))(_endnetent__bn); \
})

#define getnetent() ({ \
  register char * _getnetent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct netent * (*)(char * __asm("a6"))) \
  (_getnetent__bn - 552))(_getnetent__bn); \
})

#define setprotoent(stay_open) ({ \
  LONG _setprotoent_stay_open = (stay_open); \
  ({ \
  register char * _setprotoent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_setprotoent__bn - 558))(_setprotoent__bn, _setprotoent_stay_open); \
});})

#define endprotoent() ({ \
  register char * _endprotoent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_endprotoent__bn - 564))(_endprotoent__bn); \
})

#define getprotoent() ({ \
  register char * _getprotoent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct protoent * (*)(char * __asm("a6"))) \
  (_getprotoent__bn - 570))(_getprotoent__bn); \
})

#define setservent(stay_open) ({ \
  LONG _setservent_stay_open = (stay_open); \
  ({ \
  register char * _setservent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_setservent__bn - 576))(_setservent__bn, _setservent_stay_open); \
});})

#define endservent() ({ \
  register char * _endservent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"))) \
  (_endservent__bn - 582))(_endservent__bn); \
})

#define getservent() ({ \
  register char * _getservent__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct servent * (*)(char * __asm("a6"))) \
  (_getservent__bn - 588))(_getservent__bn); \
})

#define inet_aton(cp, addr) ({ \
  STRPTR _inet_aton_cp = (cp); \
  struct in_addr * _inet_aton_addr = (addr); \
  ({ \
  register char * _inet_aton__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), struct in_addr * __asm("a1"))) \
  (_inet_aton__bn - 594))(_inet_aton__bn, _inet_aton_cp, _inet_aton_addr); \
});})

#define inet_ntop(af, src, dst, size) ({ \
  LONG _inet_ntop_af = (af); \
  APTR _inet_ntop_src = (src); \
  STRPTR _inet_ntop_dst = (dst); \
  LONG _inet_ntop_size = (size); \
  ({ \
  register char * _inet_ntop__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"), LONG __asm("d0"), APTR __asm("a0"), STRPTR __asm("a1"), LONG __asm("d1"))) \
  (_inet_ntop__bn - 600))(_inet_ntop__bn, _inet_ntop_af, _inet_ntop_src, _inet_ntop_dst, _inet_ntop_size); \
});})

#define inet_pton(af, src, dst) ({ \
  LONG _inet_pton_af = (af); \
  STRPTR _inet_pton_src = (src); \
  APTR _inet_pton_dst = (dst); \
  ({ \
  register char * _inet_pton__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"), STRPTR __asm("a0"), APTR __asm("a1"))) \
  (_inet_pton__bn - 606))(_inet_pton__bn, _inet_pton_af, _inet_pton_src, _inet_pton_dst); \
});})

#define In_LocalAddr(address) ({ \
  LONG _In_LocalAddr_address = (address); \
  ({ \
  register char * _In_LocalAddr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_In_LocalAddr__bn - 612))(_In_LocalAddr__bn, _In_LocalAddr_address); \
});})

#define In_CanForward(address) ({ \
  LONG _In_CanForward_address = (address); \
  ({ \
  register char * _In_CanForward__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_In_CanForward__bn - 618))(_In_CanForward__bn, _In_CanForward_address); \
});})

#define mbuf_copym(m, off, len) ({ \
  struct mbuf * _mbuf_copym_m = (m); \
  LONG _mbuf_copym_off = (off); \
  LONG _mbuf_copym_len = (len); \
  ({ \
  register char * _mbuf_copym__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct mbuf * (*)(char * __asm("a6"), struct mbuf * __asm("a0"), LONG __asm("d0"), LONG __asm("d1"))) \
  (_mbuf_copym__bn - 624))(_mbuf_copym__bn, _mbuf_copym_m, _mbuf_copym_off, _mbuf_copym_len); \
});})

#define mbuf_copyback(m, off, len, cp) ({ \
  struct mbuf * _mbuf_copyback_m = (m); \
  LONG _mbuf_copyback_off = (off); \
  LONG _mbuf_copyback_len = (len); \
  APTR _mbuf_copyback_cp = (cp); \
  ({ \
  register char * _mbuf_copyback__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct mbuf * __asm("a0"), LONG __asm("d0"), LONG __asm("d1"), APTR __asm("a1"))) \
  (_mbuf_copyback__bn - 630))(_mbuf_copyback__bn, _mbuf_copyback_m, _mbuf_copyback_off, _mbuf_copyback_len, _mbuf_copyback_cp); \
});})

#define mbuf_copydata(m, off, len, cp) ({ \
  struct mbuf * _mbuf_copydata_m = (m); \
  LONG _mbuf_copydata_off = (off); \
  LONG _mbuf_copydata_len = (len); \
  APTR _mbuf_copydata_cp = (cp); \
  ({ \
  register char * _mbuf_copydata__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct mbuf * __asm("a0"), LONG __asm("d0"), LONG __asm("d1"), APTR __asm("a1"))) \
  (_mbuf_copydata__bn - 636))(_mbuf_copydata__bn, _mbuf_copydata_m, _mbuf_copydata_off, _mbuf_copydata_len, _mbuf_copydata_cp); \
});})

#define mbuf_free(m) ({ \
  struct mbuf * _mbuf_free_m = (m); \
  ({ \
  register char * _mbuf_free__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct mbuf * (*)(char * __asm("a6"), struct mbuf * __asm("a0"))) \
  (_mbuf_free__bn - 642))(_mbuf_free__bn, _mbuf_free_m); \
});})

#define mbuf_freem(m) ({ \
  struct mbuf * _mbuf_freem_m = (m); \
  ({ \
  register char * _mbuf_freem__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct mbuf * __asm("a0"))) \
  (_mbuf_freem__bn - 648))(_mbuf_freem__bn, _mbuf_freem_m); \
});})

#define mbuf_get() ({ \
  register char * _mbuf_get__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct mbuf * (*)(char * __asm("a6"))) \
  (_mbuf_get__bn - 654))(_mbuf_get__bn); \
})

#define mbuf_gethdr() ({ \
  register char * _mbuf_gethdr__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct mbuf * (*)(char * __asm("a6"))) \
  (_mbuf_gethdr__bn - 660))(_mbuf_gethdr__bn); \
})

#define mbuf_prepend(m, len) ({ \
  struct mbuf * _mbuf_prepend_m = (m); \
  LONG _mbuf_prepend_len = (len); \
  ({ \
  register char * _mbuf_prepend__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct mbuf * (*)(char * __asm("a6"), struct mbuf * __asm("a0"), LONG __asm("d0"))) \
  (_mbuf_prepend__bn - 666))(_mbuf_prepend__bn, _mbuf_prepend_m, _mbuf_prepend_len); \
});})

#define mbuf_cat(m, n) ({ \
  struct mbuf * _mbuf_cat_m = (m); \
  struct mbuf * _mbuf_cat_n = (n); \
  ({ \
  register char * _mbuf_cat__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct mbuf * __asm("a0"), struct mbuf * __asm("a1"))) \
  (_mbuf_cat__bn - 672))(_mbuf_cat__bn, _mbuf_cat_m, _mbuf_cat_n); \
});})

#define mbuf_adj(mp, req_len) ({ \
  struct mbuf * _mbuf_adj_mp = (mp); \
  LONG _mbuf_adj_req_len = (req_len); \
  ({ \
  register char * _mbuf_adj__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct mbuf * __asm("a0"), LONG __asm("d0"))) \
  (_mbuf_adj__bn - 678))(_mbuf_adj__bn, _mbuf_adj_mp, _mbuf_adj_req_len); \
});})

#define mbuf_pullup(m, len) ({ \
  struct mbuf * _mbuf_pullup_m = (m); \
  LONG _mbuf_pullup_len = (len); \
  ({ \
  register char * _mbuf_pullup__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct mbuf * (*)(char * __asm("a6"), struct mbuf * __asm("a0"), LONG __asm("d0"))) \
  (_mbuf_pullup__bn - 684))(_mbuf_pullup__bn, _mbuf_pullup_m, _mbuf_pullup_len); \
});})

#define ProcessIsServer(pr) ({ \
  struct Process * _ProcessIsServer_pr = (pr); \
  ({ \
  register char * _ProcessIsServer__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct Process * __asm("a0"))) \
  (_ProcessIsServer__bn - 690))(_ProcessIsServer__bn, _ProcessIsServer_pr); \
});})

#define ObtainServerSocket() ({ \
  register char * _ObtainServerSocket__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"))) \
  (_ObtainServerSocket__bn - 696))(_ObtainServerSocket__bn); \
})

#define GetDefaultDomainName(buffer, buffer_size) ({ \
  STRPTR _GetDefaultDomainName_buffer = (buffer); \
  LONG _GetDefaultDomainName_buffer_size = (buffer_size); \
  ({ \
  register char * _GetDefaultDomainName__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), STRPTR __asm("a0"), LONG __asm("d0"))) \
  (_GetDefaultDomainName__bn - 702))(_GetDefaultDomainName__bn, _GetDefaultDomainName_buffer, _GetDefaultDomainName_buffer_size); \
});})

#define SetDefaultDomainName(buffer) ({ \
  STRPTR _SetDefaultDomainName_buffer = (buffer); \
  ({ \
  register char * _SetDefaultDomainName__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), STRPTR __asm("a0"))) \
  (_SetDefaultDomainName__bn - 708))(_SetDefaultDomainName__bn, _SetDefaultDomainName_buffer); \
});})

#define ObtainRoadshowData(access) ({ \
  LONG _ObtainRoadshowData_access = (access); \
  ({ \
  register char * _ObtainRoadshowData__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct List * (*)(char * __asm("a6"), LONG __asm("d0"))) \
  (_ObtainRoadshowData__bn - 714))(_ObtainRoadshowData__bn, _ObtainRoadshowData_access); \
});})

#define ReleaseRoadshowData(list) ({ \
  struct List * _ReleaseRoadshowData_list = (list); \
  ({ \
  register char * _ReleaseRoadshowData__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct List * __asm("a0"))) \
  (_ReleaseRoadshowData__bn - 720))(_ReleaseRoadshowData__bn, _ReleaseRoadshowData_list); \
});})

#define ChangeRoadshowData(list, name, length, data) ({ \
  struct List * _ChangeRoadshowData_list = (list); \
  STRPTR _ChangeRoadshowData_name = (name); \
  ULONG _ChangeRoadshowData_length = (length); \
  APTR _ChangeRoadshowData_data = (data); \
  ({ \
  register char * _ChangeRoadshowData__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((BOOL (*)(char * __asm("a6"), struct List * __asm("a0"), STRPTR __asm("a1"), ULONG __asm("d0"), APTR __asm("a2"))) \
  (_ChangeRoadshowData__bn - 726))(_ChangeRoadshowData__bn, _ChangeRoadshowData_list, _ChangeRoadshowData_name, _ChangeRoadshowData_length, _ChangeRoadshowData_data); \
});})

#define RemoveInterface(interface_name, force) ({ \
  STRPTR _RemoveInterface_interface_name = (interface_name); \
  LONG _RemoveInterface_force = (force); \
  ({ \
  register char * _RemoveInterface__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), LONG __asm("d0"))) \
  (_RemoveInterface__bn - 732))(_RemoveInterface__bn, _RemoveInterface_interface_name, _RemoveInterface_force); \
});})

#define gethostbyname_r(name, hp, buf, buflen, he) ({ \
  STRPTR _gethostbyname_r_name = (name); \
  struct hostent * _gethostbyname_r_hp = (hp); \
  APTR _gethostbyname_r_buf = (buf); \
  ULONG _gethostbyname_r_buflen = (buflen); \
  LONG * _gethostbyname_r_he = (he); \
  ({ \
  register char * _gethostbyname_r__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct hostent * (*)(char * __asm("a6"), STRPTR __asm("a0"), struct hostent * __asm("a1"), APTR __asm("a2"), ULONG __asm("d0"), LONG * __asm("a3"))) \
  (_gethostbyname_r__bn - 738))(_gethostbyname_r__bn, _gethostbyname_r_name, _gethostbyname_r_hp, _gethostbyname_r_buf, _gethostbyname_r_buflen, _gethostbyname_r_he); \
});})

#define gethostbyaddr_r(addr, len, type, hp, buf, buflen, he) ({ \
  STRPTR _gethostbyaddr_r_addr = (addr); \
  LONG _gethostbyaddr_r_len = (len); \
  LONG _gethostbyaddr_r_type = (type); \
  struct hostent * _gethostbyaddr_r_hp = (hp); \
  APTR _gethostbyaddr_r_buf = (buf); \
  ULONG _gethostbyaddr_r_buflen = (buflen); \
  LONG * _gethostbyaddr_r_he = (he); \
  ({ \
  register char * _gethostbyaddr_r__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((struct hostent * (*)(char * __asm("a6"), STRPTR __asm("a0"), LONG __asm("d0"), LONG __asm("d1"), struct hostent * __asm("a1"), APTR __asm("a2"), ULONG __asm("d2"), LONG * __asm("a3"))) \
  (_gethostbyaddr_r__bn - 744))(_gethostbyaddr_r__bn, _gethostbyaddr_r_addr, _gethostbyaddr_r_len, _gethostbyaddr_r_type, _gethostbyaddr_r_hp, _gethostbyaddr_r_buf, _gethostbyaddr_r_buflen, _gethostbyaddr_r_he); \
});})

#define freeaddrinfo(ai) ({ \
  struct addrinfo * _freeaddrinfo_ai = (ai); \
  ({ \
  register char * _freeaddrinfo__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((VOID (*)(char * __asm("a6"), struct addrinfo * __asm("a0"))) \
  (_freeaddrinfo__bn - 804))(_freeaddrinfo__bn, _freeaddrinfo_ai); \
});})

#define getaddrinfo(hostname, servname, hints, res) ({ \
  STRPTR _getaddrinfo_hostname = (hostname); \
  STRPTR _getaddrinfo_servname = (servname); \
  struct addrinfo * _getaddrinfo_hints = (hints); \
  struct addrinfo ** _getaddrinfo_res = (res); \
  ({ \
  register char * _getaddrinfo__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), STRPTR __asm("a0"), STRPTR __asm("a1"), struct addrinfo * __asm("a2"), struct addrinfo ** __asm("a3"))) \
  (_getaddrinfo__bn - 810))(_getaddrinfo__bn, _getaddrinfo_hostname, _getaddrinfo_servname, _getaddrinfo_hints, _getaddrinfo_res); \
});})

#define gai_strerror(errnum) ({ \
  LONG _gai_strerror_errnum = (errnum); \
  ({ \
  register char * _gai_strerror__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((STRPTR (*)(char * __asm("a6"), LONG __asm("a0"))) \
  (_gai_strerror__bn - 816))(_gai_strerror__bn, _gai_strerror_errnum); \
});})

#define getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) ({ \
  struct sockaddr * _getnameinfo_sa = (sa); \
  ULONG _getnameinfo_salen = (salen); \
  STRPTR _getnameinfo_host = (host); \
  ULONG _getnameinfo_hostlen = (hostlen); \
  STRPTR _getnameinfo_serv = (serv); \
  ULONG _getnameinfo_servlen = (servlen); \
  ULONG _getnameinfo_flags = (flags); \
  ({ \
  register char * _getnameinfo__bn __asm("a6") = (char *) (BSDSOCKET_BASE_NAME);\
  ((LONG (*)(char * __asm("a6"), struct sockaddr * __asm("a0"), ULONG __asm("d0"), STRPTR __asm("a1"), ULONG __asm("d1"), STRPTR __asm("a2"), ULONG __asm("d2"), ULONG __asm("d3"))) \
  (_getnameinfo__bn - 822))(_getnameinfo__bn, _getnameinfo_sa, _getnameinfo_salen, _getnameinfo_host, _getnameinfo_hostlen, _getnameinfo_serv, _getnameinfo_servlen, _getnameinfo_flags); \
});})

#endif /*  _INLINE_BSDSOCKET_H  */
