/*

	models.c
	model loading and caching
	models are the only shared resource between a client and server
	running on the same machine.

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/model.c,v 1.4 2005-04-30 12:07:17 sezero Exp $
*/

#include "quakedef.h"
#include "r_local.h"

#ifndef PLATFORM_UNIX
#include <windows.h>
#endif

model_t	*loadmodel;
char	loadname[32];	// for hunk tags

void Mod_LoadSpriteModel (model_t *mod, void *buffer);
void Mod_LoadBrushModel (model_t *mod, void *buffer);
void Mod_LoadAliasModel (model_t *mod, void *buffer);
void Mod_LoadAliasModelNew (model_t *mod, void *buffer);

model_t *Mod_LoadModel (model_t *mod, qboolean crash);

byte	mod_novis[MAX_MAP_LEAFS/8];

#define	MAX_MOD_KNOWN	512
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;
static vec3_t	mins,maxs;

// values for model_t's needload
#define NL_PRESENT		0
#define NL_NEEDS_LOADED	1
#define NL_UNREFERENCED	2

extern float	aliastransform[3][4];

int entity_file_size;

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
===============
Mod_Extradata

Caches the data if needed
===============
*/
void *Mod_Extradata (model_t *mod)
{
	void	*r;
	
	r = Cache_Check (&mod->cache);
	if (r)
		return r;

	Mod_LoadModel (mod, true);
	
	if (!mod->cache.data)
		Sys_Error ("Mod_Extradata: caching failed");
	return mod->cache.data;
}

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;
	float		d;
	mplane_t	*plane;
	

	if (!model)
		Sys_Error ("Mod_PointInLeaf: NULL model");
	if (!model->nodes)
		Sys_Error ("Mod_PointInLeaf: model w/o nodes : %s",model->name);

	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}
	
	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->numleafs+7)>>3;	
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;		
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
	
	return decompressed;
}

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if (leaf == model->leafs)
		return mod_novis;
	return Mod_DecompressVis (leaf->compressed_vis, model);
}

/*
===================
Mod_ClearAll
===================
*/
void Mod_ClearAll (void)
{
	int		i;
	model_t	*mod;


	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
		mod->needload = NL_UNREFERENCED;
}

/*
==================
Mod_FindName

==================
*/
model_t *Mod_FindName (char *name)
{
	int		i;
	model_t	*mod;
	model_t	*avail = NULL;

	if (!name[0])
		Sys_Error ("Mod_ForName: NULL name");
		
//
// search the currently loaded models
//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!strcmp (mod->name, name) )
			break;
		if (mod->needload == NL_UNREFERENCED)
			if (!avail || mod->type != mod_alias)
				avail = mod;
	}
			
	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
		{
			if (avail)
			{
				mod = avail;
				if (mod->type == mod_alias)
				{
					if (Cache_Check (&mod->cache))
						Cache_Free (&mod->cache);
				}
				else if (mod->type == mod_sprite)
					mod->cache.data = NULL;
			}
			else
				Sys_Error ("mod_numknown == MAX_MOD_KNOWN");
		}
		else
			mod_numknown++;
		strcpy (mod->name, name);
		mod->needload = NL_NEEDS_LOADED;
	}

	return mod;
}

/*
==================
Mod_TouchModel

==================
*/
void Mod_TouchModel (char *name)
{
	model_t	*mod;
	
	mod = Mod_FindName (name);
	
	if (mod->needload == NL_PRESENT)
	{
		if (mod->type == mod_alias)
			Cache_Check (&mod->cache);
	}
}

/*
==================
Mod_LoadModel

Loads a model into the cache
==================
*/
model_t *Mod_LoadModel (model_t *mod, qboolean crash)
{
	unsigned *buf;
	byte	stackbuf[1024];		// avoid dirtying the cache heap


	if (mod->type == mod_alias)
	{
		if (Cache_Check (&mod->cache))
		{
			mod->needload = NL_PRESENT;
			return mod;
		}
	}
	else
	{
		if (mod->needload == NL_PRESENT)
			return mod;
	}

//
// load the file
//
	buf = (unsigned *)COM_LoadStackFile (mod->name, stackbuf, sizeof(stackbuf));
	if (!buf)
	{
		if (crash)
			Sys_Error ("Mod_NumForName: %s not found", mod->name);
		return NULL;
	}
	
//
// allocate a new model
//
	COM_FileBase (mod->name, loadname);
	
	loadmodel = mod;

//
// fill it in
//

// call the apropriate loader
	mod->needload = NL_PRESENT;

	switch (LittleLong(*(unsigned *)buf))
	{
	case RAPOLYHEADER:
		Mod_LoadAliasModelNew (mod, buf);
		break;
	case IDPOLYHEADER:
		Mod_LoadAliasModel (mod, buf);
		break;
		
	case IDSPRITEHEADER:
		Mod_LoadSpriteModel (mod, buf);
		break;
	
	default:
		Mod_LoadBrushModel (mod, buf);
		break;
	}

	return mod;
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (char *name, qboolean crash)
{
	model_t	*mod;
	
/*	if (strcmpi(name,"models/boss/dthrider.mdl") == 0)// ||
//		strcmpi(name,"models/h_golem.mdl") == 0)
		DebugBreak();
*/

	mod = Mod_FindName (name);
	
	return Mod_LoadModel (mod, crash);
}


/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;


/*
=================
Mod_LoadTextures
=================
*/
void Mod_LoadTextures (lump_t *l)
{
	int		i, j, pixels, num, max, altmax;
	miptex_t	*mt;
	texture_t	*tx, *tx2;
	texture_t	*anims[10];
	texture_t	*altanims[10];
	dmiptexlump_t *m;

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}
	m = (dmiptexlump_t *)(mod_base + l->fileofs);
	
	m->nummiptex = LittleLong (m->nummiptex);
	
	loadmodel->numtextures = m->nummiptex;
//	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , loadname);
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , "texture");

	for (i=0 ; i<m->nummiptex ; i++)
	{
		m->dataofs[i] = LittleLong(m->dataofs[i]);
		if (m->dataofs[i] == -1)
			continue;
		mt = (miptex_t *)((byte *)m + m->dataofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);
		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);
		
		if ( (mt->width & 15) || (mt->height & 15) )
			Sys_Error ("Texture %s is not 16 aligned", mt->name);
		pixels = mt->width*mt->height/64*85;
//		tx = Hunk_AllocName (sizeof(texture_t) +pixels, loadname );
		tx = Hunk_AllocName (sizeof(texture_t) +pixels, "texture" );
		loadmodel->textures[i] = tx;

		memcpy (tx->name, mt->name, sizeof(tx->name));
		tx->width = mt->width;
		tx->height = mt->height;
		for (j=0 ; j<MIPLEVELS ; j++)
			tx->offsets[j] = mt->offsets[j] + sizeof(texture_t) - sizeof(miptex_t);
		// the pixels immediately follow the structures
		memcpy ( tx+1, mt+1, pixels);
		
		if (!strncmp(mt->name,"sky",3))	
			R_InitSky (tx);
	}

//
// sequence the animations
//
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// allready sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error ("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}
		
#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[ (j+1)%altmax ];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}
}

/*
=================
Mod_LoadLighting
=================
*/
void Mod_LoadLighting (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
//	loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);	
	loadmodel->lightdata = Hunk_AllocName ( l->filelen, "light");	
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}
//	loadmodel->visdata = Hunk_AllocName ( l->filelen, loadname);	
	loadmodel->visdata = Hunk_AllocName ( l->filelen, "vis");	
	memcpy (loadmodel->visdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadEntities
=================
*/
void Mod_LoadEntities (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->entities = NULL;
		return;
	}
//	loadmodel->entities = Hunk_AllocName ( l->filelen, loadname);	
	loadmodel->entities = Hunk_AllocName ( l->filelen, "entities");	
	memcpy (loadmodel->entities, mod_base + l->fileofs, l->filelen);
	entity_file_size = l->filelen;
}


/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "vertexes");	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
	}
}

/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	dmodel_t	*out;
	int			i, j, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "submodels");	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		for (j=0 ; j<MAX_MAP_HULLS ; j++)
			out->headnode[j] = LittleLong (in->headnode[j]);
		out->visleafs = LittleLong (in->visleafs);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l)
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( (count + 1) * sizeof(*out), loadname);	
	out = Hunk_AllocName ( (count + 1) * sizeof(*out), "edges");	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out;
	int 	i, j, count;
	int		miptex;
	float	len1, len2;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "texture");	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
		len1 = Length (out->vecs[0]);
		len2 = Length (out->vecs[1]);
		len1 = (len1 + len2)/2;
		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;
#if 0
		if (len1 + len2 < 0.001)
			out->mipadjust = 1;		// don't crash
		else
			out->mipadjust = 1 / floor( (len1+len2)/2 + 0.1 );
#endif

		miptex = LittleLong (in->miptex);
		out->flags = LittleLong (in->flags);
	
		if (!loadmodel->textures)
		{
			out->texture = r_notexture_mip;	// checkerboard texture
			out->flags = 0;
		}
		else
		{
			if (miptex >= loadmodel->numtextures)
				Sys_Error ("miptex >= loadmodel->numtextures");
			out->texture = loadmodel->textures[miptex];
			if (!out->texture)
			{
				out->texture = r_notexture_mip; // texture not found
				out->flags = 0;
			}
		}
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents (msurface_t *s)
{
	float	min_s[2], max_s[2], val;
	int		i,j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	min_s[0] = min_s[1] = 999999;
	max_s[0] = max_s[1] = -99999;

	tex = s->texinfo;
	
	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];
		
		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + 
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < min_s[j])
				min_s[j] = val;
			if (val > max_s[j])
				max_s[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{	
		bmins[i] = floor(min_s[i]/16);
		bmaxs[i] = ceil(max_s[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;
		if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 256)
			Sys_Error ("Bad surface extents");
	}
}


/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "faces");	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);		
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
			out->flags |= SURF_PLANEBACK;			

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (out);
				
	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + i;
		
	// set the drawing flags flag
		
		if (!strncmp(out->texinfo->texture->name,"sky",3))	// sky
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			continue;
		}
		
		if (out->texinfo->texture->name[0]=='*')		// turbulent
		{
			if(!strncmp(out->texinfo->texture->name,"*BLACK",6))
			{
				out->flags |= (SURF_DRAWTILED | SURF_DRAWBLACK);
				continue;
			}
			out->flags |= (SURF_DRAWTURB | SURF_DRAWTILED);

			// stupid stupid way of doing this:
/*			if (r_transwater.value && 
				_strnicmp(out->texinfo->texture->name,"*lava",5) &&
				_strnicmp(out->texinfo->texture->name,"*slime",6) &&
				_strnicmp(out->texinfo->texture->name,"*cur_90",7) &&
				_strnicmp(out->texinfo->texture->name,"*cur_0",6) &&
				_strnicmp(out->texinfo->texture->name,"*cur_270",8) &&
				_strnicmp(out->texinfo->texture->name,"*cur_180",8) &&
				_strnicmp(out->texinfo->texture->name,"*cur_up",7) &&
				_strnicmp(out->texinfo->texture->name,"*cur_dwn",8))
				out->flags |= SURF_TRANSLUCENT;
*/
			if (r_transwater.value && 
				( (!_strnicmp(out->texinfo->texture->name,"*rtex078",8)) ||
				  (!_strnicmp(out->texinfo->texture->name,"*lowlight",9)) ) )
				out->flags |= SURF_TRANSLUCENT;

			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			continue;
		}
	}
}


/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents < 0)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "nodes");	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}
	
		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
		
		for (j=0 ; j<2 ; j++)
		{
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}
	
	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count, p;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "leafs");	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->contents);
		out->contents = p;

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);
		
		p = LittleLong(in->visofs);
		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = loadmodel->visdata + p;
		out->efrags = NULL;
		
		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];
	}	
}

/*
=================
Mod_LoadClipnodes
=================
*/
void Mod_LoadClipnodes (lump_t *l)
{
	dclipnode_t *in, *out;
	int			i, count;
	hull_t		*hull;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "clipnodes");	

	loadmodel->clipnodes = out;
	loadmodel->numclipnodes = count;

//player
	hull = &loadmodel->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;

//scorpion
	hull = &loadmodel->hulls[2];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -24;
	hull->clip_mins[1] = -24;
	hull->clip_mins[2] = -20;
	hull->clip_maxs[0] = 24;
	hull->clip_maxs[1] = 24;
	hull->clip_maxs[2] = 20;

//crouch
	hull = &loadmodel->hulls[3];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -12;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 16;

//hydra -changing in MP to '-8 -8 -8', '8 8 8' for pentacles
	hull = &loadmodel->hulls[4];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
#if 0	
	hull->clip_mins[0] = -40;
	hull->clip_mins[1] = -40;
	hull->clip_mins[2] = -42;
	hull->clip_maxs[0] = 40;
	hull->clip_maxs[1] = 40;
	hull->clip_maxs[2] = 42;
#else
	hull->clip_mins[0] = -8;
	hull->clip_mins[1] = -8;
	hull->clip_mins[2] = -8;
	hull->clip_maxs[0] = 8;
	hull->clip_maxs[1] = 8;
	hull->clip_maxs[2] = 8;
#endif

//golem - maybe change to '-28 -28 -40', '28 28 40' for Yakman
	hull = &loadmodel->hulls[5];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
#if 0	//use yak sizes
	hull->clip_mins[0] = -28;
	hull->clip_mins[1] = -28;
	hull->clip_mins[2] = -40;
	hull->clip_maxs[0] = 28;
	hull->clip_maxs[1] = 28;
	hull->clip_maxs[2] = 40;
#else
	hull->clip_mins[0] = -48;
	hull->clip_mins[1] = -48;
	hull->clip_mins[2] = -50;
	hull->clip_maxs[0] = 48;
	hull->clip_maxs[1] = 48;
	hull->clip_maxs[2] = 50;
#endif

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
	}
}

/*
=================
Mod_MakeHull0

Deplicate the drawing hull structure as a clipping hull
=================
*/
void Mod_MakeHull0 (void)
{
	mnode_t		*in, *child;
	dclipnode_t *out;
	int			i, j, count;
	hull_t		*hull;
	
	hull = &loadmodel->hulls[0];	
	
	in = loadmodel->nodes;
	count = loadmodel->numnodes;
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "hull0");	

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = in->plane - loadmodel->planes;
		for (j=0 ; j<2 ; j++)
		{
			child = in->children[j];
			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - loadmodel->nodes;
		}
	}
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l)
{	
	int		i, j, count;
	short		*in;
	msurface_t **out;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "marksurfaces");	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j >= loadmodel->numsurfaces)
			Sys_Error ("Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l)
{	
	int		i, count;
	int		*in, *out;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*sizeof(*out), "surfedges");	

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}

/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l)
{
	int			i, j;
	mplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
//	out = Hunk_AllocName ( count*2*sizeof(*out), loadname);	
	out = Hunk_AllocName ( count*2*sizeof(*out), "planes");	
	
	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (vec3_t min_ss, vec3_t max_ss)
{
	int		i;
	vec3_t	corner;

	for (i=0 ; i<3 ; i++)
	{
		corner[i] = fastfabs(min_ss[i]) > fastfabs(max_ss[i]) ? fastfabs(min_ss[i]) : fastfabs(max_ss[i]);
	}

	return Length (corner);
}

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, void *buffer)
{
	int			i, j;
	dheader_t	*header;
	dmodel_t 	*bm;
	
	loadmodel->type = mod_brush;
	
	header = (dheader_t *)buffer;

	i = LittleLong (header->version);
	if (i != BSPVERSION)
		Sys_Error ("Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, i, BSPVERSION);

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

// load into heap
	
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadTextures (&header->lumps[LUMP_TEXTURES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_MARKSURFACES]);
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadClipnodes (&header->lumps[LUMP_CLIPNODES]);
	Mod_LoadEntities (&header->lumps[LUMP_ENTITIES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);

	Mod_MakeHull0 ();
	
	mod->numframes = 2;		// regular and alternate animation
	mod->flags = 0;
	
//
// set up the submodels (FIXME: this is confusing)
//
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];
		for (j=1 ; j<MAX_MAP_HULLS ; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes-1;
		}
		
		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;
		
		VectorCopy (bm->maxs, mod->maxs);
		VectorCopy (bm->mins, mod->mins);
		mod->radius = RadiusFromBounds (mod->mins, mod->maxs);
	
		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels-1)
		{	// duplicate the basic information
			char	name[10];

			sprintf (name, "*%i", i+1);
			loadmodel = Mod_FindName (name);
			*loadmodel = *mod;
			strcpy (loadmodel->name, name);
			mod = loadmodel;
		}
	}
}

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

float	aliastransform2[3][4];

/*
================
R_AliasTransformVector
================
*/
void R_AliasTransformVector2 (vec3_t in, vec3_t out)
{
	out[0] = DotProduct(in, aliastransform2[0]) + aliastransform2[0][3];
	out[1] = DotProduct(in, aliastransform2[1]) + aliastransform2[1][3];
	out[2] = DotProduct(in, aliastransform2[2]) + aliastransform2[2][3];
}

/*
=================
Mod_LoadAliasFrame
=================
*/
void * Mod_LoadAliasFrame (void * pin, int *pframeindex, int numv,
	trivertx_t *pbboxmin, trivertx_t *pbboxmax, aliashdr_t *pheader, char *name,
	newmdl_t *pmodel)
{
	trivertx_t		*pframe, *pinframe;
	int				i, j;
	daliasframe_t	*pdaliasframe;
	vec3_t in,out;

	pdaliasframe = (daliasframe_t *)pin;

	strcpy (name, pdaliasframe->name);

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about
	// endianness
		pbboxmin->v[i] = pdaliasframe->bboxmin.v[i];
		pbboxmax->v[i] = pdaliasframe->bboxmax.v[i];
	}

	pinframe = (trivertx_t *)(pdaliasframe + 1);
	pframe = Hunk_AllocName (numv * sizeof(*pframe), loadname);

	*pframeindex = (byte *)pframe - (byte *)pheader;

	aliastransform2[0][0] = pmodel->scale[0];
	aliastransform2[1][1] = pmodel->scale[1];
	aliastransform2[2][2] = pmodel->scale[2];
	aliastransform2[0][3] = pmodel->scale_origin[0];
	aliastransform2[1][3] = pmodel->scale_origin[1];
	aliastransform2[2][3] = pmodel->scale_origin[2];

	for (j=0 ; j<numv ; j++)
	{
		int		k;

		in[0] = pinframe[j].v[0];
		in[1] = pinframe[j].v[1];
		in[2] = pinframe[j].v[2];
		R_AliasTransformVector2(in,out);

	// these are all byte values, so no need to deal with endianness
		pframe[j].lightnormalindex = pinframe[j].lightnormalindex;

		for (k=0 ; k<3 ; k++)
		{
			pframe[j].v[k] = pinframe[j].v[k];

			if (mins[k] > out[k])
				mins[k] = out[k];
			if (maxs[k] < out[k])
				maxs[k] = out[k];
		}
	}

	pinframe += numv;

	return (void *)pinframe;
}


/*
=================
Mod_LoadAliasGroup
=================
*/
void * Mod_LoadAliasGroup (void * pin, int *pframeindex, int numv,
	trivertx_t *pbboxmin, trivertx_t *pbboxmax, aliashdr_t *pheader, char *name,
	newmdl_t *pmodel)
{
	daliasgroup_t		*pingroup;
	maliasgroup_t		*paliasgroup;
	int					i, numframes;
	daliasinterval_t	*pin_intervals;
	float				*poutintervals;
	void				*ptemp;
	
	pingroup = (daliasgroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);

	paliasgroup = Hunk_AllocName (sizeof (maliasgroup_t) +
			(numframes - 1) * sizeof (paliasgroup->frames[0]), loadname);

	paliasgroup->numframes = numframes;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		pbboxmin->v[i] = pingroup->bboxmin.v[i];
		pbboxmax->v[i] = pingroup->bboxmax.v[i];
	}

	*pframeindex = (byte *)paliasgroup - (byte *)pheader;

	pin_intervals = (daliasinterval_t *)(pingroup + 1);

	poutintervals = Hunk_AllocName (numframes * sizeof (float), loadname);

	paliasgroup->intervals = (byte *)poutintervals - (byte *)pheader;

	for (i=0 ; i<numframes ; i++)
	{
		*poutintervals = LittleFloat (pin_intervals->interval);
		if (*poutintervals <= 0.0)
			Sys_Error ("Mod_LoadAliasGroup: interval<=0");

		poutintervals++;
		pin_intervals++;
	}

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
	{
		ptemp = Mod_LoadAliasFrame (ptemp,
									&paliasgroup->frames[i].frame,
									numv,
									&paliasgroup->frames[i].bboxmin,
									&paliasgroup->frames[i].bboxmax,
									pheader, name, pmodel);
	}

	return ptemp;
}


/*
=================
Mod_LoadAliasSkin
=================
*/
void * Mod_LoadAliasSkin (void * pin, int *pskinindex, int skinsize,
	aliashdr_t *pheader)
{
	int		i;
	byte	*pskin, *pinskin;
	unsigned short	*pusskin;

	pskin = Hunk_AllocName (skinsize * r_pixbytes, loadname);
	pinskin = (byte *)pin;
	*pskinindex = (byte *)pskin - (byte *)pheader;

	if (r_pixbytes == 1)
	{
		memcpy (pskin, pinskin, skinsize);
	}
	else if (r_pixbytes == 2)
	{
		pusskin = (unsigned short *)pskin;

		for (i=0 ; i<skinsize ; i++)
			pusskin[i] = d_8to16table[pinskin[i]];
	}
	else
	{
		Sys_Error ("Mod_LoadAliasSkin: driver set invalid r_pixbytes: %d\n",
				 r_pixbytes);
	}

	pinskin += skinsize;

	return ((void *)pinskin);
}


/*
=================
Mod_LoadAliasSkinGroup
=================
*/
void * Mod_LoadAliasSkinGroup (void * pin, int *pskinindex, int skinsize,
	aliashdr_t *pheader)
{
	daliasskingroup_t		*pinskingroup;
	maliasskingroup_t		*paliasskingroup;
	int						i, numskins;
	daliasskininterval_t	*pinskinintervals;
	float					*poutskinintervals;
	void					*ptemp;

	pinskingroup = (daliasskingroup_t *)pin;

	numskins = LittleLong (pinskingroup->numskins);

	paliasskingroup = Hunk_AllocName (sizeof (maliasskingroup_t) +
			(numskins - 1) * sizeof (paliasskingroup->skindescs[0]),
			loadname);

	paliasskingroup->numskins = numskins;

	*pskinindex = (byte *)paliasskingroup - (byte *)pheader;

	pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);

	poutskinintervals = Hunk_AllocName (numskins * sizeof (float),loadname);

	paliasskingroup->intervals = (byte *)poutskinintervals - (byte *)pheader;

	for (i=0 ; i<numskins ; i++)
	{
		*poutskinintervals = LittleFloat (pinskinintervals->interval);
		if (*poutskinintervals <= 0)
			Sys_Error ("Mod_LoadAliasSkinGroup: interval<=0");

		poutskinintervals++;
		pinskinintervals++;
	}

	ptemp = (void *)pinskinintervals;

	for (i=0 ; i<numskins ; i++)
	{
		ptemp = Mod_LoadAliasSkin (ptemp,
				&paliasskingroup->skindescs[i].skin, skinsize, pheader);
	}

	return ptemp;
}


/*
=================
Mod_LoadAliasModelNew
loads new expanded header format needed to do vert opts
=================
*/
void Mod_LoadAliasModelNew (model_t *mod, void *buffer)
{
	int					i;
	newmdl_t			*pinmodel;
	newmdl_t			*pmodel;
	stvert_t			*pstverts, *pinstverts;
	aliashdr_t			*pheader;
	mtriangle_t			*ptri;
	dnewtriangle_t 		*pintriangles;
	int					version, numframes, numskins;
	int					size;
	daliasframetype_t	*pframetype;
	daliasskintype_t	*pskintype;
	maliasskindesc_t	*pskindesc;
	int					skinsize;
	int					start, end, total;
	int					j;

//	Mod_SaveAliasModel (mod->name, buffer);
	
	start = Hunk_LowMark ();

	pinmodel = (newmdl_t*)buffer;

	version = LittleLong (pinmodel->version);
	if (version != ALIAS_NEWVERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_NEWVERSION);

//
// allocate space for a working header, plus all the data except the frames,
// skin and group info
//
	size = 	sizeof (aliashdr_t) + (LittleLong (pinmodel->numframes) - 1) *
			 sizeof (pheader->frames[0]) +
			sizeof (newmdl_t) +
			LittleLong (pinmodel->num_st_verts) * sizeof (stvert_t) +
			LittleLong (pinmodel->numtris) * sizeof (mtriangle_t);

	pheader = Hunk_AllocName (size, loadname);
	pmodel = (newmdl_t *) ((byte *)&pheader[1] +
			(LittleLong (pinmodel->numframes) - 1) *
			 sizeof (pheader->frames[0]));
	
//	mod->cache.data = pheader;
	mod->flags = LittleLong (pinmodel->flags);

//
// endian-adjust and copy the data, starting with the alias model header
//
	pmodel->boundingradius = LittleFloat (pinmodel->boundingradius);
	pmodel->numskins = LittleLong (pinmodel->numskins);
	pmodel->skinwidth = LittleLong (pinmodel->skinwidth);
	pmodel->skinheight = LittleLong (pinmodel->skinheight);

	if (pmodel->skinheight > MAX_SKIN_HEIGHT)
		Sys_Error ("model %s has a skin taller than %d", mod->name,
				   MAX_SKIN_HEIGHT);

	pmodel->numverts = LittleLong (pinmodel->numverts);
	//use the new num
	pmodel->num_st_verts = LittleLong (pinmodel->num_st_verts);

	if (pmodel->numverts <= 0)
		Sys_Error ("model %s has no vertices", mod->name);
	if (pmodel->num_st_verts <= 0)
		Sys_Error ("model %s has no ST vertices", mod->name);

	if (pmodel->numverts > MAXALIASVERTS)
		Sys_Error ("model %s has too many vertices", mod->name);

	pmodel->numtris = LittleLong (pinmodel->numtris);

	if (pmodel->numtris <= 0)
		Sys_Error ("model %s has no triangles", mod->name);

	pmodel->numframes = LittleLong (pinmodel->numframes);
	pmodel->size = LittleFloat (pinmodel->size) * ALIAS_BASE_SIZE_RATIO;
	mod->synctype = LittleLong (pinmodel->synctype);
	mod->numframes = pmodel->numframes;

	for (i=0 ; i<3 ; i++)
	{
		pmodel->scale[i] = LittleFloat (pinmodel->scale[i]);
		pmodel->scale_origin[i] = LittleFloat (pinmodel->scale_origin[i]);
		pmodel->eyeposition[i] = LittleFloat (pinmodel->eyeposition[i]);
	}

	numskins = pmodel->numskins;
	numframes = pmodel->numframes;

	if (pmodel->skinwidth & 0x03)
		Sys_Error ("Mod_LoadAliasModel: skinwidth not multiple of 4");

	pheader->model = (byte *)pmodel - (byte *)pheader;

//
// load the skins
//
	skinsize = pmodel->skinheight * pmodel->skinwidth;

	if (numskins < 1)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	pskintype = (daliasskintype_t *)&pinmodel[1];

	pskindesc = Hunk_AllocName (numskins * sizeof (maliasskindesc_t),
								loadname);

	pheader->skindesc = (byte *)pskindesc - (byte *)pheader;

	for (i=0 ; i<numskins ; i++)
	{
		aliasskintype_t	skintype;

		skintype = LittleLong (pskintype->type);
		pskindesc[i].type = skintype;

		if (skintype == ALIAS_SKIN_SINGLE)
		{
			pskintype = (daliasskintype_t *)
					Mod_LoadAliasSkin (pskintype + 1,
									   &pskindesc[i].skin,
									   skinsize, pheader);
		}
		else
		{
			pskintype = (daliasskintype_t *)
					Mod_LoadAliasSkinGroup (pskintype + 1,
											&pskindesc[i].skin,
											skinsize, pheader);
		}
	}

//
// set base s and t vertices
//
	pstverts = (stvert_t *)&pmodel[1];
	pinstverts = (stvert_t *)pskintype;

	pheader->stverts = (byte *)pstverts - (byte *)pheader;

	//jfm: change for new models (use num_st_verts)
	for (i=0 ; i<pmodel->num_st_verts; i++)
	{
		pstverts[i].onseam = LittleLong (pinstverts[i].onseam);
	// put s and t in 16.16 format
		pstverts[i].s = LittleLong (pinstverts[i].s) << 16;
		pstverts[i].t = LittleLong (pinstverts[i].t) << 16;
	}

//
// set up the triangles
//
	//jfm: change for new models (use num_st_verts)
	ptri = (mtriangle_t *)&pstverts[pmodel->num_st_verts];
	pintriangles = (dnewtriangle_t *)&pinstverts[pmodel->num_st_verts];

	pheader->triangles = (byte *)ptri - (byte *)pheader;

	for (i=0 ; i<pmodel->numtris ; i++)
	{
		ptri[i].facesfront = LittleLong (pintriangles[i].facesfront);

		for (j=0 ; j<3 ; j++)
		{
			ptri[i].vertindex[j] = LittleShort (pintriangles[i].vertindex[j]);
			ptri[i].stindex[j] = LittleShort (pintriangles[i].stindex[j]);
		}
	}

//
// load the frames
//
	if (numframes < 1)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of frames: %d\n", numframes);

	pframetype = (daliasframetype_t *)&pintriangles[pmodel->numtris];

	mins[0] = mins[1] = mins[2] = 32768;
	maxs[0] = maxs[1] = maxs[2] = -32768;

	for (i=0 ; i<numframes ; i++)
	{
		aliasframetype_t	frametype;

		frametype = LittleLong (pframetype->type);
		pheader->frames[i].type = frametype;


		if (frametype == ALIAS_SINGLE)
		{
			pframetype = (daliasframetype_t *)
					Mod_LoadAliasFrame (pframetype + 1,
										&pheader->frames[i].frame,
										pmodel->numverts,
										&pheader->frames[i].bboxmin,
										&pheader->frames[i].bboxmax,
										pheader, pheader->frames[i].name,
										pmodel);
		}
		else
		{
			pframetype = (daliasframetype_t *)
					Mod_LoadAliasGroup (pframetype + 1,
										&pheader->frames[i].frame,
										pmodel->numverts,
										&pheader->frames[i].bboxmin,
										&pheader->frames[i].bboxmax,
										pheader, pheader->frames[i].name,
										pmodel);
		}
	}

	mod->type = mod_alias;

// FIXME: do this right
//	mod->mins[0] = mod->mins[1] = mod->mins[2] = -16;
//	mod->maxs[0] = mod->maxs[1] = mod->maxs[2] = 16;
	mod->mins[0] = mins[0];
	mod->mins[1] = mins[1];
	mod->mins[2] = mins[2];
	mod->maxs[0] = maxs[0];
	mod->maxs[1] = maxs[1];
	mod->maxs[2] = maxs[2];
//
// move the complete, relocatable alias model to the cache
//	
	end = Hunk_LowMark ();
	total = end - start;

	Cache_Alloc (&mod->cache, total, mod->name);
	if (!mod->cache.data)
		return;
	memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}	

/*
=================
Mod_LoadAliasModel
loads old model fmt, converts to new
=================
*/
void Mod_LoadAliasModel (model_t *mod, void *buffer)
{
	int					i;
	mdl_t				*pinmodel;
	newmdl_t			*pmodel;
	stvert_t			*pstverts, *pinstverts;
	aliashdr_t			*pheader;
	mtriangle_t			*ptri;
	dtriangle_t			*pintriangles;
	int					version, numframes, numskins;
	int					size;
	daliasframetype_t	*pframetype;
	daliasskintype_t	*pskintype;
	maliasskindesc_t	*pskindesc;
	int					skinsize;
	int					start, end, total;
	int					j;

//	Mod_SaveAliasModel (mod->name, buffer);
	
	start = Hunk_LowMark ();

	pinmodel = (mdl_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != ALIAS_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

//
// allocate space for a working header, plus all the data except the frames,
// skin and group info
//
	size = 	sizeof (aliashdr_t) + (LittleLong (pinmodel->numframes) - 1) *
			 sizeof (pheader->frames[0]) +
			sizeof (newmdl_t) +
			LittleLong (pinmodel->numverts) * sizeof (stvert_t) +
			LittleLong (pinmodel->numtris) * sizeof (mtriangle_t);

	pheader = Hunk_AllocName (size, loadname);
	pmodel = (newmdl_t *) ((byte *)&pheader[1] +
			(LittleLong (pinmodel->numframes) - 1) *
			 sizeof (pheader->frames[0]));
	
//	mod->cache.data = pheader;
	mod->flags = LittleLong (pinmodel->flags);

//
// endian-adjust and copy the data, starting with the alias model header
//
	pmodel->boundingradius = LittleFloat (pinmodel->boundingradius);
	pmodel->numskins = LittleLong (pinmodel->numskins);
	pmodel->skinwidth = LittleLong (pinmodel->skinwidth);
	pmodel->skinheight = LittleLong (pinmodel->skinheight);

	if (pmodel->skinheight > MAX_SKIN_HEIGHT)
		Sys_Error ("model %s has a skin taller than %d", mod->name,
				   MAX_SKIN_HEIGHT);

	pmodel->numverts = LittleLong (pinmodel->numverts);
	pmodel->num_st_verts = LittleLong (pinmodel->numverts);

	if (pmodel->numverts <= 0)
		Sys_Error ("model %s has no vertices", mod->name);

	if (pmodel->numverts > MAXALIASVERTS)
		Sys_Error ("model %s has too many vertices", mod->name);

	pmodel->numtris = LittleLong (pinmodel->numtris);

	if (pmodel->numtris <= 0)
		Sys_Error ("model %s has no triangles", mod->name);

	pmodel->numframes = LittleLong (pinmodel->numframes);
	pmodel->size = LittleFloat (pinmodel->size) * ALIAS_BASE_SIZE_RATIO;
	mod->synctype = LittleLong (pinmodel->synctype);
	mod->numframes = pmodel->numframes;

	for (i=0 ; i<3 ; i++)
	{
		pmodel->scale[i] = LittleFloat (pinmodel->scale[i]);
		pmodel->scale_origin[i] = LittleFloat (pinmodel->scale_origin[i]);
		pmodel->eyeposition[i] = LittleFloat (pinmodel->eyeposition[i]);
	}

	numskins = pmodel->numskins;
	numframes = pmodel->numframes;

	if (pmodel->skinwidth & 0x03)
		Sys_Error ("Mod_LoadAliasModel: skinwidth not multiple of 4");

	pheader->model = (byte *)pmodel - (byte *)pheader;

//
// load the skins
//
	skinsize = pmodel->skinheight * pmodel->skinwidth;

	if (numskins < 1)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of skins: %d\n", numskins);

	pskintype = (daliasskintype_t *)&pinmodel[1];

	pskindesc = Hunk_AllocName (numskins * sizeof (maliasskindesc_t),
								loadname);

	pheader->skindesc = (byte *)pskindesc - (byte *)pheader;

	for (i=0 ; i<numskins ; i++)
	{
		aliasskintype_t	skintype;

		skintype = LittleLong (pskintype->type);
		pskindesc[i].type = skintype;

		if (skintype == ALIAS_SKIN_SINGLE)
		{
			pskintype = (daliasskintype_t *)
					Mod_LoadAliasSkin (pskintype + 1,
									   &pskindesc[i].skin,
									   skinsize, pheader);
		}
		else
		{
			pskintype = (daliasskintype_t *)
					Mod_LoadAliasSkinGroup (pskintype + 1,
											&pskindesc[i].skin,
											skinsize, pheader);
		}
	}

//
// set base s and t vertices
//
	pstverts = (stvert_t *)&pmodel[1];
	pinstverts = (stvert_t *)pskintype;

	pheader->stverts = (byte *)pstverts - (byte *)pheader;

	//jfm: change for new models (use num_st_verts)
	for (i=0 ; i<pmodel->num_st_verts; i++)
	{
		pstverts[i].onseam = LittleLong (pinstverts[i].onseam);
	// put s and t in 16.16 format
		pstverts[i].s = LittleLong (pinstverts[i].s) << 16;
		pstverts[i].t = LittleLong (pinstverts[i].t) << 16;
	}

//
// set up the triangles
//
	//jfm: change for new models (use num_st_verts)
	ptri = (mtriangle_t *)&pstverts[pmodel->num_st_verts];
	pintriangles = (dtriangle_t *)&pinstverts[pmodel->num_st_verts];

	pheader->triangles = (byte *)ptri - (byte *)pheader;

	for (i=0 ; i<pmodel->numtris ; i++)
	{
		ptri[i].facesfront = LittleLong (pintriangles[i].facesfront);

		for (j=0 ; j<3 ; j++)
		{
#if 0
			if (pintriangles[i].vertindex[j]>Q_MAXSHORT)
			{
				Sys_Error ("ind too big!\n");
			}
#endif
			ptri[i].vertindex[j] =(short) LittleLong (pintriangles[i].vertindex[j]);
			ptri[i].stindex[j] = ptri[i].vertindex[j];	//MAKE A COPY
		}
	}

//
// load the frames
//
	if (numframes < 1)
		Sys_Error ("Mod_LoadAliasModel: Invalid # of frames: %d\n", numframes);

	pframetype = (daliasframetype_t *)&pintriangles[pmodel->numtris];

	mins[0] = mins[1] = mins[2] = 32768;
	maxs[0] = maxs[1] = maxs[2] = -32768;

	for (i=0 ; i<numframes ; i++)
	{
		aliasframetype_t	frametype;

		frametype = LittleLong (pframetype->type);
		pheader->frames[i].type = frametype;


		if (frametype == ALIAS_SINGLE)
		{
			pframetype = (daliasframetype_t *)
					Mod_LoadAliasFrame (pframetype + 1,
										&pheader->frames[i].frame,
										pmodel->numverts,
										&pheader->frames[i].bboxmin,
										&pheader->frames[i].bboxmax,
										pheader, pheader->frames[i].name,
										pmodel);
		}
		else
		{
			pframetype = (daliasframetype_t *)
					Mod_LoadAliasGroup (pframetype + 1,
										&pheader->frames[i].frame,
										pmodel->numverts,
										&pheader->frames[i].bboxmin,
										&pheader->frames[i].bboxmax,
										pheader, pheader->frames[i].name,
										pmodel);
		}
	}

	mod->type = mod_alias;

// FIXME: do this right
//	mod->mins[0] = mod->mins[1] = mod->mins[2] = -16;
//	mod->maxs[0] = mod->maxs[1] = mod->maxs[2] = 16;
	mod->mins[0] = mins[0];
	mod->mins[1] = mins[1];
	mod->mins[2] = mins[2];
	mod->maxs[0] = maxs[0];
	mod->maxs[1] = maxs[1];
	mod->maxs[2] = maxs[2];
//
// move the complete, relocatable alias model to the cache
//	
	end = Hunk_LowMark ();
	total = end - start;

	Cache_Alloc (&mod->cache, total, mod->name);
	if (!mod->cache.data)
		return;
	memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}

//=============================================================================

/*
=================
Mod_LoadSpriteFrame
=================
*/
void * Mod_LoadSpriteFrame (model_t *mod, void * pin, mspriteframe_t **ppframe)
{
	dspriteframe_t		*pinframe;
	mspriteframe_t		*pspriteframe;
	int					i, width, height, size, origin[2];
	unsigned short		*ppixout;
	byte				*ppixin;

	pinframe = (dspriteframe_t *)pin;

	width = LittleLong (pinframe->width);
	height = LittleLong (pinframe->height);
	size = width * height;

	pspriteframe = Hunk_AllocName (sizeof (mspriteframe_t) + size*r_pixbytes,
								   mod->name);

	memset (pspriteframe, 0, sizeof (mspriteframe_t) + size);
	*ppframe = pspriteframe;

	pspriteframe->width = width;
	pspriteframe->height = height;
	origin[0] = LittleLong (pinframe->origin[0]);
	origin[1] = LittleLong (pinframe->origin[1]);

	pspriteframe->up = origin[1];
	pspriteframe->down = origin[1] - height;
	pspriteframe->left = origin[0];
	pspriteframe->right = width + origin[0];

	if (r_pixbytes == 1)
	{
		memcpy (&pspriteframe->pixels[0], (byte *)(pinframe + 1), size);
	}
	else if (r_pixbytes == 2)
	{
		ppixin = (byte *)(pinframe + 1);
		ppixout = (unsigned short *)&pspriteframe->pixels[0];

		for (i=0 ; i<size ; i++)
			ppixout[i] = d_8to16table[ppixin[i]];
	}
	else
	{
		Sys_Error ("Mod_LoadSpriteFrame: driver set invalid r_pixbytes: %d\n",
				 r_pixbytes);
	}

	return (void *)((byte *)pinframe + sizeof (dspriteframe_t) + size);
}


/*
=================
Mod_LoadSpriteGroup
=================
*/
void * Mod_LoadSpriteGroup (model_t *mod, void * pin, mspriteframe_t **ppframe)
{
	dspritegroup_t		*pingroup;
	mspritegroup_t		*pspritegroup;
	int					i, numframes;
	dspriteinterval_t	*pin_intervals;
	float				*poutintervals;
	void				*ptemp;

	pingroup = (dspritegroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);

	pspritegroup = Hunk_AllocName (sizeof (mspritegroup_t) +
				(numframes - 1) * sizeof (pspritegroup->frames[0]), mod->name);

	pspritegroup->numframes = numframes;

	*ppframe = (mspriteframe_t *)pspritegroup;

	pin_intervals = (dspriteinterval_t *)(pingroup + 1);

	poutintervals = Hunk_AllocName (numframes * sizeof (float), mod->name);

	pspritegroup->intervals = poutintervals;

	for (i=0 ; i<numframes ; i++)
	{
		*poutintervals = LittleFloat (pin_intervals->interval);
		if (*poutintervals <= 0.0)
			Sys_Error ("Mod_LoadSpriteGroup: interval<=0");

		poutintervals++;
		pin_intervals++;
	}

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
	{
		ptemp = Mod_LoadSpriteFrame (mod,ptemp, &pspritegroup->frames[i]);
	}

	return ptemp;
}


/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadSpriteModel (model_t *mod, void *buffer)
{
	int					i;
	int					version;
	dsprite_t			*pin;
	msprite_t			*psprite;
	int					numframes;
	int					size;
	dspriteframetype_t	*pframetype;
	
	pin = (dsprite_t *)buffer;

	version = LittleLong (pin->version);
	if (version != SPRITE_VERSION)
		Sys_Error ("%s has wrong version number "
				 "(%i should be %i)", mod->name, version, SPRITE_VERSION);

	numframes = LittleLong (pin->numframes);

	size = sizeof (msprite_t) +	(numframes - 1) * sizeof (psprite->frames);

	psprite = Hunk_AllocName (size, mod->name);

	mod->cache.data = psprite;

	psprite->type = LittleLong (pin->type);
	psprite->maxwidth = LittleLong (pin->width);
	psprite->maxheight = LittleLong (pin->height);
	psprite->beamlength = LittleFloat (pin->beamlength);
	mod->synctype = LittleLong (pin->synctype);
	psprite->numframes = numframes;

	mod->mins[0] = mod->mins[1] = -psprite->maxwidth/2;
	mod->maxs[0] = mod->maxs[1] = psprite->maxwidth/2;
	mod->mins[2] = -psprite->maxheight/2;
	mod->maxs[2] = psprite->maxheight/2;
	
//
// load the frames
//
	if (numframes < 1)
		Sys_Error ("Mod_LoadSpriteModel: Invalid # of frames: %d\n", numframes);

	mod->numframes = numframes;
	mod->flags = 0;

	pframetype = (dspriteframetype_t *)(pin + 1);

	for (i=0 ; i<numframes ; i++)
	{
		spriteframetype_t	frametype;

		frametype = LittleLong (pframetype->type);
		psprite->frames[i].type = frametype;

		if (frametype == SPR_SINGLE)
		{
			pframetype = (dspriteframetype_t *)
					Mod_LoadSpriteFrame (mod, pframetype + 1,
										 &psprite->frames[i].frameptr);
		}
		else
		{
			pframetype = (dspriteframetype_t *)
					Mod_LoadSpriteGroup (mod, pframetype + 1,
										 &psprite->frames[i].frameptr);
		}
	}

	mod->type = mod_sprite;
}

//=============================================================================

/*
================
Mod_Print
================
*/
void Mod_Print (void)
{
	int		i;
	model_t	*mod;

	Con_Printf ("Cached models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		Con_Printf ("%8p : %s",mod->cache.data, mod->name);
		if (mod->needload & NL_UNREFERENCED)
			Con_Printf (" (!R)");
		if (mod->needload & NL_NEEDS_LOADED)
			Con_Printf (" (!P)");
		Con_Printf ("\n");
	}
}


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2004/12/19 17:25:55  sezero
 * Another parentheses warning fix
 *
 * Revision 1.2  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:05:18  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 13    3/12/98 4:44p Jmonroe
 * 
 * 12    3/12/98 1:12p Jmonroe
 * removed strcmp from render code
 * 
 * 11    3/10/98 6:42p Jmonroe
 * first pass of new model fmt, fully in view render needs work
 * 
 * 10    3/06/98 4:57p Mgummelt
 * 
 * 9     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 8     2/27/98 3:54p Jmonroe
 * changed hull 4 size
 * 
 * 7     2/20/98 3:54p Mgummelt
 * 
 * 6     2/10/98 2:42p Jmonroe
 * improved precache some more
 * 
 * 5     2/04/98 12:53a Jmonroe
 * added fastfabs
 * 
 * 4     1/26/98 7:26p Jmonroe
 * inline math funcs
 * 
 * 3     1/20/98 11:09a Jmonroe
 * fix for bad model shit because of the remaining particles in restarts
 * 
 * 2     1/16/98 4:24p Jmonroe
 * rick's model tests
 * 
 * 22    1/16/98 4:23p Jmonroe
 * rick's changes
 * 
 * 20    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 19    8/29/97 4:48p Rjohnson
 * Changed hull dimmensions
 * 
 * 18    8/29/97 2:49p Rjohnson
 * Clipping hull change
 * 
 * 17    8/19/97 11:28a Rjohnson
 * Fix for black models
 * 
 * 16    8/18/97 12:03a Rjohnson
 * Added loading progress
 * 
 * 15    8/17/97 4:14p Rjohnson
 * Fix for model lighting
 * 
 * 14    6/13/97 2:22p Rjohnson
 * Added the 2nd translucent water
 * 
 * 13    6/12/97 11:08a Rjohnson
 * Only water is translucent now
 * 
 * 12    6/12/97 10:29a Rjohnson
 * Only water textures should be translucent now
 * 
 * 11    6/03/97 5:50p Rjohnson
 * Added translucent water
 * 
 * 10    5/09/97 3:52p Rjohnson
 * Change to allow more than 256 precache models
 * 
 * 9     5/06/97 3:50p Rjohnson
 * Added new clipping hulls
 * 
 * 8     3/07/97 1:49p Rjohnson
 * Id Updates
 * 
 * 7     3/06/97 10:29a Rjohnson
 * Added memory statistics
 * 
 * 6     2/18/97 2:49p Rjohnson
 * Forgot headers
 * 
 * 4     2/10/97 2:59p Rjohnson
 * Changed max_map_hulls to 8 and added a 5th test hull
 */
