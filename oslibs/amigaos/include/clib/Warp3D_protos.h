/*
**
** Prototypes for Warp3D main library
** $VER: Warp3D_protos.h 1.0 (26.07.98)
**
**
** This file is part of the Warp3D Project.
** Warp3D is copyrighted
** (C) 1998 Sam Jordan, Hans-Jörg Frieden, Thomas Frieden
** All rights reserved
**
** See the documentation for conditions.
**
*/

#ifndef __CLIB_WARP3D_PROTOS_H
#define __CLIB_WARP3D_PROTOS_H

#ifndef EXEC_TYPES_H
#include <exex/types.h>
#endif
#ifndef __WARP3D_WARP3D_H
#include <Warp3D/Warp3D.h>
#endif
#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#if defined(__STORM__) && defined(__PPC__)

#ifndef STORMPROTOS_WARP3D_SPROTOS_H
#include <stormprotos/warp3d_sprotos.h>
#endif

#else


#ifdef __cplusplus
extern "C" {
#endif


/************************** Context functions ***********************************/
W3D_Context    *W3D_CreateContext(ULONG *error, struct TagItem *CCTags);
W3D_Context    *W3D_CreateContextTags(ULONG *error, Tag tag1, ...);
void            W3D_DestroyContext(W3D_Context *context);
ULONG           W3D_GetState(W3D_Context *context, ULONG state);
ULONG           W3D_SetState(W3D_Context *context, ULONG state, ULONG action);
ULONG           W3D_Hint(W3D_Context *context, ULONG mode, ULONG quality);

/************************** Hardware/Driver functions ***************************/
ULONG           W3D_CheckDriver(void);
ULONG           W3D_LockHardware(W3D_Context *context);
void            W3D_UnLockHardware(W3D_Context *context);
void            W3D_WaitIdle(W3D_Context *context);
ULONG           W3D_CheckIdle(W3D_Context *context);
ULONG           W3D_Query(W3D_Context *context, ULONG query, ULONG destfmt);
ULONG           W3D_GetTexFmtInfo(W3D_Context *context, ULONG texfmt, ULONG destfmt);
ULONG           W3D_GetDriverState(W3D_Context *context);
ULONG           W3D_GetDestFmt(void);
W3D_Driver **   W3D_GetDrivers(void);
ULONG           W3D_QueryDriver(W3D_Driver* driver, ULONG query, ULONG destfmt);
ULONG           W3D_GetDriverTexFmtInfo(W3D_Driver* driver, ULONG query, ULONG destfmt);
ULONG           W3D_RequestMode(struct TagItem *taglist);
ULONG           W3D_RequestModeTags(Tag tag1, ...);
W3D_Driver *    W3D_TestMode(ULONG ModeID);

/************************** Texture functions ***********************************/
W3D_Texture    *W3D_AllocTexObj(W3D_Context *context, ULONG *error,
										struct TagItem *ATOTags);
W3D_Texture    *W3D_AllocTexObjTags(W3D_Context *context, ULONG *error,
										Tag tag1, ...);
void            W3D_FreeTexObj(W3D_Context *context, W3D_Texture *texture);
void            W3D_ReleaseTexture(W3D_Context *context, W3D_Texture *texture);
void            W3D_FlushTextures(W3D_Context *context);
ULONG           W3D_SetFilter(W3D_Context *context, W3D_Texture *texture,
										ULONG MinFilter, ULONG MagFilter);
ULONG           W3D_SetTexEnv(W3D_Context *context, W3D_Texture *texture,
										ULONG envparam, W3D_Color *envcolor);
ULONG           W3D_SetWrapMode(W3D_Context *context, W3D_Texture *texture,
										ULONG s_mode, ULONG t_mode, W3D_Color *bordercolor);
ULONG           W3D_UpdateTexImage(W3D_Context *context, W3D_Texture *texture,
										void *teximage, int level, ULONG *palette);
ULONG           W3D_UpdateTexSubImage(W3D_Context *context, W3D_Texture *texture, void *teximage,
					ULONG level, ULONG *palette, W3D_Scissor* scissor, ULONG srcbpr);
ULONG           W3D_UploadTexture(W3D_Context *context, W3D_Texture *texture);
ULONG           W3D_FreeAllTexObj(W3D_Context *context);
ULONG           W3D_SetChromaTestBounds(W3D_Context *context, W3D_Texture *texture,
							ULONG rgba_lower, ULONG rgba_upper, ULONG mode);

/************************** Drawing functions ***********************************/
ULONG           W3D_DrawLine(W3D_Context *context, W3D_Line *line);
ULONG           W3D_DrawPoint(W3D_Context *context, W3D_Point *point);
ULONG           W3D_DrawTriangle(W3D_Context *context, W3D_Triangle *triangle);
ULONG           W3D_DrawTriFan(W3D_Context *context, W3D_Triangles *triangles);
ULONG           W3D_DrawTriStrip(W3D_Context *context, W3D_Triangles *triangles);
ULONG           W3D_Flush(W3D_Context *context);
ULONG           W3D_DrawLineStrip(W3D_Context *context, W3D_Lines *lines);
ULONG           W3D_DrawLineLoop(W3D_Context *context, W3D_Lines *lines);
ULONG           W3D_ClearDrawRegion(W3D_Context *context, ULONG color);
/************************** Effect functions ************************************/
ULONG           W3D_SetAlphaMode(W3D_Context *context, ULONG mode, W3D_Float *refval);
ULONG           W3D_SetBlendMode(W3D_Context *context, ULONG srcfunc, ULONG dstfunc);
ULONG           W3D_SetDrawRegion(W3D_Context *context, struct BitMap *bm,
										int yoffset, W3D_Scissor *scissor);
ULONG           W3D_SetDrawRegionWBM(W3D_Context *context, W3D_Bitmap *bm,
										W3D_Scissor *scissor);
ULONG           W3D_SetFogParams(W3D_Context *context, W3D_Fog *fogparams,
										ULONG fogmode);
ULONG           W3D_SetLogicOp(W3D_Context *context, ULONG operation);
ULONG           W3D_SetColorMask(W3D_Context *context, W3D_Bool red, W3D_Bool green,
										W3D_Bool blue, W3D_Bool alpha);
ULONG           W3D_SetPenMask(W3D_Context *context, ULONG pen);
ULONG           W3D_SetCurrentColor(W3D_Context *context, W3D_Color *color);
ULONG           W3D_SetCurrentPen(W3D_Context *context, ULONG pen);
void            W3D_SetScissor(W3D_Context *context, W3D_Scissor *scissor);
void            W3D_FlushFrame(W3D_Context *context);

/************************** ZBuffer functions ***********************************/
ULONG           W3D_AllocZBuffer(W3D_Context *context);
ULONG           W3D_FreeZBuffer(W3D_Context *context);
ULONG           W3D_ClearZBuffer(W3D_Context *context, W3D_Double *clearvalue);
ULONG           W3D_ReadZPixel(W3D_Context *context, ULONG x, ULONG y,
										W3D_Double *z);
ULONG           W3D_ReadZSpan(W3D_Context *context, ULONG x, ULONG y,
										ULONG n, W3D_Double *z);
ULONG           W3D_SetZCompareMode(W3D_Context *context, ULONG mode);
void            W3D_WriteZPixel(W3D_Context *context, ULONG x, ULONG y,
										W3D_Double *z);
void            W3D_WriteZSpan(W3D_Context *context, ULONG x, ULONG y,
										ULONG n, W3D_Double *z, UBYTE *mask);

/************************** StencilBuffer functions *****************************/
ULONG           W3D_AllocStencilBuffer(W3D_Context *context);
ULONG           W3D_ClearStencilBuffer(W3D_Context *context, ULONG *clearvalue);
ULONG           W3D_FillStencilBuffer(W3D_Context *context, ULONG x, ULONG y,
										ULONG width, ULONG height, ULONG depth,
										void *data);
ULONG           W3D_FreeStencilBuffer(W3D_Context *context);
ULONG           W3D_ReadStencilPixel(W3D_Context *context, ULONG x, ULONG y,
										ULONG *st);
ULONG           W3D_ReadStencilSpan(W3D_Context *context, ULONG x, ULONG y,
										ULONG n, ULONG *st);
ULONG           W3D_SetStencilFunc(W3D_Context *context, ULONG func, ULONG refvalue,
										ULONG mask);
ULONG           W3D_SetStencilOp(W3D_Context *context, ULONG sfail, ULONG dpfail,
										ULONG dppass);
ULONG           W3D_SetWriteMask(W3D_Context *context, ULONG mask);
ULONG           W3D_WriteStencilPixel(W3D_Context *context, ULONG x, ULONG y, ULONG st);
ULONG           W3D_WriteStencilSpan(W3D_Context *context, ULONG x, ULONG y, ULONG n,
										ULONG *st, UBYTE *mask);
/*************************** V3 Vector functions **********************************/
ULONG           W3D_DrawTriangleV(W3D_Context *context, W3D_TriangleV *triangle);
ULONG           W3D_DrawTriFanV(W3D_Context *context, W3D_TrianglesV *triangles);
ULONG           W3D_DrawTriStripV(W3D_Context *context, W3D_TrianglesV *triangles);

/*************************** V3 Screenmode functions ******************************/
W3D_ScreenMode *W3D_GetScreenmodeList(void);
void            W3D_FreeScreenmodeList(W3D_ScreenMode *list);
ULONG           W3D_BestModeID(struct TagItem *tags);
ULONG           W3D_BestModeIDTags(Tag tag1, ...);

/*************************** V4 Array function ************************************/
ULONG           W3D_VertexPointer(W3D_Context* context, void *pointer, int stride,
					ULONG mode, ULONG flags);
ULONG           W3D_TexCoordPointer(W3D_Context* context, void *pointer, int stride,
					int unit, int off_v, int off_w, ULONG flags);
ULONG           W3D_ColorPointer(W3D_Context* context, void *pointer, int stride,
					ULONG format, ULONG mode, ULONG flags);
ULONG           W3D_BindTexture(W3D_Context* context, ULONG tmu, W3D_Texture *texture);
ULONG           W3D_DrawArray(W3D_Context* context, ULONG primitive, ULONG base, ULONG count);
ULONG           W3D_DrawElements(W3D_Context* context, ULONG primitive, ULONG type, ULONG count,
					void *indices);
void            W3D_SetFrontFace(W3D_Context* context, ULONG direction);

#ifdef __cplusplus
};
#endif


#ifndef __PPC__
#ifdef STORMPRAGMAS
#ifndef _INCLUDE_PRAGMA_WARP3D_LIB_H
#include <pragma/Warp3D_lib.h>
#endif
#endif
#endif

#endif /* __STORM__ && __PPC__ */

#endif


