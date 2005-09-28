typedef void (APIENTRY *glBindTexture_f)(GLenum,GLuint);
typedef void (APIENTRY *glDeleteTextures_f)(GLsizei,const GLuint *);
typedef void (APIENTRY *glTexParameterf_f)(GLenum,GLenum,GLfloat);
typedef void (APIENTRY *glTexEnvf_f)(GLenum,GLenum,GLfloat);
typedef void (APIENTRY *glScalef_f)(GLfloat,GLfloat,GLfloat);
typedef void (APIENTRY *glTexImage2D_f)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const GLvoid*);
typedef void (APIENTRY *glTexSubImage2D_f)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const GLvoid *);

typedef void (APIENTRY *glBegin_f)(GLenum);
typedef void (APIENTRY *glEnd_f)();
typedef void (APIENTRY *glEnable_f)(GLenum);
typedef void (APIENTRY *glDisable_f)(GLenum);
typedef GLboolean (APIENTRY *glIsEnabled_f)(GLenum);
typedef void (APIENTRY *glFinish_f)(void);
typedef void (APIENTRY *glFlush_f)(void);
typedef void (APIENTRY *glClear_f)(GLbitfield);

typedef void (APIENTRY *glVertex2f_f)(GLfloat,GLfloat);
typedef void (APIENTRY *glVertex3f_f)(GLfloat,GLfloat,GLfloat);
typedef void (APIENTRY *glVertex3fv_f)(const GLfloat *);
typedef void (APIENTRY *glTexCoord2f_f)(GLfloat,GLfloat);
typedef void (APIENTRY *glTexCoord3f_f)(GLfloat,GLfloat,GLfloat);
typedef void (APIENTRY *glColor4f_f)(GLfloat,GLfloat,GLfloat,GLfloat);
typedef void (APIENTRY *glColor4fv_f)(const GLfloat *);
typedef void (APIENTRY *glColor4ub_f)(GLubyte,GLubyte,GLubyte,GLubyte);
typedef void (APIENTRY *glColor4ubv_f)(const GLubyte *);
typedef void (APIENTRY *glColor3ubv_f)(const GLubyte *);
typedef void (APIENTRY *glColor3f_f)(GLfloat,GLfloat,GLfloat);
typedef void (APIENTRY *glClearColor_f)(GLclampf,GLclampf,GLclampf,GLclampf);

typedef void (APIENTRY *glAlphaFunc_f)(GLenum,GLclampf);
typedef void (APIENTRY *glBlendFunc_f)(GLenum,GLenum);
typedef void (APIENTRY *glShadeModel_f)(GLenum);
typedef void (APIENTRY *glPolygonMode_f)(GLenum,GLenum);
typedef void (APIENTRY *glDepthMask_f)(GLboolean);
typedef void (APIENTRY *glDepthRange_f)(GLclampd,GLclampd);
typedef void (APIENTRY *glDepthFunc_f)(GLenum);

typedef void (APIENTRY *glDrawBuffer_f)(GLenum);
typedef void (APIENTRY *glReadBuffer_f)(GLenum);
typedef void (APIENTRY *glReadPixels_f)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum, GLvoid *);
typedef void (APIENTRY *glHint_f)(GLenum,GLenum);
typedef void (APIENTRY *glCullFace_f)(GLenum);

typedef void (APIENTRY *glRotatef_f)(GLfloat,GLfloat,GLfloat,GLfloat);
typedef void (APIENTRY *glTranslatef_f)(GLfloat,GLfloat,GLfloat);

typedef void (APIENTRY *glOrtho_f)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
typedef void (APIENTRY *glFrustum_f)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
typedef void (APIENTRY *glViewport_f)(GLint,GLint,GLsizei,GLsizei);
typedef void (APIENTRY *glPushMatrix_f)(void);
typedef void (APIENTRY *glPopMatrix_f)(void);
typedef void (APIENTRY *glLoadIdentity_f)(void);
typedef void (APIENTRY *glMatrixMode_f)(GLenum);
typedef void (APIENTRY *glLoadMatrixf_f)(const GLfloat *);

typedef const GLubyte* (APIENTRY *glGetString_f)(GLenum);
typedef void (APIENTRY *glGetFloatv_f)(GLenum,GLfloat *);
typedef void (APIENTRY *glGetIntegerv_f)(GLenum,GLint *);

typedef void (APIENTRY *glActiveTextureARB_f)(GLenum);
typedef void (APIENTRY *glMultiTexCoord2fARB_f)(GLenum,GLfloat,GLfloat);

typedef void (APIENTRY *glStencilFunc_f)(GLenum,GLint,GLuint);
typedef void (APIENTRY *glStencilOp_f)(GLenum,GLenum,GLenum);
typedef void (APIENTRY *glClearStencil_f)(GLint);


glBegin_f glBegin_fp;
glEnd_f glEnd_fp;
glEnable_f glEnable_fp;
glDisable_f glDisable_fp;
glIsEnabled_f glIsEnabled_fp;
glFinish_f glFinish_fp;
glFlush_f glFlush_fp;
glClear_f glClear_fp;

glOrtho_f glOrtho_fp;
glFrustum_f glFrustum_fp;
glViewport_f glViewport_fp;
glPushMatrix_f glPushMatrix_fp;
glPopMatrix_f glPopMatrix_fp;
glLoadIdentity_f glLoadIdentity_fp;
glMatrixMode_f glMatrixMode_fp;
glLoadMatrixf_f glLoadMatrixf_fp;

glVertex2f_f glVertex2f_fp;
glVertex3f_f glVertex3f_fp;
glVertex3fv_f glVertex3fv_fp;
glTexCoord2f_f glTexCoord2f_fp;
glTexCoord3f_f glTexCoord3f_fp;
glColor4f_f glColor4f_fp;
glColor4fv_f glColor4fv_fp;
glColor4ub_f glColor4ub_fp;
glColor4ubv_f glColor4ubv_fp;
glColor3f_f glColor3f_fp;
glColor3ubv_f glColor3ubv_fp;
glClearColor_f glClearColor_fp;

glRotatef_f glRotatef_fp;
glTranslatef_f glTranslatef_fp;

glBindTexture_f glBindTexture_fp;
glDeleteTextures_f glDeleteTextures_fp;
glTexParameterf_f glTexParameterf_fp;
glTexEnvf_f glTexEnvf_fp;
glScalef_f glScalef_fp;
glTexImage2D_f glTexImage2D_fp;
glTexSubImage2D_f glTexSubImage2D_fp;

glAlphaFunc_f glAlphaFunc_fp;
glBlendFunc_f glBlendFunc_fp;
glShadeModel_f glShadeModel_fp;
glPolygonMode_f glPolygonMode_fp;
glDepthMask_f glDepthMask_fp;
glDepthRange_f glDepthRange_fp;
glDepthFunc_f glDepthFunc_fp;

glDrawBuffer_f glDrawBuffer_fp;
glReadBuffer_f glReadBuffer_fp;
glReadPixels_f glReadPixels_fp;
glHint_f glHint_fp;
glCullFace_f glCullFace_fp;

glGetString_f glGetString_fp;
glGetFloatv_f glGetFloatv_fp;
glGetIntegerv_f glGetIntegerv_fp;

glActiveTextureARB_f glActiveTextureARB_fp;
glMultiTexCoord2fARB_f glMultiTexCoord2fARB_fp;

glStencilFunc_f glStencilFunc_fp;
glStencilOp_f glStencilOp_fp;
glClearStencil_f glClearStencil_fp;

