/*
	draw.c
	this is the only file outside the refresh that touches the vid buffer

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/gl_draw.c,v 1.16 2005-01-12 11:56:30 sezero Exp $
*/

#include "quakedef.h"

extern int ColorIndex[16];
extern unsigned ColorPercent[16];
extern qboolean	vid_initialized;

#define MAX_DISC 18

int		gl_max_size = 256;
cvar_t		gl_round_down = {"gl_round_down", "0"};
cvar_t		gl_picmip = {"gl_picmip", "0"};

qboolean	plyrtex[NUM_CLASSES][16][16];		// whether or not the corresponding player textures
							// (in multiplayer config screens) have been loaded
byte		*draw_chars;				// 8*8 graphic characters
byte		*draw_smallchars;			// Small characters for status bar
byte		*draw_menufont; 			// Big Menu Font
qpic_t		*draw_disc[MAX_DISC] =
{
	NULL  // make the first one null for sure
};
qpic_t		*draw_backtile;

int			translate_texture[NUM_CLASSES];
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
qboolean is_PowerVR = false;

gltexture_t	gltextures[MAX_GLTEXTURES];
int			numgltextures;


void GL_Bind (int texnum)
{
	if (currenttexture == texnum)
		return;
	currenttexture = texnum;
	glfunc.glBindTexture_fp (GL_TEXTURE_2D, texnum);
}

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

#define MAX_SCRAPS		1
#define BLOCK_WIDTH		256
#define BLOCK_HEIGHT	256

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
#define MAX_CACHED_PICS 	256
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
	glpic_t *gl;

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
	glpic_t 	*gl;

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

	// point sample status bar
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
		if (glt->mipmap)
		{
			GL_Bind (glt->texnum);
			glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
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
	int		i;
	qpic_t	*cb,*mf;
	byte	*dest;
	int		x, y;
	char	ver[40];
	glpic_t *gl;
	char temp[MAX_QPATH];

	Cvar_RegisterVariable (&gl_round_down);
	Cvar_RegisterVariable (&gl_picmip);

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

	char_texture = GL_LoadTexture ("charset", 256, 128, draw_chars, false, true, 0);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	draw_smallchars = W_GetLumpName("tinyfont");
	for (i=0 ; i<128*32 ; i++)
		if (draw_smallchars[i] == 0)
			draw_smallchars[i] = 255;	// proper transparent color

	// now turn them into textures
	char_smalltexture = GL_LoadTexture ("smallcharset", 128, 32, draw_smallchars, false, true, 0);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	mf = (qpic_t *)COM_LoadTempFile("gfx/menu/bigfont2.lmp");
	for (i=0 ; i<160*80 ; i++)
		if (mf->data[i] == 0)
			mf->data[i] = 255;	// proper transparent color


	char_menufonttexture = GL_LoadTexture ("menufont", 160, 80, mf->data, false, true, 0);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	cb = (qpic_t *)COM_LoadTempFile ("gfx/menu/conback.lmp");
	if (!cb)
		Sys_Error ("Couldn't load gfx/menu/conback.lmp");
	SwapPic (cb);

	// hack the version number directly into the pic

	dest = cb->data + 320 - 43 + 320*186;
	sprintf (ver, "%4.2f", HEXEN2_VERSION);

//	sprintf (ver, "(gl %4.2f) %4.2f", (float)GLQUAKE_VERSION, (float)VERSION);
//	dest = cb->data + 320*186 + 320 - 11 - 8*strlen(ver);
	y = strlen(ver);
	for (x=0 ; x<y ; x++)
		Draw_CharToConback (ver[x], dest+(x<<3));

	gl = (glpic_t *)conback->data;
	gl->texnum = GL_LoadTexture ("conback", cb->width, cb->height, cb->data, false, false, 0);
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;
	conback->width = vid.conwidth;
	conback->height = vid.conheight;

	// save a texture slot for translated picture
	translate_texture[0] = texture_extension_number++;
	translate_texture[1] = texture_extension_number++;
	translate_texture[2] = texture_extension_number++;
	translate_texture[3] = texture_extension_number++;
	translate_texture[4] = texture_extension_number++;

	// save slots for scraps
	scrap_texnum = texture_extension_number;
	texture_extension_number += MAX_SCRAPS;

	//
	// get the other pics we need
	//
	for(i=MAX_DISC-1;i>=0;i--)
	{
		sprintf(temp,"gfx/menu/skull%d.lmp",i);
		draw_disc[i] = Draw_PicFromFile (temp);
	}

//	draw_disc = Draw_PicFromWad ("disc");
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
	int				row, col;
	float			frow, fcol, xsize,ysize;

	if (num == 32)
		return; 	// space

	num &= 511;

	if (y <= -8)
		return; 		// totally off screen

	row = num>>5;
	col = num&31;

	xsize = 0.03125;
	ysize = 0.0625;
	fcol = col*xsize;
	frow =row*ysize;

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
	int				row, col;
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
	glpic_t 		*gl;

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
		int x, y;
		
		for( x = 0; x < PLAYER_PIC_WIDTH; x++ )
			for( y = 0; y < PLAYER_PIC_HEIGHT; y++ )
			{
				trans[y * PLAYER_DEST_WIDTH + x] = d_8to24table[translation[menuplyr_pixels[setup_class-1][y * PLAYER_PIC_WIDTH + x]]];
			}
	}

/*
	glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, gl_alpha_format, PLAYER_DEST_WIDTH, PLAYER_DEST_HEIGHT,
		      0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	//	  glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, 1, 64, 64, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, menuplyr_pixels);

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
*/

	// See if the texture has already been loaded; if not, do it (Pa3PyX)
	if (!plyrtex[setup_class - 1][setup_top][setup_bottom]) {
		snprintf(texname, 19, "plyrmtex%i%i%i", setup_class, setup_top, setup_bottom);
		plyrtex[setup_class - 1][setup_top][setup_bottom] = GL_LoadTexture32(texname, PLAYER_DEST_WIDTH, PLAYER_DEST_HEIGHT, trans, false, true, 0);
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
	int				row, col;
	float			frow, fcol, xsize,ysize;
	int				add;

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
	Draw_Pic (0, lines-vid.height, conback);
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
		glfunc.glVertex2f_fp (bx,by);
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

	glfunc.glDrawBuffer_fp  (GL_FRONT);

	Draw_Pic (vid.width - 28, 0, draw_disc[index]);

	glfunc.glDrawBuffer_fp  (GL_BACK);
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
}

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
//	glfunc.glOrtho_fp  (0, 320, 200, 0, -99999, 99999);
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
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[1024], p2[1024];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);

		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
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

// Acts the same as glfunc.glTexImage2D_fp, except that it maps color into the
// current palette and uses paletteized textures.
static void fxPalTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
	static unsigned char dest_image[256*256];
	long i;
	long mip_width, mip_height;

	mip_width = width;
	mip_height = height;

	if( internalformat != 3 )
		Sys_Error( "fxPalTexImage2D: internalformat != 3" );
	for( i = 0; i < mip_width * mip_height; i++ )
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
//	glfunc.glTexImage2D_fp( target, level, 1, width, height, border, GL_LUMINANCE, GL_UNSIGNED_BYTE, dest_image );
	glfunc.glTexImage2D_fp( target, level, 1, width, height, border, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, dest_image );
/*	if( fxMarkPalTextureExtension )
		fxMarkPalTextureExtension();*/
}


/*
===============
GL_Upload32
===============
*/
void GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap, qboolean alpha)
{
	int			samples;
	static	unsigned	scaled[1024*512];	// [512*256];
	int			scaled_width, scaled_height;

	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	if (gl_round_down.value && scaled_width > width)
		scaled_width >>= 1;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;
	if (gl_round_down.value && scaled_height > height)
		scaled_height >>= 1;

	if ((scaled_width >> (int)gl_picmip.value) &&
		(scaled_height >> (int)gl_picmip.value))
	{
		scaled_width >>= (int)gl_picmip.value;
		scaled_height >>= (int)gl_picmip.value;
	}

	if (scaled_width > gl_max_size)
		scaled_width = gl_max_size;
	if (scaled_height > gl_max_size)
		scaled_height = gl_max_size;

	if (scaled_width * scaled_height > sizeof(scaled)/4)
		Sys_Error ("GL_LoadTexture: too big");

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

	texels += scaled_width * scaled_height;

#if 0	// DDOI
	// If you are on a 3Dfx card and your texture has no alpha, then download it
	// as a palettized texture to save memory.
	if( fxSetPaletteExtension && ( samples == 3 ) )
	{
		fxPalTexImage2D( GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled );
	}
	else
#endif
	{
		glfunc.glTexImage2D_fp (GL_TEXTURE_2D, 0, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}

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
			texels += scaled_width * scaled_height;

			// If you are on a 3Dfx card and your texture has no alpha, then download it
			// as a palettized texture to save memory.
#if 0	// DDOI
			if( fxSetPaletteExtension && ( samples == 3) )
			{
				fxPalTexImage2D (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
			}
			else
#endif
			{
				glfunc.glTexImage2D_fp (GL_TEXTURE_2D, miplevel, samples, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
			}
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

static	unsigned	trans[640*480]; 	// FIXME, temporary

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
	int		i, s;
	qboolean	noalpha;
	int		p;

	s = width*height;
	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
	if ((alpha || mode != 0) && mode != 10 )
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

	GL_Upload32 (trans, width, height, mipmap, alpha);
}

/*
================
GL_LoadTexture
================
*/
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int mode)
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
				// TODO - Maybe add the hash check some day
					Con_Printf ("GL_LoadTexture: reloading tex due to cache mismatch\n");
					glfunc.glDeleteTextures_fp (1, &(glt->texnum));
					glt->width = width;
					glt->height = height;
					glt->mipmap = mipmap;
					GL_Bind (glt->texnum);
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

	GL_Upload8 (data, width, height, mipmap, alpha, mode);

	texture_extension_number++;

	return texture_extension_number-1;
}

int GL_LoadTexture32 (char *identifier, int width, int height, unsigned *data, qboolean mipmap, qboolean alpha, int mode)
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
				// TODO - Maybe add the hash check some day
					Con_Printf ("GL_LoadTexture32: reloading tex due to cache mismatch\n");
					glfunc.glDeleteTextures_fp (1, &(glt->texnum));
					glt->width = width;
					glt->height = height;
					glt->mipmap = mipmap;
					GL_Bind (glt->texnum);
					GL_Upload32 (data, width, height, mipmap, alpha);
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

	GL_Upload32 (data, width, height, mipmap, alpha);

	texture_extension_number++;

	return texture_extension_number-1;
}

/*
===============
GL_Upload8
===============
*/
/*
void GL_UploadTrans8 (byte *data, int width, int height,  qboolean mipmap, byte Alpha)
{
	int			i, s;
	int			p;
	unsigned NewAlpha;

	NewAlpha = ((unsigned)Alpha)<<24;

	s = width*height;
	for (i=0 ; i<s ; i++)
	{
		p = data[i];
		trans[i] = d_8to24table[p];
		if (p != 255)
		{
			trans[i] &= 0x00ffffff;
			trans[i] |= NewAlpha;
		}
	}

	GL_Upload32 (trans, width, height, mipmap, true);
}
*/

/*
================
GL_LoadTransTexture
================
*/
/*int GL_LoadTransTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, byte Alpha)
{
	qboolean	noalpha;
	int			i, p, s;
	gltexture_t	*glt;

	// see if the texture is allready present
	if (identifier[0])
	{
		for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
		{
			if (!strcmp (identifier, glt->identifier))
			{
				if (width != glt->width || height != glt->height)
					Sys_Error ("GL_LoadTexture: cache mismatch");
				return gltextures[i].texnum;
			}
		}
	}
	else
		glt = &gltextures[numgltextures];
	numgltextures++;

	strcpy (glt->identifier, identifier);
	glt->texnum = texture_extension_number;
	glt->width = width;
	glt->height = height;
	glt->mipmap = mipmap;

	GL_Bind(texture_extension_number );

	GL_UploadTrans8 (data, width, height, mipmap, Alpha);

	texture_extension_number++;

	return texture_extension_number-1;
}
*/

/*
================
GL_LoadPicTexture
================
*/
int GL_LoadPicTexture (qpic_t *pic)
{
	return GL_LoadTexture ("", pic->width, pic->height, pic->data, false, true, 0);
}

/*
 * $Log: not supported by cvs2svn $
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
