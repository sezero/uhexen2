/*
	gl_draw.c
	this is the only file outside the refresh that touches the vid buffer

	$Id: gl_draw.c,v 1.35 2005-05-27 17:56:40 sezero Exp $
*/

#include "quakedef.h"

#ifndef GL_COLOR_INDEX8_EXT
#define GL_COLOR_INDEX8_EXT	0x80E5
#endif

extern int ColorIndex[16];
extern unsigned ColorPercent[16];
extern qboolean	vid_initialized;
extern qboolean	is8bit;
extern unsigned char d_15to8table[65536];

int		gl_max_size = 256;
cvar_t		gl_picmip = {"gl_picmip", "0"};
cvar_t		gl_spritemip = {"gl_spritemip", "0"};

qboolean	plyrtex[NUM_CLASSES][16][16];		// whether or not the corresponding player textures
							// (in multiplayer config screens) have been loaded
byte		*draw_chars;				// 8*8 graphic characters
byte		*draw_smallchars;			// Small characters for status bar
byte		*draw_menufont; 			// Big Menu Font
qpic_t		*draw_backtile;

int			char_texture;
int			char_smalltexture;
int			char_menufonttexture;

byte		conback_buffer[sizeof(qpic_t) + sizeof(glpic_t)];
qpic_t		*conback = (qpic_t *)&conback_buffer;

int		gl_lightmap_format = 4;
int		gl_solid_format = 3;
int		gl_alpha_format = 4;

int		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gl_filter_max = GL_LINEAR;


int		texels;

qboolean is_3dfx = false;

gltexture_t	gltextures[MAX_GLTEXTURES];
int			numgltextures;


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

//=============================================================================
/* Support Routines */

//#define	MAX_CACHED_PICS 	128
#define	MAX_CACHED_PICS		256
cachepic_t	menu_cachepics[MAX_CACHED_PICS];
int			menu_numcachepics;

/*
 * Geometry for the player/skin selection screen image.
 */
#define PLAYER_PIC_WIDTH 68
#define PLAYER_PIC_HEIGHT 114
#define PLAYER_DEST_WIDTH 128
#define PLAYER_DEST_HEIGHT 128

byte		menuplyr_pixels[NUM_CLASSES][PLAYER_PIC_WIDTH*PLAYER_PIC_HEIGHT];

int		pic_texels;
int		pic_count;

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

	// load little ones into the scrap
/*	if (p->width < 64 && p->height < 64)
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
	}
	else*/
	{
//nonscrap:
		gl->texnum = GL_LoadPicTexture (p);

		gl->sl = 0;
		gl->sh = 1;
		gl->tl = 0;
		gl->th = 1;
	}
	return p;
}

qpic_t *Draw_PicFromWad (char *name)
{
	qpic_t	*p;
	glpic_t	*gl;

	p = W_GetLumpName (name);
	gl = (glpic_t *)p->data;

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
	}
	else
	{
nonscrap:
		gl->texnum = GL_LoadPicTexture (p);
		gl->sl = 0;
		gl->sh = 1;
		gl->tl = 0;
		gl->th = 1;
	}
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
	else if (!strcmp (path, "gfx/menu/netp5.lmp"))
		memcpy (menuplyr_pixels[4], dat->data, dat->width*dat->height);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;

	gl = (glpic_t *)pic->pic.data;
	gl->texnum = GL_LoadPicTexture (dat);

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return &pic->pic;
}

/*
================
Draw_CachePicNoTrans

Pa3PyX: Function added to cache pics ignoring transparent colors
	 (e.g. in intermission screens)
================
*/
qpic_t *Draw_CachePicNoTrans(char *path)
{
	cachepic_t      *pic;
	int                     i;
	qpic_t          *dat;
	glpic_t         *gl;

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

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return &pic->pic;
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
			if (source[x] != 255)
				dest[x] = 0x60 + source[x];
		source += 256;
		dest += 320;
	}

}

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
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

		if (glt->mipmap)
		{
			glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		}
		else
		{
			glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		}
	}
}

/*
===============
Draw_Init
===============
*/
void Draw_Init (void)
{
	int	i;
	qpic_t	*cb, *mf;
	byte	*dest;
	int	x, y;
	char	ver[40];
	glpic_t	*gl;

	Cvar_RegisterVariable (&gl_picmip);
	Cvar_RegisterVariable (&gl_spritemip);

	glfunc.glGetIntegerv_fp(GL_MAX_TEXTURE_SIZE, &gl_max_size);
	if (gl_max_size < 64)	/* Any living examples for this? */
		gl_max_size = 64;
	if (gl_max_size > 1024) /* O.S: write a cmdline override if necessary */
		gl_max_size = 1024;
	Con_Printf("OpenGL max.texture size: %i\n", gl_max_size);

	Cmd_AddCommand ("gl_texturemode", &Draw_TextureMode_f);
	Cmd_AddCommand ("gl_texels", &GL_Texels_f);


	// load the console background and the charset
	// by hand, because we need to write the version
	// string into the background before turning
	// it into a texture
	//draw_chars = W_GetLumpName ("conchars");
	draw_chars = COM_LoadHunkFile ("gfx/menu/conchars.lmp");
	for (i=0 ; i<256*128 ; i++)
		if (draw_chars[i] == 0)
			draw_chars[i] = 255;	// proper transparent color

	char_texture = GL_LoadTexture ("charset", 256, 128, draw_chars, false, true, 0, false);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);


	draw_smallchars = W_GetLumpName("tinyfont");
	for (i=0 ; i<128*32 ; i++)
		if (draw_smallchars[i] == 0)
			draw_smallchars[i] = 255;	// proper transparent color

	// now turn them into textures
	char_smalltexture = GL_LoadTexture ("smallcharset", 128, 32, draw_smallchars, false, true, 0, false);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	mf = (qpic_t *)COM_LoadTempFile("gfx/menu/bigfont2.lmp");
	for (i=0 ; i<160*80 ; i++)
		if (mf->data[i] == 0)
			mf->data[i] = 255;	// proper transparent color


	char_menufonttexture = GL_LoadTexture ("menufont", 160, 80, mf->data, false, true, 0, false);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	cb = (qpic_t *)COM_LoadTempFile ("gfx/menu/conback.lmp");
	if (!cb)
		Sys_Error ("Couldn't load gfx/menu/conback.lmp");
	SwapPic (cb);

	// hack the version number directly into the pic

	sprintf (ver, "%4.2f (%s)", HEXEN2_VERSION, VERSION_PLATFORM);
	dest = cb->data + 320 + 320*186 - 11 - 8*strlen(ver);

//	sprintf (ver, "(gl %4.2f) %4.2f", (float)GLQUAKE_VERSION, (float)VERSION);
//	dest = cb->data + 320*186 + 320 - 11 - 8*strlen(ver);
	y = strlen(ver);
	for (x=0 ; x<y ; x++)
		Draw_CharToConback (ver[x], dest+(x<<3));

	gl = (glpic_t *)conback->data;
	gl->texnum = GL_LoadTexture ("conback", cb->width, cb->height, cb->data, false, false, 0, false);
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;
	conback->width = vid.conwidth;
	conback->height = vid.conheight;

	// save slots for scraps
	scrap_texnum = texture_extension_number;
	texture_extension_number += MAX_SCRAPS;
//	draw_backtile = Draw_PicFromWad ("backtile");
	draw_backtile = Draw_PicFromFile ("gfx/menu/backtile.lmp");
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
		return;			// totally off screen

	row = num>>5;
	col = num&31;

	xsize = 0.03125;
	ysize = 0.0625;
	fcol = col*xsize;
	frow = row*ysize;

	GL_Bind (char_texture);

	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (fcol, frow);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (fcol + xsize, frow);
	glfunc.glVertex2f_fp (x+8, y);
	glfunc.glTexCoord2f_fp (fcol + xsize, frow + ysize);
	glfunc.glVertex2f_fp (x+8, y+8);
	glfunc.glTexCoord2f_fp (fcol, frow + ysize);
	glfunc.glVertex2f_fp (x, y+8);
	glfunc.glEnd_fp ();
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
		return; 		// totally off screen

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

	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (fcol, frow);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (fcol + xsize, frow);
	glfunc.glVertex2f_fp (x+8, y);
	glfunc.glTexCoord2f_fp (fcol + xsize, frow + ysize);
	glfunc.glVertex2f_fp (x+8, y+8);
	glfunc.glTexCoord2f_fp (fcol, frow + ysize);
	glfunc.glVertex2f_fp (x, y+8);
	glfunc.glEnd_fp ();
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
================
Draw_DebugChar

Draws a single character directly to the upper right corner of the screen.
This is for debugging lockups by drawing different chars in different parts
of the code.
================
*/
void Draw_DebugChar (char num)
{
}

/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	glpic_t			*gl;

	if (scrap_dirty)
		Scrap_Upload ();
	gl = (glpic_t *)pic->data;
	glfunc.glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (gl->sl, gl->tl);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (gl->sh, gl->tl);
	glfunc.glVertex2f_fp (x+pic->width, y);
	glfunc.glTexCoord2f_fp (gl->sh, gl->th);
	glfunc.glVertex2f_fp (x+pic->width, y+pic->height);
	glfunc.glTexCoord2f_fp (gl->sl, gl->th);
	glfunc.glVertex2f_fp (x, y+pic->height);
	glfunc.glEnd_fp ();

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/*
=============
Draw_AlphaPic
=============
*/
void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha)
{
	glpic_t			*gl;

	if (scrap_dirty)
		Scrap_Upload ();
	gl = (glpic_t *)pic->data;
	glfunc.glDisable_fp(GL_ALPHA_TEST);
	glfunc.glEnable_fp (GL_BLEND);
//	glfunc.glBlendFunc_fp(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfunc.glCullFace_fp(GL_FRONT);
	glfunc.glColor4f_fp (1,1,1,alpha);
	GL_Bind (gl->texnum);
	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (gl->sl, gl->tl);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (gl->sh, gl->tl);
	glfunc.glVertex2f_fp (x+pic->width, y);
	glfunc.glTexCoord2f_fp (gl->sh, gl->th);
	glfunc.glVertex2f_fp (x+pic->width, y+pic->height);
	glfunc.glTexCoord2f_fp (gl->sl, gl->th);
	glfunc.glVertex2f_fp (x, y+pic->height);
	glfunc.glEnd_fp ();
	glfunc.glColor4f_fp (1,1,1,1);
	glfunc.glEnable_fp(GL_ALPHA_TEST);
	glfunc.glDisable_fp (GL_BLEND);
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

	if (scrap_dirty)
		Scrap_Upload ();
	gl = (glpic_t *)pic->data;
	glfunc.glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glfunc.glBegin_fp(GL_QUADS);
	glfunc.glTexCoord2f_fp(0.0f, 0.0f);
	glfunc.glVertex2f_fp(0.0f, 0.0f);
	glfunc.glTexCoord2f_fp(1.0f, 0.0f);
	glfunc.glVertex2f_fp(vid.width, 0.0f);
	glfunc.glTexCoord2f_fp(1.0f, 1.0f);
	glfunc.glVertex2f_fp(vid.width, vid.height);
	glfunc.glTexCoord2f_fp(0.0f, 1.0f);
	glfunc.glVertex2f_fp(0.0f, vid.height);
	glfunc.glEnd_fp();

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Draw_PicCropped(int x, int y, qpic_t *pic)
{
	int height;
	glpic_t 		*gl;
	float th,tl;

	if((x < 0) || (x+pic->width > vid.width))
	{
		Sys_Error("Draw_PicCropped: bad coordinates");
	}

	if (y >= (int)vid.height || y+pic->height < 0)
	{ // Totally off screen
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();
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


	glfunc.glColor4f_fp (1,1,1,1);
	GL_Bind (gl->texnum);
	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (gl->sl, tl);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (gl->sh, tl);
	glfunc.glVertex2f_fp (x+pic->width, y);
	glfunc.glTexCoord2f_fp (gl->sh, th);
	glfunc.glVertex2f_fp (x+pic->width, y+height);
	glfunc.glTexCoord2f_fp (gl->sl, th);
	glfunc.glVertex2f_fp (x, y+height);
	glfunc.glEnd_fp ();
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
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation)
{
	int			v, u/*, c*/;
	unsigned		trans[PLAYER_DEST_WIDTH * PLAYER_DEST_HEIGHT], *dest;
	byte			*src;
	int			p;

	// texture handle, name and trackers (Pa3PyX)
	char texname[20];
	static qboolean first_time = true;
	extern int setup_top;
	extern int setup_bottom;
	extern int setup_class;

	// Initialize array of texnums
	if (first_time) {
		memset(plyrtex, 0, NUM_CLASSES * 16 * 16 * sizeof(qboolean));
		first_time = false;
	}

/*
	GL_Bind (translate_texture[setup_class-1]);

	c = pic->width * pic->height;
*/

	dest = trans;
	for (v=0 ; v<64 ; v++, dest += 64)
	{
		src = &menuplyr_pixels[setup_class-1][ ((v*pic->height)>>6) *pic->width];
		for (u=0 ; u<64 ; u++)
		{
			p = src[(u*pic->width)>>6];
			if (p == 255)
				dest[u] = p;
			else
				dest[u] =  d_8to24table[translation[p]];
		}
	}

#if 0
	{
		int i;

		for( i = 0; i < 64 * 64; i++ )
		{
			trans[i] = d_8to24table[translation[menuplyr_pixels[i]]];
		}
	}
#endif

	{
		int xi, yi;

		for( xi = 0; xi < PLAYER_PIC_WIDTH; xi++ )
			for( yi = 0; yi < PLAYER_PIC_HEIGHT; yi++ )
			{
				trans[yi * PLAYER_DEST_WIDTH + xi] = d_8to24table[translation[menuplyr_pixels[setup_class-1][yi * PLAYER_PIC_WIDTH + xi]]];
			}
	}

	// See if the texture has already been loaded; if not, do it (Pa3PyX)
	if (!plyrtex[setup_class - 1][setup_top][setup_bottom]) {
		snprintf(texname, 19, "plyrmtex%i%i%i", setup_class, setup_top, setup_bottom);
		plyrtex[setup_class - 1][setup_top][setup_bottom] = GL_LoadTexture(texname, PLAYER_DEST_WIDTH, PLAYER_DEST_HEIGHT, (byte *)trans, false, true, 0, true);
	}
	GL_Bind(plyrtex[setup_class - 1][setup_top][setup_bottom]);

	glfunc.glColor3f_fp (1,1,1);
	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (0, 0);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (( float )PLAYER_PIC_WIDTH / PLAYER_DEST_WIDTH, 0);
	glfunc.glVertex2f_fp (x+pic->width, y);
	glfunc.glTexCoord2f_fp (( float )PLAYER_PIC_WIDTH / PLAYER_DEST_WIDTH, ( float )PLAYER_PIC_HEIGHT / PLAYER_DEST_HEIGHT);
	glfunc.glVertex2f_fp (x+pic->width, y+pic->height);
	glfunc.glTexCoord2f_fp (0, ( float )PLAYER_PIC_HEIGHT / PLAYER_DEST_HEIGHT);
	glfunc.glVertex2f_fp (x, y+pic->height);
	glfunc.glEnd_fp ();
}

int M_DrawBigCharacter (int x, int y, int num, int numNext)
{
	int			row, col;
	float			frow, fcol, xsize,ysize;
	int			add;

	if (num == ' ') return 32;

	if (num == '/') num = 26;
	else num -= 65;

	if (num < 0 || num >= 27)  // only a-z and /
		return 0;

	if (numNext == '/') numNext = 26;
	else numNext -= 65;

	row = num/8;
	col = num%8;

	xsize = 0.125;
	ysize = 0.25;
	fcol = col*xsize;
	frow = row*ysize;

	GL_Bind (char_menufonttexture);

	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (fcol, frow);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp (fcol + xsize, frow);
	glfunc.glVertex2f_fp (x+20, y);
	glfunc.glTexCoord2f_fp (fcol + xsize, frow + ysize);
	glfunc.glVertex2f_fp (x+20, y+20);
	glfunc.glTexCoord2f_fp (fcol, frow + ysize);
	glfunc.glVertex2f_fp (x, y+20);
	glfunc.glEnd_fp ();

	if (numNext < 0 || numNext >= 27) return 0;

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
	int y;

	y = (vid.height * 3) >> 2;
	if (lines > y)
		Draw_Pic (0, lines-vid.height, conback);
	else
		Draw_AlphaPic (0, lines - vid.height, conback, (float)(1.1 * lines)/y);
		// O.S: hexenworld had 1.2 as multiplier, I used 1.1
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
	glfunc.glColor3f_fp (1,1,1);
	GL_Bind (*(int *)draw_backtile->data);
	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glTexCoord2f_fp (x/64.0, y/64.0);
	glfunc.glVertex2f_fp (x, y);
	glfunc.glTexCoord2f_fp ( (x+w)/64.0, y/64.0);
	glfunc.glVertex2f_fp (x+w, y);
	glfunc.glTexCoord2f_fp ( (x+w)/64.0, (y+h)/64.0);
	glfunc.glVertex2f_fp (x+w, y+h);
	glfunc.glTexCoord2f_fp ( x/64.0, (y+h)/64.0 );
	glfunc.glVertex2f_fp (x, y+h);
	glfunc.glEnd_fp ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	glfunc.glDisable_fp (GL_TEXTURE_2D);
	glfunc.glColor3f_fp (host_basepal[c*3]/255.0,
		host_basepal[c*3+1]/255.0,
		host_basepal[c*3+2]/255.0);

	glfunc.glBegin_fp (GL_QUADS);

	glfunc.glVertex2f_fp (x,y);
	glfunc.glVertex2f_fp (x+w, y);
	glfunc.glVertex2f_fp (x+w, y+h);
	glfunc.glVertex2f_fp (x, y+h);

	glfunc.glEnd_fp ();
	glfunc.glColor3f_fp (1,1,1);
	glfunc.glEnable_fp (GL_TEXTURE_2D);
}
//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	int bx,by,ex,ey;
	int c;

#if 0
	// fadescreen ripple effect stops help/quit screens
	// from appearing on 3dfx with Mesa Glide.  S.A

	// fadescreen actually works with supported versions
	// of mesa, such as 3.0, 3.2.1, 3.4.2 and 6.2.1.
	// It only seems to break when using an old binary
	// mesaGL from a quake3 package.		O.S.

	if (is_3dfx) return;
#endif

	glfunc.glAlphaFunc_fp(GL_ALWAYS, 0);

	glfunc.glEnable_fp (GL_BLEND);
	glfunc.glDisable_fp (GL_TEXTURE_2D);

//	glfunc.glColor4f_fp (248.0/255.0, 220.0/255.0, 120.0/255.0, 0.2);
	glfunc.glColor4f_fp (208.0/255.0, 180.0/255.0, 80.0/255.0, 0.2);
	glfunc.glBegin_fp (GL_QUADS);
	glfunc.glVertex2f_fp (0,0);
	glfunc.glVertex2f_fp (vid.width, 0);
	glfunc.glVertex2f_fp (vid.width, vid.height);
	glfunc.glVertex2f_fp (0, vid.height);
	glfunc.glEnd_fp ();

	glfunc.glColor4f_fp (208.0/255.0, 180.0/255.0, 80.0/255.0, 0.035);
	for(c=0;c<40;c++)
	{
		bx = rand() % vid.width-20;
		by = rand() % vid.height-20;
		ex = bx + (rand() % 40) + 20;
		ey = by + (rand() % 40) + 20;
		if (bx < 0) bx = 0;
		if (by < 0) by = 0;
		if (ex > vid.width) ex = vid.width;
		if (ey > vid.height) ey = vid.height;

		glfunc.glBegin_fp (GL_QUADS);
		glfunc.glVertex2f_fp (bx, by);
		glfunc.glVertex2f_fp (ex, by);
		glfunc.glVertex2f_fp (ex, ey);
		glfunc.glVertex2f_fp (bx, ey);
		glfunc.glEnd_fp ();
	}

	glfunc.glColor4f_fp (1,1,1,1);
	glfunc.glEnable_fp (GL_TEXTURE_2D);
	glfunc.glDisable_fp (GL_BLEND);

	glfunc.glAlphaFunc_fp(GL_GREATER, 0.666);

	SB_Changed();
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
	glfunc.glViewport_fp (glx, gly, glwidth, glheight);

	glfunc.glMatrixMode_fp(GL_PROJECTION);
	glfunc.glLoadIdentity_fp ();
	glfunc.glOrtho_fp  (0, vid.width, vid.height, 0, -99999, 99999);

	glfunc.glMatrixMode_fp(GL_MODELVIEW);
	glfunc.glLoadIdentity_fp ();

	glfunc.glDisable_fp (GL_DEPTH_TEST);
	glfunc.glDisable_fp (GL_CULL_FACE);
	glfunc.glDisable_fp (GL_BLEND);
	glfunc.glEnable_fp (GL_ALPHA_TEST);
//	glfunc.glDisable_fp (GL_ALPHA_TEST);

	glfunc.glColor4f_fp (1,1,1,1);
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
void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
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
GL_Resample8BitTexture -- JACK
================
*/
void GL_Resample8BitTexture (unsigned char *in, int inwidth, int inheight, unsigned char *out,  int outwidth, int outheight)
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
GL_MipMap

Operates in place, quartering the size of the texture
================
*/
void GL_MipMap (byte *in, int width, int height)
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

/*
================
GL_MipMap8Bit

Mipping for 8 bit textures
================
*/
void GL_MipMap8Bit (byte *in, int width, int height)
{
	int		i, j;
	unsigned short     r,g,b;
	byte	*out, *at1, *at2, *at3, *at4;

//	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
//	{
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
//	}
}

/*
===============
GL_Upload32
===============
*/
void GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap, qboolean alpha, qboolean sprite)
{
	int			samples;
	static	unsigned	scaled[1024*512];	// [512*256];
	int			scaled_width, scaled_height;

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
	{
		scaled_width = 1;
	}
	if (scaled_height < 1)
	{
		scaled_height = 1;
	}

	if (scaled_width > gl_max_size)
		scaled_width = gl_max_size;
	if (scaled_height > gl_max_size)
		scaled_height = gl_max_size;

	// 3dfx has some aspect ratio constraints. . . can't go beyond 8 to 1 or below 1 to 8.
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

	if (scaled_width * scaled_height > sizeof(scaled)/4)
		Sys_Error ("GL_LoadTexture: too big");

	samples = alpha ? gl_alpha_format : gl_solid_format;

#if 0
	if (mipmap)
		gluBuild2DMipmaps (GL_TEXTURE_2D, samples, width, height, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else if (scaled_width == width && scaled_height == height)
		glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else
	{
		gluScaleImage (GL_RGBA, width, height, GL_UNSIGNED_BYTE, trans,
			scaled_width, scaled_height, GL_UNSIGNED_BYTE, scaled);
		glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
#else
	texels += scaled_width * scaled_height;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			goto done;
		}
		memcpy (scaled, data, width*height*4);
	}
	else
		GL_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

	glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
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
			glfunc.glTexImage2D_fp (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		}
	}
done: ;
#endif


	if (mipmap)
	{
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
}

void GL_Upload8_EXT (byte *data, int width, int height,  qboolean mipmap, qboolean alpha, qboolean sprite) 
{
	int			i, s;
	qboolean		noalpha;
	int			samples;
	static	unsigned char scaled[1024*512];	// [512*256];
	int			scaled_width, scaled_height;

	s = width*height;
	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
	if (alpha)
	{
		noalpha = true;
		for (i=0 ; i<s ; i++)
		{
			if (data[i] == 255)
				noalpha = false;
		}

		if (alpha && noalpha)
			alpha = false;
	}
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
	{
		scaled_width = 1;
	}
	if (scaled_height < 1)
	{
		scaled_height = 1;
	}

	if (scaled_width > gl_max_size)
		scaled_width = gl_max_size;
	if (scaled_height > gl_max_size)
		scaled_height = gl_max_size;

	// 3dfx has some aspect ratio constraints. . . can't go beyond 8 to 1 or below 1 to 8.
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

	if (scaled_width * scaled_height > sizeof(scaled))
		Sys_Error ("GL_LoadTexture: too big");

	samples = 1; // alpha ? gl_alpha_format : gl_solid_format;

	texels += scaled_width * scaled_height;

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX , GL_UNSIGNED_BYTE, data);
			goto done;
		}
		memcpy (scaled, data, width*height);
	}
	else
		GL_Resample8BitTexture (data, width, height, scaled, scaled_width, scaled_height);

	glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, scaled);
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
			glfunc.glTexImage2D_fp (GL_TEXTURE_2D, miplevel, GL_COLOR_INDEX8_EXT, scaled_width, scaled_height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, scaled);
		}
	}
done: ;

	if (mipmap)
	{
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
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
	static	unsigned	trans[640*480];		// FIXME, temporary
	int			i, s;
	qboolean		noalpha;
	int			p;
	qboolean		sprite = false;

	if (mode >= 10)
	{
		sprite = true;
		mode -= 10;
	}

	s = width*height;
	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
	if ((alpha || mode != 0))
	{
		noalpha = true;
		for (i=0 ; i<s ; i++)
		{
			p = data[i];
			if (p == 255)
				noalpha = false;
			trans[i] = d_8to24table[p];
		}

		for (i=0 ; i<s ; i++)
		{
			int n;
			int r = 0, g = 0, b = 0;

			p = data[i];
			if (p == 255)
			{
				unsigned long neighbors[9];
				int num_neighbors_valid = 0;
				int neighbor_u, neighbor_v;

				int u, v;
				u = s % width;
				v = s / width;

				for( neighbor_u = u - 1; neighbor_u <= u + 1; neighbor_u++ )
				{
					for( neighbor_v = v - 1; neighbor_v <= v + 1; neighbor_v++ )
					{
						if( neighbor_u == neighbor_v )
							continue;
						// Make sure  that we are accessing a texel in the image, not out of range.
						if( neighbor_u < 0 || neighbor_u > width || neighbor_v < 0 || neighbor_v > height )
							continue;
						if( data[neighbor_u + neighbor_v * width] == 255 )
							continue;
						neighbors[num_neighbors_valid++] = trans[neighbor_u + neighbor_v * width];
					}
				}

				if( num_neighbors_valid == 0 )
					continue;

				for( n = 0; n < num_neighbors_valid; n++ )
				{
					r += neighbors[n] & 0xff;
					g += ( neighbors[n] & 0xff00 ) >> 8;
					b += ( neighbors[n] & 0xff0000 ) >> 16;
				}

				r /= num_neighbors_valid;
				g /= num_neighbors_valid;
				b /= num_neighbors_valid;

				if( r > 255 )
					r = 255;
				if( g > 255 )
					g = 255;
				if( b > 255 )
					b = 255;

				trans[i] = ( b << 16  ) | ( g << 8 ) | r;
//				trans[i] = 0;
			}
		}

		if (alpha && noalpha)
			alpha = false;

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

	if (is8bit && !alpha && (data!=scrap_texels[0])) {
		GL_Upload8_EXT (data, width, height, mipmap, alpha, sprite);
		return;
	}

	GL_Upload32 (trans, width, height, mipmap, alpha, sprite);
}

/*
================
GL_LoadTexture
================
*/
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int mode, qboolean rgba)
{
	int		i;
	gltexture_t	*glt;
	char search[64];

	if (!vid_initialized)
		return -1;

	sprintf (search, "%s%d%d",identifier,width,height);

	// see if the texture is already present
	if (identifier[0])
	{
		for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
		{
			if (!strcmp (search, glt->identifier))
			{
				if (width != glt->width || height != glt->height || mipmap != glt->mipmap) {
				// Not the same texture - dont die, delete and rebind to new image
				// Paranoia??? We already flush opengl textures upon map change...
				// TODO - Maybe add the hash check some day
					Con_Printf ("GL_LoadTexture: reloading tex due to cache mismatch\n");
					glfunc.glDeleteTextures_fp (1, &(glt->texnum));
					glt->width = width;
					glt->height = height;
					glt->mipmap = mipmap;
					GL_Bind (glt->texnum);
					if (rgba)
						GL_Upload32 ((unsigned *)data, width, height, mipmap, alpha, false);
					else
						GL_Upload8 (data, width, height, mipmap, alpha, mode);
					return glt->texnum;
				} else {
				// No need to rebind
				return gltextures[i].texnum;
				}
			}
		}
	} else {
		glt = &gltextures[numgltextures];
	}
	numgltextures++;

	strcpy (glt->identifier, search);
	glt->texnum = texture_extension_number;
	glt->width = width;
	glt->height = height;
	glt->mipmap = mipmap;

	GL_Bind (texture_extension_number);

	if (rgba)
		GL_Upload32 ((unsigned *)data, width, height, mipmap, alpha, false);
	else
		GL_Upload8 (data, width, height, mipmap, alpha, mode);

	texture_extension_number++;

	return texture_extension_number-1;
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.34  2005/05/26 18:20:42  sezero
 * whitespace + unused stuff...
 *
 * Revision 1.33  2005/05/21 17:32:03  sezero
 * disabled the rotating skull annoyance in GL mode (used to
 * cause problems with voodoo1/mesa6 when using gamma tricks)
 *
 * Revision 1.32  2005/05/07 10:39:07  sezero
 * display platform in console background
 *
 * Revision 1.31  2005/04/30 09:59:16  sezero
 * Many things in gl_vidsdl.c, and *especially in vid_sdl.c, are there
 * for the dynamic video mode swithching which we removed a long time
 * ago (and we don't seem to put it back in any foreseeable future.)
 * Some stuff were there only to provide human readable descriptions in
 * the menu and I removed them in 1.2.3 or in 1.2.4. In this patch:
 * 1. Tried cleaning-up the remaining mess: There still were some
 *    windoze left-overs, unused variables/cvars, functions using those
 *    vars/cvars serving no purpose (especially those window_rect and
 *    window_center stuff, and more). I removed them as best as I could.
 *    There still are things in vid_sdl.c that I didn't fully understand,
 *    they are there, for now.
 * 2. The -window and -w cmdline args are now now removed: They actually
 *    did nothing, unless the user did some silly thing like using both
 *    -w and -f on the same cmdline.
 * 3. The two mode-setting functions (windowed and f/s) are made into one
 *    as VID_SDL_SetMode
 * 4. The -height arg now is functional *only* if used together -height.
 *    Since we only do the normal modes, I removed the width switch and
 *    calculated:  height = 3*width/4
 *    Issue: We need some sanity check in case of both -width and -height
 *    args are specified
 * 5. -bpp wasn't written into modenum[x].bpp, I did it here. As a side
 *    note, bpp doesn't affect anything, or my eyes are in more need of a
 *    doctor than I know: -bpp 8 / 16 / 32 give the same picture.
 * 6. The code calls VID_SetPalette very multiple times in gl_vidsdl.c.
 *    Why the hell is that?.. Something windoze spesific?  I unified them
 *    here in VID_Init: After VID_SetMode, VID_SetPalette is called first,
 *    and then 8-bit palette is activated if -paltex is specified.
 *    Note: I didn't touch vid_sdl.c in this manner, but DDOI (one of the
 *    guys during Dan's porting, perpahs) has a comment on a VID_SetPalette
 *    call being "Useless".
 * 7. Many whitespace clean-up as a bonus material ;)
 *
 * Revision 1.30  2005/04/30 09:06:07  sezero
 * Remove the 3dfx-spesific 3DFX_set_global_palette usage and favor
 * GL_EXT_shared_texture_palette, instead. VID_Download3DfxPalette
 * code taken from Pa3PyX.
 *
 * Revision 1.29  2005/04/30 09:02:14  sezero
 * Patch for voodoo1/voodoo2/rush, enabling 3dfx paletted texture extensions:
 * * it may help low vidmem issues without using gl_picmip uglies.
 * * may also help with a little performance.
 * Notes:
 * * paletted textures enabled only when -paltex cmdline arg is used, otherwise
 *   voodoo2 rendering is borked with both GL_EXT_shared_texture_palette and
 *   3DFX_set_global_palette (at least in my experience with Mesa-3.4.2 on
 *   RedHat-7.3)
 * * some dlsym fiddling from quake1 was added in order to successfuly link to
 *   the gl function. SDL_GL_GetProcAddress did not work for me...
 * * This 3dfx extension is hardware spesific and even Mesa says not to use it,
 *   but use the generic GL_EXT_shared_texture_palette, instead.
 *
 * Revision 1.28  2005/04/30 08:48:39  sezero
 * silenced warnings about x and y parameters being shadowed
 *
 * Revision 1.27  2005/04/30 08:10:19  sezero
 * re-enable fadescreen for is_3dfx == true (but keep the note in
 * for future reference)
 *
 * Revision 1.26  2005/04/13 12:20:44  sezero
 * made GL_Bind a macro (from Pa3PyX)
 *
 * Revision 1.25  2005/04/08 17:30:21  sezero
 * 3dfx stuff initial clean-up
 *
 * Revision 1.24  2005/04/05 19:37:51  sezero
 * fadescreen ripple effect may stop help/quit screens
 * from appearing on 3dfx with Mesa Glide. (from Steve)
 *
 * Revision 1.23  2005/02/20 13:48:55  sezero
 * add comments on gl_picmip scale-down and ugly help screens (Steven)
 *
 * Revision 1.22  2005/02/07 22:01:03  sezero
 * Draw_AlphaPic(), draw alpha console background (from HW)
 *
 * Revision 1.21  2005/02/07 21:58:23  sezero
 * if0-out fxPalTexImage2D (we don't use it)
 *
 * Revision 1.20  2005/01/24 20:34:21  sezero
 * gl filter changes
 *
 * Revision 1.19  2005/01/24 20:29:43  sezero
 * fix flush_textures decision which used to be always true
 *
 * Revision 1.18  2005/01/24 20:27:12  sezero
 * consolidate GL_LoadTexture functions
 *
 * Revision 1.17  2005/01/12 11:57:40  sezero
 * glfunc stuff (nitpicking)
 *
 * Revision 1.16  2005/01/12 11:56:30  sezero
 * missed dlsym'ing this one
 *
 * Revision 1.15  2005/01/11 07:11:41  sezero
 * missing newline
 *
 * Revision 1.14  2005/01/08 16:10:46  sezero
 * slight gl cleanup
 *
 * Revision 1.13  2005/01/08 16:07:45  sezero
 * don't die on cache mismatch, delete and and reload the new texture
 *
 * Revision 1.12  2005/01/04 07:17:51  sezero
 * make gcc-2.96 happy
 *
 * Revision 1.10  2004/12/19 12:47:26  sezero
 * fix the datatype for our new load32
 *
 * Revision 1.9  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.8  2004/12/18 14:15:34  sezero
 * Clean-up and kill warnings 10:
 * Remove some already commented-out functions and code fragments.
 * They seem to be of no-future use. Also remove some unused functions.
 *
 * Revision 1.7  2004/12/18 14:11:53  sezero
 * Clean-up and kill warnings 9a:
 * More unused vars...
 *
 * Revision 1.6  2004/12/18 14:08:07  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.5  2004/12/18 13:30:50  sezero
 * Hack to prevent textures going awol and some info-plaques start looking
 * white upon succesive load games. The solution is not beautiful but seems
 * to work for now. Adapted from Pa3PyX sources.
 *
 * Revision 1.4  2004/12/16 18:10:12  sezero
 * - Add glGetIntegerv, glActiveTextureARB and glMultiTexCoord2fARB to the
 *   gl_func lists. (glGetIntegerv is required to init. The others are for
 *   future use).
 * - Use glGetIntegerv to detect gl_max_size, not vendor string. if > 1024,
 *   default to 1024 (inspired from pa3pyx).
 * - gl_max_size is not a cvar anymore (ditto).
 * - Kill cvar gl_nobind (ditto).
 *
 * Revision 1.3  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.2  2004/11/29 12:17:46  sezero
 * draw fullscreen intermission pics. borrowed from Pa3PyX sources.
 *
 * Revision 1.1.1.1  2004/11/28 00:02:55  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.1  2002/01/02 15:14:44  phneutre
 * dlsym'ed all calls to GL functions with SDL_GL_GetProcAddress
 *
 * 2     2/26/98 3:09p Jmonroe
 * fixed gl for numclasses
 * 
 * 34    9/30/97 6:12p Rlove
 * Updates
 * 
 * 33    9/30/97 4:22p Rjohnson
 * PowerVRUpdates
 * 
 * 32    9/25/97 2:10p Rjohnson
 * Smaller status bar
 * 
 * 31    9/23/97 9:47p Rjohnson
 * Fix for dedicated gl server and color maps for sheeps
 * 
 * 30    9/09/97 10:49a Rjohnson
 * Updates
 * 
 * 29    9/03/97 9:10a Rjohnson
 * Update
 * 
 * 28    9/02/97 12:25a Rjohnson
 * Font Update
 * 
 * 27    8/31/97 9:27p Rjohnson
 * GL Updates
 *
 * 24	 8/20/97 2:05p Rjohnson
 * fix for internationalization
 *
 * 23	 8/20/97 11:40a Rjohnson
 * Character Fixes
 *
 * 22	 8/19/97 10:35p Rjohnson
 * Fix for loading plaque
 *
 * 21	 8/18/97 12:03a Rjohnson
 * Added loading progress
 *
 * 20	 8/15/97 11:27a Rlove
 * Changed MAX_CACHED_PICS to 256
 *
 * 19	 6/17/97 10:03a Rjohnson
 * GL Updates
 *
 * 18	 6/16/97 4:25p Rjohnson
 * Fixed a few minor things
 *
 * 17	 6/16/97 5:28a Rjohnson
 * Minor fixes
 *
 * 16	 6/15/97 7:52p Rjohnson
 * Added new paused and loading graphics
 *
 * 15	 6/10/97 9:09a Rjohnson
 * GL Updates
 *
 * 14	 6/06/97 5:17p Rjohnson
 * New console characters
 *
 * 13	 6/02/97 3:42p Gmctaggart
 * GL Catchup
 *
 * 12	 4/30/97 11:20p Bgokey
 *
 * 11	 4/18/97 11:24a Rjohnson
 * Changed the background of the menus when in the game
 *
 * 10	 4/17/97 3:42p Rjohnson
 * Modifications for the gl version for menus
 *
 * 9	 4/17/97 12:14p Rjohnson
 * Modified the cropped drawing routine
 *
 * 8	 3/22/97 5:19p Rjohnson
 * No longer has static large arrays for texture loading
 *
 * 7	 3/22/97 3:22p Rjohnson
 * Moved the glpic structure to the glquake.h header file
 *
 * 6	 3/13/97 10:53p Rjohnson
 * Support for small font and uploading a texture with a specific alpha
 * value
 *
 * 5	 3/13/97 12:24p Rjohnson
 * Implemented the draw "cropped" commands in the gl version
 *
 * 4	 3/07/97 5:54p Rjohnson
 * Made it so that gl_round_down defaults to 0
 *
 * 3	 3/07/97 1:06p Rjohnson
 * Id Updates
 *
 * 2	 2/20/97 12:13p Rjohnson
 * Code fixes for id update
 */
