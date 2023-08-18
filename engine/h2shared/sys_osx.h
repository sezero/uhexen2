/* Mac OS X specifics needed by common sys_unix.c : */

#ifndef SYS_OSX_H
#define SYS_OSX_H

#ifdef __cplusplus
extern "C" {
#endif

int OSX_GetBasedir (char *argv0, char *dst, size_t dstsize);
#define Sys_GetBasedir		OSX_GetBasedir

void Cocoa_ErrorMessage (const char *errorMsg);
#define Sys_ErrorMessage	Cocoa_ErrorMessage

#define Sys_GetClipboardData	Sys_GetClipboardData
	/* this is public, therefore not OSX_GetClipboardData  */

#ifdef __cplusplus
}
#endif

#endif /* SYS_OSX_H */

