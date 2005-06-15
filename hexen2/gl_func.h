typedef void (*glBindTexture_f)(GLenum,GLuint);
typedef void (*glDeleteTextures_f)(GLsizei,const GLuint *);
typedef void (*glTexParameterf_f)(GLenum,GLenum,GLfloat);
typedef void (*glTexEnvf_f)(GLenum,GLenum,GLfloat);
typedef void (*glScalef_f)(GLfloat,GLfloat,GLfloat);
typedef void (*glTexImage2D_f)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const GLvoid*);
typedef void (*glTexSubImage2D_f)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const GLvoid *);

typedef void (*glBegin_f)(GLenum);
typedef void (*glEnd_f)();
typedef void (*glEnable_f)(GLenum);
typedef void (*glDisable_f)(GLenum);
typedef GLboolean (*glIsEnabled_f)(GLenum);
typedef void (*glFinish_f)(void);
typedef void (*glClear_f)(GLbitfield);

typedef void (*glVertex2f_f)(GLfloat,GLfloat);
typedef void (*glVertex3f_f)(GLfloat,GLfloat,GLfloat);
typedef void (*glVertex3fv_f)(const GLfloat *);
typedef void (*glTexCoord2f_f)(GLfloat,GLfloat);
typedef void (*glTexCoord3f_f)(GLfloat,GLfloat,GLfloat);
typedef void (*glColor4f_f)(GLfloat,GLfloat,GLfloat,GLfloat);
typedef void (*glColor4fv_f)(const GLfloat *);
typedef void (*glColor4ub_f)(GLubyte,GLubyte,GLubyte,GLubyte);
typedef void (*glColor4ubv_f)(const GLubyte *);
typedef void (*glColor3ubv_f)(const GLubyte *);
typedef void (*glColor3f_f)(GLfloat,GLfloat,GLfloat);
typedef void (*glClearColor_f)(GLclampf,GLclampf,GLclampf,GLclampf);

typedef void (*glAlphaFunc_f)(GLenum,GLclampf);
typedef void (*glBlendFunc_f)(GLenum,GLenum);
typedef void (*glShadeModel_f)(GLenum);
typedef void (*glPolygonMode_f)(GLenum,GLenum);
typedef void (*glDepthMask_f)(GLboolean);
typedef void (*glDepthRange_f)(GLclampd,GLclampd);
typedef void (*glDepthFunc_f)(GLenum);

typedef void (*glDrawBuffer_f)(GLenum);
typedef void (*glReadBuffer_f)(GLenum);
typedef void (*glReadPixels_f)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum, GLvoid *);
typedef void (*glHint_f)(GLenum,GLenum);
typedef void (*glCullFace_f)(GLenum);

typedef void (*glRotatef_f)(GLfloat,GLfloat,GLfloat,GLfloat);
typedef void (*glTranslatef_f)(GLfloat,GLfloat,GLfloat);

typedef void (*glOrtho_f)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
typedef void (*glFrustum_f)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
typedef void (*glViewport_f)(GLint,GLint,GLsizei,GLsizei);
typedef void (*glPushMatrix_f)(void);
typedef void (*glPopMatrix_f)(void);
typedef void (*glLoadIdentity_f)(void);
typedef void (*glMatrixMode_f)(GLenum);
typedef void (*glLoadMatrixf_f)(const GLfloat *);

typedef const GLubyte* (*glGetString_f)(GLenum);
typedef void (*glGetFloatv_f)(GLenum,GLfloat *);
typedef void (*glGetIntegerv_f)(GLenum,GLint *);

typedef void (*glActiveTextureARB_f)(GLenum);
typedef void (*glMultiTexCoord2fARB_f)(GLenum,GLfloat,GLfloat);


glBegin_f glBegin_fp;
glEnd_f glEnd_fp;
glEnable_f glEnable_fp;
glDisable_f glDisable_fp;
glIsEnabled_f glIsEnabled_fp;
glFinish_f glFinish_fp;
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

