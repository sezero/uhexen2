/*
	d_clear.c
	clears a specified rectangle to the specified color

	$Id: d_fill.c,v 1.6 2007-06-16 14:41:35 sezero Exp $
*/

#include "quakedef.h"

/*
================
D_FillRect
================
*/
void D_FillRect (vrect_t *rect, int color)
{
	int				rx, ry, rwidth, rheight;
	unsigned char	*dest;
	unsigned		*ldest;

	rx = rect->x;
	ry = rect->y;
	rwidth = rect->width;
	rheight = rect->height;

	if (rx < 0)
	{
		rwidth += rx;
		rx = 0;
	}
	if (ry < 0)
	{
		rheight += ry;
		ry = 0;
	}
	if (rx+rwidth > vid.width)
		rwidth = vid.width - rx;
	if (ry+rheight > vid.height)
		rheight = vid.height - rx;

	if (rwidth < 1 || rheight < 1)
		return;

	dest = ((byte *)vid.buffer + ry*vid.rowbytes + rx);

	if (((rwidth & 0x03) == 0) && (((intptr_t)dest & 0x03) == 0))
	{
	// faster aligned dword clear
		ldest = (unsigned *)dest;
		color += color << 16;

		rwidth >>= 2;
		color += color << 8;

		for (ry = 0; ry < rheight; ry++)
		{
			for (rx = 0; rx < rwidth; rx++)
				ldest[rx] = color;
			ldest = (unsigned *)((byte*)ldest + vid.rowbytes);
		}
	}
	else
	{
	// slower byte-by-byte clear for unaligned cases
		for (ry = 0; ry < rheight; ry++)
		{
			for (rx = 0; rx < rwidth; rx++)
				dest[rx] = color;
			dest += vid.rowbytes;
		}
	}
}

