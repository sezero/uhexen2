/*
 * $Id: gl.h,v 1.10 2000/03/15 17:58:19 hfrieden Exp $
 *
 * $Date: 2000/03/15 17:58:19 $
 * $Revision: 1.10 $
 *
 * (C) 1999 by Hyperion
 * All rights reserved
 *
 * This file is part of the MiniGL library project
 * See the file Licence.txt for more details
 *
 */

#ifndef GL_H_
#define GL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__PPC__) || defined (__VBCC__) || defined(__STORM__)
#define mykprintf kprintf
#endif

#include "mgl/config.h"
#include "mgl/log.h"

#ifndef NLOGGING
extern int MGLDebugLevel;
#define mglSetDebugLevel(level) \
	MGLDebugLevel = level
#endif


/*
	GL types
*/

typedef void            GLvoid;
//typedef unsigned char   GLboolean;
typedef char            GLbyte;
typedef unsigned char   GLubyte;
typedef short           GLshort;
typedef unsigned short  GLushort;
typedef int             GLint;
typedef unsigned int    GLuint;
typedef unsigned int    GLboolean;
typedef long            GLsizei;
typedef unsigned long   GLbitfield;
typedef float           GLfloat;
typedef double          GLdouble;
typedef float           GLclampf;
typedef double          GLclampd;
//typedef int             GLenum;
typedef unsigned int    GLenum;

/*
	GL enum
	Currently a dummy
*/

#define MAX_TEXUNIT 2

enum
{
	GL_BASE             = 0,
      GL_NO_ERROR         = 0,
	GL_ALPHA,
	GL_ALPHA8,
	GL_ALPHA_BITS,
	GL_ALPHA_SCALE,
	GL_ALPHA_TEST,
	GL_ALPHA_TEST_FUNC,
	GL_ALPHA_TEST_REF,
	GL_ALWAYS,
	GL_AUX_BUFFERS,
	GL_BACK,
	GL_BACK_LEFT,
	GL_BACK_RIGHT,
	GL_BLEND,
	GL_BLEND_DST,
	GL_BLEND_SRC,
	GL_BLUE,
	GL_BLUE_BITS,
	GL_BYTE,
	GL_C3F_V3F,
	GL_C4UB_V2F,
	GL_C4UB_V3F,
	GL_CCW,
	GL_COLOR_ARRAY,
	GL_COLOR_ARRAY_POINTER,
	GL_COLOR_ARRAY_SIZE,
	GL_COLOR_ARRAY_STRIDE,
	GL_COLOR_ARRAY_TYPE,
	GL_COLOR_CLEAR_VALUE,
	GL_COLOR_INDEX,
	GL_CULL_FACE,
	GL_CULL_FACE_MODE,
	GL_CURRENT_COLOR,
	GL_CURRENT_INDEX,
	GL_CURRENT_TEXTURE_COORDS,
	GL_CW,
	GL_DECAL,
	GL_DEPTH_BITS,
	GL_DEPTH_CLEAR_VALUE,
	GL_DEPTH_COMPONENT,
	GL_DEPTH_FUNC,
	GL_DEPTH_RANGE,
	GL_DEPTH_SCALE,
	GL_DEPTH_TEST,
	GL_DEPTH_WRITEMASK,
	GL_DITHER,
	GL_DONT_CARE,
	GL_DOUBLE,
	GL_DOUBLEBUFFER,
	GL_DRAW_BUFFER,
	GL_DST_ALPHA,
	GL_DST_COLOR,
	GL_EDGE_FLAG,
	GL_EDGE_FLAG_ARRAY,
	GL_EDGE_FLAG_ARRAY_POINTER,
	GL_EDGE_FLAG_ARRAY_STRIDE,
	GL_EQUAL,
	GL_EXTENSIONS,
	GL_FASTEST,
	GL_FLAT,
	GL_FLOAT,
	GL_FOG,
	GL_FOG_COLOR,
	GL_FOG_DENSITY,
	GL_FOG_END,
	GL_FOG_HINT,
	GL_FOG_INDEX,
	GL_FOG_MODE,
	GL_FOG_START,
	GL_FRONT_AND_BACK,
	GL_FRONT,
	GL_FRONT_FACE,
	GL_FRONT_LEFT,
	GL_FRONT_RIGHT,
	GL_GEQUAL,
	GL_GREATER,
	GL_GREEN,
	GL_GREEN_BITS,
	GL_INDEX_ARRAY,
	GL_INDEX_ARRAY_POINTER,
	GL_INDEX_ARRAY_STRIDE,
	GL_INDEX_ARRAY_TYPE,
	GL_INDEX_BITS,
	GL_INDEX_CLEAR_VALUE,
	GL_INDEX_MODE,
	GL_INT,
	GL_INTENSITY,
	GL_INTENSITY8,
	GL_INVALID_ENUM,
	GL_INVALID_OPERATION,
	GL_INVALID_VALUE,
	GL_LEFT,
	GL_LEQUAL,
	GL_LESS,
	GL_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_LINES,
	GL_LINE_LOOP,
	GL_LINE_STRIP,
	GL_LUMINANCE,
	GL_LUMINANCE8,
	GL_LUMINANCE8_ALPHA8,
	GL_LUMINANCE_ALPHA,
	GL_MATRIX_MODE,
	GL_MAX_TEXTURE_SIZE,
	GL_MAX_VIEWPORT_DIMS,
	GL_MODELVIEW,
	GL_MODELVIEW_MATRIX,
	GL_MODELVIEW_STACK_DEPTH,
	GL_MODULATE,
	GL_NEVER,
	GL_NEAREST,
	GL_NEAREST_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_NICEST,
	GL_NONE,
	GL_NOTEQUAL,
	GL_ONE,
	GL_ONE_MINUS_DST_ALPHA,
	GL_ONE_MINUS_DST_COLOR,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_ONE_MINUS_SRC_COLOR,
	GL_OUT_OF_MEMORY,
	GL_PACK_ALIGNMENT,
	GL_PACK_LSB_FIRST,
	GL_PACK_ROW_LENGTH,
	GL_PACK_SKIP_PIXELS,
	GL_PACK_SKIP_ROWS,
	GL_PACK_SWAP_BYTES,
	GL_PERSPECTIVE_CORRECTION_HINT,
	GL_POINTS,
	GL_POLYGON_MODE,
	GL_POLYGON_OFFSET,
	GL_POLYGON_OFFSET_FACTOR,
	GL_POLYGON_OFFSET_FILL,
	GL_POLYGON_OFFSET_LINE,
	GL_POLYGON_OFFSET_POINT,
	GL_POLYGON_OFFSET_UNITS,
	GL_POLYGON,
	GL_PROJECTION,
	GL_PROJECTION_MATRIX,
	GL_PROJECTION_STACK_DEPTH,
	GL_QUADS,
	GL_QUAD_STRIP,
	GL_READ_BUFFER,
	GL_RED,
	GL_RED_BITS,
	GL_RENDERER,
	GL_REPLACE,
	GL_REPEAT,
	GL_RGB,
	GL_RGB5,
	GL_RGB5_A1,
	GL_RGB8,
	GL_RGBA,
	GL_RGBA8,
	GL_RGBA_MODE,
	GL_RIGHT,
	GL_SCISSOR_BOX,
	GL_SCISSOR_TEST,
	GL_SHADE_MODEL,
	GL_SHORT,
	GL_SMOOTH,
	GL_SRC_ALPHA,
	GL_SRC_ALPHA_SATURATE,
	GL_SRC_COLOR,
	GL_STACK_OVERFLOW,
	GL_STACK_UNDERFLOW,
	GL_STEREO,
	GL_T2F_C3F_V3F,
	GL_T2F_C4UB_V3F,
	GL_T2F_V3F,
	GL_TABLE_TOO_LARGE,
	GL_TEXTURE_2D,
	GL_TEXTURE_2D_BINDING,
	GL_TEXTURE_COORD_ARRAY,
	GL_TEXTURE_COORD_ARRAY_SIZE,
	GL_TEXTURE_COORD_ARRAY_STRIDE,
	GL_TEXTURE_COORD_ARRAY_TYPE,
	GL_TEXTURE_DOOR_ARRAY_POINTER,
	GL_TEXTURE_ENV,
	GL_TEXTURE_ENV_COLOR,
	GL_TEXTURE_ENV_MODE,
	GL_TEXTURE_MAG_FILTER,
	GL_TEXTURE_MIN_FILTER,
	GL_TEXTURE_PRIORITY,
	GL_TEXTURE_WRAP_S,
	GL_TEXTURE_WRAP_T,
	GL_TEXTURE_GEN_S,
	GL_TEXTURE_GEN_T,
	GL_TRIANGLES,
	GL_TRIANGLE_FAN,
	GL_TRIANGLE_STRIP,
	GL_UNPACK_ALIGNMENT,
	GL_UNPACK_LSB_FIRST,
	GL_UNPACK_ROW_LENGTH,
	GL_UNPACK_SKIP_PIXELS,
	GL_UNPACK_SKIP_ROWS,
	GL_UNPACK_SWAP_BYTES,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT,
	GL_UNSIGNED_SHORT,
	GL_V2F,
	GL_V3F,
	GL_VENDOR,
	GL_VERSION,
	GL_VERTEX_ARRAY,
	GL_VERTEX_ARRAY_POINTER,
	GL_VERTEX_ARRAY_SIZE,
	GL_VERTEX_ARRAY_STRIDE,
	GL_VERTEX_ARRAY_TYPE,
	GL_VIEWPORT,
	GL_ZERO,
	GL_POINT_SMOOTH,
	GL_CLAMP,
	GL_EXP,
	GL_EXP2,
	GL_TEXTURE_GEN_MODE,
	GL_SPHERE_MAP,
	GL_T,
	GL_S,
	GL_FILL,
	GL_TEXTURE0_ARB,
	GL_TEXTURE1_ARB,
	GL_TEXTURE2_ARB,
	GL_TEXTURE3_ARB,
	GL_MAX_TEXTURE_UNITS_ARB,
	MGL_LOCK_AUTOMATIC,
	MGL_LOCK_MANUAL,
	MGL_LOCK_SMART,
	MGL_FLATFAN,
	MGL_FLATSTRIP,
	MGL_PERSPECTIVE_MAPPING,
	MGL_W_ONE_HINT,
	MGL_Z_OFFSET,
	GL_COLOR_TABLE,
	MGL_UBYTE_BGRA,
	MGL_UBYTE_ARGB,
	MGL_UNSIGNED_SHORT_5_6_5,
	MGL_UNSIGNED_SHORT_4_4_4_4,
	MGL_FIXPOINTTRANS_HINT,
	MGL_ARRAY_TRANSFORMATIONS,

	/**/
	GL_STENCIL_BITS,
	GL_STENCIL_TEST,
	GL_KEEP,
	GL_INCR,
	GL_STENCIL_BUFFER_BIT,
};

#define GL_COLOR_BUFFER_BIT     0x00000001
#define GL_DEPTH_BUFFER_BIT     0x00000002

#define GL_TRUE                 1
#define GL_FALSE                0

#define MGL_BUTTON_LEFT         0x00000001
#define MGL_BUTTON_RIGHT        0x00000002
#define MGL_BUTTON_MID          0x00000004

#define MGL_SM_BESTMODE         0xFFFFFFFF
#define MGL_SM_WINDOWMODE       0x00000000

typedef struct MGLColor_t
{
	GLfloat r,g,b,a;
} MGLColor;

//Surgeon: w-coord added (currently used for padding)
typedef struct MGLNormal_t
{
	GLfloat x,y,z,w;
} MGLNormal;

#include "mgl/vertexbuffer.h"
#include "mgl/context.h"
#include "mgl/clip.h"
#include "mgl/modes.h"

/*
	The current context is refered to as an extern variable, which
	is a pointer to the context.
*/

extern GLcontext mini_CurrentContext;

#ifndef GLNDEBUG
	#define GLASSERT(c) assert(c)
	#define dprintf(x) printf x
#else
	#define GLASSERT(c)
	#define dprintf(x)
#endif

#ifndef GL_NOERRORCHECK
	extern int kprintf(char *, ...);
	#define GLFlagError(context,c,err) while ((c)) {\
		context->CurrentError = err;\
	/*	kprintf("GLError: %ld at %s:%ld\n", (int)err, __FILE__,(int)__LINE__);*/\
		return;\
	}
#else
	#define GLFlagError(context,c,err) if ((c)) {\
	/*	context->CurrentError = err;*/\
		return;\
	}
#endif


/*
	Prototypes and appropriate defines
	These are derived from the OpenGL manpages
	Some defines are duplicated with EXT suffix, to be compatible.
	Additionally, some of these may not be needed (Maybe glBegin).
	There may also be a problem with floating point parameters for
	certain compilers. May be addressed in the macros.
*/

void        GLActiveTextureARB(GLcontext context, GLenum unit);
void        GLMultiTexCoord2fARB(GLcontext context, GLenum unit, GLfloat s, GLfloat t);
void        GLMultiTexCoord2fvARB(GLcontext context, GLenum unit, const GLfloat *v);

void MGLDrawMultitexBuffer (GLcontext context, GLenum BSrc, GLenum BDst, GLenum TexEnv);



void        GLAlphaFunc(GLcontext context, GLenum func, GLclampf ref);
//GLboolean   GLAreTexturesResident(GLcontext context, GLsizei n, const GLuint *textures, GLboolean *residences);
void        GLArrayElement(GLcontext context, GLint i);
void        GLBegin(GLcontext context, GLenum mode);
void        GLBindTexture(GLcontext context, GLenum target, GLuint texture);
void        GLBlendFunc(GLcontext context, GLenum sfactor, GLenum dfactor);
void        GLClear(GLcontext context, GLbitfield mask);
void        GLClearColor(GLcontext context, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void        GLClearDepth(GLcontext context, GLclampd depth);
void        GLColor3fv(GLcontext context, const GLfloat *v);
void        GLColor3ubv(GLcontext context, const GLubyte *v);
void        GLColor4f(GLcontext context, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void        GLColor4fv(GLcontext context, const GLfloat *v);
void        GLColor4ub(GLcontext context, GLubyte red, GLubyte green, GLubyte blue, GLubyte alhpa);
void        GLColor4ubv(GLcontext context, const GLubyte *v);
//void        GLColorPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void        GLColorTable(GLcontext context, GLenum target, GLenum internalformat, GLint width, GLenum format, GLenum type, const GLvoid *data);
void        GLCullFace(GLcontext context, GLenum mode);
void        GLDeleteTextures(GLcontext context, GLsizei n, const GLuint *textures);
void        GLDepthFunc(GLcontext context, GLenum func);
void        GLDepthMask(GLcontext context, GLboolean flag);
void        GLDepthRange(GLcontext context, GLclampd n, GLclampd f);
void        GLDisableClientState(GLcontext context, GLenum cap);
//void        GLDrawArrays(GLcontext context, GLenum mode, GLint first, GLsizei count);
void        GLDrawBuffer(GLcontext context, GLenum mode);
//void        GLDrawElements(GLcontext context, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
//void        GLEdgeFlag(GLcontext context, GLboolean flag);
//void        GLEdgeFlagPointer(GLcontext context, GLsizei stride, const GLboolean *flags);
//void        GLEdgeFlagv(GLcontext context, const GLboolean *flag);
void        GLEnableClientState(GLcontext context, GLenum cap);

void        GLInterleavedArrays(GLcontext context, GLenum format, GLsizei stride, const GLvoid *pointer);

void        GLEnd(GLcontext context);
void        GLFinish(GLcontext context);
void        GLFlush(GLcontext context);
void        GLFogf(GLcontext context, GLenum pname, GLfloat param);
void        GLFogfv(GLcontext context, GLenum pname, const GLfloat *param);
void        GLFrontFace(GLcontext context, GLenum mode);
void        GLFrustum(GLcontext context, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void        GLGenTextures(GLcontext context, GLsizei n, GLuint *textures);
void        GLGetBooleanv(GLcontext context, GLenum pname, GLboolean *params);
//void        GLGetDoublev(GLcontext context, GLenum pname, GLdouble *params);
GLenum      GLGetError(GLcontext context);
void        GLGetFloatv(GLcontext context, GLenum pname, GLfloat *params);
void        GLGetIntegerv(GLcontext context, GLenum pname, GLint *params);
void        GLGetPointerv(GLcontext context, GLenum pname, GLvoid* *params);
const GLubyte* GLGetString(GLcontext context, GLenum name);
void        GLHint(GLcontext context, GLenum target, GLenum mode);
//void        GLIndexi(GLcontext context, GLint c);
//void        GLIndexiv(GLcontext context, const GLint *c);
//void        GLIndexPointer(GLcontext context, GLenum type, GLsizei stride, const GLvoid *pointer);
//void        GLInterleavedArrays(GLcontext context, GLenum format, GLsizei stride, const GLvoid *pointer);
GLboolean   GLIsEnabled(GLcontext context, GLenum cap);
//GLboolean   GLIsTexture(GLcontext context, GLuint texture);
void        GLLoadIdentity(GLcontext context);
void        GLLoadMatrixd(GLcontext context, const GLdouble *m);
void        GLLoadMatrixf(GLcontext context, const GLfloat *m);
void        GLMatrixMode(GLcontext context, GLenum mode);
void        GLMultMatrixd(GLcontext context, const GLdouble *m);
void        GLMultMatrixf(GLcontext context, const GLfloat *m);
void        GLNormal3f(GLcontext context, GLfloat x, GLfloat y, GLfloat z);
void        GLOrtho(GLcontext context, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void        GLPixelStorei(GLcontext context, GLenum pname, GLint param);
void        GLPolygonMode(GLcontext context, GLenum face, GLenum mode);
//void        GLPolygonOffset(GLcontext context, GLfloat factor, GLfloat units);
//surgeon:
void	      GLPointSize(GLcontext context, GLfloat size);

void        GLPopMatrix(GLcontext context);
//void        GLPrioritizeTextures(GLcontext context, GLsizei n, const GLuint *textures, const GLclampf *priorities);
void        GLPushMatrix(GLcontext context);
//void        GLReadBuffer(GLcontext context, GLenum mode);
void        GLReadPixels(GLcontext context, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void        GLRotated(GLcontext context, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void        GLRotatef(GLcontext context, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

//simplified rotation-routine
//first 3 matches the values of corresponding matrix flags

#define GLROT_001		0x800
#define GLROT_010		0x1000
#define GLROT_100		0x2000

#define GLROT_011		0x4000
#define GLROT_101		0x8000
#define GLROT_110		0x10000
#define GLROT_111		0x20000

void        GLRotatefEXT(GLcontext context, GLfloat angle, const GLint xyz);

void        GLRotatefEXTs(GLcontext context, GLfloat sin_an, GLfloat cos_an, const GLint xyz);

void        GLScaled(GLcontext context, GLdouble x, GLdouble y, GLdouble z);
void        GLScalef(GLcontext context, GLfloat x, GLfloat y, GLfloat z);
void        GLScissor(GLcontext context, GLint x, GLint y, GLsizei width, GLsizei height);
void        GLShadeModel(GLcontext context, GLenum mode);
void        GLTexCoord2f(GLcontext context, GLfloat s, GLfloat t);
void        GLTexCoord2fv(GLcontext context, const GLfloat *v);
void        GLTexCoord4f(GLcontext context, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void        GLTexCoord4fv(GLcontext context, const GLfloat *v);
//void        GLTexCoordPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void        GLTexEnvi(GLcontext context, GLenum target, GLenum pname, GLint param);
void        GLTexGeni(GLcontext context, GLenum coord, GLenum mode, GLenum map);
void        GLTexImage2D(GLcontext context, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void        GLTexParameteri(GLcontext context, GLenum target, GLenum pname, GLint param);
void        GLTexSubImage2D(GLcontext context, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void        GLTranslated(GLcontext context, GLdouble x, GLdouble y, GLdouble z);
void        GLTranslatef(GLcontext context, GLfloat x, GLfloat y, GLfloat z);
void        GLVertex4f(GLcontext context, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void        GLVertex2f(GLcontext context, GLfloat x, GLfloat y);

void        GLVertex4fv(GLcontext context, const GLfloat *v);
void        GLVertex3fv(GLcontext context, const GLfloat *v);
void        GLVertex2fv(GLcontext context, const GLfloat *v);

//void        GLVertexPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void        GLViewport(GLcontext context, GLint x, GLint y, GLsizei width, GLsizei height);

void		GLLockArrays(GLcontext context, GLuint first, GLsizei count);
void		GLUnlockArrays(GLcontext context);

void        mglChooseGuardBand(GLboolean flag);
void        mglChooseNumberOfBuffers(int number);
void        mglChoosePixelDepth(int depth);
void        mglChooseTextureBufferSize(int size);
void        mglChooseVertexBufferSize(int size);
void        mglChooseMtexBufferSize(int size);
void        mglChooseWindowMode(GLboolean flag);
void *      MGLCreateContext(int offx, int offy, int w, int h);
void        MGLDeleteContext(GLcontext context);
void        MGLEnableSync(GLcontext context, GLboolean enable);
void        MGLExit(GLcontext context);
void *      MGLGetWindowHandle(GLcontext context);
void        MGLIdleFunc(GLcontext context, IdleFn i);
void        MGLKeyFunc(GLcontext context, KeyHandlerFn k);
GLboolean   MGLLockDisplay(GLcontext context);
void        MGLMainLoop(GLcontext context);
void	      MGLMinTriArea(GLcontext context, GLfloat area);
void        MGLMouseFunc(GLcontext context, MouseHandlerFn m);
void        mglProhibitAlphaFallback(GLboolean flag);
void        mglProhibitMipMapping(GLboolean flag);
void        mglProposeCloseDesktop(GLboolean closeme);
void        MGLResizeContext(GLcontext context, GLsizei width, GLsizei height);
void        MGLSetState(GLcontext context, GLenum cap, GLboolean state);
void        MGLSpecialFunc(GLcontext context, SpecialHandlerFn s);
void        MGLSwitchBuffer(GLcontext context, int bufnr);
void        MGLSwitchDisplay(GLcontext context);
void        MGLTexMemStat(GLcontext context, GLint *Current, GLint *Peak);
void        MGLUnlockDisplay(GLcontext context);
void        MGLWriteShotPPM(GLcontext context, const char *filename);
GLboolean        MGLInit(void);
void        MGLTerm(void);

#ifdef AUTOMATIC_LOCKING_ENABLE
void        MGLLockMode(GLcontext context, GLenum lockMode);
#endif

void        MGLPrintMatrix(GLcontext context, int mode);
void        MGLPrintMatrixStack(GLcontext context, int mode);

void        MGLSetZOffset(GLcontext context, GLfloat offset);

void        GLULookAt(GLfloat ex, GLfloat ey, GLfloat ez, GLfloat cx, GLfloat cy, GLfloat cz, GLfloat ux, GLfloat uy, GLfloat uz);
void        GLUPerspective(GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar);

GLint       mglGetSupportedScreenModes(MGLScreenModeCallback CallbackFn);
void *      MGLCreateContextFromID(GLint ID, GLint *w, GLint *h);
GLboolean   MGLLockBack(GLcontext context, MGLLockInfo *info);

void        GLEnableClientState(GLcontext context, GLenum state);
void        GLDisableClientState(GLcontext context, GLenum state);

void        GLTexCoordPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void        GLColorPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void        GLVertexPointer(GLcontext context, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

void        GLDrawElements(GLcontext context, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void        GLDrawArrays(GLcontext context, GLenum mode, GLint first, GLsizei count);

/*
** These macros define the OpenGL compatibility macros. If you don't want them,
** define USE_MGLAPI before including this file.
*/


#ifdef USE_MGLAPI
	#include "mgl/minigl.h"

#else

#define glActiveTextureARB(unit)                GLActiveTextureARB(mini_CurrentContext, unit)

#define glMultiTexCoord2fARB(a, u, v)           GLMultiTexCoord2fARB(mini_CurrentContext, a, u, v)

#define glMultiTexCoord2fvARB(a, v)             GLMultiTexCoord2fvARB(mini_CurrentContext, a,  v)

#define mglDrawMultitexBuffer(bsrc, bdst, env) MGLDrawMultitexBuffer(mini_CurrentContext, bsrc, bdst, env)

#define glAlphaFunc(func, ref) GLAlphaFunc(mini_CurrentContext, func, ref)

#define glAreTexturesResident(n, textures, residences) GLAreTexturesResident(mini_CurrentContext, n, textures, residences)

#define glArrayElement(i) GLArrayElement(mini_CurrentContext, i)

#define glArrayElementEXT(i) GLArrayElement(mini_CurrentContext, i)

#define glBegin(mode) GLBegin(mini_CurrentContext, mode)
#define glEnd() GLEnd(mini_CurrentContext)

#define glTexGeni(coord,mode,map) GLTexGeni(mini_CurrentContext, coord, mode, map)

#define glBindTexture(target, texture) GLBindTexture(mini_CurrentContext, target, texture)

#define glBlendFunc(sfactor, dfactor) GLBlendFunc(mini_CurrentContext, sfactor, dfactor)

#define glClear(mask) GLClear(mini_CurrentContext, mask)

#define glClearColor(red, green, blue, alpha) GLClearColor(mini_CurrentContext, red, green, blue, alpha)

#define glClearDepth(depth) GLClearDepth(mini_CurrentContext, depth)

#define glColorPointer(size, type, stride, pointer) GLColorPointer(mini_CurrentContext, size, type, stride, pointer)

#define glColorPointerEXT(size, type, stride, pointer) GLColorPointer(mini_CurrentContext, size, type, stride, pointer)

#define glColorTable(target, internalformat, width, format, type, data) GLColorTable(mini_CurrentContext, target, internalformat, width, format, type, data)

#define glCullFace(mode) GLCullFace(mini_CurrentContext, mode)

#define glDeleteTextures(n, textures) GLDeleteTextures(mini_CurrentContext, n, textures)

#define glDepthFunc(func) GLDepthFunc(mini_CurrentContext, func)

#define glDepthMask(flag) GLDepthMask(mini_CurrentContext, flag)

#define glEnable(cap) MGLSetState(mini_CurrentContext, cap, GL_TRUE)

#define glDisable(cap) MGLSetState(mini_CurrentContext, cap, GL_FALSE)

#define glDisableClientState(cap) GLDisableClientState(mini_CurrentContext, cap)

#define glEnableClientState(cap) GLEnableClientState(mini_CurrentContext, cap)

#define glInterleavedArrays(format, stride, pointer) GLInterleavedArrays(mini_CurrentContext, format, stride, pointer)

#define glDrawArrays(mode, first, count) GLDrawArrays(mini_CurrentContext, mode, first, count)

#define glDrawArraysEXT(mode, first, count) GLDrawArrays(mini_CurrentContext, mode, first, count)

#define glDrawBuffer(mode) GLDrawBuffer(mini_CurrentContext, mode)

#define glDrawElements(mode, count, type, indices) GLDrawElements(mini_CurrentContext, mode, count, type, indices)

#define glEdgeFlag(flag) GLEdgeFlag(mini_CurrentContext, flag)

#define glEdgeFlagv(flag) GLEdgeFlagv(mini_CurrentContext, flag)

#define glEdgeFlagPointer(stride, flags) GLEdgeFlagPointer(mini_CurrentContext, stride, flags)

#define glEdgeFlagPointerEXT(stride, flags) GLEdgeFlagPointer(mini_CurrentContext, stride, flags)

#define glFinish() GLFinish(mini_CurrentContext)

#define glFlush() GLFlush(mini_CurrentContext)

#define glFogf(pname, param) GLFogf(mini_CurrentContext, pname, param)

#define glFogfv(pname, param) GLFogfv(mini_CurrentContext, pname, param)

#define glFogi(pname, param) glFogf(pname, (GLfloat)param)
#define glFrontFace(mode) GLFrontFace(mini_CurrentContext, mode)

#define glFrustum(left, right, bottom, top, zNear, zFar) GLFrustum(mini_CurrentContext, left, right, bottom, top, zNear, zFar)

#define glGenTextures(n, textures) GLGenTextures(mini_CurrentContext, n, textures)

#define glGetError() GLGetError(mini_CurrentContext)

#define glGetBooleanv(pname, params) GLGetBooleanv(mini_CurrentContext, pname, params);

#define glGetDoublev(pname, params) GLGetDoublev(mini_CurrentContext, pname, params)

#define glGetFloatv(pname, params) GLGetFloatv(mini_CurrentContext, pname, params)

#define glGetIntegerv(pname, params) GLGetIntegerv(mini_CurrentContext, pname, params)

#define glGetPointerv(pname, params) GLGetPointerv(mini_CurrentContext, pname, params)

#define glGetPointervEXT(pname, params) GLGetPointerv(mini_CurrentContext, pname, params)

#define glGetString(name) GLGetString(mini_CurrentContext, name)

#define glHint(target, mode) GLHint(mini_CurrentContext, target, mode)

#define glIndexi(c) GLIndexi(mini_CurrentContext, c)

#define glIndexiv(c) GLIndexfv(mini_CurrentContext, c)

#define glIndexPointer(type, stride, pointer) GLIndexPointer(mini_CurrentContext, type, stride, pointer)

#define glIndexPointerEXT(type, stride, pointer) GLIndexPointer(mini_CurrentContext, type, stride, pointer)

#define glInterleavedArrays(format, stride, pointer) GLInterleavedArrays(mini_CurrentContext, format, stride, pointer)

#define glIsEnabled(cap) GLIsEnabled(mini_CurrentContext, cap)

#define glIsTexture(texture) GLIsTexture(mini_CurrentContext, texture)

#define glLoadIdentity() GLLoadIdentity(mini_CurrentContext)

#define glLoadMatrixf(m) GLLoadMatrixf(mini_CurrentContext, m)

#define glLoadMatrixd(m) GLLoadMatrixd(mini_CurrentContext, m)

#define glMatrixMode(mode) GLMatrixMode(mini_CurrentContext, mode)

#define glMultMatrixd(m) GLMultMatrixd(mini_CurrentContext, m)

#define glMultMatrixf(m) GLMultMatrixf(mini_CurrentContext, m)

#define glOrtho(left, right, bottom, top, zNear, zFar) GLOrtho(mini_CurrentContext, left, right, bottom, top, zNear, zFar)

#define glPixelStorei(pname, param) GLPixelStorei(mini_CurrentContext, pname, param)

#define glPixelStoref(pname, param) GLPixelStorei(mini_CurrentContext, pname, (int)(param))

#define glPolygonMode(face, mode) GLPolygonMode(mini_CurrentContext, face, mode)

#define glPolygonOffset(factor, units) GLPolygonOffset(mini_CurrentContext, factor, units)

#define glPointSize(s) GLPointSize(mini_CurrentContext, s)

#define glPushMatrix() GLPushMatrix(mini_CurrentContext)

#define glPopMatrix() GLPopMatrix(mini_CurrentContext)

#define glPrioritizeTextures(n, textures, pri) GLPrioritizeTextures(mini_CurrentContext, n, textures, pri)

#define glReadBuffer(mode) GLReadBuffer(mini_CurrentContext, mode)

#define glReadPixels(x, y, width, height, format, type, pixels) GLReadPixels(mini_CurrentContext, x, y, width, height, format, type, pixels)

#define glRotated(angle, x, y, z) GLRotated(mini_CurrentContext, angle, x, y, z)

#define glRotatef(angle, x, y, z) GLRotatef(mini_CurrentContext, (angle), (x), (y), (z))

#define glRotatefEXT(angle, xyz) GLRotatefEXT(mini_CurrentContext, angle, xyz)

#define glRotatefEXTs(sin_an, cos_an, xyz) GLRotatefEXTs(mini_CurrentContext, sin_an, cos_an, xyz)

#define glScaled(x, y, z) GLScaled(mini_CurrentContext, x, y, z)

#define glScalef(x, y, z) GLScaled(mini_CurrentContext, (GLdouble)(x), (GLdouble)(y), (GLdouble)(z))

#define glScissor(x, y, w, h) GLScissor(mini_CurrentContext, x, y, w, h)

#define glShadeModel(mode) GLShadeModel(mini_CurrentContext, mode)

#define glTexCoord2f(s, t) GLTexCoord2f(mini_CurrentContext, s, t)

#define glTexCoord2fv(v) GLTexCoord2fv(mini_CurrentContext, v)

#define glTexCoord4f(s, t, r, q) GLTexCoord4f(mini_CurrentContext, s, t, r, q)

#define glTexCoord4fv(v) GLTexCoord4fv(mini_CurrentContext, v)

#define glTexCoordPointer(size, type, stride, pointer) GLTexCoordPointer(mini_CurrentContext, size, type, stride, pointer)

#define glTexCoordPointerEXT(size, type, stride, pointer) GLTexCoordPointer(mini_CurrentContext, size, type, stride, pointer)

#define glTexImage2D(target, level, internal, width, height, border, format, type, pixels) GLTexImage2D(mini_CurrentContext, target, level, internal, width, height, border, format, type, pixels)

#define glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) GLTexSubImage2D(mini_CurrentContext, target, level, xoffset, yoffset, width, height, format, type, pixels)

#define glTranslated(x, y, z) GLTranslated(mini_CurrentContext, x, y, z)

#define glTranslatef(x, y, z) GLTranslatef(mini_CurrentContext, x, y, z)

#define glViewport(x, y, width, height) GLViewport(mini_CurrentContext, x, y, width, height)

#define glVertex4f(x,y,z,w) GLVertex4f(mini_CurrentContext, x,y,z,w)

#define glVertex3f(x,y,z)   GLVertex4f(mini_CurrentContext, x,y,z,    1.f)

#define glVertex2f(x,y)     GLVertex2f(mini_CurrentContext, x,y)

#define glVertex2i(x,y)     GLVertex2f(mini_CurrentContext, (GLfloat)x, (GLfloat)y)

#define glVertex4fv(v)      GLVertex4fv(mini_CurrentContext, v);
#define glVertex3fv(v)      GLVertex3fv(mini_CurrentContext, v);
#define glVertex2fv(v)     GLVertex2fv(mini_CurrentContext, v)


#define glVertexPointer(size, type, stride, pointer) GLVertexPointer(mini_CurrentContext, size, type, stride, pointer)

#define glVertexPointerEXT(size, type, stride, pointer) GLVertexPointer(mini_CurrentContext, size, type, stride, pointer)

#define glDepthRange(n,f) GLDepthRange(mini_CurrentContext, n,f)

#define glLockArrays(f,c) GLLockArrays(mini_CurrentContext, f,c)
#define glUnlockArrays() GLUnlockArrays(mini_CurrentContext)

/*
#define mglCreateContext(offx, offy, w, h) mini_CurrentContext = MGLCreateContext(offx, offy, w,h)
*/

//Olivier Fabre
#define mglCreateContext(offx, offy, w, h) (mini_CurrentContext = MGLCreateContext(offx, offy, w,h))

#define mglResizeContext(width, height) MGLResizeContext(mini_CurrentContext, width, height)
#define mglSwitchBuffer(bufnr) MGLSwitchBuffer(mini_CurrentContext, bufnr)
#define mglDeleteContext() MGLDeleteContext(mini_CurrentContext)
#define mglGetWindowHandle() MGLGetWindowHandle(mini_CurrentContext)
#define mglSwitchDisplay() MGLSwitchDisplay(mini_CurrentContext)
#define mglLockDisplay() MGLLockDisplay(mini_CurrentContext)
#define mglUnlockDisplay() MGLUnlockDisplay(mini_CurrentContext)


#define glColor4f(red, green, blue, alpha)  GLColor4f(mini_CurrentContext, red, green, blue, alpha)

#define glColor4fv(v)                       GLColor4fv(mini_CurrentContext, v)

#define glColor3f(red,green,blue)           GLColor4f(mini_CurrentContext, red, green, blue, 1.0)

#define glColor3fv(v)                       GLColor3fv(mini_CurrentContext,v)

#define glColor4ub(r,g,b,a) GLColor4ub(mini_CurrentContext, r,g,b,a)

#define glColor4ubv(v)      GLColor4ubv(mini_CurrentContext, v)

#define glColor3ub(r,g,b)   GLColor4ub(mini_CurrentContext, r,g,b,255)

#define glColor3ubv(v)      GLColor3ubv(mini_CurrentContext, v)

#define glNormal3f(x,y,z) GLNormal3f(mini_CurrentContext, x,y,z)

#define glcopTexEnvi(target, pname, param) GLTexEnvi(mini_CurrentContext, target, pname, param)

#define glTexEnvf(target, pname, param)  GLTexEnvi(mini_CurrentContext, target, pname, (GLint)param)

#define glTexEnvi(target, pname, param)  GLTexEnvi(mini_CurrentContext, target, pname, (GLint)param)

#define glTexEnviv(target, pname, param) GLTexEnvi(mini_CurrentContext, target, pname, *(param))

#define glTexEnvfv(target, pname, param) GLTexEnvi(mini_CurrentContext, target, pname, (GLint)(*(param)))

#define glTexParameteri(target, pname, param) GLTexParameteri(mini_CurrentContext, target, pname, param)

#define glTexParameterf(target, pname, param) glTexParameteri(target, pname, (GLint)param)

#define glTexParameteriv(target, pname, param) glTexParameteri(target, pname, *(param))

#define glTexParameterfv(target, pname, param) glTexParameteri(target, pname, (GLint)*(param))

#define mglEnableSync(enable) MGLEnableSync(mini_CurrentContext, enable)

#define mglWriteShotPPM(filename) MGLWriteShotPPM(mini_CurrentContext, filename)

#define mglTexMemStat(Current, Peak) MGLTexMemStat(mini_CurrentContext, Current, Peak)

#define mglSetZOffset(offset) MGLSetZOffset(mini_CurrentContext, offset)

#define mglCreateContextFromID(ID, w, h) MGLCreateContextFromID(ID, w, h)

#define mglLockBack(info) MGLLockBack(mini_CurrentContext, info)

//#define glEnableClientState(state) GLEnableClientState(mini_CurrentContext, state)
//#define glDisableClientState(state) GLDisableClientState(mini_CurrentContext, state)

#define glTexCoordPointer(size, type, stride, pointer) GLTexCoordPointer(mini_CurrentContext, size, type, stride, pointer)

#define glColorPointer(size, type, stride, pointer) GLColorPointer(mini_CurrentContext, size, type, stride, pointer)

#define glVertexPointer(size, type, stride, pointer) GLVertexPointer(mini_CurrentContext, size, type, stride, pointer)

#define glDrawElements(mode, count, type, indices) GLDrawElements(mini_CurrentContext, mode, count, type, indices)

#define glDrawArrays(mode, first, count) GLDrawArrays(mini_CurrentContext, mode, first, count)

#ifdef AUTOMATIC_LOCKING_ENABLE
	#define mglLockMode(lockMode) MGLLockMode(mini_CurrentContext, lockMode)
#else
	#define mglLockMode(lockMode) (NULL)
#endif

#define mglKeyFunc(k) MGLKeyFunc(mini_CurrentContext, k)
#define mglSpecialFunc(s) MGLSpecialFunc(mini_CurrentContext, s)
#define mglMouseFunc(m) MGLMouseFunc(mini_CurrentContext, m)
#define mglMinTriArea(a) MGLMinTriArea(mini_CurrentContext, a)
#define mglIdleFunc(i) MGLIdleFunc(mini_CurrentContext, i)

#define mglExit() MGLExit(mini_CurrentContext)
#define mglMainLoop() MGLMainLoop(mini_CurrentContext)

#ifndef GLNDEBUG
#define mglPrintMatrix(mode) MGLPrintMatrix(mini_CurrentContext, mode)
#define mglPrintMatrixStack(mode) MGLPrintMatrixStack(mini_CurrentContext, mode)
#endif

#define gluLookAt(ex, ey, ez, cx, cy, cz, ux, uy, uz) GLULookAt(ex, ey, ez, cx, cy, cz, ux, uy, uz);
#define gluPerspective(fovy, aspect, znear, zfar) GLUPerspective(fovy, aspect, znear, zfar);

#define glStencilFunc(f,r,m) do{(void)f;(void)r; (void)m; }while(0)
#define glStencilOp(f,zf,zp) do{(void)f;(void)zf;(void)zp;}while(0)
#define glClearStencil(s)    do{(void)s;                  }while(0)

#endif // USE_MGLAPI not defined

#ifdef __cplusplus
}
#endif


#endif
