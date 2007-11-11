/*
	glquake.h
	common glquake header

	$Id: glquake.h,v 1.67 2007-11-11 13:17:44 sezero Exp $
*/


#ifndef __GLQUAKE_H
#define __GLQUAKE_H


/* ====================================================================
   COMMON DEFINITIONS
   ================================================================== */

#define MAX_GLTEXTURES		2048
#define MAX_EXTRA_TEXTURES	156	/* 255-100+1 */
#define	MAX_CACHED_PICS		256
#define	MAX_LIGHTMAPS		64U

#define	GL_UNUSED_TEXTURE	((GLuint)-1)

#define	gl_solid_format		3
#define	gl_alpha_format		4

/* # of supported texture filter modes[] (gl_draw.c) */
#define	NUM_GL_FILTERS		6

/* defs for palettized textures	*/
#define	INVERSE_PAL_R_BITS	6
#define	INVERSE_PAL_G_BITS	6
#define	INVERSE_PAL_B_BITS	6
#define	INVERSE_PAL_TOTAL_BITS	(INVERSE_PAL_R_BITS + INVERSE_PAL_G_BITS + INVERSE_PAL_B_BITS)
#define	INVERSE_PAL_SIZE	(1 << INVERSE_PAL_TOTAL_BITS)

/* r_local.h defs		*/
#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works
					// out to about 1 pixel per triangle
#define MAX_SKIN_HEIGHT		480

#define BACKFACE_EPSILON	0.01

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)


/* ====================================================================
   ENDIANNESS: RGBA
   ================================================================== */

#if (BYTE_ORDER == BIG_ENDIAN)	/* R G B A */
#define	MASK_r		0xff000000
#define	MASK_g		0x00ff0000
#define	MASK_b		0x0000ff00
#define	MASK_a		0x000000ff
#define	SHIFT_r		24
#define	SHIFT_g		16
#define	SHIFT_b		8
#define	SHIFT_a		0
#elif (BYTE_ORDER == LITTLE_ENDIAN) /* A B G R */
#define	MASK_r		0x000000ff
#define	MASK_g		0x0000ff00
#define	MASK_b		0x00ff0000
#define	MASK_a		0xff000000
#define	SHIFT_r		0
#define	SHIFT_g		8
#define	SHIFT_b		16
#define	SHIFT_a		24
#elif (BYTE_ORDER == PDP_ENDIAN) /* G R A B */
#define	MASK_r		0x00ff0000
#define	MASK_g		0xff000000
#define	MASK_b		0x000000ff
#define	MASK_a		0x0000ff00
#define	SHIFT_r		16
#define	SHIFT_g		24
#define	SHIFT_b		0
#define	SHIFT_a		8
#endif

#define	MASK_rgb	(MASK_r|MASK_g|MASK_b)


/* ====================================================================
   TYPES
   ================================================================== */

/* texture types */
typedef struct
{
	GLuint		texnum;
	float	sl, tl, sh, th;
} glpic_t;

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	qpic_t		pic;
	byte		padding[32];	/* for appended glpic */
} cachepic_t;

typedef struct
{
	GLuint		texnum;
	char	identifier[MAX_QPATH];
	int		width, height;
	qboolean	mipmap;
//	unsigned short	crc;
	unsigned long	hash;
} gltexture_t;

/* texture filters */
typedef struct
{
	const char	*name;
	int	minimize, maximize;
} glmode_t;

/* particle enums and types: note that hexen2 and
   hexenworld versions of these are different!! */
#include "particle.h"


/* ====================================================================
   GLOBAL VARIABLES
   ================================================================== */

/* gl texture objects */
extern	GLuint		texture_extension_number;
extern	GLuint		currenttexture;
extern	GLuint		particletexture;
extern	GLuint		lightmap_textures;
extern	GLuint		playertextures[MAX_CLIENTS];
extern	GLuint		plyrtex[MAX_PLAYER_CLASS][16][16];	// player textures in multiplayer config screens
extern	GLuint		gl_extra_textures[MAX_EXTRA_TEXTURES];	// generic textures for models

/* the GL_Bind macro */
#define GL_Bind(texnum)	{					\
	if (currenttexture != (texnum)) {			\
		currenttexture = (texnum);			\
		glBindTexture_fp(GL_TEXTURE_2D,currenttexture);	\
	}							\
}

extern	int		gl_texlevel;
extern	int		numgltextures;
extern	qboolean	flush_textures;		// we can't detect mapname change early enough
						// in hw, so flush_textures is only for hexen2
extern	gltexture_t	gltextures[MAX_GLTEXTURES];

extern	int		gl_filter_min;
extern	int		gl_filter_max;
extern	float		gldepthmin, gldepthmax;
extern	int		glx, gly, glwidth, glheight;

extern	glmode_t	gl_texmodes[NUM_GL_FILTERS];

/* hardware-caps related globals */
extern	GLint		gl_max_size;
extern	qboolean	is_3dfx;
extern	qboolean	is8bit;
extern	qboolean	gl_mtexable;
extern	qboolean	have_stencil;

/* view origin */
extern	vec3_t		vup;
extern	vec3_t		vpn;
extern	vec3_t		vright;
extern	vec3_t		r_origin;

/* screen size info */
extern	refdef_t	r_refdef;
extern	vrect_t		scr_vrect;

extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	float		r_world_matrix[16];
extern	entity_t	r_worldentity;
extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t	*currententity;
extern	int		r_visframecount;	// ??? what difs?
extern	int		r_framecount;
extern	mplane_t	frustum[4];
extern	int		c_brush_polys, c_alias_polys;

/* palette stuff */
extern	int		ColorIndex[16];
extern	unsigned int	ColorPercent[16];
extern	float		RTint[256], GTint[256], BTint[256];
#if USE_HEXEN2_PALTEX_CODE
extern	unsigned char	*inverse_pal;
#else
extern	unsigned char	d_15to8table[65536];
#endif

/* global cvars */
extern	cvar_t	r_norefresh;
extern	cvar_t	r_drawentities;
extern	cvar_t	r_drawworld;
extern	cvar_t	r_drawviewmodel;
extern	cvar_t	r_speeds;
extern	cvar_t	r_waterwarp;
extern	cvar_t	r_fullbright;
extern	cvar_t	r_lightmap;
extern	cvar_t	r_shadows;
extern	cvar_t	r_mirroralpha;
extern	cvar_t	r_wateralpha;
extern	cvar_t	r_skyalpha;
extern	cvar_t	r_dynamic;
extern	cvar_t	r_novis;
extern	cvar_t	r_wholeframe;
extern	cvar_t	r_texture_external;

#if defined(H2W)
extern	cvar_t	r_netgraph;
extern	cvar_t	r_entdistance;
extern	cvar_t	r_teamcolor;
#endif	/* H2W */

extern	cvar_t	gl_playermip;

extern	cvar_t	gl_clear;
extern	cvar_t	gl_cull;
extern	cvar_t	gl_poly;
extern	cvar_t	gl_ztrick;
extern	cvar_t	gl_multitexture;
extern	cvar_t	gl_purge_maptex;
extern	cvar_t	gl_smoothmodels;
extern	cvar_t	gl_affinemodels;
extern	cvar_t	gl_polyblend;
extern	cvar_t	gl_keeptjunctions;
extern	cvar_t	gl_reporttjunctions;
extern	cvar_t	gl_flashblend;
extern	cvar_t	gl_nocolors;
extern	cvar_t	gl_waterripple;
extern	cvar_t	gl_waterwarp;
extern	cvar_t	gl_stencilshadow;
extern	cvar_t	gl_glows;
extern	cvar_t	gl_other_glows;
extern	cvar_t	gl_missile_glows;

extern	cvar_t	gl_coloredlight;
extern	cvar_t	gl_colored_dynamic_lights;
extern	cvar_t	gl_extra_dynamic_lights;
extern	cvar_t	gl_lightmapfmt;

/* other globals */
extern	int		gl_coloredstatic;	/* value of gl_coloredlight stored at level start */
extern	int		gl_lightmap_format;	/* value of gl_lightmapfmt stored at level start */
extern	qboolean	lightmap_modified[MAX_LIGHTMAPS];

extern	vec3_t		lightcolor;
extern	vec3_t		lightspot;

extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	byte		*playerTranslation;
extern	const int	color_offsets[MAX_PLAYER_CLASS];

extern	qboolean	mirror;
extern	mplane_t	*mirror_plane;
extern	int		mirrortexturenum;	/* quake texturenum, not gltexturenum */
extern	int		skytexturenum;		/* index in cl.loadmodel, not gl texture object */

extern	qboolean	envmap;
extern	qboolean	gl_dogamma;		/* should we use gl tricks for brightening. */


/* ====================================================================
   GLOBAL FUNCTIONS
   ================================================================== */

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

void GL_Set2D (void);

GLuint GL_LoadTexture (const char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int mode, qboolean rgba);
GLuint GL_LoadPicTexture (qpic_t *pic);
void D_ClearOpenGLTextures (int last_tex);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_DrawBrushModel (entity_t *e, qboolean Translucent);
void R_DrawWorld (void);
void R_RenderBrushPoly (msurface_t *fa, qboolean override);
void R_RotateForEntity (entity_t *e);
void R_StoreEfrags (efrag_t **ppefrag);

#if defined(QUAKE2)
void R_LoadSkys (void);
void R_DrawSkyBox (void);
void R_ClearSkyBox (void);
#endif	/* QUAKE2 */
void GL_SubdivideSurface (msurface_t *fa);
void EmitWaterPolys (msurface_t *fa);
void EmitBothSkyLayers (msurface_t *fa);
void R_DrawSkyChain (msurface_t *s);
void R_DrawWaterSurfaces (void);

void R_RenderDlights (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);
void R_AnimateLight(void);
int R_LightPoint (vec3_t p);
float R_LightPointColor (vec3_t p);
void GL_BuildLightmaps (void);
void GL_SetupLightmapFmt (qboolean check_cmdline);
void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr);

void R_InitParticleTexture (void);
#if defined(H2W)
void R_NetGraph (void);
void R_InitNetgraphTexture (void);
#endif	/* H2W */

void R_ReadPointFile_f (void);
void R_TranslatePlayerSkin (int playernum);

#endif	/* __GLQUAKE_H */

