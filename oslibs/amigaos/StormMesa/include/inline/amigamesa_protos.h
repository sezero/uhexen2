struct amigamesa_context* APIENTRY  __AmigaMesaCreateContext(__reg("a0") struct TagItem *tagList ,__reg("a6") void *)="\tjsr\t-36(a6)";
#define AmigaMesaCreateContext(x1) __AmigaMesaCreateContext((x1),glBase)
void APIENTRY  __AmigaMesaDestroyContext(__reg("a0") struct amigamesa_context *c ,__reg("a6") void *)="\tjsr\t-42(a6)";
#define AmigaMesaDestroyContext(x1) __AmigaMesaDestroyContext((x1),glBase)
struct amigamesa_visual* APIENTRY  __AmigaMesaCreateVisual(__reg("a0") struct TagItem *tagList,__reg("a6") void *)="\tjsr\t-48(a6)";
#define AmigaMesaCreateVisual(x1) __AmigaMesaCreateVisual((x1),glBase)
void APIENTRY  __AmigaMesaDestroyVisual(__reg("a0") struct amigamesa_visual *v ,__reg("a6") void *)="\tjsr\t-54(a6)";
#define AmigaMesaDestroyVisual(x1) __AmigaMesaDestroyVisual((x1),glBase)
void APIENTRY  __AmigaMesaMakeCurrent(__reg("a0") struct amigamesa_context *c ,__reg("a1") struct amigamesa_buffer *b,__reg("a6") void *)="\tjsr\t-60(a6)";
#define AmigaMesaMakeCurrent(x1,x2) __AmigaMesaMakeCurrent((x1),(x2),glBase)
void APIENTRY  __AmigaMesaSwapBuffers(__reg("a0") struct amigamesa_context *amesa,__reg("a6") void *)="\tjsr\t-66(a6)";
#define AmigaMesaSwapBuffers(x1) __AmigaMesaSwapBuffers((x1),glBase)
void APIENTRY  __AmigaMesaSetOneColor(__reg("a0") struct amigamesa_context *c,__reg("d0") int index,__reg("fp0") float r,__reg("fp1") float g,__reg("fp2") float b,__reg("a6") void *)="\tjsr\t-72(a6)";
#define AmigaMesaSetOneColor(x1,x2,x3,x4,x5) __AmigaMesaSetOneColor((x1),(x2),(x3),(x4),(x5),glBase)
void APIENTRY  __AmigaMesaSetRast(__reg("a0") struct amigamesa_context *c,__reg("a1") struct TagItem *tagList ,__reg("a6") void *)="\tjsr\t-78(a6)";
#define AmigaMesaSetRast(x1,x2) __AmigaMesaSetRast((x1),(x2),glBase)
void APIENTRY  __AmigaMesaGetConfig(__reg("a0") struct amigamesa_visual *v,__reg("d0") GLenum pname,__reg("a1") GLint* params ,__reg("a6") void *)="\tjsr\t-84(a6)";
#define AmigaMesaGetConfig(x1,x2,x3) __AmigaMesaGetConfig((x1),(x2),(x3),glBase)
