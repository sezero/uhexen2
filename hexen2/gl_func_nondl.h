#define glBindTexture_fp	glBindTexture
#define glDeleteTextures_fp	glDeleteTextures
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
#define glClear_fp		glClear

#define glVertex2f_fp		glVertex2f
#define glVertex3f_fp		glVertex3f
#define glVertex3fv_fp		glVertex3fv
#define glTexCoord2f_fp		glTexCoord2f
#define glTexCoord3f_fp		glTexCoord3f
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
#define glReadBuffer_fp		glReadBuffer
#define glReadPixels_fp		glReadPixels
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

#define glGetString_fp		glGetString
#define glGetFloatv_fp		glGetFloatv
#define glGetIntegerv_fp	glGetIntegerv

#define glStencilFunc_fp	glStencilFunc
#define glStencilOp_fp		glStencilOp
#define glClearStencil_fp	glClearStencil

typedef void (APIENTRY *glActiveTextureARB_f)(GLenum);
typedef void (APIENTRY *glMultiTexCoord2fARB_f)(GLenum,GLfloat,GLfloat);
glActiveTextureARB_f glActiveTextureARB_fp;
glMultiTexCoord2fARB_f glMultiTexCoord2fARB_fp;

