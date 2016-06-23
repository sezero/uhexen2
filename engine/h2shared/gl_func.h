/*
 * gl_func.h -- opengl function pointers
 * make sure NOT to protect this file against multiple inclusions!
 *
 * $Id$
 *
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
 * Copyright (C) 2005-2016  O.Sezer <sezero@users.sourceforge.net>
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

/* whether to dlsym gl function calls:
 * the define GL_DLSYM is decided in the Makefile */

#ifndef __GL_FUNC_EXTERN
#define __GL_FUNC_EXTERN extern
#endif

/* core gl functions
 */
#if defined(GL_DLSYM)

#ifndef GL_FUNCTION
#define GL_FUNCTION(ret, func, params) \
typedef ret (APIENTRY *func##_f) params; \
__GL_FUNC_EXTERN func##_f func##_fp;
#endif

GL_FUNCTION(void, glBindTexture, (GLenum,GLuint))
GL_FUNCTION(void, glDeleteTextures, (GLsizei,const GLuint *))
/* glGenTextures() is broken in 3dfx mini-drivers.  (I can
 * easily check and add a replacement. should I bother?^?) */
GL_FUNCTION(void, glGenTextures, (GLsizei,GLuint *))
GL_FUNCTION(void, glTexParameterf, (GLenum,GLenum,GLfloat))
GL_FUNCTION(void, glTexEnvf, (GLenum,GLenum,GLfloat))
GL_FUNCTION(void, glScalef, (GLfloat,GLfloat,GLfloat))
GL_FUNCTION(void, glTexImage2D, (GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const GLvoid*))
GL_FUNCTION(void, glTexSubImage2D, (GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const GLvoid *))

GL_FUNCTION(void, glBegin, (GLenum))
GL_FUNCTION(void, glEnd, (void))
GL_FUNCTION(void, glEnable, (GLenum))
GL_FUNCTION(void, glDisable, (GLenum))
GL_FUNCTION(GLboolean, glIsEnabled, (GLenum))

GL_FUNCTION(void, glFinish, (void))
GL_FUNCTION(void, glFlush, (void))
GL_FUNCTION(void, glClear, (GLbitfield))

GL_FUNCTION(void, glVertex2f, (GLfloat,GLfloat))
GL_FUNCTION(void, glVertex3f, (GLfloat,GLfloat,GLfloat))
GL_FUNCTION(void, glVertex3fv, (const GLfloat *))
GL_FUNCTION(void, glTexCoord2f, (GLfloat,GLfloat))
GL_FUNCTION(void, glTexCoord2fv, (const GLfloat *))
GL_FUNCTION(void, glColor4f, (GLfloat,GLfloat,GLfloat,GLfloat))
GL_FUNCTION(void, glColor4fv, (const GLfloat *))
GL_FUNCTION(void, glColor4ub, (GLubyte,GLubyte,GLubyte,GLubyte))
GL_FUNCTION(void, glColor4ubv, (const GLubyte *))
GL_FUNCTION(void, glColor3ubv, (const GLubyte *))
GL_FUNCTION(void, glColor3f, (GLfloat,GLfloat,GLfloat))
GL_FUNCTION(void, glClearColor, (GLclampf,GLclampf,GLclampf,GLclampf))

GL_FUNCTION(void, glAlphaFunc, (GLenum,GLclampf))
GL_FUNCTION(void, glBlendFunc, (GLenum,GLenum))
GL_FUNCTION(void, glShadeModel, (GLenum))
GL_FUNCTION(void, glPolygonMode, (GLenum,GLenum))
GL_FUNCTION(void, glDepthMask, (GLboolean))
GL_FUNCTION(void, glDepthRange, (GLclampd,GLclampd))
GL_FUNCTION(void, glDepthFunc, (GLenum))

#if defined(DRAW_PROGRESSBARS) /* D_ShowLoadingSize() */
GL_FUNCTION(void, glDrawBuffer, (GLenum))
#endif
GL_FUNCTION(void, glReadPixels, (GLint,GLint,GLsizei,GLsizei,GLenum,GLenum, GLvoid *))
GL_FUNCTION(void, glPixelStorei, (GLenum,GLint))
GL_FUNCTION(void, glHint, (GLenum,GLenum))
GL_FUNCTION(void, glCullFace, (GLenum))

GL_FUNCTION(void, glRotatef, (GLfloat,GLfloat,GLfloat,GLfloat))
GL_FUNCTION(void, glTranslatef, (GLfloat,GLfloat,GLfloat))

GL_FUNCTION(void, glOrtho, (GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble))
GL_FUNCTION(void, glFrustum, (GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble))
GL_FUNCTION(void, glViewport, (GLint,GLint,GLsizei,GLsizei))
GL_FUNCTION(void, glPushMatrix, (void))
GL_FUNCTION(void, glPopMatrix, (void))
GL_FUNCTION(void, glLoadIdentity, (void))
GL_FUNCTION(void, glMatrixMode, (GLenum))
GL_FUNCTION(void, glLoadMatrixf, (const GLfloat *))
/*
GL_FUNCTION(void, glPolygonOffset, (GLfloat,GLfloat))
*/

GL_FUNCTION(const GLubyte*, glGetString, (GLenum))
GL_FUNCTION(void, glGetFloatv, (GLenum,GLfloat *))
GL_FUNCTION(void, glGetIntegerv, (GLenum,GLint *))

GL_FUNCTION(void, glStencilFunc, (GLenum,GLint,GLuint))
GL_FUNCTION(void, glStencilOp, (GLenum,GLenum,GLenum))
GL_FUNCTION(void, glClearStencil, (GLint))

#else

#ifndef GL_FUNC_H
#define GL_FUNC_H

#define glBindTexture_fp	glBindTexture
#define glDeleteTextures_fp	glDeleteTextures
/* glGenTextures() is broken in 3dfx mini-drivers.  (I can
 * easily check and add a replacement. should I bother?^?) */
#define glGenTextures_fp	glGenTextures
#define glTexParameterf_fp	glTexParameterf
#define glTexEnvf_fp		glTexEnvf
#define glScalef_fp		glScalef
#define glTexImage2D_fp		glTexImage2D
#define glTexSubImage2D_fp	glTexSubImage2D

#define glBegin_fp		glBegin
#define glEnd_fp		glEnd
#define glEnable_fp		glEnable
#define glDisable_fp		glDisable
#define glIsEnabled_fp		glIsEnabled
#define glFinish_fp		glFinish
#define glFlush_fp		glFlush
#define glClear_fp		glClear

#define glVertex2f_fp		glVertex2f
#define glVertex3f_fp		glVertex3f
#define glVertex3fv_fp		glVertex3fv
#define glTexCoord2f_fp		glTexCoord2f
#define glTexCoord2fv_fp	glTexCoord2fv
#define glColor4f_fp		glColor4f
#define glColor4fv_fp		glColor4fv
#define glColor4ub_fp		glColor4ub
#define glColor4ubv_fp		glColor4ubv
#define glColor3ubv_fp		glColor3ubv
#define glColor3f_fp		glColor3f
#define glClearColor_fp		glClearColor

#define glAlphaFunc_fp		glAlphaFunc
#define glBlendFunc_fp		glBlendFunc
#define glShadeModel_fp		glShadeModel
#define glPolygonMode_fp	glPolygonMode
#define glDepthMask_fp		glDepthMask
#define glDepthRange_fp		glDepthRange
#define glDepthFunc_fp		glDepthFunc

#define glDrawBuffer_fp		glDrawBuffer
#define glReadPixels_fp		glReadPixels
#define glPixelStorei_fp	glPixelStorei
#define glHint_fp		glHint
#define glCullFace_fp		glCullFace

#define glRotatef_fp		glRotatef
#define glTranslatef_fp		glTranslatef

#define glOrtho_fp		glOrtho
#define glFrustum_fp		glFrustum
#define glViewport_fp		glViewport
#define glPushMatrix_fp		glPushMatrix
#define glPopMatrix_fp		glPopMatrix
#define glLoadIdentity_fp	glLoadIdentity
#define glMatrixMode_fp		glMatrixMode
#define glLoadMatrixf_fp	glLoadMatrixf
#define glPolygonOffset_fp	glPolygonOffset

#define glGetString_fp		glGetString
#define glGetFloatv_fp		glGetFloatv
#define glGetIntegerv_fp	glGetIntegerv

#define glStencilFunc_fp	glStencilFunc
#define glStencilOp_fp		glStencilOp
#define glClearStencil_fp	glClearStencil

#endif	/* GL_FUNC_H */

#endif	/* !defined(GL_DLSYM) */

#undef GL_FUNCTION


/* global gl functions link to at runtime
 */
#ifndef GL_FUNCTION_OPT
#define GL_FUNCTION_OPT(ret, func, params) \
typedef ret (APIENTRY *func##_f) params; \
__GL_FUNC_EXTERN func##_f func##_fp;
#endif

/* GL_ARB_multitexture */
GL_FUNCTION_OPT(void, glActiveTextureARB, (GLenum))
GL_FUNCTION_OPT(void, glMultiTexCoord2fARB, (GLenum,GLfloat,GLfloat))

#undef GL_FUNCTION_OPT


/* typedefs for functions linked to locally at runtime
 */
#ifndef GL_FUNC_TYPEDEFS
#define GL_FUNC_TYPEDEFS

/* this one doesn't seem to in amiga opengl libs :( */
typedef void (APIENTRY *glGetTexParameterfv_f) (GLenum,GLenum,GLfloat *);

/* GL_EXT_shared_texture_palette */
typedef void (APIENTRY *glColorTableEXT_f) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);

/* 3DFX_set_global_palette (NOTE: the PowerVR equivalent is
   POWERVR_set_global_palette / glSetGlobalPalettePOWERVR) */
typedef void (APIENTRY *gl3DfxSetPaletteEXT_f) (GLuint *);

#endif /* GL_FUNC_TYPEDEFS */
