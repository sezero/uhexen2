/*
 * $Id$
 *
 * :ts=8
 *
 * 'Roadshow' -- Amiga TCP/IP stack
 * Copyright © 2001-2016 by Olaf Barthel.
 * All Rights Reserved.
 *
 * Amiga specific TCP/IP 'C' header files;
 * Freely Distributable
 */

#ifndef CLIB_BSDSOCKET_PROTOS_H
#define CLIB_BSDSOCKET_PROTOS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

#ifdef __NEW_TIMEVAL_DEFINITION_USED__
#define __timeval TimeVal
#else
#define __timeval timeval
#endif

LONG socket( LONG domain, LONG type, LONG protocol );
LONG bind( LONG sock, struct sockaddr *name, socklen_t namelen );
LONG listen( LONG sock, LONG backlog );
LONG accept( LONG sock, struct sockaddr *addr, socklen_t *addrlen );
LONG connect( LONG sock, struct sockaddr *name, socklen_t namelen );
LONG sendto( LONG sock, APTR buf, LONG len, LONG flags, struct sockaddr *to, socklen_t tolen );
LONG send( LONG sock, APTR buf, LONG len, LONG flags );
LONG recvfrom( LONG sock, APTR buf, LONG len, LONG flags, struct sockaddr *addr, socklen_t *addrlen );
LONG recv( LONG sock, APTR buf, LONG len, LONG flags );
LONG shutdown( LONG sock, LONG how );
LONG setsockopt( LONG sock, LONG level, LONG optname, APTR optval, socklen_t optlen );
LONG getsockopt( LONG sock, LONG level, LONG optname, APTR optval, socklen_t *optlen );
LONG getsockname( LONG sock, struct sockaddr *name, socklen_t *namelen );
LONG getpeername( LONG sock, struct sockaddr *name, socklen_t *namelen );
LONG IoctlSocket( LONG sock, ULONG req, APTR argp );
LONG CloseSocket( LONG sock );
LONG WaitSelect( LONG nfds, APTR read_fds, APTR write_fds, APTR except_fds, struct __timeval *_timeout, ULONG *signals );
VOID SetSocketSignals( ULONG int_mask, ULONG io_mask, ULONG urgent_mask );
LONG getdtablesize( VOID );
LONG ObtainSocket( LONG id, LONG domain, LONG type, LONG protocol );
LONG ReleaseSocket( LONG sock, LONG id );
LONG ReleaseCopyOfSocket( LONG sock, LONG id );
LONG Errno( VOID );
VOID SetErrnoPtr( APTR errno_ptr, LONG size );
STRPTR Inet_NtoA( in_addr_t ip );
in_addr_t inet_addr( STRPTR cp );
in_addr_t Inet_LnaOf( in_addr_t in );
in_addr_t Inet_NetOf( in_addr_t in );
in_addr_t Inet_MakeAddr( in_addr_t net, in_addr_t host );
in_addr_t inet_network( STRPTR cp );
struct hostent *gethostbyname( STRPTR name );
struct hostent *gethostbyaddr( STRPTR addr, LONG len, LONG type );
struct netent *getnetbyname( STRPTR name );
struct netent *getnetbyaddr( in_addr_t net, LONG type );
struct servent *getservbyname( STRPTR name, STRPTR proto );
struct servent *getservbyport( LONG port, STRPTR proto );
struct protoent *getprotobyname( STRPTR name );
struct protoent *getprotobynumber( LONG proto );
VOID vsyslog( LONG pri, STRPTR msg, APTR args );
VOID syslog( LONG pri, STRPTR msg, LONG first_parameter, ... );
LONG Dup2Socket( LONG old_socket, LONG new_socket );
LONG sendmsg( LONG sock, struct msghdr *msg, LONG flags );
LONG recvmsg( LONG sock, struct msghdr *msg, LONG flags );
LONG gethostname( STRPTR name, LONG namelen );
in_addr_t gethostid( VOID );
LONG SocketBaseTagList( struct TagItem *tags );
LONG SocketBaseTags( Tag first_tag, ... );
LONG GetSocketEvents( ULONG *event_ptr );
/* Ten reserved slots for future expansion */
/* Berkeley Packet Filter (Roadshow extensions start here) */
LONG bpf_open( LONG channel );
LONG bpf_close( LONG channel );
LONG bpf_read( LONG channel, APTR buffer, LONG len );
LONG bpf_write( LONG channel, APTR buffer, LONG len );
LONG bpf_set_notify_mask( LONG channel, ULONG signal_mask );
LONG bpf_set_interrupt_mask( LONG channel, ULONG signal_mask );
LONG bpf_ioctl( LONG channel, ULONG command, APTR buffer );
LONG bpf_data_waiting( LONG channel );
/* Route management */
LONG AddRouteTagList( struct TagItem *tags );
LONG AddRouteTags( Tag first_tag, ... );
LONG DeleteRouteTagList( struct TagItem *tags );
LONG DeleteRouteTags( Tag first_tag, ... );
VOID FreeRouteInfo( struct rt_msghdr *buf );
struct rt_msghdr *GetRouteInfo( LONG address_family, LONG flags );
/* Interface management */
LONG AddInterfaceTagList( STRPTR interface_name, STRPTR device_name, LONG unit, struct TagItem *tags );
LONG AddInterfaceTags( STRPTR interface_name, STRPTR device_name, LONG unit, Tag first_tag, ... );
LONG ConfigureInterfaceTagList( STRPTR interface_name, struct TagItem *tags );
LONG ConfigureInterfaceTags( STRPTR interface_name, Tag first_tag, ... );
VOID ReleaseInterfaceList( struct List *list );
struct List *ObtainInterfaceList( VOID );
LONG QueryInterfaceTagList( STRPTR interface_name, struct TagItem *tags );
LONG QueryInterfaceTags( STRPTR interface_name, Tag first_tag, ... );
LONG CreateAddrAllocMessageA( LONG version, LONG protocol, STRPTR interface_name, struct AddressAllocationMessage **result_ptr, struct TagItem *tags );
LONG CreateAddrAllocMessage( LONG version, LONG protocol, STRPTR interface_name, struct AddressAllocationMessage **result_ptr, Tag first_tag, ... );
VOID DeleteAddrAllocMessage( struct AddressAllocationMessage *aam );
VOID BeginInterfaceConfig( struct AddressAllocationMessage *message );
VOID AbortInterfaceConfig( struct AddressAllocationMessage *message );
/* Monitor management */
LONG AddNetMonitorHookTagList( LONG type, struct Hook *hook, struct TagItem *tags );
LONG AddNetMonitorHookTags( LONG type, struct Hook *hook, Tag first_tag, ... );
VOID RemoveNetMonitorHook( struct Hook *hook );
/* Status query */
LONG GetNetworkStatistics( LONG type, LONG version, APTR destination, LONG size );
/* Domain name server management */
LONG AddDomainNameServer( STRPTR address );
LONG RemoveDomainNameServer( STRPTR address );
VOID ReleaseDomainNameServerList( struct List *list );
struct List *ObtainDomainNameServerList( VOID );
/* Local database access */
VOID setnetent( LONG stay_open );
VOID endnetent( VOID );
struct netent *getnetent( VOID );
VOID setprotoent( LONG stay_open );
VOID endprotoent( VOID );
struct protoent *getprotoent( VOID );
VOID setservent( LONG stay_open );
VOID endservent( VOID );
struct servent *getservent( VOID );
/* Address conversion */
LONG inet_aton( STRPTR cp, struct in_addr *addr );
STRPTR inet_ntop( LONG af, APTR src, STRPTR dst, LONG size );
LONG inet_pton( LONG af, STRPTR src, APTR dst );
LONG In_LocalAddr( in_addr_t address );
LONG In_CanForward( in_addr_t address );
/* Kernel memory management */
struct mbuf *mbuf_copym( struct mbuf *m, LONG off, LONG len );
LONG mbuf_copyback( struct mbuf *m, LONG off, LONG len, APTR cp );
LONG mbuf_copydata( struct mbuf *m, LONG off, LONG len, APTR cp );
struct mbuf *mbuf_free( struct mbuf *m );
VOID mbuf_freem( struct mbuf *m );
struct mbuf *mbuf_get( VOID );
struct mbuf *mbuf_gethdr( VOID );
struct mbuf *mbuf_prepend( struct mbuf *m, LONG len );
LONG mbuf_cat( struct mbuf *m, struct mbuf *n );
LONG mbuf_adj( struct mbuf *mp, LONG req_len );
struct mbuf *mbuf_pullup( struct mbuf *m, LONG len );
/* Internet servers */
BOOL ProcessIsServer( struct Process *pr );
LONG ObtainServerSocket( VOID );
/* Default domain name */
BOOL GetDefaultDomainName( STRPTR buffer, LONG buffer_size );
VOID SetDefaultDomainName( STRPTR buffer );
/* Global data access */
struct List *ObtainRoadshowData( LONG access );
VOID ReleaseRoadshowData( struct List *list );
BOOL ChangeRoadshowData( struct List *list, STRPTR name, ULONG length, APTR data );
/* The counterpart to AddInterfaceTagList */
LONG RemoveInterface( STRPTR interface_name, LONG force );
/* Reentrant versions of the gethostbyname/gethostbyaddr functions, BSD-style */
struct hostent *gethostbyname_r( STRPTR name, struct hostent *hp, APTR buf, ULONG buflen, LONG *he );
struct hostent *gethostbyaddr_r( STRPTR addr, LONG len, LONG type, struct hostent *hp, APTR buf, ULONG buflen, LONG *he );
/* Two reserved slots for future expansion */
/* Node and service name translation (rfc3493) */
VOID freeaddrinfo( struct addrinfo *ai );
LONG getaddrinfo( STRPTR hostname, STRPTR servname, struct addrinfo *hints, struct addrinfo **res );
STRPTR gai_strerror( LONG errnum );
LONG getnameinfo( struct sockaddr *sa, ULONG salen, STRPTR host, ULONG hostlen, STRPTR serv, ULONG servlen, ULONG flags );
/* Six reserved slots for future expansion */

#undef __timeval

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_BSDSOCKET_PROTOS_H */
