#ifndef _VBCCINLINE_BSDSOCKET_H
#define _VBCCINLINE_BSDSOCKET_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

LONG __socket(__reg("a6") struct Library *, __reg("d0") LONG domain, __reg("d1") LONG type, __reg("d2") LONG protocol)="\tjsr\t-30(a6)";
#define socket(domain, type, protocol) __socket(SocketBase, (domain), (type), (protocol))

LONG __bind(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct sockaddr * name, __reg("d1") LONG namelen)="\tjsr\t-36(a6)";
#define bind(sock, name, namelen) __bind(SocketBase, (sock), (name), (namelen))

LONG __listen(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") LONG backlog)="\tjsr\t-42(a6)";
#define listen(sock, backlog) __listen(SocketBase, (sock), (backlog))

LONG __accept(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct sockaddr * addr, __reg("a1") socklen_t * addrlen)="\tjsr\t-48(a6)";
#define accept(sock, addr, addrlen) __accept(SocketBase, (sock), (addr), (addrlen))

LONG __connect(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct sockaddr * name, __reg("d1") LONG namelen)="\tjsr\t-54(a6)";
#define connect(sock, name, namelen) __connect(SocketBase, (sock), (name), (namelen))

LONG __sendto(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len, __reg("d2") LONG flags, __reg("a1") struct sockaddr * to, __reg("d3") LONG tolen)="\tjsr\t-60(a6)";
#define sendto(sock, buf, len, flags, to, tolen) __sendto(SocketBase, (sock), (buf), (len), (flags), (to), (tolen))

LONG __send(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len, __reg("d2") LONG flags)="\tjsr\t-66(a6)";
#define send(sock, buf, len, flags) __send(SocketBase, (sock), (buf), (len), (flags))

LONG __recvfrom(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len, __reg("d2") LONG flags, __reg("a1") struct sockaddr * addr, __reg("a2") socklen_t * addrlen)="\tjsr\t-72(a6)";
#define recvfrom(sock, buf, len, flags, addr, addrlen) __recvfrom(SocketBase, (sock), (buf), (len), (flags), (addr), (addrlen))

LONG __recv(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") APTR buf, __reg("d1") LONG len, __reg("d2") LONG flags)="\tjsr\t-78(a6)";
#define recv(sock, buf, len, flags) __recv(SocketBase, (sock), (buf), (len), (flags))

LONG __shutdown(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") LONG how)="\tjsr\t-84(a6)";
#define shutdown(sock, how) __shutdown(SocketBase, (sock), (how))

LONG __setsockopt(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") LONG level, __reg("d2") LONG optname, __reg("a0") APTR optval, __reg("d3") LONG optlen)="\tjsr\t-90(a6)";
#define setsockopt(sock, level, optname, optval, optlen) __setsockopt(SocketBase, (sock), (level), (optname), (optval), (optlen))

LONG __getsockopt(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") LONG level, __reg("d2") LONG optname, __reg("a0") APTR optval, __reg("a1") socklen_t * optlen)="\tjsr\t-96(a6)";
#define getsockopt(sock, level, optname, optval, optlen) __getsockopt(SocketBase, (sock), (level), (optname), (optval), (optlen))

LONG __getsockname(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct sockaddr * name, __reg("a1") socklen_t * namelen)="\tjsr\t-102(a6)";
#define getsockname(sock, name, namelen) __getsockname(SocketBase, (sock), (name), (namelen))

LONG __getpeername(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct sockaddr * name, __reg("a1") socklen_t * namelen)="\tjsr\t-108(a6)";
#define getpeername(sock, name, namelen) __getpeername(SocketBase, (sock), (name), (namelen))

LONG __IoctlSocket(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") ULONG req, __reg("a0") APTR argp)="\tjsr\t-114(a6)";
#define IoctlSocket(sock, req, argp) __IoctlSocket(SocketBase, (sock), (req), (argp))

LONG __CloseSocket(__reg("a6") struct Library *, __reg("d0") LONG sock)="\tjsr\t-120(a6)";
#define CloseSocket(sock) __CloseSocket(SocketBase, (sock))

LONG __WaitSelect(__reg("a6") struct Library *, __reg("d0") LONG nfds, __reg("a0") APTR read_fds, __reg("a1") APTR write_fds, __reg("a2") APTR except_fds, __reg("a3") struct timeval * _timeout, __reg("d1") ULONG * signals)="\tjsr\t-126(a6)";
#define WaitSelect(nfds, read_fds, write_fds, except_fds, _timeout, signals) __WaitSelect(SocketBase, (nfds), (read_fds), (write_fds), (except_fds), (_timeout), (signals))

VOID __SetSocketSignals(__reg("a6") struct Library *, __reg("d0") ULONG int_mask, __reg("d1") ULONG io_mask, __reg("d2") ULONG urgent_mask)="\tjsr\t-132(a6)";
#define SetSocketSignals(int_mask, io_mask, urgent_mask) __SetSocketSignals(SocketBase, (int_mask), (io_mask), (urgent_mask))

LONG __getdtablesize(__reg("a6") struct Library *)="\tjsr\t-138(a6)";
#define getdtablesize() __getdtablesize(SocketBase)

LONG __ObtainSocket(__reg("a6") struct Library *, __reg("d0") LONG id, __reg("d1") LONG domain, __reg("d2") LONG type, __reg("d3") LONG protocol)="\tjsr\t-144(a6)";
#define ObtainSocket(id, domain, type, protocol) __ObtainSocket(SocketBase, (id), (domain), (type), (protocol))

LONG __ReleaseSocket(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") LONG id)="\tjsr\t-150(a6)";
#define ReleaseSocket(sock, id) __ReleaseSocket(SocketBase, (sock), (id))

LONG __ReleaseCopyOfSocket(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("d1") LONG id)="\tjsr\t-156(a6)";
#define ReleaseCopyOfSocket(sock, id) __ReleaseCopyOfSocket(SocketBase, (sock), (id))

LONG __Errno(__reg("a6") struct Library *)="\tjsr\t-162(a6)";
#define Errno() __Errno(SocketBase)

VOID __SetErrnoPtr(__reg("a6") struct Library *, __reg("a0") APTR errno_ptr, __reg("d0") LONG size)="\tjsr\t-168(a6)";
#define SetErrnoPtr(errno_ptr, size) __SetErrnoPtr(SocketBase, (errno_ptr), (size))

STRPTR __Inet_NtoA(__reg("a6") struct Library *, __reg("d0") LONG ip)="\tjsr\t-174(a6)";
#define Inet_NtoA(ip) __Inet_NtoA(SocketBase, (ip))

in_addr_t __inet_addr(__reg("a6") struct Library *, __reg("a0") STRPTR cp)="\tjsr\t-180(a6)";
#define inet_addr(cp) __inet_addr(SocketBase, (cp))

in_addr_t __Inet_LnaOf(__reg("a6") struct Library *, __reg("d0") LONG in)="\tjsr\t-186(a6)";
#define Inet_LnaOf(in) __Inet_LnaOf(SocketBase, (in))

in_addr_t __Inet_NetOf(__reg("a6") struct Library *, __reg("d0") LONG in)="\tjsr\t-192(a6)";
#define Inet_NetOf(in) __Inet_NetOf(SocketBase, (in))

in_addr_t __Inet_MakeAddr(__reg("a6") struct Library *, __reg("d0") LONG net, __reg("d1") LONG host)="\tjsr\t-198(a6)";
#define Inet_MakeAddr(net, host) __Inet_MakeAddr(SocketBase, (net), (host))

in_addr_t __inet_network(__reg("a6") struct Library *, __reg("a0") STRPTR cp)="\tjsr\t-204(a6)";
#define inet_network(cp) __inet_network(SocketBase, (cp))

struct hostent * __gethostbyname(__reg("a6") struct Library *, __reg("a0") STRPTR name)="\tjsr\t-210(a6)";
#define gethostbyname(name) __gethostbyname(SocketBase, (name))

struct hostent * __gethostbyaddr(__reg("a6") struct Library *, __reg("a0") STRPTR addr, __reg("d0") LONG len, __reg("d1") LONG type)="\tjsr\t-216(a6)";
#define gethostbyaddr(addr, len, type) __gethostbyaddr(SocketBase, (addr), (len), (type))

struct netent * __getnetbyname(__reg("a6") struct Library *, __reg("a0") STRPTR name)="\tjsr\t-222(a6)";
#define getnetbyname(name) __getnetbyname(SocketBase, (name))

struct netent * __getnetbyaddr(__reg("a6") struct Library *, __reg("d0") LONG net, __reg("d1") LONG type)="\tjsr\t-228(a6)";
#define getnetbyaddr(net, type) __getnetbyaddr(SocketBase, (net), (type))

struct servent * __getservbyname(__reg("a6") struct Library *, __reg("a0") STRPTR name, __reg("a1") STRPTR proto)="\tjsr\t-234(a6)";
#define getservbyname(name, proto) __getservbyname(SocketBase, (name), (proto))

struct servent * __getservbyport(__reg("a6") struct Library *, __reg("d0") LONG port, __reg("a0") STRPTR proto)="\tjsr\t-240(a6)";
#define getservbyport(port, proto) __getservbyport(SocketBase, (port), (proto))

struct protoent * __getprotobyname(__reg("a6") struct Library *, __reg("a0") STRPTR name)="\tjsr\t-246(a6)";
#define getprotobyname(name) __getprotobyname(SocketBase, (name))

struct protoent * __getprotobynumber(__reg("a6") struct Library *, __reg("d0") LONG proto)="\tjsr\t-252(a6)";
#define getprotobynumber(proto) __getprotobynumber(SocketBase, (proto))

VOID __vsyslog(__reg("a6") struct Library *, __reg("d0") LONG pri, __reg("a0") STRPTR msg, __reg("a1") APTR args)="\tjsr\t-258(a6)";
#define vsyslog(pri, msg, args) __vsyslog(SocketBase, (pri), (msg), (args))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
VOID __syslog(__reg("a6") struct Library *, __reg("d0") LONG pri, __reg("a0") STRPTR msg, LONG args, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-258(a6)\n\tmovea.l\t(a7)+,a1";
#define syslog(pri, msg, ...) __syslog(SocketBase, (pri), (msg), __VA_ARGS__)
#endif

LONG __Dup2Socket(__reg("a6") struct Library *, __reg("d0") LONG old_socket, __reg("d1") LONG new_socket)="\tjsr\t-264(a6)";
#define Dup2Socket(old_socket, new_socket) __Dup2Socket(SocketBase, (old_socket), (new_socket))

LONG __sendmsg(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct msghdr * msg, __reg("d1") LONG flags)="\tjsr\t-270(a6)";
#define sendmsg(sock, msg, flags) __sendmsg(SocketBase, (sock), (msg), (flags))

LONG __recvmsg(__reg("a6") struct Library *, __reg("d0") LONG sock, __reg("a0") struct msghdr * msg, __reg("d1") LONG flags)="\tjsr\t-276(a6)";
#define recvmsg(sock, msg, flags) __recvmsg(SocketBase, (sock), (msg), (flags))

LONG __gethostname(__reg("a6") struct Library *, __reg("a0") STRPTR name, __reg("d0") LONG namelen)="\tjsr\t-282(a6)";
#define gethostname(name, namelen) __gethostname(SocketBase, (name), (namelen))

in_addr_t __gethostid(__reg("a6") struct Library *)="\tjsr\t-288(a6)";
#define gethostid() __gethostid(SocketBase)

LONG __SocketBaseTagList(__reg("a6") struct Library *, __reg("a0") struct TagItem * tags)="\tjsr\t-294(a6)";
#define SocketBaseTagList(tags) __SocketBaseTagList(SocketBase, (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __SocketBaseTags(__reg("a6") struct Library *, Tag tags, ...)="\tmove.l\ta0,-(a7)\n\tlea\t4(a7),a0\n\tjsr\t-294(a6)\n\tmovea.l\t(a7)+,a0";
#define SocketBaseTags(...) __SocketBaseTags(SocketBase, __VA_ARGS__)
#endif

LONG __GetSocketEvents(__reg("a6") struct Library *, __reg("a0") ULONG * event_ptr)="\tjsr\t-300(a6)";
#define GetSocketEvents(event_ptr) __GetSocketEvents(SocketBase, (event_ptr))

LONG __bpf_open(__reg("a6") struct Library *, __reg("d0") LONG channel)="\tjsr\t-366(a6)";
#define bpf_open(channel) __bpf_open(SocketBase, (channel))

LONG __bpf_close(__reg("a6") struct Library *, __reg("d0") LONG channel)="\tjsr\t-372(a6)";
#define bpf_close(channel) __bpf_close(SocketBase, (channel))

LONG __bpf_read(__reg("a6") struct Library *, __reg("d0") LONG channel, __reg("a0") APTR buffer, __reg("d1") LONG len)="\tjsr\t-378(a6)";
#define bpf_read(channel, buffer, len) __bpf_read(SocketBase, (channel), (buffer), (len))

LONG __bpf_write(__reg("a6") struct Library *, __reg("d0") LONG channel, __reg("a0") APTR buffer, __reg("d1") LONG len)="\tjsr\t-384(a6)";
#define bpf_write(channel, buffer, len) __bpf_write(SocketBase, (channel), (buffer), (len))

LONG __bpf_set_notify_mask(__reg("a6") struct Library *, __reg("d1") LONG channel, __reg("d0") ULONG signal_mask)="\tjsr\t-390(a6)";
#define bpf_set_notify_mask(channel, signal_mask) __bpf_set_notify_mask(SocketBase, (channel), (signal_mask))

LONG __bpf_set_interrupt_mask(__reg("a6") struct Library *, __reg("d0") LONG channel, __reg("d1") ULONG signal_mask)="\tjsr\t-396(a6)";
#define bpf_set_interrupt_mask(channel, signal_mask) __bpf_set_interrupt_mask(SocketBase, (channel), (signal_mask))

LONG __bpf_ioctl(__reg("a6") struct Library *, __reg("d0") LONG channel, __reg("d1") ULONG command, __reg("a0") APTR buffer)="\tjsr\t-402(a6)";
#define bpf_ioctl(channel, command, buffer) __bpf_ioctl(SocketBase, (channel), (command), (buffer))

LONG __bpf_data_waiting(__reg("a6") struct Library *, __reg("d0") LONG channel)="\tjsr\t-408(a6)";
#define bpf_data_waiting(channel) __bpf_data_waiting(SocketBase, (channel))

LONG __AddRouteTagList(__reg("a6") struct Library *, __reg("a0") struct TagItem * tags)="\tjsr\t-414(a6)";
#define AddRouteTagList(tags) __AddRouteTagList(SocketBase, (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __AddRouteTags(__reg("a6") struct Library *, Tag tags, ...)="\tmove.l\ta0,-(a7)\n\tlea\t4(a7),a0\n\tjsr\t-414(a6)\n\tmovea.l\t(a7)+,a0";
#define AddRouteTags(...) __AddRouteTags(SocketBase, __VA_ARGS__)
#endif

LONG __DeleteRouteTagList(__reg("a6") struct Library *, __reg("a0") struct TagItem * tags)="\tjsr\t-420(a6)";
#define DeleteRouteTagList(tags) __DeleteRouteTagList(SocketBase, (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __DeleteRouteTags(__reg("a6") struct Library *, Tag tags, ...)="\tmove.l\ta0,-(a7)\n\tlea\t4(a7),a0\n\tjsr\t-420(a6)\n\tmovea.l\t(a7)+,a0";
#define DeleteRouteTags(...) __DeleteRouteTags(SocketBase, __VA_ARGS__)
#endif

VOID __FreeRouteInfo(__reg("a6") struct Library *, __reg("a0") struct rt_msghdr * buf)="\tjsr\t-432(a6)";
#define FreeRouteInfo(buf) __FreeRouteInfo(SocketBase, (buf))

struct rt_msghdr * __GetRouteInfo(__reg("a6") struct Library *, __reg("d0") LONG address_family, __reg("d1") LONG flags)="\tjsr\t-438(a6)";
#define GetRouteInfo(address_family, flags) __GetRouteInfo(SocketBase, (address_family), (flags))

LONG __AddInterfaceTagList(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, __reg("a1") STRPTR device_name, __reg("d0") LONG unit, __reg("a2") struct TagItem * tags)="\tjsr\t-444(a6)";
#define AddInterfaceTagList(interface_name, device_name, unit, tags) __AddInterfaceTagList(SocketBase, (interface_name), (device_name), (unit), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __AddInterfaceTags(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, __reg("a1") STRPTR device_name, __reg("d0") LONG unit, Tag tags, ...)="\tmove.l\ta2,-(a7)\n\tlea\t4(a7),a2\n\tjsr\t-444(a6)\n\tmovea.l\t(a7)+,a2";
#define AddInterfaceTags(interface_name, device_name, unit, ...) __AddInterfaceTags(SocketBase, (interface_name), (device_name), (unit), __VA_ARGS__)
#endif

LONG __ConfigureInterfaceTagList(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, __reg("a1") struct TagItem * tags)="\tjsr\t-450(a6)";
#define ConfigureInterfaceTagList(interface_name, tags) __ConfigureInterfaceTagList(SocketBase, (interface_name), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __ConfigureInterfaceTags(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-450(a6)\n\tmovea.l\t(a7)+,a1";
#define ConfigureInterfaceTags(interface_name, ...) __ConfigureInterfaceTags(SocketBase, (interface_name), __VA_ARGS__)
#endif

VOID __ReleaseInterfaceList(__reg("a6") struct Library *, __reg("a0") struct List * list)="\tjsr\t-456(a6)";
#define ReleaseInterfaceList(list) __ReleaseInterfaceList(SocketBase, (list))

struct List * __ObtainInterfaceList(__reg("a6") struct Library *)="\tjsr\t-462(a6)";
#define ObtainInterfaceList() __ObtainInterfaceList(SocketBase)

LONG __QueryInterfaceTagList(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, __reg("a1") struct TagItem * tags)="\tjsr\t-468(a6)";
#define QueryInterfaceTagList(interface_name, tags) __QueryInterfaceTagList(SocketBase, (interface_name), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __QueryInterfaceTags(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-468(a6)\n\tmovea.l\t(a7)+,a1";
#define QueryInterfaceTags(interface_name, ...) __QueryInterfaceTags(SocketBase, (interface_name), __VA_ARGS__)
#endif

LONG __CreateAddrAllocMessageA(__reg("a6") struct Library *, __reg("d0") LONG version, __reg("d1") LONG protocol, __reg("a0") STRPTR interface_name, __reg("a1") struct AddressAllocationMessage ** result_ptr, __reg("a2") struct TagItem * tags)="\tjsr\t-474(a6)";
#define CreateAddrAllocMessageA(version, protocol, interface_name, result_ptr, tags) __CreateAddrAllocMessageA(SocketBase, (version), (protocol), (interface_name), (result_ptr), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __CreateAddrAllocMessage(__reg("a6") struct Library *, __reg("d0") LONG version, __reg("d1") LONG protocol, __reg("a0") STRPTR interface_name, __reg("a1") struct AddressAllocationMessage ** result_ptr, Tag tags, ...)="\tmove.l\ta2,-(a7)\n\tlea\t4(a7),a2\n\tjsr\t-474(a6)\n\tmovea.l\t(a7)+,a2";
#define CreateAddrAllocMessage(version, protocol, interface_name, result_ptr, ...) __CreateAddrAllocMessage(SocketBase, (version), (protocol), (interface_name), (result_ptr), __VA_ARGS__)
#endif

VOID __DeleteAddrAllocMessage(__reg("a6") struct Library *, __reg("a0") struct AddressAllocationMessage * aam)="\tjsr\t-480(a6)";
#define DeleteAddrAllocMessage(aam) __DeleteAddrAllocMessage(SocketBase, (aam))

VOID __BeginInterfaceConfig(__reg("a6") struct Library *, __reg("a0") struct AddressAllocationMessage * message)="\tjsr\t-486(a6)";
#define BeginInterfaceConfig(message) __BeginInterfaceConfig(SocketBase, (message))

VOID __AbortInterfaceConfig(__reg("a6") struct Library *, __reg("a0") struct AddressAllocationMessage * message)="\tjsr\t-492(a6)";
#define AbortInterfaceConfig(message) __AbortInterfaceConfig(SocketBase, (message))

LONG __AddNetMonitorHookTagList(__reg("a6") struct Library *, __reg("d0") LONG type, __reg("a0") struct Hook * hook, __reg("a1") struct TagItem * tags)="\tjsr\t-498(a6)";
#define AddNetMonitorHookTagList(type, hook, tags) __AddNetMonitorHookTagList(SocketBase, (type), (hook), (tags))

#if !defined(NO_INLINE_STDARG) && (__STDC__ == 1L) && (__STDC_VERSION__ >= 199901L)
LONG __AddNetMonitorHookTags(__reg("a6") struct Library *, __reg("d0") LONG type, __reg("a0") struct Hook * hook, Tag tags, ...)="\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-498(a6)\n\tmovea.l\t(a7)+,a1";
#define AddNetMonitorHookTags(type, hook, ...) __AddNetMonitorHookTags(SocketBase, (type), (hook), __VA_ARGS__)
#endif

VOID __RemoveNetMonitorHook(__reg("a6") struct Library *, __reg("a0") struct Hook * hook)="\tjsr\t-504(a6)";
#define RemoveNetMonitorHook(hook) __RemoveNetMonitorHook(SocketBase, (hook))

LONG __GetNetworkStatistics(__reg("a6") struct Library *, __reg("d0") LONG type, __reg("d1") LONG version, __reg("a0") APTR destination, __reg("d2") LONG size)="\tjsr\t-510(a6)";
#define GetNetworkStatistics(type, version, destination, size) __GetNetworkStatistics(SocketBase, (type), (version), (destination), (size))

LONG __AddDomainNameServer(__reg("a6") struct Library *, __reg("a0") STRPTR address)="\tjsr\t-516(a6)";
#define AddDomainNameServer(address) __AddDomainNameServer(SocketBase, (address))

LONG __RemoveDomainNameServer(__reg("a6") struct Library *, __reg("a0") STRPTR address)="\tjsr\t-522(a6)";
#define RemoveDomainNameServer(address) __RemoveDomainNameServer(SocketBase, (address))

VOID __ReleaseDomainNameServerList(__reg("a6") struct Library *, __reg("a0") struct List * list)="\tjsr\t-528(a6)";
#define ReleaseDomainNameServerList(list) __ReleaseDomainNameServerList(SocketBase, (list))

struct List * __ObtainDomainNameServerList(__reg("a6") struct Library *)="\tjsr\t-534(a6)";
#define ObtainDomainNameServerList() __ObtainDomainNameServerList(SocketBase)

VOID __setnetent(__reg("a6") struct Library *, __reg("d0") LONG stay_open)="\tjsr\t-540(a6)";
#define setnetent(stay_open) __setnetent(SocketBase, (stay_open))

VOID __endnetent(__reg("a6") struct Library *)="\tjsr\t-546(a6)";
#define endnetent() __endnetent(SocketBase)

struct netent * __getnetent(__reg("a6") struct Library *)="\tjsr\t-552(a6)";
#define getnetent() __getnetent(SocketBase)

VOID __setprotoent(__reg("a6") struct Library *, __reg("d0") LONG stay_open)="\tjsr\t-558(a6)";
#define setprotoent(stay_open) __setprotoent(SocketBase, (stay_open))

VOID __endprotoent(__reg("a6") struct Library *)="\tjsr\t-564(a6)";
#define endprotoent() __endprotoent(SocketBase)

struct protoent * __getprotoent(__reg("a6") struct Library *)="\tjsr\t-570(a6)";
#define getprotoent() __getprotoent(SocketBase)

VOID __setservent(__reg("a6") struct Library *, __reg("d0") LONG stay_open)="\tjsr\t-576(a6)";
#define setservent(stay_open) __setservent(SocketBase, (stay_open))

VOID __endservent(__reg("a6") struct Library *)="\tjsr\t-582(a6)";
#define endservent() __endservent(SocketBase)

struct servent * __getservent(__reg("a6") struct Library *)="\tjsr\t-588(a6)";
#define getservent() __getservent(SocketBase)

LONG __inet_aton(__reg("a6") struct Library *, __reg("a0") STRPTR cp, __reg("a1") struct in_addr * addr)="\tjsr\t-594(a6)";
#define inet_aton(cp, addr) __inet_aton(SocketBase, (cp), (addr))

STRPTR __inet_ntop(__reg("a6") struct Library *, __reg("d0") LONG af, __reg("a0") APTR src, __reg("a1") STRPTR dst, __reg("d1") LONG size)="\tjsr\t-600(a6)";
#define inet_ntop(af, src, dst, size) __inet_ntop(SocketBase, (af), (src), (dst), (size))

LONG __inet_pton(__reg("a6") struct Library *, __reg("d0") LONG af, __reg("a0") STRPTR src, __reg("a1") APTR dst)="\tjsr\t-606(a6)";
#define inet_pton(af, src, dst) __inet_pton(SocketBase, (af), (src), (dst))

LONG __In_LocalAddr(__reg("a6") struct Library *, __reg("d0") LONG address)="\tjsr\t-612(a6)";
#define In_LocalAddr(address) __In_LocalAddr(SocketBase, (address))

LONG __In_CanForward(__reg("a6") struct Library *, __reg("d0") LONG address)="\tjsr\t-618(a6)";
#define In_CanForward(address) __In_CanForward(SocketBase, (address))

struct mbuf * __mbuf_copym(__reg("a6") struct Library *, __reg("a0") struct mbuf * m, __reg("d0") LONG off, __reg("d1") LONG len)="\tjsr\t-624(a6)";
#define mbuf_copym(m, off, len) __mbuf_copym(SocketBase, (m), (off), (len))

LONG __mbuf_copyback(__reg("a6") struct Library *, __reg("a0") struct mbuf * m, __reg("d0") LONG off, __reg("d1") LONG len, __reg("a1") APTR cp)="\tjsr\t-630(a6)";
#define mbuf_copyback(m, off, len, cp) __mbuf_copyback(SocketBase, (m), (off), (len), (cp))

LONG __mbuf_copydata(__reg("a6") struct Library *, __reg("a0") struct mbuf * m, __reg("d0") LONG off, __reg("d1") LONG len, __reg("a1") APTR cp)="\tjsr\t-636(a6)";
#define mbuf_copydata(m, off, len, cp) __mbuf_copydata(SocketBase, (m), (off), (len), (cp))

struct mbuf * __mbuf_free(__reg("a6") struct Library *, __reg("a0") struct mbuf * m)="\tjsr\t-642(a6)";
#define mbuf_free(m) __mbuf_free(SocketBase, (m))

VOID __mbuf_freem(__reg("a6") struct Library *, __reg("a0") struct mbuf * m)="\tjsr\t-648(a6)";
#define mbuf_freem(m) __mbuf_freem(SocketBase, (m))

struct mbuf * __mbuf_get(__reg("a6") struct Library *)="\tjsr\t-654(a6)";
#define mbuf_get() __mbuf_get(SocketBase)

struct mbuf * __mbuf_gethdr(__reg("a6") struct Library *)="\tjsr\t-660(a6)";
#define mbuf_gethdr() __mbuf_gethdr(SocketBase)

struct mbuf * __mbuf_prepend(__reg("a6") struct Library *, __reg("a0") struct mbuf * m, __reg("d0") LONG len)="\tjsr\t-666(a6)";
#define mbuf_prepend(m, len) __mbuf_prepend(SocketBase, (m), (len))

LONG __mbuf_cat(__reg("a6") struct Library *, __reg("a0") struct mbuf * m, __reg("a1") struct mbuf * n)="\tjsr\t-672(a6)";
#define mbuf_cat(m, n) __mbuf_cat(SocketBase, (m), (n))

LONG __mbuf_adj(__reg("a6") struct Library *, __reg("a0") struct mbuf * mp, __reg("d0") LONG req_len)="\tjsr\t-678(a6)";
#define mbuf_adj(mp, req_len) __mbuf_adj(SocketBase, (mp), (req_len))

struct mbuf * __mbuf_pullup(__reg("a6") struct Library *, __reg("a0") struct mbuf * m, __reg("d0") LONG len)="\tjsr\t-684(a6)";
#define mbuf_pullup(m, len) __mbuf_pullup(SocketBase, (m), (len))

BOOL __ProcessIsServer(__reg("a6") struct Library *, __reg("a0") struct Process * pr)="\tjsr\t-690(a6)";
#define ProcessIsServer(pr) __ProcessIsServer(SocketBase, (pr))

LONG __ObtainServerSocket(__reg("a6") struct Library *)="\tjsr\t-696(a6)";
#define ObtainServerSocket() __ObtainServerSocket(SocketBase)

BOOL __GetDefaultDomainName(__reg("a6") struct Library *, __reg("a0") STRPTR buffer, __reg("d0") LONG buffer_size)="\tjsr\t-702(a6)";
#define GetDefaultDomainName(buffer, buffer_size) __GetDefaultDomainName(SocketBase, (buffer), (buffer_size))

VOID __SetDefaultDomainName(__reg("a6") struct Library *, __reg("a0") STRPTR buffer)="\tjsr\t-708(a6)";
#define SetDefaultDomainName(buffer) __SetDefaultDomainName(SocketBase, (buffer))

struct List * __ObtainRoadshowData(__reg("a6") struct Library *, __reg("d0") LONG access)="\tjsr\t-714(a6)";
#define ObtainRoadshowData(access) __ObtainRoadshowData(SocketBase, (access))

VOID __ReleaseRoadshowData(__reg("a6") struct Library *, __reg("a0") struct List * list)="\tjsr\t-720(a6)";
#define ReleaseRoadshowData(list) __ReleaseRoadshowData(SocketBase, (list))

BOOL __ChangeRoadshowData(__reg("a6") struct Library *, __reg("a0") struct List * list, __reg("a1") STRPTR name, __reg("d0") ULONG length, __reg("a2") APTR data)="\tjsr\t-726(a6)";
#define ChangeRoadshowData(list, name, length, data) __ChangeRoadshowData(SocketBase, (list), (name), (length), (data))

LONG __RemoveInterface(__reg("a6") struct Library *, __reg("a0") STRPTR interface_name, __reg("d0") LONG force)="\tjsr\t-732(a6)";
#define RemoveInterface(interface_name, force) __RemoveInterface(SocketBase, (interface_name), (force))

struct hostent * __gethostbyname_r(__reg("a6") struct Library *, __reg("a0") STRPTR name, __reg("a1") struct hostent * hp, __reg("a2") APTR buf, __reg("d0") ULONG buflen, __reg("a3") LONG * he)="\tjsr\t-738(a6)";
#define gethostbyname_r(name, hp, buf, buflen, he) __gethostbyname_r(SocketBase, (name), (hp), (buf), (buflen), (he))

struct hostent * __gethostbyaddr_r(__reg("a6") struct Library *, __reg("a0") STRPTR addr, __reg("d0") LONG len, __reg("d1") LONG type, __reg("a1") struct hostent * hp, __reg("a2") APTR buf, __reg("d2") ULONG buflen, __reg("a3") LONG * he)="\tjsr\t-744(a6)";
#define gethostbyaddr_r(addr, len, type, hp, buf, buflen, he) __gethostbyaddr_r(SocketBase, (addr), (len), (type), (hp), (buf), (buflen), (he))

VOID __freeaddrinfo(__reg("a6") struct Library *, __reg("a0") struct addrinfo * ai)="\tjsr\t-804(a6)";
#define freeaddrinfo(ai) __freeaddrinfo(SocketBase, (ai))

LONG __getaddrinfo(__reg("a6") struct Library *, __reg("a0") STRPTR hostname, __reg("a1") STRPTR servname, __reg("a2") struct addrinfo * hints, __reg("a3") struct addrinfo ** res)="\tjsr\t-810(a6)";
#define getaddrinfo(hostname, servname, hints, res) __getaddrinfo(SocketBase, (hostname), (servname), (hints), (res))

STRPTR __gai_strerror(__reg("a6") struct Library *, __reg("a0") void * errnum)="\tjsr\t-816(a6)";
#define gai_strerror(errnum) __gai_strerror(SocketBase, (void *)(errnum))

LONG __getnameinfo(__reg("a6") struct Library *, __reg("a0") struct sockaddr * sa, __reg("d0") ULONG salen, __reg("a1") STRPTR host, __reg("d1") ULONG hostlen, __reg("a2") STRPTR serv, __reg("d2") ULONG servlen, __reg("d3") ULONG flags)="\tjsr\t-822(a6)";
#define getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) __getnameinfo(SocketBase, (sa), (salen), (host), (hostlen), (serv), (servlen), (flags))

#endif /*  _VBCCINLINE_BSDSOCKET_H  */
