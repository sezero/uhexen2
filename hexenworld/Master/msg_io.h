/*
	msg_io.h
	Message IO functions

	$Id: msg_io.h,v 1.1 2007-02-12 16:54:49 sezero Exp $
*/

#ifndef __MSGIO_H
#define __MSGIO_H

void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);

#endif	/* __MSGIO_H */

