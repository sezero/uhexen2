// WGL functions we may need to link to

#ifdef GL_DLSYM
// These we can't do without
typedef PROC (WINAPI *wglGetProcAddress_f)(LPCSTR);
typedef HGLRC (WINAPI *wglCreateContext_f)(HDC);
typedef BOOL (WINAPI *wglDeleteContext_f)(HGLRC);
typedef BOOL (WINAPI *wglMakeCurrent_f)(HDC, HGLRC);
typedef HGLRC (WINAPI *wglGetCurrentContext_f)(VOID);
typedef HDC (WINAPI *wglGetCurrentDC_f)(VOID);

wglGetProcAddress_f wglGetProcAddress_fp;
wglCreateContext_f wglCreateContext_fp;
wglDeleteContext_f wglDeleteContext_fp;
wglMakeCurrent_f wglMakeCurrent_fp;
wglGetCurrentContext_f wglGetCurrentContext_fp;
wglGetCurrentDC_f wglGetCurrentDC_fp;

#else

#define wglGetProcAddress_fp(a)		wglGetProcAddress((a))
#define wglCreateContext_fp(a)		wglCreateContext((a))
#define wglDeleteContext_fp(a)		wglDeleteContext((a))
#define wglMakeCurrent_fp(a,b)		wglMakeCurrent((a),(b))
#define wglGetCurrentContext_fp()	wglGetCurrentContext()
#define wglGetCurrentDC_fp()		wglGetCurrentDC()

//#define wglGetExtensionsStringARB_fp(a)	wglGetExtensionsStringARB((a))
//#define wglSwapBuffers_fp(a)		wglSwapBuffers((a))
//#define wglSwapIntervalEXT_fp(a)	wglSwapIntervalEXT((a))

#endif

// These are optional
typedef const char *(WINAPI *wglGetExtensionsStringARB_f)(HDC hdc);
typedef BOOL (WINAPI *wglSwapBuffers_f)(HDC);
typedef BOOL (WINAPI *wglSwapIntervalEXT_f)(int interval);
wglGetExtensionsStringARB_f wglGetExtensionsStringARB_fp;
wglSwapBuffers_f wglSwapBuffers_fp;
wglSwapIntervalEXT_f wglSwapIntervalEXT_fp;
