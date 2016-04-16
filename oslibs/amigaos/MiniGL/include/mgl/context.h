/*
 * $Id: context.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
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

#ifndef __CONTEXT_H
#define __CONTEXT_H

#include "mgl/matrix.h"
#include "mgl/config.h"
#include "mgl/vertexbuffer.h"

#ifdef __VBCC__
#pragma amiga-align
#endif

#include <intuition/intuition.h>

#ifdef __PPC__
#include <devices/timer.h>
#endif


#ifdef __PPC__
typedef struct LockTimeHandle_s
{
	struct timeval StartTime;
} LockTimeHandle;

#else

typedef struct LockTimeHandle_s
{
	ULONG s_hi, s_lo;
	ULONG e_freq;
} LockTimeHandle;

#endif


#ifdef __VBCC__
#pragma default-align
#endif

typedef struct GLcontext_t * GLcontext;

struct GLcontext_t;

typedef void (*DrawFn)(struct GLcontext_t *);

typedef enum
{
	MGLKEY_F1, MGLKEY_F2, MGLKEY_F3, MGLKEY_F4, MGLKEY_F5, MGLKEY_F6, MGLKEY_F7, MGLKEY_F8,
	MGLKEY_F9, MGLKEY_F10,
	MGLKEY_CUP, MGLKEY_CDOWN, MGLKEY_CLEFT, MGLKEY_CRIGHT
} MGLspecial;

typedef enum
{
	GLCS_TEXTURE = 0x01,
	GLCS_COLOR   = 0x02,
	GLCS_VERTEX  = 0x04,
	GLCS_MASK    = 0x07,
} ClientStates;

typedef void (*KeyHandlerFn)(char key);
typedef void (*SpecialHandlerFn)(MGLspecial special_key);
typedef void (*MouseHandlerFn)(GLint x, GLint y, GLbitfield buttons);
typedef void (*IdleFn)(void);


/*
ArrayPointer and stride storage:

We define all pointers as UBYTE to avoid problems with badly aligned arrays.
*/


#define AP_FIXPOINT           1<<0
#define AP_COMPILED           1<<1
#define AP_CLIP_BYPASS        1<<2
#define AP_CHECK_OUTCODES     1<<3
#define AP_VOLATILE           1<<4 //current array range is "recycled" after drawing operation so it is okay to modify data in the current arrays.


typedef struct MGLAPointer_s
{
	GLubyte	*texcoords;	//application array
	GLint	texcoordstride;

	GLubyte	*w_buffer;      //w coord pointer
	GLint	w_off;		//w-offset rel. to texcoords

	GLubyte	*colors;	//application array
	GLint	colorstride;
	ULONG	colormode;	//w3d bitfield

	GLubyte	*verts;		//application array
	GLint	vertexstride;
	ULONG	vertexmode;	// fff / ddd / ffd

	//GL_EXT_compiled_vertex_arrays:
	GLuint	lockfirst;	//start of locked range
	GLsizei	locksize;
	GLuint	transformed;	//vertexbuffer offset

	GLbitfield	state;	//pipeline state (AP_##)

} MGLAPointer;


typedef struct GLarray_t
{
	/*
	** Vertex array
	*/

	GLint       size;           /* Number of elements per entry (mostly 3 or 4) */
	GLenum      type;           /* Data type of entries */
	GLsizei     stride;         /* How to reach the next array element */
	GLvoid*     pointer;        /* Pointer to the actual data */
} GLarray;

#if 0
typedef void (*DrawElementsFn)(GLcontext context, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (*DrawArraysFn)(GLcontext context, GLenum mode, GLint first, GLsizei count);
#endif

struct GLcontext_t
{
	/*
	** The primitive with which glBegin was called,
	** or GL_BASE if outside glBegin/glEnd
	*/

	GLenum      CurrentPrimitive;

	/*
	** Current error
	*/
	GLenum      CurrentError;

	/*
	** The ModelView/Projection matrix stack.
	** Note that the topmost (= current) matrix is not the
	** top of the stack, but rather one of the ModelView[]/Projection[] below.
	** This makes copying the matrices unnecessary...
	*/
	int         ModelViewStackPointer;
	Matrix      ModelViewStack[MODELVIEW_STACK_SIZE];

	int         ProjectionStackPointer;
	Matrix      ProjectionStack[PROJECTION_STACK_SIZE];

	/*
	** The current ModelView/Projeciton matrix.
	** The matrix multiplication routine will switch between those
	** two to avoid copying stuff.
	*/
	GLuint      ModelViewNr;
	Matrix      ModelView[2];

	#define     CurrentMV (&(context->ModelView[context->ModelViewNr]))
	#define     SwitchMV  context->ModelViewNr = !(context->ModelViewNr)

	GLuint      ProjectionNr;
	Matrix      Projection[2];

	#define     CurrentP (&(context->Projection[context->ProjectionNr]))
	#define     SwitchP  context->ProjectionNr = !(context->ProjectionNr)

	// The current matrix mode (GL_MODELVIEW or GL_PROJECTION)
	GLuint      CurrentMatrixMode;


	/*
	** flexible buffer reserved for vertexarrays
	*/

	GLfloat   * WBuffer;

	UWORD     * ElementIndex; //for glArrayElement and index conversion in glDrawElements

	/*
	** Vertex buffers
	** A call to glVertex*() will fill one entry of the vertex buffer
	** with the current data. glEnd() will go over this data and
	** draw the primitives based on this.
	*/

	MGLVertex * VertexBuffer;
	GLuint      VertexBufferPointer;        // Next free entry
	GLuint      VertexBufferSize;           // Size of the buffer

	/*
	** Surgeon: Normals are only used in auto-texcoord generation and are not part of the W3D_Vertex struct used for drawing.
	** Besides, Normals are more often specified for a surface rather than for single verts.
	** The solution is a normal-buffer with position in the buffer recorded in glVertex*() functions.
	** With propewr implementation I believe this should generate a lot of cache hits in v_GenTexCoords :)
	**
	** Note: CurrentNormal is now unused
	**
	**
	** A similar approach for colors is worth considering
	** (see vertexbuffer.h for a suggestion)
	** ubyte colors could be kept in that format until further down the pipeline.
	** If colors were stored in a buffer, the check for shademodel currently within glVertex could be delayed until after on/off-screen culling.
	** With smoothshading enabled, colorcopy and optional conversion from ubyte to float should probably be delayed until after the backface tests.
	**
	*/

	MGLNormal * NormalBuffer;
	GLuint	    NormalBufferPointer;

	/*
	** Current colors
	*/
	GLuint      ClearColor;
	W3D_Double  ClearDepth;
	W3D_Color   CurrentColor; //was MGLColor

 //Surgeon: minimize W3D_SetCurrentColor calls
	GLboolean   UpdateCurrentColor;

#if 0 //exchanged for normalbuffer approach - Surgeon
	MGLNormal   CurrentNormal;
#endif

	GLfloat     CurrentTexS, CurrentTexT, CurrentTexQ;
	GLboolean   CurrentTexQValid;
	/*
	** The flag indicates wether the combined matrix is valid or not.
	** If it indicates GL_TRUE, the CombinedMatrix field contains the
	** product of the ModelView and Projection matrix.
	*/
	GLboolean   CombinedValid;
	Matrix      CombinedMatrix;

	/*
	** Scale factors for the transformation of normalized coordinates
	** to window coordinates. The *x and *y values are set by glViewPort.
	** *z is set by glDepthRange, which also sets near and far.
	*/

//	GLdouble    sx,sy,sz;
//	GLdouble    ax,ay,az;
#if 0 //surgeon: double precision not needed
	GLdouble    sx,ax;
	GLdouble    sy,ay;
	GLdouble    sz,az;
#else
	GLfloat    sx,ax;
	GLfloat    sy,ay;
	GLfloat    sz,az;
#endif
	GLdouble    near,far;

	GLuint	    ClipFlags;	//surgeon: viewport flags used with guardband clipping
	GLboolean    GuardBand;	//surgeon

	// CullFace mode
	GLenum      CurrentCullFace;
	GLenum      CurrentFrontFace;

	// Sign extracted from above
	//  0 means no culling
	//  1 means back + ccw or front + cw
	// -1 means back + cw  or front + ccw

	GLint	    CurrentCullSign;	

	// Pixel states
	GLint       PackAlign;
	GLint       UnpackAlign;

	/*
	** GL Rendering States
	*/
	GLboolean   AlphaTest_State;
	GLboolean   Blend_State;
	GLboolean   Texture2D_State[MAX_TEXUNIT];
	GLboolean   TextureGenS_State;
	GLboolean   TextureGenT_State;
	GLboolean   Fog_State;
	GLboolean   Scissor_State;
	GLboolean   CullFace_State;
	GLboolean   DepthTest_State;
	GLboolean   PointSmooth_State;
	GLboolean   Dither_State;
	GLboolean   ZOffset_State;

	/*
	** 'Internal' states
	*/

	GLboolean   FogDirty;

	GLdouble    FogStart;
	GLdouble    FogEnd;

	/*
	** Drawing and clipping functions for the current primitive
	*/

	DrawFn      CurrentDraw;

	/*
	** Warp3D specific stuff
	*/

	W3D_Context *           w3dContext;
	struct Window *         w3dWindow;
	struct Screen *         w3dScreen;

	GLboolean		ArrayTexBound;

	GLint                   CurrentBinding;

	//Multitexture
	GLint                   VirtualBinding;
	GLuint			VirtualTexUnits; //Surgeon
	GLuint	    		ActiveTexture; //THF

	W3D_Texture **          w3dTexBuffer;
	GLubyte **              w3dTexMemory;
	GLint                   TexBufferSize;
	struct ScreenBuffer *   Buffers[3];
	struct BitMap *         w3dBitMap; // If in windowed mode
	struct RastPort *       w3dRastPort; // for windowed ClipBlit mode
	int                     BufNr;
	int                     NumBuffers;
	W3D_Scissor             scissor;
	GLboolean               w3dLocked;

#ifdef AUTOMATIC_LOCKING_ENABLE
	GLenum                  LockMode;
	LockTimeHandle          LockTime;
#endif
	GLboolean               DoSync;
	ULONG                   w3dChipID;
	ULONG                   w3dFormat;
	ULONG                   w3dAlphaFormat;
	GLint                   w3dBytesPerTexel;

	GLenum                  TexEnv[MAX_TEXUNIT];
	GLenum                  CurTexEnv;
	GLenum                  MinFilter;
	GLenum                  MagFilter;
	GLenum                  WrapS;
	GLenum                  WrapT;

	W3D_Fog                 w3dFog;
	ULONG                   w3dFogMode;
	GLfloat                 FogRange;
	GLfloat                 FogMult;
	GLenum                  ShadeModel;
	GLboolean               DepthMask;

	GLboolean               NoMipMapping;
	GLboolean               NoFallbackAlpha;

	KeyHandlerFn            KeyHandler;
	MouseHandlerFn          MouseHandler;
	SpecialHandlerFn        SpecialHandler;
	IdleFn                  Idle;
	GLboolean               Running;

	GLenum              SrcAlpha;
	GLenum              DstAlpha;
	GLboolean               AlphaFellBack;

	GLfloat                 InvRot[9];

	GLboolean       InvRotValid;
	GLboolean       WOne_Hint;
	GLboolean       FixpointTrans_Hint; //Surgeon

	GLfloat         ZOffset;

	void           *PaletteData;
	GLenum          PaletteFormat;
	GLint           PaletteSize;

/* Begin Joe Sera Sept. 23 2000 */
	/*
	** GL Current Modes and States for glGetIntegerv
	*/
	GLint CurPolygonMode ;      /* GL_POLYGON_MODE       */
	GLint CurShadeModel ;       /* GL_SHADE_MODEL        */
	GLint CurBlendSrc ;         /* GL_BLEND_SRC          */
	GLint CurBlendDst ;         /* GL_BLEND_DST          */
	GLint CurUnpackRowLength ;  /* GL_UNPACK_ROW_LENGTH  */
	GLint CurUnpackSkipPixels ; /* GL_UNPACK_SKIP_PIXELS */
	GLint CurUnpackSkipRows ;   /* GL_UNPACK_SKIP_ROWS   */

/* End Joe Sera Sept. 23 2000 */

/* Begin Joe Sera Oct. 21, 2000  */
	/*
	** GL Current Modes and States for glGetBooleanv
	*/

	GLboolean CurWriteMask ;    /* GL_DEPTH_WRITE_MASK   */
	GLboolean CurDepthTest ;    /* GL_DEPTH_TEST         */


/* End Joe Sera Oct. 21 2000 */

	/*
	** Client state
	*/

	GLbitfield      ClientState;        /* Current client state mask */

#if 0

	GLarray         ColorArray;         /* Current arrays */
	GLarray         TexCoordArray;
	GLarray         VertexArray;
#endif

	MGLAPointer	ArrayPointer;
	GLboolean	VertexArrayPipeline;

#if 0
	DrawElementsFn  DrawElementsHook;   /* Function pointers. These will depend on the state */
	DrawArraysFn    DrawArraysHook;
#endif

	GLfloat         MinTriArea;         /* Minimal area of triangles to be drawn (smaller ones will be rejected)*/

	GLfloat         CurrentPointSize;   /* diameter */

	GLubyte*        GeneratedTextures;  /* Array to keep track of generated textures */

};


/*
** The CMATRIX macro give the address of the currently
** active matrix, depending on the matrix mode.
** The OMATRIX macro gives the address of the secondary matrix
** The SMATRIX macro switches the active and backup matrix
*/
#define CMATRIX(context) context->CurrentMatrixMode == GL_MODELVIEW ?\
	(&(context->ModelView[context->ModelViewNr])):\
	(&(context->Projection[context->ProjectionNr]))

#define OMATRIX(context) context->CurrentMatrixMode == GL_MODELVIEW ?\
	(&(context->ModelView[!(context->ModelViewNr)])):\
	(&(context->Projection[!(context->ProjectionNr)]))

#define SMATRIX(context) if (context->CurrentMatrixMode == GL_MODELVIEW)\
	context->ModelViewNr = !(context->ModelViewNr);\
   else context->ProjectionNr = !(context->ProjectionNr)

#endif

