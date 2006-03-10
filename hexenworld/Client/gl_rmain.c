/*
	gl_main.c

	$Id: gl_rmain.c,v 1.30 2006-03-10 08:08:46 sezero Exp $
*/


#include "quakedef.h"

entity_t	r_worldentity;

qboolean	r_cache_thrash;		// compatability

vec3_t		modelorg, r_entorigin;
entity_t	*currententity;

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

mplane_t	frustum[4];

int			c_brush_polys, c_alias_polys;

qboolean	envmap;				// true during envmap command capture 

int			currenttexture = -1;	// to avoid unnecessary texture sets

int			particletexture;	// little dot for particles
int			playertextures[16];	// up to 16 color translated skins
int			gl_extra_textures[MAX_EXTRA_TEXTURES];   // generic textures for models

int			mirrortexturenum;	// quake texturenum, not gltexturenum
qboolean	mirror;
mplane_t	*mirror_plane;

float		model_constant_alpha;

float		r_time1;
float		r_lasttime1 = 0;

extern model_t *player_models[MAX_PLAYER_CLASS];

//
// view origin
//
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float	r_world_matrix[16];
float	r_base_world_matrix[16];

//
// screen size info
//
refdef_t	r_refdef;

mleaf_t		*r_viewleaf, *r_oldviewleaf;

texture_t	*r_notexture_mip;

int		d_lightstylevalue[256];	// 8.8 fraction of base light value


void R_MarkLeaves (void);

extern	cvar_t	v_gamma;
extern	qboolean gl_dogamma;

cvar_t	r_norefresh = {"r_norefresh","0"};
cvar_t	r_drawentities = {"r_drawentities","1"};
cvar_t	r_drawviewmodel = {"r_drawviewmodel","1"};
cvar_t	r_speeds = {"r_speeds","0"};
cvar_t	r_fullbright = {"r_fullbright","0"};
cvar_t	r_lightmap = {"r_lightmap","0"};
cvar_t	r_shadows = {"r_shadows","0"};
cvar_t	r_mirroralpha = {"r_mirroralpha","1"};
cvar_t	r_wateralpha = {"r_wateralpha","0.33", true};
cvar_t	r_skyalpha = {"r_skyalpha", "0.67", true};
cvar_t	r_dynamic = {"r_dynamic","1"};
cvar_t	r_novis = {"r_novis","0"};
cvar_t	r_netgraph = {"r_netgraph","0"};
cvar_t	r_entdistance = {"r_entdistance", "0", true};
cvar_t	gl_clear = {"gl_clear","0"};
cvar_t	gl_cull = {"gl_cull","1"};
cvar_t	gl_multitexture = {"gl_multitexture","0",true};
cvar_t	gl_smoothmodels = {"gl_smoothmodels","1"};
cvar_t	gl_affinemodels = {"gl_affinemodels","0"};
cvar_t	gl_polyblend = {"gl_polyblend","1"};
cvar_t	gl_flashblend = {"gl_flashblend","0"};
cvar_t	gl_playermip = {"gl_playermip","0"};
cvar_t	gl_nocolors = {"gl_nocolors","0"};
cvar_t	gl_keeptjunctions = {"gl_keeptjunctions","1", true};
cvar_t	gl_reporttjunctions = {"gl_reporttjunctions","0"};
cvar_t	gl_waterripple = {"gl_waterripple", "2", true};
cvar_t	gl_waterwarp = {"gl_waterwarp", "0", true};
cvar_t	r_teamcolor = {"r_teamcolor", "187", true};
cvar_t	gl_stencilshadow = {"gl_stencilshadow", "0",true};
cvar_t	gl_glows = {"gl_glows","1",true};
cvar_t	gl_other_glows = {"gl_other_glows","0",true};
cvar_t	gl_missile_glows = {"gl_missile_glows","1",true};

cvar_t	gl_coloredlight = {"gl_coloredlight","0",true};
cvar_t	gl_colored_dynamic_lights = {"gl_colored_dynamic_lights","0",true};
cvar_t	gl_extra_dynamic_lights = {"gl_extra_dynamic_lights","0",true};

extern	cvar_t	gl_ztrick;
extern	cvar_t	scr_fov;
static qboolean AlwaysDrawModel;

static void R_RotateForEntity2(entity_t *e);


// idea originally nicked from LordHavoc,
// re-worked + extended - muff 5 Feb 2001
// called from polyblend
void GL_DoGamma()
{
/* This trick is useful if normal ways of gamma adjustment fail:
   In case of 3dfx Voodoo1/2/Rush, we can't use 3dfx specific
   extensions in unix, so this can be our friend at cost of 4-5 fps */
	if (v_gamma.value <0.2)
		v_gamma.value=0.2;
	if (v_gamma.value>= 1) {
		v_gamma.value = 1;
		return;
	}

	// believe it or not this actually does brighten the picture!!
	glBlendFunc_fp (  GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f_fp (1, 1, 1, v_gamma.value);

	glBegin_fp (GL_QUADS);
	glVertex3f_fp (10, 100, 100);
	glVertex3f_fp (10, -100, 100);
	glVertex3f_fp (10, -100, -100);
	glVertex3f_fp (10, 100, -100);

	// if we do this twice, we double the brightening
	// effect for a wider range of gamma's
/*
	glVertex3f_fp (11, 100, 100);
	glVertex3f_fp (11, -100, 100);
	glVertex3f_fp (11, -100, -100);
	glVertex3f_fp (11, 100, -100);
*/
	glEnd_fp ();
}


/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int		i;

	for (i=0 ; i<4 ; i++)
		if (BoxOnPlaneSide (mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


void R_RotateForEntity (entity_t *e)
{
    glTranslatef_fp (e->origin[0],  e->origin[1],  e->origin[2]);

    glRotatef_fp (e->angles[1],  0, 0, 1);
    glRotatef_fp (-e->angles[0],  0, 1, 0);
    glRotatef_fp (e->angles[2],  1, 0, 0);	//RDM: switched sign so it matches software
}

//==========================================================================
//
// R_RotateForEntity2
//
// Same as R_RotateForEntity(), but checks for EF_ROTATE and modifies
// yaw appropriately.
//
//==========================================================================

static void R_RotateForEntity2(entity_t *e)
{
	float	forward;
	float	yaw, pitch;
	vec3_t			angles;

	glTranslatef_fp(e->origin[0], e->origin[1], e->origin[2]);

	if (e->model->flags & EF_FACE_VIEW)
	{
		VectorSubtract(e->origin,r_origin,angles);
		VectorSubtract(r_origin,e->origin,angles);
		VectorNormalize(angles);

		if (angles[1] == 0 && angles[0] == 0)
		{
			yaw = 0;
			if (angles[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(angles[1], angles[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (angles[0]*angles[0] + angles[1]*angles[1]);
			pitch = (int) (atan2(angles[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;

		glRotatef_fp(-angles[0], 0, 1, 0);
		glRotatef_fp(angles[1], 0, 0, 1);
//		glRotatef_fp(-angles[2], 1, 0, 0);
		glRotatef_fp(-e->angles[2], 1, 0, 0);
	}
	else 
	{
		if (e->model->flags & EF_ROTATE)
		{
			glRotatef_fp(anglemod((e->origin[0]+e->origin[1])*0.8
				+(108*cl.time)), 0, 0, 1);
		}
		else
		{
			glRotatef_fp(e->angles[1], 0, 0, 1);
		}

		glRotatef_fp(-e->angles[0], 0, 1, 0);
		glRotatef_fp(-e->angles[2], 1, 0, 0);

		// For clientside rotation stuff
		glRotatef_fp(e->angleAdd[0], 0, 1, 0);
		glRotatef_fp(e->angleAdd[1], 0, 0, 1);
		glRotatef_fp(e->angleAdd[2], 1, 0, 0);
	}
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/

/*
================
R_GetSpriteFrame
================
*/
mspriteframe_t *R_GetSpriteFrame (entity_t *curr_ent)
{
	msprite_t		*psprite;
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	psprite = curr_ent->model->cache.data;
	frame = curr_ent->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cl.time + curr_ent->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}


/*
=================
R_DrawSpriteModel

=================
*/
void R_DrawSpriteModel (entity_t *e)
{
	vec3_t	point;
	mspriteframe_t	*frame;
	float		*up, *right;
	vec3_t		v_forward, v_right, v_up;
	msprite_t		*psprite;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	if (currententity->drawflags & DRF_TRANSLUCENT)
	{
		glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable_fp( GL_BLEND );
		glColor4f_fp (1,1,1,r_wateralpha.value);
	}
	else if (currententity->model->flags & EF_TRANSPARENT)
	{
		glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable_fp( GL_BLEND );
		glColor3f_fp(1,1,1);
	}
	else
	{
//		glColor3f_fp(1,1,1);
		glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable_fp( GL_BLEND );
		glColor3f_fp(1,1,1);
	}

	frame = R_GetSpriteFrame (e);
	psprite = currententity->model->cache.data;

	if (psprite->type == SPR_ORIENTED)
	{	// bullet marks on walls
		AngleVectors (currententity->angles, v_forward, v_right, v_up);
		up = v_up;
		right = v_right;
	}
	else
	{	// normal sprite
		up = vup;
		right = vright;
	}

	GL_Bind(frame->gl_texturenum);

	glBegin_fp (GL_QUADS);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexCoord2f_fp (0, 1);
	VectorMA (e->origin, frame->down, up, point);
	VectorMA (point, frame->left, right, point);
	glVertex3fv_fp (point);

	glTexCoord2f_fp (0, 0);
	VectorMA (e->origin, frame->up, up, point);
	VectorMA (point, frame->left, right, point);
	glVertex3fv_fp (point);

	glTexCoord2f_fp (1, 0);
	VectorMA (e->origin, frame->up, up, point);
	VectorMA (point, frame->right, right, point);
	glVertex3fv_fp (point);

	glTexCoord2f_fp (1, 1);
	VectorMA (e->origin, frame->down, up, point);
	VectorMA (point, frame->right, right, point);
	glVertex3fv_fp (point);

	glEnd_fp ();

	//restore tex parms
	// replaced GL_REPEAT with GL_CLAMP below (courtesy of Pa3Pyx)
	// fixing the demoness flame's "lines" bug S.A
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glDisable_fp( GL_BLEND );
}

/*
=============================================================

  ALIAS MODELS

=============================================================
*/


#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

vec3_t	shadevector;
float	shadelight, ambientlight;

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] = {
#include "anorm_dots.h"
};

float	shadelightcolor[4];
float	*shadedots = r_avertexnormal_dots[0];

int	lastposenum;

/*
=============
GL_DrawAliasFrame
=============
*/
extern float RTint[256],GTint[256],BTint[256];

void GL_DrawAliasFrame (aliashdr_t *paliashdr, int posenum)
{
	float 		l;
	int		i;
	trivertx_t	*verts;
	int		*order;
	int		count;
	float		r,g,b;
	char		client_team[16], this_team[16];
	qboolean	OnTeam = false;
	byte		ColorShade;
	int			my_team, ve_team;

	lastposenum = posenum;

	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts += posenum * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	ColorShade = currententity->colorshade;

	i = currententity->scoreboard - cl.players;
	if (i >= 0 && i<MAX_CLIENTS)
	{
		my_team = cl.players[cl.playernum].siege_team;
		ve_team = cl.players[i].siege_team;
		if((ambientlight+shadelight)>50||(cl_siege&&my_team==ve_team))
			cl.players[i].shownames_off = false;
		else
			cl.players[i].shownames_off = true;
		if(cl_siege)
		{
			if(cl.players[cl.playernum].playerclass==CLASS_DWARF&&currententity->skinnum==101)
			{
				ColorShade = 133;
				if(ambientlight<128)
					shadelight += (128 - ambientlight);
				cl.players[i].shownames_off = false;
			}
			else if(cl.players[cl.playernum].playerclass==CLASS_DWARF&&(ambientlight+shadelight)<51)//OOps, use darkmaps in GL
			{
				ColorShade = 128 + (int)((ambientlight+shadelight)/5);
				shadelight += (51 - ambientlight);
				cl.players[i].shownames_off = false;
			}
			else if(ve_team==ST_DEFENDER)
			{//tint gold since we can't have seperate skins
				OnTeam = true;
				ColorShade = 165;
			}
		}
		else
		{
			strncpy(client_team, Info_ValueForKey(cl.players[cl.playernum].userinfo, "team"), 16);
			client_team[15] = 0;
			if (client_team[0])
			{
				strncpy(this_team, Info_ValueForKey(cl.players[i].userinfo, "team"), 16);
				this_team[15] = 0;
				if (Q_strcasecmp(client_team, this_team) == 0)
				{
					OnTeam = true;
					ColorShade = r_teamcolor.value;
				}
			}
		}
	}

	if (ColorShade)
	{
		r = RTint[ColorShade];
		g = GTint[ColorShade];
		b = BTint[ColorShade];
	}
	else
		r = g = b = 1;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin_fp (GL_TRIANGLE_FAN);
		}
		else
			glBegin_fp (GL_TRIANGLE_STRIP);

		do
		{
			// texture coordinates come from the draw list
			glTexCoord2f_fp (((float *)order)[0], ((float *)order)[1]);
			order += 2;

			// normals and vertexes come from the frame list

			if (gl_lightmap_format == GL_RGBA)
			{
				l = shadedots[verts->lightnormalindex];
				glColor4f_fp (l * shadelightcolor[0], l * shadelightcolor[1], l * shadelightcolor[2], model_constant_alpha);
			}
			else
			{
				l = shadedots[verts->lightnormalindex] * shadelight;
				glColor4f_fp (r*l, g*l, b*l, model_constant_alpha);
			}

			glVertex3f_fp (verts->v[0], verts->v[1], verts->v[2]);
			verts++;
		} while (--count);

		glEnd_fp ();
	}
}


/*
=============
GL_DrawAliasShadow
=============
*/
extern	vec3_t			lightspot;

void GL_DrawAliasShadow (aliashdr_t *paliashdr, int posenum)
{
	trivertx_t	*verts;
	int		*order;
	vec3_t		point;
	float		height, lheight;
	int		count;

	lheight = currententity->origin[2] - lightspot[2];

	height = 0;
	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts += posenum * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	height = -lheight + 1.0;

	if (have_stencil == true && gl_stencilshadow.value != 0)
	{
		glEnable_fp(GL_STENCIL_TEST);
		glStencilFunc_fp(GL_EQUAL,1,2);
		glStencilOp_fp(GL_KEEP,GL_KEEP,GL_INCR);
	}

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin_fp (GL_TRIANGLE_FAN);
		}
		else
			glBegin_fp (GL_TRIANGLE_STRIP);

		do
		{
			// texture coordinates come from the draw list
			// (skipped for shadows) glTexCoord2fv_fp ((float *)order);
			order += 2;

			// normals and vertexes come from the frame list
			point[0] = verts->v[0] * paliashdr->scale[0] + paliashdr->scale_origin[0];
			point[1] = verts->v[1] * paliashdr->scale[1] + paliashdr->scale_origin[1];
			point[2] = verts->v[2] * paliashdr->scale[2] + paliashdr->scale_origin[2];

			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
//			height -= 0.001;
			glVertex3fv_fp (point);

			verts++;
		} while (--count);

		glEnd_fp ();
	}	

	if (have_stencil == true && gl_stencilshadow.value != 0)
		glDisable_fp(GL_STENCIL_TEST);
}



/*
=================
R_SetupAliasFrame

=================
*/
void R_SetupAliasFrame (int frame, aliashdr_t *paliashdr)
{
	int				pose, numposes;
	float			interval;

	if ((frame >= paliashdr->numframes) || (frame < 0))
	{
		Con_DPrintf ("R_AliasSetupFrame: no such frame %d\n", frame);
		frame = 0;
	}

	pose = paliashdr->frames[frame].firstpose;
	numposes = paliashdr->frames[frame].numposes;

	if (numposes > 1)
	{
		interval = paliashdr->frames[frame].interval;
		pose += (int)(cl.time / interval) % numposes;
	}

	GL_DrawAliasFrame (paliashdr, pose);
}



/*
=================
R_DrawAliasModel

=================
*/
void R_DrawAliasModel (entity_t *e)
{
	int		i;
	int		lnum;
	vec3_t		dist;
	float		add;
	model_t		*clmodel;
	vec3_t		mins, maxs;
	aliashdr_t	*paliashdr;
	float		an;
	int		anim;
	static float	tmatrix[3][4];
	float		entScale;
	float		xyfact = 1.0, zfact = 1.0; // avoid compiler warning
	qpic_t		*stonepic;
	glpic_t		*gl;
	char		temp[80];
	int		mls;
	int		*lpc;
	vec3_t		adjust_origin;

	clmodel = currententity->model;

	VectorAdd (currententity->origin, clmodel->mins, mins);
	VectorAdd (currententity->origin, clmodel->maxs, maxs);

	if (!AlwaysDrawModel && R_CullBox (mins, maxs))
		return;


	VectorCopy (currententity->origin, r_entorigin);
	VectorSubtract (r_origin, r_entorigin, modelorg);

	//
	// get lighting information
	//

	VectorCopy(currententity->origin, adjust_origin);
	adjust_origin[2] += (currententity->model->mins[2] + currententity->model->maxs[2]) / 2;
	ambientlight = shadelight = R_LightPoint (adjust_origin);

	if (gl_lightmap_format == GL_RGBA)
	{	// get lighting information
		lpc = R_LightPointColour (adjust_origin);
		shadelightcolor[0] = (float) lpc[0];
		shadelightcolor[1] = (float) lpc[1];
		shadelightcolor[2] = (float) lpc[2];
		shadelightcolor[3] = (float) lpc[3];
		ambientlight = shadelightcolor[3];
	}

	// always give the gun some light
	if (e == &cl.viewent && ambientlight < 24)
		ambientlight = shadelight = 24;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if (cl_dlights[lnum].die >= cl.time)
		{
			VectorSubtract (currententity->origin,
							cl_dlights[lnum].origin,
							dist);
			add = cl_dlights[lnum].radius - Length(dist);

			if (add > 0)
			{
				ambientlight += add;
				//ZOID models should be affected by dlights as well
				shadelight += add;
				shadelightcolor[0] += (cl_dlights[lnum].color[0] * add);
				shadelightcolor[1] += (cl_dlights[lnum].color[1] * add);
				shadelightcolor[2] += (cl_dlights[lnum].color[2] * add);
				shadelightcolor[3] += add;
			}
		}
	}

	// clamp lighting so it doesn't overbright as much
	if (ambientlight > 128)
		ambientlight = 128;
	if (ambientlight + shadelight > 192)
		shadelight = 192 - ambientlight;

	mls = currententity->drawflags&MLS_MASKIN;
	if(currententity->model->flags&EF_ROTATE)
	{
		ambientlight = shadelight =
			60+34+sin(currententity->origin[0]+currententity->origin[1]
				+(cl.time*3.8))*34;
	}
	else if (mls == MLS_ABSLIGHT)
	{
		shadelightcolor[0] = shadelightcolor[1] = shadelightcolor[2] = ambientlight = shadelight = currententity->abslight;
	}
	else if (mls != MLS_NONE)
	{ // Use a model light style (25-30)
		shadelightcolor[0] = shadelightcolor[1] = shadelightcolor[2] = ambientlight = shadelight = d_lightstylevalue[24+mls]/2;
	}

	shadedots = r_avertexnormal_dots[((int)(e->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	shadelight = shadelight / 200.0;

	VectorScale(shadelightcolor, 1.0f / 200.0f, shadelightcolor);

	an = e->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);

	//
	// locate the proper data
	//
	paliashdr = (aliashdr_t *)Mod_Extradata (currententity->model);

	c_alias_polys += paliashdr->numtris;

	//
	// draw all the triangles
	//

	glPushMatrix_fp ();
	R_RotateForEntity2(e);

	if(currententity->scale != 0 && currententity->scale != 100)
	{
		entScale = (float)currententity->scale/100.0;
		switch(currententity->drawflags&SCALE_TYPE_MASKIN)
		{
		case SCALE_TYPE_UNIFORM:
			tmatrix[0][0] = paliashdr->scale[0]*entScale;
			tmatrix[1][1] = paliashdr->scale[1]*entScale;
			tmatrix[2][2] = paliashdr->scale[2]*entScale;
			xyfact = zfact = (entScale-1.0)*127.95;
			break;
		case SCALE_TYPE_XYONLY:
			tmatrix[0][0] = paliashdr->scale[0]*entScale;
			tmatrix[1][1] = paliashdr->scale[1]*entScale;
			tmatrix[2][2] = paliashdr->scale[2];
			xyfact = (entScale-1.0)*127.95;
			zfact = 1.0;
			break;
		case SCALE_TYPE_ZONLY:
			tmatrix[0][0] = paliashdr->scale[0];
			tmatrix[1][1] = paliashdr->scale[1];
			tmatrix[2][2] = paliashdr->scale[2]*entScale;
			xyfact = 1.0;
			zfact = (entScale-1.0)*127.95;
			break;
		}
		switch(currententity->drawflags&SCALE_ORIGIN_MASKIN)
		{
		case SCALE_ORIGIN_CENTER:
			tmatrix[0][3] = paliashdr->scale_origin[0]-paliashdr->scale[0]*xyfact;
			tmatrix[1][3] = paliashdr->scale_origin[1]-paliashdr->scale[1]*xyfact;
			tmatrix[2][3] = paliashdr->scale_origin[2]-paliashdr->scale[2]*zfact;
			break;
		case SCALE_ORIGIN_BOTTOM:
			tmatrix[0][3] = paliashdr->scale_origin[0]-paliashdr->scale[0]*xyfact;
			tmatrix[1][3] = paliashdr->scale_origin[1]-paliashdr->scale[1]*xyfact;
			tmatrix[2][3] = paliashdr->scale_origin[2];
			break;
		case SCALE_ORIGIN_TOP:
			tmatrix[0][3] = paliashdr->scale_origin[0]-paliashdr->scale[0]*xyfact;
			tmatrix[1][3] = paliashdr->scale_origin[1]-paliashdr->scale[1]*xyfact;
			tmatrix[2][3] = paliashdr->scale_origin[2]-paliashdr->scale[2]*zfact*2.0;
			break;
		}
	}
	else
	{
		tmatrix[0][0] = paliashdr->scale[0];
		tmatrix[1][1] = paliashdr->scale[1];
		tmatrix[2][2] = paliashdr->scale[2];
		tmatrix[0][3] = paliashdr->scale_origin[0];
		tmatrix[1][3] = paliashdr->scale_origin[1];
		tmatrix[2][3] = paliashdr->scale_origin[2];
	}

	if(clmodel->flags&EF_ROTATE)
	{ // Floating motion
		tmatrix[2][3] += sin(currententity->origin[0]
			+currententity->origin[1]+(cl.time*3))*5.5;
	}

// [0][3] [1][3] [2][3]
//	glTranslatef_fp (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
	glTranslatef_fp (tmatrix[0][3],tmatrix[1][3],tmatrix[2][3]);
// [0][0] [1][1] [2][2]
//	glScalef_fp (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);
	glScalef_fp (tmatrix[0][0],tmatrix[1][1],tmatrix[2][2]);

	if ((currententity->model->flags & EF_SPECIAL_TRANS))
	{
		glEnable_fp (GL_BLEND);
		glBlendFunc_fp (GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
//		glColor3f_fp( 1,1,1);
		model_constant_alpha = 1.0f;
		glDisable_fp( GL_CULL_FACE );
	}
	else if (currententity->drawflags & DRF_TRANSLUCENT)
	{
		glEnable_fp (GL_BLEND);
		glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glColor4f_fp( 1,1,1,r_wateralpha.value);
		model_constant_alpha = r_wateralpha.value;
	}
	else if ((currententity->model->flags & EF_TRANSPARENT))
	{
		glEnable_fp (GL_BLEND);
		glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glColor3f_fp( 1,1,1);
		model_constant_alpha = 1.0f;
	}
	else if ((currententity->model->flags & EF_HOLEY))
	{
		glEnable_fp (GL_BLEND);
		glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//		glColor3f_fp( 1,1,1);
		model_constant_alpha = 1.0f;
	}
	else
	{
		glColor3f_fp( 1,1,1);
		model_constant_alpha = 1.0f;
	}

//	if(cl.players[currententity->scoreboard - cl.players].siege_team==ST_DEFENDER)
//		currententity->skinnum = cl.players[currententity->scoreboard - cl.players].playerclass+110;

	if (currententity->skinnum >= 100)
	{
		if (currententity->skinnum > 255) 
		{
			Sys_Error ("skinnum > 255");
		}

		if (gl_extra_textures[currententity->skinnum-100] == -1)  // Need to load it in
		{
			sprintf(temp,"gfx/skin%d.lmp",currententity->skinnum);
			stonepic = Draw_CachePic(temp);
			gl = (glpic_t *)stonepic->data;
			gl_extra_textures[currententity->skinnum-100] = gl->texnum;
		}

		GL_Bind(gl_extra_textures[currententity->skinnum-100]);
	}
	else
	{
		anim = (int)(cl.time*10) & 3;
		GL_Bind(paliashdr->gl_texturenum[currententity->skinnum][anim]);

		// we can't dynamically colormap textures, so they are cached
		// seperately for the players.  Heads are just uncolored.
	
		if (currententity->colormap != vid.colormap && !gl_nocolors.value)
		{//FIXME? What about Demoness and Dwarf?
			if (currententity->model == player_models[0] ||
			    currententity->model == player_models[1] ||
			    currententity->model == player_models[2] ||
			    currententity->model == player_models[3])
			{
				i = currententity->scoreboard - cl.players;
//				if (currententity->scoreboard && !currententity->scoreboard->skin)
//				{
//					//Skin_Find(currententity->scoreboard);
//					R_TranslatePlayerSkin(i);
//				}
				if (i >= 0 && i<MAX_CLIENTS)
				{
					if (!cl.players[i].Translated)
					{
						R_TranslatePlayerSkin(i);
					}

					GL_Bind(playertextures[i]);
				}
			}
		}
	}

	if (gl_smoothmodels.value)
		glShadeModel_fp (GL_SMOOTH);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (gl_affinemodels.value)
		glHint_fp (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	R_SetupAliasFrame (currententity->frame, paliashdr);
	if ((currententity->drawflags & DRF_TRANSLUCENT) ||
		(currententity->model->flags & EF_SPECIAL_TRANS))
		glDisable_fp (GL_BLEND);

	if ((currententity->model->flags & EF_TRANSPARENT))
		glDisable_fp (GL_BLEND);

	if ((currententity->model->flags & EF_HOLEY))
		glDisable_fp (GL_BLEND);

	if ((currententity->model->flags & EF_SPECIAL_TRANS))
	{
		glEnable_fp( GL_CULL_FACE );
	}

	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glShadeModel_fp (GL_FLAT);
	if (gl_affinemodels.value)
		glHint_fp (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glPopMatrix_fp ();

	if (r_shadows.value)
	{
		glPushMatrix_fp ();
		R_RotateForEntity2 (e);
		glDisable_fp (GL_TEXTURE_2D);
		glEnable_fp (GL_BLEND);
		glColor4f_fp (0,0,0,0.5);
		glDepthMask_fp (0);	// prevent Z fighting
		GL_DrawAliasShadow (paliashdr, lastposenum);
		glDepthMask_fp (1);
		glEnable_fp (GL_TEXTURE_2D);
		glDisable_fp (GL_BLEND);
		glColor4f_fp (1,1,1,1);
		glPopMatrix_fp ();
	}

}

//==================================================================================

typedef struct sortedent_s {
	entity_t *ent;
	vec_t len;
} sortedent_t;

sortedent_t     cl_transvisedicts[MAX_VISEDICTS];
sortedent_t		cl_transwateredicts[MAX_VISEDICTS];

int				cl_numtransvisedicts;
int				cl_numtranswateredicts;

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList (void)
{
	int			i;
	qboolean	item_trans;
	mleaf_t		*pLeaf;
	vec3_t		diff;
	int			test_length, calc_length;

	cl_numtransvisedicts=0;
	cl_numtranswateredicts=0;

	if (!r_drawentities.value)
		return;

	if (r_entdistance.value <= 0)
	{
		test_length = 9999*9999;
	}
	else
	{
		test_length = r_entdistance.value * r_entdistance.value;
	}

	// draw sprites seperately, because of alpha blending
	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = &cl_visedicts[i];

//		if(currententity->drawflags&5)//MLS_INVIS - but dwarf can see
//			if(cl.v.playerclass!=CLASS_DWARF)
//				continue;

		switch (currententity->model->type)
		{
		case mod_alias:
			VectorSubtract(currententity->origin, r_origin, diff);
			calc_length = (diff[0]*diff[0]) + (diff[1]*diff[1]) + (diff[2]*diff[2]);
			if (calc_length > test_length)
			{
				continue;
			}

			item_trans = ((currententity->drawflags & DRF_TRANSLUCENT) ||
						  (currententity->model->flags & (EF_TRANSPARENT|EF_HOLEY|EF_SPECIAL_TRANS))) != 0;
			if (!item_trans)
				R_DrawAliasModel (currententity);
			break;

		case mod_brush:
			item_trans = ((currententity->drawflags & DRF_TRANSLUCENT)) != 0;
			if (!item_trans)
				R_DrawBrushModel (currententity,false);
			break;

		case mod_sprite:
			VectorSubtract(currententity->origin, r_origin, diff);
			calc_length = (diff[0]*diff[0]) + (diff[1]*diff[1]) + (diff[2]*diff[2]);
			if (calc_length > test_length)
			{
				continue;
			}

			item_trans = true;
			break;

		default:
			item_trans = false;
			break;
		}

		if (item_trans)
		{
			pLeaf = Mod_PointInLeaf (currententity->origin, cl.worldmodel);
//			if (pLeaf->contents == CONTENTS_EMPTY)
			if (pLeaf->contents != CONTENTS_WATER)
				cl_transvisedicts[cl_numtransvisedicts++].ent = currententity;
			else
				cl_transwateredicts[cl_numtranswateredicts++].ent = currententity;
		}
	}
}


// Glow styles. These rely on unchanged game code!
#define	TORCH_STYLE	1	/* Flicker	*/
#define	MISSILE_STYLE	6	/* Flicker	*/
#define	PULSE_STYLE	11	/* Slow pulse	*/

void R_DrawGlow (entity_t *e)
{
	model_t		*clmodel;

	clmodel = currententity->model;

    // Torches & Flames
	if ((gl_glows.value && (clmodel->ex_flags & XF_TORCH_GLOW )) ||
	    (gl_missile_glows.value && (clmodel->ex_flags & XF_MISSILE_GLOW)) ||
	    (gl_other_glows.value && (clmodel->ex_flags & XF_GLOW)))
	{
	// Draw torch flares. KH
		// NOTE: It would be better if we batched these up.
		//	 All those state changes are not nice. KH
		vec3_t	lightorigin;		// Origin of torch.
		vec3_t	glow_vect;		// Vector to torch.
		float	radius;			// Radius of torch flare.
		float	distance;		// Vector distance to torch.
		float	intensity;		// Intensity of torch flare.
		int	i,j;
		vec3_t	vp2;

		// NOTE: I don't think this is centered on the model.
		VectorCopy(currententity->origin, lightorigin);

		radius = 20.0f;

		// for mana make it bit bigger

		if ( !Q_strncasecmp (clmodel->name, "models/i_btmana",15) )
			radius += 5.0f;

		VectorSubtract(lightorigin, r_origin, vp2);

		// See if view is outside the light.
		distance = Length(glow_vect);
		// See if view is outside the light.
		distance = Length(vp2);

		if (distance > radius) {
			VectorNormalize(vp2);
			glPushMatrix_fp();

			// Translate the glow to coincide with the flame. KH
			if (clmodel->ex_flags & XF_TORCH_GLOW) {
				// egypt torch fix
			    if (!Q_strncasecmp (clmodel->name, "models/eflmtrch",15))
				glTranslatef_fp( cos(e->angles[1]/180*M_PI)*8.0f,sin(e->angles[1]/180*M_PI)*8.0f, 16.0f);
			    else
				glTranslatef_fp(0.0f, 0.0f, 8.0f);
			}

			// 'floating' movement

			if(clmodel->flags & EF_ROTATE)
			{
				glTranslatef_fp(0,0,sin(currententity->origin[0]+currententity->origin[1]+(cl.time*3))*5.5 );
			}

			glBegin_fp(GL_TRIANGLE_FAN);
			// Diminish torch flare inversely with distance.
			intensity = (1024.0f - distance) / 1024.0f;

			// Invert (fades as you approach).
			intensity = (1.0f - intensity);

			// Clamp, but don't let the flare disappear.
			if (intensity > 1.0f) intensity = 1.0f;
			if (intensity < 0.0f) intensity = 0.0f;

			// Now modulate with flicker.
			if (clmodel->ex_flags & XF_TORCH_GLOW) {
				i = (int)(cl.time*10);
				if (!cl_lightstyle[TORCH_STYLE].length) {
					j = 256;
				} else {
					j = i % cl_lightstyle[TORCH_STYLE].length;
					j = cl_lightstyle[TORCH_STYLE].map[j] - 'a';
					j = j*22;
				}
			}
			else if (clmodel->ex_flags & XF_MISSILE_GLOW) {
				i = (int)(cl.time*10);
				if (!cl_lightstyle[MISSILE_STYLE].length) {
					j = 256;
				} else {
					j = i % cl_lightstyle[MISSILE_STYLE].length;
					j = cl_lightstyle[MISSILE_STYLE].map[j] - 'a';
					j = j*22;
				}
			}
			else if (clmodel->ex_flags & XF_GLOW) {
				i = (int)(cl.time*10);
				if (!cl_lightstyle[PULSE_STYLE].length) {
					j = 256;
				} else {
					j = i % cl_lightstyle[PULSE_STYLE].length;
					j = cl_lightstyle[PULSE_STYLE].map[j] - 'a';
					j = j*22;
				}
			}

			intensity *= ((float)j / 255.0f);

			if (clmodel->ex_flags & XF_TORCH_GLOW)
			// Set yellow intensity
			    glColor4f_fp(0.8f*intensity, 0.4f*intensity, 0.1f*intensity,1.0f);
			else 
			    glColor4f_fp(clmodel->glow_color[0]*intensity,
			    clmodel->glow_color[1]*intensity,
			    clmodel->glow_color[2]*intensity,
			    0.5f);

			for (i=0 ; i<3 ; i++)
			    glow_vect[i] = lightorigin[i] - vp2[i]*radius;

			glVertex3fv_fp(glow_vect);

			glColor4f_fp(0.0f, 0.0f, 0.0f, 1.0f);

			for (i=16; i>=0; i--) {
				float a = i/16.0f * M_PI*2;

				for (j=0; j<3; j++)
					glow_vect[j] = lightorigin[j] + 
					vright[j]*cos(a)*radius +
					vup[j]*sin(a)*radius;

				glVertex3fv_fp(glow_vect);
			}

			glEnd_fp();
			glColor4f_fp (0.0f,0.0f,0.0f,1.0f);
			// Restore previous matrix! KH
			glPopMatrix_fp();		        
		}
	}
// end of glows    
}

void R_DrawAllGlows(void)
{
	int i;

	if (!r_drawentities.value)
		return;

	glDepthMask_fp (0);
	glDisable_fp (GL_TEXTURE_2D);
	glShadeModel_fp (GL_SMOOTH);
	glEnable_fp (GL_BLEND);
	glBlendFunc_fp (GL_ONE, GL_ONE);

	for (i=0 ; i<cl_numvisedicts ; i++) {
		currententity = &cl_visedicts[i];

		switch (currententity->model->type) {
		  case mod_alias:
			R_DrawGlow (currententity);
			break;
		  default:
			break;
		}		
	}

	glDisable_fp (GL_BLEND);
	glEnable_fp (GL_TEXTURE_2D);
	glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask_fp (1);
	glShadeModel_fp (GL_FLAT);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


/*
================
R_DrawTransEntitiesOnList
Implemented by: jack
================
*/

int transCompare(const void *arg1, const void *arg2 )
{
	const sortedent_t *a1, *a2;
	a1 = (sortedent_t *) arg1;
	a2 = (sortedent_t *) arg2;
	return (a2->len - a1->len); // Sorted in reverse order.  Neat, huh?
}

void R_DrawTransEntitiesOnList ( qboolean inwater)
{
	int i;
	int numents;
	sortedent_t *theents;
	int depthMaskWrite = 0;
	vec3_t result;

	theents = (inwater) ? cl_transwateredicts : cl_transvisedicts;
	numents = (inwater) ? cl_numtranswateredicts : cl_numtransvisedicts;

	for (i=0; i<numents; i++)
	{
		VectorSubtract(
			theents[i].ent->origin, 
			r_origin, 
			result);
//		theents[i].len = Length(result);
		theents[i].len = (result[0] * result[0]) + (result[1] * result[1]) + (result[2] * result[2]);
	}

	qsort((void *) theents, numents, sizeof(sortedent_t), transCompare);
	// Add in BETTER sorting here

	glDepthMask_fp(0);
	for (i=0;i<numents;i++)
	{
		currententity = theents[i].ent;

		switch (currententity->model->type)
		{
		case mod_alias:
			if (!depthMaskWrite)
			{
				depthMaskWrite = 1;
				glDepthMask_fp(1);
			}
			R_DrawAliasModel (currententity);
			break;
		case mod_brush:
			if (!depthMaskWrite)
			{
				depthMaskWrite = 1;
				glDepthMask_fp(1);
			}
			R_DrawBrushModel (currententity,true);
			break;
		case mod_sprite:
			if (depthMaskWrite)
			{
				depthMaskWrite = 0;
				glDepthMask_fp(0);
			}
			R_DrawSpriteModel (currententity);
			break;
		}
	}
	if (!depthMaskWrite) 
		glDepthMask_fp(1);
}

/*
=============
R_DrawViewModel
=============
*/
void R_DrawViewModel (void)
{
	int			lnum;
	vec3_t		dist;
	float		add;
	dlight_t	*dl;
	int		*lpc;

	if (cl.spectator)
		return;

	currententity = &cl.viewent;

	if (!currententity->model)
		return;

	if (gl_lightmap_format == GL_RGBA)
	{
		lpc = R_LightPointColour (currententity->origin);
		// always give some light on gun
		ambientlight = (float) lpc[3] > 24 ? lpc[3] : 24;
		shadelightcolor[0] = (float) lpc[0] > 24 ? lpc[0] : 24;
		shadelightcolor[1] = (float) lpc[1] > 24 ? lpc[1] : 24;
		shadelightcolor[2] = (float) lpc[2] > 24 ? lpc[2] : 24;
		shadelightcolor[3] = (float) lpc[3] > 24 ? lpc[3] : 24;
	}
	else
	{
		ambientlight = R_LightPoint (currententity->origin);
		if (ambientlight < 24)
			ambientlight = 24;	// always give some light on gun
	}

// add dynamic lights		
	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		dl = &cl_dlights[lnum];
		if (!dl->radius)
			continue;
		if (dl->die < cl.time)
			continue;

		VectorSubtract (currententity->origin, dl->origin, dist);
		add = dl->radius - Length(dist);
		if (add > 0)
		{
			if (gl_lightmap_format == GL_RGBA)
			{
				shadelightcolor[0] += (float) (dl->color[0] * add);
				shadelightcolor[1] += (float) (dl->color[1] * add);
				shadelightcolor[2] += (float) (dl->color[2] * add);
				shadelightcolor[3] += (float) add;
			}
			else
			{
				shadelight += (float) add;    // id left this out...
			}

			ambientlight += add;
		}
	}

	cl.light_level = ambientlight;

	if ((!r_drawviewmodel.value) ||
	    (cl.v.health <= 0) ||
	    (!r_drawentities.value) ||
	    (envmap))
	{
		return;
	}

//rjr	if (cl.items & IT_INVISIBILITY)
//rjr		return;

	// hack the depth range to prevent view model from poking into walls
	glDepthRange_fp (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));
	AlwaysDrawModel = true;
	R_DrawAliasModel (currententity);
	AlwaysDrawModel = false;
	glDepthRange_fp (gldepthmin, gldepthmax);
}


/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	if (!gl_polyblend.value)
		return;

	glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable_fp (GL_BLEND);
	glDisable_fp (GL_DEPTH_TEST);
	glDisable_fp (GL_TEXTURE_2D);

	glLoadIdentity_fp ();

	glRotatef_fp (-90,  1, 0, 0);	// put Z going up
	glRotatef_fp (90,  0, 0, 1);	// put Z going up

	if (v_blend[3])
	{
		glColor4fv_fp (v_blend);

		glBegin_fp (GL_QUADS);

		glVertex3f_fp (10, 100, 100);
		glVertex3f_fp (10, -100, 100);
		glVertex3f_fp (10, -100, -100);
		glVertex3f_fp (10, 100, -100);
		glEnd_fp ();
	}

	if (gl_dogamma) {
	//  if (v_gamma.value != 1)
		GL_DoGamma();
	}

	glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable_fp (GL_BLEND);
	glEnable_fp (GL_TEXTURE_2D);
	glEnable_fp (GL_ALPHA_TEST);
}


int SignbitsForPlane (mplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}


void R_SetFrustum (void)
{
	int		i;

	// front side is visible

	VectorAdd (vpn, vright, frustum[0].normal);
	VectorSubtract (vpn, vright, frustum[1].normal);

	VectorAdd (vpn, vup, frustum[2].normal);
	VectorSubtract (vpn, vup, frustum[3].normal);

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}



/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
// don't allow cheats in multiplayer
	r_fullbright.value = 0;

	R_AnimateLight ();

	r_framecount++;

// build the transformation matrix for the given view angles
	VectorCopy (r_refdef.vieworg, r_origin);

	AngleVectors (r_refdef.viewangles, vpn, vright, vup);

// current viewleaf
	r_oldviewleaf = r_viewleaf;
	r_viewleaf = Mod_PointInLeaf (r_origin, cl.worldmodel);

	V_SetContentsColor (r_viewleaf->contents);
	V_CalcBlend ();

	r_cache_thrash = false;

	c_brush_polys = 0;
	c_alias_polys = 0;

}


void MYgluPerspective( GLdouble fovy, GLdouble aspect,
		     GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan( fovy * M_PI / 360.0 );
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glFrustum_fp( xmin, xmax, ymin, ymax, zNear, zFar );
}


typedef struct _MATRIX {
    GLfloat M[4][4];
} MATRIX;

typedef struct _point3d {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} POINT3D;

static MATRIX	ModelviewMatrix, ProjectionMatrix, FinalMatrix;

void MultiplyMatrix( MATRIX *m1, MATRIX *m2, MATRIX *m3 )
{
    int i, j;

    for( j = 0; j < 4; j ++ )
	{
		for( i = 0; i < 4; i ++ )
		{
			m1->M[j][i] = m2->M[j][0] * m3->M[0][i] +
				m2->M[j][1] * m3->M[1][i] +
				m2->M[j][2] * m3->M[2][i] +
				m2->M[j][3] * m3->M[3][i];
		}
    }
}

void TransformPoint(POINT3D *ptOut, POINT3D *ptIn, MATRIX *mat)
{
    double x, y, z;

    x = (ptIn->x * mat->M[0][0]) + (ptIn->y * mat->M[0][1]) +
        (ptIn->z * mat->M[0][2]) + mat->M[0][3];

    y = (ptIn->x * mat->M[1][0]) + (ptIn->y * mat->M[1][1]) +
        (ptIn->z * mat->M[1][2]) + mat->M[1][3];

    z = (ptIn->x * mat->M[2][0]) + (ptIn->y * mat->M[2][1]) +
        (ptIn->z * mat->M[2][2]) + mat->M[2][3];

    ptOut->x = (float) x;
    ptOut->y = (float) y;
    ptOut->z = (float) z;
}


/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
	float	yfov;
	extern	int glwidth, glheight;
	int	x, x2, y2, y, w, h;

	//
	// set up viewpoint
	//
	glMatrixMode_fp(GL_PROJECTION);
	glLoadIdentity_fp ();
	x = r_refdef.vrect.x * glwidth/vid.width;
	x2 = (r_refdef.vrect.x + r_refdef.vrect.width) * glwidth/vid.width;
	y = (vid.height-r_refdef.vrect.y) * glheight/vid.height;
	y2 = (vid.height - (r_refdef.vrect.y + r_refdef.vrect.height)) * glheight/vid.height;

	// fudge around because of frac screen scale
	if (x > 0)
		x--;
	if (x2 < glwidth)
		x2++;
	if (y2 < 0)
		y2--;
	if (y < glheight)
		y++;

	w = x2 - x;
	h = y - y2;

	if (envmap)
	{
		x = y2 = 0;
		w = h = 256;
	}

	glViewport_fp (glx + x, gly + y2, w, h);
	screenaspect = (float)r_refdef.vrect.width/r_refdef.vrect.height;
//	yfov = 2*atan((float)r_refdef.vrect.height/r_refdef.vrect.width)*180/M_PI;
//	yfov = (2.0 * tan (scr_fov.value/360*M_PI)) / screenaspect;
	yfov = 2*atan((float)r_refdef.vrect.height/r_refdef.vrect.width)*(scr_fov.value*2)/M_PI;
	MYgluPerspective (yfov,  screenaspect,  4,  4096);

	if (mirror)
	{
		if (mirror_plane->normal[2])
			glScalef_fp (1, -1, 1);
		else
			glScalef_fp (-1, 1, 1);
		glCullFace_fp(GL_BACK);
	}
	else
		glCullFace_fp(GL_FRONT);

	glMatrixMode_fp(GL_MODELVIEW);
	glLoadIdentity_fp ();

	glRotatef_fp (-90,  1, 0, 0);	// put Z going up
	glRotatef_fp (90,  0, 0, 1);	// put Z going up
	glRotatef_fp (-r_refdef.viewangles[2],  1, 0, 0);
	glRotatef_fp (-r_refdef.viewangles[0],  0, 1, 0);
	glRotatef_fp (-r_refdef.viewangles[1],  0, 0, 1);
	glTranslatef_fp (-r_refdef.vieworg[0],  -r_refdef.vieworg[1],  -r_refdef.vieworg[2]);

	glGetFloatv_fp (GL_MODELVIEW_MATRIX, r_world_matrix);

	//
	// set drawing parms
	//
	if (gl_cull.value)
		glEnable_fp(GL_CULL_FACE);
	else
		glDisable_fp(GL_CULL_FACE);

	glDisable_fp(GL_BLEND);
	glDisable_fp(GL_ALPHA_TEST);
	glEnable_fp(GL_DEPTH_TEST);

	glGetFloatv_fp(GL_MODELVIEW_MATRIX, (float *)ModelviewMatrix.M);
//	ModelviewMatrix.M[0][3] = 0;
//	ModelviewMatrix.M[1][3] = 0;
//	ModelviewMatrix.M[2][3] = 0;
//	ModelviewMatrix.M[3][3] = 0;
	glGetFloatv_fp(GL_PROJECTION_MATRIX, (float *)ProjectionMatrix.M);
//	MultiplyMatrix(&FinalMatrix, &ModelviewMatrix, &ProjectionMatrix);
	MultiplyMatrix(&FinalMatrix, &ProjectionMatrix, &ModelviewMatrix);
}

/*
================
R_RenderScene

r_refdef must be set before the first call
================
*/
void R_RenderScene (void)
{
	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_DrawWorld ();		// adds static entities to the list

	S_ExtraUpdate ();	// don't let sound get messed up if going slow

	R_DrawEntitiesOnList ();

	R_DrawAllGlows();

	R_RenderDlights ();

#ifdef GLTEST
	Test_Draw ();
#endif

}


/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	if (r_mirroralpha.value != 1.0)
	{
		if (gl_clear.value)
			glClear_fp (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			glClear_fp (GL_DEPTH_BUFFER_BIT);
		gldepthmin = 0;
		gldepthmax = 0.5;
		glDepthFunc_fp (GL_LEQUAL);
	}
	else if (gl_ztrick.value)
	{
		static int trickframe;

		if (gl_clear.value)
			glClear_fp (GL_COLOR_BUFFER_BIT);

		trickframe++;
		if (trickframe & 1)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
			glDepthFunc_fp (GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
			glDepthFunc_fp (GL_GEQUAL);
		}
	}
	else
	{
		if (gl_clear.value)
			glClear_fp (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
			glClear_fp (GL_DEPTH_BUFFER_BIT);
		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc_fp (GL_LEQUAL);
	}

	glDepthRange_fp (gldepthmin, gldepthmax);

	if (have_stencil == true && gl_stencilshadow.value > 0 && r_shadows.value > 0)
	{
		glClearStencil_fp(1);
		glClear_fp(GL_STENCIL_BUFFER_BIT);
	}
}

#if 0 //!!! FIXME, Zoid, mirror is disabled for now
/*
=============
R_Mirror
=============
*/
void R_Mirror (void)
{
	float		d;
	msurface_t	*s;
	entity_t	*ent;

	if (!mirror)
		return;

	memcpy (r_base_world_matrix, r_world_matrix, sizeof(r_base_world_matrix));

	d = DotProduct (r_refdef.vieworg, mirror_plane->normal) - mirror_plane->dist;
	VectorMA (r_refdef.vieworg, -2*d, mirror_plane->normal, r_refdef.vieworg);

	d = DotProduct (vpn, mirror_plane->normal);
	VectorMA (vpn, -2*d, mirror_plane->normal, vpn);

	r_refdef.viewangles[0] = -asin (vpn[2])/M_PI*180;
	r_refdef.viewangles[1] = atan2 (vpn[1], vpn[0])/M_PI*180;
	r_refdef.viewangles[2] = -r_refdef.viewangles[2];

	ent = &cl_entities[cl.viewentity];
	if (cl_numvisedicts < MAX_VISEDICTS)
	{
		cl_visedicts[cl_numvisedicts] = ent;
		cl_numvisedicts++;
	}

	gldepthmin = 0.5;
	gldepthmax = 1;
	glDepthRange_fp (gldepthmin, gldepthmax);
	glDepthFunc_fp (GL_LEQUAL);

	glDepthMask_fp(0);

	R_DrawParticles ();
// THIS IS THE F*S*D(KCING MIRROR ROUTINE!  Go down!!!
	R_DrawTransEntitiesOnList( true ); // This restores the depth mask

	R_DrawWaterSurfaces ();

	R_DrawTransEntitiesOnList( false );

	gldepthmin = 0;
	gldepthmax = 0.5;
	glDepthRange_fp (gldepthmin, gldepthmax);
	glDepthFunc_fp (GL_LEQUAL);

	// blend on top
	glEnable_fp (GL_BLEND);
	glMatrixMode_fp(GL_PROJECTION);
	if (mirror_plane->normal[2])
		glScalef_fp (1,-1,1);
	else
		glScalef_fp (-1,1,1);
	glCullFace_fp(GL_FRONT);
	glMatrixMode_fp(GL_MODELVIEW);

	glLoadMatrixf_fp (r_base_world_matrix);

	glColor4f_fp (1,1,1,r_mirroralpha.value);
	s = cl.worldmodel->textures[mirrortexturenum]->texturechain;
	for ( ; s ; s=s->texturechain)
		R_RenderBrushPoly (s, true);
	cl.worldmodel->textures[mirrortexturenum]->texturechain = NULL;
	glDisable_fp (GL_BLEND);
	glColor4f_fp (1,1,1,1);
}
#endif


/*
=============
R_PrintTimes
=============
*/
void R_PrintTimes(void)
{
	float r_time2;
	float ms, fps;

	r_lasttime1 = r_time2 = Sys_DoubleTime();

	ms = 1000*(r_time2-r_time1);
	fps = 1000/ms;

	Con_Printf("%3.1f fps %5.0f ms\n%4i wpoly  %4i epoly  %4i(%i) edicts\n",
		fps, ms, c_brush_polys, c_alias_polys, cl_numvisedicts, cl_numtransvisedicts+cl_numtranswateredicts);
}


void R_DrawName(vec3_t origin, char *Name, int Red);

/*
================
R_RenderView

r_refdef must be set before the first call
================
*/
void R_RenderView (void)
{
	if (r_norefresh.value)
		return;

	if (!r_worldentity.model || !cl.worldmodel)
		Sys_Error ("R_RenderView: NULL worldmodel");

	if (r_speeds.value)
	{
		glFinish_fp ();
		r_time1 = Sys_DoubleTime ();
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	mirror = false;

//	glFinish_fp ();

	R_Clear ();

	// render normal view
	R_RenderScene ();

	glDepthMask_fp(0);

	R_DrawParticles ();

	R_DrawTransEntitiesOnList( r_viewleaf->contents == CONTENTS_EMPTY ); // This restores the depth mask

	R_DrawWaterSurfaces ();

	R_DrawTransEntitiesOnList( r_viewleaf->contents != CONTENTS_EMPTY );

	R_DrawViewModel();

	// render mirror view
//	R_Mirror ();

	R_PolyBlend ();

	if (r_speeds.value)
	{
		R_PrintTimes ();
	}
}

void R_DrawName(vec3_t origin, char *Name, int Red)
{
	float	zi;
	int		u, v;
	POINT3D	In, Out;

	if (!Name)
	{
		return;
	}

	In.x = origin[0];
	In.y = origin[1];
	In.z = origin[2];
	TransformPoint(&Out, &In, &FinalMatrix);

	if (Out.z < 0)
	{
		return;
	}

	zi = 1.0 / (Out.z + 8);
	u = (int)(r_refdef.vrect.width / 2 * (zi * Out.x + 1) ) + r_refdef.vrect.x;
	v = (int)(r_refdef.vrect.height / 2 * (zi * (-Out.y) + 1) ) + r_refdef.vrect.y;

	u -= strlen(Name) * 4;

	if(cl_siege)
	{
		if(Red>10)
		{
			Red-=10;
			Draw_Character (u, v, 145);//key
			u+=8;
		}
		if(Red>0&&Red<3)//def
		{
			if(Red==true)
				Draw_Character (u, v, 143);//shield
			else
				Draw_Character (u, v, 130);//crown
			Draw_RedString(u+8, v, Name);
		}
		else if(!Red)
		{
			Draw_Character (u, v, 144);//sword
			Draw_String (u+8, v, Name);
		}
		else
			Draw_String (u+8, v, Name);
	}
	else
		Draw_String (u, v, Name);
}

