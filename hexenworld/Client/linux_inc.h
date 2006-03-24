#ifndef _LINUX_INC
#define _LINUX_INC

#if defined(_WIN32)
#error linux_inc.h is not meant to be used for Windows
#endif

#include <ctype.h>

//#define _inline inline
//#define __inline static inline
#define HANDLE int
#define HINST int
#define HWND int
#define APIENTRY
#define PASCAL
#define FAR
#define SOCKET int
#define LPWSADATA int
#define BOOL int
#define UINT unsigned int
#define WORD unsigned short
#define DWORD unsigned long
#define LONG long
#define LONGLONG long long

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef struct RECT_s {
	int left;
	int right;
	int top;
	int bottom;
} RECT;

typedef union _LARGE_INTEGER { 
	struct {
		DWORD LowPart; 
		LONG  HighPart; 
	} part;
	LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER; 

#endif
