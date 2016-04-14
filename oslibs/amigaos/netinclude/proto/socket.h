#ifndef PROTO_SOCKET_H
#define PROTO_SOCKET_H \
       "$Id: socket.h,v 4.2 1996/02/26 21:28:14 too Exp $"
/*
 *	SAS C Prototypes for bsdsocket.library
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

#ifndef __NOLIBBASE__       
extern struct Library *SocketBase;
#endif

#include <clib/socket_protos.h>
#if __SASC
#include <pragmas/socket_pragmas.h>
#elif __GNUC__
#include <inline/socket.h>
#endif

#ifdef _OPTINLINE		/* for SAS C 6.3 and later */
#include <clib/socket_inlines.h>
#endif
#endif /* PROTO_SOCKET_H */
