/*
 * nodraw.c
 * $Id: nodraw.c,v 1.4 2007-12-14 16:41:25 sezero Exp $
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "mathlib.h"
#include "bspfile.h"
#include "bsp5.h"

void Draw_ClearBounds (void)
{
}

void Draw_AddToBounds (vec3_t v)
{
}

void Draw_DrawFace (face_t *f)
{
}

void Draw_ClearWindow (void)
{
}

void Draw_SetRed (void)
{
}

void Draw_SetGrey (void)
{
}

void Draw_SetBlack (void)
{
}

void DrawPoint (vec3_t v)
{
}

void DrawLeaf (node_t *l, int color)
{
}

void DrawBrush (brush_t *b)
{
}

void DrawWinding (winding_t *w)
{
}

void DrawTri (vec3_t p1, vec3_t p2, vec3_t p3)
{
}

void DrawPortal (portal_t *portal)
{
}
