/****************************************************************************
*
*						MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MegaGraph Graphics Library 3D functions.
*				This header file contains references to all the MGL 3D
*				functions that are obsolete in the MGL 4.0, since the
*				addition of the new OpenGL 3D API support. These functions
*				will probably be deleted from the API in the next release
*				so if you are using these functions, you should switch over
*				to the more powerful and faster OpenGL functions.
*
*
****************************************************************************/

#ifndef	__MGL3D_H
#define	__MGL3D_H

#if	defined(MGL_FIX3D) || defined(MGL_FLT3D)
#define	MGL_3D
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Define the value used to clear the software ZBuffer. The MGL always uses
 * a > operator for the z compare, and the smallest value is 0.
 */

#define	MGL_ZCLEARVAL	0

/****************************************************************************
REMARKS:
Type definition for all fixed point color values used in the MGL. All fixed
point color values are 32-bits wide, and are represented in 8.16 fixed point
format (8 bits of unsigned integer, 16 bits of fraction) and can represent
unsigned numbers from 0-255.9. Fixed point color values are used to represent
color channel information used for smooth shaded rendering functions.

HEADER:
mgraph.h
****************************************************************************/
typedef fix32_t         fxcolor_t;

/****************************************************************************
REMARKS:
Type definition for all fixed point Z-buffer coordinates used for the MGL 3D
rendering functions (not for OpenGL functions!). All fixed point Z-buffer
coordinates are 32-bits wide and are represented in 4.28 fixed point format
(4 bits of unsigned integer and 28 bits of fraction) and can represent
unsigned numbers from 0 to 7.9. Note that the high bit is reserved for use by
the MGL to handle overflow conditions in the rasterizing code, and all
Z-buffer values must be greater than 0. Note that the Z-buffer coordinates
are always in this format, regardless of whether the actual Z-buffer itself
is represented with 16-bits or 32-bits of precision, and the Z-buffer
rasterizing code will convert the coordinates to the appropriate format
internally.

HEADER:
mgraph.h
****************************************************************************/
typedef long            zfix32_t;

/****************************************************************************
REMARKS:
Structure describing an 8.16 fixed point RGB color value.

HEADER:
mgraph.h

MEMBERS:
r   - Fixed point red channel information (0    - 255.9)
g	- Fixed point green channel information (0	- 255.9)
b   - Fixed point blue channel information (0   - 255.9)
****************************************************************************/
typedef struct {
	fxcolor_t	r,g,b;
	} fxrgb_t;

/****************************************************************************
REMARKS:
Structure describing a 16.16 fixed point vertex, with an associated 8.16
color map index.

HEADER:
mgraph.h

MEMBERS:
c   - Color map index (0    - 255.9)
p   - Fixed point vertex
****************************************************************************/
typedef struct {
	fxcolor_t	c;
	fxpoint_t   p;
	} fxpointc_t;

/****************************************************************************
REMARKS:
Structure representing a 16.16 fixed point vertex, with an associated 8.16
RGB color value.

HEADER:
mgraph.h

MEMBERS:
c   - RGB color value (0 - 255.9 for R,G and B)
p   - Fixed point vertex
****************************************************************************/
typedef struct {
	fxrgb_t		c;
	fxpoint_t	p;
	} fxpointrgb_t;

/****************************************************************************
REMARKS:
Structure representing a 16.16 fixed point vertex, with an associated 4.28
Z-buffer coordinate.

HEADER:
mgraph.h

MEMBERS:
p   - Fixed point vertex
z   - 4.28 Z-buffer coordinate
****************************************************************************/
typedef struct {
	fxpoint_t	p;
	zfix32_t	z;
	} fxpointz_t;

/****************************************************************************
REMARKS:
Structure representing a 16.16 fixed point vertex, with an associated 4.28
Z-buffer coordinate and 8.16 color map index.

HEADER:
mgraph.h

MEMBERS:
c   - Color map index (0 - 255.9)
p	- Fixed point vertex
z   - 4.28 Z-buffer coordinate
****************************************************************************/
typedef struct {
	fxcolor_t	c;
	fxpoint_t	p;
	zfix32_t	z;
	} fxpointcz_t;

/****************************************************************************
REMARKS:
Structure representing a 16.16 fixed point vertex, with an associated 4.28
Z-buffer coordinate and 8.16 RGB color value.

HEADER:
mgraph.h

MEMBERS:
c   - RGB color value (0 - 255.9 for R,G and B)
p	- Fixed point vertex
z   - 4.28 Z-buffer coordinate
****************************************************************************/
typedef struct {
	fxrgb_t		c;
	fxpoint_t	p;
	zfix32_t	z;
	} fxpointrgbz_t;

#define	MGL_ZFIX_1			0x10000000L
#define	MGL_ZFIX_HALF		0x08000000L
#define MGL_FIXTOZ(i)		((i) << 12)
#define	MGL_ZTOFIX(i)		((i) >> 12)
#define	MGL_TOZFIX(i)		((long)(i) << 28)
#define MGL_ZFIXTOINT(f)	((int)((f) >> 28))
#define	MGL_ZFIXROUND(f) 	((int)(((f) + MGL_ZFIX_HALF) >> 28))

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/*------------------------- Function Prototypes ---------------------------*/

/* Zbuffering support */

#ifdef	MGL_3D
int		MGLAPI MGL_getHardwareZBufferDepth(MGLDC *dc);
ibool    ASMAPI MGL_zBegin(MGLDC *dc,int zbits);
ibool	MGLAPI MGL_zShareZBuffer(MGLDC *dc,MGLDC *dcShared,int zbits);
#endif

/* 3D rasterization routines */

#ifdef	MGL_3D
void    MGLAPI MGL_zClearCoord(int left,int top,int right,int bottom,zfix32_t clearVal);
#endif

#ifdef	MGL_FIX3D
void	ASMAPI MGL_cLineCoord(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	ASMAPI MGL_rgbLineCoord(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	ASMAPI MGL_zLineCoord(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	ASMAPI MGL_czLineCoord(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	ASMAPI MGL_rgbzLineCoord(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t g2,fix32_t b2);

void	ASMAPI MGL_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);

void	ASMAPI MGL_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void	ASMAPI MGL_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	ASMAPI MGL_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
#endif

/* Routine to set the currently active shade table. In HiColor and TrueColor
 * video modes, you must set a valid shadeTable before you call any of the
 * color index shaded rendering routines (MGL_cTri() etc). These routines
 * will interpolate an index into the current shade table rather than
 * each of the RGB color channels, and the appropriate full RGB color is
 * extracted directly from the shade table. The shade table can be any size,
 * but the application must ensure that the indices passed in are within
 * the range of the current shade table.
 */

#ifdef	MGL_3D
void	MGLAPI MGL_setShadeTable(color_t *shadeTab);
#endif

#ifdef	__cplusplus
} /* End of "C" linkage for C++	*/
#endif	/* __cplusplus */

/*---------------------- Inline functions as Macros -----------------------*/

#define	MGL_cLine(p1,p2)		MGL_cLineCoord((p1).p.x,(p1).p.y,(p1).c,(p2).p.x,(p2).p.y,(p2).c)
#define	MGL_rgbLine(p1,p2)		MGL_rgbLineCoord((p1).p.x,(p1).p.y,(p1).c.r,(p1).c.g,(p1).c.b,(p2).p.x,(p2).p.y,(p2).c.r,(p2).c.g,(p2).c.b)
#define	MGL_zLine(p1,p2)		MGL_zLineCoord((p1).p.x,(p1).p.y,(p1).z,(p2).p.x,(p2).p.y,(p2).z)
#define	MGL_czLine(p1,p2)		MGL_czLineCoord((p1).p.x,(p1).p.y,(p1).z,(p1).c,(p2).p.x,(p2).p.y,(p2).z,(p2).c)
#define	MGL_rgbzLine(p1,p2)		MGL_rgbzLineCoord((p1).p.x,(p1).p.y,(p1).z,(p1).c.r,(p1).c.g,(p1).c.b,(p2).p.x,(p2).p.y,(p2).z,(p2).c.r,(p2).c.g,(p2).c.b)

#define	MGL_zClearPt(lt,rb,z)	MGL_zClearCoord((lt).x,(lt).y,	\
									(rb).x,(rb).y,z)
#define	MGL_zClear(r,z)			MGL_zClearCoord((r).left,(r).top,	\
									(r).right,(r).bottom,z)

#pragma pack()

#endif	/* __MGL3D_H */

