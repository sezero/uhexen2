/*
	gl_draw.c
	this is the only file outside the refresh that touches the vid buffer

	$Id: gl_draw.c,v 1.53 2006-01-15 22:07:50 sezero Exp $
*/

#include "quakedef.h"

qboolean draw_reinit = false;

extern int ColorIndex[16];
extern unsigned ColorPercent[16];

#ifndef GL_COLOR_INDEX8_EXT
#define GL_COLOR_INDEX8_EXT	0x80E5
#endif

extern qboolean	is8bit;
#ifdef	OLD_8_BIT_PALETTE_CODE
extern unsigned char inverse_pal[(1<<INVERSE_PAL_TOTAL_BITS)+1];
#else
extern unsigned char d_15to8table[65536];
#endif

extern vrect_t	scr_vrect;
extern cvar_t	crosshair, cl_crossx, cl_crossy, crosshaircolor;

extern int	gl_max_size;
cvar_t		gl_picmip = {"gl_picmip", "0"};
cvar_t		gl_spritemip = {"gl_spritemip", "0"};

qboolean	plyrtex[MAX_PLAYER_CLASS][16][16];	// whether or not the corresponding player textures
							// (in multiplayer config screens) have been loaded
byte		*draw_chars;				// 8*8 graphic characters
byte		*draw_smallchars;			// Small characters for status bar
byte		*draw_menufont; 			// Big Menu Font
qpic_t		*draw_backtile;

int			char_texture;
int			cs_texture; // crosshair texture
int			char_smalltexture;
int			char_menufonttexture;

int	trans_level = 0;

// Crosshair texture is a 32x32 alpha map with 8 levels of alpha.
// The format is similar to an X11 pixmap, but not the same.
// 7 is 100% solid, 0 and any other characters are transparent.
static char *cs_data = {
/* This is actually the QuakeWorld crosshair
   which Raven didn't bother changing. It is
   possible to make class-based crosshairs. */
	"................................"
	"................................"
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"................................"
	"................................"
	"................................"
	"................................"
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"..7777....7777....7777....7777.."
	"..7777....7777....7777....7777.."
	"..7777....7777....7777....7777.."
	"..7777....7777....7777....7777.."
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"................................"
	"................................"
	"................................"
	"................................"
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"..............7777.............."
	"................................"
	"................................"
};

byte		conback_buffer[sizeof(qpic_t) + sizeof(glpic_t)];
qpic_t		*conback = (qpic_t *)&conback_buffer;

int		gl_solid_format = 3;
int		gl_alpha_format = 4;

int		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gl_filter_max = GL_LINEAR;

int		texels;

extern qboolean	is_3dfx;

gltexture_t	gltextures[MAX_GLTEXTURES];
int			numgltextures;

static int GL_LoadPixmap(char *name, char *data);


void GL_Texels_f (void)
{
	Con_Printf ("Current uploaded texels: %i\n", texels);
}

/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up stupid hardware / drivers

=============================================================================
*/

// disabled scrap allocation. doesn't work good with vid_mode changes
#define ENABLE_SCRAP	0

#if ENABLE_SCRAP

#define	MAX_SCRAPS		1
#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT		256

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT*4];
qboolean	scrap_dirty;
int			scrap_texnum;

// returns a texture number and the position inside it
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	return -1;
//	Sys_Error ("Scrap_AllocBlock: full");
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	scrap_uploads++;
	GL_Bind(scrap_texnum);
	GL_Upload8 (scrap_texels[0], BLOCK_WIDTH, BLOCK_HEIGHT, false, true, 0);
	scrap_dirty = false;
}
#endif	// ENABLE_SCRAP

//=============================================================================
/* Support Routines */

#define	MAX_CACHED_PICS		256
cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;

int		pic_texels;
int		pic_count;

/*
 * Geometry for the player/skin selection screen image.
 */
#define PLAYER_PIC_WIDTH 68
#define PLAYER_PIC_HEIGHT 114
#define PLAYER_DEST_WIDTH 128
#define PLAYER_DEST_HEIGHT 128

byte		menuplyr_pixels[MAX_PLAYER_CLASS][PLAYER_PIC_WIDTH*PLAYER_PIC_HEIGHT];

qpic_t *Draw_PicFromFile (char *name)
{
	qpic_t	*p;
	glpic_t *gl;

	p = (qpic_t	*)COM_LoadHunkFile (name);
	if (!p)
	{
		return NULL;
	}

	gl = (glpic_t *)p->data;

	gl->texnum = GL_LoadPicTexture (p);

	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return p;
}

// Pa3PyX: Like Draw_PicFromFile, except loads pic into
// a specified buffer if there is room
qpic_t *Draw_PicFileBuf(char *name, void *p, int *size)
{
	glpic_t *gl;

	p = (void *)COM_LoadBufFile(name, p, size);
	if (!p)
		return NULL;

	gl = (glpic_t *)(((qpic_t *)p)->data);

	gl->texnum = GL_LoadPicTexture(p);
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return p;
}

qpic_t *Draw_PicFromWad (char *name)
{
	qpic_t	*p;
	glpic_t	*gl;

	p = W_GetLumpName (name);
	gl = (glpic_t *)p->data;

#if ENABLE_SCRAP
	// load little ones into the scrap
	if (p->width < 64 && p->height < 64)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock (p->width, p->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;
		k = 0;
		for (i=0 ; i<p->height ; i++)
			for (j=0 ; j<p->width ; j++, k++)
				scrap_texels[texnum][(y+i)*BLOCK_WIDTH + x + j] = p->data[k];
		texnum += scrap_texnum;
		gl->texnum = texnum;
		gl->sl = (x+0.01)/(float)BLOCK_WIDTH;
		gl->sh = (x+p->width-0.01)/(float)BLOCK_WIDTH;
		gl->tl = (y+0.01)/(float)BLOCK_WIDTH;
		gl->th = (y+p->height-0.01)/(float)BLOCK_WIDTH;

		pic_count++;
		pic_texels += p->width*p->height;

		return p;
	}
nonscrap:
#endif	// ENABLE_SCRAP
	gl->texnum = GL_LoadPicTexture (p);
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;
	return p;
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
	glpic_t		*gl;

	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			return &pic->pic;

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
	menu_numcachepics++;
	strcpy (pic->name, path);

//
// load the pic from disk
//
	dat = (qpic_t *)COM_LoadTempFile (path);
	if (!dat)
		Sys_Error ("Draw_CachePic: failed to load %s", path);
	SwapPic (dat);

	// HACK HACK HACK --- we need to keep the bytes for
	// the translatable player picture just for the menu
	// configuration dialog
	/* garymct */
	if (!strcmp (path, "gfx/menu/netp1.lmp"))
		memcpy (menuplyr_pixels[0], dat->data, dat->width*dat->height);
	else if (!strcmp (path, "gfx/menu/netp2.lmp"))
		memcpy (menuplyr_pixels[1], dat->data, dat->width*dat->height);
	else if (!strcmp (path, "gfx/menu/netp3.lmp"))
		memcpy (menuplyr_pixels[2], dat->data, dat->width*dat->height);
	else if (!strcmp (path, "gfx/menu/netp4.lmp"))
		memcpy (menuplyr_pixels[3], dat->data, dat->width*dat->height);
#if defined(H2MP) || defined (H2W)
	else if (!strcmp (path, "gfx/menu/netp5.lmp"))
		memcpy (menuplyr_pixels[4], dat->data, dat->width*dat->height);
#endif
#if defined (H2W)
	else if (!strcmp (path, "gfx/menu/netp6.lmp"))
		memcpy (menuplyr_pixels[5], dat->data, dat->width*dat->height);
#endif

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gl = (glpic_t *)pic->pic.data;
	gl->texnum = GL_LoadPicTexture (dat);
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return &pic->pic;
}

/*
================
Draw_CachePicNoTrans

Pa3PyX: Function added to cache pics ignoring transparent
colors (e.g. in intermission screens)
================
*/
qpic_t *Draw_CachePicNoTrans(char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	glpic_t		*gl;

	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			return &pic->pic;

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
	menu_numcachepics++;
	strcpy (pic->name, path);

//
// load the pic from disk
//
	dat = (qpic_t *)COM_LoadTempFile (path);
	if (!dat)
		Sys_Error ("Draw_CachePicNoTrans: failed to load %s", path);
	SwapPic (dat);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gl = (glpic_t *)pic->pic.data;
	// Get rid of transparencies
	for (i = 0; i < dat->width * dat->height; i++)
	     if (dat->data[i] == 255)
		 dat->data[i] = 31; // pal(31) == pal(255) == FCFCFC (white)
	gl->texnum = GL_LoadPicTexture (dat);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return &pic->pic;
}

#if 0
static void Draw_CharToConback (int num, byte *dest)
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
			if (source[x] != 255)
				dest[x] = 0x60 + source[x];

		source += 256;
		dest += 320;
	}
}
#endif

typedef struct
{
	char *name;
	int	minimize, maximize;
} glmode_t;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

/*
===============
Draw_TextureMode_f
===============
*/
void Draw_TextureMode_f (void)
{
	int		i;
	gltexture_t	*glt;

	if (Cmd_Argc() == 1)
	{
		for (i=0 ; i< 6 ; i++)
			if (gl_filter_min == modes[i].minimize)
			{
				Con_Printf ("%s\n", modes[i].name);
				return;
			}
		Con_Printf ("current filter is unknown???\n");
		return;
	}

	for (i=0 ; i< 6 ; i++)
	{
		if (!Q_strcasecmp (modes[i].name, Cmd_Argv(1) ) )
			break;
	}
	if (i == 6)
	{
		Con_Printf ("bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		GL_Bind (glt->texnum);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

		if (glt->mipmap)
		{
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		}
		else
		{
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		}
	}

	// Pa3PyX: force update of all lightmaps
	for (i = 0; i < MAX_LIGHTMAPS; i++)
		lightmap_modified[i] = true;
}

/*
===============
Draw_Init
===============
*/
void Draw_Init (void)
{
	static int bt_len;
	int	i;
	qpic_t	*cb, *mf;
/*	byte	*dest;
	int	x;
	char	ver[40];*/
	glpic_t	*gl;
	int	start;
	byte	*ncdata;

	if (!draw_reinit)
	{
		Cvar_RegisterVariable (&gl_picmip);
		Cvar_RegisterVariable (&gl_spritemip);

		Cmd_AddCommand ("gl_texturemode", &Draw_TextureMode_f);
		Cmd_AddCommand ("gl_texels", &GL_Texels_f);
		// load the console background and the charset
		// by hand, because we need to write the version
		// string into the background before turning
		// it into a texture
		draw_chars = COM_LoadHunkFile ("gfx/menu/conchars.lmp");
		for (i=0 ; i<256*128 ; i++)
			if (draw_chars[i] == 0)
				draw_chars[i] = 255;	// proper transparent color
	}

	char_texture = GL_LoadTexture ("charset", 256, 128, draw_chars, false, true, 0, false);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	cs_texture = GL_LoadPixmap ("crosshair", cs_data);

	draw_smallchars = W_GetLumpName("tinyfont");
	for (i=0 ; i<128*32 ; i++)
		if (draw_smallchars[i] == 0)
			draw_smallchars[i] = 255;	// proper transparent color

	// now turn them into textures
	char_smalltexture = GL_LoadTexture ("smallcharset", 128, 32, draw_smallchars, false, true, 0, false);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	mf = (qpic_t *)COM_LoadTempFile("gfx/menu/bigfont2.lmp");
	for (i=0 ; i<160*80 ; i++)
		if (mf->data[i] == 0)
			mf->data[i] = 255;	// proper transparent color

	char_menufonttexture = GL_LoadTexture ("menufont", 160, 80, mf->data, false, true, 0, false);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	start = Hunk_LowMark ();

	cb = (qpic_t *)COM_LoadHunkFile ("gfx/menu/conback.lmp");
	if (!cb)
		Sys_Error ("Couldn't load gfx/menu/conback.lmp");
	SwapPic (cb);

/*	// hack the version number directly into the pic
	sprintf (ver, ENGINE_WATERMARK);
	dest = cb->data + 320 + 320*186 - 11 - 8*strlen(ver);
	for (x=0 ; x<strlen(ver) ; x++)
		Draw_CharToConback (ver[x], dest+(x<<3));
*/
	conback->width = cb->width;
	conback->height = cb->height;
	ncdata = cb->data;

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	gl = (glpic_t *)conback->data;
	gl->texnum = GL_LoadTexture ("conback", conback->width, conback->height, ncdata, false, false, 0, false);
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;
	conback->width = vid.conwidth;
	conback->height = vid.conheight;

	// free loaded console
	Hunk_FreeToLowMark (start);

#if ENABLE_SCRAP
	// save slots for scraps
	scrap_texnum = texture_extension_number;
	texture_extension_number += MAX_SCRAPS;
#endif
//	draw_backtile = Draw_PicFromWad ("backtile");
//	draw_backtile = Draw_PicFromFile ("gfx/menu/backtile.lmp");
	draw_backtile = Draw_PicFileBuf("gfx/menu/backtile.lmp", draw_backtile, &bt_len);
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
	int			row, col;
	float			frow, fcol, xsize,ysize;

	if (num == 32)
		return;		// space

	num &= 511;

	if (y <= -8)
		return;		// totally off screen

	row = num>>5;
	col = num&31;

	xsize = 0.03125;
	ysize = 0.0625;
	fcol = col*xsize;
	frow = row*ysize;

	GL_Bind (char_texture);

	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (fcol, frow);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (fcol + xsize, frow);
	glVertex2f_fp (x+8, y);
	glTexCoord2f_fp (fcol + xsize, frow + ysize);
	glVertex2f_fp (x+8, y+8);
	glTexCoord2f_fp (fcol, frow + ysize);
	glVertex2f_fp (x, y+8);
	glEnd_fp ();
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

void Draw_RedString (int x, int y, char *str)
{
	while (*str)
	{
		Draw_Character (x, y, ((unsigned char)(*str))+256);
		str++;
		x += 8;
	}
}

void Draw_Crosshair(void)
{
	int x, y;
	unsigned char *pColor;

	x = scr_vrect.x + scr_vrect.width/2 + cl_crossx.value; 
	y = scr_vrect.y + scr_vrect.height/2 + cl_crossy.value;

	if ((int)crosshair.value == 2)
	{
		pColor = (unsigned char *) &d_8to24table[(byte) crosshaircolor.value];

		glTexEnvf_fp (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4ubv_fp (pColor);
		GL_Bind (cs_texture);

		// Our crosshair is now 32x32, but we're drawing 16x16 here
		// to have a smaller pic. If, in the pixmap, the pixels are
		// not drawn in doubles, the final image on the screen may
		// have some of the pixels missing. Sigh...
		glBegin_fp (GL_QUADS);
		glTexCoord2f_fp (0, 0);
		glVertex2f_fp (x - 7, y - 7);
		glTexCoord2f_fp (1, 0);
		glVertex2f_fp (x + 9, y - 7);
		glTexCoord2f_fp (1, 1);
		glVertex2f_fp (x + 9, y + 9);
		glTexCoord2f_fp (0, 1);
		glVertex2f_fp (x - 7, y + 9);

		glEnd_fp ();
		glTexEnvf_fp (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
	else if (crosshair.value)
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
	int			row, col;
	float			frow, fcol, xsize,ysize;

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

	if (num == 0) return;

	if (y <= -8)
		return; 	// totally off screen

	if(y >= vid.height)
	{ // Totally off screen
		return;
	}

	row = num>>4;
	col = num&15;

	xsize = 0.0625;
	ysize = 0.25;
	fcol = col*xsize;
	frow = row*ysize;

	GL_Bind (char_smalltexture);

	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (fcol, frow);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (fcol + xsize, frow);
	glVertex2f_fp (x+8, y);
	glTexCoord2f_fp (fcol + xsize, frow + ysize);
	glVertex2f_fp (x+8, y+8);
	glTexCoord2f_fp (fcol, frow + ysize);
	glVertex2f_fp (x, y+8);
	glEnd_fp ();
}

//==========================================================================
//
// Draw_SmallString
//
//==========================================================================
void Draw_SmallString(int x, int y, char *str)
{
	while (*str)
	{
		Draw_SmallCharacter (x, y, *str);
		str++;
		x += 6;
	}
}

/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	glpic_t			*gl;

#if ENABLE_SCRAP
	if (scrap_dirty)
		Scrap_Upload ();
#endif
	gl = (glpic_t *)pic->data;
	glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (gl->sl, gl->tl);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (gl->sh, gl->tl);
	glVertex2f_fp (x+pic->width, y);
	glTexCoord2f_fp (gl->sh, gl->th);
	glVertex2f_fp (x+pic->width, y+pic->height);
	glTexCoord2f_fp (gl->sl, gl->th);
	glVertex2f_fp (x, y+pic->height);
	glEnd_fp ();

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/*
=============
Draw_AlphaPic
=============
*/
void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha)
{
	glpic_t			*gl;

#if ENABLE_SCRAP
	if (scrap_dirty)
		Scrap_Upload ();
#endif
	gl = (glpic_t *)pic->data;
	glDisable_fp(GL_ALPHA_TEST);
	glEnable_fp (GL_BLEND);
//	glBlendFunc_fp(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace_fp(GL_FRONT);
	glColor4f_fp (1,1,1,alpha);
	GL_Bind (gl->texnum);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (gl->sl, gl->tl);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (gl->sh, gl->tl);
	glVertex2f_fp (x+pic->width, y);
	glTexCoord2f_fp (gl->sh, gl->th);
	glVertex2f_fp (x+pic->width, y+pic->height);
	glTexCoord2f_fp (gl->sl, gl->th);
	glVertex2f_fp (x, y+pic->height);
	glEnd_fp ();
	glColor4f_fp (1,1,1,1);
	glEnable_fp(GL_ALPHA_TEST);
	glDisable_fp (GL_BLEND);
}

/*
=============
Draw_IntermissionPic

Pa3PyX: this new function introduced to draw the intermission screen only
=============
*/
void Draw_IntermissionPic (qpic_t *pic)
{
	glpic_t			*gl;

#if ENABLE_SCRAP
	if (scrap_dirty)
		Scrap_Upload ();
#endif
	gl = (glpic_t *)pic->data;
	glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBegin_fp(GL_QUADS);
	glTexCoord2f_fp(0.0f, 0.0f);
	glVertex2f_fp(0.0f, 0.0f);
	glTexCoord2f_fp(1.0f, 0.0f);
	glVertex2f_fp(vid.width, 0.0f);
	glTexCoord2f_fp(1.0f, 1.0f);
	glVertex2f_fp(vid.width, vid.height);
	glTexCoord2f_fp(0.0f, 1.0f);
	glVertex2f_fp(0.0f, vid.height);
	glEnd_fp();

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Draw_SubPic(int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height)
{
	glpic_t			*gl;
	float	newsl, newtl, newsh, newth;
	float	oldglwidth, oldglheight;

#if ENABLE_SCRAP
	if (scrap_dirty)
		Scrap_Upload ();
#endif
	gl = (glpic_t *)pic->data;

	oldglwidth = gl->sh - gl->sl;
	oldglheight = gl->th - gl->tl;

	newsl = gl->sl + (srcx*oldglwidth)/pic->width;
	newsh = newsl + (width*oldglwidth)/pic->width;

	newtl = gl->tl + (srcy*oldglheight)/pic->height;
	newth = newtl + (height*oldglheight)/pic->height;

	glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (newsl, newtl);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (newsh, newtl);
	glVertex2f_fp (x+width, y);
	glTexCoord2f_fp (newsh, newth);
	glVertex2f_fp (x+width, y+height);
	glTexCoord2f_fp (newsl, newth);
	glVertex2f_fp (x, y+height);
	glEnd_fp ();
}

void Draw_PicCropped(int x, int y, qpic_t *pic)
{
	int		height;
	glpic_t 	*gl;
	float		th,tl;

	if((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("Draw_PicCropped: bad coordinates");
	}

	if (y >= (int)vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

#if ENABLE_SCRAP
	if (scrap_dirty)
		Scrap_Upload ();
#endif
	gl = (glpic_t *)pic->data;

	// rjr tl/th need to be computed based upon pic->tl and pic->th
	//     cuz the piece may come from the scrap
	if(y+pic->height > vid.height)
	{
		height = vid.height-y;
		tl = 0;
		th = (height-0.01)/pic->height;
	}
	else if (y < 0)
	{
		height = pic->height+y;
		y = -y;
		tl = (y-0.01)/pic->height;
		th = (pic->height-0.01)/pic->height;
		y = 0;
	}
	else
	{
		height = pic->height;
		tl = gl->tl;
		th = gl->th;//(height-0.01)/pic->height;
	}

	glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (gl->sl, tl);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (gl->sh, tl);
	glVertex2f_fp (x+pic->width, y);
	glTexCoord2f_fp (gl->sh, th);
	glVertex2f_fp (x+pic->width, y+height);
	glTexCoord2f_fp (gl->sl, th);
	glVertex2f_fp (x, y+height);
	glEnd_fp ();
}

void Draw_SubPicCropped(int x, int y, int h, qpic_t *pic)
{
	int		height;
	glpic_t 	*gl;
	float		th,tl;

	if((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("Draw_PicCropped: bad coordinates");
	}

	if (y >= (int)vid.height || y+h < 0)
	{ // Totally off screen
		return;
	}

#if ENABLE_SCRAP
	if (scrap_dirty)
		Scrap_Upload ();
#endif
	gl = (glpic_t *)pic->data;

	// rjr tl/th need to be computed based upon pic->tl and pic->th
	//     cuz the piece may come from the scrap
	if(y+pic->height > vid.height)
	{
		height = vid.height-y;
		tl = 0;
		th = (height-0.01)/pic->height;
	}
	else if (y < 0)
	{
		height = pic->height+y;
		y = -y;
		tl = (y-0.01)/pic->height;
		th = (pic->height-0.01)/pic->height;
		y = 0;
	}
	else
	{
		height = pic->height;
		tl = gl->tl;
		th = gl->th;//(height-0.01)/pic->height;
	}

	if (height > h) 
	{
		height = h;
	}

	glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (gl->sl, tl);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (gl->sh, tl);
	glVertex2f_fp (x+pic->width, y);
	glTexCoord2f_fp (gl->sh, th);
	glVertex2f_fp (x+pic->width, y+height);
	glTexCoord2f_fp (gl->sl, th);
	glVertex2f_fp (x, y+height);
	glEnd_fp ();
}

/*
=============
Draw_TransPic
=============
*/
void Draw_TransPic (int x, int y, qpic_t *pic)
{
	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 ||
		 (unsigned)(y + pic->height) > vid.height)
	{
		Sys_Error ("Draw_TransPic: bad coordinates");
	}

	Draw_Pic (x, y, pic);
}

void Draw_TransPicCropped(int x, int y, qpic_t *pic)
{
	Draw_PicCropped (x, y, pic);
}

/*
=============
Draw_TransPicTranslate

Only used for the player color selection menu
=============
*/
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation, int p_class, int top, int bottom)
{
	int		i, j, v, u;
	unsigned	trans[PLAYER_DEST_WIDTH * PLAYER_DEST_HEIGHT], *dest;
	byte		*src;
	int		p;

	// texture handle, name and trackers (Pa3PyX)
	char texname[20];
	static qboolean first_time = true;

	// Initialize array of texnums
	if (first_time) {
		memset(plyrtex, 0, MAX_PLAYER_CLASS * 16 * 16 * sizeof(qboolean));
		first_time = false;
	}

	dest = trans;
	for (v=0 ; v<64 ; v++, dest += 64)
	{
		src = &menuplyr_pixels[p_class-1][ ((v*pic->height)>>6) *pic->width];
		for (u=0 ; u<64 ; u++)
		{
			p = src[(u*pic->width)>>6];
			if (p == 255)
				dest[u] = p;
			else
				dest[u] =  d_8to24table[translation[p]];
		}
	}

	for( i = 0; i < PLAYER_PIC_WIDTH; i++ )
	{
		for( j = 0; j < PLAYER_PIC_HEIGHT; j++ )
		{
			trans[j * PLAYER_DEST_WIDTH + i] = d_8to24table[translation[menuplyr_pixels[p_class-1][j * PLAYER_PIC_WIDTH + i]]];
		}
	}

	// See if the texture has already been loaded; if not, do it (Pa3PyX)
	if (!plyrtex[p_class - 1][top][bottom])
	{
		snprintf(texname, 19, "plyrmtex%i%i%i", p_class, top, bottom);
		plyrtex[p_class - 1][top][bottom] = GL_LoadTexture(texname, PLAYER_DEST_WIDTH, PLAYER_DEST_HEIGHT, (byte *)trans, false, true, 0, true);
	}
	GL_Bind(plyrtex[p_class - 1][top][bottom]);

	glColor3f_fp (1,1,1);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (0, 0);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (( float )PLAYER_PIC_WIDTH / PLAYER_DEST_WIDTH, 0);
	glVertex2f_fp (x+pic->width, y);
	glTexCoord2f_fp (( float )PLAYER_PIC_WIDTH / PLAYER_DEST_WIDTH, ( float )PLAYER_PIC_HEIGHT / PLAYER_DEST_HEIGHT);
	glVertex2f_fp (x+pic->width, y+pic->height);
	glTexCoord2f_fp (0, ( float )PLAYER_PIC_HEIGHT / PLAYER_DEST_HEIGHT);
	glVertex2f_fp (x, y+pic->height);
	glEnd_fp ();
}

int M_DrawBigCharacter (int x, int y, int num, int numNext)
{
	int		row, col;
	float		frow, fcol, xsize,ysize;
	int		add;

	if (num == ' ')
		return 32;

	if (num == '/')
		num = 26;
	else
		num -= 65;

	if (num < 0 || num >= 27)  // only a-z and /
		return 0;

	if (numNext == '/')
		numNext = 26;
	else
		numNext -= 65;

	row = num/8;
	col = num%8;

	xsize = 0.125;
	ysize = 0.25;
	fcol = col*xsize;
	frow = row*ysize;

	GL_Bind (char_menufonttexture);

	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (fcol, frow);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp (fcol + xsize, frow);
	glVertex2f_fp (x+20, y);
	glTexCoord2f_fp (fcol + xsize, frow + ysize);
	glVertex2f_fp (x+20, y+20);
	glTexCoord2f_fp (fcol, frow + ysize);
	glVertex2f_fp (x, y+20);
	glEnd_fp ();

	if (numNext < 0 || numNext >= 27)
		return 0;

	add = 0;
	if (num == (int)'C'-65 && numNext == (int)'P'-65)
		add = 3;

	return BigCharWidth[num][numNext] + add;
}


/*
================
Draw_ConsoleBackground

================
*/
void Draw_ConsoleBackground (int lines)
{
	char	ver[80];
	int	i, x, y;

	y = (vid.height * 3) >> 2;
	if (lines > y)
		Draw_Pic (0, lines-vid.height, conback);
	else
		Draw_AlphaPic (0, lines - vid.height, conback, (float)(1.1 * lines)/y);

	// print the version number and platform
//	y = lines-186;
	y = lines-14;
	sprintf (ver, ENGINE_WATERMARK);
	x = vid.conwidth - (strlen(ver)*8 + 11);
#if defined(H2W)
	if (!cls.download)
#endif
		for (i=0 ; i<strlen(ver) ; i++)
			Draw_Character (x + i * 8, y, ver[i] | 0x100);
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
	glColor3f_fp (1,1,1);
	GL_Bind (*(int *)draw_backtile->data);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (x/64.0, y/64.0);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp ( (x+w)/64.0, y/64.0);
	glVertex2f_fp (x+w, y);
	glTexCoord2f_fp ( (x+w)/64.0, (y+h)/64.0);
	glVertex2f_fp (x+w, y+h);
	glTexCoord2f_fp ( x/64.0, (y+h)/64.0 );
	glVertex2f_fp (x, y+h);
	glEnd_fp ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	glDisable_fp (GL_TEXTURE_2D);
	glColor3f_fp (host_basepal[c*3]/255.0,
		host_basepal[c*3+1]/255.0,
		host_basepal[c*3+2]/255.0);

	glBegin_fp (GL_QUADS);

	glVertex2f_fp (x,y);
	glVertex2f_fp (x+w, y);
	glVertex2f_fp (x+w, y+h);
	glVertex2f_fp (x, y+h);

	glEnd_fp ();
	glColor3f_fp (1,1,1);
	glEnable_fp (GL_TEXTURE_2D);
}
//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	int bx, by, ex, ey;
	int c;

#if 0
	// fadescreen ripple effect stops help/quit screens
	// from appearing on 3dfx with Mesa Glide.  S.A

	// fadescreen actually works with supported versions
	// of mesa, such as 3.0, 3.2.1, 3.4.2 and 6.2.1.
	// It only seems to break when using an old binary
	// mesaGL from a quake3 package.		O.S.

	if (is_3dfx)
		return;
#endif

	glAlphaFunc_fp(GL_ALWAYS, 0);

	glEnable_fp (GL_BLEND);
	glDisable_fp (GL_TEXTURE_2D);

//	glColor4f_fp (248.0/255.0, 220.0/255.0, 120.0/255.0, 0.2);
	glColor4f_fp (208.0/255.0, 180.0/255.0, 80.0/255.0, 0.2);
	glBegin_fp (GL_QUADS);

	glVertex2f_fp (0,0);
	glVertex2f_fp (vid.width, 0);
	glVertex2f_fp (vid.width, vid.height);
	glVertex2f_fp (0, vid.height);
	glEnd_fp ();

	glColor4f_fp (208.0/255.0, 180.0/255.0, 80.0/255.0, 0.035);
	for(c=0;c<40;c++)
	{
		bx = rand() % vid.width-20;
		by = rand() % vid.height-20;
		ex = bx + (rand() % 40) + 20;
		ey = by + (rand() % 40) + 20;
		if (bx < 0)
			bx = 0;
		if (by < 0)
			by = 0;
		if (ex > vid.width)
			ex = vid.width;
		if (ey > vid.height)
			ey = vid.height;

		glBegin_fp (GL_QUADS);
		glVertex2f_fp (bx, by);
		glVertex2f_fp (ex, by);
		glVertex2f_fp (ex, ey);
		glVertex2f_fp (bx, ey);
		glEnd_fp ();
	}

	glColor4f_fp (1,1,1,1);
	glEnable_fp (GL_TEXTURE_2D);
	glDisable_fp (GL_BLEND);

	glAlphaFunc_fp(GL_GREATER, 0.632);

	Sbar_Changed();
}

//=============================================================================

/*
================
GL_Set2D

Setup as if the screen was 320*200
================
*/
void GL_Set2D (void)
{
	glViewport_fp (glx, gly, glwidth, glheight);

	glMatrixMode_fp(GL_PROJECTION);
	glLoadIdentity_fp ();
	glOrtho_fp  (0, vid.width, vid.height, 0, -99999, 99999);

	glMatrixMode_fp(GL_MODELVIEW);
	glLoadIdentity_fp ();

	glDisable_fp (GL_DEPTH_TEST);
	glDisable_fp (GL_CULL_FACE);
	glDisable_fp (GL_BLEND);
	glEnable_fp (GL_ALPHA_TEST);
//	glDisable_fp (GL_ALPHA_TEST);

	glColor4f_fp (1,1,1,1);
}

//====================================================================

/*
================
GL_FindTexture
================
*/
int GL_FindTexture (char *identifier)
{
	int		i;
	gltexture_t	*glt;

	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (!strcmp (identifier, glt->identifier))
			return gltextures[i].texnum;
	}

	return -1;
}

/*
================
GL_ResampleTexture
================
*/
static void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow;
	unsigned	frac, fracstep;

	fracstep = inwidth*0x10000/outwidth;
	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j+=4)
		{
			out[j] = inrow[frac>>16];
			frac += fracstep;
			out[j+1] = inrow[frac>>16];
			frac += fracstep;
			out[j+2] = inrow[frac>>16];
			frac += fracstep;
			out[j+3] = inrow[frac>>16];
			frac += fracstep;
		}
	}
}

/*
================
GL_MipMap

Operates in place, quartering the size of the texture
================
*/
static void GL_MipMap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

#ifdef OLD_8_BIT_PALETTE_CODE
/*
================
fxPalTexImage2D

Acts the same as glTexImage2D, except that it maps color
into the current palette and uses paletteized textures.
If you are on a 3Dfx card and your texture has no alpha,
then download it as a palettized texture to save memory.

This is original hexen2 code for palettized textures
Hexenworld replaced it with quake's newer code below
================
*/
static void fxPalTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, unsigned char *dest_image)
{
	long	i;
	long	mip_width, mip_height;

	mip_width = width;
	mip_height = height;

	// we don't want textures with alpha
	if (internalformat != 3)
		Sys_Error ("fxPalTexImage2D: internalformat != 3");

	for (i = 0; i < mip_width * mip_height; i++)
	{
		int r, g, b, index;
		r = ( ( unsigned char * )pixels )[i * 4];
		g = ( ( unsigned char * )pixels )[i * 4+1];
		b = ( ( unsigned char * )pixels )[i * 4+2];
		r >>= 8 - INVERSE_PAL_R_BITS;
		g >>= 8 - INVERSE_PAL_G_BITS;
		b >>= 8 - INVERSE_PAL_B_BITS;
		index = ( r << ( INVERSE_PAL_G_BITS + INVERSE_PAL_B_BITS ) ) |
			( g << INVERSE_PAL_B_BITS ) |
			  b;
		dest_image[i] = inverse_pal[index];
//		dest_image[i] = ( ( unsigned char * )pixels )[i*4];
	}

	glTexImage2D_fp(target, level, GL_COLOR_INDEX8_EXT, mip_width, mip_height, border, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, dest_image);
}

#else	// end of OLD_8_BIT_PALETTE_CODE
/*
================
GL_Resample8BitTexture -- JACK
================
*/
static void GL_Resample8BitTexture (unsigned char *in, int inwidth, int inheight, unsigned char *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	char *inrow;
	unsigned	frac, fracstep;

	fracstep = inwidth*0x10000/outwidth;
	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j+=4)
		{
			out[j] = inrow[frac>>16];
			frac += fracstep;
			out[j+1] = inrow[frac>>16];
			frac += fracstep;
			out[j+2] = inrow[frac>>16];
			frac += fracstep;
			out[j+3] = inrow[frac>>16];
			frac += fracstep;
		}
	}
}

/*
================
GL_MipMap8Bit

Mipping for 8 bit textures
================
*/
static void GL_MipMap8Bit (byte *in, int width, int height)
{
	int		i, j;
	unsigned short     r,g,b;
	byte	*out, *at1, *at2, *at3, *at4;

//	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=2, out+=1, in+=2)
		{
			at1 = (byte *) &d_8to24table[in[0]];
			at2 = (byte *) &d_8to24table[in[1]];
			at3 = (byte *) &d_8to24table[in[width+0]];
			at4 = (byte *) &d_8to24table[in[width+1]];

 			r = (at1[0]+at2[0]+at3[0]+at4[0]); r>>=5;
 			g = (at1[1]+at2[1]+at3[1]+at4[1]); g>>=5;
 			b = (at1[2]+at2[2]+at3[2]+at4[2]); b>>=5;

			out[0] = d_15to8table[(r<<0) + (g<<5) + (b<<10)];
//			out[0] = (in[0] + in[1] + in[width+0] + in[width+1])>>2;
//			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
//			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
//			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

static void GL_Upload8_EXT (byte *data, int width, int height,  qboolean mipmap, qboolean alpha, qboolean sprite)
{
	unsigned char 		*scaled;
	int			mark = 0;
	int			scaled_width, scaled_height;

	// Snap the height and width to a power of 2
	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;

	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;

	if (sprite)
	{
		scaled_width >>= (int)gl_spritemip.value;
		scaled_height >>= (int)gl_spritemip.value;
	}
	else
	{
		scaled_width >>= (int)gl_picmip.value;
		scaled_height >>= (int)gl_picmip.value;
	}

	if (scaled_width < 1)
		scaled_width = 1;

	if (scaled_height < 1)
		scaled_height = 1;

	if (scaled_width > gl_max_size)
		scaled_width = gl_max_size;

	if (scaled_height > gl_max_size)
		scaled_height = gl_max_size;

	// 3dfx has some aspect ratio constraints.
	// can't go beyond 8 to 1 or below 1 to 8.
	if( is_3dfx )
	{
		if( scaled_width * 8 < scaled_height )
		{
			scaled_width = scaled_height >> 3;
		}
		else if( scaled_height * 8 < scaled_width )
		{
			scaled_height = scaled_width >> 3;
		}
	}

	texels += scaled_width * scaled_height;
	
	mark = Hunk_LowMark();
	scaled = Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			glTexImage2D_fp (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX , GL_UNSIGNED_BYTE, data);
			goto done;
		}
		//scaled = data;
		memcpy (scaled, data, width*height);
	}
	else
	{
		//mark = Hunk_LowMark();
		//scaled = Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");
		GL_Resample8BitTexture (data, width, height, scaled, scaled_width, scaled_height);
	}

	glTexImage2D_fp (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, scaled);
	if (mipmap)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			GL_MipMap8Bit ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if (scaled_width < 1)
				scaled_width = 1;
			if (scaled_height < 1)
				scaled_height = 1;
			miplevel++;
			glTexImage2D_fp (GL_TEXTURE_2D, miplevel, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, scaled);
		}
	}
done: ;

	if (mipmap)
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	if (mark)
		Hunk_FreeToLowMark(mark);
}
#endif	// end of new 8_BIT_PALETTE_CODE

/*
===============
GL_Upload32
===============
*/
void GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap, qboolean alpha, qboolean sprite)
{
	int		samples;
	unsigned	*scaled;
	int		mark = 0;
	int		scaled_width, scaled_height;
#ifdef OLD_8_BIT_PALETTE_CODE
	unsigned char	*fxpal_buf = NULL;
#endif

	// Snap the height and width to a power of 2.
	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;

	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;

	if (sprite)
	{
		scaled_width >>= (int)gl_spritemip.value;
		scaled_height >>= (int)gl_spritemip.value;
	}
	else
	{
		scaled_width >>= (int)gl_picmip.value;
		scaled_height >>= (int)gl_picmip.value;
	}

	if (scaled_width < 1)
		scaled_width = 1;

	if (scaled_height < 1)
		scaled_height = 1;

	if (scaled_width > gl_max_size)
		scaled_width = gl_max_size;

	if (scaled_height > gl_max_size)
		scaled_height = gl_max_size;

	// 3dfx has some aspect ratio constraints.
	// can't go beyond 8 to 1 or below 1 to 8.
	if( is_3dfx )
	{
		if( scaled_width * 8 < scaled_height )
		{
			scaled_width = scaled_height >> 3;
		}
		else if( scaled_height * 8 < scaled_width )
		{
			scaled_height = scaled_width >> 3;
		}
	}

	samples = alpha ? gl_alpha_format : gl_solid_format;

	texels += scaled_width * scaled_height;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
#ifdef OLD_8_BIT_PALETTE_CODE
			if (is8bit && !alpha)
			{
				mark = Hunk_LowMark();
				fxpal_buf = Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");
				fxPalTexImage2D (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data, fxpal_buf);
				Hunk_FreeToLowMark(mark);
				mark = 0;
			}
			else
#endif
				glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			goto done;
		}
		scaled = data;;
	}
	else
	{
		mark = Hunk_LowMark();
		scaled = Hunk_AllocName(scaled_width * scaled_height * sizeof(unsigned int), "texbuf_upload32");
		GL_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);
	}

#ifdef OLD_8_BIT_PALETTE_CODE
	if (is8bit && !alpha)
	{
		if (!mark)
			mark = Hunk_LowMark();
		fxpal_buf = Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");
		fxPalTexImage2D (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled, fxpal_buf);
	}
	else
#endif
		glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);

	if (mipmap)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			GL_MipMap ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if (scaled_width < 1)
				scaled_width = 1;
			if (scaled_height < 1)
				scaled_height = 1;
			miplevel++;
#ifdef OLD_8_BIT_PALETTE_CODE
			if (is8bit && !alpha)
				fxPalTexImage2D (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled, fxpal_buf);
			else
#endif
				glTexImage2D_fp (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		}
	}
done: ;

	if (mipmap)
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	if (mark)
		Hunk_FreeToLowMark(mark);
}

/*
===============
GL_Upload8
===============
*/
/*
* mode:
* 0 - standard
* 1 - color 0 transparent, odd - translucent, even - full value
* 2 - color 0 transparent
* 3 - special (particle translucency table)
*/
void GL_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean alpha, int mode)
{
	unsigned		*trans;
	int			mark;
	int			i, p, s;
	qboolean		noalpha;
	qboolean		sprite = false;

	if (mode >= 10)
	{
		sprite = true;
		mode -= 10;
	}

	s = width*height;
	mark = Hunk_LowMark();
	trans = Hunk_AllocName(s * sizeof(unsigned), "texbuf_upload8");

	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
	if ((alpha || mode != 0))
	{
		i = 0;
		noalpha = true;
		while (i<s)
		{
			if (data[i] == 255)
			{
				noalpha = false;
				break;
			}
			i++;
		}

		if (alpha && noalpha)
			alpha = false;

		for (i=0 ; i<s ; i++)
		{
			p = data[i];
			trans[i] = d_8to24table[p];

			if (alpha && p == 255)
			{	// transparent, so scan around for another color
				// to avoid alpha fringes
				if (i > width && data[i-width] != 255)
					p = data[i-width];
				else if (i < s-width && data[i+width] != 255)
					p = data[i+width];
				else if (i > 0 && data[i-1] != 255)
					p = data[i-1];
				else if (i < s-1 && data[i+1] != 255)
					p = data[i+1];
				else
					p = 0;
				// copy rgb components
				((byte *)&trans[i])[0] = ((byte *)&d_8to24table[p])[0];
				((byte *)&trans[i])[1] = ((byte *)&d_8to24table[p])[1];
				((byte *)&trans[i])[2] = ((byte *)&d_8to24table[p])[2];
			}
		}

		switch( mode )
		{
		case 1:
			alpha = true;
			for (i=0 ; i<s ; i++)
			{
				p = data[i];
				if (p == 0)
					trans[i] &= 0x00ffffff;
				else if( p & 1 )
				{
					trans[i] &= 0x00ffffff;
					trans[i] |= ( ( int )( 255 * r_wateralpha.value ) ) << 24;
				}
				else
				{
					trans[i] |= 0xff000000;
				}
			}
			break;
		case 2:
			alpha = true;
			for (i=0 ; i<s ; i++)
			{
				p = data[i];
				if (p == 0)
					trans[i] &= 0x00ffffff;
			}
			break;
		case 3:
			alpha = true;
			for (i=0 ; i<s ; i++)
			{
				p = data[i];
				trans[i] = d_8to24table[ColorIndex[p>>4]] & 0x00ffffff;
				trans[i] |= ( int )ColorPercent[p&15] << 24;
				//trans[i] = 0x7fff0000;
			}
			break;
		}
	}
	else
	{
		if (s&3)
			Sys_Error ("GL_Upload8: s&3");
		for (i=0 ; i<s ; i+=4)
		{
			trans[i] = d_8to24table[data[i]];
			trans[i+1] = d_8to24table[data[i+1]];
			trans[i+2] = d_8to24table[data[i+2]];
			trans[i+3] = d_8to24table[data[i+3]];
		}
	}

#ifndef OLD_8_BIT_PALETTE_CODE
	if (is8bit && 
#   if ENABLE_SCRAP
		(data!=scrap_texels[0]) && 
#   endif
		!alpha)
	{
		GL_Upload8_EXT (data, width, height, mipmap, alpha, sprite);
		Hunk_FreeToLowMark(mark);
		return;
	}
#endif	// !OLD_8_BIT_PALETTE_CODE

	GL_Upload32 (trans, width, height, mipmap, alpha, sprite);
	Hunk_FreeToLowMark(mark);
}


/*
================
GL_LoadTexture
================
*/
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int mode, qboolean rgba)
{
	int		i, size;
	unsigned long	hash = 0;
//	unsigned short	crc;
	gltexture_t	*glt;

	size = width * height;
	if (rgba)
		size *= 4;

	// generate texture checksum
	for (i = 0; i < size; i++)
		hash += data[i];

/*	// alternative: use stock crc functions for checksumming
	crc = CRC_Block (data, size);
*/
	// see if the texture is already present
	if (identifier[0])
	{
		for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
		{
			if (!strcmp (identifier, glt->identifier))
			{
				//if ( crc != glt->crc ||
				if ( hash != glt->hash ||
					width  != glt->width  ||
					height != glt->height ||
					mipmap != glt->mipmap )
				{	// not the same, delete and rebind to new image
					Con_Printf ("Texture cache mismatch: %d, %s, reloading\n", glt->texnum, identifier);
#				if !defined (H2W)
					if (cls.state != ca_dedicated)
#				endif
						glDeleteTextures_fp (1, &(glt->texnum));
					goto gl_rebind;
				}
				else
				{	// No need to rebind
					return gltextures[i].texnum;
				}
			}
		}
	}

	if (numgltextures >= MAX_GLTEXTURES)
	{
		Sys_Error ("GL_LoadTexture: cache full, max is %i textures.\n"
			   "Consider setting gl_purge_maptex option to 1 .",
			   MAX_GLTEXTURES);
	}

	glt = &gltextures[numgltextures];
	numgltextures++;
	strcpy (glt->identifier, identifier);
	glt->texnum = texture_extension_number;
	texture_extension_number++;

gl_rebind:
	glt->width = width;
	glt->height = height;
	glt->mipmap = mipmap;
//	glt->crc = crc;
	glt->hash = hash;

#if !defined (H2W)
	if (cls.state == ca_dedicated)
		goto dedicated;
#endif

	GL_Bind (glt->texnum);
	if (rgba)
		GL_Upload32 ((unsigned *)data, width, height, mipmap, alpha, false);
	else
		GL_Upload8 (data, width, height, mipmap, alpha, mode);

#if !defined (H2W)
dedicated:
#endif
	return glt->texnum;
}

/*
===============
GL_LoadPixmap
from LordHavoc's Twilight (DarkPlaces) project

Loads a string into a named 32x32 greyscale OpenGL texture, suitable for
crosshairs or pointers. The data string is in a format similar to an X11
pixmap.  '0'-'7' are brightness levels, any other character is considered
transparent. Remember, NO error checking is performed on the input string.
*/
static int GL_LoadPixmap (char *name, char *data)
{
	int		i;
	unsigned char	pixels[32*32][4];

	for (i = 0; i < 32*32; i++)
	{
		if (data[i] >= '0' && data[i] < '8')
		{
			pixels[i][0] = 255;
			pixels[i][1] = 255;
			pixels[i][2] = 255;
			pixels[i][3] = (data[i] - '0') * 32;
		} else {
			pixels[i][0] = 255;
			pixels[i][1] = 255;
			pixels[i][2] = 255;
			pixels[i][3] = 0;
		}
	}

	return GL_LoadTexture (name, 32, 32, (unsigned char *) pixels, false, true, 0, true);
}

/*
================
GL_LoadPicTexture
================
*/
int GL_LoadPicTexture (qpic_t *pic)
{
	return GL_LoadTexture ("", pic->width, pic->height, pic->data, false, true, 0, false);
}

