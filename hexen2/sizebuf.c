/*
	sizebuf.c
	sized buffers

	$Id: sizebuf.c,v 1.5 2007-07-04 08:49:58 sezero Exp $
*/

#include "q_types.h"
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "sys.h"
#include "printsys.h"
#include "sizebuf.h"
#include "zone.h"


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
		buf->data = (byte *) Hunk_AllocName (length, "sizebuf");
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
			Sys_Error ("%s: overflow without allowoverflow set", __thisfunc__);

		if (length > buf->maxsize)
			Sys_Error ("%s: %i is > full buffer size", __thisfunc__, length);

		Sys_Printf ("%s: overflow\nCurrently %d of %d, requested %d\n",
				__thisfunc__, buf->cursize, buf->maxsize, length);
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

void SZ_Print (sizebuf_t *buf, const char *data)
{
	int		len;

	len = strlen(data)+1;

	if (!buf->cursize || buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}

