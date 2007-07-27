/*
	common.h
	misc utilities used in client and server

	$Id: common.h,v 1.52 2007-07-27 21:17:18 sezero Exp $
*/

#ifndef __HX2_COMMON_H
#define __HX2_COMMON_H

#if defined(__SOLARIS__)
/* Workaround conflict with /usr/include/sys/model.h */
#define model_t hx2_model_t
#endif

//============================================================================

#if defined(PLATFORM_WINDOWS) && !defined(F_OK)
// values for the mode argument of access(). MS does not define them
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif

#if defined(PLATFORM_WINDOWS)
#define Q_strncasecmp	strnicmp
#define Q_strcasecmp	stricmp
#else
#define Q_strncasecmp	strncasecmp
#define Q_strcasecmp	strcasecmp
#endif

// strlcpy and strlcat :
#include "strl_fn.h"

#if HAVE_STRLCAT && HAVE_STRLCPY

// use native library functions
#define Q_strlcpy strlcpy
#define Q_strlcat strlcat

#else

// use our own copies of strlcpy and strlcat taken from OpenBSD :
extern size_t Q_strlcpy (char *dst, const char *src, size_t size);
extern size_t Q_strlcat (char *dst, const char *src, size_t size);

#endif

#define Q_strlcat_err(DST,SRC,SIZE) {							\
	if (Q_strlcat((DST),(SRC),(SIZE)) >= (SIZE))					\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#define Q_strlcpy_err(DST,SRC,SIZE) {							\
	if (Q_strlcpy((DST),(SRC),(SIZE)) >= (SIZE))					\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#if defined(__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define Q_snprintf_err(DST,SIZE,fmt,args...) {						\
	if (snprintf((DST),(SIZE),fmt,##args) >= (SIZE))				\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#else
#define Q_snprintf_err(DST,SIZE,...) {							\
	if (snprintf((DST),(SIZE),__VA_ARGS__) >= (SIZE))				\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#endif

extern char *Q_strlwr (char *str);
extern char *Q_strupr (char *str);

//============================================================================

extern	char		com_token[1024];
extern	qboolean	com_eof;

char *COM_Parse (char *data);

extern	int		safemode;
/* safe mode: in true, the engine will behave as if one
   of these arguments were actually on the command line:
   -nosound, -nocdaudio, -nomidi, -stdvid, -dibonly,
   -nomouse, -nojoy, -nolan
 */

void COM_Init (void);
int COM_CheckParm (const char *parm);

/* macros for compatibility with quake api */
#define	com_argc	host_parms->argc
#define	com_argv	host_parms->argv

char *COM_SkipPath (char *pathname);
void COM_StripExtension (const char *in, char *out);
void COM_FileBase (const char *in, char *out);
void COM_DefaultExtension (char *path, const char *extension, size_t len);

char	*va (const char *format, ...) __attribute__((format(printf,1,2)));
// does a varargs printf into a temp buffer. cycles between
// 4 different static buffers. the number of buffers cycled
// is defined in VA_NUM_BUFFS.

int COM_StrCompare (const void *arg1, const void *arg2);
// quick'n'dirty string comparison function for use with qsort


#endif	/* __HX2_COMMON_H */

