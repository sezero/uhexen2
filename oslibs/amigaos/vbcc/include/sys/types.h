/* the bare minimum needed for us with vbcc */
#ifndef VBCC_SYS_TYPES_H_
#define	VBCC_SYS_TYPES_H_

#include <machine/types.h>
#include <machine/endian.h>

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		/* Sys V compatibility */
typedef	unsigned int	uint;		/* Sys V compatibility */

typedef	char *		caddr_t;	/* core address */
typedef	int32_t		off_t;		/* file offset */
typedef	int		ssize_t;

#define	NBBY	8		/* number of bits in a byte */

#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef int32_t	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#ifndef howmany
#define	howmany(x, y)	(((x) + ((y) - 1)) / (y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define	FD_ZERO(p)	memset((void *)(p), 0, sizeof(*(p)))

#endif /* !_SYS_TYPES_H_ */
