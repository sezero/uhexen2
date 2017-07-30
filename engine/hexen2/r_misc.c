/*
 * r_misc.c --
 * $Id: r_misc.c,v 1.14 2007-07-31 21:03:27 sezero Exp $
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
#include "r_local.h"


/*
===============
R_CheckVariables
===============
*/
static void R_CheckVariables (void)
{
	static int	oldbright;

	if (r_fullbright.integer != oldbright)
	{
		oldbright = r_fullbright.integer;
		D_FlushCaches ();	// so all lighting changes
	}
}


/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (void)
{
	int			i;
	float		start, stop, time;
	int			startangle;
	vrect_t		vr;

	if (cls.state != ca_connected)
	{
		Con_Printf("Not connected to a server\n");
		return;
	}

	startangle = r_refdef.viewangles[1];

	start = Sys_DoubleTime ();
	for (i = 0; i < 128; i++)
	{
		r_refdef.viewangles[1] = i/128.0*360.0;

		VID_LockBuffer ();

		R_RenderView ();

		VID_UnlockBuffer ();

		vr.x = r_refdef.vrect.x;
		vr.y = r_refdef.vrect.y;
		vr.width = r_refdef.vrect.width;
		vr.height = r_refdef.vrect.height;
		vr.pnext = NULL;
		VID_Update (&vr);
	}
	stop = Sys_DoubleTime ();
	time = stop-start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);

	r_refdef.viewangles[1] = startangle;
}

/*
================
R_LineGraph
================
*/
static void R_LineGraph (int x, int y, int h, int drawType, int marker)
{
	int		i;
	byte	*dest;
	int		s;

// FIXME: should be disabled on no-buffer adapters, or should be in the driver

	// drawType
	// 0-000: expanded  / solid / background
	// 1-001: condensed / background
	// 2-010: expanded  / holey / background
	// 3-011: condensed / background
	// 4-100: expanded  / solid / no background
	// 5-101: condensed / no background
	// 6-110: expanded  / holey / no background
	// 7-111: condensed / no background

	x += r_refdef.vrect.x;
	y += r_refdef.vrect.y;
	dest = vid.buffer + vid.rowbytes*y + x;
	s = r_graphheight.integer;
	if (s > r_refdef.vrect.height)
	{
		s = r_refdef.vrect.height;
	}

	if (h > s)
	{
		h = s;
	}
	for (i = 0; i < h; i++, dest -= vid.rowbytes)
	{
		dest[0] = ((i+1) % marker == 0) ? 143 : 255;
		if (!(drawType & 1))
		{ // Expanded
			if (!(drawType & 2))
			{ // Solid
				*(dest-vid.rowbytes) = 53;
			}
			dest -= vid.rowbytes;
		}
	}

	if (drawType & 4)
	{ // No background
		return;
	}
	for ( ; i < s; i++, dest -= vid.rowbytes)
	{
		dest[0] = 53;
		if (!(drawType & 1))
		{ // Expanded
			if (!(drawType & 2))
			{ // Solid
				*(dest-vid.rowbytes) = 53;
			}
			dest -= vid.rowbytes;
		}
	}
}

/*
==============
R_TimeGraph

Performance monitoring tool
==============
*/
#define	MAX_TIMINGS		100
#define GRAPH_TYPE_COUNT	2
extern int LastServerMessageSize;
void R_TimeGraph (void)
{
	int	a, x;
	int	drawType, graphType;
	static int	timex;
	static byte	r_timings[MAX_TIMINGS];
	static int	graphMarkers[GRAPH_TYPE_COUNT] =
	{
		10000,
		10
	};

	graphType = r_timegraph.integer;
	if (graphType < 1 || graphType > GRAPH_TYPE_COUNT)
	{
		return;
	}
	drawType = (int)((r_timegraph.value-floor(r_timegraph.value)+1E-3)*10);

	if (graphType == 1)
	{ // Frame times
		a = (Sys_DoubleTime() - r_time1) / 0.01;
		r_timings[timex] = a;
	}
	else
	{ // Packet sizes
		a = LastServerMessageSize/10;
		LastServerMessageSize = 0;
		r_timings[timex] = a;
	}

	a = timex;

	if (r_refdef.vrect.width <= MAX_TIMINGS)
	{
		x = r_refdef.vrect.width-1;
	}
	else
	{
		x = r_refdef.vrect.width - (r_refdef.vrect.width-MAX_TIMINGS)/2;
	}

	do
	{
		R_LineGraph(x, r_refdef.vrect.height-2, r_timings[a], drawType,
						graphMarkers[graphType-1]);
		if (x == 0)
		{
			break;	// screen too small to hold entire thing
		}
		x--;
		a--;
		if (a == -1)
		{
			a = MAX_TIMINGS-1;
		}
	} while (a != timex);

	timex = (timex+1) % MAX_TIMINGS;
}

/*
=============
R_PrintTimes
=============
*/
void R_PrintTimes(void)
{
	float	r_time2;
	float	ms, fps;

	r_lasttime1 = r_time2 = Sys_DoubleTime();

	ms = 1000 * (r_time2 - r_time1);
	fps = 1000 / ms;

	Con_Printf("%3.1f fps %5.0f ms\n%3i/%3i/%3i poly %3i surf\n",
		fps, ms, c_faceclip, r_polycount, r_drawnpolycount, c_surf);

	c_surf = 0;
}

/*
=============
R_PrintDSpeeds
=============
*/
void R_PrintDSpeeds (void)
{
	float	ms, dp_time, r_time2, rw_time, db_time, se_time, de_time, dv_time;

	r_time2 = Sys_DoubleTime ();

	dp_time = (dp_time2 - dp_time1) * 1000;
	rw_time = (rw_time2 - rw_time1) * 1000;
	db_time = (db_time2 - db_time1) * 1000;
	se_time = (se_time2 - se_time1) * 1000;
	de_time = (de_time2 - de_time1) * 1000;
	dv_time = (dv_time2 - dv_time1) * 1000;
	ms = (r_time2 - r_time1) * 1000;

	Con_Printf ("%3i %4.1fp %3iw %4.1fb %3is %4.1fe %4.1fv\n",
				(int)ms, dp_time, (int)rw_time, db_time, (int)se_time, de_time,
				dv_time);
}


/*
=============
R_PrintAliasStats
=============
*/
void R_PrintAliasStats (void)
{
	Con_Printf ("%3i polygon model drawn\n", r_amodels_drawn);
}


/*
===================
R_TransformFrustum
===================
*/
void R_TransformFrustum (void)
{
	int		i;
	vec3_t	v, v2;

	for (i = 0 ; i < 4 ; i++)
	{
		v[0] = screenedge[i].normal[2];
		v[1] = -screenedge[i].normal[0];
		v[2] = screenedge[i].normal[1];

		v2[0] = v[1]*vright[0] + v[2]*vup[0] + v[0]*vpn[0];
		v2[1] = v[1]*vright[1] + v[2]*vup[1] + v[0]*vpn[1];
		v2[2] = v[1]*vright[2] + v[2]*vup[2] + v[0]*vpn[2];

		VectorCopy (v2, view_clipplanes[i].normal);

		view_clipplanes[i].dist = DotProduct (modelorg, v2);
	}
}


#if	!id386 && !id68k

/*
================
TransformVector
================
*/
void TransformVector (vec3_t in, vec3_t out)
{
	out[0] = DotProduct(in,vright);
	out[1] = DotProduct(in,vup);
	out[2] = DotProduct(in,vpn);
}

#endif


/*
================
R_TransformPlane
================
*/
void R_TransformPlane (mplane_t *p, float *normal, float *dist)
{
	float	d;

	d = DotProduct (r_origin, p->normal);
	*dist = p->dist - d;
// TODO: when we have rotating entities, this will need to use the view matrix
	TransformVector (p->normal, normal);
}


/*
===============
R_SetUpFrustumIndexes
===============
*/
static void R_SetUpFrustumIndexes (void)
{
	int		i, j, *pindex;

	pindex = r_frustum_indexes;

	for (i = 0 ; i < 4 ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
		{
			if (view_clipplanes[i].normal[j] < 0)
			{
				pindex[j] = j;
				pindex[j+3] = j+3;
			}
			else
			{
				pindex[j] = j+3;
				pindex[j+3] = j;
			}
		}

	// FIXME: do just once at start
		pfrustum_indexes[i] = pindex;
		pindex += 6;
	}
}


/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	int		edgecount;
	vrect_t		vrect;
	float		w, h;

// don't allow cheats in multiplayer
	if (cl.maxclients > 1)
	{
		Cvar_Set ("r_draworder", "0");
		Cvar_Set ("r_fullbright", "0");
		Cvar_Set ("r_ambient", "0");
		Cvar_Set ("r_drawflat", "0");
	}

	if (r_numsurfs.integer)
	{
		if ((surface_p - surfaces) > r_maxsurfsseen)
			r_maxsurfsseen = surface_p - surfaces;

		Con_Printf ("Used %ld of %ld surfs; %d max\n", (long)(surface_p - surfaces),
				(long)(surf_max - surfaces), r_maxsurfsseen);
	}

	if (r_numedges.integer)
	{
		edgecount = edge_p - r_edges;

		if (edgecount > r_maxedgesseen)
			r_maxedgesseen = edgecount;

		Con_Printf ("Used %d of %d edges; %d max\n", edgecount,
				r_numallocatededges, r_maxedgesseen);
	}

	r_refdef.ambientlight = r_ambient.integer;

	if (r_refdef.ambientlight < 0)
		r_refdef.ambientlight = 0;

	if (!sv.active)
		r_draworder.integer = 0;	// don't let cheaters look behind walls

	R_CheckVariables ();

	R_AnimateLight ();

	r_framecount++;

	numbtofpolys = 0;

#if 0
// debugging
	r_refdef.vieworg[0]	= 80;
	r_refdef.vieworg[1]	= 64;
	r_refdef.vieworg[2]	= 40;
	r_refdef.viewangles[0]	= 0;
	r_refdef.viewangles[1]	= 46.763641357;
	r_refdef.viewangles[2]	= 0;
#endif

// build the transformation matrix for the given view angles
	VectorCopy (r_refdef.vieworg, modelorg);
	VectorCopy (r_refdef.vieworg, r_origin);

	AngleVectors (r_refdef.viewangles, vpn, vright, vup);

// current viewleaf
	r_oldviewleaf = r_viewleaf;
	r_viewleaf = Mod_PointInLeaf (r_origin, cl.worldmodel);

	r_dowarpold = r_dowarp;
	r_dowarp = r_waterwarp.integer && (r_viewleaf->contents <= CONTENTS_WATER);

	if ((r_dowarp != r_dowarpold) || r_viewchanged)
	{
		if (r_dowarp)
		{
			if ((vid.width <= vid.maxwarpwidth) &&
				(vid.height <= vid.maxwarpheight))
			{
				vrect.x = 0;
				vrect.y = 0;
				vrect.width = vid.width;
				vrect.height = vid.height;

				R_SetVrect (&vrect, &r_refdef.vrect, sb_lines);
				R_ViewChanged (vid.aspect);
			}
			else
			{
				w = vid.width;
				h = vid.height;

				if (w > vid.maxwarpwidth)
				{
					h *= (float)vid.maxwarpwidth / w;
					w = vid.maxwarpwidth;
				}

				if (h > vid.maxwarpheight)
				{
					h = vid.maxwarpheight;
					w *= (float)vid.maxwarpheight / h;
				}

				vrect.x = 0;
				vrect.y = 0;
				vrect.width = (int)w;
				vrect.height = (int)h;

				R_SetVrect (&vrect, &r_refdef.vrect, (int)((float)sb_lines * (h/(float)vid.height)));
				R_ViewChanged (vid.aspect * (h / w) * ((float)vid.width / (float)vid.height));
			}
		}
		else
		{
		// scr_vrect alredy holds the original data,
		// therefore no need for extra R_SetVrect()
			r_refdef.vrect = scr_vrect;
			R_ViewChanged (vid.aspect);
		}

		r_viewchanged = false;
	}

// start off with just the four screen edge clip planes
	R_TransformFrustum ();

// save base values
	VectorCopy (vpn, base_vpn);
	VectorCopy (vright, base_vright);
	VectorCopy (vup, base_vup);
	VectorCopy (modelorg, base_modelorg);

	R_SetSkyFrame ();

	R_SetUpFrustumIndexes ();

	r_cache_thrash = false;

// clear frame counts
	c_faceclip = 0;
	d_spanpixcount = 0;
	r_polycount = 0;
	r_drawnpolycount = 0;
	r_wholepolycount = 0;
	r_amodels_drawn = 0;
	r_outofsurfaces = 0;
	r_outofedges = 0;

	D_SetupFrame ();
}

