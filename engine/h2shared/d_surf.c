/*
 * d_surf.c - rasterization driver surface heap manager
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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
#include "d_local.h"
#include "r_local.h"

static float	surfscale;
qboolean	r_cache_thrash = false;	// set if surface cache is thrashing

static int				sc_size;
surfcache_t			*sc_rover, *sc_base;

#define GUARDSIZE       4


int D_SurfaceCacheForRes (int width, int height)
{
	int		i, size, pix;

	i = COM_CheckParm ("-surfcachesize");
	if (i && i < com_argc-1)
	{
		size = atoi(com_argv[i+1]) * 1024;
		return size;
	}

	size = SURFCACHE_SIZE_AT_320X200;

	pix = width*height;
	if (pix > 64000)
		size += (pix-64000)*3;

	return size;
}

static void D_CheckCacheGuard (void)
{
	byte	*s;
	int		i;

	s = (byte *)sc_base + sc_size;
	for (i = 0; i < GUARDSIZE; i++)
	{
		if (s[i] != (byte)i)
			Sys_Error ("%s: failed", __thisfunc__);
	}
}

static void D_ClearCacheGuard (void)
{
	byte	*s;
	int		i;

	s = (byte *)sc_base + sc_size;
	for (i = 0; i < GUARDSIZE; i++)
		s[i] = (byte)i;
}


/*
================
D_InitCaches

================
*/
void D_InitCaches (void *buffer, int size)
{
	if (!msg_suppress_1)
		Con_Printf ("%ik surface cache\n", size/1024);

	sc_size = size - GUARDSIZE;
	sc_base = (surfcache_t *)buffer;
	sc_rover = sc_base;

	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;

	D_ClearCacheGuard ();
}


/*
==================
D_FlushCaches
==================
*/
void D_FlushCaches (void)
{
	surfcache_t	*c;

	if (!sc_base)
		return;

	for (c = sc_base ; c ; c = c->next)
	{
		if (c->owner)
			*c->owner = NULL;
	}

	sc_rover = sc_base;
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
}

/*
=================
D_SCAlloc
=================
*/
static surfcache_t *D_SCAlloc (int width, int size)
{
	surfcache_t		*new_sc;
	qboolean		wrapped_this_time;

	if ((width < 0) || (width > 256))
		Sys_Error ("%s: bad cache width %d", __thisfunc__, width);

	if ((size <= 0) || (size > 0x10000))
		Sys_Error ("%s: bad cache size %d", __thisfunc__, size);

	/* This adds the offset of data[0] in the surfcache_t struct. */
	size += (size_t) &(((surfcache_t *)0)->data[0]);
#define SIZE_ALIGN	(sizeof (surfcache_t *) - 1)
	size = (size + SIZE_ALIGN) & ~SIZE_ALIGN;
#undef	SIZE_ALIGN
	size = (size + 3) & ~3;
	if (size > sc_size)
		Sys_Error ("%s: %i > cache size", __thisfunc__, size);

// if there is not size bytes after the rover, reset to the start
	wrapped_this_time = false;

	if ( !sc_rover || (byte *)sc_rover - (byte *)sc_base > sc_size - size)
	{
		if (sc_rover)
		{
			wrapped_this_time = true;
		}
		sc_rover = sc_base;
	}

// colect and free surfcache_t blocks until the rover block is large enough
	new_sc = sc_rover;
	if (sc_rover->owner)
		*sc_rover->owner = NULL;

	while (new_sc->size < size)
	{
	// free another
		sc_rover = sc_rover->next;
		if (!sc_rover)
			Sys_Error ("%s: hit the end of memory", __thisfunc__);
		if (sc_rover->owner)
			*sc_rover->owner = NULL;

		new_sc->size += sc_rover->size;
		new_sc->next = sc_rover->next;
	}

// create a fragment out of any leftovers
	if (new_sc->size - size > 256)
	{
		sc_rover = (surfcache_t *)( (byte *)new_sc + size);
		sc_rover->size = new_sc->size - size;
		sc_rover->next = new_sc->next;
		sc_rover->width = 0;
		sc_rover->owner = NULL;
		new_sc->next = sc_rover;
		new_sc->size = size;
	}
	else
		sc_rover = new_sc->next;

	new_sc->width = width;
// DEBUG
	if (width > 0)
		new_sc->height = (size - sizeof(*new_sc) + sizeof(new_sc->data)) / width;

	new_sc->owner = NULL;		// should be set properly after return

	if (d_roverwrapped)
	{
		if (wrapped_this_time || (sc_rover >= d_initial_rover))
			r_cache_thrash = true;
	}
	else if (wrapped_this_time)
	{       
		d_roverwrapped = true;
	}

	D_CheckCacheGuard ();	// DEBUG
	return new_sc;
}


/*
=================
D_SCDump
=================
*/
#if 0	/* unused */
static void D_SCDump (void)
{
	surfcache_t		*test;

	for (test = sc_base ; test ; test = test->next)
	{
		if (test == sc_rover)
			Sys_Printf ("ROVER:\n");
		printf ("%p : %i bytes     %i width\n",test, test->size, test->width);
	}
}
#endif

//=============================================================================

/*
================
D_CacheSurface
================
*/
surfcache_t *D_CacheSurface (msurface_t *surface, int miplevel)
{
	surfcache_t	*cache;
	qboolean		DoSurface;

//
// if the surface is animating or flashing, flush the cache
//
	r_drawsurf.texture = R_TextureAnimation (surface->texinfo->texture);
	r_drawsurf.lightadj[0] = d_lightstylevalue[surface->styles[0]];
	r_drawsurf.lightadj[1] = d_lightstylevalue[surface->styles[1]];
	r_drawsurf.lightadj[2] = d_lightstylevalue[surface->styles[2]];
	r_drawsurf.lightadj[3] = d_lightstylevalue[surface->styles[3]];

//
// see if the cache holds apropriate data
//
	cache = surface->cachespots[miplevel];

	DoSurface = false;
	if (cache)
	{
		if ((cache->drawflags & MLS_ABSLIGHT) == MLS_ABSLIGHT &&
			(currententity->drawflags & MLS_ABSLIGHT) != MLS_ABSLIGHT)
		{
			DoSurface = true;
		}
		else if (cache->abslight != currententity->abslight)
		{
			DoSurface = true;
		}
	}

	if (cache && !cache->dlight && surface->dlightframe != r_framecount
			&& cache->texture == r_drawsurf.texture
			&& cache->lightadj[0] == r_drawsurf.lightadj[0]
			&& cache->lightadj[1] == r_drawsurf.lightadj[1]
			&& cache->lightadj[2] == r_drawsurf.lightadj[2]
			&& cache->lightadj[3] == r_drawsurf.lightadj[3] 
			&& !DoSurface)
		return cache;

//
// determine shape of surface
//
	surfscale = 1.0 / (1<<miplevel);
	r_drawsurf.surfmip = miplevel;
	r_drawsurf.surfwidth = surface->extents[0] >> miplevel;
	r_drawsurf.rowbytes = r_drawsurf.surfwidth;
	r_drawsurf.surfheight = surface->extents[1] >> miplevel;

//
// allocate memory if needed
//
	if (!cache)	// if a texture just animated, don't reallocate it
	{
		cache = D_SCAlloc (r_drawsurf.surfwidth,
						   r_drawsurf.surfwidth * r_drawsurf.surfheight);
		surface->cachespots[miplevel] = cache;
		cache->owner = &surface->cachespots[miplevel];
		cache->mipscale = surfscale;
	}

	cache->drawflags = currententity->drawflags;
	cache->abslight = currententity->abslight;

	if (surface->dlightframe == r_framecount)
		cache->dlight = 1;
	else
		cache->dlight = 0;

	r_drawsurf.surfdat = (pixel_t *)cache->data;

	cache->texture = r_drawsurf.texture;
	cache->lightadj[0] = r_drawsurf.lightadj[0];
	cache->lightadj[1] = r_drawsurf.lightadj[1];
	cache->lightadj[2] = r_drawsurf.lightadj[2];
	cache->lightadj[3] = r_drawsurf.lightadj[3];

//
// draw and light the surface texture
//
	r_drawsurf.surf = surface;

	c_surf++;
	R_DrawSurface ();

	return surface->cachespots[miplevel];
}

