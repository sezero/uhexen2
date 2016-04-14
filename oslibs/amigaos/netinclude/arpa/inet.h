#ifndef ARPA_INET_H
#define ARPA_INET_H \
       "$Id: inet.h,v 4.2 1996/03/26 23:26:27 too Exp $"
/*
 *      Inet Library Functions 
 *
 *      Copyright © 1994 AmiTCP/IP Group,
 *                       Network Solutions Development, Inc.
 *                       All rights reserved.
 */

#ifndef KERNEL
#ifndef NETINET_IN_H
#include <netinet/in.h>
#endif

/*
 * Include socket protos/inlines/pragmas
 */
#if !defined(BSDSOCKET_H) && !defined(CLIB_SOCKET_PROTOS_H)
#include <bsdsocket.h>
#endif

#endif /* !KERNEL */

#endif /* ARPA_INET_H */
