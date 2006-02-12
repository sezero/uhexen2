/*  Copyright (C) 1996-1997  Id Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    See file, 'COPYING', for details.
*/

/*
 * tyrlight/tyrlite.h
 * Modifications by Kevin Shanahan, 1999-2000
 */

#ifndef __TYRLITE_TYRLITE_H__
#define __TYRLITE_TYRLITE_H__

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "tyrlog.h"
#include "entities.h"
#include "threads.h"

#define ON_EPSILON 0.1
#define MAXLIGHTS  1024

//++ [js] new feature
#define MAX_PATH _MAX_PATH  
#define MAX_ENTRYNUM 32784                  
#define MAX_TEX_NAME 64

typedef struct tex_col {
	char name[MAX_TEX_NAME];
	int red;
	int green;
	int blue;
} tex_col;

typedef struct tex_col_list {
	int	num;
	tex_col* entries;
} tex_col_list;
//-- [js] new feature

void LoadNodes (char *file);

/* TYR - added TestSky */
qboolean TestLine (vec3_t start, vec3_t stop);
qboolean TestSky  (vec3_t start, vec3_t dirn);

void LightFace (int surfnum, qboolean nolight, vec3_t faceoffset);
void TestLightFace (int surfnum, qboolean nolight, vec3_t faceoffset);
void LightLeaf (dleaf_t *leaf);
void FindTexlightColour (int *surf_r, int *surf_g, int *surf_b, char *texname);
void FindTexlightColourExt (int *surf_r, int *surf_g, int *surf_b, char *texname, tex_col_list list);
void LightFaceLIT (int surfnum, qboolean nolight, vec3_t faceoffset);
void CheckTex (void);
void MakeTnodes (dmodel_t *bm);

extern float  scaledist;
extern float  scalecos;
extern float  rangescale;
extern int    worldminlight;
extern vec3_t minlight_color;
extern int    sunlight;
extern vec3_t sunlight_color;
extern vec3_t sunmangle;

extern int    c_culldistplane;
extern int    c_proper;

byte   *GetFileSpace (int size);
extern byte *filebase;

extern vec3_t bsp_origin;
extern vec3_t bsp_xvector;
extern vec3_t bsp_yvector;

void TransformSample (vec3_t in, vec3_t out);
void RotateSample (vec3_t in, vec3_t out);

extern qboolean extrasamples;
extern qboolean compress_ents;
extern qboolean facecounter;
extern qboolean colored;
extern qboolean force;
extern qboolean nominlimit;
extern qboolean makelit;
//++ [js] new feature 
extern qboolean external;
extern qboolean nodefault;
extern tex_col_list tc_list;
//-- [js] new feature
#endif /* __TYRLITE_TYRLITE_H__ */
