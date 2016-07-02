/* Automatically generated header (sfdc 1.10)! Do not edit! */

#ifndef _INLINE_WARP3D_H
#define _INLINE_WARP3D_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef WARP3D_BASE_NAME
#define WARP3D_BASE_NAME Warp3DBase
#endif /* !WARP3D_BASE_NAME */

#define W3D_CreateContext(___error, ___CCTags) \
      LP2(0x1e, W3D_Context    *, W3D_CreateContext , ULONG *, ___error, a0, struct TagItem *, ___CCTags, a1,\
      , WARP3D_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define W3D_CreateContextTags(___error, ___CCTags, ...) \
    ({_sfdc_vararg _tags[] = { ___CCTags, __VA_ARGS__ }; W3D_CreateContext((___error), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define W3D_DestroyContext(___context) \
      LP1NR(0x24, W3D_DestroyContext , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_GetState(___context, ___state) \
      LP2(0x2a, ULONG, W3D_GetState , W3D_Context *, ___context, a0, ULONG, ___state, d1,\
      , WARP3D_BASE_NAME)

#define W3D_SetState(___context, ___state, ___action) \
      LP3(0x30, ULONG, W3D_SetState , W3D_Context *, ___context, a0, ULONG, ___state, d0, ULONG, ___action, d1,\
      , WARP3D_BASE_NAME)

#define W3D_CheckDriver() \
      LP0(0x36, ULONG, W3D_CheckDriver ,\
      , WARP3D_BASE_NAME)

#define W3D_LockHardware(___context) \
      LP1(0x3c, ULONG, W3D_LockHardware , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_UnLockHardware(___context) \
      LP1NR(0x42, W3D_UnLockHardware , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_WaitIdle(___context) \
      LP1NR(0x48, W3D_WaitIdle , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_CheckIdle(___context) \
      LP1(0x4e, ULONG, W3D_CheckIdle , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_Query(___context, ___query, ___destfmt) \
      LP3(0x54, ULONG, W3D_Query , W3D_Context *, ___context, a0, ULONG, ___query, d0, ULONG, ___destfmt, d1,\
      , WARP3D_BASE_NAME)

#define W3D_GetTexFmtInfo(___context, ___format, ___destfmt) \
      LP3(0x5a, ULONG, W3D_GetTexFmtInfo , W3D_Context *, ___context, a0, ULONG, ___format, d0, ULONG, ___destfmt, d1,\
      , WARP3D_BASE_NAME)

#define W3D_AllocTexObj(___context, ___error, ___ATOTags) \
      LP3(0x60, W3D_Texture    *, W3D_AllocTexObj , W3D_Context *, ___context, a0, ULONG *, ___error, a1, struct TagItem *, ___ATOTags, a2,\
      , WARP3D_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define W3D_AllocTexObjTags(___context, ___error, ___ATOTags, ...) \
    ({_sfdc_vararg _tags[] = { ___ATOTags, __VA_ARGS__ }; W3D_AllocTexObj((___context), (___error), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define W3D_FreeTexObj(___context, ___texture) \
      LP2NR(0x66, W3D_FreeTexObj , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1,\
      , WARP3D_BASE_NAME)

#define W3D_ReleaseTexture(___context, ___texture) \
      LP2NR(0x6c, W3D_ReleaseTexture , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1,\
      , WARP3D_BASE_NAME)

#define W3D_FlushTextures(___context) \
      LP1NR(0x72, W3D_FlushTextures , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_SetFilter(___context, ___texture, ___min, ___mag) \
      LP4(0x78, ULONG, W3D_SetFilter , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1, ULONG, ___min, d0, ULONG, ___mag, d1,\
      , WARP3D_BASE_NAME)

#define W3D_SetTexEnv(___context, ___texture, ___envparam, ___envcolor) \
      LP4(0x7e, ULONG, W3D_SetTexEnv , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1, ULONG, ___envparam, d1, W3D_Color *, ___envcolor, a2,\
      , WARP3D_BASE_NAME)

#define W3D_SetWrapMode(___context, ___texture, ___mode_s, ___mode_t, ___bordercolor) \
      LP5(0x84, ULONG, W3D_SetWrapMode , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1, ULONG, ___mode_s, d0, ULONG, ___mode_t, d1, W3D_Color *, ___bordercolor, a2,\
      , WARP3D_BASE_NAME)

#define W3D_UpdateTexImage(___context, ___texture, ___teximage, ___level, ___palette) \
      LP5(0x8a, ULONG, W3D_UpdateTexImage , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1, void *, ___teximage, a2, int, ___level, d1, ULONG *, ___palette, a3,\
      , WARP3D_BASE_NAME)

#define W3D_UploadTexture(___context, ___texture) \
      LP2(0x90, ULONG, W3D_UploadTexture , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawLine(___context, ___line) \
      LP2(0x96, ULONG, W3D_DrawLine , W3D_Context *, ___context, a0, W3D_Line *, ___line, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawPoint(___context, ___point) \
      LP2(0x9c, ULONG, W3D_DrawPoint , W3D_Context *, ___context, a0, W3D_Point *, ___point, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawTriangle(___context, ___triangle) \
      LP2(0xa2, ULONG, W3D_DrawTriangle , W3D_Context *, ___context, a0, W3D_Triangle *, ___triangle, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawTriFan(___context, ___triangles) \
      LP2(0xa8, ULONG, W3D_DrawTriFan , W3D_Context *, ___context, a0, W3D_Triangles *, ___triangles, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawTriStrip(___context, ___triangles) \
      LP2(0xae, ULONG, W3D_DrawTriStrip , W3D_Context *, ___context, a0, W3D_Triangles *, ___triangles, a1,\
      , WARP3D_BASE_NAME)

#define W3D_SetAlphaMode(___context, ___mode, ___refval) \
      LP3(0xb4, ULONG, W3D_SetAlphaMode , W3D_Context *, ___context, a0, ULONG, ___mode, d1, W3D_Float *, ___refval, a1,\
      , WARP3D_BASE_NAME)

#define W3D_SetBlendMode(___context, ___srcfunc, ___dstfunc) \
      LP3(0xba, ULONG, W3D_SetBlendMode , W3D_Context *, ___context, a0, ULONG, ___srcfunc, d0, ULONG, ___dstfunc, d1,\
      , WARP3D_BASE_NAME)

#define W3D_SetDrawRegion(___context, ___bm, ___yoffset, ___scissor) \
      LP4(0xc0, ULONG, W3D_SetDrawRegion , W3D_Context *, ___context, a0, struct BitMap *, ___bm, a1, int, ___yoffset, d1, W3D_Scissor *, ___scissor, a2,\
      , WARP3D_BASE_NAME)

#define W3D_SetFogParams(___context, ___fogparams, ___fogmode) \
      LP3(0xc6, ULONG, W3D_SetFogParams , W3D_Context *, ___context, a0, W3D_Fog *, ___fogparams, a1, ULONG, ___fogmode, d1,\
      , WARP3D_BASE_NAME)

#define W3D_SetColorMask(___context, ___red, ___green, ___blue, ___alpha) \
      LP5(0xcc, ULONG, W3D_SetColorMask , W3D_Context *, ___context, a0, W3D_Bool, ___red, d0, W3D_Bool, ___green, d1, W3D_Bool, ___blue, d2, W3D_Bool, ___alpha, d3,\
      , WARP3D_BASE_NAME)

#define W3D_SetStencilFunc(___context, ___func, ___refvalue, ___mask) \
      LP4(0xd2, ULONG, W3D_SetStencilFunc , W3D_Context *, ___context, a0, ULONG, ___func, d0, ULONG, ___refvalue, d1, ULONG, ___mask, d2,\
      , WARP3D_BASE_NAME)

#define W3D_AllocZBuffer(___context) \
      LP1(0xd8, ULONG, W3D_AllocZBuffer , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_FreeZBuffer(___context) \
      LP1(0xde, ULONG, W3D_FreeZBuffer , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_ClearZBuffer(___context, ___clearvalue) \
      LP2(0xe4, ULONG, W3D_ClearZBuffer , W3D_Context *, ___context, a0, W3D_Double *, ___clearvalue, a1,\
      , WARP3D_BASE_NAME)

#define W3D_ReadZPixel(___context, ___x, ___y, ___z) \
      LP4(0xea, ULONG, W3D_ReadZPixel , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, W3D_Double *, ___z, a1,\
      , WARP3D_BASE_NAME)

#define W3D_ReadZSpan(___context, ___x, ___y, ___n, ___z) \
      LP5(0xf0, ULONG, W3D_ReadZSpan , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG, ___n, d2, W3D_Double *, ___z, a1,\
      , WARP3D_BASE_NAME)

#define W3D_SetZCompareMode(___context, ___mode) \
      LP2(0xf6, ULONG, W3D_SetZCompareMode , W3D_Context *, ___context, a0, ULONG, ___mode, d1,\
      , WARP3D_BASE_NAME)

#define W3D_AllocStencilBuffer(___context) \
      LP1(0xfc, ULONG, W3D_AllocStencilBuffer , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_ClearStencilBuffer(___context, ___clearval) \
      LP2(0x102, ULONG, W3D_ClearStencilBuffer , W3D_Context *, ___context, a0, ULONG *, ___clearval, a1,\
      , WARP3D_BASE_NAME)

#define W3D_FillStencilBuffer(___context, ___x, ___y, ___width, ___height, ___depth, ___data) \
      LP7(0x108, ULONG, W3D_FillStencilBuffer , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG, ___width, d2, ULONG, ___height, d3, ULONG, ___depth, d4, void *, ___data, a1,\
      , WARP3D_BASE_NAME)

#define W3D_FreeStencilBuffer(___context) \
      LP1(0x10e, ULONG, W3D_FreeStencilBuffer , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_ReadStencilPixel(___context, ___x, ___y, ___st) \
      LP4(0x114, ULONG, W3D_ReadStencilPixel , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG *, ___st, a1,\
      , WARP3D_BASE_NAME)

#define W3D_ReadStencilSpan(___context, ___x, ___y, ___n, ___st) \
      LP5(0x11a, ULONG, W3D_ReadStencilSpan , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG, ___n, d2, ULONG *, ___st, a1,\
      , WARP3D_BASE_NAME)

#define W3D_SetLogicOp(___context, ___operation) \
      LP2(0x120, ULONG, W3D_SetLogicOp , W3D_Context *, ___context, a0, ULONG, ___operation, d1,\
      , WARP3D_BASE_NAME)

#define W3D_Hint(___context, ___mode, ___quality) \
      LP3(0x126, ULONG, W3D_Hint , W3D_Context *, ___context, a0, ULONG, ___mode, d0, ULONG, ___quality, d1,\
      , WARP3D_BASE_NAME)

#define W3D_SetDrawRegionWBM(___context, ___bitmap, ___scissor) \
      LP3(0x12c, ULONG, W3D_SetDrawRegionWBM , W3D_Context *, ___context, a0, W3D_Bitmap *, ___bitmap, a1, W3D_Scissor *, ___scissor, a2,\
      , WARP3D_BASE_NAME)

#define W3D_GetDriverState(___context) \
      LP1(0x132, ULONG, W3D_GetDriverState , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_Flush(___context) \
      LP1(0x138, ULONG, W3D_Flush , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_SetPenMask(___context, ___pen) \
      LP2(0x13e, ULONG, W3D_SetPenMask , W3D_Context *, ___context, a0, ULONG, ___pen, d1,\
      , WARP3D_BASE_NAME)

#define W3D_SetStencilOp(___context, ___sfail, ___dpfail, ___dppass) \
      LP4(0x144, ULONG, W3D_SetStencilOp , W3D_Context *, ___context, a0, ULONG, ___sfail, d0, ULONG, ___dpfail, d1, ULONG, ___dppass, d2,\
      , WARP3D_BASE_NAME)

#define W3D_SetWriteMask(___context, ___mask) \
      LP2(0x14a, ULONG, W3D_SetWriteMask , W3D_Context *, ___context, a0, ULONG, ___mask, d1,\
      , WARP3D_BASE_NAME)

#define W3D_WriteStencilPixel(___context, ___x, ___y, ___st) \
      LP4(0x150, ULONG, W3D_WriteStencilPixel , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG, ___st, d2,\
      , WARP3D_BASE_NAME)

#define W3D_WriteStencilSpan(___context, ___x, ___y, ___n, ___st, ___mask) \
      LP6(0x156, ULONG, W3D_WriteStencilSpan , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG, ___n, d2, ULONG *, ___st, a1, UBYTE *, ___mask, a2,\
      , WARP3D_BASE_NAME)

#define W3D_WriteZPixel(___context, ___x, ___y, ___z) \
      LP4NR(0x15c, W3D_WriteZPixel , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, W3D_Double *, ___z, a1,\
      , WARP3D_BASE_NAME)

#define W3D_WriteZSpan(___context, ___x, ___y, ___n, ___z, ___maks) \
      LP6NR(0x162, W3D_WriteZSpan , W3D_Context *, ___context, a0, ULONG, ___x, d0, ULONG, ___y, d1, ULONG, ___n, d2, W3D_Double *, ___z, a1, UBYTE *, ___maks, a2,\
      , WARP3D_BASE_NAME)

#define W3D_SetCurrentColor(___context, ___color) \
      LP2(0x168, ULONG, W3D_SetCurrentColor , W3D_Context *, ___context, a0, W3D_Color *, ___color, a1,\
      , WARP3D_BASE_NAME)

#define W3D_SetCurrentPen(___context, ___pen) \
      LP2(0x16e, ULONG, W3D_SetCurrentPen , W3D_Context *, ___context, a0, ULONG, ___pen, d1,\
      , WARP3D_BASE_NAME)

#define W3D_UpdateTexSubImage(___context, ___texture, ___teximage, ___lev, ___palette, ___scissor, ___srcbpr) \
      LP7A4(0x174, ULONG, W3D_UpdateTexSubImage , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1, void *, ___teximage, a2, ULONG, ___lev, d1, ULONG *, ___palette, a3, W3D_Scissor*, ___scissor, d7, ULONG, ___srcbpr, d0,\
      , WARP3D_BASE_NAME)

#define W3D_FreeAllTexObj(___context) \
      LP1(0x17a, ULONG, W3D_FreeAllTexObj , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_GetDestFmt() \
      LP0(0x180, ULONG, W3D_GetDestFmt ,\
      , WARP3D_BASE_NAME)

#define W3D_DrawLineStrip(___context, ___lines) \
      LP2(0x186, ULONG, W3D_DrawLineStrip , W3D_Context *, ___context, a0, W3D_Lines *, ___lines, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawLineLoop(___context, ___lines) \
      LP2(0x18c, ULONG, W3D_DrawLineLoop , W3D_Context *, ___context, a0, W3D_Lines *, ___lines, a1,\
      , WARP3D_BASE_NAME)

#define W3D_GetDrivers() \
      LP0(0x192, W3D_Driver **, W3D_GetDrivers ,\
      , WARP3D_BASE_NAME)

#define W3D_QueryDriver(___driver, ___query, ___destfmt) \
      LP3(0x198, ULONG, W3D_QueryDriver , W3D_Driver*, ___driver, a0, ULONG, ___query, d0, ULONG, ___destfmt, d1,\
      , WARP3D_BASE_NAME)

#define W3D_GetDriverTexFmtInfo(___driver, ___format, ___destfmt) \
      LP3(0x19e, ULONG, W3D_GetDriverTexFmtInfo , W3D_Driver*, ___driver, a0, ULONG, ___format, d0, ULONG, ___destfmt, d1,\
      , WARP3D_BASE_NAME)

#define W3D_RequestMode(___taglist) \
      LP1(0x1a4, ULONG, W3D_RequestMode , struct TagItem *, ___taglist, a0,\
      , WARP3D_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define W3D_RequestModeTags(___taglist, ...) \
    ({_sfdc_vararg _tags[] = { ___taglist, __VA_ARGS__ }; W3D_RequestMode((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define W3D_SetScissor(___context, ___scissor) \
      LP2NR(0x1aa, W3D_SetScissor , W3D_Context *, ___context, a0, W3D_Scissor *, ___scissor, a1,\
      , WARP3D_BASE_NAME)

#define W3D_FlushFrame(___context) \
      LP1NR(0x1b0, W3D_FlushFrame , W3D_Context *, ___context, a0,\
      , WARP3D_BASE_NAME)

#define W3D_TestMode(___ModeID) \
      LP1(0x1b6, W3D_Driver *, W3D_TestMode , ULONG, ___ModeID, d0,\
      , WARP3D_BASE_NAME)

#define W3D_SetChromaTestBounds(___context, ___texture, ___rgba_lower, ___rgba_upper, ___mode) \
      LP5(0x1bc, ULONG, W3D_SetChromaTestBounds , W3D_Context *, ___context, a0, W3D_Texture *, ___texture, a1, ULONG, ___rgba_lower, d0, ULONG, ___rgba_upper, d1, ULONG, ___mode, d2,\
      , WARP3D_BASE_NAME)

#define W3D_ClearDrawRegion(___context, ___color) \
      LP2(0x1c2, ULONG, W3D_ClearDrawRegion , W3D_Context *, ___context, a0, ULONG, ___color, d0,\
      , WARP3D_BASE_NAME)

#define W3D_DrawTriangleV(___context, ___triangle) \
      LP2(0x1c8, ULONG, W3D_DrawTriangleV , W3D_Context *, ___context, a0, W3D_TriangleV *, ___triangle, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawTriFanV(___context, ___triangles) \
      LP2(0x1ce, ULONG, W3D_DrawTriFanV , W3D_Context *, ___context, a0, W3D_TrianglesV *, ___triangles, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawTriStripV(___context, ___triangles) \
      LP2(0x1d4, ULONG, W3D_DrawTriStripV , W3D_Context *, ___context, a0, W3D_TrianglesV *, ___triangles, a1,\
      , WARP3D_BASE_NAME)

#define W3D_GetScreenmodeList() \
      LP0(0x1da, W3D_ScreenMode *, W3D_GetScreenmodeList ,\
      , WARP3D_BASE_NAME)

#define W3D_FreeScreenmodeList(___list) \
      LP1NR(0x1e0, W3D_FreeScreenmodeList , W3D_ScreenMode *, ___list, a0,\
      , WARP3D_BASE_NAME)

#define W3D_BestModeID(___tags) \
      LP1(0x1e6, ULONG, W3D_BestModeID , struct TagItem *, ___tags, a0,\
      , WARP3D_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define W3D_BestModeIDTags(___tags, ...) \
    ({_sfdc_vararg _tags[] = { ___tags, __VA_ARGS__ }; W3D_BestModeID((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define W3D_VertexPointer(___context, ___pointer, ___stride, ___mode, ___flags) \
      LP5(0x1ec, ULONG, W3D_VertexPointer , W3D_Context*, ___context, a0, void *, ___pointer, a1, int, ___stride, d0, ULONG, ___mode, d1, ULONG, ___flags, d2,\
      , WARP3D_BASE_NAME)

#define W3D_TexCoordPointer(___context, ___pointer, ___stride, ___unit, ___off_v, ___off_w, ___flags) \
      LP7(0x1f2, ULONG, W3D_TexCoordPointer , W3D_Context*, ___context, a0, void *, ___pointer, a1, int, ___stride, d0, int, ___unit, d1, int, ___off_v, d2, int, ___off_w, d3, ULONG, ___flags, d4,\
      , WARP3D_BASE_NAME)

#define W3D_ColorPointer(___context, ___pointer, ___stride, ___format, ___mode, ___flags) \
      LP6(0x1f8, ULONG, W3D_ColorPointer , W3D_Context*, ___context, a0, void *, ___pointer, a1, int, ___stride, d0, ULONG, ___format, d1, ULONG, ___mode, d2, ULONG, ___flags, d3,\
      , WARP3D_BASE_NAME)

#define W3D_BindTexture(___context, ___tmu, ___texture) \
      LP3(0x1fe, ULONG, W3D_BindTexture , W3D_Context*, ___context, a0, ULONG, ___tmu, d0, W3D_Texture *, ___texture, a1,\
      , WARP3D_BASE_NAME)

#define W3D_DrawArray(___context, ___primitive, ___base, ___count) \
      LP4(0x204, ULONG, W3D_DrawArray , W3D_Context*, ___context, a0, ULONG, ___primitive, d0, ULONG, ___base, d1, ULONG, ___count, d2,\
      , WARP3D_BASE_NAME)

#define W3D_DrawElements(___context, ___primitive, ___type, ___count, ___indices) \
      LP5(0x20a, ULONG, W3D_DrawElements , W3D_Context*, ___context, a0, ULONG, ___primitive, d0, ULONG, ___type, d1, ULONG, ___count, d2, void *, ___indices, a1,\
      , WARP3D_BASE_NAME)

#define W3D_SetFrontFace(___context, ___direction) \
      LP2NR(0x210, W3D_SetFrontFace , W3D_Context*, ___context, a0, ULONG, ___direction, d0,\
      , WARP3D_BASE_NAME)

#endif /* !_INLINE_WARP3D_H */
