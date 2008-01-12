/*
	byteordr.c
	init / validate byteorder functions.

	$Id: byteordr.c,v 1.1 2008-01-12 09:46:18 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "q_endian.h"
#include "byteordr.h"

/* call this from your main() */
void ValidateByteorder (void)
{
	const char	*endianism[] = { "BE", "LE", "PDP", "Unknown" };
	const char	*tmp;

	ByteOrder_Init ();
	if (host_byteorder < 0)
		Error ("%s: Unsupported byte order.", __thisfunc__);
	switch (host_byteorder)
	{
	case BIG_ENDIAN:
		tmp = endianism[0];
		break;
	case LITTLE_ENDIAN:
		tmp = endianism[1];
		break;
	case PDP_ENDIAN:
		tmp = endianism[2];
		break;
	default:
		tmp = endianism[3];
		break;
	}
//	printf("Detected byte order: %s\n", tmp);
#if !ENDIAN_RUNTIME_DETECT
	if (host_byteorder != BYTE_ORDER)
	{
		const char	*tmp2;

		switch (BYTE_ORDER)
		{
		case BIG_ENDIAN:
			tmp2 = endianism[0];
			break;
		case LITTLE_ENDIAN:
			tmp2 = endianism[1];
			break;
		case PDP_ENDIAN:
			tmp2 = endianism[2];
			break;
		default:
			tmp2 = endianism[3];
			break;
		}
		Error ("Detected byte order %s doesn't match compiled %s order!", tmp, tmp2);
	}
#endif	/* ENDIAN_RUNTIME_DETECT */
}

