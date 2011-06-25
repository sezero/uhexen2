/*
 * dos_sock.h
 * quick socket api compatibility definitions for dos.
 * $Id$
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

