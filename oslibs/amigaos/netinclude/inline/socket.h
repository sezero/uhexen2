#ifndef _INLINE_SOCKET_H
#define _INLINE_SOCKET_H

#ifndef CLIB_SOCKET_PROTOS_H
#define CLIB_SOCKET_PROTOS_H
#endif

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  SYS_TYPES_H
#include <sys/types.h>
#endif
#ifndef  SYS_TIME_H
#include <sys/time.h>
#endif
#ifndef  SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifndef  NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef SOCKET_BASE_NAME
#define SOCKET_BASE_NAME SocketBase
#endif

#define socket(domain, type, protocol) \
	LP3(0x1e, LONG, socket, LONG, domain, d0, LONG, type, d1, LONG, protocol, d2, \
	, SOCKET_BASE_NAME)

#define bind(s, name, namelen) \
	LP3(0x24, LONG, bind, LONG, s, d0, const struct sockaddr *, name, a0, LONG, namelen, d1, \
	, SOCKET_BASE_NAME)

#define listen(s, backlog) \
	LP2(0x2a, LONG, listen, LONG, s, d0, LONG, backlog, d1, \
	, SOCKET_BASE_NAME)

#define accept(s, addr, addrlen) \
	LP3(0x30, LONG, accept, LONG, s, d0, struct sockaddr *, addr, a0, LONG *, addrlen, a1, \
	, SOCKET_BASE_NAME)

#define connect(s, name, namelen) \
	LP3(0x36, LONG, connect, LONG, s, d0, const struct sockaddr *, name, a0, LONG, namelen, d1, \
	, SOCKET_BASE_NAME)

#define sendto(s, msg, len, flags, to, tolen) \
	LP6(0x3c, LONG, sendto, LONG, s, d0, const UBYTE *, msg, a0, LONG, len, d1, LONG, flags, d2, const struct sockaddr *, to, a1, LONG, tolen, d3, \
	, SOCKET_BASE_NAME)

#define send(s, msg, len, flags) \
	LP4(0x42, LONG, send, LONG, s, d0, const UBYTE *, msg, a0, LONG, len, d1, LONG, flags, d2, \
	, SOCKET_BASE_NAME)

#define recvfrom(s, buf, len, flags, from, fromlen) \
	LP6(0x48, LONG, recvfrom, LONG, s, d0, UBYTE *, buf, a0, LONG, len, d1, LONG, flags, d2, struct sockaddr *, from, a1, LONG *, fromlen, a2, \
	, SOCKET_BASE_NAME)

#define recv(s, buf, len, flags) \
	LP4(0x4e, LONG, recv, LONG, s, d0, UBYTE *, buf, a0, LONG, len, d1, LONG, flags, d2, \
	, SOCKET_BASE_NAME)

#define shutdown(s, how) \
	LP2(0x54, LONG, shutdown, LONG, s, d0, LONG, how, d1, \
	, SOCKET_BASE_NAME)

#define setsockopt(s, level, optname, optval, optlen) \
	LP5(0x5a, LONG, setsockopt, LONG, s, d0, LONG, level, d1, LONG, optname, d2, const void *, optval, a0, LONG, optlen, d3, \
	, SOCKET_BASE_NAME)

#define getsockopt(s, level, optname, optval, optlen) \
	LP5(0x60, LONG, getsockopt, LONG, s, d0, LONG, level, d1, LONG, optname, d2, void *, optval, a0, LONG *, optlen, a1, \
	, SOCKET_BASE_NAME)

#define getsockname(s, hostname, namelen) \
	LP3(0x66, LONG, getsockname, LONG, s, d0, struct sockaddr *, hostname, a0, LONG *, namelen, a1, \
	, SOCKET_BASE_NAME)

#define getpeername(s, hostname, namelen) \
	LP3(0x6c, LONG, getpeername, LONG, s, d0, struct sockaddr *, hostname, a0, LONG *, namelen, a1, \
	, SOCKET_BASE_NAME)

#define IoctlSocket(d, request, argp) \
	LP3(0x72, LONG, IoctlSocket, LONG, d, d0, ULONG, request, d1, char *, argp, a0, \
	, SOCKET_BASE_NAME)

#define CloseSocket(d) \
	LP1(0x78, LONG, CloseSocket, LONG, d, d0, \
	, SOCKET_BASE_NAME)

#define WaitSelect(nfds, readfds, writefds, execptfds, timeout, maskp) \
	LP6(0x7e, LONG, WaitSelect, LONG, nfds, d0, fd_set *, readfds, a0, fd_set *, writefds, a1, fd_set *, execptfds, a2, struct timeval *, timeout, a3, ULONG *, maskp, d1, \
	, SOCKET_BASE_NAME)

/* uhexen2: uglified param names (otherwise SIGIO is a macro expanding to 23 !!!) */
#define SetSocketSignals(_SIGINTR, _SIGIO, _SIGURG) \
	LP3NR(0x84, SetSocketSignals, ULONG, _SIGINTR, d0, ULONG, _SIGIO, d1, ULONG, _SIGURG, d2, \
	, SOCKET_BASE_NAME)

#define getdtablesize() \
	LP0(0x8a, LONG, getdtablesize, \
	, SOCKET_BASE_NAME)

#define ObtainSocket(id, domain, type, protocol) \
	LP4(0x90, LONG, ObtainSocket, LONG, id, d0, LONG, domain, d1, LONG, type, d2, LONG, protocol, d3, \
	, SOCKET_BASE_NAME)

#define ReleaseSocket(fd, id) \
	LP2(0x96, LONG, ReleaseSocket, LONG, fd, d0, LONG, id, d1, \
	, SOCKET_BASE_NAME)

#define ReleaseCopyOfSocket(fd, id) \
	LP2(0x9c, LONG, ReleaseCopyOfSocket, LONG, fd, d0, LONG, id, d1, \
	, SOCKET_BASE_NAME)

#define Errno() \
	LP0(0xa2, LONG, Errno, \
	, SOCKET_BASE_NAME)

#define SetErrnoPtr(errno_p, size) \
	LP2(0xa8, LONG, SetErrnoPtr, void *, errno_p, a0, LONG, size, d0, \
	, SOCKET_BASE_NAME)

#define Inet_NtoA(in) \
	LP1(0xae, char *, Inet_NtoA, ULONG, in, d0, \
	, SOCKET_BASE_NAME)

#define inet_addr(cp) \
	LP1(0xb4, ULONG, inet_addr, const UBYTE *, cp, a0, \
	, SOCKET_BASE_NAME)

#define Inet_LnaOf(in) \
	LP1(0xba, ULONG, Inet_LnaOf, LONG, in, d0, \
	, SOCKET_BASE_NAME)

#define Inet_NetOf(in) \
	LP1(0xc0, ULONG, Inet_NetOf, LONG, in, d0, \
	, SOCKET_BASE_NAME)

#define Inet_MakeAddr(net, host) \
	LP2(0xc6, ULONG, Inet_MakeAddr, ULONG, net, d0, ULONG, host, d1, \
	, SOCKET_BASE_NAME)

#define inet_network(cp) \
	LP1(0xcc, ULONG, inet_network, const UBYTE *, cp, a0, \
	, SOCKET_BASE_NAME)

#define gethostbyname(name) \
	LP1(0xd2, struct hostent  *, gethostbyname, const UBYTE *, name, a0, \
	, SOCKET_BASE_NAME)

#define gethostbyaddr(addr, len, type) \
	LP3(0xd8, struct hostent  *, gethostbyaddr, const UBYTE *, addr, a0, LONG, len, d0, LONG, type, d1, \
	, SOCKET_BASE_NAME)

#define getnetbyname(name) \
	LP1(0xde, struct netent   *, getnetbyname, const UBYTE *, name, a0, \
	, SOCKET_BASE_NAME)

#define getnetbyaddr(net, type) \
	LP2(0xe4, struct netent   *, getnetbyaddr, LONG, net, d0, LONG, type, d1, \
	, SOCKET_BASE_NAME)

#define getservbyname(name, proto) \
	LP2(0xea, struct servent  *, getservbyname, const UBYTE *, name, a0, const UBYTE *, proto, a1, \
	, SOCKET_BASE_NAME)

#define getservbyport(port, proto) \
	LP2(0xf0, struct servent  *, getservbyport, LONG, port, d0, const UBYTE *, proto, a0, \
	, SOCKET_BASE_NAME)

#define getprotobyname(name) \
	LP1(0xf6, struct protoent *, getprotobyname, const UBYTE *, name, a0, \
	, SOCKET_BASE_NAME)

#define getprotobynumber(proto) \
	LP1(0xfc, struct protoent *, getprotobynumber, LONG, proto, d0, \
	, SOCKET_BASE_NAME)

#define vsyslog(level, format, ap) \
	LP3NR(0x102, vsyslog, ULONG, level, d0, const char *, format, a0, LONG *, ap, a1, \
	, SOCKET_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define syslog(level, format, tags...) \
	({ULONG _tags[] = {tags}; vsyslog((level), (format), (LONG *) _tags);})
#endif

#define Dup2Socket(fd1, fd2) \
	LP2(0x108, LONG, Dup2Socket, LONG, fd1, d0, LONG, fd2, d1, \
	, SOCKET_BASE_NAME)

#define sendmsg(s, msg, flags) \
	LP3(0x10e, LONG, sendmsg, LONG, s, d0, struct msghdr *, msg, a0, LONG, flags, d1, \
	, SOCKET_BASE_NAME)

#define recvmsg(s, msg, flags) \
	LP3(0x114, LONG, recvmsg, LONG, s, d0, struct msghdr *, msg, a0, LONG, flags, d1, \
	, SOCKET_BASE_NAME)

#define gethostname(hostname, size) \
	LP2(0x11a, LONG, gethostname, STRPTR, hostname, a0, LONG, size, d0, \
	, SOCKET_BASE_NAME)

#define gethostid() \
	LP0(0x120, ULONG, gethostid, \
	, SOCKET_BASE_NAME)

#define SocketBaseTagList(taglist) \
	LP1(0x126, LONG, SocketBaseTagList, struct TagItem *, taglist, a0, \
	, SOCKET_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SocketBaseTags(tags...) \
	({ULONG _tags[] = {tags}; SocketBaseTagList((struct TagItem *) _tags);})
#endif

#define GetSocketEvents(eventmaskp) \
	LP1(0x12c, LONG, GetSocketEvents, ULONG *, eventmaskp, a0, \
	, SOCKET_BASE_NAME)

#endif /*  _INLINE_SOCKET_H  */
