#ifdef __CLIB2__
#include_next <sys/types.h>

#ifndef clib2_missing_typedefs
#define clib2_missing_typedefs
/* missing typedefs : */
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;
typedef	unsigned int	uint;
/* for fd_set & co */
#define __NO_WAITSELECT
#define __NO_NET_API
#include <sys/select.h>
#endif

#else /* __CLIB2__ */

/* workaround for ixemul headers >= 48.3:
 * AmiTCP SDK expect socklen_t as LONG,
 * however ixemul defines it as int... */
#ifndef sys_types_hackity_h
#define sys_types_hackity_h
#define socklen_t ixemul_socklen_t
#include_next <sys/types.h>
#undef socklen_t
#endif

#endif /* __CLIB2__ */
