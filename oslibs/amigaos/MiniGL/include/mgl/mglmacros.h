#ifndef MGL_MACROS_H
#define MGL_MACROS_H

#ifndef NO_MGLMACROS

/*********************************************************

Marcos for in-application vertexbuffer construction.

- Should be included after gl.h.

Limitations: (for the sake of performance)
- Normals not supported
- glBegin/glEnd construct can not be spred across objects
- Texture units are not validated with multitexturing
- No color-clamping:
  floats should always be in the range 0.0 - 1.0

Note: The Triangle Drawfn is decided in glEnd after checking how many verts entered the pipeline.

**********************************************************/


extern void d_DrawPoints		(GLcontext context);
extern void d_DrawLines		(GLcontext context);
extern void d_DrawLineStrip		(GLcontext context);
extern void d_DrawTriangleFan		(GLcontext context);
extern void d_DrawTriangleStrip	(GLcontext context);
extern void d_DrawQuads		(GLcontext context);
extern void d_DrawNormalPoly		(GLcontext context);
extern void d_DrawSmoothPoly		(GLcontext context);
extern void d_DrawMtexPoly		(GLcontext context);
extern void d_DrawQuadStrip		(GLcontext context);
extern void d_DrawFlat		(GLcontext context);

extern GLcontext mini_CurrentContext;

#define CC mini_CurrentContext

static MGLVertex *mglvert = NULL;
#define MGL_UBTOF (1.f/255.f)


#undef glBegin
#undef glTexCoord4f
#undef glTexCoord2f
#undef glTexCoord4fv
#undef glTexCoord2fv
#undef glVertex4f
#undef glVertex3f
#undef glVertex2f
#undef glVertex2i
#undef glVertex4fv
#undef glVertex3fv
#undef glVertex2fv
#undef glColor4f
#undef glColor3f
#undef glColor4ub
#undef glColor3ub
#undef glColor4fv
#undef glColor3fv
#undef glColor4ubv
#undef glColor3ubv
#undef glActiveTextureARB
#undef glMultiTexCoord2fARB
#undef glMultiTexCoord2fvARB
#undef glEnableClientState
#undef glDisableClientState
#undef glArrayElement
#undef glPointSize


// The macros:

// Texture unit selection

#define glActiveTextureARB(unit) {\
CC->ActiveTexture = (GLenum)(unit) - (GLenum)GL_TEXTURE0_ARB;}

#define glBegin(mode)\
{\
CC->CurrentTexQValid = GL_FALSE;\
\
switch((int)(mode)){\
	case GL_POINTS:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawPoints;\
	break;\
	case GL_LINES:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawLines;\
	break;\
	case GL_LINE_STRIP:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawLineStrip;\
	break;\
	case GL_LINE_LOOP:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawLineStrip;\
	break;\
	case GL_TRIANGLES:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		break;\
	case GL_TRIANGLE_STRIP:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawTriangleStrip;\
	break;\
	case GL_TRIANGLE_FAN:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawTriangleFan;\
	break;\
	case GL_QUADS:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawQuads;\
	break;\
	case GL_QUAD_STRIP:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawQuadStrip;\
	break;\
	case GL_POLYGON:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		\
		if (CC->Texture2D_State[1] == GL_TRUE)\
		CC->CurrentDraw = (DrawFn)d_DrawMtexPoly;\
		else if (CC->ShadeModel == GL_SMOOTH)\
		CC->CurrentDraw = (DrawFn)d_DrawSmoothPoly;\
		else\
		CC->CurrentDraw = (DrawFn)d_DrawNormalPoly;\
	break;\
	case MGL_FLATFAN:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawFlat;\
	break;\
	case MGL_FLATSTRIP:\
		CC->CurrentPrimitive = (GLenum)(mode);\
		CC->CurrentDraw = (DrawFn)d_DrawFlat;\
	break;\
	default:\
		GLFlagError (CC, 1, GL_INVALID_OPERATION);\
	break;}\
\
CC->VertexBufferPointer = 0;\
mglvert = &(CC->VertexBuffer[0]);\
}


//Colors

#define glColor3f(red,green,blue) {\
CC->CurrentColor.r = (W3D_Float)(red);\
CC->CurrentColor.g = (W3D_Float)(green);\
CC->CurrentColor.b = (W3D_Float)(blue);\
CC->CurrentColor.a = 1.f;\
CC->UpdateCurrentColor = GL_TRUE;}

#define glColor4f(red,green,blue,alpha) {\
CC->CurrentColor.r = (W3D_Float)(red);\
CC->CurrentColor.g = (W3D_Float)(green);\
CC->CurrentColor.b = (W3D_Float)(blue);\
CC->CurrentColor.a = (W3D_Float)(alpha);\
CC->UpdateCurrentColor = GL_TRUE;}

#define glColor3ub(red,green,blue) {\
CC->CurrentColor.r = (W3D_Float)(red)*MGL_UBTOF;\
CC->CurrentColor.g = (W3D_Float)(green)*MGL_UBTOF;\
CC->CurrentColor.b = (W3D_Float)(blue)*MGL_UBTOF;\
CC->CurrentColor.a = 1.f;\
CC->UpdateCurrentColor = GL_TRUE;}

#define glColor4ub(red,green,blue,alpha) {\
CC->CurrentColor.r = (W3D_Float)(red)*MGL_UBTOF;\
CC->CurrentColor.g = (W3D_Float)(green)*MGL_UBTOF;\
CC->CurrentColor.b = (W3D_Float)(blue)*MGL_UBTOF;\
CC->CurrentColor.a = (W3D_Float)(alpha)*MGL_UBTOF;\
CC->UpdateCurrentColor = GL_TRUE;}

//pointer versions

#define glColor3fv(color) {\
CC->CurrentColor.r = ((GLfloat*)(color))[0];\
CC->CurrentColor.g = ((GLfloat*)(color))[1];\
CC->CurrentColor.b = ((GLfloat*)(color))[2];\
CC->CurrentColor.a = 1.f;\
CC->UpdateCurrentColor = GL_TRUE;}

#define glColor4fv(color) {\
CC->CurrentColor.r = ((GLfloat*)(color))[0];\
CC->CurrentColor.g = ((GLfloat*)(color))[1];\
CC->CurrentColor.b = ((GLfloat*)(color))[2];\
CC->CurrentColor.a = ((GLfloat*)(color))[3];\
CC->UpdateCurrentColor = GL_TRUE;}

#define glColor3ubv(color) {\
CC->CurrentColor.r = (W3D_Float)((GLubyte*)(color))[0]*MGL_UBTOF;\
CC->CurrentColor.g = (W3D_Float)((GLubyte*)(color))[1]*MGL_UBTOF;\
CC->CurrentColor.b = (W3D_Float)((GLubyte*)(color))[2]*MGL_UBTOF;\
CC->CurrentColor.a = 1.f;\
CC->UpdateCurrentColor = GL_TRUE;}

#define glColor4ubv(color) {\
CC->CurrentColor.r = (W3D_Float)((GLubyte*)(color))[0]*MGL_UBTOF;\
CC->CurrentColor.g = (W3D_Float)((GLubyte*)(color))[1]*MGL_UBTOF;\
CC->CurrentColor.b = (W3D_Float)((GLubyte*)(color))[2]*MGL_UBTOF;\
CC->CurrentColor.a = (W3D_Float)((GLubyte*)(color))[3]*MGL_UBTOF;\
CC->UpdateCurrentColor = GL_TRUE;}


//Texcoords

#define glTexCoord4f(s,t,r,q) {\
mglvert->v.u = (W3D_Float)((GLfloat)(s)/(GLfloat)(q));\
mglvert->v.v = (W3D_Float)((GLfloat)(t)/(GLfloat)(q));\
mglvert->q = (GLfloat)(q);CC->CurrentTexQValid = GL_TRUE;}

#define glTexCoord2f(s,t) {\
mglvert->v.u = (W3D_Float)(s);\
mglvert->v.v = (W3D_Float)(t);}

#define glMultiTexCoord2fARB(unit,s,t) {\
if((GLenum)(unit) - (GLenum)GL_TEXTURE0_ARB){\
mglvert->tcoord.s = (W3D_Float)(s);\
mglvert->tcoord.t = (W3D_Float)(t);\
}else{\
mglvert->v.u = (W3D_Float)(s);\
mglvert->v.v = (W3D_Float)(t);}\
}

//pointer versions

#define glTexCoord4fv(coord) {\
mglvert->v.u = ((GLfloat*)(coord))[0]/((GLfloat*)(coord))[3]);\
mglvert->v.v = ((GLfloat*)(coord))[1]/((GLfloat*)(coord))[3]);\
mglvert->q = ((GLfloat*)(coord))[3];CC->CurrentTexQValid = GL_TRUE;}

#define glTexCoord2fv(coord) {\
mglvert->v.u = ((GLfloat*)(coord))[0];\
mglvert->v.v = ((GLfloat*)(coord))[1];}

#define glMultiTexCoord2fvARB(unit, coord) {\
if((GLenum)(unit) - (GLenum)GL_TEXTURE0_ARB){\
mglvert->tcoord.s = ((GLfloat*)(coord))[0];\
mglvert->tcoord.t = ((GLfloat*)(coord))[1];\
}else{\
mglvert->v.u = ((GLfloat*)(coord))[0];\
mglvert->v.v = ((GLfloat*)(coord))[1];}\
}


//Verts

#define glVertex4f(x,y,z,w) {\
if(CC->ShadeModel == GL_SMOOTH)\
{\
mglvert->v.color.r = CC->CurrentColor.r;\
mglvert->v.color.g = CC->CurrentColor.g;\
mglvert->v.color.b = CC->CurrentColor.b;\
mglvert->v.color.a = CC->CurrentColor.a;\
}\
mglvert->bx=(GLfloat)(x);\
mglvert->by=(GLfloat)(y);\
mglvert->bz=(GLfloat)(z);\
mglvert->bw=(GLfloat)(w);\
CC->VertexBufferPointer++;\
mglvert++;\
}\

#define glVertex3f(x,y,z) {\
if(CC->ShadeModel == GL_SMOOTH)\
{\
mglvert->v.color.r = CC->CurrentColor.r;\
mglvert->v.color.g = CC->CurrentColor.g;\
mglvert->v.color.b = CC->CurrentColor.b;\
mglvert->v.color.a = CC->CurrentColor.a;\
}\
mglvert->bx=(GLfloat)(x);\
mglvert->by=(GLfloat)(y);\
mglvert->bz=(GLfloat)(z);\
mglvert->bw=(GLfloat)1.f;\
CC->VertexBufferPointer++;\
mglvert++;\
}\

#define glVertex2f(x,y) {\
if(CC->ShadeModel == GL_SMOOTH)\
{\
mglvert->v.color.r = CC->CurrentColor.r;\
mglvert->v.color.g = CC->CurrentColor.g;\
mglvert->v.color.b = CC->CurrentColor.b;\
mglvert->v.color.a = CC->CurrentColor.a;\
}\
mglvert->bx=(GLfloat)(x);\
mglvert->by=(GLfloat)(y);\
mglvert->bz=(GLfloat)0.f;\
mglvert->bw=(GLfloat)1.f;\
CC->VertexBufferPointer++;\
mglvert++;\
}\

//pointer versions

#define glVertex4fv(vtx) {\
if(CC->ShadeModel == GL_SMOOTH)\
{\
mglvert->v.color.r = CC->CurrentColor.r;\
mglvert->v.color.g = CC->CurrentColor.g;\
mglvert->v.color.b = CC->CurrentColor.b;\
mglvert->v.color.a = CC->CurrentColor.a;\
}\
mglvert->bx = ((GLfloat*)(vtx))[0];\
mglvert->by = ((GLfloat*)(vtx))[1];\
mglvert->bz = ((GLfloat*)(vtx))[2];\
mglvert->bw = ((GLfloat*)(vtx))[3];\
CC->VertexBufferPointer++;\
mglvert++;\
}\

#define glVertex3fv(vtx) {\
if(CC->ShadeModel == GL_SMOOTH)\
{\
mglvert->v.color.r = CC->CurrentColor.r;\
mglvert->v.color.g = CC->CurrentColor.g;\
mglvert->v.color.b = CC->CurrentColor.b;\
mglvert->v.color.a = CC->CurrentColor.a;\
}\
mglvert->bx = ((GLfloat*)(vtx))[0];\
mglvert->by = ((GLfloat*)(vtx))[1];\
mglvert->bz = ((GLfloat*)(vtx))[2];\
mglvert->bw = (GLfloat)1.f;\
CC->VertexBufferPointer++;\
mglvert++;\
}\

#define glVertex2fv(vtx) {\
if(CC->ShadeModel == GL_SMOOTH)\
{\
mglvert->v.color.r = CC->CurrentColor.r;\
mglvert->v.color.g = CC->CurrentColor.g;\
mglvert->v.color.b = CC->CurrentColor.b;\
mglvert->v.color.a = CC->CurrentColor.a;\
}\
mglvert->bx = ((GLfloat*)(vtx))[0];\
mglvert->by = ((GLfloat*)(vtx))[1];\
mglvert->bz = (GLfloat)0.f;\
mglvert->bw = (GLfloat)1.f;\
CC->VertexBufferPointer++;\
mglvert++;\
}\

#define glVertex2i(ax,ay) {mglVertex2f((GLfloat)(ax),(GLfloat)(ay))}


//Client states

#ifdef VA_SANITY_CHECK

	extern void Swap_TextureCoordPointers(GLcontext context, GLboolean enable);

#else 

	#define Swap_TextureCoordPointers(c,e) {}

#endif


#define glEnableClientState(state) {\
switch((GLenum)(state)) {\
case GL_TEXTURE_COORD_ARRAY:CC->ClientState |= GLCS_TEXTURE;Swap_TextureCoordPointers(CC, GL_TRUE);break;\
case GL_COLOR_ARRAY:CC->ClientState |= GLCS_COLOR;break;\
case GL_VERTEX_ARRAY:CC->ClientState |= GLCS_VERTEX;break;\
default:GLFlagError(context, 1, GL_INVALID_ENUM);break;} }

#define glDisableClientState(state) {\
switch((GLenum)(state)) {\
case GL_TEXTURE_COORD_ARRAY:CC->ClientState &= ~GLCS_TEXTURE;Swap_TextureCoordPointers(CC, GL_FALSE);break;\
case GL_COLOR_ARRAY:CC->ClientState &= ~GLCS_COLOR;break;\
case GL_VERTEX_ARRAY:CC->ClientState &= ~GLCS_VERTEX;break;\
default:GLFlagError(context, 1, GL_INVALID_ENUM);break;} }


//index generation in glBegin/glEnd construct:

#define glArrayElement(element) {\
CC->ElementIndex[CC->VertexBufferPointer++] = (UWORD)(element);}

#define glPointSize(size) {\
CC->CurrentPointSize = (GLfloat)(size);}

#endif

#endif
