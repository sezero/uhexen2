#define glBindTexture_fp(a,b)		glBindTexture((a),(b))
#define glDeleteTextures_fp(a,b)	glDeleteTextures((a),(b))
#define glTexParameterf_fp(a,b,c)	glTexParameterf((a),(b),(c))
#define glTexEnvf_fp(a,b,c)		glTexEnvf((a),(b),(c))
#define glScalef_fp(a,b,c)		glScalef((a),(b),(c))
#define glTexImage2D_fp(a,b,c,d,e,f,g,h,i)	glTexImage2D((a),(b),(c),(d),(e),(f),(g),(h),(i))
#define glTexSubImage2D_fp(a,b,c,d,e,f,g,h,i)	glTexSubImage2D((a),(b),(c),(d),(e),(f),(g),(h),(i))

#define glBegin_fp(a)			glBegin((a))
#define glEnd_fp()			glEnd()
#define glEnable_fp(a)			glEnable((a))
#define glDisable_fp(a)			glDisable((a))
#define glIsEnabled_fp(a)		glIsEnabled((a))
#define glFinish_fp()			glFinish()
#define glClear_fp(a)			glClear((a))

#define glVertex2f_fp(a,b)		glVertex2f((a),(b))
#define glVertex3f_fp(a,b,c)		glVertex3f((a),(b),(c))
#define glVertex3fv_fp(a)		glVertex3fv((a))
#define glTexCoord2f_fp(a,b)		glTexCoord2f((a),(b))
#define glTexCoord3f_fp(a,b,c)		glTexCoord3f((a),(b),(c))
#define glColor4f_fp(a,b,c,d)		glColor4f((a),(b),(c),(d))
#define glColor4fv_fp(a)		glColor4fv((a))
#define glColor4ub_fp(a,b,c,d)		glColor4ub((a),(b),(c),(d))
#define glColor4ubv_fp(a)		glColor4ubv((a))
#define glColor3ubv_fp(a)		glColor3ubv((a))
#define glColor3f_fp(a,b,c)		glColor3f((a),(b),(c))
#define glClearColor_fp(a,b,c,d)	glClearColor((a),(b),(c),(d))

#define glAlphaFunc_fp(a,b)		glAlphaFunc((a),(b))
#define glBlendFunc_fp(a,b)		glBlendFunc((a),(b))
#define glShadeModel_fp(a)		glShadeModel((a))
#define glPolygonMode_fp(a,b)		glPolygonMode((a),(b))
#define glDepthMask_fp(a)		glDepthMask((a))
#define glDepthRange_fp(a,b)		glDepthRange((a),(b))
#define glDepthFunc_fp(a)		glDepthFunc((a))

#define glDrawBuffer_fp(a)		glDrawBuffer((a))
#define glReadBuffer_fp(a)		glReadBuffer((a))
#define glReadPixels_fp(a,b,c,d,e,f,g)	glReadPixels((a),(b),(c),(d),(e),(f),(g))
#define glHint_fp(a,b)			glHint((a),(b))
#define glCullFace_fp(a)		glCullFace((a))

#define glRotatef_fp(a,b,c,d)		glRotatef((a),(b),(c),(d))
#define glTranslatef_fp(a,b,c)		glTranslatef((a),(b),(c))

#define glOrtho_fp(a,b,c,d,e,f)		glOrtho((a),(b),(c),(d),(e),(f))
#define glFrustum_fp(a,b,c,d,e,f)	glFrustum((a),(b),(c),(d),(e),(f))
#define glViewport_fp(a,b,c,d)		glViewport((a),(b),(c),(d))
#define glPushMatrix_fp()		glPushMatrix()
#define glPopMatrix_fp()		glPopMatrix()
#define glLoadIdentity_fp()		glLoadIdentity()
#define glMatrixMode_fp(a)		glMatrixMode((a))
#define glLoadMatrixf_fp(a)		glLoadMatrixf((a))

#define glGetString_fp(a)		glGetString((a))
#define glGetFloatv_fp(a,b)		glGetFloatv((a),(b))

#define glGetIntegerv_fp(a,b)		glGetIntegerv((a),(b))

#define glStencilFunc_fp(a,b,c)		glStencilFunc((a),(b),(c))
#define glStencilOp_fp(a,b,c)		glStencilOp((a),(b),(c))
#define glClearStencil_fp(a)		glClearStencil((a))

typedef void (APIENTRY *glActiveTextureARB_f)(GLenum);
typedef void (APIENTRY *glMultiTexCoord2fARB_f)(GLenum,GLfloat,GLfloat);
glActiveTextureARB_f glActiveTextureARB_fp;
glMultiTexCoord2fARB_f glMultiTexCoord2fARB_fp;

