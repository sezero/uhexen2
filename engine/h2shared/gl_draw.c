/*
 * gl_draw.c
 * this is the only file outside the refresh that touches the vid buffer
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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

#if ENDIAN_RUNTIME_DETECT
/* initialized by VID_Init() */
unsigned int	MASK_r;
unsigned int	MASK_g;
unsigned int	MASK_b;
unsigned int	MASK_a;
unsigned int	MASK_rgb;
unsigned int	SHIFT_r;
unsigned int	SHIFT_g;
unsigned int	SHIFT_b;
unsigned int	SHIFT_a;
#endif

qboolean	draw_reinit = false;

static cvar_t	gl_picmip = {"gl_picmip", "0", CVAR_NONE};
static cvar_t	gl_constretch = {"gl_constretch", "0", CVAR_ARCHIVE};
static cvar_t	gl_texturemode = {"gl_texturemode", "", CVAR_ARCHIVE};
static cvar_t	gl_texture_anisotropy = {"gl_texture_anisotropy", "1", CVAR_ARCHIVE};

static GLuint		menuplyr_textures[MAX_PLAYER_CLASS];	// player textures in multiplayer config screens
static GLuint		draw_backtile;
static GLuint		conback;
static GLuint		char_texture;
static GLuint		cs_texture;	// crosshair texture
static GLuint		char_smalltexture;
static GLuint		char_menufonttexture;

// Crosshair texture is a 32x32 alpha map with 8 levels of alpha.
// The format is similar to an X11 pixmap, but not the same.
// 7 is 100% solid, 0 and any other characters are transparent.
static const char	*cs_data = {
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

int		gl_filter_idx = 4; /* Bilinear */

gltexture_t	gltextures[MAX_GLTEXTURES];
int			numgltextures;

static GLuint GL_LoadPixmap (const char *name, const char *data);
static void GL_Upload32 (unsigned int *data, gltexture_t *glt);
static void GL_Upload8 (byte *data, gltexture_t *glt);


//=============================================================================
/* Support Routines */

cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;

/*
 * Geometry for the player/skin selection screen image.
 */
#define	PLAYER_PIC_WIDTH	68
#define	PLAYER_PIC_HEIGHT	114
#define	PLAYER_DEST_WIDTH	128
#define	PLAYER_DEST_HEIGHT	128

static byte	menuplyr_pixels[MAX_PLAYER_CLASS][PLAYER_PIC_WIDTH*PLAYER_PIC_HEIGHT];


static void Draw_PicCheckError (void *ptr, const char *name)
{
	if (!ptr)
		Sys_Error ("Failed to load %s", name);
}


qpic_t *Draw_PicFromFile (const char *name)
{
	qpic_t	*p;
	glpic_t	gl;

	p = (qpic_t *)FS_LoadHunkFile (name, NULL);
	if (!p)
		return NULL;

	SwapPic (p);

	gl.texnum = GL_LoadPicTexture (p);
	gl.sl = 0;
	gl.sh = 1;
	gl.tl = 0;
	gl.th = 1;
	memcpy (p->data, &gl, sizeof(glpic_t));

	return p;
}

qpic_t *Draw_PicFromWad (const char *name)
{
	qpic_t	*p;
	glpic_t	gl;

	p = (qpic_t *) W_GetLumpName (name);

	gl.texnum = GL_LoadPicTexture (p);
	gl.sl = 0;
	gl.sh = 1;
	gl.tl = 0;
	gl.th = 1;
	memcpy (p->data, &gl, sizeof(glpic_t));

	return p;
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
	glpic_t		gl;

	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
	{
		if (!strcmp (path, pic->name))
			return &pic->pic;
	}

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
	menu_numcachepics++;
	q_strlcpy (pic->name, path, MAX_QPATH);

//
// load the pic from disk
//
	dat = (qpic_t *)FS_LoadTempFile (path, NULL);
	Draw_PicCheckError (dat, path);
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
	else if (!strcmp (path, "gfx/menu/netp5.lmp"))
		memcpy (menuplyr_pixels[4], dat->data, dat->width*dat->height);
#if defined (H2W)
	else if (!strcmp (path, "gfx/menu/netp6.lmp"))
		memcpy (menuplyr_pixels[5], dat->data, dat->width*dat->height);
#endif

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gl.texnum = GL_LoadPicTexture (dat);
	gl.sl = 0;
	gl.sh = 1;
	gl.tl = 0;
	gl.th = 1;
	memcpy (pic->pic.data, &gl, sizeof(glpic_t));

	return &pic->pic;
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
	glpic_t		gl;

	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
	{
		if (!strcmp (ls_path, pic->name))
			return &pic->pic;
	}

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");

	dat = (qpic_t *)FS_LoadTempFile (ls_path, NULL);
	Draw_PicCheckError (dat, ls_path);
	SwapPic (dat);
	if (fs_filesize != 17592 || dat->width != 157 || dat->height != 112)
		return Draw_CachePic(ls_path);

	q_strlcpy (pic->name, ls_path, MAX_QPATH);
	menu_numcachepics++;

	/* kill the progress slot pixels between rows [85:103] */
	memmove(dat->data + 157*85, dat->data + 157*104, 157*(112 - 104));
	dat->height -= (104 - 85);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gl.texnum = GL_LoadPicTexture (dat);
	gl.sl = 0;
	gl.sh = 1;
	gl.tl = 0;
	gl.th = 1;
	memcpy (pic->pic.data, &gl, sizeof(glpic_t));

	return &pic->pic;
}
#endif	/* !DRAW_PROGRESSBARS */

glmode_t gl_texmodes[NUM_GL_FILTERS] =
{
	{ "GL_NEAREST",			GL_NEAREST,			GL_NEAREST },	/* point sampled	*/
	{ "GL_NEAREST_MIPMAP_NEAREST",	GL_NEAREST_MIPMAP_NEAREST,	GL_NEAREST },	/* nearest, 1 mipmap	*/
	{ "GL_NEAREST_MIPMAP_LINEAR",	GL_NEAREST_MIPMAP_LINEAR,	GL_NEAREST },	/* nearest, 2 mipmaps	*/
	{ "GL_LINEAR",			GL_LINEAR,			GL_LINEAR  },	/* Bilinear, no mipmaps	*/
	{ "GL_LINEAR_MIPMAP_NEAREST",	GL_LINEAR_MIPMAP_NEAREST,	GL_LINEAR  },	/* Bilinear, 1 mipmap	*/
	{ "GL_LINEAR_MIPMAP_LINEAR",	GL_LINEAR_MIPMAP_LINEAR,	GL_LINEAR  }	/* Trilinear: 2 mipmaps	*/
};

/*
===============
Draw_TextureMode_f
===============
*/
static void Draw_TouchAllFilterModes (void)
{
	gltexture_t	*glt;
	int	i;

	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
	{
		if (glt->flags & (TEX_NEAREST|TEX_LINEAR))	/* TEX_MIPMAP mustn't be set in this case */
			continue;
		GL_Bind (glt->texnum);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_texmodes[gl_filter_idx].maximize);
		if (glt->flags & TEX_MIPMAP)
		{
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texmodes[gl_filter_idx].minimize);
			if (gl_max_anisotropy >= 2)
				glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_texture_anisotropy.value);
		}
		else	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texmodes[gl_filter_idx].maximize);
	}
}

static void Draw_TextureMode_f (cvar_t *var)
{
	int	i;

	for (i = 0; i < NUM_GL_FILTERS; i++)
	{
		if (!strcmp (gl_texmodes[i].name, var->string))
		{
			if (gl_filter_idx != i)
			{
				gl_filter_idx = i;
				// change all the existing mipmap texture objects
				Draw_TouchAllFilterModes ();
			}
			return;
		}
	}

	for (i = 0; i < NUM_GL_FILTERS; i++)
	{
		if (!q_strcasecmp (gl_texmodes[i].name, var->string))
		{
			Cvar_SetQuick (var, gl_texmodes[i].name);
			return;
		}
	}

	Con_Printf ("bad filter name\n");
	Cvar_SetQuick (var, gl_texmodes[gl_filter_idx].name);
}

static void Draw_TouchMipmapFilterModes (void)
{
	gltexture_t	*glt;
	int	i;

	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
	{
		if (glt->flags & TEX_MIPMAP)
		{
			GL_Bind (glt->texnum);
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_texmodes[gl_filter_idx].maximize);
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texmodes[gl_filter_idx].minimize);
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_texture_anisotropy.value);
		}
	}
}

static void Draw_Anisotropy_f (cvar_t *var)
{
	if (var->value < 1)
	{
		Cvar_SetQuick (var, "1");
	}
	else if (var->value > gl_max_anisotropy)
	{
		Cvar_SetValueQuick (var, gl_max_anisotropy);
	}
	else
	{
		if (gl_max_anisotropy >= 2)
			Draw_TouchMipmapFilterModes ();
	}
}

/*
===============
Draw_ClearAllModels
Callback for Draw_ReInit() and Draw_ReinitTextures():
Clear all models along with their textures.
===============
*/
static void Draw_ClearAllModels (void)
{
	int	temp = gl_purge_maptex.integer;
	flush_textures = true;
	Cvar_Set ("gl_purge_maptex", "1");
	Mod_ClearAll ();
	Cvar_SetValue ("gl_purge_maptex", temp);
}

#if 0
/*
===============
Draw_ReinitTextures
Delete and reload all textures
===============
*/
static void Draw_ReinitTextures (void)
{
	int	temp;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	draw_reinit = true;

	D_ClearOpenGLTextures(0);
	memset (lightmap_textures, 0, sizeof(lightmap_textures));
	// make sure all of alias models are cleared
	if (cls.state != ca_active)
		Draw_ClearAllModels ();

	// Reload pre-map pics, fonts, console, etc
	W_LoadWadFile ("gfx.wad");
	Draw_Init();
	SCR_Init();
	Sbar_Init();
	// Reload the particle texture
	R_InitParticleTexture();
	R_InitExtraTextures ();
#ifdef H2W
	R_InitNetgraphTexture();
#endif

	// Reload the map's textures
	if (cls.state == ca_active)
	{
		Mod_ReloadTextures();
		GL_BuildLightmaps();
	}

	draw_reinit = false;
	scr_disabled_for_loading = temp;
}
#endif

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

	D_ClearOpenGLTextures(0);
	memset (lightmap_textures, 0, sizeof(lightmap_textures));
	// make sure all of alias models are cleared
	Draw_ClearAllModels ();

	// Reload pre-map pics, fonts, console, etc
	W_LoadWadFile ("gfx.wad");
	Draw_Init();
	SCR_Init();
	Sbar_Init();
	// Reload the particle texture
	R_InitParticleTexture();
	R_InitExtraTextures ();
#ifdef H2W
	R_InitNetgraphTexture();
#endif

	draw_reinit = false;
	scr_disabled_for_loading = temp;
}

/*
===============
Draw_Init
===============
*/
void Draw_Init (void)
{
	qpic_t		*p;
	byte		*chars;
	int		i;

	if (!draw_reinit)
	{
		Cvar_RegisterVariable (&gl_picmip);
		Cvar_RegisterVariable (&gl_constretch);
		gl_texturemode.string = gl_texmodes[gl_filter_idx].name;
		Cvar_RegisterVariable (&gl_texturemode);
		Cvar_RegisterVariable (&gl_texture_anisotropy);
		Cvar_SetCallback (&gl_texturemode, Draw_TextureMode_f);
		Cvar_SetCallback (&gl_texture_anisotropy, Draw_Anisotropy_f);
	}

	// load the charset: 8*8 graphic characters
	chars = FS_LoadTempFile ("gfx/menu/conchars.lmp", NULL);
	Draw_PicCheckError (chars, "gfx/menu/conchars.lmp");
	for (i = 0; i < 256*128; i++)
	{
		if (chars[i] == 0)
			chars[i] = 255;	// proper transparent color
	}
	char_texture = GL_LoadTexture ("charset", chars, 256, 128, TEX_ALPHA|TEX_NEAREST);

	// load the small characters for status bar
	chars = (byte *) W_GetLumpName("tinyfont");
	for (i = 0; i < 128*32; i++)
	{
		if (chars[i] == 0)
			chars[i] = 255;	// proper transparent color
	}
	char_smalltexture = GL_LoadTexture ("smallcharset", chars, 128, 32, TEX_ALPHA|TEX_NEAREST);

	// load the big menu font
	// Note: old version of demo has bigfont.lmp, not bigfont2.lmp
	p = (qpic_t *)FS_LoadTempFile("gfx/menu/bigfont2.lmp", NULL);
	if (!p) p = (qpic_t *)FS_LoadTempFile("gfx/menu/bigfont.lmp", NULL);
	Draw_PicCheckError (p, "gfx/menu/bigfont2.lmp");
	SwapPic (p);
	for (i = 0; i < p->width * p->height; i++)	// MUST be 160 * 80
	{
		if (p->data[i] == 0)
			p->data[i] = 255;	// proper transparent color
	}
	char_menufonttexture = GL_LoadTexture ("menufont", p->data, p->width, p->height, TEX_ALPHA|TEX_LINEAR);

	// load the console background
	p = (qpic_t *)FS_LoadTempFile ("gfx/menu/conback.lmp", NULL);
	Draw_PicCheckError (p, "gfx/menu/conback.lmp");
	SwapPic (p);
	conback = GL_LoadTexture ("conback", p->data, p->width, p->height, TEX_LINEAR);

	// load the backtile
	p = (qpic_t *)FS_LoadTempFile ("gfx/menu/backtile.lmp", NULL);
	Draw_PicCheckError (p, "gfx/menu/backtile.lmp");
	SwapPic (p);
	draw_backtile = GL_LoadPicTexture (p);

	// load the crosshair texture
	cs_texture = GL_LoadPixmap ("crosshair", cs_data);

	// initialize the player texnums for multiplayer config screens
	glGenTextures_fp(MAX_PLAYER_CLASS, menuplyr_textures);
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
	int		row, col;
	float	frow, fcol, xsize,ysize;

	num &= 511;

	if (num == 32)
		return;		// space

	if (y <= -8)
		return;		// totally off screen

	row = num >> 5;
	col = num & 31;

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

/*
================
Draw_Crosshair
================
*/
void Draw_Crosshair (void)
{
	int		x, y;
	unsigned char	*pColor;

	x = scr_vrect.x + scr_vrect.width/2 + cl_crossx.value;
	y = scr_vrect.y + scr_vrect.height/2 + cl_crossy.value;

	if (crosshair.integer == 2)
	{
		pColor = (unsigned char *) &d_8to24table[(byte) crosshaircolor.integer];

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
	int		row, col;
	float	frow, fcol, xsize,ysize;

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

	if (num == 0)
		return;

	if (y <= -8 || y >= vid.height)
		return; 	// totally off screen

	row = num >> 4;
	col = num & 15;

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
void Draw_SmallString (int x, int y, const char *str)
{
	while (*str)
	{
		Draw_SmallCharacter (x, y, *str);
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
	int		row, col;
	float	frow, fcol, xsize, ysize;

	row = num / 8;
	col = num % 8;

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
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	glpic_t			*gl;

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

	gl = (glpic_t *)pic->data;
	glDisable_fp(GL_ALPHA_TEST);
	glEnable_fp (GL_BLEND);
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

#if FULLSCREEN_INTERMISSIONS
/*
================
Draw_CachePicNoTrans

Pa3PyX: Function added to cache pics ignoring transparent
colors (e.g. in intermission screens)
================
*/
qpic_t *Draw_CachePicNoTrans (const char *path)
{
	cachepic_t	*pic;
	int		i;
	qpic_t		*dat;
	glpic_t		gl;

	for (pic = menu_cachepics, i = 0; i < menu_numcachepics; pic++, i++)
	{
		if (!strcmp (path, pic->name))
			return &pic->pic;
	}

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
	menu_numcachepics++;
	q_strlcpy (pic->name, path, MAX_QPATH);

//
// load the pic from disk
//
	dat = (qpic_t *)FS_LoadTempFile (path, NULL);
	Draw_PicCheckError (dat, path);
	SwapPic (dat);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	// Get rid of transparencies
	for (i = 0; i < dat->width * dat->height; i++)
	{
		if (dat->data[i] == 255)
			dat->data[i] = 31; // pal(31) == pal(255) == FCFCFC (white)
	}
	gl.texnum = GL_LoadPicTexture (dat);

	gl.sl = 0;
	gl.sh = 1;
	gl.tl = 0;
	gl.th = 1;
	memcpy (pic->pic.data, &gl, sizeof(glpic_t));

	return &pic->pic;
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
#endif	/* FULLSCREEN_INTERMISSIONS */


void Draw_SubPic (int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height)
{
	glpic_t			*gl;
	float	newsl, newtl, newsh, newth;
	float	oldglwidth, oldglheight;

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

void Draw_PicCropped (int x, int y, qpic_t *pic)
{
	int		height;
	glpic_t 	*gl;
	float		th, tl;

	if ((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("%s: bad coordinates", __thisfunc__);
	}

	if (y >= vid.height || y+pic->height < 0)
		return;		// totally off screen

	gl = (glpic_t *)pic->data;

	// rjr	tl/th need to be computed based upon pic->tl and pic->th
	//	cuz the piece may come from the scrap
	if (y+pic->height > vid.height)
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

void Draw_SubPicCropped (int x, int y, int h, qpic_t *pic)
{
	int		height;
	glpic_t 	*gl;
	float		th,tl;

	if ((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("%s: bad coordinates", __thisfunc__);
	}

	if (y >= vid.height || y+h < 0)
		return;		// totally off screen

	gl = (glpic_t *)pic->data;

	// rjr	tl/th need to be computed based upon pic->tl and pic->th
	//	cuz the piece may come from the scrap
	if (y+pic->height > vid.height)
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
	if (x < 0 || (x + pic->width) > vid.width ||
	    y < 0 || (y + pic->height) > vid.height)
	{
		Sys_Error ("%s: bad coordinates", __thisfunc__);
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
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation, int p_class)
{
	int		i, j, v, u;
	unsigned int	trans[PLAYER_DEST_WIDTH * PLAYER_DEST_HEIGHT], *dest;
	byte		*src;
	int		p;

	GL_Bind(menuplyr_textures[p_class-1]);
	dest = trans;
	for (v = 0; v < 64; v++, dest += 64)
	{
		src = &menuplyr_pixels[p_class-1][((v*pic->height)>>6) * pic->width];
		for (u = 0; u < 64; u++)
		{
			p = src[(u*pic->width)>>6];
			dest[u] = (p == 255) ? 255 : d_8to24table[translation[p]];
		}
	}

	for (i = 0; i < PLAYER_PIC_WIDTH; i++)
	{
		for (j = 0; j < PLAYER_PIC_HEIGHT; j++)
		{
			trans[j * PLAYER_DEST_WIDTH + i] =
			 d_8to24table[translation[menuplyr_pixels[p_class-1][j * PLAYER_PIC_WIDTH + i]]];
		}
	}

	glTexImage2D_fp (GL_TEXTURE_2D, 0, gl_alpha_format, PLAYER_DEST_WIDTH, PLAYER_DEST_HEIGHT,
			 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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


/*
================
Draw_ConsoleBackground

================
*/
static void Draw_ConsolePic (int lines, float ofs, GLuint num, float alpha)
{
	glDisable_fp(GL_ALPHA_TEST);
	glEnable_fp (GL_BLEND);
	glCullFace_fp(GL_FRONT);
	glColor4f_fp (1,1,1,alpha);
	GL_Bind (num);

	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (0, 0 + ofs);
	glVertex2f_fp (0, 0);
	glTexCoord2f_fp (1, 0 + ofs);
	glVertex2f_fp (vid.conwidth, 0);
	glTexCoord2f_fp (1, 1);
	glVertex2f_fp (vid.conwidth, lines);
	glTexCoord2f_fp (0, 1);
	glVertex2f_fp (0, lines);
	glEnd_fp ();

	glColor4f_fp (1,1,1,1);
	glEnable_fp(GL_ALPHA_TEST);
	glDisable_fp (GL_BLEND);
}

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
	int	y;
	float	ofs, alpha;

	y = (vid.height * 3) >> 2;
	ofs = (gl_constretch.integer) ? 0.0f : (vid.conheight - lines) / (float) vid.conheight;
	alpha = (lines > y) ? 1.0f : 1.1f * lines / y;

	Draw_ConsolePic (lines, ofs, conback, alpha);

#if defined(H2W)
	if (cls.download)
		return;
#endif	/* H2W */

	Draw_ConsoleVersionInfo (lines);
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
	GL_Bind (draw_backtile);
	glBegin_fp (GL_QUADS);
	glTexCoord2f_fp (x/64.0, y/64.0);
	glVertex2f_fp (x, y);
	glTexCoord2f_fp ( (x+w)/64.0, y/64.0);
	glVertex2f_fp (x+w, y);
	glTexCoord2f_fp ( (x+w)/64.0, (y+h)/64.0);
	glVertex2f_fp (x+w, y+h);
	glTexCoord2f_fp (x/64.0, (y+h)/64.0);
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
	int		bx, by, ex, ey;
	int		c;

	glAlphaFunc_fp(GL_ALWAYS, 0);

	glEnable_fp (GL_BLEND);
	glDisable_fp (GL_TEXTURE_2D);

	glColor4f_fp (248.0/255.0, 220.0/255.0, 120.0/255.0, 0.1);
	glBegin_fp (GL_QUADS);

	glVertex2f_fp (0,0);
	glVertex2f_fp (vid.width, 0);
	glVertex2f_fp (vid.width, vid.height);
	glVertex2f_fp (0, vid.height);
	glEnd_fp ();

	glColor4f_fp (248.0/255.0, 220.0/255.0, 120.0/255.0, 0.018);

	for (c = 0 ; c < 40 ; c++)
	{
		bx = (rand() % vid.width) - 20;
		by = (rand() % vid.height) - 20;
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
#if 0	/* seems to have no users */
int GL_FindTexture (const char *identifier)
{
	int		i;
	gltexture_t	*glt;

	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
	{
		if (!strcmp (identifier, glt->identifier))
			return gltextures[i].texnum;
	}

	return -1;
}
#endif

/*
================
GL_ResampleTexture
================
*/
static void GL_ResampleTexture (unsigned int *in, int inwidth, int inheight, unsigned int *out, int outwidth, int outheight)
{
	int		i, j, mark;
	unsigned int	*inrow, *inrow2;
	unsigned int	frac, fracstep;
	unsigned int	*p1, *p2;
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth * 0x10000 / outwidth;

	mark = Hunk_LowMark();
	p1 = (unsigned int *) Hunk_Alloc (outwidth * sizeof(unsigned int));
	p2 = (unsigned int *) Hunk_Alloc (outwidth * sizeof(unsigned int));

	frac = fracstep >> 2;
	for (i = 0; i < outwidth; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3 * (fracstep >> 2);
	for (i = 0; i < outwidth; i++)
	{
		p2[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	for (i = 0; i < outheight; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);

		frac = fracstep >> 1;
		for (j = 0 ; j < outwidth; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}

	Hunk_FreeToLowMark(mark);
}

/*
================
GL_MipMap

Quarters the size of the texture
May operate in-place.
This version is from Darkplaces.
================
*/
static void GL_MipMap (const byte *in, byte *out, int *width, int *height, int destwidth, int destheight)
{
	const byte *inrow;
	int x, y, nextrow;

	// if given odd width/height this discards the last row/column
	// of pixels, rather than doing a proper box-filter scale down
	inrow = in;
	nextrow = *width * 4;
	if (*width > destwidth)
	{
		*width >>= 1;
		if (*height > destheight)
		{
			// reduce both
			*height >>= 1;
			for (y = 0; y < *height; y++, inrow += nextrow * 2)
			{
				for (in = inrow, x = 0; x < *width; x++)
				{
					out[0] = (byte) ((in[0] + in[4] + in[nextrow  ] + in[nextrow+4]) >> 2);
					out[1] = (byte) ((in[1] + in[5] + in[nextrow+1] + in[nextrow+5]) >> 2);
					out[2] = (byte) ((in[2] + in[6] + in[nextrow+2] + in[nextrow+6]) >> 2);
					out[3] = (byte) ((in[3] + in[7] + in[nextrow+3] + in[nextrow+7]) >> 2);
					out += 4;
					in += 8;
				}
			}
		}
		else
		{
			// reduce width
			for (y = 0; y < *height; y++, inrow += nextrow)
			{
				for (in = inrow, x = 0; x < *width; x++)
				{
					out[0] = (byte) ((in[0] + in[4]) >> 1);
					out[1] = (byte) ((in[1] + in[5]) >> 1);
					out[2] = (byte) ((in[2] + in[6]) >> 1);
					out[3] = (byte) ((in[3] + in[7]) >> 1);
					out += 4;
					in += 8;
				}
			}
		}
	}
	else
	{
		if (*height > destheight)
		{
			// reduce height
			*height >>= 1;
			for (y = 0; y < *height; y++, inrow += nextrow * 2)
			{
				for (in = inrow, x = 0; x < *width; x++)
				{
					out[0] = (byte) ((in[0] + in[nextrow  ]) >> 1);
					out[1] = (byte) ((in[1] + in[nextrow+1]) >> 1);
					out[2] = (byte) ((in[2] + in[nextrow+2]) >> 1);
					out[3] = (byte) ((in[3] + in[nextrow+3]) >> 1);
					out += 4;
					in += 4;
				}
			}
		}
	}
}

/*
================
fxPalTexImage2D

Acts the same as glTexImage2D, except that it maps color
into the current palette and uses paletteized textures.
If you are on a 3Dfx card and your texture has no alpha,
then download it as a palettized texture to save memory.

fxpal_buf is a pointer to hunk allocated temporary buffer.
The callers of fxPalTexImage2D() are responsible for the
allocation and freeing of fxpal_buf. According to Pa3PyX,
fxpal_buf must remain static until all mipmap reductions
are uploaded, otherwise garbage results with 3dfx.
================
*/
static unsigned char	*fxpal_buf;

static void fxPalTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	GLsizei		i;

	// we don't want textures with alpha
	if (internalformat != 3)
		Sys_Error ("%s: internalformat != 3", __thisfunc__);

	for (i = 0; i < width * height; i++)
	{
		int	r, g, b, idx;
		r = ((unsigned char *)pixels)[i * 4];
		g = ((unsigned char *)pixels)[i * 4 +1];
		b = ((unsigned char *)pixels)[i * 4 +2];
		r >>= 8 - INVERSE_PAL_R_BITS;
		g >>= 8 - INVERSE_PAL_G_BITS;
		b >>= 8 - INVERSE_PAL_B_BITS;
		idx = (r << (INVERSE_PAL_G_BITS + INVERSE_PAL_B_BITS)) | (g << INVERSE_PAL_B_BITS) | b;
		fxpal_buf[i] = inverse_pal[idx];
	}

	glTexImage2D_fp(target, level, GL_COLOR_INDEX8_EXT, width, height, border, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, fxpal_buf);
}

/*
===============
GL_Upload32
===============
*/
static void GL_Upload32 (unsigned int *data, gltexture_t *glt)
{
	int		samples;
	unsigned int	*scaled;
	int		mark = 0;
	int		scaled_width, scaled_height;

	if (gl_tex_NPOT && !is8bit)
	{
		scaled_width = glt->width >> gl_picmip.integer;
		scaled_height = glt->height >> gl_picmip.integer;
	}
	else
	{
	// Snap the height and width to a power of 2.
		for (scaled_width = 1; scaled_width < glt->width; scaled_width <<= 1)
			;
		for (scaled_height = 1; scaled_height < glt->height; scaled_height <<= 1)
			;
		scaled_width >>= gl_picmip.integer;
		scaled_height >>= gl_picmip.integer;
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
	if (is_3dfx)
	{
		if (scaled_width * 8 < scaled_height)
		{
			scaled_width = scaled_height >> 3;
		}
		else if (scaled_height * 8 < scaled_width)
		{
			scaled_height = scaled_width >> 3;
		}
	}

	samples = (glt->flags & TEX_ALPHA) ? gl_alpha_format : gl_solid_format;

	if (scaled_width == glt->width && scaled_height == glt->height)
	{
		scaled = data;
	}
	else
	{
		mark = Hunk_LowMark();
		scaled = (unsigned int *) Hunk_AllocName(scaled_width * scaled_height * sizeof(unsigned int), "texbuf_upload32");
		GL_ResampleTexture (data, glt->width, glt->height, scaled, scaled_width, scaled_height);
	}

	if (is8bit && !(glt->flags & TEX_ALPHA))
	{
		if (!mark)
			mark = Hunk_LowMark();
		fxpal_buf = (unsigned char *) Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");
		fxPalTexImage2D (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
	else
	{
		glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}

	if (glt->flags & TEX_MIPMAP)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			GL_MipMap ((byte *)scaled, (byte *)scaled, &scaled_width, &scaled_height, 1, 1);
			miplevel++;
			if (is8bit && !(glt->flags & TEX_ALPHA))
				fxPalTexImage2D (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
			else	glTexImage2D_fp (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		}
	}

	if (glt->flags & TEX_NEAREST)
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if (glt->flags & TEX_LINEAR)
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else if (glt->flags & TEX_MIPMAP)
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texmodes[gl_filter_idx].minimize);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_texmodes[gl_filter_idx].maximize);
		if (gl_max_anisotropy >= 2)
			glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_texture_anisotropy.value);
	}
	else
	{
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texmodes[gl_filter_idx].maximize);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_texmodes[gl_filter_idx].maximize);
	}

	if (mark)
		Hunk_FreeToLowMark(mark);
}

/*
===============
GL_Upload8

modes:
0 - standard
1 - color 0 transparent, odd - translucent, even - full value
2 - color 0 transparent
3 - special (particle translucency table)
===============
*/
static void GL_Upload8 (byte *data, gltexture_t *glt)
{
	unsigned int		*trans;
	int			mark;
	int			i, p, s;

	s = glt->width * glt->height;
	mark = Hunk_LowMark();
	trans = (unsigned int *) Hunk_AllocName(s * sizeof(unsigned int), "texbuf_upload8");

	if (glt->flags & (TEX_ALPHA|TEX_TRANSPARENT|TEX_HOLEY|TEX_SPECIAL_TRANS))
	{
		// if there are no transparent pixels, make it a 3 component
		// texture even if it was flagged as TEX_ALPHA.
		qboolean noalpha = !(glt->flags & (TEX_TRANSPARENT|TEX_HOLEY|TEX_SPECIAL_TRANS));

		for (i = 0; i < s; i++)
		{
			p = data[i];
			trans[i] = d_8to24table[p];

			if (p == 255)
			{
				if (noalpha)
					noalpha = false;

				/* transparent, so scan around for another color
				 * to avoid alpha fringes */
				/* this is a replacement from Quake II for Raven's
				 * "neighboring colors" code */
				if (i > glt->width && data[i-glt->width] != 255)
					p = data[i-glt->width];
				else if (i < s-glt->width && data[i+glt->width] != 255)
					p = data[i+glt->width];
				else if (i > 0 && data[i-1] != 255)
					p = data[i-1];
				else if (i < s-1 && data[i+1] != 255)
					p = data[i+1];
				else
					p = 0;
				/* copy rgb components */
				((byte *)&trans[i])[0] = ((byte *)&d_8to24table[p])[0];
				((byte *)&trans[i])[1] = ((byte *)&d_8to24table[p])[1];
				((byte *)&trans[i])[2] = ((byte *)&d_8to24table[p])[2];
			}

			if (glt->flags & TEX_TRANSPARENT)
			{
				p = data[i];
				if (p == 0)
				{
					trans[i] &= MASK_rgb;
				}
				else if (p & 1)
				{
					p = (int)(255 * r_wateralpha.value) & 0xff;
					trans[i] &= MASK_rgb;
					trans[i] |= p << SHIFT_a;
				}
				else
				{
					trans[i] |= MASK_a;
				}
			}
			else if (glt->flags & TEX_HOLEY)
			{
				p = data[i];
				if (p == 0)
					trans[i] &= MASK_rgb;
			}
			else if (glt->flags & TEX_SPECIAL_TRANS)
			{
				p = data[i];
				trans[i] = d_8to24table[ColorIndex[p>>4]] & MASK_rgb;
				trans[i] |= (( int )ColorPercent[p&15] & 0xff) << SHIFT_a;
			}
		}

		if (noalpha)
			glt->flags &= ~TEX_ALPHA;
		if (glt->flags & (TEX_TRANSPARENT|TEX_HOLEY|TEX_SPECIAL_TRANS))
			glt->flags |= TEX_ALPHA;
	}
	else
	{
		if (s&3)
			Sys_Error ("%s: s&3", __thisfunc__);
		for (i = 0; i < s; i += 4)
		{
			trans[i] = d_8to24table[data[i]];
			trans[i+1] = d_8to24table[data[i+1]];
			trans[i+2] = d_8to24table[data[i+2]];
			trans[i+3] = d_8to24table[data[i+3]];
		}
	}

	GL_Upload32 (trans, glt);
	Hunk_FreeToLowMark(mark);
}


/*
================
GL_LoadTexture
================
*/
GLuint GL_LoadTexture (const char *identifier, byte *data, int width, int height, int flags)
{
	int		i, size;
	unsigned short	crc;
	gltexture_t	*glt;

#if !defined (H2W)
	if (cls.state == ca_dedicated)
		return GL_UNUSED_TEXTURE;
#endif

	size = width * height;
	if (flags & TEX_RGBA)
		size *= 4;
	crc = CRC_Block (data, size);

	if (identifier[0])
	{
		/* texture already present? */
		for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
		{
			if (!strcmp (identifier, glt->identifier))
			{
				if (crc != glt->crc ||
				    (glt->flags & TEX_MIPMAP) != (flags & TEX_MIPMAP) ||
				    width  != glt->width || height != glt->height)
				{ /* not the same, delete and rebind to new image */
					Con_DPrintf ("Texture cache mismatch: %lu, %s, reloading\n",
							    (unsigned long)glt->texnum, identifier);
					glDeleteTextures_fp (1, &glt->texnum);
					goto gl_rebind;
				}
				else	return glt->texnum;	/* the same is present. */
			}
		}
	}

	if (numgltextures >= MAX_GLTEXTURES)
		Sys_Error ("%s: cache full, max is %i textures.", __thisfunc__, MAX_GLTEXTURES);

	glt = &gltextures[numgltextures];
	numgltextures++;
	q_strlcpy (glt->identifier, identifier, MAX_QPATH);

gl_rebind:
	glGenTextures_fp(1, &glt->texnum);
	glt->width = width;
	glt->height = height;
	glt->flags = flags;
	glt->crc = crc;

	GL_Bind (glt->texnum);
	if (flags & TEX_RGBA)
		GL_Upload32 ((unsigned int *)data, glt);
	else	GL_Upload8 (data, glt);

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
static GLuint GL_LoadPixmap (const char *name, const char *data)
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
		}
		else
		{
			pixels[i][0] = 255;
			pixels[i][1] = 255;
			pixels[i][2] = 255;
			pixels[i][3] = 0;
		}
	}

	return GL_LoadTexture (name, (unsigned char *) pixels, 32, 32, TEX_ALPHA | TEX_RGBA | TEX_LINEAR);
}

/*
================
GL_LoadPicTexture
================
*/
GLuint GL_LoadPicTexture (qpic_t *pic)
{
	return GL_LoadTexture ("", pic->data, pic->width, pic->height, TEX_ALPHA|TEX_LINEAR);
}

