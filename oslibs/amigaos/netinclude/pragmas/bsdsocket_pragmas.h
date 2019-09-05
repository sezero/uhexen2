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

#ifndef PRAGMAS_BSDSOCKET_PRAGMAS_H
#define PRAGMAS_BSDSOCKET_PRAGMAS_H

#ifndef CLIB_BSDSOCKET_PROTOS_H
#include <clib/bsdsocket_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(SocketBase,0x01e,socket(d0,d1,d2))
#pragma amicall(SocketBase,0x024,bind(d0,a0,d1))
#pragma amicall(SocketBase,0x02a,listen(d0,d1))
#pragma amicall(SocketBase,0x030,accept(d0,a0,a1))
#pragma amicall(SocketBase,0x036,connect(d0,a0,d1))
#pragma amicall(SocketBase,0x03c,sendto(d0,a0,d1,d2,a1,d3))
#pragma amicall(SocketBase,0x042,send(d0,a0,d1,d2))
#pragma amicall(SocketBase,0x048,recvfrom(d0,a0,d1,d2,a1,a2))
#pragma amicall(SocketBase,0x04e,recv(d0,a0,d1,d2))
#pragma amicall(SocketBase,0x054,shutdown(d0,d1))
#pragma amicall(SocketBase,0x05a,setsockopt(d0,d1,d2,a0,d3))
#pragma amicall(SocketBase,0x060,getsockopt(d0,d1,d2,a0,a1))
#pragma amicall(SocketBase,0x066,getsockname(d0,a0,a1))
#pragma amicall(SocketBase,0x06c,getpeername(d0,a0,a1))
#pragma amicall(SocketBase,0x072,IoctlSocket(d0,d1,a0))
#pragma amicall(SocketBase,0x078,CloseSocket(d0))
#pragma amicall(SocketBase,0x07e,WaitSelect(d0,a0,a1,a2,a3,d1))
#pragma amicall(SocketBase,0x084,SetSocketSignals(d0,d1,d2))
#pragma amicall(SocketBase,0x08a,getdtablesize())
#pragma amicall(SocketBase,0x090,ObtainSocket(d0,d1,d2,d3))
#pragma amicall(SocketBase,0x096,ReleaseSocket(d0,d1))
#pragma amicall(SocketBase,0x09c,ReleaseCopyOfSocket(d0,d1))
#pragma amicall(SocketBase,0x0a2,Errno())
#pragma amicall(SocketBase,0x0a8,SetErrnoPtr(a0,d0))
#pragma amicall(SocketBase,0x0ae,Inet_NtoA(d0))
#pragma amicall(SocketBase,0x0b4,inet_addr(a0))
#pragma amicall(SocketBase,0x0ba,Inet_LnaOf(d0))
#pragma amicall(SocketBase,0x0c0,Inet_NetOf(d0))
#pragma amicall(SocketBase,0x0c6,Inet_MakeAddr(d0,d1))
#pragma amicall(SocketBase,0x0cc,inet_network(a0))
#pragma amicall(SocketBase,0x0d2,gethostbyname(a0))
#pragma amicall(SocketBase,0x0d8,gethostbyaddr(a0,d0,d1))
#pragma amicall(SocketBase,0x0de,getnetbyname(a0))
#pragma amicall(SocketBase,0x0e4,getnetbyaddr(d0,d1))
#pragma amicall(SocketBase,0x0ea,getservbyname(a0,a1))
#pragma amicall(SocketBase,0x0f0,getservbyport(d0,a0))
#pragma amicall(SocketBase,0x0f6,getprotobyname(a0))
#pragma amicall(SocketBase,0x0fc,getprotobynumber(d0))
#pragma amicall(SocketBase,0x102,vsyslog(d0,a0,a1))
#pragma amicall(SocketBase,0x108,Dup2Socket(d0,d1))
#pragma amicall(SocketBase,0x10e,sendmsg(d0,a0,d1))
#pragma amicall(SocketBase,0x114,recvmsg(d0,a0,d1))
#pragma amicall(SocketBase,0x11a,gethostname(a0,d0))
#pragma amicall(SocketBase,0x120,gethostid())
#pragma amicall(SocketBase,0x126,SocketBaseTagList(a0))
#pragma amicall(SocketBase,0x12c,GetSocketEvents(a0))
#pragma amicall(SocketBase,0x16e,bpf_open(d0))
#pragma amicall(SocketBase,0x174,bpf_close(d0))
#pragma amicall(SocketBase,0x17a,bpf_read(d0,a0,d1))
#pragma amicall(SocketBase,0x180,bpf_write(d0,a0,d1))
#pragma amicall(SocketBase,0x186,bpf_set_notify_mask(d1,d0))
#pragma amicall(SocketBase,0x18c,bpf_set_interrupt_mask(d0,d1))
#pragma amicall(SocketBase,0x192,bpf_ioctl(d0,d1,a0))
#pragma amicall(SocketBase,0x198,bpf_data_waiting(d0))
#pragma amicall(SocketBase,0x19e,AddRouteTagList(a0))
#pragma amicall(SocketBase,0x1a4,DeleteRouteTagList(a0))
#pragma amicall(SocketBase,0x1aa,ChangeRouteTagList(a0))
#pragma amicall(SocketBase,0x1b0,FreeRouteInfo(a0))
#pragma amicall(SocketBase,0x1b6,GetRouteInfo(d0,d1))
#pragma amicall(SocketBase,0x1bc,AddInterfaceTagList(a0,a1,d0,a2))
#pragma amicall(SocketBase,0x1c2,ConfigureInterfaceTagList(a0,a1))
#pragma amicall(SocketBase,0x1c8,ReleaseInterfaceList(a0))
#pragma amicall(SocketBase,0x1ce,ObtainInterfaceList())
#pragma amicall(SocketBase,0x1d4,QueryInterfaceTagList(a0,a1))
#pragma amicall(SocketBase,0x1da,CreateAddrAllocMessageA(d0,d1,a0,a1,a2))
#pragma amicall(SocketBase,0x1e0,DeleteAddrAllocMessage(a0))
#pragma amicall(SocketBase,0x1e6,BeginInterfaceConfig(a0))
#pragma amicall(SocketBase,0x1ec,AbortInterfaceConfig(a0))
#pragma amicall(SocketBase,0x1f2,AddNetMonitorHookTagList(d0,a0,a1))
#pragma amicall(SocketBase,0x1f8,RemoveNetMonitorHook(a0))
#pragma amicall(SocketBase,0x1fe,GetNetworkStatistics(d0,d1,a0,d2))
#pragma amicall(SocketBase,0x204,AddDomainNameServer(a0))
#pragma amicall(SocketBase,0x20a,RemoveDomainNameServer(a0))
#pragma amicall(SocketBase,0x210,ReleaseDomainNameServerList(a0))
#pragma amicall(SocketBase,0x216,ObtainDomainNameServerList())
#pragma amicall(SocketBase,0x21c,setnetent(d0))
#pragma amicall(SocketBase,0x222,endnetent())
#pragma amicall(SocketBase,0x228,getnetent())
#pragma amicall(SocketBase,0x22e,setprotoent(d0))
#pragma amicall(SocketBase,0x234,endprotoent())
#pragma amicall(SocketBase,0x23a,getprotoent())
#pragma amicall(SocketBase,0x240,setservent(d0))
#pragma amicall(SocketBase,0x246,endservent())
#pragma amicall(SocketBase,0x24c,getservent())
#pragma amicall(SocketBase,0x252,inet_aton(a0,a1))
#pragma amicall(SocketBase,0x258,inet_ntop(d0,a0,a1,d1))
#pragma amicall(SocketBase,0x25e,inet_pton(d0,a0,a1))
#pragma amicall(SocketBase,0x264,In_LocalAddr(d0))
#pragma amicall(SocketBase,0x26a,In_CanForward(d0))
#pragma amicall(SocketBase,0x270,mbuf_copym(a0,d0,d1))
#pragma amicall(SocketBase,0x276,mbuf_copyback(a0,d0,d1,a1))
#pragma amicall(SocketBase,0x27c,mbuf_copydata(a0,d0,d1,a1))
#pragma amicall(SocketBase,0x282,mbuf_free(a0))
#pragma amicall(SocketBase,0x288,mbuf_freem(a0))
#pragma amicall(SocketBase,0x28e,mbuf_get())
#pragma amicall(SocketBase,0x294,mbuf_gethdr())
#pragma amicall(SocketBase,0x29a,mbuf_prepend(a0,d0))
#pragma amicall(SocketBase,0x2a0,mbuf_cat(a0,a1))
#pragma amicall(SocketBase,0x2a6,mbuf_adj(a0,d0))
#pragma amicall(SocketBase,0x2ac,mbuf_pullup(a0,d0))
#pragma amicall(SocketBase,0x2b2,ProcessIsServer(a0))
#pragma amicall(SocketBase,0x2b8,ObtainServerSocket())
#pragma amicall(SocketBase,0x2be,GetDefaultDomainName(a0,d0))
#pragma amicall(SocketBase,0x2c4,SetDefaultDomainName(a0))
#pragma amicall(SocketBase,0x2ca,ObtainRoadshowData(d0))
#pragma amicall(SocketBase,0x2d0,ReleaseRoadshowData(a0))
#pragma amicall(SocketBase,0x2d6,ChangeRoadshowData(a0,a1,d0,a2))
#pragma amicall(SocketBase,0x2dc,RemoveInterface(a0,d0))
#pragma amicall(SocketBase,0x2e2,gethostbyname_r(a0,a1,a2,d0,a3))
#pragma amicall(SocketBase,0x2e8,gethostbyaddr_r(a0,d0,d1,a1,a2,d2,a3))
#pragma amicall(SocketBase,0x2fa,ipf_open(d0))
#pragma amicall(SocketBase,0x300,ipf_close(d0))
#pragma amicall(SocketBase,0x306,ipf_ioctl(d0,d1,a0))
#pragma amicall(SocketBase,0x30c,ipf_log_read(d0,a0,d1))
#pragma amicall(SocketBase,0x312,ipf_log_data_waiting(d0))
#pragma amicall(SocketBase,0x318,ipf_set_notify_mask(d0,d1))
#pragma amicall(SocketBase,0x31e,ipf_set_interrupt_mask(d0,d1))
#pragma amicall(SocketBase,0x324,freeaddrinfo(a0))
#pragma amicall(SocketBase,0x32a,getaddrinfo(a0,a1,a2,a3))
#pragma amicall(SocketBase,0x330,gai_strerror(a0))
#pragma amicall(SocketBase,0x336,getnameinfo(a0,d0,a1,d1,a2,d2,d3))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall SocketBase socket                 01e 21003
#pragma  libcall SocketBase bind                   024 18003
#pragma  libcall SocketBase listen                 02a 1002
#pragma  libcall SocketBase accept                 030 98003
#pragma  libcall SocketBase connect                036 18003
#pragma  libcall SocketBase sendto                 03c 39218006
#pragma  libcall SocketBase send                   042 218004
#pragma  libcall SocketBase recvfrom               048 a9218006
#pragma  libcall SocketBase recv                   04e 218004
#pragma  libcall SocketBase shutdown               054 1002
#pragma  libcall SocketBase setsockopt             05a 3821005
#pragma  libcall SocketBase getsockopt             060 9821005
#pragma  libcall SocketBase getsockname            066 98003
#pragma  libcall SocketBase getpeername            06c 98003
#pragma  libcall SocketBase IoctlSocket            072 81003
#pragma  libcall SocketBase CloseSocket            078 001
#pragma  libcall SocketBase WaitSelect             07e 1ba98006
#pragma  libcall SocketBase SetSocketSignals       084 21003
#pragma  libcall SocketBase getdtablesize          08a 00
#pragma  libcall SocketBase ObtainSocket           090 321004
#pragma  libcall SocketBase ReleaseSocket          096 1002
#pragma  libcall SocketBase ReleaseCopyOfSocket    09c 1002
#pragma  libcall SocketBase Errno                  0a2 00
#pragma  libcall SocketBase SetErrnoPtr            0a8 0802
#pragma  libcall SocketBase Inet_NtoA              0ae 001
#pragma  libcall SocketBase inet_addr              0b4 801
#pragma  libcall SocketBase Inet_LnaOf             0ba 001
#pragma  libcall SocketBase Inet_NetOf             0c0 001
#pragma  libcall SocketBase Inet_MakeAddr          0c6 1002
#pragma  libcall SocketBase inet_network           0cc 801
#pragma  libcall SocketBase gethostbyname          0d2 801
#pragma  libcall SocketBase gethostbyaddr          0d8 10803
#pragma  libcall SocketBase getnetbyname           0de 801
#pragma  libcall SocketBase getnetbyaddr           0e4 1002
#pragma  libcall SocketBase getservbyname          0ea 9802
#pragma  libcall SocketBase getservbyport          0f0 8002
#pragma  libcall SocketBase getprotobyname         0f6 801
#pragma  libcall SocketBase getprotobynumber       0fc 001
#pragma  libcall SocketBase vsyslog                102 98003
#pragma  libcall SocketBase Dup2Socket             108 1002
#pragma  libcall SocketBase sendmsg                10e 18003
#pragma  libcall SocketBase recvmsg                114 18003
#pragma  libcall SocketBase gethostname            11a 0802
#pragma  libcall SocketBase gethostid              120 00
#pragma  libcall SocketBase SocketBaseTagList      126 801
#pragma  libcall SocketBase GetSocketEvents        12c 801
#pragma  libcall SocketBase bpf_open               16e 001
#pragma  libcall SocketBase bpf_close              174 001
#pragma  libcall SocketBase bpf_read               17a 18003
#pragma  libcall SocketBase bpf_write              180 18003
#pragma  libcall SocketBase bpf_set_notify_mask    186 0102
#pragma  libcall SocketBase bpf_set_interrupt_mask 18c 1002
#pragma  libcall SocketBase bpf_ioctl              192 81003
#pragma  libcall SocketBase bpf_data_waiting       198 001
#pragma  libcall SocketBase AddRouteTagList        19e 801
#pragma  libcall SocketBase DeleteRouteTagList     1a4 801
#pragma  libcall SocketBase ChangeRouteTagList     1aa 801
#pragma  libcall SocketBase FreeRouteInfo          1b0 801
#pragma  libcall SocketBase GetRouteInfo           1b6 1002
#pragma  libcall SocketBase AddInterfaceTagList    1bc a09804
#pragma  libcall SocketBase ConfigureInterfaceTagList 1c2 9802
#pragma  libcall SocketBase ReleaseInterfaceList   1c8 801
#pragma  libcall SocketBase ObtainInterfaceList    1ce 00
#pragma  libcall SocketBase QueryInterfaceTagList  1d4 9802
#pragma  libcall SocketBase CreateAddrAllocMessageA 1da a981005
#pragma  libcall SocketBase DeleteAddrAllocMessage 1e0 801
#pragma  libcall SocketBase BeginInterfaceConfig   1e6 801
#pragma  libcall SocketBase AbortInterfaceConfig   1ec 801
#pragma  libcall SocketBase AddNetMonitorHookTagList 1f2 98003
#pragma  libcall SocketBase RemoveNetMonitorHook   1f8 801
#pragma  libcall SocketBase GetNetworkStatistics   1fe 281004
#pragma  libcall SocketBase AddDomainNameServer    204 801
#pragma  libcall SocketBase RemoveDomainNameServer 20a 801
#pragma  libcall SocketBase ReleaseDomainNameServerList 210 801
#pragma  libcall SocketBase ObtainDomainNameServerList 216 00
#pragma  libcall SocketBase setnetent              21c 001
#pragma  libcall SocketBase endnetent              222 00
#pragma  libcall SocketBase getnetent              228 00
#pragma  libcall SocketBase setprotoent            22e 001
#pragma  libcall SocketBase endprotoent            234 00
#pragma  libcall SocketBase getprotoent            23a 00
#pragma  libcall SocketBase setservent             240 001
#pragma  libcall SocketBase endservent             246 00
#pragma  libcall SocketBase getservent             24c 00
#pragma  libcall SocketBase inet_aton              252 9802
#pragma  libcall SocketBase inet_ntop              258 198004
#pragma  libcall SocketBase inet_pton              25e 98003
#pragma  libcall SocketBase In_LocalAddr           264 001
#pragma  libcall SocketBase In_CanForward          26a 001
#pragma  libcall SocketBase mbuf_copym             270 10803
#pragma  libcall SocketBase mbuf_copyback          276 910804
#pragma  libcall SocketBase mbuf_copydata          27c 910804
#pragma  libcall SocketBase mbuf_free              282 801
#pragma  libcall SocketBase mbuf_freem             288 801
#pragma  libcall SocketBase mbuf_get               28e 00
#pragma  libcall SocketBase mbuf_gethdr            294 00
#pragma  libcall SocketBase mbuf_prepend           29a 0802
#pragma  libcall SocketBase mbuf_cat               2a0 9802
#pragma  libcall SocketBase mbuf_adj               2a6 0802
#pragma  libcall SocketBase mbuf_pullup            2ac 0802
#pragma  libcall SocketBase ProcessIsServer        2b2 801
#pragma  libcall SocketBase ObtainServerSocket     2b8 00
#pragma  libcall SocketBase GetDefaultDomainName   2be 0802
#pragma  libcall SocketBase SetDefaultDomainName   2c4 801
#pragma  libcall SocketBase ObtainRoadshowData     2ca 001
#pragma  libcall SocketBase ReleaseRoadshowData    2d0 801
#pragma  libcall SocketBase ChangeRoadshowData     2d6 a09804
#pragma  libcall SocketBase RemoveInterface        2dc 0802
#pragma  libcall SocketBase gethostbyname_r        2e2 b0a9805
#pragma  libcall SocketBase gethostbyaddr_r        2e8 b2a910807
#pragma  libcall SocketBase ipf_open               2fa 001
#pragma  libcall SocketBase ipf_close              300 001
#pragma  libcall SocketBase ipf_ioctl              306 81003
#pragma  libcall SocketBase ipf_log_read           30c 18003
#pragma  libcall SocketBase ipf_log_data_waiting   312 001
#pragma  libcall SocketBase ipf_set_notify_mask    318 1002
#pragma  libcall SocketBase ipf_set_interrupt_mask 31e 1002
#pragma  libcall SocketBase freeaddrinfo           324 801
#pragma  libcall SocketBase getaddrinfo            32a ba9804
#pragma  libcall SocketBase gai_strerror           330 801
#pragma  libcall SocketBase getnameinfo            336 32a190807
#endif
#ifdef __STORM__
#pragma tagcall(SocketBase,0x102,syslog(d0,a0,a1))
#pragma tagcall(SocketBase,0x126,SocketBaseTags(a0))
#pragma tagcall(SocketBase,0x19e,AddRouteTags(a0))
#pragma tagcall(SocketBase,0x1a4,DeleteRouteTags(a0))
#pragma tagcall(SocketBase,0x1aa,ChangeRouteTags(a0))
#pragma tagcall(SocketBase,0x1bc,AddInterfaceTags(a0,a1,d0,a2))
#pragma tagcall(SocketBase,0x1c2,ConfigureInterfaceTags(a0,a1))
#pragma tagcall(SocketBase,0x1d4,QueryInterfaceTags(a0,a1))
#pragma tagcall(SocketBase,0x1da,CreateAddrAllocMessage(d0,d1,a0,a1,a2))
#pragma tagcall(SocketBase,0x1f2,AddNetMonitorHookTags(d0,a0,a1))
#endif
#ifdef __SASC_60
#pragma  tagcall SocketBase syslog                 102 98003
#pragma  tagcall SocketBase SocketBaseTags         126 801
#pragma  tagcall SocketBase AddRouteTags           19e 801
#pragma  tagcall SocketBase DeleteRouteTags        1a4 801
#pragma  tagcall SocketBase ChangeRouteTags        1aa 801
#pragma  tagcall SocketBase AddInterfaceTags       1bc a09804
#pragma  tagcall SocketBase ConfigureInterfaceTags 1c2 9802
#pragma  tagcall SocketBase QueryInterfaceTags     1d4 9802
#pragma  tagcall SocketBase CreateAddrAllocMessage 1da a981005
#pragma  tagcall SocketBase AddNetMonitorHookTags  1f2 98003
#endif

#endif /* PRAGMAS_BSDSOCKET_PRAGMAS_H */
