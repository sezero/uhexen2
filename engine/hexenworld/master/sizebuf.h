/*
	sizebuf.h
	sized buffer defs

	$Id: sizebuf.h,v 1.2 2007-07-04 09:30:22 sezero Exp $
*/

#ifndef __SIZEBUF_H
#define __SIZEBUF_H

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;	// set to true if the buffer size failed
	byte		*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;


void SZ_Init (sizebuf_t *buf, byte *data, int length);
			// if the data buffer is NULL, a new one will be allocated.

void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);


#endif	/* __SIZEBUF_H */

