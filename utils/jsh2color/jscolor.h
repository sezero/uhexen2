/*
 * jscolor.h
 * $Id$
 *
 * based on MHColour v0.5
 * Copyright (C) 2002 Juraj Styk <jurajstyk@host.sk>
 * Copyright (C) 2005-2012 O.Sezer <sezero@users.sourceforge.net>
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

#ifndef JSCOLOR_H
#define JSCOLOR_H

#define JSH2COLOR_VER	"1.2.6"		/* version string */

#define	MAX_ENTRYNUM	32784
#define	MAX_TEX_NAME	64

extern	miptex_t	miptex[512];
extern	int		numlighttex;
extern	int		faces_ltoffset[MAX_MAP_FACES];
extern	byte		newdlightdata[MAX_MAP_LIGHTING*3];
extern	int		newlightdatasize;
extern	int		num_clights;
extern	int		num_lights;
extern	qboolean	external;
extern	qboolean	nodefault;

void Init_JSColor (void);
void InitDefFile (const char *fname);
void CloseDefFile (void);

void CheckTex (void);
void FindTexlightColor (int *surf_r, int *surf_g, int *surf_b, const char *texname);

#endif	/* JSCOLOR_H */

