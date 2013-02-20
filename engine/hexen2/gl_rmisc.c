/*
 * r_misc.c --
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

byte			*playerTranslation;
const int	color_offsets[MAX_PLAYER_CLASS] =
{
		2 * 14 * 256,
		0,
		1 * 14 * 256,
		2 * 14 * 256,
		2 * 14 * 256
#if defined(H2W)
		,
		2 * 14 * 256
#endif
};

cvar_t			gl_purge_maptex = {"gl_purge_maptex", "1", CVAR_ARCHIVE};
				// whether or not map-specific OGL textures
				// are purged on map change. default == yes

qboolean		flush_textures;
int			gl_texlevel;
extern int		menu_numcachepics;
extern cachepic_t	menu_cachepics[MAX_CACHED_PICS];

extern void R_InitBubble (void);

/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x, y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = (texture_t *) Hunk_AllocName (sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");

	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;

	for (m = 0; m < 4; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];

		for (y = 0; y < (16 >> m); y++)
		{
			for (x = 0; x < (16 >> m); x++)
			{
				if ( (y < (8 >> m)) ^ (x < (8 >> m)) )
					*dest++ = 0;
				else
					*dest++ = 0xff;
			}
		}
	}
}


#define	TEXSIZE		16	/* was 8 */
/*
static byte dottexture[TEXSIZE][TEXSIZE] =
{
	{0,1,1,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,1,1,0,0,0,0,0},
	{0,0,0,0,1,0,0,0},
	{0,0,0,0,1,0,0,1},
	{0,0,0,0,0,0,1,0},
	{0,0,0,1,0,0,0,1},
};
*/

static byte dottexture[TEXSIZE][TEXSIZE] =
{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//1
	{0,0,0,1,0,1,0,0,0,0,0,0,1,1,0,0},
	{0,0,0,0,1,0,0,0,0,0,0,1,1,1,1,0},
	{0,1,0,1,1,1,0,1,0,0,0,1,1,1,1,0},//4
	{0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0},
	{0,1,0,1,1,1,0,1,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0},//8
	{0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0},
	{0,0,0,0,0,0,0,0,0,1,1,1,0,1,0,0},
	{0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,0},
	{0,0,0,1,0,0,0,0,1,1,1,1,1,0,1,0},//12
	{0,1,1,1,0,0,0,0,1,1,0,1,1,0,1,0},
	{0,0,1,1,1,0,0,0,0,1,1,1,0,1,0,0},
	{0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},//16
};

void R_InitParticleTexture (void)
{
	int		x, y;
	byte	data[TEXSIZE][TEXSIZE][4];

	//
	// particle texture
	//
	for (x = 0; x < TEXSIZE; x++)
	{
		for (y = 0; y < TEXSIZE; y++)
		{
			data[y][x][0] = 255;
			data[y][x][1] = 255;
			data[y][x][2] = 255;
			data[y][x][3] = dottexture[x][y]*255;
		}
	}

	particletexture = GL_LoadTexture("", (byte *)data, TEXSIZE, TEXSIZE, TEX_ALPHA | TEX_RGBA | TEX_LINEAR);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void R_InitExtraTextures (void)
{
	int	i;

	for (i = 0; i < MAX_EXTRA_TEXTURES; i++)
		gl_extra_textures[i] = GL_UNUSED_TEXTURE;

	// see R_TranslatePlayerSkin() below
	glGenTextures_fp(MAX_CLIENTS, playertextures);
}

/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
static void R_TimeRefresh_f (void)
{
	int		i;
	float		start, stop, time;

	if (cls.state != ca_connected)
	{
		Con_Printf("Not connected to a server\n");
		return;
	}

	start = Sys_DoubleTime ();
	for (i = 0; i < 128; i++)
	{
		GL_BeginRendering(&glx, &gly, &glwidth, &glheight);
		r_refdef.viewangles[1] = i/128.0*360.0;
		R_RenderView ();
		GL_EndRendering ();
	}

	glFinish_fp ();
	stop = Sys_DoubleTime ();
	time = stop-start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);
}

/*
===============
R_Init
===============
*/
void R_Init (void)
{
	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);

	Cvar_RegisterVariable (&r_norefresh);
	Cvar_RegisterVariable (&r_lightmap);
	Cvar_RegisterVariable (&r_fullbright);
	Cvar_RegisterVariable (&r_waterwarp);
	Cvar_RegisterVariable (&r_drawentities);
	Cvar_RegisterVariable (&r_drawviewmodel);
	Cvar_RegisterVariable (&r_shadows);
	Cvar_RegisterVariable (&r_mirroralpha);
	Cvar_RegisterVariable (&r_wateralpha);
	Cvar_RegisterVariable (&r_skyalpha);
	Cvar_RegisterVariable (&r_dynamic);
	Cvar_RegisterVariable (&r_novis);
	Cvar_RegisterVariable (&r_speeds);
	Cvar_RegisterVariable (&r_wholeframe);
	Cvar_RegisterVariable (&r_texture_external);

	Cvar_RegisterVariable (&gl_clear);
	Cvar_RegisterVariable (&gl_cull);
	Cvar_RegisterVariable (&gl_smoothmodels);
	Cvar_RegisterVariable (&gl_affinemodels);
	Cvar_RegisterVariable (&gl_polyblend);
	Cvar_RegisterVariable (&gl_flashblend);
	Cvar_RegisterVariable (&gl_playermip);
	Cvar_RegisterVariable (&gl_nocolors);
	Cvar_RegisterVariable (&gl_waterripple);

	Cvar_RegisterVariable (&gl_ztrick);
	Cvar_RegisterVariable (&gl_zfix);
	Cvar_RegisterVariable (&gl_purge_maptex);

	Cvar_RegisterVariable (&gl_keeptjunctions);
	Cvar_RegisterVariable (&gl_reporttjunctions);
	Cvar_RegisterVariable (&gl_multitexture);

	Cvar_RegisterVariable (&gl_glows);
	Cvar_RegisterVariable (&gl_missile_glows);
	Cvar_RegisterVariable (&gl_other_glows);

	Cvar_RegisterVariable (&gl_coloredlight);
	Cvar_RegisterVariable (&gl_colored_dynamic_lights);
	Cvar_RegisterVariable (&gl_extra_dynamic_lights);

	R_InitBubble();

	R_InitParticles ();
	R_InitParticleTexture ();
	R_InitExtraTextures ();

	playerTranslation = (byte *)FS_LoadHunkFile ("gfx/player.lmp", NULL);
	if (!playerTranslation)
		Sys_Error ("Couldn't load gfx/player.lmp");
}

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
extern	byte	player_8bit_texels[MAX_PLAYER_CLASS][620*245];

void R_TranslatePlayerSkin (int playernum)
{
	int		top, bottom;
	byte		translate[256];
	unsigned int	translate32[256];
	unsigned int	i, j;
	qmodel_t	*model;
	aliashdr_t	*paliashdr;
	byte		*original;
	unsigned int	pixels[512*256], *out;
	unsigned int	scaled_width, scaled_height;
	int		inwidth, inheight;
	byte		*inrow;
	unsigned int	frac, fracstep;
	byte		*sourceA, *sourceB, *colorA, *colorB;
	int		playerclass = (int)cl.scores[playernum].playerclass;
	int		s;
//	char		texname[20];

	for (i = 0; i < 256; i++)
		translate[i] = i;

	top = (cl.scores[playernum].colors & 0xf0) >> 4;
	bottom = (cl.scores[playernum].colors & 15);

	if (top > 10)
		top = 0;
	if (bottom > 10)
		bottom = 0;

	top -= 1;
	bottom -= 1;

	colorA = playerTranslation + 256 + color_offsets[playerclass-1];
	colorB = colorA + 256;
	sourceA = colorB + 256 + (top * 256);
	sourceB = colorB + 256 + (bottom * 256);
	for (i = 0; i < 256; i++, colorA++, colorB++, sourceA++, sourceB++)
	{
		if (top >= 0 && (*colorA != 255))
			translate[i] = *sourceA;
		if (bottom >= 0 && (*colorB != 255))
			translate[i] = *sourceB;
	}

	//
	// locate the original skin pixels
	//
	model = cl_entities[1+playernum].model;
	if (!model)
		return;		// player doesn't have a model yet

	// class limit is mission pack dependant
	s = (gameflags & GAME_PORTALS) ? MAX_PLAYER_CLASS : MAX_PLAYER_CLASS - PORTALS_EXTRA_CLASSES;
	if (playerclass >= 1 && playerclass <= s)
		original = player_8bit_texels[playerclass-1];
	else	original = player_8bit_texels[0];

	paliashdr = (aliashdr_t *)Mod_Extradata (model);
	s = paliashdr->skinwidth * paliashdr->skinheight;
	if (s & 3)
		Sys_Error ("%s: s&3", __thisfunc__);

	for (i = 0; i < 256; i++)
		translate32[i] = d_8to24table[translate[i]];
	scaled_width  = gl_max_size < 512 ? gl_max_size : 512;
	scaled_height = gl_max_size < 256 ? gl_max_size : 256;

	// allow users to crunch sizes down even more if they want
	scaled_width >>= gl_playermip.integer;
	scaled_height >>= gl_playermip.integer;

	inwidth = paliashdr->skinwidth;
	inheight = paliashdr->skinheight;
	out = pixels;
	fracstep = inwidth*0x10000/scaled_width;
	for (i = 0; i < scaled_height; i++, out += scaled_width)
	{
		inrow = original + inwidth*(i*inheight/scaled_height);
		frac = fracstep >> 1;
		for (j = 0; j < scaled_width; j += 4)
		{
			out[j] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+1] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+2] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+3] = translate32[inrow[frac>>16]];
			frac += fracstep;
		}
	}

// playertextures doesn't like GL_LoadTexture() and its associated glDeleteTextures()
// call, not sure why for now, so I have to do this the old way until I figure it out.
//	q_snprintf(texname, 19, "player%i", playernum);
//	playertextures[playernum] = GL_LoadTexture(texname, (byte *)pixels, scaled_width, scaled_height, TEX_RGBA|TEX_LINEAR);
	GL_Bind(playertextures[playernum]);
	glTexImage2D_fp(GL_TEXTURE_2D, 0, gl_solid_format, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*
===============
R_NewMap
===============
*/
void R_NewMap (void)
{
	int		i;

	for (i = 0; i < 256; i++)
		d_lightstylevalue[i] = 264;		// normal light value

	memset (&r_worldentity, 0, sizeof(r_worldentity));
	r_worldentity.model = cl.worldmodel;

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for (i = 0; i < cl.worldmodel->numleafs; i++)
		cl.worldmodel->leafs[i].efrags = NULL;

	r_viewleaf = NULL;
	R_ClearParticles ();

	GL_BuildLightmaps ();

	// identify sky texture
	skytexturenum = -1;
	mirrortexturenum = -1;
	for (i = 0; i < cl.worldmodel->numtextures; i++)
	{
		if (!cl.worldmodel->textures[i])
			continue;
		if (!strncmp(cl.worldmodel->textures[i]->name,"sky",3) )
			skytexturenum = i;
		if (!strncmp(cl.worldmodel->textures[i]->name,"window02_1",10) )
			mirrortexturenum = i;
	}
#ifdef QUAKE2
	R_LoadSkys ();
#endif
}


/* D_ClearOpenGLTexture
   this procedure (called by Host_ClearMemory/SV_SpawnServer in hexen2 on new
   map, or by CL_ClearState/CL_ParseServerData in HW on new connection) will
   purge all OpenGL textures beyond static ones (console, menu, etc, whatever
   was loaded at initialization time). This will save a lot of video memory,
   because the textures won't keep accumulating from map to map, thus bloating
   more and more the more time the client is running, which gets pretty nasty
   on 8-16-32M machines with OpenGL drivers like nVidia, which cache all
   textures in system memory. (Pa3PyX)
*/
void D_ClearOpenGLTextures (int last_tex)
{
	int		i;

	// Delete OpenGL textures
	for (i = last_tex; i < numgltextures; i++)
		glDeleteTextures_fp(1, &(gltextures[i].texnum));

	memset(&(gltextures[last_tex]), 0, (numgltextures - last_tex) * sizeof(gltexture_t));
	numgltextures = last_tex;

	if (currenttexture >= (GLuint)last_tex)
		currenttexture = GL_UNUSED_TEXTURE;

	// Clear menu pic cache
	memset(menu_cachepics, 0, menu_numcachepics * sizeof(cachepic_t));
	menu_numcachepics = 0;

	Con_DPrintf ("Purged OpenGL textures\n");
}

void D_FlushCaches (void)
{
	if (numgltextures - gl_texlevel > 0 && flush_textures && gl_purge_maptex.integer)
		D_ClearOpenGLTextures (gl_texlevel);
}

