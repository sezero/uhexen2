void APIENTRY  __glClearIndex(__reg("fp0") GLfloat c ,__reg("a6") void *)="\tjsr\t-180(a6)";
#define glClearIndex(x1) __glClearIndex((x1),glBase)
void APIENTRY  __glClearColor(__reg("fp0") GLclampf red,__reg("fp1") GLclampf green,__reg("fp2") GLclampf blue,__reg("fp3") GLclampf alpha ,__reg("a6") void *)="\tjsr\t-186(a6)";
#define glClearColor(x1,x2,x3,x4) __glClearColor((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glClear(__reg("d0") GLbitfield mask ,__reg("a6") void *)="\tjsr\t-192(a6)";
#define glClear(x1) __glClear((x1),glBase)
void APIENTRY  __glIndexMask(__reg("d0") GLuint mask ,__reg("a6") void *)="\tjsr\t-198(a6)";
#define glIndexMask(x1) __glIndexMask((x1),glBase)
void APIENTRY  __glColorMask(__reg("d0") GLboolean red,__reg("d1") GLboolean green,__reg("d2") GLboolean blue,__reg("d3") GLboolean alpha ,__reg("a6") void *)="\tjsr\t-204(a6)";
#define glColorMask(x1,x2,x3,x4) __glColorMask((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glAlphaFunc(__reg("d0") GLenum func,__reg("fp0") GLclampf ref ,__reg("a6") void *)="\tjsr\t-210(a6)";
#define glAlphaFunc(x1,x2) __glAlphaFunc((x1),(x2),glBase)
void APIENTRY  __glBlendFunc(__reg("d0") GLenum sfactor,__reg("d1") GLenum dfactor ,__reg("a6") void *)="\tjsr\t-216(a6)";
#define glBlendFunc(x1,x2) __glBlendFunc((x1),(x2),glBase)
void APIENTRY  __glLogicOp(__reg("d0") GLenum opcode ,__reg("a6") void *)="\tjsr\t-222(a6)";
#define glLogicOp(x1) __glLogicOp((x1),glBase)
void APIENTRY  __glCullFace(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-228(a6)";
#define glCullFace(x1) __glCullFace((x1),glBase)
void APIENTRY  __glFrontFace(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-234(a6)";
#define glFrontFace(x1) __glFrontFace((x1),glBase)
void APIENTRY  __glPointSize(__reg("fp0") GLfloat size ,__reg("a6") void *)="\tjsr\t-240(a6)";
#define glPointSize(x1) __glPointSize((x1),glBase)
void APIENTRY  __glLineWidth(__reg("fp0") GLfloat width ,__reg("a6") void *)="\tjsr\t-246(a6)";
#define glLineWidth(x1) __glLineWidth((x1),glBase)
void APIENTRY  __glLineStipple(__reg("d0") GLint factor,__reg("d1") GLushort pattern ,__reg("a6") void *)="\tjsr\t-252(a6)";
#define glLineStipple(x1,x2) __glLineStipple((x1),(x2),glBase)
void APIENTRY  __glPolygonMode(__reg("d0") GLenum face,__reg("d1") GLenum mode ,__reg("a6") void *)="\tjsr\t-258(a6)";
#define glPolygonMode(x1,x2) __glPolygonMode((x1),(x2),glBase)
void APIENTRY  __glPolygonOffset(__reg("fp0") GLfloat factor,__reg("fp1") GLfloat units ,__reg("a6") void *)="\tjsr\t-264(a6)";
#define glPolygonOffset(x1,x2) __glPolygonOffset((x1),(x2),glBase)
void APIENTRY  __glPolygonStipple(__reg("a0") const GLubyte *mask ,__reg("a6") void *)="\tjsr\t-270(a6)";
#define glPolygonStipple(x1) __glPolygonStipple((x1),glBase)
void APIENTRY  __glGetPolygonStipple(__reg("a0") GLubyte *mask ,__reg("a6") void *)="\tjsr\t-276(a6)";
#define glGetPolygonStipple(x1) __glGetPolygonStipple((x1),glBase)
void APIENTRY  __glEdgeFlag(__reg("d0") GLboolean flag ,__reg("a6") void *)="\tjsr\t-282(a6)";
#define glEdgeFlag(x1) __glEdgeFlag((x1),glBase)
void APIENTRY  __glEdgeFlagv(__reg("a0") const GLboolean *flag ,__reg("a6") void *)="\tjsr\t-288(a6)";
#define glEdgeFlagv(x1) __glEdgeFlagv((x1),glBase)
void APIENTRY  __glScissor(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLsizei width,__reg("d3") GLsizei height,__reg("a6") void *)="\tjsr\t-294(a6)";
#define glScissor(x1,x2,x3,x4) __glScissor((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glClipPlane(__reg("d0") GLenum plane,__reg("a0") const GLdouble *equation ,__reg("a6") void *)="\tjsr\t-300(a6)";
#define glClipPlane(x1,x2) __glClipPlane((x1),(x2),glBase)
void APIENTRY  __glGetClipPlane(__reg("d0") GLenum plane,__reg("a0") GLdouble *equation ,__reg("a6") void *)="\tjsr\t-306(a6)";
#define glGetClipPlane(x1,x2) __glGetClipPlane((x1),(x2),glBase)
void APIENTRY  __glDrawBuffer(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-312(a6)";
#define glDrawBuffer(x1) __glDrawBuffer((x1),glBase)
void APIENTRY  __glReadBuffer(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-318(a6)";
#define glReadBuffer(x1) __glReadBuffer((x1),glBase)
void APIENTRY  __glEnable(__reg("d0") GLenum cap ,__reg("a6") void *)="\tjsr\t-324(a6)";
#define glEnable(x1) __glEnable((x1),glBase)
void APIENTRY  __glDisable(__reg("d0") GLenum cap ,__reg("a6") void *)="\tjsr\t-330(a6)";
#define glDisable(x1) __glDisable((x1),glBase)
GLboolean APIENTRY  __glIsEnabled(__reg("d0") GLenum cap ,__reg("a6") void *)="\tjsr\t-336(a6)";
#define glIsEnabled(x1) __glIsEnabled((x1),glBase)
void APIENTRY  __glEnableClientState(__reg("d0") GLenum cap ,__reg("a6") void *)="\tjsr\t-342(a6)";
#define glEnableClientState(x1) __glEnableClientState((x1),glBase)
void APIENTRY  __glDisableClientState(__reg("d0") GLenum cap ,__reg("a6") void *)="\tjsr\t-348(a6)";
#define glDisableClientState(x1) __glDisableClientState((x1),glBase)
void APIENTRY  __glGetBooleanv(__reg("d0") GLenum pname,__reg("a0") GLboolean *params ,__reg("a6") void *)="\tjsr\t-354(a6)";
#define glGetBooleanv(x1,x2) __glGetBooleanv((x1),(x2),glBase)
void APIENTRY  __glGetDoublev(__reg("d0") GLenum pname,__reg("a0") GLdouble *params ,__reg("a6") void *)="\tjsr\t-360(a6)";
#define glGetDoublev(x1,x2) __glGetDoublev((x1),(x2),glBase)
void APIENTRY  __glGetFloatv(__reg("d0") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-366(a6)";
#define glGetFloatv(x1,x2) __glGetFloatv((x1),(x2),glBase)
void APIENTRY  __glGetIntegerv(__reg("d0") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-372(a6)";
#define glGetIntegerv(x1,x2) __glGetIntegerv((x1),(x2),glBase)
void APIENTRY  __glPushAttrib(__reg("d0") GLbitfield mask ,__reg("a6") void *)="\tjsr\t-378(a6)";
#define glPushAttrib(x1) __glPushAttrib((x1),glBase)
void APIENTRY  __glPopAttrib(__reg("a6") void *)="\tjsr\t-384(a6)";
#define glPopAttrib() __glPopAttrib(glBase)
void APIENTRY  __glPushClientAttrib(__reg("d0") GLbitfield mask ,__reg("a6") void *)="\tjsr\t-390(a6)";
#define glPushClientAttrib(x1) __glPushClientAttrib((x1),glBase)
void APIENTRY  __glPopClientAttrib(__reg("a6") void *)="\tjsr\t-396(a6)";
#define glPopClientAttrib() __glPopClientAttrib(glBase)
GLint APIENTRY  __glRenderMode(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-402(a6)";
#define glRenderMode(x1) __glRenderMode((x1),glBase)
GLenum APIENTRY  __glGetError(__reg("a6") void *)="\tjsr\t-408(a6)";
#define glGetError() __glGetError(glBase)
const GLubyte* APIENTRY  __glGetString(__reg("d0") GLenum name ,__reg("a6") void *)="\tjsr\t-414(a6)";
#define glGetString(x1) __glGetString((x1),glBase)
void APIENTRY  __glFinish(__reg("a6") void *)="\tjsr\t-420(a6)";
#define glFinish() __glFinish(glBase)
void APIENTRY  __glFlush(__reg("a6") void *)="\tjsr\t-426(a6)";
#define glFlush() __glFlush(glBase)
void APIENTRY  __glHint(__reg("d0") GLenum target,__reg("d1") GLenum mode ,__reg("a6") void *)="\tjsr\t-432(a6)";
#define glHint(x1,x2) __glHint((x1),(x2),glBase)
void APIENTRY  __glClearDepth(__reg("fp0") GLclampd depth ,__reg("a6") void *)="\tjsr\t-438(a6)";
#define glClearDepth(x1) __glClearDepth((x1),glBase)
void APIENTRY  __glDepthFunc(__reg("d0") GLenum func ,__reg("a6") void *)="\tjsr\t-444(a6)";
#define glDepthFunc(x1) __glDepthFunc((x1),glBase)
void APIENTRY  __glDepthMask(__reg("d0") GLboolean flag ,__reg("a6") void *)="\tjsr\t-450(a6)";
#define glDepthMask(x1) __glDepthMask((x1),glBase)
void APIENTRY  __glDepthRange(__reg("fp0") GLclampd near_val,__reg("fp1") GLclampd far_val ,__reg("a6") void *)="\tjsr\t-456(a6)";
#define glDepthRange(x1,x2) __glDepthRange((x1),(x2),glBase)
void APIENTRY  __glClearAccum(__reg("fp0") GLfloat red,__reg("fp1") GLfloat green,__reg("fp2") GLfloat blue,__reg("fp3") GLfloat alpha ,__reg("a6") void *)="\tjsr\t-462(a6)";
#define glClearAccum(x1,x2,x3,x4) __glClearAccum((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glAccum(__reg("d0") GLenum op,__reg("fp0") GLfloat value ,__reg("a6") void *)="\tjsr\t-468(a6)";
#define glAccum(x1,x2) __glAccum((x1),(x2),glBase)
void APIENTRY  __glMatrixMode(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-474(a6)";
#define glMatrixMode(x1) __glMatrixMode((x1),glBase)
void APIENTRY  __glOrtho(__reg("fp0") GLdouble left,__reg("fp1") GLdouble right,__reg("fp2") GLdouble bottom,__reg("fp3") GLdouble top,__reg("fp4") GLdouble near_val,__reg("fp5") GLdouble far_val ,__reg("a6") void *)="\tjsr\t-480(a6)";
#define glOrtho(x1,x2,x3,x4,x5,x6) __glOrtho((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glFrustum(__reg("fp0") GLdouble left,__reg("fp1") GLdouble right,__reg("fp2") GLdouble bottom,__reg("fp3") GLdouble top,__reg("fp4") GLdouble near_val,__reg("fp5") GLdouble far_val ,__reg("a6") void *)="\tjsr\t-486(a6)";
#define glFrustum(x1,x2,x3,x4,x5,x6) __glFrustum((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glViewport(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLsizei width,__reg("d3") GLsizei height ,__reg("a6") void *)="\tjsr\t-492(a6)";
#define glViewport(x1,x2,x3,x4) __glViewport((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glPushMatrix(__reg("a6") void *)="\tjsr\t-498(a6)";
#define glPushMatrix() __glPushMatrix(glBase)
void APIENTRY  __glPopMatrix(__reg("a6") void *)="\tjsr\t-504(a6)";
#define glPopMatrix() __glPopMatrix(glBase)
void APIENTRY  __glLoadIdentity(__reg("a6") void *)="\tjsr\t-510(a6)";
#define glLoadIdentity() __glLoadIdentity(glBase)
void APIENTRY  __glLoadMatrixd(__reg("a0") const GLdouble *m ,__reg("a6") void *)="\tjsr\t-516(a6)";
#define glLoadMatrixd(x1) __glLoadMatrixd((x1),glBase)
void APIENTRY  __glLoadMatrixf(__reg("a0") const GLfloat *m ,__reg("a6") void *)="\tjsr\t-522(a6)";
#define glLoadMatrixf(x1) __glLoadMatrixf((x1),glBase)
void APIENTRY  __glMultMatrixd(__reg("a0") const GLdouble *m ,__reg("a6") void *)="\tjsr\t-528(a6)";
#define glMultMatrixd(x1) __glMultMatrixd((x1),glBase)
void APIENTRY  __glMultMatrixf(__reg("a0") const GLfloat *m ,__reg("a6") void *)="\tjsr\t-534(a6)";
#define glMultMatrixf(x1) __glMultMatrixf((x1),glBase)
void APIENTRY  __glRotated(__reg("fp0") GLdouble angle,__reg("fp1") GLdouble x,__reg("fp2") GLdouble y,__reg("fp3") GLdouble z ,__reg("a6") void *)="\tjsr\t-540(a6)";
#define glRotated(x1,x2,x3,x4) __glRotated((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRotatef(__reg("fp0") GLfloat angle,__reg("fp1") GLfloat x,__reg("fp2") GLfloat y,__reg("fp3") GLfloat z ,__reg("a6") void *)="\tjsr\t-546(a6)";
#define glRotatef(x1,x2,x3,x4) __glRotatef((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glScaled(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z ,__reg("a6") void *)="\tjsr\t-552(a6)";
#define glScaled(x1,x2,x3) __glScaled((x1),(x2),(x3),glBase)
void APIENTRY  __glScalef(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z ,__reg("a6") void *)="\tjsr\t-558(a6)";
#define glScalef(x1,x2,x3) __glScalef((x1),(x2),(x3),glBase)
void APIENTRY  __glTranslated(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z ,__reg("a6") void *)="\tjsr\t-564(a6)";
#define glTranslated(x1,x2,x3) __glTranslated((x1),(x2),(x3),glBase)
void APIENTRY  __glTranslatef(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z ,__reg("a6") void *)="\tjsr\t-570(a6)";
#define glTranslatef(x1,x2,x3) __glTranslatef((x1),(x2),(x3),glBase)
GLboolean APIENTRY  __glIsList(__reg("d0") GLuint list ,__reg("a6") void *)="\tjsr\t-576(a6)";
#define glIsList(x1) __glIsList((x1),glBase)
void APIENTRY  __glDeleteLists(__reg("d0") GLuint list,__reg("d1") GLsizei range ,__reg("a6") void *)="\tjsr\t-582(a6)";
#define glDeleteLists(x1,x2) __glDeleteLists((x1),(x2),glBase)
GLuint APIENTRY  __glGenLists(__reg("d0") GLsizei range ,__reg("a6") void *)="\tjsr\t-588(a6)";
#define glGenLists(x1) __glGenLists((x1),glBase)
void APIENTRY  __glNewList(__reg("d0") GLuint list,__reg("d1") GLenum mode ,__reg("a6") void *)="\tjsr\t-594(a6)";
#define glNewList(x1,x2) __glNewList((x1),(x2),glBase)
void APIENTRY  __glEndList(__reg("a6") void *)="\tjsr\t-600(a6)";
#define glEndList() __glEndList(glBase)
void APIENTRY  __glCallList(__reg("d0") GLuint list ,__reg("a6") void *)="\tjsr\t-606(a6)";
#define glCallList(x1) __glCallList((x1),glBase)
void APIENTRY  __glCallLists(__reg("d0") GLsizei n,__reg("d1") GLenum type,__reg("a0") const GLvoid *lists ,__reg("a6") void *)="\tjsr\t-612(a6)";
#define glCallLists(x1,x2,x3) __glCallLists((x1),(x2),(x3),glBase)
void APIENTRY  __glListBase(__reg("d0") GLuint base ,__reg("a6") void *)="\tjsr\t-618(a6)";
#define glListBase(x1) __glListBase((x1),glBase)
void APIENTRY  __glBegin(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-624(a6)";
#define glBegin(x1) __glBegin((x1),glBase)
void APIENTRY  __glEnd(__reg("a6") void *)="\tjsr\t-630(a6)";
#define glEnd() __glEnd(glBase)
void APIENTRY  __glVertex2d(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y ,__reg("a6") void *)="\tjsr\t-636(a6)";
#define glVertex2d(x1,x2) __glVertex2d((x1),(x2),glBase)
void APIENTRY  __glVertex2f(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y ,__reg("a6") void *)="\tjsr\t-642(a6)";
#define glVertex2f(x1,x2) __glVertex2f((x1),(x2),glBase)
void APIENTRY  __glVertex2i(__reg("d0") GLint x,__reg("d1") GLint y ,__reg("a6") void *)="\tjsr\t-648(a6)";
#define glVertex2i(x1,x2) __glVertex2i((x1),(x2),glBase)
void APIENTRY  __glVertex2s(__reg("d0") GLshort x,__reg("d1") GLshort y ,__reg("a6") void *)="\tjsr\t-654(a6)";
#define glVertex2s(x1,x2) __glVertex2s((x1),(x2),glBase)
void APIENTRY  __glVertex3d(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z ,__reg("a6") void *)="\tjsr\t-660(a6)";
#define glVertex3d(x1,x2,x3) __glVertex3d((x1),(x2),(x3),glBase)
void APIENTRY  __glVertex3f(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z ,__reg("a6") void *)="\tjsr\t-666(a6)";
#define glVertex3f(x1,x2,x3) __glVertex3f((x1),(x2),(x3),glBase)
void APIENTRY  __glVertex3i(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLint z ,__reg("a6") void *)="\tjsr\t-672(a6)";
#define glVertex3i(x1,x2,x3) __glVertex3i((x1),(x2),(x3),glBase)
void APIENTRY  __glVertex3s(__reg("d0") GLshort x,__reg("d1") GLshort y,__reg("d2") GLshort z ,__reg("a6") void *)="\tjsr\t-678(a6)";
#define glVertex3s(x1,x2,x3) __glVertex3s((x1),(x2),(x3),glBase)
void APIENTRY  __glVertex4d(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z,__reg("fp3") GLdouble w ,__reg("a6") void *)="\tjsr\t-684(a6)";
#define glVertex4d(x1,x2,x3,x4) __glVertex4d((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glVertex4f(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z,__reg("fp3") GLfloat w ,__reg("a6") void *)="\tjsr\t-690(a6)";
#define glVertex4f(x1,x2,x3,x4) __glVertex4f((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glVertex4i(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLint z,__reg("d3") GLint w ,__reg("a6") void *)="\tjsr\t-696(a6)";
#define glVertex4i(x1,x2,x3,x4) __glVertex4i((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glVertex4s(__reg("d0") GLshort x,__reg("d1") GLshort y,__reg("d2") GLshort z,__reg("d3") GLshort w ,__reg("a6") void *)="\tjsr\t-702(a6)";
#define glVertex4s(x1,x2,x3,x4) __glVertex4s((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glVertex2dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-708(a6)";
#define glVertex2dv(x1) __glVertex2dv((x1),glBase)
void APIENTRY  __glVertex2fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-714(a6)";
#define glVertex2fv(x1) __glVertex2fv((x1),glBase)
void APIENTRY  __glVertex2iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-720(a6)";
#define glVertex2iv(x1) __glVertex2iv((x1),glBase)
void APIENTRY  __glVertex2sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-726(a6)";
#define glVertex2sv(x1) __glVertex2sv((x1),glBase)
void APIENTRY  __glVertex3dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-732(a6)";
#define glVertex3dv(x1) __glVertex3dv((x1),glBase)
void APIENTRY  __glVertex3fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-738(a6)";
#define glVertex3fv(x1) __glVertex3fv((x1),glBase)
void APIENTRY  __glVertex3iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-744(a6)";
#define glVertex3iv(x1) __glVertex3iv((x1),glBase)
void APIENTRY  __glVertex3sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-750(a6)";
#define glVertex3sv(x1) __glVertex3sv((x1),glBase)
void APIENTRY  __glVertex4dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-756(a6)";
#define glVertex4dv(x1) __glVertex4dv((x1),glBase)
void APIENTRY  __glVertex4fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-762(a6)";
#define glVertex4fv(x1) __glVertex4fv((x1),glBase)
void APIENTRY  __glVertex4iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-768(a6)";
#define glVertex4iv(x1) __glVertex4iv((x1),glBase)
void APIENTRY  __glVertex4sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-774(a6)";
#define glVertex4sv(x1) __glVertex4sv((x1),glBase)
void APIENTRY  __glNormal3b(__reg("d0") GLbyte nx,__reg("d1") GLbyte ny,__reg("d2") GLbyte nz ,__reg("a6") void *)="\tjsr\t-780(a6)";
#define glNormal3b(x1,x2,x3) __glNormal3b((x1),(x2),(x3),glBase)
void APIENTRY  __glNormal3d(__reg("fp0") GLdouble nx,__reg("fp1") GLdouble ny,__reg("fp2") GLdouble nz ,__reg("a6") void *)="\tjsr\t-786(a6)";
#define glNormal3d(x1,x2,x3) __glNormal3d((x1),(x2),(x3),glBase)
void APIENTRY  __glNormal3f(__reg("fp0") GLfloat nx,__reg("fp1") GLfloat ny,__reg("fp2") GLfloat nz ,__reg("a6") void *)="\tjsr\t-792(a6)";
#define glNormal3f(x1,x2,x3) __glNormal3f((x1),(x2),(x3),glBase)
void APIENTRY  __glNormal3i(__reg("d0") GLint nx,__reg("d1") GLint ny,__reg("d2") GLint nz ,__reg("a6") void *)="\tjsr\t-798(a6)";
#define glNormal3i(x1,x2,x3) __glNormal3i((x1),(x2),(x3),glBase)
void APIENTRY  __glNormal3s(__reg("d0") GLshort nx,__reg("d1") GLshort ny,__reg("d2") GLshort nz ,__reg("a6") void *)="\tjsr\t-804(a6)";
#define glNormal3s(x1,x2,x3) __glNormal3s((x1),(x2),(x3),glBase)
void APIENTRY  __glNormal3bv(__reg("a0") const GLbyte *v ,__reg("a6") void *)="\tjsr\t-810(a6)";
#define glNormal3bv(x1) __glNormal3bv((x1),glBase)
void APIENTRY  __glNormal3dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-816(a6)";
#define glNormal3dv(x1) __glNormal3dv((x1),glBase)
void APIENTRY  __glNormal3fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-822(a6)";
#define glNormal3fv(x1) __glNormal3fv((x1),glBase)
void APIENTRY  __glNormal3iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-828(a6)";
#define glNormal3iv(x1) __glNormal3iv((x1),glBase)
void APIENTRY  __glNormal3sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-834(a6)";
#define glNormal3sv(x1) __glNormal3sv((x1),glBase)
void APIENTRY  __glIndexd(__reg("fp0") GLdouble c ,__reg("a6") void *)="\tjsr\t-840(a6)";
#define glIndexd(x1) __glIndexd((x1),glBase)
void APIENTRY  __glIndexf(__reg("fp0") GLfloat c ,__reg("a6") void *)="\tjsr\t-846(a6)";
#define glIndexf(x1) __glIndexf((x1),glBase)
void APIENTRY  __glIndexi(__reg("d0") GLint c ,__reg("a6") void *)="\tjsr\t-852(a6)";
#define glIndexi(x1) __glIndexi((x1),glBase)
void APIENTRY  __glIndexs(__reg("d0") GLshort c ,__reg("a6") void *)="\tjsr\t-858(a6)";
#define glIndexs(x1) __glIndexs((x1),glBase)
void APIENTRY  __glIndexub(__reg("d0") GLubyte c ,__reg("a6") void *)="\tjsr\t-864(a6)";
#define glIndexub(x1) __glIndexub((x1),glBase)
void APIENTRY  __glIndexdv(__reg("a0") const GLdouble *c ,__reg("a6") void *)="\tjsr\t-870(a6)";
#define glIndexdv(x1) __glIndexdv((x1),glBase)
void APIENTRY  __glIndexfv(__reg("a0") const GLfloat *c ,__reg("a6") void *)="\tjsr\t-876(a6)";
#define glIndexfv(x1) __glIndexfv((x1),glBase)
void APIENTRY  __glIndexiv(__reg("a0") const GLint *c ,__reg("a6") void *)="\tjsr\t-882(a6)";
#define glIndexiv(x1) __glIndexiv((x1),glBase)
void APIENTRY  __glIndexsv(__reg("a0") const GLshort *c ,__reg("a6") void *)="\tjsr\t-888(a6)";
#define glIndexsv(x1) __glIndexsv((x1),glBase)
void APIENTRY  __glIndexubv(__reg("a0") const GLubyte *c ,__reg("a6") void *)="\tjsr\t-894(a6)";
#define glIndexubv(x1) __glIndexubv((x1),glBase)
void APIENTRY  __glColor3b(__reg("d0") GLbyte red,__reg("d1") GLbyte green,__reg("d2") GLbyte blue ,__reg("a6") void *)="\tjsr\t-900(a6)";
#define glColor3b(x1,x2,x3) __glColor3b((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3d(__reg("fp0") GLdouble red,__reg("fp1") GLdouble green,__reg("fp2") GLdouble blue ,__reg("a6") void *)="\tjsr\t-906(a6)";
#define glColor3d(x1,x2,x3) __glColor3d((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3f(__reg("fp0") GLfloat red,__reg("fp1") GLfloat green,__reg("fp2") GLfloat blue ,__reg("a6") void *)="\tjsr\t-912(a6)";
#define glColor3f(x1,x2,x3) __glColor3f((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3i(__reg("d0") GLint red,__reg("d1") GLint green,__reg("d2") GLint blue ,__reg("a6") void *)="\tjsr\t-918(a6)";
#define glColor3i(x1,x2,x3) __glColor3i((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3s(__reg("d0") GLshort red,__reg("d1") GLshort green,__reg("d2") GLshort blue ,__reg("a6") void *)="\tjsr\t-924(a6)";
#define glColor3s(x1,x2,x3) __glColor3s((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3ub(__reg("d0") GLubyte red,__reg("d1") GLubyte green,__reg("d2") GLubyte blue ,__reg("a6") void *)="\tjsr\t-930(a6)";
#define glColor3ub(x1,x2,x3) __glColor3ub((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3ui(__reg("d0") GLuint red,__reg("d1") GLuint green,__reg("d2") GLuint blue ,__reg("a6") void *)="\tjsr\t-936(a6)";
#define glColor3ui(x1,x2,x3) __glColor3ui((x1),(x2),(x3),glBase)
void APIENTRY  __glColor3us(__reg("d0") GLushort red,__reg("d1") GLushort green,__reg("d2") GLushort blue ,__reg("a6") void *)="\tjsr\t-942(a6)";
#define glColor3us(x1,x2,x3) __glColor3us((x1),(x2),(x3),glBase)
void APIENTRY  __glColor4b(__reg("d0") GLbyte red,__reg("d1") GLbyte green,__reg("d2") GLbyte blue,__reg("d3") GLbyte alpha ,__reg("a6") void *)="\tjsr\t-948(a6)";
#define glColor4b(x1,x2,x3,x4) __glColor4b((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4d(__reg("fp0") GLdouble red,__reg("fp1") GLdouble green,__reg("fp2") GLdouble blue,__reg("fp3") GLdouble alpha ,__reg("a6") void *)="\tjsr\t-954(a6)";
#define glColor4d(x1,x2,x3,x4) __glColor4d((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4f(__reg("fp0") GLfloat red,__reg("fp1") GLfloat green,__reg("fp2") GLfloat blue,__reg("fp3") GLfloat alpha ,__reg("a6") void *)="\tjsr\t-960(a6)";
#define glColor4f(x1,x2,x3,x4) __glColor4f((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4i(__reg("d0") GLint red,__reg("d1") GLint green,__reg("d2") GLint blue,__reg("d3") GLint alpha ,__reg("a6") void *)="\tjsr\t-966(a6)";
#define glColor4i(x1,x2,x3,x4) __glColor4i((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4s(__reg("d0") GLshort red,__reg("d1") GLshort green,__reg("d2") GLshort blue,__reg("d3") GLshort alpha ,__reg("a6") void *)="\tjsr\t-972(a6)";
#define glColor4s(x1,x2,x3,x4) __glColor4s((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4ub(__reg("d0") GLubyte red,__reg("d1") GLubyte green,__reg("d2") GLubyte blue,__reg("d3") GLubyte alpha ,__reg("a6") void *)="\tjsr\t-978(a6)";
#define glColor4ub(x1,x2,x3,x4) __glColor4ub((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4ui(__reg("d0") GLuint red,__reg("d1") GLuint green,__reg("d2") GLuint blue,__reg("d3") GLuint alpha ,__reg("a6") void *)="\tjsr\t-984(a6)";
#define glColor4ui(x1,x2,x3,x4) __glColor4ui((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor4us(__reg("d0") GLushort red,__reg("d1") GLushort green,__reg("d2") GLushort blue,__reg("d3") GLushort alpha ,__reg("a6") void *)="\tjsr\t-990(a6)";
#define glColor4us(x1,x2,x3,x4) __glColor4us((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColor3bv(__reg("a0") const GLbyte *v ,__reg("a6") void *)="\tjsr\t-996(a6)";
#define glColor3bv(x1) __glColor3bv((x1),glBase)
void APIENTRY  __glColor3dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1002(a6)";
#define glColor3dv(x1) __glColor3dv((x1),glBase)
void APIENTRY  __glColor3fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1008(a6)";
#define glColor3fv(x1) __glColor3fv((x1),glBase)
void APIENTRY  __glColor3iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1014(a6)";
#define glColor3iv(x1) __glColor3iv((x1),glBase)
void APIENTRY  __glColor3sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1020(a6)";
#define glColor3sv(x1) __glColor3sv((x1),glBase)
void APIENTRY  __glColor3ubv(__reg("a0") const GLubyte *v ,__reg("a6") void *)="\tjsr\t-1026(a6)";
#define glColor3ubv(x1) __glColor3ubv((x1),glBase)
void APIENTRY  __glColor3uiv(__reg("a0") const GLuint *v ,__reg("a6") void *)="\tjsr\t-1032(a6)";
#define glColor3uiv(x1) __glColor3uiv((x1),glBase)
void APIENTRY  __glColor3usv(__reg("a0") const GLushort *v ,__reg("a6") void *)="\tjsr\t-1038(a6)";
#define glColor3usv(x1) __glColor3usv((x1),glBase)
void APIENTRY  __glColor4bv(__reg("a0") const GLbyte *v ,__reg("a6") void *)="\tjsr\t-1044(a6)";
#define glColor4bv(x1) __glColor4bv((x1),glBase)
void APIENTRY  __glColor4dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1050(a6)";
#define glColor4dv(x1) __glColor4dv((x1),glBase)
void APIENTRY  __glColor4fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1056(a6)";
#define glColor4fv(x1) __glColor4fv((x1),glBase)
void APIENTRY  __glColor4iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1062(a6)";
#define glColor4iv(x1) __glColor4iv((x1),glBase)
void APIENTRY  __glColor4sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1068(a6)";
#define glColor4sv(x1) __glColor4sv((x1),glBase)
void APIENTRY  __glColor4ubv(__reg("a0") const GLubyte *v ,__reg("a6") void *)="\tjsr\t-1074(a6)";
#define glColor4ubv(x1) __glColor4ubv((x1),glBase)
void APIENTRY  __glColor4uiv(__reg("a0") const GLuint *v ,__reg("a6") void *)="\tjsr\t-1080(a6)";
#define glColor4uiv(x1) __glColor4uiv((x1),glBase)
void APIENTRY  __glColor4usv(__reg("a0") const GLushort *v ,__reg("a6") void *)="\tjsr\t-1086(a6)";
#define glColor4usv(x1) __glColor4usv((x1),glBase)
void APIENTRY  __glTexCoord1d(__reg("fp0") GLdouble s ,__reg("a6") void *)="\tjsr\t-1092(a6)";
#define glTexCoord1d(x1) __glTexCoord1d((x1),glBase)
void APIENTRY  __glTexCoord1f(__reg("fp0") GLfloat s ,__reg("a6") void *)="\tjsr\t-1098(a6)";
#define glTexCoord1f(x1) __glTexCoord1f((x1),glBase)
void APIENTRY  __glTexCoord1i(__reg("d0") GLint s ,__reg("a6") void *)="\tjsr\t-1104(a6)";
#define glTexCoord1i(x1) __glTexCoord1i((x1),glBase)
void APIENTRY  __glTexCoord1s(__reg("d0") GLshort s ,__reg("a6") void *)="\tjsr\t-1110(a6)";
#define glTexCoord1s(x1) __glTexCoord1s((x1),glBase)
void APIENTRY  __glTexCoord2d(__reg("fp0") GLdouble s,__reg("fp1") GLdouble t ,__reg("a6") void *)="\tjsr\t-1116(a6)";
#define glTexCoord2d(x1,x2) __glTexCoord2d((x1),(x2),glBase)
void APIENTRY  __glTexCoord2f(__reg("fp0") GLfloat s,__reg("fp1") GLfloat t ,__reg("a6") void *)="\tjsr\t-1122(a6)";
#define glTexCoord2f(x1,x2) __glTexCoord2f((x1),(x2),glBase)
void APIENTRY  __glTexCoord2i(__reg("d0") GLint s,__reg("d1") GLint t ,__reg("a6") void *)="\tjsr\t-1128(a6)";
#define glTexCoord2i(x1,x2) __glTexCoord2i((x1),(x2),glBase)
void APIENTRY  __glTexCoord2s(__reg("d0") GLshort s,__reg("d1") GLshort t ,__reg("a6") void *)="\tjsr\t-1134(a6)";
#define glTexCoord2s(x1,x2) __glTexCoord2s((x1),(x2),glBase)
void APIENTRY  __glTexCoord3d(__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("fp2") GLdouble r ,__reg("a6") void *)="\tjsr\t-1140(a6)";
#define glTexCoord3d(x1,x2,x3) __glTexCoord3d((x1),(x2),(x3),glBase)
void APIENTRY  __glTexCoord3f(__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("fp2") GLfloat r ,__reg("a6") void *)="\tjsr\t-1146(a6)";
#define glTexCoord3f(x1,x2,x3) __glTexCoord3f((x1),(x2),(x3),glBase)
void APIENTRY  __glTexCoord3i(__reg("d0") GLint s,__reg("d1") GLint t,__reg("d2") GLint r ,__reg("a6") void *)="\tjsr\t-1152(a6)";
#define glTexCoord3i(x1,x2,x3) __glTexCoord3i((x1),(x2),(x3),glBase)
void APIENTRY  __glTexCoord3s(__reg("d0") GLshort s,__reg("d1") GLshort t,__reg("d2") GLshort r ,__reg("a6") void *)="\tjsr\t-1158(a6)";
#define glTexCoord3s(x1,x2,x3) __glTexCoord3s((x1),(x2),(x3),glBase)
void APIENTRY  __glTexCoord4d(__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("fp2") GLdouble r,__reg("fp3") GLdouble q ,__reg("a6") void *)="\tjsr\t-1164(a6)";
#define glTexCoord4d(x1,x2,x3,x4) __glTexCoord4d((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glTexCoord4f(__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("fp2") GLfloat r,__reg("fp3") GLfloat q ,__reg("a6") void *)="\tjsr\t-1170(a6)";
#define glTexCoord4f(x1,x2,x3,x4) __glTexCoord4f((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glTexCoord4i(__reg("d0") GLint s,__reg("d1") GLint t,__reg("d2") GLint r,__reg("d3") GLint q ,__reg("a6") void *)="\tjsr\t-1176(a6)";
#define glTexCoord4i(x1,x2,x3,x4) __glTexCoord4i((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glTexCoord4s(__reg("d0") GLshort s,__reg("d1") GLshort t,__reg("d2") GLshort r,__reg("d3") GLshort q ,__reg("a6") void *)="\tjsr\t-1182(a6)";
#define glTexCoord4s(x1,x2,x3,x4) __glTexCoord4s((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glTexCoord1dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1188(a6)";
#define glTexCoord1dv(x1) __glTexCoord1dv((x1),glBase)
void APIENTRY  __glTexCoord1fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1194(a6)";
#define glTexCoord1fv(x1) __glTexCoord1fv((x1),glBase)
void APIENTRY  __glTexCoord1iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1200(a6)";
#define glTexCoord1iv(x1) __glTexCoord1iv((x1),glBase)
void APIENTRY  __glTexCoord1sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1206(a6)";
#define glTexCoord1sv(x1) __glTexCoord1sv((x1),glBase)
void APIENTRY  __glTexCoord2dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1212(a6)";
#define glTexCoord2dv(x1) __glTexCoord2dv((x1),glBase)
void APIENTRY  __glTexCoord2fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1218(a6)";
#define glTexCoord2fv(x1) __glTexCoord2fv((x1),glBase)
void APIENTRY  __glTexCoord2iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1224(a6)";
#define glTexCoord2iv(x1) __glTexCoord2iv((x1),glBase)
void APIENTRY  __glTexCoord2sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1230(a6)";
#define glTexCoord2sv(x1) __glTexCoord2sv((x1),glBase)
void APIENTRY  __glTexCoord3dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1236(a6)";
#define glTexCoord3dv(x1) __glTexCoord3dv((x1),glBase)
void APIENTRY  __glTexCoord3fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1242(a6)";
#define glTexCoord3fv(x1) __glTexCoord3fv((x1),glBase)
void APIENTRY  __glTexCoord3iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1248(a6)";
#define glTexCoord3iv(x1) __glTexCoord3iv((x1),glBase)
void APIENTRY  __glTexCoord3sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1254(a6)";
#define glTexCoord3sv(x1) __glTexCoord3sv((x1),glBase)
void APIENTRY  __glTexCoord4dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1260(a6)";
#define glTexCoord4dv(x1) __glTexCoord4dv((x1),glBase)
void APIENTRY  __glTexCoord4fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1266(a6)";
#define glTexCoord4fv(x1) __glTexCoord4fv((x1),glBase)
void APIENTRY  __glTexCoord4iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1272(a6)";
#define glTexCoord4iv(x1) __glTexCoord4iv((x1),glBase)
void APIENTRY  __glTexCoord4sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1278(a6)";
#define glTexCoord4sv(x1) __glTexCoord4sv((x1),glBase)
void APIENTRY  __glRasterPos2d(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y ,__reg("a6") void *)="\tjsr\t-1284(a6)";
#define glRasterPos2d(x1,x2) __glRasterPos2d((x1),(x2),glBase)
void APIENTRY  __glRasterPos2f(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y ,__reg("a6") void *)="\tjsr\t-1290(a6)";
#define glRasterPos2f(x1,x2) __glRasterPos2f((x1),(x2),glBase)
void APIENTRY  __glRasterPos2i(__reg("d0") GLint x,__reg("d1") GLint y ,__reg("a6") void *)="\tjsr\t-1296(a6)";
#define glRasterPos2i(x1,x2) __glRasterPos2i((x1),(x2),glBase)
void APIENTRY  __glRasterPos2s(__reg("d0") GLshort x,__reg("d1") GLshort y ,__reg("a6") void *)="\tjsr\t-1302(a6)";
#define glRasterPos2s(x1,x2) __glRasterPos2s((x1),(x2),glBase)
void APIENTRY  __glRasterPos3d(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z ,__reg("a6") void *)="\tjsr\t-1308(a6)";
#define glRasterPos3d(x1,x2,x3) __glRasterPos3d((x1),(x2),(x3),glBase)
void APIENTRY  __glRasterPos3f(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z ,__reg("a6") void *)="\tjsr\t-1314(a6)";
#define glRasterPos3f(x1,x2,x3) __glRasterPos3f((x1),(x2),(x3),glBase)
void APIENTRY  __glRasterPos3i(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLint z ,__reg("a6") void *)="\tjsr\t-1320(a6)";
#define glRasterPos3i(x1,x2,x3) __glRasterPos3i((x1),(x2),(x3),glBase)
void APIENTRY  __glRasterPos3s(__reg("d0") GLshort x,__reg("d1") GLshort y,__reg("d2") GLshort z ,__reg("a6") void *)="\tjsr\t-1326(a6)";
#define glRasterPos3s(x1,x2,x3) __glRasterPos3s((x1),(x2),(x3),glBase)
void APIENTRY  __glRasterPos4d(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z,__reg("fp3") GLdouble w ,__reg("a6") void *)="\tjsr\t-1332(a6)";
#define glRasterPos4d(x1,x2,x3,x4) __glRasterPos4d((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRasterPos4f(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z,__reg("fp3") GLfloat w ,__reg("a6") void *)="\tjsr\t-1338(a6)";
#define glRasterPos4f(x1,x2,x3,x4) __glRasterPos4f((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRasterPos4i(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLint z,__reg("d3") GLint w ,__reg("a6") void *)="\tjsr\t-1344(a6)";
#define glRasterPos4i(x1,x2,x3,x4) __glRasterPos4i((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRasterPos4s(__reg("d0") GLshort x,__reg("d1") GLshort y,__reg("d2") GLshort z,__reg("d3") GLshort w ,__reg("a6") void *)="\tjsr\t-1350(a6)";
#define glRasterPos4s(x1,x2,x3,x4) __glRasterPos4s((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRasterPos2dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1356(a6)";
#define glRasterPos2dv(x1) __glRasterPos2dv((x1),glBase)
void APIENTRY  __glRasterPos2fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1362(a6)";
#define glRasterPos2fv(x1) __glRasterPos2fv((x1),glBase)
void APIENTRY  __glRasterPos2iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1368(a6)";
#define glRasterPos2iv(x1) __glRasterPos2iv((x1),glBase)
void APIENTRY  __glRasterPos2sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1374(a6)";
#define glRasterPos2sv(x1) __glRasterPos2sv((x1),glBase)
void APIENTRY  __glRasterPos3dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1380(a6)";
#define glRasterPos3dv(x1) __glRasterPos3dv((x1),glBase)
void APIENTRY  __glRasterPos3fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1386(a6)";
#define glRasterPos3fv(x1) __glRasterPos3fv((x1),glBase)
void APIENTRY  __glRasterPos3iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1392(a6)";
#define glRasterPos3iv(x1) __glRasterPos3iv((x1),glBase)
void APIENTRY  __glRasterPos3sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1398(a6)";
#define glRasterPos3sv(x1) __glRasterPos3sv((x1),glBase)
void APIENTRY  __glRasterPos4dv(__reg("a0") const GLdouble *v ,__reg("a6") void *)="\tjsr\t-1404(a6)";
#define glRasterPos4dv(x1) __glRasterPos4dv((x1),glBase)
void APIENTRY  __glRasterPos4fv(__reg("a0") const GLfloat *v ,__reg("a6") void *)="\tjsr\t-1410(a6)";
#define glRasterPos4fv(x1) __glRasterPos4fv((x1),glBase)
void APIENTRY  __glRasterPos4iv(__reg("a0") const GLint *v ,__reg("a6") void *)="\tjsr\t-1416(a6)";
#define glRasterPos4iv(x1) __glRasterPos4iv((x1),glBase)
void APIENTRY  __glRasterPos4sv(__reg("a0") const GLshort *v ,__reg("a6") void *)="\tjsr\t-1422(a6)";
#define glRasterPos4sv(x1) __glRasterPos4sv((x1),glBase)
void APIENTRY  __glRectd(__reg("fp0") GLdouble x1,__reg("fp1") GLdouble y1,__reg("fp2") GLdouble x2,__reg("fp3") GLdouble y2 ,__reg("a6") void *)="\tjsr\t-1428(a6)";
#define glRectd(x1,x2,x3,x4) __glRectd((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRectf(__reg("fp0") GLfloat x1,__reg("fp1") GLfloat y1,__reg("fp2") GLfloat x2,__reg("fp3") GLfloat y2 ,__reg("a6") void *)="\tjsr\t-1434(a6)";
#define glRectf(x1,x2,x3,x4) __glRectf((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRecti(__reg("d0") GLint x1,__reg("d1") GLint y1,__reg("d2") GLint x2,__reg("d3") GLint y2 ,__reg("a6") void *)="\tjsr\t-1440(a6)";
#define glRecti(x1,x2,x3,x4) __glRecti((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRects(__reg("d0") GLshort x1,__reg("d1") GLshort y1,__reg("d2") GLshort x2,__reg("d3") GLshort y2 ,__reg("a6") void *)="\tjsr\t-1446(a6)";
#define glRects(x1,x2,x3,x4) __glRects((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glRectdv(__reg("a0") const GLdouble *v1,__reg("a1") const GLdouble *v2 ,__reg("a6") void *)="\tjsr\t-1452(a6)";
#define glRectdv(x1,x2) __glRectdv((x1),(x2),glBase)
void APIENTRY  __glRectfv(__reg("a0") const GLfloat *v1,__reg("a1") const GLfloat *v2 ,__reg("a6") void *)="\tjsr\t-1458(a6)";
#define glRectfv(x1,x2) __glRectfv((x1),(x2),glBase)
void APIENTRY  __glRectiv(__reg("a0") const GLint *v1,__reg("a1") const GLint *v2 ,__reg("a6") void *)="\tjsr\t-1464(a6)";
#define glRectiv(x1,x2) __glRectiv((x1),(x2),glBase)
void APIENTRY  __glRectsv(__reg("a0") const GLshort *v1,__reg("a1") const GLshort *v2 ,__reg("a6") void *)="\tjsr\t-1470(a6)";
#define glRectsv(x1,x2) __glRectsv((x1),(x2),glBase)
void APIENTRY  __glVertexPointer(__reg("d0") GLint size,__reg("d1") GLenum type,__reg("d2") GLsizei stride,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-1476(a6)";
#define glVertexPointer(x1,x2,x3,x4) __glVertexPointer((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glNormalPointer(__reg("d0") GLenum type,__reg("d1") GLsizei stride,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-1482(a6)";
#define glNormalPointer(x1,x2,x3) __glNormalPointer((x1),(x2),(x3),glBase)
void APIENTRY  __glColorPointer(__reg("d0") GLint size,__reg("d1") GLenum type,__reg("d2") GLsizei stride,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-1488(a6)";
#define glColorPointer(x1,x2,x3,x4) __glColorPointer((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glIndexPointer(__reg("d0") GLenum type,__reg("d1") GLsizei stride,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-1494(a6)";
#define glIndexPointer(x1,x2,x3) __glIndexPointer((x1),(x2),(x3),glBase)
void APIENTRY  __glTexCoordPointer(__reg("d0") GLint size,__reg("d1") GLenum type,__reg("d2") GLsizei stride,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-1500(a6)";
#define glTexCoordPointer(x1,x2,x3,x4) __glTexCoordPointer((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glEdgeFlagPointer(__reg("d0") GLsizei stride,__reg("a0") const GLboolean *ptr ,__reg("a6") void *)="\tjsr\t-1506(a6)";
#define glEdgeFlagPointer(x1,x2) __glEdgeFlagPointer((x1),(x2),glBase)
void APIENTRY  __glGetPointerv(__reg("d0") GLenum pname,__reg("a0") void **params ,__reg("a6") void *)="\tjsr\t-1512(a6)";
#define glGetPointerv(x1,x2) __glGetPointerv((x1),(x2),glBase)
void APIENTRY  __glArrayElement(__reg("d0") GLint i ,__reg("a6") void *)="\tjsr\t-1518(a6)";
#define glArrayElement(x1) __glArrayElement((x1),glBase)
void APIENTRY  __glDrawArrays(__reg("d0") GLenum mode,__reg("d1") GLint first,__reg("d2") GLsizei count ,__reg("a6") void *)="\tjsr\t-1524(a6)";
#define glDrawArrays(x1,x2,x3) __glDrawArrays((x1),(x2),(x3),glBase)
void APIENTRY  __glDrawElements(__reg("d0") GLenum mode,__reg("d1") GLsizei count,__reg("d2") GLenum type,__reg("a0") const GLvoid *indices ,__reg("a6") void *)="\tjsr\t-1530(a6)";
#define glDrawElements(x1,x2,x3,x4) __glDrawElements((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glInterleavedArrays(__reg("d0") GLenum format,__reg("d1") GLsizei stride,__reg("a0") const GLvoid *pointer ,__reg("a6") void *)="\tjsr\t-1536(a6)";
#define glInterleavedArrays(x1,x2,x3) __glInterleavedArrays((x1),(x2),(x3),glBase)
void APIENTRY  __glShadeModel(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-1542(a6)";
#define glShadeModel(x1) __glShadeModel((x1),glBase)
void APIENTRY  __glLightf(__reg("d0") GLenum light,__reg("d1") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1548(a6)";
#define glLightf(x1,x2,x3) __glLightf((x1),(x2),(x3),glBase)
void APIENTRY  __glLighti(__reg("d0") GLenum light,__reg("d1") GLenum pname,__reg("d2") GLint param ,__reg("a6") void *)="\tjsr\t-1554(a6)";
#define glLighti(x1,x2,x3) __glLighti((x1),(x2),(x3),glBase)
void APIENTRY  __glLightfv(__reg("d0") GLenum light,__reg("d1") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-1560(a6)";
#define glLightfv(x1,x2,x3) __glLightfv((x1),(x2),(x3),glBase)
void APIENTRY  __glLightiv(__reg("d0") GLenum light,__reg("d1") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-1566(a6)";
#define glLightiv(x1,x2,x3) __glLightiv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetLightfv(__reg("d0") GLenum light,__reg("d1") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-1572(a6)";
#define glGetLightfv(x1,x2,x3) __glGetLightfv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetLightiv(__reg("d0") GLenum light,__reg("d1") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-1578(a6)";
#define glGetLightiv(x1,x2,x3) __glGetLightiv((x1),(x2),(x3),glBase)
void APIENTRY  __glLightModelf(__reg("d0") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1584(a6)";
#define glLightModelf(x1,x2) __glLightModelf((x1),(x2),glBase)
void APIENTRY  __glLightModeli(__reg("d0") GLenum pname,__reg("d1") GLint param ,__reg("a6") void *)="\tjsr\t-1590(a6)";
#define glLightModeli(x1,x2) __glLightModeli((x1),(x2),glBase)
void APIENTRY  __glLightModelfv(__reg("d0") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-1596(a6)";
#define glLightModelfv(x1,x2) __glLightModelfv((x1),(x2),glBase)
void APIENTRY  __glLightModeliv(__reg("d0") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-1602(a6)";
#define glLightModeliv(x1,x2) __glLightModeliv((x1),(x2),glBase)
void APIENTRY  __glMaterialf(__reg("d0") GLenum face,__reg("d1") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1608(a6)";
#define glMaterialf(x1,x2,x3) __glMaterialf((x1),(x2),(x3),glBase)
void APIENTRY  __glMateriali(__reg("d0") GLenum face,__reg("d1") GLenum pname,__reg("d2") GLint param ,__reg("a6") void *)="\tjsr\t-1614(a6)";
#define glMateriali(x1,x2,x3) __glMateriali((x1),(x2),(x3),glBase)
void APIENTRY  __glMaterialfv(__reg("d0") GLenum face,__reg("d1") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-1620(a6)";
#define glMaterialfv(x1,x2,x3) __glMaterialfv((x1),(x2),(x3),glBase)
void APIENTRY  __glMaterialiv(__reg("d0") GLenum face,__reg("d1") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-1626(a6)";
#define glMaterialiv(x1,x2,x3) __glMaterialiv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetMaterialfv(__reg("d0") GLenum face,__reg("d1") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-1632(a6)";
#define glGetMaterialfv(x1,x2,x3) __glGetMaterialfv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetMaterialiv(__reg("d0") GLenum face,__reg("d1") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-1638(a6)";
#define glGetMaterialiv(x1,x2,x3) __glGetMaterialiv((x1),(x2),(x3),glBase)
void APIENTRY  __glColorMaterial(__reg("d0") GLenum face,__reg("d1") GLenum mode ,__reg("a6") void *)="\tjsr\t-1644(a6)";
#define glColorMaterial(x1,x2) __glColorMaterial((x1),(x2),glBase)
void APIENTRY  __glPixelZoom(__reg("fp0") GLfloat xfactor,__reg("fp1") GLfloat yfactor ,__reg("a6") void *)="\tjsr\t-1650(a6)";
#define glPixelZoom(x1,x2) __glPixelZoom((x1),(x2),glBase)
void APIENTRY  __glPixelStoref(__reg("d0") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1656(a6)";
#define glPixelStoref(x1,x2) __glPixelStoref((x1),(x2),glBase)
void APIENTRY  __glPixelStorei(__reg("d0") GLenum pname,__reg("d1") GLint param ,__reg("a6") void *)="\tjsr\t-1662(a6)";
#define glPixelStorei(x1,x2) __glPixelStorei((x1),(x2),glBase)
void APIENTRY  __glPixelTransferf(__reg("d0") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1668(a6)";
#define glPixelTransferf(x1,x2) __glPixelTransferf((x1),(x2),glBase)
void APIENTRY  __glPixelTransferi(__reg("d0") GLenum pname,__reg("d1") GLint param ,__reg("a6") void *)="\tjsr\t-1674(a6)";
#define glPixelTransferi(x1,x2) __glPixelTransferi((x1),(x2),glBase)
void APIENTRY  __glPixelMapfv(__reg("d0") GLenum map,__reg("d1") GLint mapsize,__reg("a0") const GLfloat *values ,__reg("a6") void *)="\tjsr\t-1680(a6)";
#define glPixelMapfv(x1,x2,x3) __glPixelMapfv((x1),(x2),(x3),glBase)
void APIENTRY  __glPixelMapuiv(__reg("d0") GLenum map,__reg("d1") GLint mapsize,__reg("a0") const GLuint *values ,__reg("a6") void *)="\tjsr\t-1686(a6)";
#define glPixelMapuiv(x1,x2,x3) __glPixelMapuiv((x1),(x2),(x3),glBase)
void APIENTRY  __glPixelMapusv(__reg("d0") GLenum map,__reg("d1") GLint mapsize,__reg("a0") const GLushort *values ,__reg("a6") void *)="\tjsr\t-1692(a6)";
#define glPixelMapusv(x1,x2,x3) __glPixelMapusv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetPixelMapfv(__reg("d0") GLenum map,__reg("a0") GLfloat *values ,__reg("a6") void *)="\tjsr\t-1698(a6)";
#define glGetPixelMapfv(x1,x2) __glGetPixelMapfv((x1),(x2),glBase)
void APIENTRY  __glGetPixelMapuiv(__reg("d0") GLenum map,__reg("a0") GLuint *values ,__reg("a6") void *)="\tjsr\t-1704(a6)";
#define glGetPixelMapuiv(x1,x2) __glGetPixelMapuiv((x1),(x2),glBase)
void APIENTRY  __glGetPixelMapusv(__reg("d0") GLenum map,__reg("a0") GLushort *values ,__reg("a6") void *)="\tjsr\t-1710(a6)";
#define glGetPixelMapusv(x1,x2) __glGetPixelMapusv((x1),(x2),glBase)
void APIENTRY  __glBitmap(__reg("d0") GLsizei width,__reg("d1") GLsizei height,__reg("fp0") GLfloat xorig,__reg("fp1") GLfloat yorig,__reg("fp2") GLfloat xmove,__reg("fp3") GLfloat ymove,__reg("a0") const GLubyte *bitmap ,__reg("a6") void *)="\tjsr\t-1716(a6)";
#define glBitmap(x1,x2,x3,x4,x5,x6,x7) __glBitmap((x1),(x2),(x3),(x4),(x5),(x6),(x7),glBase)
void APIENTRY  __glReadPixels(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLsizei width,__reg("d3") GLsizei height,__reg("d4") GLenum format,__reg("d5") GLenum type,__reg("a0") GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1722(a6)";
#define glReadPixels(x1,x2,x3,x4,x5,x6,x7) __glReadPixels((x1),(x2),(x3),(x4),(x5),(x6),(x7),glBase)
void APIENTRY  __glDrawPixels(__reg("d0") GLsizei width,__reg("d1") GLsizei height,__reg("d2") GLenum format,__reg("d3") GLenum type,__reg("a0") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1728(a6)";
#define glDrawPixels(x1,x2,x3,x4,x5) __glDrawPixels((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glCopyPixels(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLsizei width,__reg("d3") GLsizei height,__reg("d4") GLenum type ,__reg("a6") void *)="\tjsr\t-1734(a6)";
#define glCopyPixels(x1,x2,x3,x4,x5) __glCopyPixels((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glStencilFunc(__reg("d0") GLenum func,__reg("d1") GLint ref,__reg("d2") GLuint mask ,__reg("a6") void *)="\tjsr\t-1740(a6)";
#define glStencilFunc(x1,x2,x3) __glStencilFunc((x1),(x2),(x3),glBase)
void APIENTRY  __glStencilMask(__reg("d0") GLuint mask ,__reg("a6") void *)="\tjsr\t-1746(a6)";
#define glStencilMask(x1) __glStencilMask((x1),glBase)
void APIENTRY  __glStencilOp(__reg("d0") GLenum fail,__reg("d1") GLenum zfail,__reg("d2") GLenum zpass ,__reg("a6") void *)="\tjsr\t-1752(a6)";
#define glStencilOp(x1,x2,x3) __glStencilOp((x1),(x2),(x3),glBase)
void APIENTRY  __glClearStencil(__reg("d0") GLint s ,__reg("a6") void *)="\tjsr\t-1758(a6)";
#define glClearStencil(x1) __glClearStencil((x1),glBase)
void APIENTRY  __glTexGend(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("fp0") GLdouble param ,__reg("a6") void *)="\tjsr\t-1764(a6)";
#define glTexGend(x1,x2,x3) __glTexGend((x1),(x2),(x3),glBase)
void APIENTRY  __glTexGenf(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1770(a6)";
#define glTexGenf(x1,x2,x3) __glTexGenf((x1),(x2),(x3),glBase)
void APIENTRY  __glTexGeni(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("d2") GLint param ,__reg("a6") void *)="\tjsr\t-1776(a6)";
#define glTexGeni(x1,x2,x3) __glTexGeni((x1),(x2),(x3),glBase)
void APIENTRY  __glTexGendv(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("a0") const GLdouble *params ,__reg("a6") void *)="\tjsr\t-1782(a6)";
#define glTexGendv(x1,x2,x3) __glTexGendv((x1),(x2),(x3),glBase)
void APIENTRY  __glTexGenfv(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-1788(a6)";
#define glTexGenfv(x1,x2,x3) __glTexGenfv((x1),(x2),(x3),glBase)
void APIENTRY  __glTexGeniv(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-1794(a6)";
#define glTexGeniv(x1,x2,x3) __glTexGeniv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexGendv(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("a0") GLdouble *params ,__reg("a6") void *)="\tjsr\t-1800(a6)";
#define glGetTexGendv(x1,x2,x3) __glGetTexGendv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexGenfv(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-1806(a6)";
#define glGetTexGenfv(x1,x2,x3) __glGetTexGenfv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexGeniv(__reg("d0") GLenum coord,__reg("d1") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-1812(a6)";
#define glGetTexGeniv(x1,x2,x3) __glGetTexGeniv((x1),(x2),(x3),glBase)
void APIENTRY  __glTexEnvf(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1818(a6)";
#define glTexEnvf(x1,x2,x3) __glTexEnvf((x1),(x2),(x3),glBase)
void APIENTRY  __glTexEnvi(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("d2") GLint param ,__reg("a6") void *)="\tjsr\t-1824(a6)";
#define glTexEnvi(x1,x2,x3) __glTexEnvi((x1),(x2),(x3),glBase)
void APIENTRY  __glTexEnvfv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-1830(a6)";
#define glTexEnvfv(x1,x2,x3) __glTexEnvfv((x1),(x2),(x3),glBase)
void APIENTRY  __glTexEnviv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-1836(a6)";
#define glTexEnviv(x1,x2,x3) __glTexEnviv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexEnvfv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-1842(a6)";
#define glGetTexEnvfv(x1,x2,x3) __glGetTexEnvfv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexEnviv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-1848(a6)";
#define glGetTexEnviv(x1,x2,x3) __glGetTexEnviv((x1),(x2),(x3),glBase)
void APIENTRY  __glTexParameterf(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-1854(a6)";
#define glTexParameterf(x1,x2,x3) __glTexParameterf((x1),(x2),(x3),glBase)
void APIENTRY  __glTexParameteri(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("d2") GLint param ,__reg("a6") void *)="\tjsr\t-1860(a6)";
#define glTexParameteri(x1,x2,x3) __glTexParameteri((x1),(x2),(x3),glBase)
void APIENTRY  __glTexParameterfv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-1866(a6)";
#define glTexParameterfv(x1,x2,x3) __glTexParameterfv((x1),(x2),(x3),glBase)
void APIENTRY  __glTexParameteriv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-1872(a6)";
#define glTexParameteriv(x1,x2,x3) __glTexParameteriv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexParameterfv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") GLfloat *params,__reg("a6") void *)="\tjsr\t-1878(a6)";
#define glGetTexParameterfv(x1,x2,x3) __glGetTexParameterfv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexParameteriv(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-1884(a6)";
#define glGetTexParameteriv(x1,x2,x3) __glGetTexParameteriv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetTexLevelParameterfv(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-1890(a6)";
#define glGetTexLevelParameterfv(x1,x2,x3,x4) __glGetTexLevelParameterfv((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glGetTexLevelParameteriv(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-1896(a6)";
#define glGetTexLevelParameteriv(x1,x2,x3,x4) __glGetTexLevelParameteriv((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glTexImage1D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint internalFormat,__reg("d3") GLsizei width,__reg("d4") GLint border,__reg("d5") GLenum format,__reg("d6") GLenum type,__reg("a0") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1902(a6)";
#define glTexImage1D(x1,x2,x3,x4,x5,x6,x7,x8) __glTexImage1D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),glBase)
/**void APIENTRY  __glTexImage2D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint internalFormat,__reg("d3") GLsizei width,__reg("d4") GLsizei height,__reg("d5") GLint border,__reg("d6") GLenum format,__reg("d7") GLenum type,__reg("a0") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1908(a6)";*/
/**#define glTexImage2D(x1,x2,x3,x4,x5,x6,x7,x8,x9) __glTexImage2D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),glBase)*/
void APIENTRY  __glGetTexImage(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum format,__reg("d3") GLenum type,__reg("a0") GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1914(a6)";
#define glGetTexImage(x1,x2,x3,x4,x5) __glGetTexImage((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glGenTextures(__reg("d0") GLsizei n,__reg("a0") GLuint *textures ,__reg("a6") void *)="\tjsr\t-1920(a6)";
#define glGenTextures(x1,x2) __glGenTextures((x1),(x2),glBase)
void APIENTRY  __glDeleteTextures(__reg("d0") GLsizei n,__reg("a0") const GLuint *textures,__reg("a6") void *)="\tjsr\t-1926(a6)";
#define glDeleteTextures(x1,x2) __glDeleteTextures((x1),(x2),glBase)
void APIENTRY  __glBindTexture(__reg("d0") GLenum target,__reg("d1") GLuint texture ,__reg("a6") void *)="\tjsr\t-1932(a6)";
#define glBindTexture(x1,x2) __glBindTexture((x1),(x2),glBase)
void APIENTRY  __glPrioritizeTextures(__reg("d0") GLsizei n,__reg("a0") const GLuint *textures,__reg("a1") const GLclampf *priorities ,__reg("a6") void *)="\tjsr\t-1938(a6)";
#define glPrioritizeTextures(x1,x2,x3) __glPrioritizeTextures((x1),(x2),(x3),glBase)
GLboolean APIENTRY  __glAreTexturesResident(__reg("d0") GLsizei n,__reg("a0") const GLuint *textures,__reg("a1") GLboolean *residences ,__reg("a6") void *)="\tjsr\t-1944(a6)";
#define glAreTexturesResident(x1,x2,x3) __glAreTexturesResident((x1),(x2),(x3),glBase)
GLboolean APIENTRY  __glIsTexture(__reg("d0") GLuint texture ,__reg("a6") void *)="\tjsr\t-1950(a6)";
#define glIsTexture(x1) __glIsTexture((x1),glBase)
void APIENTRY  __glTexSubImage1D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLsizei width,__reg("d4") GLenum format,__reg("d5") GLenum type,__reg("a0") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1956(a6)";
#define glTexSubImage1D(x1,x2,x3,x4,x5,x6,x7) __glTexSubImage1D((x1),(x2),(x3),(x4),(x5),(x6),(x7),glBase)
/**void APIENTRY  __glTexSubImage2D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint yoffset,__reg("d4") GLsizei width,__reg("d5") GLsizei height,__reg("d6") GLenum format,__reg("d7") GLenum type,__reg("a0") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-1962(a6)";*/
/**#define glTexSubImage2D(x1,x2,x3,x4,x5,x6,x7,x8,x9) __glTexSubImage2D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),glBase)*/
void APIENTRY  __glCopyTexImage1D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum internalformat,__reg("d3") GLint x,__reg("d4") GLint y,__reg("d5") GLsizei width,__reg("d6") GLint border ,__reg("a6") void *)="\tjsr\t-1968(a6)";
#define glCopyTexImage1D(x1,x2,x3,x4,x5,x6,x7) __glCopyTexImage1D((x1),(x2),(x3),(x4),(x5),(x6),(x7),glBase)
void APIENTRY  __glCopyTexImage2D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum internalformat,__reg("d3") GLint x,__reg("d4") GLint y,__reg("d5") GLsizei width,__reg("d6") GLsizei height,__reg("d7") GLint border ,__reg("a6") void *)="\tjsr\t-1974(a6)";
#define glCopyTexImage2D(x1,x2,x3,x4,x5,x6,x7,x8) __glCopyTexImage2D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),glBase)
void APIENTRY  __glCopyTexSubImage1D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint x,__reg("d4") GLint y,__reg("d5") GLsizei width ,__reg("a6") void *)="\tjsr\t-1980(a6)";
#define glCopyTexSubImage1D(x1,x2,x3,x4,x5,x6) __glCopyTexSubImage1D((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glCopyTexSubImage2D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint yoffset,__reg("d4") GLint x,__reg("d5") GLint y,__reg("d6") GLsizei width,__reg("d7") GLsizei height ,__reg("a6") void *)="\tjsr\t-1986(a6)";
#define glCopyTexSubImage2D(x1,x2,x3,x4,x5,x6,x7,x8) __glCopyTexSubImage2D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),glBase)
void APIENTRY  __glMap1d(__reg("d0") GLenum target,__reg("fp0") GLdouble u1,__reg("fp1") GLdouble u2,__reg("d1") GLint stride,__reg("d2") GLint order,__reg("a0") const GLdouble *points ,__reg("a6") void *)="\tjsr\t-1992(a6)";
#define glMap1d(x1,x2,x3,x4,x5,x6) __glMap1d((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glMap1f(__reg("d0") GLenum target,__reg("fp0") GLfloat u1,__reg("fp1") GLfloat u2,__reg("d1") GLint stride,__reg("d2") GLint order,__reg("a0") const GLfloat *points ,__reg("a6") void *)="\tjsr\t-1998(a6)";
#define glMap1f(x1,x2,x3,x4,x5,x6) __glMap1f((x1),(x2),(x3),(x4),(x5),(x6),glBase)
/**void APIENTRY  __glMap2d(__reg("d0") GLenum target,__reg("fp0") GLdouble u1,__reg("fp1") GLdouble u2,__reg("d1") GLint ustride,__reg("d2") GLint uorder,__reg("fp2") GLdouble v1,__reg("fp3") GLdouble v2,__reg("d3") GLint vstride,__reg("d4") GLint vorder,__reg("a0") const GLdouble *points ,__reg("a6") void *)="\tjsr\t-2004(a6)";*/
/**#define glMap2d(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10) __glMap2d((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),glBase)*/
/**void APIENTRY  __glMap2f(__reg("d0") GLenum target,__reg("fp0") GLfloat u1,__reg("fp1") GLfloat u2,__reg("d1") GLint ustride,__reg("d2") GLint uorder,__reg("fp2") GLfloat v1,__reg("fp3") GLfloat v2,__reg("d3") GLint vstride,__reg("d4") GLint vorder,__reg("a0") const GLfloat *points ,__reg("a6") void *)="\tjsr\t-2010(a6)";*/
/**#define glMap2f(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10) __glMap2f((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),glBase)*/
void APIENTRY  __glGetMapdv(__reg("d0") GLenum target,__reg("d1") GLenum query,__reg("a0") GLdouble *v ,__reg("a6") void *)="\tjsr\t-2016(a6)";
#define glGetMapdv(x1,x2,x3) __glGetMapdv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetMapfv(__reg("d0") GLenum target,__reg("d1") GLenum query,__reg("a0") GLfloat *v ,__reg("a6") void *)="\tjsr\t-2022(a6)";
#define glGetMapfv(x1,x2,x3) __glGetMapfv((x1),(x2),(x3),glBase)
void APIENTRY  __glGetMapiv(__reg("d0") GLenum target,__reg("d1") GLenum query,__reg("a0") GLint *v ,__reg("a6") void *)="\tjsr\t-2028(a6)";
#define glGetMapiv(x1,x2,x3) __glGetMapiv((x1),(x2),(x3),glBase)
void APIENTRY  __glEvalCoord1d(__reg("fp0") GLdouble u ,__reg("a6") void *)="\tjsr\t-2034(a6)";
#define glEvalCoord1d(x1) __glEvalCoord1d((x1),glBase)
void APIENTRY  __glEvalCoord1f(__reg("fp0") GLfloat u ,__reg("a6") void *)="\tjsr\t-2040(a6)";
#define glEvalCoord1f(x1) __glEvalCoord1f((x1),glBase)
void APIENTRY  __glEvalCoord1dv(__reg("a0") const GLdouble *u ,__reg("a6") void *)="\tjsr\t-2046(a6)";
#define glEvalCoord1dv(x1) __glEvalCoord1dv((x1),glBase)
void APIENTRY  __glEvalCoord1fv(__reg("a0") const GLfloat *u ,__reg("a6") void *)="\tjsr\t-2052(a6)";
#define glEvalCoord1fv(x1) __glEvalCoord1fv((x1),glBase)
void APIENTRY  __glEvalCoord2d(__reg("fp0") GLdouble u,__reg("fp1") GLdouble v ,__reg("a6") void *)="\tjsr\t-2058(a6)";
#define glEvalCoord2d(x1,x2) __glEvalCoord2d((x1),(x2),glBase)
void APIENTRY  __glEvalCoord2f(__reg("fp0") GLfloat u,__reg("fp1") GLfloat v ,__reg("a6") void *)="\tjsr\t-2064(a6)";
#define glEvalCoord2f(x1,x2) __glEvalCoord2f((x1),(x2),glBase)
void APIENTRY  __glEvalCoord2dv(__reg("a0") const GLdouble *u ,__reg("a6") void *)="\tjsr\t-2070(a6)";
#define glEvalCoord2dv(x1) __glEvalCoord2dv((x1),glBase)
void APIENTRY  __glEvalCoord2fv(__reg("a0") const GLfloat *u ,__reg("a6") void *)="\tjsr\t-2076(a6)";
#define glEvalCoord2fv(x1) __glEvalCoord2fv((x1),glBase)
void APIENTRY  __glMapGrid1d(__reg("d0") GLint un,__reg("fp0") GLdouble u1,__reg("fp1") GLdouble u2 ,__reg("a6") void *)="\tjsr\t-2082(a6)";
#define glMapGrid1d(x1,x2,x3) __glMapGrid1d((x1),(x2),(x3),glBase)
void APIENTRY  __glMapGrid1f(__reg("d0") GLint un,__reg("fp0") GLfloat u1,__reg("fp1") GLfloat u2 ,__reg("a6") void *)="\tjsr\t-2088(a6)";
#define glMapGrid1f(x1,x2,x3) __glMapGrid1f((x1),(x2),(x3),glBase)
void APIENTRY  __glMapGrid2d(__reg("d0") GLint un,__reg("fp0") GLdouble u1,__reg("fp1") GLdouble u2,__reg("d1") GLint vn,__reg("fp2") GLdouble v1,__reg("fp3") GLdouble v2 ,__reg("a6") void *)="\tjsr\t-2094(a6)";
#define glMapGrid2d(x1,x2,x3,x4,x5,x6) __glMapGrid2d((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glMapGrid2f(__reg("d0") GLint un,__reg("fp0") GLfloat u1,__reg("fp1") GLfloat u2,__reg("d1") GLint vn,__reg("fp2") GLfloat v1,__reg("fp3") GLfloat v2 ,__reg("a6") void *)="\tjsr\t-2100(a6)";
#define glMapGrid2f(x1,x2,x3,x4,x5,x6) __glMapGrid2f((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glEvalPoint1(__reg("d0") GLint i ,__reg("a6") void *)="\tjsr\t-2106(a6)";
#define glEvalPoint1(x1) __glEvalPoint1((x1),glBase)
void APIENTRY  __glEvalPoint2(__reg("d0") GLint i,__reg("d1") GLint j ,__reg("a6") void *)="\tjsr\t-2112(a6)";
#define glEvalPoint2(x1,x2) __glEvalPoint2((x1),(x2),glBase)
void APIENTRY  __glEvalMesh1(__reg("d0") GLenum mode,__reg("d1") GLint i1,__reg("d2") GLint i2 ,__reg("a6") void *)="\tjsr\t-2118(a6)";
#define glEvalMesh1(x1,x2,x3) __glEvalMesh1((x1),(x2),(x3),glBase)
void APIENTRY  __glEvalMesh2(__reg("d0") GLenum mode,__reg("d1") GLint i1,__reg("d2") GLint i2,__reg("d3") GLint j1,__reg("d4") GLint j2 ,__reg("a6") void *)="\tjsr\t-2124(a6)";
#define glEvalMesh2(x1,x2,x3,x4,x5) __glEvalMesh2((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glFogf(__reg("d0") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-2130(a6)";
#define glFogf(x1,x2) __glFogf((x1),(x2),glBase)
void APIENTRY  __glFogi(__reg("d0") GLenum pname,__reg("d1") GLint param ,__reg("a6") void *)="\tjsr\t-2136(a6)";
#define glFogi(x1,x2) __glFogi((x1),(x2),glBase)
void APIENTRY  __glFogfv(__reg("d0") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-2142(a6)";
#define glFogfv(x1,x2) __glFogfv((x1),(x2),glBase)
void APIENTRY  __glFogiv(__reg("d0") GLenum pname,__reg("a0") const GLint *params ,__reg("a6") void *)="\tjsr\t-2148(a6)";
#define glFogiv(x1,x2) __glFogiv((x1),(x2),glBase)
void APIENTRY  __glFeedbackBuffer(__reg("d0") GLsizei size,__reg("d1") GLenum type,__reg("a0") GLfloat *buffer ,__reg("a6") void *)="\tjsr\t-2154(a6)";
#define glFeedbackBuffer(x1,x2,x3) __glFeedbackBuffer((x1),(x2),(x3),glBase)
void APIENTRY  __glPassThrough(__reg("fp0") GLfloat token ,__reg("a6") void *)="\tjsr\t-2160(a6)";
#define glPassThrough(x1) __glPassThrough((x1),glBase)
void APIENTRY  __glSelectBuffer(__reg("d0") GLsizei size,__reg("a0") GLuint *buffer ,__reg("a6") void *)="\tjsr\t-2166(a6)";
#define glSelectBuffer(x1,x2) __glSelectBuffer((x1),(x2),glBase)
void APIENTRY  __glInitNames(__reg("a6") void *)="\tjsr\t-2172(a6)";
#define glInitNames() __glInitNames(glBase)
void APIENTRY  __glLoadName(__reg("d0") GLuint name ,__reg("a6") void *)="\tjsr\t-2178(a6)";
#define glLoadName(x1) __glLoadName((x1),glBase)
void APIENTRY  __glPushName(__reg("d0") GLuint name ,__reg("a6") void *)="\tjsr\t-2184(a6)";
#define glPushName(x1) __glPushName((x1),glBase)
void APIENTRY  __glPopName(__reg("a6") void *)="\tjsr\t-2190(a6)";
#define glPopName() __glPopName(glBase)
void APIENTRY  __glBlendEquationEXT(__reg("d0") GLenum mode ,__reg("a6") void *)="\tjsr\t-2196(a6)";
#define glBlendEquationEXT(x1) __glBlendEquationEXT((x1),glBase)
void APIENTRY  __glBlendColorEXT(__reg("fp0") GLclampf red,__reg("fp1") GLclampf green,__reg("fp2") GLclampf blue,__reg("fp3") GLclampf alpha ,__reg("a6") void *)="\tjsr\t-2202(a6)";
#define glBlendColorEXT(x1,x2,x3,x4) __glBlendColorEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glPolygonOffsetEXT(__reg("fp0") GLfloat factor,__reg("fp1") GLfloat bias ,__reg("a6") void *)="\tjsr\t-2208(a6)";
#define glPolygonOffsetEXT(x1,x2) __glPolygonOffsetEXT((x1),(x2),glBase)
void APIENTRY  __glVertexPointerEXT(__reg("d0") GLint size,__reg("d1") GLenum type,__reg("d2") GLsizei stride,__reg("d3") GLsizei count,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-2214(a6)";
#define glVertexPointerEXT(x1,x2,x3,x4,x5) __glVertexPointerEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glNormalPointerEXT(__reg("d0") GLenum type,__reg("d1") GLsizei stride,__reg("d2") GLsizei count,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-2220(a6)";
#define glNormalPointerEXT(x1,x2,x3,x4) __glNormalPointerEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glColorPointerEXT(__reg("d0") GLint size,__reg("d1") GLenum type,__reg("d2") GLsizei stride,__reg("d3") GLsizei count,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-2226(a6)";
#define glColorPointerEXT(x1,x2,x3,x4,x5) __glColorPointerEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glIndexPointerEXT(__reg("d0") GLenum type,__reg("d1") GLsizei stride,__reg("d2") GLsizei count,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-2232(a6)";
#define glIndexPointerEXT(x1,x2,x3,x4) __glIndexPointerEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glTexCoordPointerEXT(__reg("d0") GLint size,__reg("d1") GLenum type,__reg("d2") GLsizei stride,__reg("d3") GLsizei count,__reg("a0") const GLvoid *ptr ,__reg("a6") void *)="\tjsr\t-2238(a6)";
#define glTexCoordPointerEXT(x1,x2,x3,x4,x5) __glTexCoordPointerEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glEdgeFlagPointerEXT(__reg("d0") GLsizei stride,__reg("d1") GLsizei count,__reg("a0") const GLboolean *ptr ,__reg("a6") void *)="\tjsr\t-2244(a6)";
#define glEdgeFlagPointerEXT(x1,x2,x3) __glEdgeFlagPointerEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glGetPointervEXT(__reg("d0") GLenum pname,__reg("a0") void **params ,__reg("a6") void *)="\tjsr\t-2250(a6)";
#define glGetPointervEXT(x1,x2) __glGetPointervEXT((x1),(x2),glBase)
void APIENTRY  __glArrayElementEXT(__reg("d0") GLint i ,__reg("a6") void *)="\tjsr\t-2256(a6)";
#define glArrayElementEXT(x1) __glArrayElementEXT((x1),glBase)
void APIENTRY  __glDrawArraysEXT(__reg("d0") GLenum mode,__reg("d1") GLint first,__reg("d2") GLsizei count ,__reg("a6") void *)="\tjsr\t-2262(a6)";
#define glDrawArraysEXT(x1,x2,x3) __glDrawArraysEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glGenTexturesEXT(__reg("d0") GLsizei n,__reg("a0") GLuint *textures ,__reg("a6") void *)="\tjsr\t-2268(a6)";
#define glGenTexturesEXT(x1,x2) __glGenTexturesEXT((x1),(x2),glBase)
void APIENTRY  __glDeleteTexturesEXT(__reg("d0") GLsizei n,__reg("a0") const GLuint *textures,__reg("a6") void *)="\tjsr\t-2274(a6)";
#define glDeleteTexturesEXT(x1,x2) __glDeleteTexturesEXT((x1),(x2),glBase)
void APIENTRY  __glBindTextureEXT(__reg("d0") GLenum target,__reg("d1") GLuint texture ,__reg("a6") void *)="\tjsr\t-2280(a6)";
#define glBindTextureEXT(x1,x2) __glBindTextureEXT((x1),(x2),glBase)
void APIENTRY  __glPrioritizeTexturesEXT(__reg("d0") GLsizei n,__reg("a0") const GLuint *textures,__reg("a1") const GLclampf *priorities ,__reg("a6") void *)="\tjsr\t-2286(a6)";
#define glPrioritizeTexturesEXT(x1,x2,x3) __glPrioritizeTexturesEXT((x1),(x2),(x3),glBase)
GLboolean APIENTRY  __glAreTexturesResidentEXT(__reg("d0") GLsizei n,__reg("a0") const GLuint *textures,__reg("a1") GLboolean *residences ,__reg("a6") void *)="\tjsr\t-2292(a6)";
#define glAreTexturesResidentEXT(x1,x2,x3) __glAreTexturesResidentEXT((x1),(x2),(x3),glBase)
GLboolean APIENTRY  __glIsTextureEXT(__reg("d0") GLuint texture ,__reg("a6") void *)="\tjsr\t-2298(a6)";
#define glIsTextureEXT(x1) __glIsTextureEXT((x1),glBase)
/**void APIENTRY  __glTexImage3DEXT(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum internalFormat,__reg("d3") GLsizei width,__reg("d4") GLsizei height,__reg("d5") GLsizei depth,__reg("d6") GLint border,__reg("d7") GLenum format,__reg("a0") GLenum type,__reg("a1") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-2304(a6)";*/
/**#define glTexImage3DEXT(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10) __glTexImage3DEXT((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),glBase)*/
/**void APIENTRY  __glTexSubImage3DEXT(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint yoffset,__reg("d4") GLint zoffset,__reg("d5") GLsizei width,__reg("d6") GLsizei height,__reg("d7") GLsizei depth,__reg("a0") GLenum format,__reg("a1") GLenum type,__reg("a2") const GLvoid *pixels,__reg("a6") void *)="\tjsr\t-2310(a6)";*/
/**#define glTexSubImage3DEXT(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11) __glTexSubImage3DEXT((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),(x11),glBase)*/
/**void APIENTRY  __glCopyTexSubImage3DEXT(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint yoffset,__reg("d4") GLint zoffset,__reg("d5") GLint x,__reg("d6") GLint y,__reg("d7") GLsizei width,__reg("a0") GLsizei height ,__reg("a6") void *)="\tjsr\t-2316(a6)";*/
/**#define glCopyTexSubImage3DEXT(x1,x2,x3,x4,x5,x6,x7,x8,x9) __glCopyTexSubImage3DEXT((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),glBase)*/
void APIENTRY  __glColorTableEXT(__reg("d0") GLenum target,__reg("d1") GLenum internalformat,__reg("d2") GLsizei width,__reg("d3") GLenum format,__reg("d4") GLenum type,__reg("a0") const GLvoid *table ,__reg("a6") void *)="\tjsr\t-2322(a6)";
#define glColorTableEXT(x1,x2,x3,x4,x5,x6) __glColorTableEXT((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glColorSubTableEXT(__reg("d0") GLenum target,__reg("d1") GLsizei start,__reg("d2") GLsizei count,__reg("d3") GLenum format,__reg("d4") GLenum type,__reg("a0") const GLvoid *data ,__reg("a6") void *)="\tjsr\t-2328(a6)";
#define glColorSubTableEXT(x1,x2,x3,x4,x5,x6) __glColorSubTableEXT((x1),(x2),(x3),(x4),(x5),(x6),glBase)
void APIENTRY  __glGetColorTableEXT(__reg("d0") GLenum target,__reg("d1") GLenum format,__reg("d2") GLenum type,__reg("a0") GLvoid *table ,__reg("a6") void *)="\tjsr\t-2334(a6)";
#define glGetColorTableEXT(x1,x2,x3,x4) __glGetColorTableEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glGetColorTableParameterfvEXT(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") GLfloat *params ,__reg("a6") void *)="\tjsr\t-2340(a6)";
#define glGetColorTableParameterfvEXT(x1,x2,x3) __glGetColorTableParameterfvEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glGetColorTableParameterivEXT(__reg("d0") GLenum target,__reg("d1") GLenum pname,__reg("a0") GLint *params ,__reg("a6") void *)="\tjsr\t-2346(a6)";
#define glGetColorTableParameterivEXT(x1,x2,x3) __glGetColorTableParameterivEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord1dSGIS(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("a6") void *)="\tjsr\t-2352(a6)";
#define glMultiTexCoord1dSGIS(x1,x2) __glMultiTexCoord1dSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1dvSGIS(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2358(a6)";
#define glMultiTexCoord1dvSGIS(x1,x2) __glMultiTexCoord1dvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1fSGIS(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("a6") void *)="\tjsr\t-2364(a6)";
#define glMultiTexCoord1fSGIS(x1,x2) __glMultiTexCoord1fSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1fvSGIS(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2370(a6)";
#define glMultiTexCoord1fvSGIS(x1,x2) __glMultiTexCoord1fvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1iSGIS(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("a6") void *)="\tjsr\t-2376(a6)";
#define glMultiTexCoord1iSGIS(x1,x2) __glMultiTexCoord1iSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1ivSGIS(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2382(a6)";
#define glMultiTexCoord1ivSGIS(x1,x2) __glMultiTexCoord1ivSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1sSGIS(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("a6") void *)="\tjsr\t-2388(a6)";
#define glMultiTexCoord1sSGIS(x1,x2) __glMultiTexCoord1sSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1svSGIS(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2394(a6)";
#define glMultiTexCoord1svSGIS(x1,x2) __glMultiTexCoord1svSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2dSGIS(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("a6") void *)="\tjsr\t-2400(a6)";
#define glMultiTexCoord2dSGIS(x1,x2,x3) __glMultiTexCoord2dSGIS((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2dvSGIS(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2406(a6)";
#define glMultiTexCoord2dvSGIS(x1,x2) __glMultiTexCoord2dvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2fSGIS(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("a6") void *)="\tjsr\t-2412(a6)";
#define glMultiTexCoord2fSGIS(x1,x2,x3) __glMultiTexCoord2fSGIS((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2fvSGIS(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2418(a6)";
#define glMultiTexCoord2fvSGIS(x1,x2) __glMultiTexCoord2fvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2iSGIS(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("d2") GLint t,__reg("a6") void *)="\tjsr\t-2424(a6)";
#define glMultiTexCoord2iSGIS(x1,x2,x3) __glMultiTexCoord2iSGIS((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2ivSGIS(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2430(a6)";
#define glMultiTexCoord2ivSGIS(x1,x2) __glMultiTexCoord2ivSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2sSGIS(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("d2") GLshort t,__reg("a6") void *)="\tjsr\t-2436(a6)";
#define glMultiTexCoord2sSGIS(x1,x2,x3) __glMultiTexCoord2sSGIS((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2svSGIS(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2442(a6)";
#define glMultiTexCoord2svSGIS(x1,x2) __glMultiTexCoord2svSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3dSGIS(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("fp2") GLdouble r,__reg("a6") void *)="\tjsr\t-2448(a6)";
#define glMultiTexCoord3dSGIS(x1,x2,x3,x4) __glMultiTexCoord3dSGIS((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3dvSGIS(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2454(a6)";
#define glMultiTexCoord3dvSGIS(x1,x2) __glMultiTexCoord3dvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3fSGIS(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("fp2") GLfloat r,__reg("a6") void *)="\tjsr\t-2460(a6)";
#define glMultiTexCoord3fSGIS(x1,x2,x3,x4) __glMultiTexCoord3fSGIS((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3fvSGIS(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2466(a6)";
#define glMultiTexCoord3fvSGIS(x1,x2) __glMultiTexCoord3fvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3iSGIS(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("d2") GLint t,__reg("d3") GLint r,__reg("a6") void *)="\tjsr\t-2472(a6)";
#define glMultiTexCoord3iSGIS(x1,x2,x3,x4) __glMultiTexCoord3iSGIS((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3ivSGIS(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2478(a6)";
#define glMultiTexCoord3ivSGIS(x1,x2) __glMultiTexCoord3ivSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3sSGIS(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("d2") GLshort t,__reg("d3") GLshort r,__reg("a6") void *)="\tjsr\t-2484(a6)";
#define glMultiTexCoord3sSGIS(x1,x2,x3,x4) __glMultiTexCoord3sSGIS((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3svSGIS(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2490(a6)";
#define glMultiTexCoord3svSGIS(x1,x2) __glMultiTexCoord3svSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4dSGIS(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("fp2") GLdouble r,__reg("fp3") GLdouble q,__reg("a6") void *)="\tjsr\t-2496(a6)";
#define glMultiTexCoord4dSGIS(x1,x2,x3,x4,x5) __glMultiTexCoord4dSGIS((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4dvSGIS(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2502(a6)";
#define glMultiTexCoord4dvSGIS(x1,x2) __glMultiTexCoord4dvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4fSGIS(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("fp2") GLfloat r,__reg("fp3") GLfloat q,__reg("a6") void *)="\tjsr\t-2508(a6)";
#define glMultiTexCoord4fSGIS(x1,x2,x3,x4,x5) __glMultiTexCoord4fSGIS((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4fvSGIS(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2514(a6)";
#define glMultiTexCoord4fvSGIS(x1,x2) __glMultiTexCoord4fvSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4iSGIS(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("d2") GLint t,__reg("d3") GLint r,__reg("d4") GLint q,__reg("a6") void *)="\tjsr\t-2520(a6)";
#define glMultiTexCoord4iSGIS(x1,x2,x3,x4,x5) __glMultiTexCoord4iSGIS((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4ivSGIS(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2526(a6)";
#define glMultiTexCoord4ivSGIS(x1,x2) __glMultiTexCoord4ivSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4sSGIS(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("d2") GLshort t,__reg("d3") GLshort r,__reg("d4") GLshort q,__reg("a6") void *)="\tjsr\t-2532(a6)";
#define glMultiTexCoord4sSGIS(x1,x2,x3,x4,x5) __glMultiTexCoord4sSGIS((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4svSGIS(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2538(a6)";
#define glMultiTexCoord4svSGIS(x1,x2) __glMultiTexCoord4svSGIS((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoordPointerSGIS(__reg("d0") GLenum target,__reg("d1") GLint size,__reg("d2") GLenum type,__reg("d3") GLsizei stride,__reg("a0") const GLvoid *pointer,__reg("a6") void *)="\tjsr\t-2544(a6)";
#define glMultiTexCoordPointerSGIS(x1,x2,x3,x4,x5) __glMultiTexCoordPointerSGIS((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glSelectTextureSGIS(__reg("d0") GLenum target,__reg("a6") void *)="\tjsr\t-2550(a6)";
#define glSelectTextureSGIS(x1) __glSelectTextureSGIS((x1),glBase)
void APIENTRY  __glSelectTextureCoordSetSGIS(__reg("d0") GLenum target,__reg("a6") void *)="\tjsr\t-2556(a6)";
#define glSelectTextureCoordSetSGIS(x1) __glSelectTextureCoordSetSGIS((x1),glBase)
void APIENTRY  __glMultiTexCoord1dEXT(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("a6") void *)="\tjsr\t-2562(a6)";
#define glMultiTexCoord1dEXT(x1,x2) __glMultiTexCoord1dEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1dvEXT(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2568(a6)";
#define glMultiTexCoord1dvEXT(x1,x2) __glMultiTexCoord1dvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1fEXT(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("a6") void *)="\tjsr\t-2574(a6)";
#define glMultiTexCoord1fEXT(x1,x2) __glMultiTexCoord1fEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1fvEXT(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2580(a6)";
#define glMultiTexCoord1fvEXT(x1,x2) __glMultiTexCoord1fvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1iEXT(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("a6") void *)="\tjsr\t-2586(a6)";
#define glMultiTexCoord1iEXT(x1,x2) __glMultiTexCoord1iEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1ivEXT(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2592(a6)";
#define glMultiTexCoord1ivEXT(x1,x2) __glMultiTexCoord1ivEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1sEXT(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("a6") void *)="\tjsr\t-2598(a6)";
#define glMultiTexCoord1sEXT(x1,x2) __glMultiTexCoord1sEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord1svEXT(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2604(a6)";
#define glMultiTexCoord1svEXT(x1,x2) __glMultiTexCoord1svEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2dEXT(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("a6") void *)="\tjsr\t-2610(a6)";
#define glMultiTexCoord2dEXT(x1,x2,x3) __glMultiTexCoord2dEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2dvEXT(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2616(a6)";
#define glMultiTexCoord2dvEXT(x1,x2) __glMultiTexCoord2dvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2fEXT(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("a6") void *)="\tjsr\t-2622(a6)";
#define glMultiTexCoord2fEXT(x1,x2,x3) __glMultiTexCoord2fEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2fvEXT(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2628(a6)";
#define glMultiTexCoord2fvEXT(x1,x2) __glMultiTexCoord2fvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2iEXT(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("d2") GLint t,__reg("a6") void *)="\tjsr\t-2634(a6)";
#define glMultiTexCoord2iEXT(x1,x2,x3) __glMultiTexCoord2iEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2ivEXT(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2640(a6)";
#define glMultiTexCoord2ivEXT(x1,x2) __glMultiTexCoord2ivEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord2sEXT(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("d2") GLshort t,__reg("a6") void *)="\tjsr\t-2646(a6)";
#define glMultiTexCoord2sEXT(x1,x2,x3) __glMultiTexCoord2sEXT((x1),(x2),(x3),glBase)
void APIENTRY  __glMultiTexCoord2svEXT(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2652(a6)";
#define glMultiTexCoord2svEXT(x1,x2) __glMultiTexCoord2svEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3dEXT(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("fp2") GLdouble r,__reg("a6") void *)="\tjsr\t-2658(a6)";
#define glMultiTexCoord3dEXT(x1,x2,x3,x4) __glMultiTexCoord3dEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3dvEXT(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2664(a6)";
#define glMultiTexCoord3dvEXT(x1,x2) __glMultiTexCoord3dvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3fEXT(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("fp2") GLfloat r,__reg("a6") void *)="\tjsr\t-2670(a6)";
#define glMultiTexCoord3fEXT(x1,x2,x3,x4) __glMultiTexCoord3fEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3fvEXT(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2676(a6)";
#define glMultiTexCoord3fvEXT(x1,x2) __glMultiTexCoord3fvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3iEXT(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("d2") GLint t,__reg("d3") GLint r,__reg("a6") void *)="\tjsr\t-2682(a6)";
#define glMultiTexCoord3iEXT(x1,x2,x3,x4) __glMultiTexCoord3iEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3ivEXT(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2688(a6)";
#define glMultiTexCoord3ivEXT(x1,x2) __glMultiTexCoord3ivEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord3sEXT(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("d2") GLshort t,__reg("d3") GLshort r,__reg("a6") void *)="\tjsr\t-2694(a6)";
#define glMultiTexCoord3sEXT(x1,x2,x3,x4) __glMultiTexCoord3sEXT((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glMultiTexCoord3svEXT(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2700(a6)";
#define glMultiTexCoord3svEXT(x1,x2) __glMultiTexCoord3svEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4dEXT(__reg("d0") GLenum target,__reg("fp0") GLdouble s,__reg("fp1") GLdouble t,__reg("fp2") GLdouble r,__reg("fp3") GLdouble q,__reg("a6") void *)="\tjsr\t-2706(a6)";
#define glMultiTexCoord4dEXT(x1,x2,x3,x4,x5) __glMultiTexCoord4dEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4dvEXT(__reg("d0") GLenum target,__reg("a0") const GLdouble *v,__reg("a6") void *)="\tjsr\t-2712(a6)";
#define glMultiTexCoord4dvEXT(x1,x2) __glMultiTexCoord4dvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4fEXT(__reg("d0") GLenum target,__reg("fp0") GLfloat s,__reg("fp1") GLfloat t,__reg("fp2") GLfloat r,__reg("fp3") GLfloat q,__reg("a6") void *)="\tjsr\t-2718(a6)";
#define glMultiTexCoord4fEXT(x1,x2,x3,x4,x5) __glMultiTexCoord4fEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4fvEXT(__reg("d0") GLenum target,__reg("a0") const GLfloat *v,__reg("a6") void *)="\tjsr\t-2724(a6)";
#define glMultiTexCoord4fvEXT(x1,x2) __glMultiTexCoord4fvEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4iEXT(__reg("d0") GLenum target,__reg("d1") GLint s,__reg("d2") GLint t,__reg("d3") GLint r,__reg("d4") GLint q,__reg("a6") void *)="\tjsr\t-2730(a6)";
#define glMultiTexCoord4iEXT(x1,x2,x3,x4,x5) __glMultiTexCoord4iEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4ivEXT(__reg("d0") GLenum target,__reg("a0") const GLint *v,__reg("a6") void *)="\tjsr\t-2736(a6)";
#define glMultiTexCoord4ivEXT(x1,x2) __glMultiTexCoord4ivEXT((x1),(x2),glBase)
void APIENTRY  __glMultiTexCoord4sEXT(__reg("d0") GLenum target,__reg("d1") GLshort s,__reg("d2") GLshort t,__reg("d3") GLshort r,__reg("d4") GLshort q,__reg("a6") void *)="\tjsr\t-2742(a6)";
#define glMultiTexCoord4sEXT(x1,x2,x3,x4,x5) __glMultiTexCoord4sEXT((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __glMultiTexCoord4svEXT(__reg("d0") GLenum target,__reg("a0") const GLshort *v,__reg("a6") void *)="\tjsr\t-2748(a6)";
#define glMultiTexCoord4svEXT(x1,x2) __glMultiTexCoord4svEXT((x1),(x2),glBase)
void APIENTRY  __glInterleavedTextureCoordSetsEXT(__reg("d0") GLint factor ,__reg("a6") void *)="\tjsr\t-2754(a6)";
#define glInterleavedTextureCoordSetsEXT(x1) __glInterleavedTextureCoordSetsEXT((x1),glBase)
void APIENTRY  __glSelectTextureEXT(__reg("d0") GLenum target ,__reg("a6") void *)="\tjsr\t-2760(a6)";
#define glSelectTextureEXT(x1) __glSelectTextureEXT((x1),glBase)
void APIENTRY  __glSelectTextureCoordSetEXT(__reg("d0") GLenum target ,__reg("a6") void *)="\tjsr\t-2766(a6)";
#define glSelectTextureCoordSetEXT(x1) __glSelectTextureCoordSetEXT((x1),glBase)
void APIENTRY  __glSelectTextureTransformEXT(__reg("d0") GLenum target ,__reg("a6") void *)="\tjsr\t-2772(a6)";
#define glSelectTextureTransformEXT(x1) __glSelectTextureTransformEXT((x1),glBase)
void APIENTRY  __glPointParameterfEXT(__reg("d0") GLenum pname,__reg("fp0") GLfloat param ,__reg("a6") void *)="\tjsr\t-2778(a6)";
#define glPointParameterfEXT(x1,x2) __glPointParameterfEXT((x1),(x2),glBase)
void APIENTRY  __glPointParameterfvEXT(__reg("d0") GLenum pname,__reg("a0") const GLfloat *params ,__reg("a6") void *)="\tjsr\t-2784(a6)";
#define glPointParameterfvEXT(x1,x2) __glPointParameterfvEXT((x1),(x2),glBase)
void APIENTRY  __glWindowPos2iMESA(__reg("d0") GLint x,__reg("d1") GLint y ,__reg("a6") void *)="\tjsr\t-2790(a6)";
#define glWindowPos2iMESA(x1,x2) __glWindowPos2iMESA((x1),(x2),glBase)
void APIENTRY  __glWindowPos2sMESA(__reg("d0") GLshort x,__reg("d1") GLshort y ,__reg("a6") void *)="\tjsr\t-2796(a6)";
#define glWindowPos2sMESA(x1,x2) __glWindowPos2sMESA((x1),(x2),glBase)
void APIENTRY  __glWindowPos2fMESA(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y ,__reg("a6") void *)="\tjsr\t-2802(a6)";
#define glWindowPos2fMESA(x1,x2) __glWindowPos2fMESA((x1),(x2),glBase)
void APIENTRY  __glWindowPos2dMESA(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y ,__reg("a6") void *)="\tjsr\t-2808(a6)";
#define glWindowPos2dMESA(x1,x2) __glWindowPos2dMESA((x1),(x2),glBase)
void APIENTRY  __glWindowPos2ivMESA(__reg("a0") const GLint *p ,__reg("a6") void *)="\tjsr\t-2814(a6)";
#define glWindowPos2ivMESA(x1) __glWindowPos2ivMESA((x1),glBase)
void APIENTRY  __glWindowPos2svMESA(__reg("a0") const GLshort *p ,__reg("a6") void *)="\tjsr\t-2820(a6)";
#define glWindowPos2svMESA(x1) __glWindowPos2svMESA((x1),glBase)
void APIENTRY  __glWindowPos2fvMESA(__reg("a0") const GLfloat *p ,__reg("a6") void *)="\tjsr\t-2826(a6)";
#define glWindowPos2fvMESA(x1) __glWindowPos2fvMESA((x1),glBase)
void APIENTRY  __glWindowPos2dvMESA(__reg("a0") const GLdouble *p ,__reg("a6") void *)="\tjsr\t-2832(a6)";
#define glWindowPos2dvMESA(x1) __glWindowPos2dvMESA((x1),glBase)
void APIENTRY  __glWindowPos3iMESA(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLint z ,__reg("a6") void *)="\tjsr\t-2838(a6)";
#define glWindowPos3iMESA(x1,x2,x3) __glWindowPos3iMESA((x1),(x2),(x3),glBase)
void APIENTRY  __glWindowPos3sMESA(__reg("d0") GLshort x,__reg("d1") GLshort y,__reg("d2") GLshort z ,__reg("a6") void *)="\tjsr\t-2844(a6)";
#define glWindowPos3sMESA(x1,x2,x3) __glWindowPos3sMESA((x1),(x2),(x3),glBase)
void APIENTRY  __glWindowPos3fMESA(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z ,__reg("a6") void *)="\tjsr\t-2850(a6)";
#define glWindowPos3fMESA(x1,x2,x3) __glWindowPos3fMESA((x1),(x2),(x3),glBase)
void APIENTRY  __glWindowPos3dMESA(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z ,__reg("a6") void *)="\tjsr\t-2856(a6)";
#define glWindowPos3dMESA(x1,x2,x3) __glWindowPos3dMESA((x1),(x2),(x3),glBase)
void APIENTRY  __glWindowPos3ivMESA(__reg("a0") const GLint *p ,__reg("a6") void *)="\tjsr\t-2862(a6)";
#define glWindowPos3ivMESA(x1) __glWindowPos3ivMESA((x1),glBase)
void APIENTRY  __glWindowPos3svMESA(__reg("a0") const GLshort *p ,__reg("a6") void *)="\tjsr\t-2868(a6)";
#define glWindowPos3svMESA(x1) __glWindowPos3svMESA((x1),glBase)
void APIENTRY  __glWindowPos3fvMESA(__reg("a0") const GLfloat *p ,__reg("a6") void *)="\tjsr\t-2874(a6)";
#define glWindowPos3fvMESA(x1) __glWindowPos3fvMESA((x1),glBase)
void APIENTRY  __glWindowPos3dvMESA(__reg("a0") const GLdouble *p ,__reg("a6") void *)="\tjsr\t-2880(a6)";
#define glWindowPos3dvMESA(x1) __glWindowPos3dvMESA((x1),glBase)
void APIENTRY  __glWindowPos4iMESA(__reg("d0") GLint x,__reg("d1") GLint y,__reg("d2") GLint z,__reg("d3") GLint w ,__reg("a6") void *)="\tjsr\t-2886(a6)";
#define glWindowPos4iMESA(x1,x2,x3,x4) __glWindowPos4iMESA((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glWindowPos4sMESA(__reg("d0") GLshort x,__reg("d1") GLshort y,__reg("d2") GLshort z,__reg("d3") GLshort w ,__reg("a6") void *)="\tjsr\t-2892(a6)";
#define glWindowPos4sMESA(x1,x2,x3,x4) __glWindowPos4sMESA((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glWindowPos4fMESA(__reg("fp0") GLfloat x,__reg("fp1") GLfloat y,__reg("fp2") GLfloat z,__reg("fp3") GLfloat w ,__reg("a6") void *)="\tjsr\t-2898(a6)";
#define glWindowPos4fMESA(x1,x2,x3,x4) __glWindowPos4fMESA((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glWindowPos4dMESA(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble z,__reg("fp3") GLdouble w,__reg("a6") void *)="\tjsr\t-2904(a6)";
#define glWindowPos4dMESA(x1,x2,x3,x4) __glWindowPos4dMESA((x1),(x2),(x3),(x4),glBase)
void APIENTRY  __glWindowPos4ivMESA(__reg("a0") const GLint *p ,__reg("a6") void *)="\tjsr\t-2910(a6)";
#define glWindowPos4ivMESA(x1) __glWindowPos4ivMESA((x1),glBase)
void APIENTRY  __glWindowPos4svMESA(__reg("a0") const GLshort *p ,__reg("a6") void *)="\tjsr\t-2916(a6)";
#define glWindowPos4svMESA(x1) __glWindowPos4svMESA((x1),glBase)
void APIENTRY  __glWindowPos4fvMESA(__reg("a0") const GLfloat *p ,__reg("a6") void *)="\tjsr\t-2922(a6)";
#define glWindowPos4fvMESA(x1) __glWindowPos4fvMESA((x1),glBase)
void APIENTRY  __glWindowPos4dvMESA(__reg("a0") const GLdouble *p ,__reg("a6") void *)="\tjsr\t-2928(a6)";
#define glWindowPos4dvMESA(x1) __glWindowPos4dvMESA((x1),glBase)
void APIENTRY  __glResizeBuffersMESA(__reg("a6") void *)="\tjsr\t-2934(a6)";
#define glResizeBuffersMESA() __glResizeBuffersMESA(glBase)
void APIENTRY  __glDrawRangeElements(__reg("d0") GLenum mode,__reg("d1") GLuint start,__reg("d2") GLuint end,__reg("d3") GLsizei count,__reg("d4") GLenum type,__reg("a0") const GLvoid *indices ,__reg("a6") void *)="\tjsr\t-2940(a6)";
#define glDrawRangeElements(x1,x2,x3,x4,x5,x6) __glDrawRangeElements((x1),(x2),(x3),(x4),(x5),(x6),glBase)
/**void APIENTRY  __glTexImage3D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLenum internalFormat,__reg("d3") GLsizei width,__reg("d4") GLsizei height,__reg("d5") GLsizei depth,__reg("d6") GLint border,__reg("d7") GLenum format,__reg("a0") GLenum type,__reg("a1") const GLvoid *pixels ,__reg("a6") void *)="\tjsr\t-2946(a6)";*/
/**#define glTexImage3D(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10) __glTexImage3D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),glBase)*/
/**void APIENTRY  __glTexSubImage3D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint yoffset,__reg("d4") GLint zoffset,__reg("d5") GLsizei width,__reg("d6") GLsizei height,__reg("d7") GLsizei depth,__reg("a0") GLenum format,__reg("a1") GLenum type,__reg("a2") const GLvoid *pixels,__reg("a6") void *)="\tjsr\t-2952(a6)";*/
/**#define glTexSubImage3D(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11) __glTexSubImage3D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),(x11),glBase)*/
/**void APIENTRY  __glCopyTexSubImage3D(__reg("d0") GLenum target,__reg("d1") GLint level,__reg("d2") GLint xoffset,__reg("d3") GLint yoffset,__reg("d4") GLint zoffset,__reg("d5") GLint x,__reg("d6") GLint y,__reg("d7") GLsizei width,__reg("a0") GLsizei height ,__reg("a6") void *)="\tjsr\t-2958(a6)";*/
/**#define glCopyTexSubImage3D(x1,x2,x3,x4,x5,x6,x7,x8,x9) __glCopyTexSubImage3D((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),glBase)*/
