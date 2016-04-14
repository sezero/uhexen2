#ifndef CLIB_SOCKET_PROTOS_H
#define CLIB_SOCKET_PROTOS_H \
       "$Id: socket_protos.h,v 4.6 1996/03/26 23:07:46 too Exp $"
/*
 *      Prototypes of AmiTCP/IP bsdsocket.library
 * 
 *      Copyright © 1994 AmiTCP/IP Group,
 *                       Network Solutions Development, Inc.
 *                       All rights reserved.
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef NETINET_IN_H
#include <netinet/in.h>
#endif


LONG socket(LONG domain, LONG type, LONG protocol);
LONG bind(LONG s, const struct sockaddr *name, LONG namelen);
LONG listen(LONG s, LONG backlog);
LONG accept(LONG s, struct sockaddr *addr, LONG *addrlen);
LONG connect(LONG s, const struct sockaddr *name, LONG namelen);
LONG send(LONG s, const UBYTE *msg, LONG len, LONG flags);
LONG sendto(LONG s, const UBYTE *msg, LONG len, LONG flags, 
		  const struct sockaddr *to, LONG tolen);
LONG sendmsg(LONG s, struct msghdr * msg, LONG flags);
LONG recv(LONG s, UBYTE *buf, LONG len, LONG flags);	/* V3 */
LONG recvfrom(LONG s, UBYTE *buf, LONG len, LONG flags, 
		    struct sockaddr *from, LONG *fromlen);
LONG recvmsg(LONG s, struct msghdr * msg, LONG flags);	/* V3 */
LONG shutdown(LONG s, LONG how);
LONG setsockopt(LONG s, LONG level, LONG optname, 
		     const void *optval, LONG optlen);
LONG getsockopt(LONG s, LONG level, LONG optname, 
		     void *optval, LONG *optlen);
LONG getsockname(LONG s, struct sockaddr *name, LONG *namelen);
LONG getpeername(LONG s, struct sockaddr *name, LONG *namelen);

LONG IoctlSocket(LONG d, ULONG request, char *argp);
LONG CloseSocket(LONG d);
LONG WaitSelect(LONG nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds,
		struct timeval *timeout, ULONG *maskp);

LONG Dup2Socket(LONG fd1, LONG fd2);		       /* V2 */

LONG getdtablesize(void);			       /* V3 */
void SetSocketSignals(ULONG _SIGINTR, ULONG _SIGIO, ULONG _SIGURG);/* uhexen2: uglified param names (otherwise SIGIO is a macro expanding to 23 !!!) */
LONG SetErrnoPtr(void *errno_p, LONG size);
LONG SocketBaseTagList(struct TagItem *tagList);	/* V3 */
LONG SocketBaseTags(LONG tag, ...);			/* V3 */

LONG GetSocketEvents(ULONG *eventmaskp);		/* V4 */

LONG Errno(void);

LONG gethostname(STRPTR hostname, LONG size);          /* V3 */
ULONG gethostid(void);				       /* V3 */

LONG ObtainSocket(LONG id, LONG domain, LONG type, LONG protocol);
LONG ReleaseSocket(LONG fd, LONG id);
LONG ReleaseCopyOfSocket(LONG fd, LONG id);

/* Arpa/inet functions */
ULONG inet_addr(const UBYTE *);
ULONG inet_network(const UBYTE *);
char *Inet_NtoA(ULONG s_addr);
ULONG Inet_MakeAddr(ULONG net, ULONG lna);
ULONG Inet_LnaOf(LONG s_addr);
ULONG Inet_NetOf(LONG s_addr);

/* NetDB functions */
struct hostent  *gethostbyname(const UBYTE *name);
struct hostent  *gethostbyaddr(const UBYTE *addr, LONG len, LONG type);
struct netent   *getnetbyname(const UBYTE *name);
struct netent   *getnetbyaddr(LONG net, LONG type);
struct servent  *getservbyname(const UBYTE *name, const UBYTE *proto);
struct servent  *getservbyport(LONG port, const UBYTE *proto);
struct protoent *getprotobyname(const UBYTE *name);
struct protoent *getprotobynumber(LONG proto);

/* Syslog functions */
void vsyslog(ULONG pri, const char *fmt, LONG *);
void syslog(ULONG pri, const char *fmt, ...);

#endif /* !CLIB_SOCKET_PROTOS_H */
