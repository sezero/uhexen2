/*
 * $Id: vertexbuffer.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
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

#ifndef __VERTEXBUFFER_H
#define __VERTEXBUFFER_H

#ifdef __VBCC__
#pragma amiga-align
#endif

#include <Warp3D/Warp3D.h>

#ifdef __VBCC__
#pragma default-align
#endif


struct TCoord_t
{
    W3D_Float	s,t;

};

struct MGLVertex_t {

	W3D_Vertex	v; //currently 64 bytes

	struct TCoord_t	tcoord; //virtual unit
	ULONG		outcode;

	int		xi,yi,zi;

	float		bx,by,bz,bw;
	GLfloat		q;

	GLuint		normal;   //NormalBuffer-index

	GLuint		cbuf_pos; //unused (16 byte align)
	GLubyte		color[4]; //for vertexarrays
};

typedef struct MGLVertex_t MGLVertex;

enum {
	MGL_CLIP_NEGW   =   1<<0,
	MGL_CLIP_TOP    =   1<<1,
	MGL_CLIP_BOTTOM =   1<<2,
	MGL_CLIP_LEFT   =   1<<3,
	MGL_CLIP_RIGHT  =   1<<4,
	MGL_CLIP_FRONT  =   1<<5,
	MGL_CLIP_BACK   =   1<<6
};


/* used buffering store non-clipped primitives */
/* used for glDrawArrays/glDrawElements */

typedef struct PolyIndex_s
{
	ULONG numverts;
	ULONG first;
} PolyIndex;

#endif
