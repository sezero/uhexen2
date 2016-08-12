/*
 * draw.c
 * This is the only file outside the refresh that touches the vid buffer.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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


#include "quakedef.h"
#include "r_shared.h"

typedef struct {
	vrect_t	rect;
	int		width;
	int		height;
	byte	*ptexbytes;
	int		rowbytes;
} rectdesc_t;

static rectdesc_t	r_rectdesc;

static byte	*draw_smallchars;	// Small characters for status bar
static byte	*draw_chars;		// 8*8 graphic characters
static qpic_t	*draw_backtile;
#if defined(DRAW_LOADINGSKULL)
static qpic_t	*draw_disc[MAX_DISC] =
{
	NULL	// make the first one null for sure
};
#endif

qboolean	draw_reinit = false;

//=============================================================================
/* Support Routines */

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	cache_user_t	cache;
} cachepic_t;

#define	MAX_CACHED_PICS		256
static cachepic_t	menu_cachepics[MAX_CACHED_PICS];
static int		menu_numcachepics;


static void Draw_PicCheckError (void *ptr, const char *name)
{
	if (!ptr)
		Sys_Error ("Failed to load %s", name);
}


qpic_t	*Draw_PicFromWad (const char *name)
{
	return (qpic_t *) W_GetLumpName(name);
}

/*
================
Draw_CachePic
================
*/
qpic_t	*Draw_CachePic (const char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;

	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
		if (!strcmp (path, pic->name))
			break;

	if (i == menu_numcachepics)
	{
		if (menu_numcachepics == MAX_CACHED_PICS)
			Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
		menu_numcachepics++;
		q_strlcpy (pic->name, path, MAX_QPATH);
	}

	dat = (qpic_t *) Cache_Check (&pic->cache);
	if (dat)
		return dat;

//
// load the pic from disk
//
	FS_LoadCacheFile (path, &pic->cache, NULL);

	dat = (qpic_t *)pic->cache.data;
	Draw_PicCheckError (dat, path);
	SwapPic (dat);

	return dat;
}

#if !defined(DRAW_PROGRESSBARS)
/*
================
Draw_CacheLoadingPic
like Draw_CachePic() but only for loading.lmp
with its progress bars eliminated.
================
*/
static const char ls_path[] = "gfx/menu/loading.lmp";
qpic_t	*Draw_CacheLoadingPic (void)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;

	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
		if (!strcmp (ls_path, pic->name))
			break;

	if (i == menu_numcachepics)
	{
		if (menu_numcachepics == MAX_CACHED_PICS)
			Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
		menu_numcachepics++;
		q_strlcpy (pic->name, ls_path, MAX_QPATH);
	}

	dat = (qpic_t *) Cache_Check (&pic->cache);
	if (dat)
		return dat;

//
// load the pic from disk
//
	FS_LoadCacheFile (ls_path, &pic->cache, NULL);

	dat = (qpic_t *)pic->cache.data;
	Draw_PicCheckError (dat, ls_path);
	SwapPic (dat);

	if (fs_filesize != 17592 || dat->width != 157 || dat->height != 112)
		return dat;

	/* kill the progress slot pixels between rows [85:103] */
	memmove(dat->data + 157*85, dat->data + 157*104, 157*(112 - 104));
	dat->height -= (104 - 85);

	return dat;
}

#endif	/* !DRAW_PROGRESSBARS */


#if FULLSCREEN_INTERMISSIONS
/*
================
Draw_CachePicResize
New function by Pa3PyX; will load a pic resizing it (needed for intermissions)
================
*/
qpic_t *Draw_CachePicResize (const char *path, int targetWidth, int targetHeight)
{
	cachepic_t *pic;
	int i, j;
	int sourceWidth, sourceHeight;
	qpic_t *dat, *temp;

	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
	{
		if (!strcmp(path, pic->name))
			break;
	}
	if (i == menu_numcachepics)
	{
		if (menu_numcachepics == MAX_CACHED_PICS)
			Sys_Error("menu_numcachepics == MAX_CACHED_PICS");
		menu_numcachepics++;
		q_strlcpy(pic->name, path, MAX_QPATH);
	}
	dat = (qpic_t *) Cache_Check(&pic->cache);
	if (dat)
	{
		if (targetWidth == dat->width && targetHeight == dat->height)
			return dat;
		else
			Cache_Free (&pic->cache);
	}
	// Allocate original data temporarily
	temp = (qpic_t *)FS_LoadTempFile(path, NULL);
	Draw_PicCheckError (temp, path);
	SwapPic(temp);
	/* I wish Carmack would thought of something more intuitive than
	   out-of-bounds array for storing image data */
	Cache_Alloc(&pic->cache, targetWidth * targetHeight * sizeof(byte) + sizeof(qpic_t), path);
	dat = (qpic_t *)pic->cache.data;
	if (!dat)
		Sys_Error("%s: failed to load %s (cache flushed prematurely)", __thisfunc__, path);
	dat->width = targetWidth;
	dat->height = targetHeight;
	sourceWidth = temp->width;
	sourceHeight = temp->height;
	for (j = 0; j < targetHeight; j++)
	{
		for (i = 0; i < targetWidth; i++)
		{
			dat->data[i + targetWidth * j] = temp->data[(i * sourceWidth / targetWidth) + sourceWidth * (j * sourceHeight / targetHeight)];
		}
	}
	return dat;
}
#endif	/* FULLSCREEN_INTERMISSIONS */

/*
===============
Draw_Init
===============
*/
void Draw_Init (void)
{
#if defined(DRAW_LOADINGSKULL)
	int		i;
	char	temp[MAX_QPATH];

	if (!draw_reinit)
	{
		for (i = 0; i < MAX_DISC; i++)
			draw_disc[i] = NULL;
	}
	// Do this backwards so we don't try and draw the
	// skull as we are loading
	for (i = MAX_DISC - 1; i >= 0; i--)
	{
		if (draw_disc[i])
			Z_Free (draw_disc[i]);
		q_snprintf(temp, sizeof(temp), "gfx/menu/skull%d.lmp", i);
		draw_disc[i] = (qpic_t *)FS_LoadZoneFile (temp, Z_SECZONE, NULL);
	//	Draw_PicCheckError (draw_disc[i], temp);
		if (draw_disc[i])
			SwapPic (draw_disc[i]);
	}
#endif	/* DRAW_LOADINGSKULL */

	if (draw_chars)
		Z_Free (draw_chars);
	draw_chars = FS_LoadZoneFile ("gfx/menu/conchars.lmp", Z_SECZONE, NULL);
	Draw_PicCheckError (draw_chars, "gfx/menu/conchars.lmp");

	draw_smallchars = (byte *) W_GetLumpName("tinyfont");

	if (draw_backtile)
		Z_Free (draw_backtile);
	draw_backtile = (qpic_t	*)FS_LoadZoneFile ("gfx/menu/backtile.lmp", Z_SECZONE, NULL);
	Draw_PicCheckError (draw_backtile, "gfx/menu/backtile.lmp");
	SwapPic (draw_backtile);

	r_rectdesc.width = draw_backtile->width;
	r_rectdesc.height = draw_backtile->height;
	r_rectdesc.ptexbytes = draw_backtile->data;
	r_rectdesc.rowbytes = draw_backtile->width;
}

/*
===============
Draw_ReInit
Delete and reload textures that read during engine's
init phase which may be changed by mods and purge all
others, i.e. map/model textures.
Should NEVER be called when a map is active: Only
intended to be called upon a game directory change.
===============
*/
void Draw_ReInit (void)
{
	int	temp;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	draw_reinit = true;

	W_LoadWadFile ("gfx.wad");
	Draw_Init();
	SCR_Init();
	Sbar_Init();

	draw_reinit = false;
	scr_disabled_for_loading = temp;
}


/*
================
Draw_Character

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Character (int x, int y, unsigned int num)
{
	byte		*source;
	int		drawline;
	int		row, col;

	num &= 511;

	if (y <= -8)
		return;			// totally off screen

	if (y > vid.height - 8 || x < 0 || x > vid.width - 8)
		return;

	row = num >> 5;
	col = num & 31;
	source = draw_chars + (row<<11) + (col<<3);

	if (y < 0)
	{	// clipped
		drawline = 8 + y;
		source -= 256*y;
		y = 0;
	}
	else
		drawline = 8;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.conbuffer + y*vid.conrowbytes + x;
		switch (trans_level)
		{
		case 0:
			while (drawline--)
			{
				if (source[0])
					dest[0] = source[0];
				if (source[1])
					dest[1] = source[1];
				if (source[2])
					dest[2] = source[2];
				if (source[3])
					dest[3] = source[3];
				if (source[4])
					dest[4] = source[4];
				if (source[5])
					dest[5] = source[5];
				if (source[6])
					dest[6] = source[6];
				if (source[7])
					dest[7] = source[7];
				source += 256;
				dest += vid.conrowbytes;
			}
			break;
		case 1:
			while (drawline--)
			{
				if (source[0])
					dest[0] = mainTransTable[(((unsigned int)dest[0])<<8) + source[0]];
				if (source[1])
					dest[1] = mainTransTable[(((unsigned int)dest[1])<<8) + source[1]];
				if (source[2])
					dest[2] = mainTransTable[(((unsigned int)dest[2])<<8) + source[2]];
				if (source[3])
					dest[3] = mainTransTable[(((unsigned int)dest[3])<<8) + source[3]];
				if (source[4])
					dest[4] = mainTransTable[(((unsigned int)dest[4])<<8) + source[4]];
				if (source[5])
					dest[5] = mainTransTable[(((unsigned int)dest[5])<<8) + source[5]];
				if (source[6])
					dest[6] = mainTransTable[(((unsigned int)dest[6])<<8) + source[6]];
				if (source[7])
					dest[7] = mainTransTable[(((unsigned int)dest[7])<<8) + source[7]];
				source += 256;
				dest += vid.conrowbytes;
			}
			break;
		case 2:
			while (drawline--)
			{
				if (source[0])
					dest[0] = mainTransTable[(((unsigned int)source[0])<<8) + dest[0]];
				if (source[1])
					dest[1] = mainTransTable[(((unsigned int)source[1])<<8) + dest[1]];
				if (source[2])
					dest[2] = mainTransTable[(((unsigned int)source[2])<<8) + dest[2]];
				if (source[3])
					dest[3] = mainTransTable[(((unsigned int)source[3])<<8) + dest[3]];
				if (source[4])
					dest[4] = mainTransTable[(((unsigned int)source[4])<<8) + dest[4]];
				if (source[5])
					dest[5] = mainTransTable[(((unsigned int)source[5])<<8) + dest[5]];
				if (source[6])
					dest[6] = mainTransTable[(((unsigned int)source[6])<<8) + dest[6]];
				if (source[7])
					dest[7] = mainTransTable[(((unsigned int)source[7])<<8) + dest[7]];
				source += 256;
				dest += vid.conrowbytes;
			}
			break;
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pre-expand to native format?
		unsigned short *dest = (unsigned short *)
				((byte *)vid.conbuffer + y*vid.conrowbytes + (x<<1));
		// FIXME: transparency bits are missing
		while (drawline--)
		{
			if (source[0])
				dest[0] = d_8to16table[source[0]];
			if (source[1])
				dest[1] = d_8to16table[source[1]];
			if (source[2])
				dest[2] = d_8to16table[source[2]];
			if (source[3])
				dest[3] = d_8to16table[source[3]];
			if (source[4])
				dest[4] = d_8to16table[source[4]];
			if (source[5])
				dest[5] = d_8to16table[source[5]];
			if (source[6])
				dest[6] = d_8to16table[source[6]];
			if (source[7])
				dest[7] = d_8to16table[source[7]];

			source += 256;
			dest += vid.conrowbytes / 2;
		}
	}
}

/*
================
Draw_String
================
*/
void Draw_String (int x, int y, const char *str)
{
	while (*str)
	{
		Draw_Character (x, y, *str);
		str++;
		x += 8;
	}
}

void Draw_RedString (int x, int y, const char *str)
{
	while (*str)
	{
		Draw_Character (x, y, ((unsigned char)(*str))+256);
		str++;
		x += 8;
	}
}

static void Draw_Pixel (int x, int y, const byte color)
{
	if (r_pixbytes == 1)
	{
		byte *dest = vid.conbuffer + y*vid.conrowbytes + x;
		*dest = color;
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pre-expand to native format?
		unsigned short *dest = (unsigned short *)
				((byte *)vid.conbuffer + y*vid.conrowbytes + (x<<1));
		*dest = d_8to16table[color];
	}
}

/*
================
Draw_Crosshair
================
*/
void Draw_Crosshair (void)
{
	int x, y;
	byte c = (byte)crosshaircolor.integer;

	x = scr_vrect.x + scr_vrect.width/2 + cl_crossx.value;
	y = scr_vrect.y + scr_vrect.height/2 + cl_crossy.value;

	if (crosshair.integer == 2)
	{
		Draw_Pixel(x - 1, y, c);
		Draw_Pixel(x - 3, y, c);
		Draw_Pixel(x + 1, y, c);
		Draw_Pixel(x + 3, y, c);
		Draw_Pixel(x, y - 1, c);
		Draw_Pixel(x, y - 3, c);
		Draw_Pixel(x, y + 1, c);
		Draw_Pixel(x, y + 3, c);
	}
	else if (crosshair.integer)
	{
		Draw_Character (x - 4, y - 4, '+');
	}
}

//==========================================================================
//
// Draw_SmallCharacter
//
// Draws a small character that is clipped at the bottom edge of the
// screen.
//
//==========================================================================

void Draw_SmallCharacter (int x, int y, int num)
{
	byte		*source;
	int		height, row, col;

	if (num < 32)
	{
		num = 0;
	}
	else if (num >= 'a' && num <= 'z')
	{
		num -= 64;
	}
	else if (num > '_')
	{
		num = 0;
	}
	else
	{
		num -= 32;
	}

	if (y >= vid.height)
	{ // Totally off screen
		return;
	}

#ifdef PARANOID
	if ((y < 0) || (x < 0) || (x+8 > vid.width))
	{
		Sys_Error("Bad Draw_SmallCharacter: (%d, %d)", x, y);
	}
#endif

	if (y + 5 > vid.height)
	{
		height = vid.height - y;
	}
	else
	{
		height = 5;
	}

	row = num >> 4;
	col = num & 15;
	source = draw_smallchars + (row<<10) + (col<<3);

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y*vid.rowbytes + x;
		switch (trans_level)
		{
		case 0:
			while (height--)
			{
				if (source[0])
					dest[0] = source[0];
				if (source[1])
					dest[1] = source[1];
				if (source[2])
					dest[2] = source[2];
				if (source[3])
					dest[3] = source[3];
				if (source[4])
					dest[4] = source[4];
				if (source[5])
					dest[5] = source[5];
				if (source[6])
					dest[6] = source[6];
				if (source[7])
					dest[7] = source[7];
				source += 128;
				dest += vid.conrowbytes;
			}
			break;
		case 1:
			while (height--)
			{
				if (source[0])
					dest[0] = mainTransTable[(((unsigned int)dest[0])<<8) + source[0]];
				if (source[1])
					dest[1] = mainTransTable[(((unsigned int)dest[1])<<8) + source[1]];
				if (source[2])
					dest[2] = mainTransTable[(((unsigned int)dest[2])<<8) + source[2]];
				if (source[3])
					dest[3] = mainTransTable[(((unsigned int)dest[3])<<8) + source[3]];
				if (source[4])
					dest[4] = mainTransTable[(((unsigned int)dest[4])<<8) + source[4]];
				if (source[5])
					dest[5] = mainTransTable[(((unsigned int)dest[5])<<8) + source[5]];
				if (source[6])
					dest[6] = mainTransTable[(((unsigned int)dest[6])<<8) + source[6]];
				if (source[7])
					dest[7] = mainTransTable[(((unsigned int)dest[7])<<8) + source[7]];
				source += 128;
				dest += vid.conrowbytes;
			}
			break;
		case 2:
			while (height--)
			{
				if (source[0])
					dest[0] = mainTransTable[(((unsigned int)source[0])<<8) + dest[0]];
				if (source[1])
					dest[1] = mainTransTable[(((unsigned int)source[1])<<8) + dest[1]];
				if (source[2])
					dest[2] = mainTransTable[(((unsigned int)source[2])<<8) + dest[2]];
				if (source[3])
					dest[3] = mainTransTable[(((unsigned int)source[3])<<8) + dest[3]];
				if (source[4])
					dest[4] = mainTransTable[(((unsigned int)source[4])<<8) + dest[4]];
				if (source[5])
					dest[5] = mainTransTable[(((unsigned int)source[5])<<8) + dest[5]];
				if (source[6])
					dest[6] = mainTransTable[(((unsigned int)source[6])<<8) + dest[6]];
				if (source[7])
					dest[7] = mainTransTable[(((unsigned int)source[7])<<8) + dest[7]];
				source += 128;
				dest += vid.conrowbytes;
			}
			break;
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pre-expand to native format?
		unsigned short *dest = (unsigned short *)
				((byte *)vid.buffer + y*vid.rowbytes + (x<<1));
		// FIXME: transparency bits are missing
		while (height--)
		{
			if (source[0])
				dest[0] = d_8to16table[source[0]];
			if (source[1])
				dest[1] = d_8to16table[source[1]];
			if (source[2])
				dest[2] = d_8to16table[source[2]];
			if (source[3])
				dest[3] = d_8to16table[source[3]];
			if (source[4])
				dest[4] = d_8to16table[source[4]];
			if (source[5])
				dest[5] = d_8to16table[source[5]];
			if (source[6])
				dest[6] = d_8to16table[source[6]];
			if (source[7])
				dest[7] = d_8to16table[source[7]];
			source += 128;
			dest += vid.conrowbytes / 2;
		}
	}
}

//==========================================================================
//
// Draw_SmallString
//
//==========================================================================

void Draw_SmallString (int x, int y, const char *str)
{
	while (*str)
	{
		Draw_SmallCharacter(x, y, *str);
		str++;
		x += 6;
	}
}

//==========================================================================
//
// Draw_BigCharacter
//
// Callback for M_DrawBigCharacter() of menu.c
//
//==========================================================================
void Draw_BigCharacter (int x, int y, int num)
{
	qpic_t	*p;
	int	ypos, xpos;
	byte	*dest;
	byte	*source;

	p = Draw_CachePic ("gfx/menu/bigfont.lmp");
	source = p->data + ((num % 8) * 20) + (num / 8 * p->width * 20);

// FIXME: only for r_pixbytes == 1
	for (ypos = 0; ypos < 19; ypos++)
	{
		dest = vid.buffer + (y + ypos) * vid.rowbytes + x;
		for (xpos = 0; xpos < 19; xpos++, dest++, source++)
		{
			if (*source)
			{
				*dest = *source;
			}
		}
		source += (p->width - 19);
	}
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	byte		*source;
	int		v, u;

	if ((x < 0) || (x + pic->width > vid.width) ||
	    (y < 0) || (y + pic->height > vid.height))
	{
		Sys_Error ("%s: bad coordinates", __thisfunc__);
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		for (v = 0; v < pic->height; v++)
		{
			memcpy (dest, source, pic->width);
			dest += vid.rowbytes;
			source += pic->width;
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < pic->height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				dest[u] = d_8to16table[source[u]];
			}

			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}


//==========================================================================
//
// Draw_PicCropped
//
// Draws a qpic_t that is clipped at the bottom/top edges of the screen.
//
//==========================================================================

void Draw_PicCropped (int x, int y, qpic_t *pic)
{
	byte		*source;
	int		v, u, height;

	if ((x < 0) || (x+pic->width > (int)vid.width))
	{
		Sys_Error("%s: bad coordinates", __thisfunc__);
	}

	if (y >= vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if (y+pic->height > vid.height)
	{
		height = vid.height-y;
	}
	else if (y < 0)
	{
		height = pic->height+y;
	}
	else
	{
		height = pic->height;
	}

	source = pic->data;
	if (y < 0) 
	{
		source += (pic->width * (-y));
		y = 0;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y*vid.rowbytes + x;
		switch (trans_level)
		{
		case 0:
			for (v = 0; v < height; v++)
			{
				memcpy(dest, source, pic->width);
				dest += vid.rowbytes;
				source += pic->width;
			}
			break;
		case 1:
			for (v = 0; v < height; v++)
			{
				for (u = 0; u < pic->width; u++, source++)
				{
					dest[u] = mainTransTable[(((unsigned int)dest[u])<<8) + (*source)];
				}
				dest += vid.rowbytes;
			}
			break;
		case 2:
			for (v = 0; v < height; v++)
			{
				for (u = 0; u < pic->width; u++, source++)
				{
					dest[u] = mainTransTable[(((unsigned int)(*source))<<8) + dest[u]];
				}
				dest += vid.rowbytes;
			}
			break;
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				dest[u] = d_8to16table[source[u]];
			}
			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}

/*
=============
Draw_TransPic
=============
*/
void Draw_TransPic (int x, int y, qpic_t *pic)
{
	byte		*source, tbyte;
	int		v, u;

	if (x < 0 || (x + pic->width) > vid.width ||
	    y < 0 || (y + pic->height) > vid.height)
	{
		Sys_Error("%s: bad coordinates", __thisfunc__);
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		if (pic->width & 7)
		{	// general
			for (v = 0; v < pic->height; v++)
			{
				for (u = 0; u < pic->width; u++)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = tbyte;
				}

				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v = 0; v < pic->height; v++)
			{
				for (u = 0; u < pic->width; u += 8)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = tbyte;
					if ( (tbyte=source[u+1]) != TRANSPARENT_COLOR)
						dest[u+1] = tbyte;
					if ( (tbyte=source[u+2]) != TRANSPARENT_COLOR)
						dest[u+2] = tbyte;
					if ( (tbyte=source[u+3]) != TRANSPARENT_COLOR)
						dest[u+3] = tbyte;
					if ( (tbyte=source[u+4]) != TRANSPARENT_COLOR)
						dest[u+4] = tbyte;
					if ( (tbyte=source[u+5]) != TRANSPARENT_COLOR)
						dest[u+5] = tbyte;
					if ( (tbyte=source[u+6]) != TRANSPARENT_COLOR)
						dest[u+6] = tbyte;
					if ( (tbyte=source[u+7]) != TRANSPARENT_COLOR)
						dest[u+7] = tbyte;
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < pic->height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					dest[u] = d_8to16table[tbyte];
				}
			}

			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}


//==========================================================================
//
// Draw_SubPicCropped
//
// Draws a qpic_t that is clipped at the bottom/top edges of the screen.
//
//==========================================================================

void Draw_SubPicCropped (int x, int y, int h, qpic_t *pic)
{
	byte		*source;
	int		v, u, height;

	if ((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("%s: bad coordinates", __thisfunc__);
	}

	if (y >= vid.height || y+h < 0)
	{ // Totally off screen
		return;
	}

	if (y+pic->height > vid.height)
	{
		height = vid.height-y;
	}
	else if (y < 0)
	{
		height = pic->height+y;
	}
	else
	{
		height = pic->height;
	}

	if (height > h)
	{
		height = h;
	}

	source = pic->data;
	if (y < 0)
	{
		source += (pic->width * (-y));
		y = 0;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		switch (trans_level)
		{
		case 0:
			for (v = 0; v < height; v++)
			{
				memcpy(dest, source, pic->width);
				dest += vid.rowbytes;
				source += pic->width;
			}
			break;
		case 1:
			for (v = 0; v < height; v++)
			{
				for (u = 0; u < pic->width; u++, source++)
				{
					dest[u] = mainTransTable[(((unsigned int)dest[u])<<8) + (*source)];
				}
				dest += vid.rowbytes;
			}
			break;
		case 2:
			for (v = 0; v < height; v++)
			{
				for (u = 0; u < pic->width; u++, source++)
				{
					dest[u] = mainTransTable[(((unsigned int)(*source))<<8) + dest[u]];
				}
				dest += vid.rowbytes;
			}
			break;
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				dest[u] = d_8to16table[source[u]];
			}
			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}

//==========================================================================
//
// Draw_TransPicCropped
//
// Draws a holey qpic_t that is clipped at the bottom edge of the screen.
//
//==========================================================================

void Draw_TransPicCropped (int x, int y, qpic_t *pic)
{
	byte		*source, tbyte;
	int		v, u, height;

	if ((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("%s: bad coordinates", __thisfunc__);
	}

	if (y >= vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if (y+pic->height > vid.height)
	{
		height = vid.height-y;
	}
	else if (y < 0)
	{
		height = pic->height+y;
	}
	else
	{
		height = pic->height;
	}

	source = pic->data;
	if (y < 0)
	{
		source += (pic->width * (-y));
		y = 0;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		if (pic->width & 7)
		{ // General
			switch (trans_level)
			{
			case 0:
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u++)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
						{
							dest[u] = tbyte;
						}
					}
					dest += vid.rowbytes;
					source += pic->width;
				}
				break;
			case 1:
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u++)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
						{
							dest[u] = mainTransTable[(((unsigned int)dest[u])<<8) + tbyte];
						}
					}
					dest += vid.rowbytes;
					source += pic->width;
				}
				break;
			case 2:
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u++)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
						{
							dest[u] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u]];
						}
					}
					dest += vid.rowbytes;
					source += pic->width;
				}
				break;
			}
		}
		else
		{ // Unwound
			switch (trans_level)
			{
			case 0:
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u += 8)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
							dest[u] = tbyte;
						if ((tbyte = source[u+1]) != TRANSPARENT_COLOR)
							dest[u+1] = tbyte;
						if ((tbyte = source[u+2]) != TRANSPARENT_COLOR)
							dest[u+2] = tbyte;
						if ((tbyte = source[u+3]) != TRANSPARENT_COLOR)
							dest[u+3] = tbyte;
						if ((tbyte = source[u+4]) != TRANSPARENT_COLOR)
							dest[u+4] = tbyte;
						if ((tbyte = source[u+5]) != TRANSPARENT_COLOR)
							dest[u+5] = tbyte;
						if ((tbyte = source[u+6]) != TRANSPARENT_COLOR)
							dest[u+6] = tbyte;
						if ((tbyte = source[u+7]) != TRANSPARENT_COLOR)
							dest[u+7] = tbyte;
					}
					dest += vid.rowbytes;
					source += pic->width;
				}
				break;
			case 1:
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u += 8)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
							dest[u] = mainTransTable[(((unsigned int)dest[u])<<8) + tbyte];
						if ((tbyte = source[u+1]) != TRANSPARENT_COLOR)
							dest[u+1] = mainTransTable[(((unsigned int)dest[u+1])<<8) + tbyte];
						if ((tbyte = source[u+2]) != TRANSPARENT_COLOR)
							dest[u+2] = mainTransTable[(((unsigned int)dest[u+2])<<8) + tbyte];
						if ((tbyte = source[u+3]) != TRANSPARENT_COLOR)
							dest[u+3] = mainTransTable[(((unsigned int)dest[u+3])<<8) + tbyte];
						if ((tbyte = source[u+4]) != TRANSPARENT_COLOR)
							dest[u+4] = mainTransTable[(((unsigned int)dest[u+4])<<8) + tbyte];
						if ((tbyte = source[u+5]) != TRANSPARENT_COLOR)
							dest[u+5] = mainTransTable[(((unsigned int)dest[u+5])<<8) + tbyte];
						if ((tbyte = source[u+6]) != TRANSPARENT_COLOR)
							dest[u+6] = mainTransTable[(((unsigned int)dest[u+6])<<8) + tbyte];
						if ((tbyte = source[u+7]) != TRANSPARENT_COLOR)
							dest[u+7] = mainTransTable[(((unsigned int)dest[u+7])<<8) + tbyte];
					}
					dest += vid.rowbytes;
					source += pic->width;
				}
				break;
			case 2:
				for (v = 0; v < height; v++)
				{
					for (u = 0; u < pic->width; u += 8)
					{
						if ((tbyte = source[u]) != TRANSPARENT_COLOR)
							dest[u] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u]];
						if ((tbyte = source[u+1]) != TRANSPARENT_COLOR)
							dest[u+1] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+1]];
						if ((tbyte = source[u+2]) != TRANSPARENT_COLOR)
							dest[u+2] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+2]];
						if ((tbyte = source[u+3]) != TRANSPARENT_COLOR)
							dest[u+3] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+3]];
						if ((tbyte = source[u+4]) != TRANSPARENT_COLOR)
							dest[u+4] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+4]];
						if ((tbyte = source[u+5]) != TRANSPARENT_COLOR)
							dest[u+5] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+5]];
						if ((tbyte = source[u+6]) != TRANSPARENT_COLOR)
							dest[u+6] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+6]];
						if ((tbyte = source[u+7]) != TRANSPARENT_COLOR)
							dest[u+7] = mainTransTable[(((unsigned int)tbyte)<<8) + dest[u+7]];
					}
					dest += vid.rowbytes;
					source += pic->width;
				}
				break;
			}
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				tbyte = source[u];
				if (tbyte != TRANSPARENT_COLOR)
				{
					dest[u] = d_8to16table[tbyte];
				}
			}
			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}


/*
=============
Draw_SubPic
=============
*/
void Draw_SubPic (int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height)
{
	byte		*source;
	int		v, u;

	if ((x < 0) || (x + width > vid.width) ||
	    (y < 0) || (y + height > vid.height))
	{
		Sys_Error ("%s: bad coordinates", __thisfunc__);
	}

	source = pic->data + srcy * pic->width + srcx;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		for (v = 0; v < height; v++)
		{
			memcpy (dest, source, width);
			dest += vid.rowbytes;
			source += pic->width;
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		for (v = 0; v < height; v++)
		{
			for (u = srcx; u < (srcx+width); u++)
			{
				dest[u] = d_8to16table[source[u]];
			}

			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}

/*
=============
Draw_TransPicTranslate
=============
*/
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation, int p_class)
{
	byte		*source, tbyte;
	int		v, u;

	if (x < 0 || (x + pic->width) > vid.width ||
	    y < 0 || (y + pic->height) > vid.height)
	{
		Sys_Error ("%s: bad coordinates", __thisfunc__);
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y * vid.rowbytes + x;
		if (pic->width & 7)
		{	// general
			for (v = 0; v < pic->height; v++)
			{
				for (u = 0; u < pic->width; u++)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];
				}

				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v = 0; v < pic->height; v++)
			{
				for (u = 0; u < pic->width; u += 8)
				{
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];
					if ( (tbyte=source[u+1]) != TRANSPARENT_COLOR)
						dest[u+1] = translation[tbyte];
					if ( (tbyte=source[u+2]) != TRANSPARENT_COLOR)
						dest[u+2] = translation[tbyte];
					if ( (tbyte=source[u+3]) != TRANSPARENT_COLOR)
						dest[u+3] = translation[tbyte];
					if ( (tbyte=source[u+4]) != TRANSPARENT_COLOR)
						dest[u+4] = translation[tbyte];
					if ( (tbyte=source[u+5]) != TRANSPARENT_COLOR)
						dest[u+5] = translation[tbyte];
					if ( (tbyte=source[u+6]) != TRANSPARENT_COLOR)
						dest[u+6] = translation[tbyte];
					if ( (tbyte=source[u+7]) != TRANSPARENT_COLOR)
						dest[u+7] = translation[tbyte];
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else /* r_pixbytes == 2 */
	{
		// FIXME: pretranslate at load time?
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		// FIXME: transparency bits are missing
		for (v = 0; v < pic->height; v++)
		{
			for (u = 0; u < pic->width; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					dest[u] = d_8to16table[tbyte];
				}
			}

			dest += vid.rowbytes / 2;
			source += pic->width;
		}
	}
}

/*
================
Draw_ConsoleBackground

================
*/
static void Draw_ConsoleVersionInfo (int lines)
{
	static const char ver[] = ENGINE_WATERMARK;
	const char *ptr = ver;
	int x = vid.conwidth - (strlen(ver) * 8 + 11);
	int y = lines - 14;
	for (; *ptr; ++ptr)
		Draw_Character (x + (int)(ptr - ver) * 8, y, *ptr | 0x100);
}

void Draw_ConsoleBackground (int lines)
{
	int		x, y, v;
	byte		*src;
	int		f, fstep;
	qpic_t		*conback;

	conback = Draw_CachePic ("gfx/menu/conback.lmp");

	/* Since the status bar or deahmatch overlay will never be drawn
	 * at the same as the console background we can freely play with
	 * trans_level here. */
	if (!con_forcedup)
	{
		trans_level = scr_contrans.integer;
		if (trans_level < 0 || trans_level > 2)
			trans_level = 0;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.conbuffer;
		fstep = conback->width * 0x10000 / vid.conwidth;

		for (y = 0; y < lines; y++, dest += vid.conrowbytes)
		{
			v = (vid.conheight - lines + y)* conback->height / vid.conheight;
			src = conback->data + v * conback->width;
			if (vid.conwidth == conback->width && trans_level == 0)
			{
				memcpy (dest, src, vid.conwidth);
				continue;
			}
			f = 0;
			switch (trans_level)
			{
			case 0:
				for (x = 0; x < (int)vid.conwidth; x += 4)
				{
					dest[x] = src[f>>16];
					f += fstep;
					dest[x+1] = src[f>>16];
					f += fstep;
					dest[x+2] = src[f>>16];
					f += fstep;
					dest[x+3] = src[f>>16];
					f += fstep;
				}
				break;
			case 1:
				for (x = 0; x < (int)vid.conwidth; x += 4)
				{
					dest[x] = mainTransTable[(((unsigned int)dest[x])<<8) + src[f>>16]];
					f += fstep;
					dest[x+1] = mainTransTable[(((unsigned int)dest[x+1])<<8) + src[f>>16]];
					f += fstep;
					dest[x+2] = mainTransTable[(((unsigned int)dest[x+2])<<8) + src[f>>16]];
					f += fstep;
					dest[x+3] = mainTransTable[(((unsigned int)dest[x+3])<<8) + src[f>>16]];
					f += fstep;
				}
				break;
			case 2:
				for (x = 0; x < (int)vid.conwidth; x += 4)
				{
					dest[x] = mainTransTable[(((unsigned int)src[f>>16])<<8) + dest[x]];
					f += fstep;
					dest[x+1] = mainTransTable[(((unsigned int)src[f>>16])<<8) + dest[x+1]];
					f += fstep;
					dest[x+2] = mainTransTable[(((unsigned int)src[f>>16])<<8) + dest[x+2]];
					f += fstep;
					dest[x+3] = mainTransTable[(((unsigned int)src[f>>16])<<8) + dest[x+3]];
					f += fstep;
				}
				break;
			}
		}
	}
	else /* r_pixbytes == 2 */
	{
		unsigned short *dest = (unsigned short *)vid.conbuffer;
		fstep = conback->width * 0x10000 / vid.conwidth;

		for (y = 0; y < lines; y++, dest += (vid.conrowbytes / 2))
		{
		// FIXME: pre-expand to native format?
		// FIXME: does the endian switching go away in production?
			v = (vid.conheight - lines + y) * conback->height / vid.conheight;
			src = conback->data + v * conback->width;
			f = 0;
		// FIXME: transparency bits are missing
			for (x = 0; x < (int)vid.conwidth; x += 4)
			{
				dest[x] = d_8to16table[src[f>>16]];
				f += fstep;
				dest[x+1] = d_8to16table[src[f>>16]];
				f += fstep;
				dest[x+2] = d_8to16table[src[f>>16]];
				f += fstep;
				dest[x+3] = d_8to16table[src[f>>16]];
				f += fstep;
			}
		}
	}

	trans_level = 0;

#if defined(H2W)
	if (cls.download)
		return;
#endif

	Draw_ConsoleVersionInfo (lines);
}


/*
==============
R_DrawRect8
==============
*/
void R_DrawRect8 (vrect_t *prect, int rowbytes, byte *psrc,
	int transparent)
{
	byte	t;
	int		i, j, srcdelta, destdelta;
	byte	*pdest;

	pdest = vid.buffer + (prect->y * vid.rowbytes) + prect->x;

	srcdelta = rowbytes - prect->width;
	destdelta = vid.rowbytes - prect->width;

	if (transparent)
	{
		for (i = 0; i < prect->height; i++)
		{
			for (j = 0; j < prect->width; j++)
			{
				t = *psrc;
				if (t != TRANSPARENT_COLOR)
				{
					*pdest = t;
				}

				psrc++;
				pdest++;
			}

			psrc += srcdelta;
			pdest += destdelta;
		}
	}
	else
	{
		for (i = 0; i < prect->height; i++)
		{
			memcpy (pdest, psrc, prect->width);
			psrc += rowbytes;
			pdest += vid.rowbytes;
		}
	}
}


/*
==============
R_DrawRect16
==============
*/
void R_DrawRect16 (vrect_t *prect, int rowbytes, byte *psrc,
	int transparent)
{
	byte			t;
	int				i, j, srcdelta, destdelta;
	unsigned short	*pdest;

// FIXME: would it be better to pre-expand native-format versions?

	pdest = (unsigned short *)vid.buffer +
			(prect->y * (vid.rowbytes / 2)) + prect->x;

	srcdelta = rowbytes - prect->width;
	destdelta = (vid.rowbytes / 2) - prect->width;

	if (transparent)
	{
		for (i = 0; i < prect->height; i++)
		{
			for (j = 0; j < prect->width; j++)
			{
				t = *psrc;
				if (t != TRANSPARENT_COLOR)
				{
					*pdest = d_8to16table[t];
				}

				psrc++;
				pdest++;
			}

			psrc += srcdelta;
			pdest += destdelta;
		}
	}
	else
	{
		for (i = 0; i < prect->height; i++)
		{
			for (j = 0; j < prect->width; j++)
			{
				*pdest = d_8to16table[*psrc];
				psrc++;
				pdest++;
			}

			psrc += srcdelta;
			pdest += destdelta;
		}
	}
}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h)
{
	int	width, height, tileoffsetx, tileoffsety;
	byte	*psrc;
	vrect_t	vr;

	r_rectdesc.rect.x = x;
	r_rectdesc.rect.y = y;
	r_rectdesc.rect.width = w;
	r_rectdesc.rect.height = h;

	vr.y = r_rectdesc.rect.y;
	height = r_rectdesc.rect.height;

	tileoffsety = vr.y % r_rectdesc.height;

	while (height > 0)
	{
		vr.x = r_rectdesc.rect.x;
		width = r_rectdesc.rect.width;

		if (tileoffsety != 0)
			vr.height = r_rectdesc.height - tileoffsety;
		else
			vr.height = r_rectdesc.height;

		if (vr.height > height)
			vr.height = height;

		tileoffsetx = vr.x % r_rectdesc.width;

		while (width > 0)
		{
			if (tileoffsetx != 0)
				vr.width = r_rectdesc.width - tileoffsetx;
			else
				vr.width = r_rectdesc.width;

			if (vr.width > width)
				vr.width = width;

			psrc = r_rectdesc.ptexbytes +
					(tileoffsety * r_rectdesc.rowbytes) + tileoffsetx;

			if (r_pixbytes == 1)
			{
				R_DrawRect8 (&vr, r_rectdesc.rowbytes, psrc, 0);
			}
			else
			{
				R_DrawRect16 (&vr, r_rectdesc.rowbytes, psrc, 0);
			}

			vr.x += vr.width;
			width -= vr.width;
			tileoffsetx = 0;	// only the left tile can be left-clipped
		}

		vr.y += vr.height;
		height -= vr.height;
		tileoffsety = 0;		// only the top tile can be top-clipped
	}
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	int		u, v;

	if (x < 0 || x + w > vid.width ||
	    y < 0 || y + h > vid.height)
	{
		Con_Printf("Bad Draw_Fill(%d, %d, %d, %d, %c)\n", x, y, w, h, c);
		return;
	}

	if (r_pixbytes == 1)
	{
		byte *dest = vid.buffer + y*vid.rowbytes + x;
		switch (trans_level)
		{
		case 0:
			for (v = 0; v < h; v++, dest += vid.rowbytes)
			{
				for (u = 0; u < w; u++)
				{
					dest[u] = c;
				}
			}
			break;
		case 1:
			for (v = 0; v < h; v++, dest += vid.rowbytes)
			{
				for (u = 0; u < w; u++)
				{
					dest[u] = mainTransTable[(((unsigned int)dest[u])<<8) + c];
				}
			}
			break;
		case 2:
			for (v = 0; v < h; v++, dest += vid.rowbytes)
			{
				for (u = 0; u < w; u++)
				{
					dest[u] = mainTransTable[(c<<8) + dest[u]];
				}
			}
			break;
		}
	}
	else /* r_pixbytes == 2 */
	{
		unsigned short *dest = (unsigned short *)vid.buffer + y * (vid.rowbytes / 2) + x;
		unsigned int uc = d_8to16table[c];
		// FIXME: transparency bits are missing
		for (v = 0; v < h; v++, dest += (vid.rowbytes / 2))
		{
			for (u = 0; u < w; u++)
				dest[u] = uc;
		}
	}
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	int			x, y;
	byte		*pbuf;
	int temp[2048], *pos;

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for (x = 0; x < 2048; x++)
		temp[x] = (164 + rand() % 6) * 256;

	for (y = 0; y < vid.height; y++)
	{
		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		pos = &temp[rand() % 256];

		if ((y & 127) == 127)
		{
			VID_UnlockBuffer ();
			S_ExtraUpdate ();
			VID_LockBuffer ();
		}

		for (x = 0; x < vid.width; x++, pbuf++)
		{
//			if ((x & 3) != t)
//				pbuf[x] = 0;
			*pbuf = mainTransTable[(*pos)+(*pbuf)];
			pos++;
//			pbuf[x] = mainTransTable[((170+(rand() % 6))*256)+pbuf[x]];
//			pbuf[x] = mainTransTable[159 + (256*pbuf[x])];
		}
	}

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();
}

//=============================================================================

#if defined(DRAW_LOADINGSKULL)
/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{
	static int disc_idx = 0;

#ifndef H2W
	if (loading_stage)
		return;
#endif
	if (!draw_disc[disc_idx])
		return;

	if (++disc_idx >= MAX_DISC)
		disc_idx = 0;

	D_BeginDirectRect (vid.width - 28, 0, draw_disc[disc_idx]->data, 28, 24);
	scr_topupdate = 0;	// this was disabled by rjr in the hw source for what reason?
}


/*
================
Draw_EndDisc

Erases the disc icon.
Call after completing any disc IO
================
*/
void Draw_EndDisc (void)
{
/* this is causing problems, at least with mgl-win32.
   the sdl driver seems to work fine either way. the
   problem should be some kind of a bad interaction
   with D_ShowLoadingSize(), due to clashes in direct
   access usage I think: didn't look at it carefully,
   yet. */
#if 0
	if (!draw_disc[0])
		return;

	D_EndDirectRect (vid.width - 28, 0, 28, 24);
	scr_topupdate = 0;	// this was disabled by rjr in the hw source for what reason?
#endif
}
#endif	/* DRAW_LOADINGSKULL */

