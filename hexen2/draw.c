
//**************************************************************************
//**
//** draw.c
//**
//** $Header: /home/ozzie/Download/0000/uhexen2/hexen2/draw.c,v 1.1.1.1 2004-11-28 00:02:40 sezero Exp $
//**
//** This is the only file outside the refresh that touches the vid buffer.
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "quakedef.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

typedef struct 
{
	vrect_t	rect;
	int		width;
	int		height;
	byte	*ptexbytes;
	int		rowbytes;
} rectdesc_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static rectdesc_t	r_rectdesc;

byte *draw_smallchars; // Small characters for status bar

byte		*draw_chars;				// 8*8 graphic characters
qpic_t		*draw_disc[MAX_DISC] = 
{
	NULL  // make the first one null for sure
};
qpic_t		*draw_backtile;

//=============================================================================
/* Support Routines */

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	cache_user_t	cache;
} cachepic_t;

//#define	MAX_CACHED_PICS		128
#define	MAX_CACHED_PICS		256
cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;


// CODE --------------------------------------------------------------------

qpic_t	*Draw_PicFromWad (char *name)
{
	return W_GetLumpName (name);
}

/*
================
Draw_CachePic
================
*/
qpic_t	*Draw_CachePic (char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	
	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			break;

	if (i == menu_numcachepics)
	{
		if (menu_numcachepics == MAX_CACHED_PICS)
			Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
		menu_numcachepics++;
		strcpy (pic->name, path);
	}

	dat = Cache_Check (&pic->cache);

	if (dat)
		return dat;

//
// load the pic from disk
//
	COM_LoadCacheFile (path, &pic->cache);
	
	dat = (qpic_t *)pic->cache.data;
	if (!dat)
	{
		Sys_Error ("Draw_CachePic: failed to load %s", path);
	}

	SwapPic (dat);

	return dat;
}



//==========================================================================
//
// Draw_Init
//
//==========================================================================
void Draw_Init (void)
{
	int		i;
	char temp[MAX_QPATH];

	draw_smallchars = W_GetLumpName("tinyfont");
//	draw_chars = W_GetLumpName ("conchars");
	draw_chars = COM_LoadHunkFile ("gfx/menu/conchars.lmp");

	// Do this backwards so we don't try and draw the 
	// skull as we are loading
	for(i=MAX_DISC-1;i>=0;i--)
	{
		sprintf(temp,"gfx/menu/skull%d.lmp",i);
		draw_disc[i] = (qpic_t *)COM_LoadHunkFile (temp);
	}

//	draw_disc = W_GetLumpName ("disc");
//	draw_backtile = W_GetLumpName ("backtile");
	draw_backtile = (qpic_t	*)COM_LoadHunkFile ("gfx/menu/backtile.lmp");

	r_rectdesc.width = draw_backtile->width;
	r_rectdesc.height = draw_backtile->height;
	r_rectdesc.ptexbytes = draw_backtile->data;
	r_rectdesc.rowbytes = draw_backtile->width;
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
	byte			*dest;
	byte			*source;
	unsigned short	*pusdest;
	int				drawline;	
	int				row, col;

	num &= 511;
	
	if (y <= -8)
		return;			// totally off screen

#ifdef PARANOID
	if (y > vid.height - 8 || x < 0 || x > vid.width - 8)
		Sys_Error ("Con_DrawCharacter: (%i, %i)", x, y);
	if (num < 0 || num > 511)
		Sys_Error ("Con_DrawCharacter: char %i", num);
#endif

	row = num>>5;
	col = num&31;
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
		dest = vid.conbuffer + y*vid.conrowbytes + x;
	
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
	}
	else
	{
	// FIXME: pre-expand to native format?
		pusdest = (unsigned short *)
				((byte *)vid.conbuffer + y*vid.conrowbytes + (x<<1));

		while (drawline--)
		{
			if (source[0])
				pusdest[0] = d_8to16table[source[0]];
			if (source[1])
				pusdest[1] = d_8to16table[source[1]];
			if (source[2])
				pusdest[2] = d_8to16table[source[2]];
			if (source[3])
				pusdest[3] = d_8to16table[source[3]];
			if (source[4])
				pusdest[4] = d_8to16table[source[4]];
			if (source[5])
				pusdest[5] = d_8to16table[source[5]];
			if (source[6])
				pusdest[6] = d_8to16table[source[6]];
			if (source[7])
				pusdest[7] = d_8to16table[source[7]];

			source += 256;
			pusdest += (vid.conrowbytes >> 1);
		}
	}
}

/*
================
Draw_String
================
*/
void Draw_String (int x, int y, char *str)
{
	while (*str)
	{
		Draw_Character (x, y, *str);
		str++;
		x += 8;
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

void Draw_SmallCharacter(int x, int y, int num)
{
	byte *dest;
	byte *source;
	unsigned short *pusdest;
	int height;
	int row, col;

	if(num < 32)
	{
		num = 0;
	}
	else if(num >= 'a' && num <= 'z')
	{
		num -= 64;
	}
	else if(num > '_')
	{
		num = 0;
	}
	else
	{
		num -= 32;
	}

	if(y >= vid.height)
	{ // Totally off screen
		return;
	}

#ifdef PARANOID
	if((y < 0) || (x < 0) || (x+8 > vid.width))
	{
		Sys_Error("Bad Draw_SmallCharacter: (%d, %d)", x, y);
	}
#endif

	if(y+5 > vid.height)
	{
		height = vid.height-y;
	}
	else
	{
		height = 5;
	}

	row = num>>4;
	col = num&15;
	source = draw_smallchars+(row<<10)+(col<<3);

	if(r_pixbytes == 1)
	{
		dest = vid.buffer+y*vid.rowbytes+x;
		while(height--)
		{
			if(source[0])
				dest[0] = source[0];
			if(source[1])
				dest[1] = source[1];
			if(source[2])
				dest[2] = source[2];
			if(source[3])
				dest[3] = source[3];
			if(source[4])
				dest[4] = source[4];
			if(source[5])
				dest[5] = source[5];
			if(source[6])
				dest[6] = source[6];
			if(source[7])
				dest[7] = source[7];
			source += 128;
			dest += vid.conrowbytes;
		}
	}
	else
	{ // FIXME: pre-expand to native format?
		pusdest = (unsigned short *)
			((byte *)vid.buffer+y*vid.rowbytes+(x<<1));
		while(height--)
		{
			if(source[0])
				pusdest[0] = d_8to16table[source[0]];
			if(source[1])
				pusdest[1] = d_8to16table[source[1]];
			if(source[2])
				pusdest[2] = d_8to16table[source[2]];
			if(source[3])
				pusdest[3] = d_8to16table[source[3]];
			if(source[4])
				pusdest[4] = d_8to16table[source[4]];
			if(source[5])
				pusdest[5] = d_8to16table[source[5]];
			if(source[6])
				pusdest[6] = d_8to16table[source[6]];
			if(source[7])
				pusdest[7] = d_8to16table[source[7]];
			source += 128;
			pusdest += (vid.conrowbytes>>1);
		}
	}
}

//==========================================================================
//
// Draw_SmallString
//
//==========================================================================

void Draw_SmallString(int x, int y, char *str)
{
	while(*str)
	{
		Draw_SmallCharacter(x, y, *str);
		str++;
		x += 6;
	}
}

/*
================
Draw_DebugChar

Draws a single character directly to the upper right corner of the screen.
This is for debugging lockups by drawing different chars in different parts
of the code.
================
*/
void Draw_DebugChar (char num)
{
	byte			*dest;
	byte			*source;
	int				drawline;	
	extern byte		*draw_chars;
	int				row, col;

	if (!vid.direct)
		return;		// don't have direct FB access, so no debugchars...

	drawline = 8;

	row = num>>5;
	col = num&31;
	source = draw_chars + (row<<11) + (col<<3);

	dest = vid.direct + 312;

	while (drawline--)
	{
		dest[0] = source[0];
		dest[1] = source[1];
		dest[2] = source[2];
		dest[3] = source[3];
		dest[4] = source[4];
		dest[5] = source[5];
		dest[6] = source[6];
		dest[7] = source[7];
		source += 256;
		dest += 320;
	}
}

/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	byte			*dest, *source;
	unsigned short	*pusdest;
	int				v, u;

	if ((x < 0) ||
		(x + pic->width > vid.width) ||
		(y < 0) ||
		(y + pic->height > vid.height))
	{
		Sys_Error ("Draw_Pic: bad coordinates");
	}

	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		for (v=0 ; v<pic->height ; v++)
		{
			memcpy (dest, source, pic->width);
			dest += vid.rowbytes;
			source += pic->width;
		}
	}
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				pusdest[u] = d_8to16table[source[u]];
			}

			pusdest += vid.rowbytes >> 1;
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

void Draw_PicCropped(int x, int y, qpic_t *pic)
{
	byte *dest, *source;
	unsigned short *pusdest;
	int v, u;
	int height;

	if((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("Draw_PicCropped: bad coordinates");
	}

	if (y >= (int)vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if(y+pic->height > vid.height)
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

	if(r_pixbytes == 1)
	{
		dest = vid.buffer+y*vid.rowbytes+x;
		for(v = 0; v < height; v++)
		{
			memcpy(dest, source, pic->width);
			dest += vid.rowbytes;
			source += pic->width;
		}
	}
	else
	{ // FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer+y*(vid.rowbytes>>1)+x;
		for(v = 0; v < height; v++)
		{
			for(u = 0; u < pic->width; u++)
			{
				pusdest[u] = d_8to16table[source[u]];
			}
			pusdest += vid.rowbytes>>1;
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
	byte	*dest, *source, tbyte;
	unsigned short	*pusdest;
	int				v, u;

	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error("Draw_TransPic: bad coordinates y=%d, pic_height=%d, vheight=%d\n",y,pic->height,vid.height);
	}
		
	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		if (pic->width & 7)
		{	// general
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u++)
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = tbyte;
	
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u+=8)
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
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					pusdest[u] = d_8to16table[tbyte];
				}
			}

			pusdest += vid.rowbytes >> 1;
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

void Draw_TransPicCropped(int x, int y, qpic_t *pic)
{
	byte *dest, *source, tbyte;
	unsigned short *pusdest;
	int v, u;
	int height;

	if((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("Draw_TransPicCropped: bad coordinates");
	}

	if (y >= (int)vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if(y+pic->height > vid.height)
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

	if(r_pixbytes == 1)
	{
		dest = vid.buffer+y*vid.rowbytes+x;
		if(pic->width&7)
		{ // General
			for(v = 0; v < height; v++)
			{
				for(u = 0; u < pic->width; u++)
				{
					if((tbyte = source[u]) != TRANSPARENT_COLOR)
					{
						dest[u] = tbyte;
					}
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{ // Unwound
			for(v = 0; v < height; v++)
			{
				for(u = 0; u < pic->width; u += 8)
				{
					if((tbyte = source[u]) != TRANSPARENT_COLOR)
						dest[u] = tbyte;
					if((tbyte = source[u+1]) != TRANSPARENT_COLOR)
						dest[u+1] = tbyte;
					if((tbyte = source[u+2]) != TRANSPARENT_COLOR)
						dest[u+2] = tbyte;
					if((tbyte = source[u+3]) != TRANSPARENT_COLOR)
						dest[u+3] = tbyte;
					if((tbyte = source[u+4]) != TRANSPARENT_COLOR)
						dest[u+4] = tbyte;
					if((tbyte = source[u+5]) != TRANSPARENT_COLOR)
						dest[u+5] = tbyte;
					if((tbyte = source[u+6]) != TRANSPARENT_COLOR)
						dest[u+6] = tbyte;
					if((tbyte = source[u+7]) != TRANSPARENT_COLOR)
						dest[u+7] = tbyte;
				}
				dest += vid.rowbytes;
				source += pic->width;
			}
		}
	}
	else
	{ // FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer+y*(vid.rowbytes>>1)+x;
		for(v = 0; v < height; v++)
		{
			for(u = 0; u < pic->width; u++)
			{
				tbyte = source[u];
				if(tbyte != TRANSPARENT_COLOR)
				{
					pusdest[u] = d_8to16table[tbyte];
				}
			}
			pusdest += vid.rowbytes>>1;
			source += pic->width;
		}
	}
}

/*
=============
Draw_TransPicTranslate
=============
*/
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation)
{
	byte	*dest, *source, tbyte;
	unsigned short	*pusdest;
	int				v, u;

	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error ("Draw_TransPic: bad coordinates");
	}
		
	source = pic->data;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y * vid.rowbytes + x;

		if (pic->width & 7)
		{	// general
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u++)
					if ( (tbyte=source[u]) != TRANSPARENT_COLOR)
						dest[u] = translation[tbyte];

				dest += vid.rowbytes;
				source += pic->width;
			}
		}
		else
		{	// unwound
			for (v=0 ; v<pic->height ; v++)
			{
				for (u=0 ; u<pic->width ; u+=8)
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
	else
	{
	// FIXME: pretranslate at load time?
		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;

		for (v=0 ; v<pic->height ; v++)
		{
			for (u=0 ; u<pic->width ; u++)
			{
				tbyte = source[u];

				if (tbyte != TRANSPARENT_COLOR)
				{
					pusdest[u] = d_8to16table[tbyte];
				}
			}

			pusdest += vid.rowbytes >> 1;
			source += pic->width;
		}
	}
}


void Draw_CharToConback (int num, byte *dest)
{
	int		row, col;
	byte	*source;
	int		drawline;
	int		x;

	row = num>>5;
	col = num&31;
	source = draw_chars + (row<<11) + (col<<3);

	drawline = 8;

	while (drawline--)
	{
		for (x=0 ; x<8 ; x++)
			if (source[x])
				dest[x] = 0x60 + source[x];
		source += 256;
		dest += 320;
	}

}

/*
================
Draw_ConsoleBackground

================
*/
void Draw_ConsoleBackground (int lines)
{
	int				x, y, v;
	byte			*src, *dest;
	unsigned short	*pusdest;
	int				f, fstep;
	qpic_t			*conback;
	char			ver[100];

	conback = Draw_CachePic ("gfx/menu/conback.lmp");

// hack the version number directly into the pic
	dest = conback->data + 320 - 43 + 320*186;
	sprintf (ver, "%4.2f", HEXEN2_VERSION);

	for (x=0 ; x<strlen(ver) ; x++)
		Draw_CharToConback (ver[x], dest+(x<<3));

// draw the pic
	if (r_pixbytes == 1)
	{
		dest = vid.conbuffer;

		for (y=0 ; y<lines ; y++, dest += vid.conrowbytes)
		{
			v = (vid.conheight - lines + y)*200/vid.conheight;
			src = conback->data + v*320;
			if (vid.conwidth == 320)
				memcpy (dest, src, vid.conwidth);
			else
			{
				f = 0;
				fstep = 320*0x10000/vid.conwidth;
				for (x=0 ; x<vid.conwidth ; x+=4)
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
			}
		}
	}
	else
	{
		pusdest = (unsigned short *)vid.conbuffer;

		for (y=0 ; y<lines ; y++, pusdest += (vid.conrowbytes >> 1))
		{
		// FIXME: pre-expand to native format?
		// FIXME: does the endian switching go away in production?
			v = (vid.conheight - lines + y)*200/vid.conheight;
			src = conback->data + v*320;
			f = 0;
			fstep = 320*0x10000/vid.conwidth;
			for (x=0 ; x<vid.conwidth ; x+=4)
			{
				pusdest[x] = d_8to16table[src[f>>16]];
				f += fstep;
				pusdest[x+1] = d_8to16table[src[f>>16]];
				f += fstep;
				pusdest[x+2] = d_8to16table[src[f>>16]];
				f += fstep;
				pusdest[x+3] = d_8to16table[src[f>>16]];
				f += fstep;
			}
		}
	}
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
		for (i=0 ; i<prect->height ; i++)
		{
			for (j=0 ; j<prect->width ; j++)
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
		for (i=0 ; i<prect->height ; i++)
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
			(prect->y * (vid.rowbytes >> 1)) + prect->x;

	srcdelta = rowbytes - prect->width;
	destdelta = (vid.rowbytes >> 1) - prect->width;

	if (transparent)
	{
		for (i=0 ; i<prect->height ; i++)
		{
			for (j=0 ; j<prect->width ; j++)
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
		for (i=0 ; i<prect->height ; i++)
		{
			for (j=0 ; j<prect->width ; j++)
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
	int				width, height, tileoffsetx, tileoffsety;
	byte			*psrc;
	vrect_t			vr;

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
	byte			*dest;
	unsigned short	*pusdest;
	unsigned		uc;
	int				u, v;

	if (r_pixbytes == 1)
	{
		dest = vid.buffer + y*vid.rowbytes + x;
		for (v=0 ; v<h ; v++, dest += vid.rowbytes)
			for (u=0 ; u<w ; u++)
				dest[u] = c;
	}
	else
	{
		uc = d_8to16table[c];

		pusdest = (unsigned short *)vid.buffer + y * (vid.rowbytes >> 1) + x;
		for (v=0 ; v<h ; v++, pusdest += (vid.rowbytes >> 1))
			for (u=0 ; u<w ; u++)
				pusdest[u] = uc;
	}
}
//=============================================================================

extern byte *mainTransTable; // in r_main.c

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	int			x,y,i;
	byte		*pbuf;
	int temp[2048], *pos;

	VID_UnlockBuffer ();
	S_ExtraUpdate ();
	VID_LockBuffer ();

	for(x=0;x<2048;x++)
		temp[x] = (164 + rand() % 6) * 256;
	i = 0;

	for (y=0 ; y<vid.height ; y++)
	{
		pbuf = (byte *)(vid.buffer + vid.rowbytes*y);
		pos = &temp[rand() % 256];

		if ((y & 127) == 127)
		{
			VID_UnlockBuffer ();
			S_ExtraUpdate ();
			VID_LockBuffer ();
		}

		for (x=0 ; x<vid.width ; x++,pbuf++)
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

/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{
	static int index = 0;

	if (!draw_disc[index] || loading_stage) return;

	index++;
	if (index >= MAX_DISC) index = 0;

	D_BeginDirectRect (vid.width - 28, 0, draw_disc[index]->data, 28, 24);
	scr_topupdate = 0;
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
	if (!draw_disc[0]) return;

	return;

	D_EndDirectRect (vid.width - 28, 0, 28, 24);
	scr_topupdate = 0;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/11/09 17:03:56  theoddone33
 * Inital import
 *
 * 
 * 3     3/11/98 6:20p Mgummelt
 * 
 * 2     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 19    8/27/97 12:11p Rjohnson
 * Optimized menu effect
 * 
 * 18    8/24/97 11:13a Rjohnson
 * Removed loading skulls 
 * 
 * 17    8/20/97 2:05p Rjohnson
 * fix for internationalization
 * 
 * 16    8/20/97 11:09a Rjohnson
 * Fix for console font
 * 
 * 14    8/15/97 11:27a Rlove
 * Changed MAX_CACHED_PICS to 256
 * 
 * 13    6/19/97 6:30p Rjohnson
 * Skull isn't constantly erased now
 * 
 * 12    6/15/97 7:44p Rjohnson
 * Added new pause and loading graphics
 * 
 * 11    6/06/97 5:17p Rjohnson
 * New console characters
 * 
 * 10    6/06/97 5:13p Rjohnson
 * New console font
 * 
 * 9     4/22/97 5:19p Rjohnson
 * More menu updates
 * 
 * 8     4/15/97 5:58p Rjohnson
 * Update to how the screen fades with the menu up
 * 
 * 7     4/14/97 5:02p Rjohnson
 * Make the cropped functions work from the top as well
 * 
 * 6     4/02/97 11:12a Bgokey
 * 
 * 5     3/07/97 12:35p Rjohnson
 * Id Updates
 * 
 * 4     2/17/97 3:47p Rjohnson
 * Id Updates
 */
