/**void APIENTRY  __gluLookAt(__reg("fp0") GLdouble eyex,__reg("fp1") GLdouble eyey,__reg("fp2") GLdouble eyez,__reg("fp3") GLdouble centerx,__reg("fp4") GLdouble centery,__reg("fp5") GLdouble centerz,__reg("fp6") GLdouble upx,__reg("fp7") GLdouble upy,__reg("d0") GLdouble upz ,__reg("a6") void *)="\tjsr\t-36(a6)";*/
/**#define gluLookAt(x1,x2,x3,x4,x5,x6,x7,x8,x9) __gluLookAt((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),gluBase)*/
void APIENTRY  __gluOrtho2D(__reg("fp0") GLdouble left,__reg("fp1") GLdouble right,__reg("fp2") GLdouble bottom,__reg("fp3") GLdouble top ,__reg("a6") void *)="\tjsr\t-42(a6)";
#define gluOrtho2D(x1,x2,x3,x4) __gluOrtho2D((x1),(x2),(x3),(x4),gluBase)
void APIENTRY  __gluPerspective(__reg("fp0") GLdouble fovy,__reg("fp1") GLdouble aspect,__reg("fp2") GLdouble zNear,__reg("fp3") GLdouble zFar ,__reg("a6") void *)="\tjsr\t-48(a6)";
#define gluPerspective(x1,x2,x3,x4) __gluPerspective((x1),(x2),(x3),(x4),gluBase)
void APIENTRY  __gluPickMatrix(__reg("fp0") GLdouble x,__reg("fp1") GLdouble y,__reg("fp2") GLdouble width,__reg("fp3") GLdouble height,__reg("a0") const GLint *viewport,__reg("a6") void *)="\tjsr\t-54(a6)";
#define gluPickMatrix(x1,x2,x3,x4,x5) __gluPickMatrix((x1),(x2),(x3),(x4),(x5),gluBase)
/**GLint APIENTRY  __gluProject(__reg("fp0") GLdouble objx,__reg("fp1") GLdouble objy,__reg("fp2") GLdouble objz,__reg("a0") const GLdouble modelMatrix[16],__reg("a1") const GLdouble projMatrix[16],__reg("a2") const GLint viewport[4],__reg("a3") GLdouble *winx,__reg("d0") GLdouble *winy,__reg("d1") GLdouble *winz ,__reg("a6") void *)="\tjsr\t-60(a6)";*/
/**#define gluProject(x1,x2,x3,x4,x5,x6,x7,x8,x9) __gluProject((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),gluBase)*/
/**GLint APIENTRY  __gluUnProject(__reg("fp0") GLdouble winx,__reg("fp1") GLdouble winy,__reg("fp2") GLdouble winz,__reg("a0") const GLdouble modelMatrix[16],__reg("a1") const GLdouble projMatrix[16],__reg("a2") const GLint viewport[4],__reg("a3") GLdouble *objx,__reg("d0") GLdouble *objy,__reg("d1") GLdouble *objz ,__reg("a6") void *)="\tjsr\t-66(a6)";*/
/**#define gluUnProject(x1,x2,x3,x4,x5,x6,x7,x8,x9) __gluUnProject((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),gluBase)*/
const GLubyte* APIENTRY  __gluErrorString(__reg("d0") GLenum errorCode ,__reg("a6") void *)="\tjsr\t-72(a6)";
#define gluErrorString(x1) __gluErrorString((x1),gluBase)
/**GLint APIENTRY  __gluScaleImage(__reg("d0") GLenum format,__reg("d1") GLint widthin,__reg("d2") GLint heightin,__reg("d3") GLenum typein,__reg("a0") const void *datain,__reg("d4") GLint widthout,__reg("d5") GLint heightout,__reg("d6") GLenum typeout,__reg("a1") void *dataout ,__reg("a6") void *)="\tjsr\t-78(a6)";*/
/**#define gluScaleImage(x1,x2,x3,x4,x5,x6,x7,x8,x9) __gluScaleImage((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),gluBase)*/
GLint APIENTRY  __gluBuild1DMipmaps(__reg("d0") GLenum target,__reg("d1") GLint components,__reg("d2") GLint width,__reg("d3") GLenum format,__reg("d4") GLenum type,__reg("a0") const void *data ,__reg("a6") void *)="\tjsr\t-84(a6)";
#define gluBuild1DMipmaps(x1,x2,x3,x4,x5,x6) __gluBuild1DMipmaps((x1),(x2),(x3),(x4),(x5),(x6),gluBase)
GLint APIENTRY  __gluBuild2DMipmaps(__reg("d0") GLenum target,__reg("d1") GLint components,__reg("d2") GLint width,__reg("d3") GLint height,__reg("d4") GLenum format,__reg("d5") GLenum type,__reg("a0") const void *data ,__reg("a6") void *)="\tjsr\t-90(a6)";
#define gluBuild2DMipmaps(x1,x2,x3,x4,x5,x6,x7) __gluBuild2DMipmaps((x1),(x2),(x3),(x4),(x5),(x6),(x7),gluBase)
GLUquadricObj* APIENTRY  __gluNewQuadric(__reg("a6") void *)="\tjsr\t-96(a6)";
#define gluNewQuadric() __gluNewQuadric(gluBase)
void APIENTRY  __gluDeleteQuadric(__reg("a0") GLUquadricObj *state ,__reg("a6") void *)="\tjsr\t-102(a6)";
#define gluDeleteQuadric(x1) __gluDeleteQuadric((x1),gluBase)
void APIENTRY  __gluQuadricDrawStyle(__reg("a0") GLUquadricObj *quadObject,__reg("d0") GLenum drawStyle ,__reg("a6") void *)="\tjsr\t-108(a6)";
#define gluQuadricDrawStyle(x1,x2) __gluQuadricDrawStyle((x1),(x2),gluBase)
void APIENTRY  __gluQuadricOrientation(__reg("a0") GLUquadricObj *quadObject,__reg("d0") GLenum orientation ,__reg("a6") void *)="\tjsr\t-114(a6)";
#define gluQuadricOrientation(x1,x2) __gluQuadricOrientation((x1),(x2),gluBase)
void APIENTRY  __gluQuadricNormals(__reg("a0") GLUquadricObj *quadObject,__reg("d0") GLenum normals ,__reg("a6") void *)="\tjsr\t-120(a6)";
#define gluQuadricNormals(x1,x2) __gluQuadricNormals((x1),(x2),gluBase)
void APIENTRY  __gluQuadricTexture(__reg("a0") GLUquadricObj *quadObject,__reg("d0") GLboolean textureCoords ,__reg("a6") void *)="\tjsr\t-126(a6)";
#define gluQuadricTexture(x1,x2) __gluQuadricTexture((x1),(x2),gluBase)
void APIENTRY  __gluQuadricCallback(__reg("a0") GLUquadricObj *qobj,__reg("d0") GLenum which,__reg("a1") void (CALLBACK *fn)() ,__reg("a6") void *)="\tjsr\t-132(a6)";
#define gluQuadricCallback(x1,x2,x3) __gluQuadricCallback((x1),(x2),(x3),gluBase)
void APIENTRY  __gluCylinder(__reg("a0") GLUquadricObj *qobj,__reg("fp0") GLdouble baseRadius,__reg("fp1") GLdouble topRadius,__reg("fp2") GLdouble height,__reg("d0") GLint slices,__reg("d1") GLint stacks ,__reg("a6") void *)="\tjsr\t-138(a6)";
#define gluCylinder(x1,x2,x3,x4,x5,x6) __gluCylinder((x1),(x2),(x3),(x4),(x5),(x6),gluBase)
void APIENTRY  __gluSphere(__reg("a0") GLUquadricObj *qobj,__reg("fp0") GLdouble radius,__reg("d0") GLint slices,__reg("d1") GLint stacks ,__reg("a6") void *)="\tjsr\t-144(a6)";
#define gluSphere(x1,x2,x3,x4) __gluSphere((x1),(x2),(x3),(x4),gluBase)
void APIENTRY  __gluDisk(__reg("a0") GLUquadricObj *qobj,__reg("fp0") GLdouble innerRadius,__reg("fp1") GLdouble outerRadius,__reg("d0") GLint slices,__reg("d1") GLint loops ,__reg("a6") void *)="\tjsr\t-150(a6)";
#define gluDisk(x1,x2,x3,x4,x5) __gluDisk((x1),(x2),(x3),(x4),(x5),gluBase)
void APIENTRY  __gluPartialDisk(__reg("a0") GLUquadricObj *qobj,__reg("fp0") GLdouble innerRadius,__reg("fp1") GLdouble outerRadius,__reg("d0") GLint slices,__reg("d1") GLint loops,__reg("fp2") GLdouble startAngle,__reg("fp3") GLdouble sweepAngle ,__reg("a6") void *)="\tjsr\t-156(a6)";
#define gluPartialDisk(x1,x2,x3,x4,x5,x6,x7) __gluPartialDisk((x1),(x2),(x3),(x4),(x5),(x6),(x7),gluBase)
GLUnurbsObj* APIENTRY  __gluNewNurbsRenderer(__reg("a6") void *)="\tjsr\t-162(a6)";
#define gluNewNurbsRenderer() __gluNewNurbsRenderer(gluBase)
void APIENTRY  __gluDeleteNurbsRenderer(__reg("a0") GLUnurbsObj *nobj ,__reg("a6") void *)="\tjsr\t-168(a6)";
#define gluDeleteNurbsRenderer(x1) __gluDeleteNurbsRenderer((x1),gluBase)
void APIENTRY  __gluLoadSamplingMatrices(__reg("a0") GLUnurbsObj *nobj,__reg("a1") const GLfloat *modelMatrix,__reg("a2") const GLfloat *projMatrix,__reg("a3") const GLint *viewport,__reg("a6") void *)="\tjsr\t-174(a6)";
#define gluLoadSamplingMatrices(x1,x2,x3,x4) __gluLoadSamplingMatrices((x1),(x2),(x3),(x4),gluBase)
void APIENTRY  __gluNurbsProperty(__reg("a0") GLUnurbsObj *nobj,__reg("d0") GLenum property,__reg("fp0") GLfloat value ,__reg("a6") void *)="\tjsr\t-180(a6)";
#define gluNurbsProperty(x1,x2,x3) __gluNurbsProperty((x1),(x2),(x3),gluBase)
void APIENTRY  __gluGetNurbsProperty(__reg("a0") GLUnurbsObj *nobj,__reg("d0") GLenum property,__reg("a1") GLfloat *value ,__reg("a6") void *)="\tjsr\t-186(a6)";
#define gluGetNurbsProperty(x1,x2,x3) __gluGetNurbsProperty((x1),(x2),(x3),gluBase)
void APIENTRY  __gluBeginCurve(__reg("a0") GLUnurbsObj *nobj ,__reg("a6") void *)="\tjsr\t-192(a6)";
#define gluBeginCurve(x1) __gluBeginCurve((x1),gluBase)
void APIENTRY  __gluEndCurve(__reg("a0") GLUnurbsObj * nobj ,__reg("a6") void *)="\tjsr\t-198(a6)";
#define gluEndCurve(x1) __gluEndCurve((x1),gluBase)
void APIENTRY  __gluNurbsCurve(__reg("a0") GLUnurbsObj *nobj,__reg("d0") GLint nknots,__reg("a1") GLfloat *knot,__reg("d1") GLint stride,__reg("a2") GLfloat *ctlarray,__reg("d2") GLint order,__reg("d3") GLenum type,__reg("a6") void *)="\tjsr\t-204(a6)";
#define gluNurbsCurve(x1,x2,x3,x4,x5,x6,x7) __gluNurbsCurve((x1),(x2),(x3),(x4),(x5),(x6),(x7),gluBase)
void APIENTRY  __gluBeginSurface(__reg("a0") GLUnurbsObj *nobj ,__reg("a6") void *)="\tjsr\t-210(a6)";
#define gluBeginSurface(x1) __gluBeginSurface((x1),gluBase)
void APIENTRY  __gluEndSurface(__reg("a0") GLUnurbsObj * nobj ,__reg("a6") void *)="\tjsr\t-216(a6)";
#define gluEndSurface(x1) __gluEndSurface((x1),gluBase)
/**void APIENTRY  __gluNurbsSurface(__reg("a0") GLUnurbsObj *nobj,__reg("d0") GLint sknot_count,__reg("a1") GLfloat *sknot,__reg("d1") GLint tknot_count,__reg("a2") GLfloat *tknot,__reg("d2") GLint s_stride,__reg("d3") GLint t_stride,__reg("a3") GLfloat *ctlarray,__reg("d4") GLint sorder,__reg("d5") GLint torder,__reg("d6") GLenum type,__reg("a6") void *)="\tjsr\t-222(a6)";*/
/**#define gluNurbsSurface(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11) __gluNurbsSurface((x1),(x2),(x3),(x4),(x5),(x6),(x7),(x8),(x9),(x10),(x11),gluBase)*/
void APIENTRY  __gluBeginTrim(__reg("a0") GLUnurbsObj *nobj ,__reg("a6") void *)="\tjsr\t-228(a6)";
#define gluBeginTrim(x1) __gluBeginTrim((x1),gluBase)
void APIENTRY  __gluEndTrim(__reg("a0") GLUnurbsObj *nobj ,__reg("a6") void *)="\tjsr\t-234(a6)";
#define gluEndTrim(x1) __gluEndTrim((x1),gluBase)
void APIENTRY  __gluPwlCurve(__reg("a0") GLUnurbsObj *nobj,__reg("d0") GLint count,__reg("a1") GLfloat *array,__reg("d1") GLint stride,__reg("d2") GLenum type ,__reg("a6") void *)="\tjsr\t-240(a6)";
#define gluPwlCurve(x1,x2,x3,x4,x5) __gluPwlCurve((x1),(x2),(x3),(x4),(x5),gluBase)
void APIENTRY  __gluNurbsCallback(__reg("a0") GLUnurbsObj *nobj,__reg("d0") GLenum which,__reg("a1") void (CALLBACK *fn)() ,__reg("a6") void *)="\tjsr\t-246(a6)";
#define gluNurbsCallback(x1,x2,x3) __gluNurbsCallback((x1),(x2),(x3),gluBase)
GLUtriangulatorObj* APIENTRY  __gluNewTess(__reg("a6") void *)="\tjsr\t-252(a6)";
#define gluNewTess() __gluNewTess(gluBase)
void APIENTRY  __gluTessCallback(__reg("a0") GLUtriangulatorObj *tobj,__reg("d0") GLenum which,__reg("a1") void (CALLBACK *fn)() ,__reg("a6") void *)="\tjsr\t-258(a6)";
#define gluTessCallback(x1,x2,x3) __gluTessCallback((x1),(x2),(x3),gluBase)
void APIENTRY  __gluDeleteTess(__reg("a0") GLUtriangulatorObj *tobj ,__reg("a6") void *)="\tjsr\t-264(a6)";
#define gluDeleteTess(x1) __gluDeleteTess((x1),gluBase)
void APIENTRY  __gluBeginPolygon(__reg("a0") GLUtriangulatorObj *tobj ,__reg("a6") void *)="\tjsr\t-270(a6)";
#define gluBeginPolygon(x1) __gluBeginPolygon((x1),gluBase)
void APIENTRY  __gluEndPolygon(__reg("a0") GLUtriangulatorObj *tobj ,__reg("a6") void *)="\tjsr\t-276(a6)";
#define gluEndPolygon(x1) __gluEndPolygon((x1),gluBase)
void APIENTRY  __gluNextContour(__reg("a0") GLUtriangulatorObj *tobj,__reg("d0") GLenum type ,__reg("a6") void *)="\tjsr\t-282(a6)";
#define gluNextContour(x1,x2) __gluNextContour((x1),(x2),gluBase)
void APIENTRY  __gluTessVertex(__reg("a0") GLUtriangulatorObj *tobj,__reg("a1") GLdouble *v,__reg("a2") void *data ,__reg("a6") void *)="\tjsr\t-288(a6)";
#define gluTessVertex(x1,x2,x3) __gluTessVertex((x1),(x2),(x3),gluBase)
const GLubyte* APIENTRY  __gluGetString(__reg("d0") GLenum name ,__reg("a6") void *)="\tjsr\t-294(a6)";
#define gluGetString(x1) __gluGetString((x1),gluBase)
