/*
	sizebuf.c
	sized buffers

	$Id: sizebuf.c,v 1.5 2007-12-14 16:41:13 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "sys.h"
#include "sizebuf.h"


void SZ_Init (sizebuf_t *buf, byte *data, int length)
{
	memset (buf, 0, sizeof(*buf));
	if (data != NULL)
	{
		buf->data = data;
		buf->maxsize = length;
	}
	else
	{
		if (length < 256)
			length = 256;
		buf->data = (byte *) malloc (length);
		buf->maxsize = length;
	}
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void	*data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("%s: overflow without allowoverflow set (%d)", __thisfunc__, buf->maxsize);

		if (length > buf->maxsize)
			Sys_Error ("%s: %i is > full buffer size", __thisfunc__, length);

		printf ("%s: overflow\n", __thisfunc__);
		SZ_Clear (buf);
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write (sizebuf_t *buf, const void *data, int length)
{
	memcpy (SZ_GetSpace(buf,length),data,length);
}

