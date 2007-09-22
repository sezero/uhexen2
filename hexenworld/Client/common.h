/*
	common.h
	misc utilities used in client and server

	$Id: common.h,v 1.51 2007-09-22 15:27:17 sezero Exp $
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
#define q_strncasecmp	_strnicmp
#define q_strcasecmp	_stricmp
#else
#define q_strncasecmp	strncasecmp
#define q_strcasecmp	strcasecmp
#endif

/* strlcpy and strlcat : */
#include "strl_fn.h"

#if HAVE_STRLCAT && HAVE_STRLCPY
/* use native library functions */
#define q_strlcpy	strlcpy
#define q_strlcat	strlcat
#else
/* use our own copies of strlcpy and strlcat taken from OpenBSD */
extern size_t q_strlcpy (char *dst, const char *src, size_t size);
extern size_t q_strlcat (char *dst, const char *src, size_t size);
#endif

/* snprintf and vsnprintf : */
#undef	SNPRINTF_RETURNS_NEGATIVE
#undef	SNPRINTF_DOESNT_TERMINATE

/* platforms where (v)snprintf implementations return
   a negative value upon error: DOS (DJGPP v2.0.4) and
   Windows does that. Add more here. */
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_DOS)
#define	SNPRINTF_RETURNS_NEGATIVE	1
#endif

/* platform where (v)snprintf implementations may not
   null-terminate the given buffer upon truncation :
   Windows does that. Add more here. */
#if defined(PLATFORM_WINDOWS)
#define	SNPRINTF_DOESNT_TERMINATE	1
#endif

/* platform dependant (v)snprintf function names: */
#if defined(PLATFORM_WINDOWS)
#define	snprintf_func		_snprintf
#define	vsnprintf_func		_vsnprintf
#else
#define	snprintf_func		snprintf
#define	vsnprintf_func		vsnprintf
#endif

#if defined(SNPRINTF_RETURNS_NEGATIVE) || defined(SNPRINTF_DOESNT_TERMINATE)
extern int q_snprintf (char *str, size_t size, const char *format, ...) __attribute__((format(printf,3,4)));
extern int q_vsnprintf(char *str, size_t size, const char *format, va_list args);
#else
#define	q_snprintf		snprintf_func
#define	q_vsnprintf		vsnprintf_func
#endif

#define qerr_strlcat(DST,SRC,SIZE) {							\
	if (q_strlcat((DST),(SRC),(SIZE)) >= (SIZE))					\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#define qerr_strlcpy(DST,SRC,SIZE) {							\
	if (q_strlcpy((DST),(SRC),(SIZE)) >= (SIZE))					\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#if defined(__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define qerr_snprintf(DST,SIZE,fmt,args...) {						\
	if (q_snprintf((DST),(SIZE),fmt,##args) >= (SIZE))				\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#else
#define qerr_snprintf(DST,SIZE,...) {							\
	if (q_snprintf((DST),(SIZE),__VA_ARGS__) >= (SIZE))				\
		Sys_Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#endif

extern char *q_strlwr (char *str);
extern char *q_strupr (char *str);

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

