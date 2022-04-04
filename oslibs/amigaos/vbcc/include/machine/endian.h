/* the bare minimum needed for us with vbcc */

#ifndef VBCC_M68K_ENDIAN_H
#define VBCC_M68K_ENDIAN_H

#define	LITTLE_ENDIAN	1234	/* LSB first: i386, vax */
#define	BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long */

#define	BYTE_ORDER	BIG_ENDIAN

#endif
