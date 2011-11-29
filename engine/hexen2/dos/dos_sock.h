/*
 * dos_sock.h
 * quick socket api compatibility definitions for dos.
 * $Id$
 */

#ifndef _DOS_SOCKDEFS_H
#define _DOS_SOCKDEFS_H

#if defined(USE_WATT32)
#include "net_sys.h"
#else	/* USE_WATT32 */
/*
typedef int		sys_socket_t;
*/
#define	sys_socket_t	int

#define	INVALID_SOCKET	(-1)
#define	SOCKET_ERROR	(-1)
#endif	/* USE_WATT32 */

#endif	/* _DOS_SOCKDEFS_H */

