/*
 * screen.h
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

#ifndef __HX2_SCREEN_H
#define __HX2_SCREEN_H

#define PLAQUE_WIDTH	26
	// width for the info plaques

void SCR_Init (void);
void SCR_UpdateScreen (void);
void SCR_UpdateWholeScreen (void);

void SCR_CenterPrint (const char *str);
//void SCR_BringDownConsole (void);

void SCR_BeginLoadingPlaque (void);
void SCR_EndLoadingPlaque (void);

void SCR_DrawLoading (void);	// for the Loading plaque

void SCR_SetPlaqueMessage (const char *msg);	// set pointer to current plaque message
int SCR_ModalMessage (const char *text);

extern	int			total_loading_size;	// global vars for
extern	int			current_loading_size;	// the Loading screen
extern	int			entity_file_size, loading_stage;

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	int			scr_fullupdate;	// set to 0 to force full redraw
extern	int			scr_topupdate;	// set to 0 to force top redraw
extern	int			sb_lines;

extern	int			clearnotify;	// set to 0 whenever notify text is drawn
extern	qboolean	scr_disabled_for_loading;
extern	qboolean	scr_skipupdate;
extern	qboolean	block_drawing;

extern	cvar_t		scr_fov;
extern	cvar_t		scr_fov_adapt;
extern	cvar_t		scr_viewsize;
extern	cvar_t		scr_contrans;
extern	int		trans_level;

// only the refresh window will be updated unless these variables are flagged 
extern	int			scr_copytop;
extern	int			scr_copyeverything;

#endif	/* __HX2_SCREEN_H */

