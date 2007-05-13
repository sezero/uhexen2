/*
	msg_io.c
	Message IO functions
	Handles byte ordering and avoids alignment errors

	$Id: msg_io.c,v 1.3 2007-05-13 11:59:41 sezero Exp $
*/

#include "q_types.h"
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "q_endian.h"
#include "sys.h"
#include "sizebuf.h"
#include "msg_io.h"


void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("%s: range error", __thisfunc__);
#endif

	buf = (byte *)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("%s: range error", __thisfunc__);
#endif

	buf = (byte *)SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

