/*
 * sizebuf.c -- sized buffers
 * $Id: sizebuf.c,v 1.8 2009-01-26 09:05:22 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "q_stdinc.h"
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
	int		len = (int)strlen(data) + 1;

	if (!buf->cursize || buf->data[buf->cursize-1])
	{	/* no trailing 0 */
		memcpy ((byte *)SZ_GetSpace(buf, len  )  , data, len);
	}
	else
	{	/* write over trailing 0 */
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1, data, len);
	}
}

