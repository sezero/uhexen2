// d_edge.c

#include "quakedef.h"
#include "d_local.h"

static int	miplevel;

float		scale_for_mip;
int			screenwidth;
int			ubasestep, errorterm, erroradjustup, erroradjustdown;
int			vstartscan;

// FIXME: should go away
extern void			R_RotateBmodel (void);
extern void			R_TransformFrustum (void);

vec3_t		transformed_modelorg;

/*
==============
D_DrawPoly

==============
*/
void D_DrawPoly (void)
{
// this driver takes spans, not polygons
}


/*
=============
D_MipLevelForScale
=============
*/
int D_MipLevelForScale (float scale)
{
	int		lmiplevel;

	if (scale >= d_scalemip[0] )
		lmiplevel = 0;
	else if (scale >= d_scalemip[1] )
		lmiplevel = 1;
	else if (scale >= d_scalemip[2] )
		lmiplevel = 2;
	else
		lmiplevel = 3;

	if (lmiplevel < d_minmip)
		lmiplevel = d_minmip;

	return lmiplevel;
}


/*
==============
D_DrawSolidSurface
==============
*/

// FIXME: clean this up

void D_DrawSolidSurface (surf_t *surf, int color)
{
	espan_t	*span;
	byte	*pdest;
	int		u, u2, pix;
	
	pix = (color<<24) | (color<<16) | (color<<8) | color;
	for (span=surf->spans ; span ; span=span->pnext)
	{
		pdest = (byte *)d_viewbuffer + screenwidth*span->v;
		u = span->u;
		u2 = span->u + span->count - 1;
		((byte *)pdest)[u] = pix;

		if (u2 - u < 8)
		{
			for (u++ ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
		else
		{
			for (u++ ; u & 3 ; u++)
				((byte *)pdest)[u] = pix;

			u2 -= 4;
			for ( ; u <= u2 ; u+=4)
				*(int *)((byte *)pdest + u) = pix;
			u2 += 4;
			for ( ; u <= u2 ; u++)
				((byte *)pdest)[u] = pix;
		}
	}
}


/*
==============
D_CalcGradients
==============
*/
void D_CalcGradients (msurface_t *pface)
{
	mplane_t	*pplane;
	float		mipscale;
	vec3_t		p_temp1;
	vec3_t		p_saxis, p_taxis;
	float		t;

	pplane = pface->plane;

	mipscale = 1.0 / (float)(1 << miplevel);

	TransformVector (pface->texinfo->vecs[0], p_saxis);
	TransformVector (pface->texinfo->vecs[1], p_taxis);

	t = xscaleinv * mipscale;
	d_sdivzstepu = p_saxis[0] * t;
	d_tdivzstepu = p_taxis[0] * t;

	t = yscaleinv * mipscale;
	d_sdivzstepv = -p_saxis[1] * t;
	d_tdivzstepv = -p_taxis[1] * t;

	d_sdivzorigin = p_saxis[2] * mipscale - xcenter * d_sdivzstepu -
			ycenter * d_sdivzstepv;
	d_tdivzorigin = p_taxis[2] * mipscale - xcenter * d_tdivzstepu -
			ycenter * d_tdivzstepv;

	VectorScale (transformed_modelorg, mipscale, p_temp1);

	t = 0x10000*mipscale;
	sadjust = ((fixed16_t)(DotProduct (p_temp1, p_saxis) * 0x10000 + 0.5)) -
			((pface->texturemins[0] << 16) >> miplevel)
			+ pface->texinfo->vecs[0][3]*t;
	tadjust = ((fixed16_t)(DotProduct (p_temp1, p_taxis) * 0x10000 + 0.5)) -
			((pface->texturemins[1] << 16) >> miplevel)
			+ pface->texinfo->vecs[1][3]*t;

//
// -1 (-epsilon) so we never wander off the edge of the texture
//
	bbextents = ((pface->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((pface->extents[1] << 16) >> miplevel) - 1;
}


/*
==============
D_DrawSurfaces
==============
*/
void D_DrawSurfaces (qboolean Translucent)
{
	surf_t			*s;
	msurface_t		*pface;
	surfcache_t		*pcurrentcache;
	vec3_t			world_transformed_modelorg;
	vec3_t			local_modelorg;
//	static int		FoundTrans = 0;
	int count;

	// Restore the settings
	currententity = &cl_entities[0];
//	VectorCopy (world_transformed_modelorg,
//				transformed_modelorg);
	VectorCopy (base_vpn, vpn);
	VectorCopy (base_vup, vup);
	VectorCopy (base_vright, vright);
	VectorCopy (base_modelorg, modelorg);
	R_TransformFrustum ();

	TransformVector (modelorg, transformed_modelorg);
	VectorCopy (transformed_modelorg, world_transformed_modelorg);

// TODO: could preset a lot of this at mode set time
	if (r_drawflat.value)
	{
		if (Translucent) return;

		for (s = &surfaces[1] ; s<surface_p ; s++)
		{
			if (!s->spans)
				continue;

			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;

			D_DrawSolidSurface (s, (int)s->data & 0xFF);
			D_DrawZSpans (s->spans);
		}
	}
	else
	{
		if (!Translucent)
		{
			for (s = &surfaces[1] ; s<surface_p ; s++)
			{
				if (!s->spans)
					continue;

				r_drawnpolycount++;

				if (s->flags & SURF_TRANSLUCENT)
				{
					//FoundTrans++;
					continue;
				}

				d_zistepu = s->d_zistepu;
				d_zistepv = s->d_zistepv;
				d_ziorigin = s->d_ziorigin;

				//if(!strncmp(pface->texinfo->texture->name,"*BLACK",6))
				if (s->flags & SURF_DRAWBLACK)
				{
					D_DrawSolidSurface (s, 0);//black vis-breaker, no turb
					D_DrawZSpans (s->spans);
					continue;
				}
				if (s->flags & SURF_DRAWSKY)
				{
					if (!r_skymade)
					{
						R_MakeSky ();
					}

					D_DrawSkyScans8 (s->spans);
					D_DrawZSpans (s->spans);
				}
				else if (s->flags & SURF_DRAWBACKGROUND)
				{
				// set up a gradient for the background surface that places it
				// effectively at infinity distance from the viewpoint
					d_zistepu = 0;
					d_zistepv = 0;
					d_ziorigin = -0.9;

					D_DrawSolidSurface (s, (int)r_clearcolor.value & 0xFF);
					D_DrawZSpans (s->spans);
				}
				else if (s->flags & SURF_DRAWTURB)
				{
					pface = s->data;
					miplevel = 0;
					cacheblock = (pixel_t *)
							((byte *)pface->texinfo->texture +
							pface->texinfo->texture->offsets[0]);
					cachewidth = 64;

					if (s->insubmodel)
					{
					// FIXME: we don't want to do all this for every polygon!
					// TODO: store once at start of frame
						currententity = s->entity;	//FIXME: make this passed in to
													// R_RotateBmodel ()
						VectorSubtract (r_origin, currententity->origin,
								local_modelorg);
						TransformVector (local_modelorg, transformed_modelorg);

						R_RotateBmodel ();	// FIXME: don't mess with the frustum,
											// make entity passed in
					}

					D_CalcGradients (pface);
					Turbulent8 (s);
					D_DrawZSpans (s->spans);

					if (s->insubmodel)
					{
					//
					// restore the old drawing state
					// FIXME: we don't want to do this every time!
					// TODO: speed up
					//
						currententity = &cl_entities[0];
						VectorCopy (world_transformed_modelorg,
									transformed_modelorg);
						VectorCopy (base_vpn, vpn);
						VectorCopy (base_vup, vup);
						VectorCopy (base_vright, vright);
						VectorCopy (base_modelorg, modelorg);
						R_TransformFrustum ();
					}
				}
				else
				{
					if (s->insubmodel)
					{
					// FIXME: we don't want to do all this for every polygon!
					// TODO: store once at start of frame
						currententity = s->entity;	//FIXME: make this passed in to
													// R_RotateBmodel ()
						VectorSubtract (r_origin, currententity->origin, local_modelorg);
						TransformVector (local_modelorg, transformed_modelorg);

						R_RotateBmodel ();	// FIXME: don't mess with the frustum,
											// make entity passed in
					}

					pface = s->data;
					miplevel = D_MipLevelForScale (s->nearzi * scale_for_mip
					* pface->texinfo->mipadjust);

				// FIXME: make this passed in to D_CacheSurface
					pcurrentcache = D_CacheSurface (pface, miplevel);

					cacheblock = (pixel_t *)pcurrentcache->data;
					cachewidth = pcurrentcache->width;

					D_CalcGradients (pface);

					(*d_drawspans) (s->spans);

					D_DrawZSpans (s->spans);

					if (s->insubmodel)
					{
					//
					// restore the old drawing state
					// FIXME: we don't want to do this every time!
					// TODO: speed up
					//
						currententity = &cl_entities[0];
						VectorCopy (world_transformed_modelorg,
									transformed_modelorg);
						VectorCopy (base_vpn, vpn);
						VectorCopy (base_vup, vup);
						VectorCopy (base_vright, vright);
						VectorCopy (base_modelorg, modelorg);
						R_TransformFrustum ();
					}
				}
			}
		}
		else //if (FoundTrans)
		{
			count = 0;

			for (s = &surfaces[1] ; s<surface_p ; s++)
			{
				if (!s->spans || !(s->flags & SURF_TRANSLUCENT))
					continue;

				count++;
				//FoundTrans--;

				d_zistepu = s->d_zistepu;
				d_zistepv = s->d_zistepv;
				d_ziorigin = s->d_ziorigin;


				//if(!strncmp(pface->texinfo->texture->name,"*BLACK",6))
				if (s->flags & SURF_DRAWBLACK)
				{
					D_DrawSolidSurface (s, 0);//black vis-breaker, no turb
					D_DrawZSpans (s->spans);
					continue;
				}
				if (s->flags & SURF_DRAWTURB)
				{
					pface = s->data;
					miplevel = 0;
					cacheblock = (pixel_t *)
							((byte *)pface->texinfo->texture +
							pface->texinfo->texture->offsets[0]);
					cachewidth = 64;

					if (s->insubmodel)
					{
					// FIXME: we don't want to do all this for every polygon!
					// TODO: store once at start of frame
						currententity = s->entity;	//FIXME: make this passed in to
													// R_RotateBmodel ()
						VectorSubtract (r_origin, currententity->origin,
								local_modelorg);
						TransformVector (local_modelorg, transformed_modelorg);

						R_RotateBmodel ();	// FIXME: don't mess with the frustum,
											// make entity passed in
					}

					D_CalcGradients (pface);
					Turbulent8 (s);
	//				D_DrawZSpans (s->spans);

					if (s->insubmodel)
					{
					//
					// restore the old drawing state
					// FIXME: we don't want to do this every time!
					// TODO: speed up
					//
						currententity = &cl_entities[0];
						VectorCopy (world_transformed_modelorg,
									transformed_modelorg);
						VectorCopy (base_vpn, vpn);
						VectorCopy (base_vup, vup);
						VectorCopy (base_vright, vright);
						VectorCopy (base_modelorg, modelorg);
						R_TransformFrustum ();
					}
				}
				else
				{
					if (s->insubmodel)
					{
					// FIXME: we don't want to do all this for every polygon!
					// TODO: store once at start of frame
						currententity = s->entity;	//FIXME: make this passed in to
													// R_RotateBmodel ()
						VectorSubtract (r_origin, currententity->origin, local_modelorg);
						TransformVector (local_modelorg, transformed_modelorg);

						R_RotateBmodel ();	// FIXME: don't mess with the frustum,
											// make entity passed in
					}

					pface = s->data;
					miplevel = D_MipLevelForScale (s->nearzi * scale_for_mip
					* pface->texinfo->mipadjust);

				// FIXME: make this passed in to D_CacheSurface
					pcurrentcache = D_CacheSurface (pface, miplevel);

					cacheblock = (pixel_t *)pcurrentcache->data;
					cachewidth = pcurrentcache->width;

					D_CalcGradients (pface);

		//				(*d_drawspans) (s->spans);
					D_DrawSpans16T(s->spans);

		//				D_DrawZSpans (s->spans);

					if (s->insubmodel)
					{
					//
					// restore the old drawing state
					// FIXME: we don't want to do this every time!
					// TODO: speed up
					//
						currententity = &cl_entities[0];
						VectorCopy (world_transformed_modelorg,
									transformed_modelorg);
						VectorCopy (base_vpn, vpn);
						VectorCopy (base_vup, vup);
						VectorCopy (base_vright, vright);
						VectorCopy (base_modelorg, modelorg);
						R_TransformFrustum ();
					}
				}
			}
			//Con_Printf("   Surf is %d\n",count);
		}
	}
}

