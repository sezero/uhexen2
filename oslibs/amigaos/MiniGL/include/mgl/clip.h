/*
 * $Id: clip.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
 *
 * $Date: 2000/04/07 19:44:51 $
 * $Revision: 1.1.1.1 $
 *
 * (C) 1999 by Hyperion
 * All rights reserved
 *
 * This file is part of the MiniGL library project
 * See the file Licence.txt for more details
 *
 */


#ifndef _CLIP_H
#define _CLIP_H


/*
** This structure holds the polygon data for clipping.
*/

typedef struct MGLPolygon_t
{
	int numverts;
	int verts[MGL_MAXVERTS];
} MGLPolygon;


/* Flexible Mesh-index - used for clip-buffering*/

typedef struct PolyBuffer_s
{
	int numverts;
	int nextfree;	//used for clipping
	ULONG type;	//w3dtype
	ULONG verts[64]; //check length for triangle-chains
} PolyBuffer;


void dh_DrawLine(GLcontext context, MGLPolygon *poly);
void dh_DrawPoly(GLcontext context, MGLPolygon *poly);

void hc_ClipPoly(GLcontext context, MGLPolygon *poly, PolyBuffer *out, int clipstart, ULONG or_codes);

void hc_ClipPolyFF(GLcontext context, MGLPolygon *poly, ULONG or_codes);

GLboolean hc_DecideFrontface(GLcontext context, MGLVertex *a, MGLVertex *b, MGLVertex *c);

#endif



