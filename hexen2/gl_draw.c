/*
	gl_draw.c
	this is the only file outside the refresh that touches the vid buffer

	$Id: gl_draw.c,v 1.125 2007-09-28 14:30:51 sezero Exp $
*/

#include "quakedef.h"

#if BYTE_ORDER == BIG_ENDIAN
#define	MASK_rgb	0xffffff00
#define	MASK_a		0x000000ff
#define	MASK_b		0x0000ff00
#define	MASK_g		0x00ff0000
#define	MASK_r		0xff000000
#define	SHIFT_a		0
#define	SHIFT_b		8
#define	SHIFT_g		16
#define	SHIFT_r		24
#else
#define	MASK_rgb	0x00ffffff
#define	MASK_a		0xff000000
#define	MASK_b		0x00ff0000
#define	MASK_g		0x0000ff00
#define	MASK_r		0x000000ff
#define	SHIFT_b		0
#define	SHIFT_g		8
#define	SHIFT_r		16
#define	SHIFT_a		24
#endif

qboolean	draw_reinit = false;

static cvar_t	gl_picmip = {"gl_picmip", "0", CVAR_NONE};
static cvar_t	gl_constretch = {"gl_constretch", "0", CVAR_ARCHIVE};

GLuint			plyrtex[MAX_PLAYER_CLASS][16][16];	// player textures in multiplayer config screens
static GLuint		draw_backtile;
static GLuint		conback;
static GLuint		char_texture;
static GLuint		cs_texture;	// crosshair texture
static GLuint		char_smalltexture;
static GLuint		char_menufonttexture;

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

int		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gl_filter_max = GL_LINEAR;

gltexture_t	gltextures[MAX_GLTEXTURES];
int			numgltextures;

static GLuint GL_LoadPixmap (const char *name, const char *data);
static void GL_Upload32 (unsigned int *data, int width, int height, qboolean mipmap, qboolean alpha);
static void GL_Upload8 (byte *data, int width, int height, qboolean mipmap, qboolean alpha, int mode);


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
	glpic_t	*gl;

	p = (qpic_t *)FS_LoadHunkFile (name);
	if (!p)
	{
		return NULL;
	}

	SwapPic (p);
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
qpic_t *Draw_PicFileBuf (const char *name, void *p, size_t *size)
{
	glpic_t	*gl;

	p = (void *)FS_LoadBufFile(name, p, size);
	if (!p)
		return NULL;

	SwapPic ((qpic_t *)p);
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

	p = (qpic_t *) W_GetLumpName (name);
	gl = (glpic_t *)p->data;

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
qpic_t	*Draw_CachePic (const char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	glpic_t		*gl;

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
	dat = (qpic_t *)FS_LoadTempFile (path);
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
qpic_t *Draw_CachePicNoTrans (const char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	glpic_t		*gl;

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
	dat = (qpic_t *)FS_LoadTempFile (path);
	Draw_PicCheckError (dat, path);
	SwapPic (dat);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gl = (glpic_t *)pic->pic.data;
	// Get rid of transparencies
	for (i = 0; i < dat->width * dat->height; i++)
	{
		if (dat->data[i] == 255)
			dat->data[i] = 31; // pal(31) == pal(255) == FCFCFC (white)
	}
	gl->texnum = GL_LoadPicTexture (dat);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return &pic->pic;
}


glmode_t gl_texmodes[NUM_GL_FILTERS] =
{
	{ "GL_NEAREST",			GL_NEAREST,			GL_NEAREST },	/* point sampled	*/
	{ "GL_LINEAR",			GL_LINEAR,			GL_LINEAR  },	/* Bilinear, no mipmaps	*/
	{ "GL_NEAREST_MIPMAP_NEAREST",	GL_NEAREST_MIPMAP_NEAREST,	GL_NEAREST },	/* nearest, 1 mipmap	*/
	{ "GL_NEAREST_MIPMAP_LINEAR",	GL_NEAREST_MIPMAP_LINEAR,	GL_NEAREST },	/* nearest, 2 mipmaps	*/
	{ "GL_LINEAR_MIPMAP_NEAREST",	GL_LINEAR_MIPMAP_NEAREST,	GL_LINEAR  },	/* Bilinear, 1 mipmap	*/
	{ "GL_LINEAR_MIPMAP_LINEAR",	GL_LINEAR_MIPMAP_LINEAR,	GL_LINEAR  }	/* Trilinear: 2 mipmaps	*/
};

/*
===============
Draw_TextureMode_f
===============
*/
static void Draw_TextureMode_f (void)
{
	unsigned int		i;
	gltexture_t	*glt;

	if (Cmd_Argc() == 1)
	{
		for (i = 0; i < NUM_GL_FILTERS; i++)
		{
			if (gl_filter_min == gl_texmodes[i].minimize)
			{
				Con_Printf ("%s\n", gl_texmodes[i].name);
				return;
			}
		}
		Con_Printf ("current filter is unknown???\n");
		return;
	}

	for (i = 0; i < NUM_GL_FILTERS; i++)
	{
		if (!q_strcasecmp (gl_texmodes[i].name, Cmd_Argv(1) ) )
			break;
	}
	if (i == NUM_GL_FILTERS)
	{
		Con_Printf ("bad filter name\n");
		return;
	}

	gl_filter_min = gl_texmodes[i].minimize;
	gl_filter_max = gl_texmodes[i].maximize;

	// change all the existing mipmap texture objects
	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
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
	qpic_t		*p;
	byte		*chars;
	int		i;

	if (!draw_reinit)
	{
		Cvar_RegisterVariable (&gl_picmip);
		Cvar_RegisterVariable (&gl_constretch);

		Cmd_AddCommand ("gl_texturemode", Draw_TextureMode_f);

	// initialize the player texnums array for multiplayer config screens
		memset(plyrtex, 0, MAX_PLAYER_CLASS * 16 * 16 * sizeof(GLuint));
	}

	// load the charset: 8*8 graphic characters
	chars = FS_LoadTempFile ("gfx/menu/conchars.lmp");
	for (i = 0; i < 256*128; i++)
	{
		if (chars[i] == 0)
			chars[i] = 255;	// proper transparent color
	}
	char_texture = GL_LoadTexture ("charset", 256, 128, chars, false, true, 0, false);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	// load the small characters for status bar
	chars = (byte *) W_GetLumpName("tinyfont");
	for (i = 0; i < 128*32; i++)
	{
		if (chars[i] == 0)
			chars[i] = 255;	// proper transparent color
	}
	char_smalltexture = GL_LoadTexture ("smallcharset", 128, 32, chars, false, true, 0, false);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	// load the big menu font
	// Note: old version of demo has bigfont.lmp, not bigfont2.lmp
	// add a GAME_OLD_DEMO flag check ?
	p = (qpic_t *)FS_LoadTempFile("gfx/menu/bigfont2.lmp");
	Draw_PicCheckError (p, "gfx/menu/bigfont2.lmp");
	SwapPic (p);
	for (i = 0; i < p->width * p->height; i++)	// MUST be 160 * 80
	{
		if (p->data[i] == 0)
			p->data[i] = 255;	// proper transparent color
	}
	char_menufonttexture = GL_LoadTexture ("menufont", p->width, p->height, p->data, false, true, 0, false);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	// load the console background
	p = (qpic_t *)FS_LoadTempFile ("gfx/menu/conback.lmp");
	Draw_PicCheckError (p, "gfx/menu/conback.lmp");
	SwapPic (p);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	conback = GL_LoadTexture ("conback", p->width, p->height, p->data, false, false, 0, false);

	// load the backtile
	p = (qpic_t *)FS_LoadTempFile ("gfx/menu/backtile.lmp");
	Draw_PicCheckError (p, "gfx/menu/backtile.lmp");
	SwapPic (p);
	draw_backtile = GL_LoadPicTexture (p);

	// load the crosshair texture
	cs_texture = GL_LoadPixmap ("crosshair", cs_data);
}

/*
===============
Draw_ReInit
This procedure re-inits some textures, those that
are read during engine's init phase, which may be
changed by mods. This should NEVER be called when
a level is active. This is intended to be called
just after changing the game directory.
===============
*/
void Draw_ReInit (void)
{
	int	j, temp, temp2;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	draw_reinit = true;

	D_ClearOpenGLTextures(0);
	texture_extension_number = 1U;
	lightmap_textures = 0U;
	for (j = 0; j < MAX_LIGHTMAPS; j++)
		lightmap_modified[j] = true;
	// make sure all of alias models are cleared
	flush_textures = true;
	temp2 = gl_purge_maptex.integer;
	Cvar_SetValue ("gl_purge_maptex", 1);
	Mod_ClearAll ();

	// Reload pre-map pics, fonts, console, etc
	W_LoadWadFile ("gfx.wad");
	Draw_Init();
	SCR_Init();
	Sbar_Init();
	// Reload the particle texture
	R_InitParticleTexture();
#if defined(H2W)
	R_InitNetgraphTexture();
#endif	/* H2W */

	Cvar_SetValue ("gl_purge_maptex", temp2);
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
void Draw_Character (int x, int y, const unsigned int num)
{
	int		row, col;
	unsigned int	c = num;
	float	frow, fcol, xsize,ysize;

	c &= 511;

	if (c == 32)
		return;		// space

	if (y <= -8)
		return;		// totally off screen

	row = c>>5;
	col = c&31;

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
void Draw_SmallCharacter (int x, int y, const int num)
{
	int		row, col;
	int		c = num;
	float	frow, fcol, xsize,ysize;

	if (c < 32)
	{
		c = 0;
	}
	else if (c >= 'a' && c <= 'z')
	{
		c -= 64;
	}
	else if (c > '_')
	{
		c = 0;
	}
	else
	{
		c -= 32;
	}

	if (c == 0)
		return;

	if (y <= -8 || y >= vid.height)
		return; 	// totally off screen

	row = c>>4;
	col = c&15;

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

	if (y >= (int)vid.height || y+pic->height < 0)
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

	if (y >= (int)vid.height || y+h < 0)
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
	if (x < 0 || (unsigned int)(x + pic->width) > vid.width || y < 0 ||
			(unsigned int)(y + pic->height) > vid.height)
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
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation, int p_class, int top, int bottom)
{
	int		i, j, v, u;
	unsigned int	trans[PLAYER_DEST_WIDTH * PLAYER_DEST_HEIGHT], *dest;
	byte		*src;
	int		p;
	char	texname[20];	// texture handle, name and trackers (Pa3PyX)

	dest = trans;
	for (v = 0; v < 64; v++, dest += 64)
	{
		src = &menuplyr_pixels[p_class-1][ ((v*pic->height)>>6) *pic->width];
		for (u = 0; u < 64; u++)
		{
			p = src[(u*pic->width)>>6];
			if (p == 255)
				dest[u] = p;
			else
				dest[u] =  d_8to24table[translation[p]];
		}
	}

	for (i = 0; i < PLAYER_PIC_WIDTH; i++)
	{
		for (j = 0; j < PLAYER_PIC_HEIGHT; j++)
		{
			trans[j * PLAYER_DEST_WIDTH + i] = d_8to24table[translation[menuplyr_pixels[p_class-1][j * PLAYER_PIC_WIDTH + i]]];
		}
	}

	// See if the texture has already been loaded; if not, do it (Pa3PyX)
	if (!plyrtex[p_class - 1][top][bottom])
	{
		q_snprintf(texname, 19, "plyrmtex%i%i%i", p_class, top, bottom);
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

void Draw_ConsoleBackground (int lines)
{
	char	ver[80];
	int	i, x, y;
	float	ofs, alpha;

	y = (vid.height * 3) >> 2;

	if (gl_constretch.integer)
		ofs = 0.0f;
	else
		ofs = (vid.conheight - lines) / (float) vid.conheight;

	if (lines > y)
		alpha = 1.0f;
	else
		alpha = (float) 1.1 * lines / y;

	Draw_ConsolePic (lines, ofs, conback, alpha);

	// print the version number and platform
//	y = lines-186;
	y = lines-14;
	q_strlcpy (ver, ENGINE_WATERMARK, sizeof(ver));
	x = vid.conwidth - (strlen(ver)*8 + 11);
#if defined(H2W)
	if (!cls.download)
#endif
		for (i = 0; i < strlen(ver); i++)
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
#if USE_HEXEN2_RESAMPLER_CODE
static void GL_ResampleTexture (unsigned int *in, int inwidth, int inheight, unsigned int *out, int outwidth, int outheight)
{
	int		i, j;
	unsigned int	*inrow, *inrow2;
	unsigned int	frac, fracstep;
	unsigned int	p1[1024], p2[1024];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth * 0x10000 / outwidth;

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
}
#else	/* here is the hexenworld (quake) resampler */
static void GL_ResampleTexture (unsigned int *in, int inwidth, int inheight, unsigned int *out, int outwidth, int outheight)
{
	int		i, j;
	unsigned int	*inrow;
	unsigned int	frac, fracstep;

	fracstep = inwidth * 0x10000 / outwidth;
	for (i = 0; i < outheight; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j = 0; j < outwidth; j += 4)
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
#endif

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

	width <<= 2;
	height >>= 1;
	out = in;
	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 8, out += 4, in += 8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

#if USE_HEXEN2_PALTEX_CODE
/*
================
fxPalTexImage2D

Acts the same as glTexImage2D, except that it maps color
into the current palette and uses paletteized textures.
If you are on a 3Dfx card and your texture has no alpha,
then download it as a palettized texture to save memory.

fxpal_buf is a pointer to allocated temporary buffer.
callers of fxPalTexImage2D must allocate and free it
properly.

This is original hexen2 code for palettized textures
Hexenworld replaced it with quake's newer code below
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
//		fxpal_buf[i] = ((unsigned char *)pixels)[i * 4];
	}

	glTexImage2D_fp(target, level, GL_COLOR_INDEX8_EXT, width, height, border, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, fxpal_buf);
}

#else	/* end of HEXEN2_PALTEX_CODE */
/*
================
GL_Resample8BitTexture -- JACK
================
*/
static void GL_Resample8BitTexture (unsigned char *in, int inwidth, int inheight, unsigned char *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned char	*inrow;
	unsigned int	frac, fracstep;

	fracstep = inwidth * 0x10000 / outwidth;
	for (i = 0; i < outheight; i++, out += outwidth)
	{
		inrow = in + inwidth*(i*inheight/outheight);
		frac = fracstep >> 1;
		for (j = 0; j < outwidth; j += 4)
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
	unsigned short	r,g,b;
	byte	*out, *at1, *at2, *at3, *at4;

//	width <<= 2;
	height >>= 1;
	out = in;
	for (i = 0; i < height; i++, in += width)
	{
		for (j = 0; j < width; j += 2, out += 1, in += 2)
		{
			at1 = (byte *) &d_8to24table[in[0]];
			at2 = (byte *) &d_8to24table[in[1]];
			at3 = (byte *) &d_8to24table[in[width+0]];
			at4 = (byte *) &d_8to24table[in[width+1]];

			r = (at1[0] + at2[0] + at3[0] + at4[0]);
			r >>= 5;
			g = (at1[1] + at2[1] + at3[1] + at4[1]);
			g >>= 5;
			b = (at1[2] + at2[2] + at3[2] + at4[2]);
			b >>= 5;

			out[0] = d_15to8table[(r<<0) + (g<<5) + (b<<10)];
//			out[0] = (in[0] + in[1] + in[width+0] + in[width+1])>>2;
//			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
//			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
//			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

static void GL_Upload8_EXT (byte *data, int width, int height,  qboolean mipmap, qboolean alpha)
{
	unsigned char		*scaled;
	int			mark = 0;
	int			scaled_width, scaled_height;

	// Snap the height and width to a power of 2
	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;

	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;

	scaled_width >>= gl_picmip.integer;
	scaled_height >>= gl_picmip.integer;

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

	mark = Hunk_LowMark();
	scaled = (unsigned char *) Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			glTexImage2D_fp (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX , GL_UNSIGNED_BYTE, data);
			goto done;
		}
		memcpy (scaled, data, width*height);
	}
	else
	{
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

done:
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
#endif	/* end of hexenworld 8_BIT_PALETTE_CODE */

/*
===============
GL_Upload32
===============
*/
static void GL_Upload32 (unsigned int *data, int width, int height, qboolean mipmap, qboolean alpha)
{
	int		samples;
	unsigned int	*scaled;
	int		mark = 0;
	int		scaled_width, scaled_height;

	// Snap the height and width to a power of 2.
	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;

	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;

	scaled_width >>= gl_picmip.integer;
	scaled_height >>= gl_picmip.integer;

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

	samples = alpha ? gl_alpha_format : gl_solid_format;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
#if USE_HEXEN2_PALTEX_CODE
			if (is8bit && !alpha)
			{
				mark = Hunk_LowMark();
				fxpal_buf = (unsigned char *) Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");
				fxPalTexImage2D (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				Hunk_FreeToLowMark(mark);
				mark = 0;
			}
			else
#endif
				glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			goto done;
		}
		scaled = data;
	}
	else
	{
		mark = Hunk_LowMark();
		scaled = (unsigned int *) Hunk_AllocName(scaled_width * scaled_height * sizeof(unsigned int), "texbuf_upload32");
		GL_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);
	}

#if USE_HEXEN2_PALTEX_CODE
	if (is8bit && !alpha)
	{
		if (!mark)
			mark = Hunk_LowMark();
		fxpal_buf = (unsigned char *) Hunk_AllocName(scaled_width * scaled_height, "texbuf_upload8pal");
		fxPalTexImage2D (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
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
#if USE_HEXEN2_PALTEX_CODE
			if (is8bit && !alpha)
				fxPalTexImage2D (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
			else
#endif
				glTexImage2D_fp (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		}
	}

done:
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

modes:
0 - standard
1 - color 0 transparent, odd - translucent, even - full value
2 - color 0 transparent
3 - special (particle translucency table)
===============
*/
static void GL_Upload8 (byte *data, int width, int height, qboolean mipmap, qboolean alpha, int mode)
{
	unsigned int		*trans;
	int			mark;
	int			i, p, s;

	s = width*height;
	mark = Hunk_LowMark();
	trans = (unsigned int *) Hunk_AllocName(s * sizeof(unsigned int), "texbuf_upload8");

	if ((alpha || mode != 0))
	{
		// if there are no transparent pixels, make it a 3 component
		// texture even if it was specified as otherwise
		// FIXME: should we not do this for mode == 0 only??  - O.S.
		i = 0;
		while (i < s)
		{
			if (data[i] == 255)
				break;
			i++;
		}
		if (i == s)
			alpha = false;

		for (i = 0; i < s; i++)
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

		switch (mode)
		{
		case 1:	/* EF_TRANSPARENT */
			alpha = true;
			for (i = 0; i < s; i++)
			{
				p = data[i];
				if (p == 0)
					trans[i] &= MASK_rgb;
				else if ( p & 1 )
				{
					trans[i] &= MASK_rgb;
					trans[i] |= ( ( int )( 255 * r_wateralpha.value ) & 0xff) << SHIFT_a;
				}
				else
				{
					trans[i] |= MASK_a;
				}
			}
			break;
		case 2:	/* EF_HOLEY */
			alpha = true;
			for (i = 0; i < s; i++)
			{
				p = data[i];
				if (p == 0)
					trans[i] &= MASK_rgb;
			}
			break;
		case 3:	/* EF_SPECIAL_TRANS */
			alpha = true;
			for (i = 0; i < s; i++)
			{
				p = data[i];
				trans[i] = d_8to24table[ColorIndex[p>>4]] & MASK_rgb;
				trans[i] |= (( int )ColorPercent[p&15] & 0xff) << SHIFT_a;
			}
			break;
		}
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

#if !USE_HEXEN2_PALTEX_CODE
	if (is8bit && !alpha)
	{
		GL_Upload8_EXT (data, width, height, mipmap, alpha);
		Hunk_FreeToLowMark(mark);
		return;
	}
#endif

	GL_Upload32 (trans, width, height, mipmap, alpha);
	Hunk_FreeToLowMark(mark);
}


/*
================
GL_LoadTexture
================
*/
GLuint GL_LoadTexture (const char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int mode, qboolean rgba)
{
	int		i, size;
	unsigned long	hash = 0;
//	unsigned short	crc;
	gltexture_t	*glt;

#if !defined (H2W)
	if (cls.state == ca_dedicated)
		return GL_UNUSED_TEXTURE;
#endif

	size = width * height;
	if (rgba)
		size *= 4;

// generate texture checksum
	for (i = 0; i < size; i++)
		hash += data[i];

// alternative: use stock crc functions for checksumming
//	crc = CRC_Block (data, size);

	// see if the texture is already present
	if (identifier[0])
	{
		for (i = 0, glt = gltextures; i < numgltextures; i++, glt++)
		{
			if (!strcmp (identifier, glt->identifier))
			{
				//if ( crc != glt->crc ||
				if ( hash != glt->hash ||
					width  != glt->width  ||
					height != glt->height ||
					mipmap != glt->mipmap )
				{	// not the same, delete and rebind to new image
					Con_DPrintf ("Texture cache mismatch: %lu, %s, reloading\n", (unsigned long)glt->texnum, identifier);
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
		Sys_Error ("%s: cache full, max is %i textures.", __thisfunc__, MAX_GLTEXTURES);

	glt = &gltextures[numgltextures];
	numgltextures++;
	q_strlcpy (glt->identifier, identifier, MAX_QPATH);
	glt->texnum = texture_extension_number;
	texture_extension_number++;

gl_rebind:
	glt->width = width;
	glt->height = height;
	glt->mipmap = mipmap;
//	glt->crc = crc;
	glt->hash = hash;

	GL_Bind (glt->texnum);
	if (rgba)
		GL_Upload32 ((unsigned int *)data, width, height, mipmap, alpha);
	else
		GL_Upload8 (data, width, height, mipmap, alpha, mode);

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

	return GL_LoadTexture (name, 32, 32, (unsigned char *) pixels, false, true, 0, true);
}

/*
================
GL_LoadPicTexture
================
*/
GLuint GL_LoadPicTexture (qpic_t *pic)
{
	return GL_LoadTexture ("", pic->width, pic->height, pic->data, false, true, 0, false);
}

