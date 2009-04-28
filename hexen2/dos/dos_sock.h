/*
 * dos_sock.h
 * quick socket api compatibility definitions for dos.
 * $Id: dos_sock.h,v 1.1 2009-04-28 12:03:27 sezero Exp $
 */

#ifndef _DOS_SOCKDEFS_H
#define _DOS_SOCKDEFS_H

/*
typedef int		sys_socket_t;
*/
#define	sys_socket_t	int

#define	INVALID_SOCKET	(-1)
#define	SOCKET_ERROR	(-1)

#endif	/* _DOS_SOCKDEFS_H */

