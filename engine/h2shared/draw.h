/*
 * draw.h
 * these are the only functions outside the refresh
 * allowed to touch the vid buffer
 *
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

#ifndef __HX2_DRAW_H
#define __HX2_DRAW_H

#define MAX_DISC 18

extern	qboolean	draw_reinit;

void Draw_Init (void);
void Draw_ReInit (void);

qpic_t *Draw_PicFromWad (const char *name);
qpic_t *Draw_PicFromFile (const char *name);

qpic_t *Draw_CachePic (const char *path);
#if !defined(DRAW_PROGRESSBARS)
qpic_t *Draw_CacheLoadingPic (void);	/* without the progress bars. */
#else
#define Draw_CacheLoadingPic ()		Draw_CachePic ("gfx/menu/loading.lmp")
#endif	/* DRAW_PROGRESSBARS */

void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_PicCropped (int x, int y, qpic_t *pic);
void Draw_SubPic (int x, int y, qpic_t *pic, int srcx, int srcy, int width, int height);
void Draw_SubPicCropped (int x, int y, int h, qpic_t *pic);
void Draw_TransPic (int x, int y, qpic_t *pic);
void Draw_TransPicCropped (int x, int y, qpic_t *pic);
void Draw_ConsoleBackground (int lines);
void Draw_Crosshair (void);

#if defined(GLQUAKE)
void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha);
#endif	/*  GLQUAKE */

void Draw_TransPicTranslate (int x, int y, qpic_t *pic, byte *translation, int p_class);
					/* Only used for the player color selection menu */

#if FULLSCREEN_INTERMISSIONS
# if defined(GLQUAKE)
qpic_t *Draw_CachePicNoTrans (const char *path);
void Draw_IntermissionPic (qpic_t *pic);
# else	/* !GLQUAKE */
qpic_t *Draw_CachePicResize (const char *path, int targetWidth, int targetHeight);
# endif	/*  GLQUAKE */
#endif	/*  FULLSCREEN_INTERMISSIONS */

#if defined(GLQUAKE)
#undef DRAW_LOADINGSKULL
#endif

#if !defined(DRAW_LOADINGSKULL)
#define Draw_BeginDisc()
#define Draw_EndDisc()
#else
void Draw_BeginDisc (void);
void Draw_EndDisc (void);
#endif

void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen (void);

void Draw_Character (int x, int y, unsigned int num);
void Draw_String (int x, int y, const char *str);
void Draw_SmallCharacter (int x, int y, int num);
void Draw_SmallString (int x, int y, const char *str);
void Draw_RedString (int x, int y, const char *str);
void Draw_BigCharacter (int x, int y, int num);

/* game/engine name to draw on the console */
#define GAME_MOD_NAME		ENGINE_NAME
#define ENGINE_WATERMARK	GAME_MOD_NAME " " STRINGIFY(ENGINE_VERSION) " (" PLATFORM_STRING ")"

#endif	/* __HX2_DRAW_H */

