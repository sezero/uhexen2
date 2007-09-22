/*
	cmdlib.h

	$Id: cmdlib.h,v 1.20 2007-09-22 15:27:36 sezero Exp $
*/

#ifndef __CMDLIB_H__
#define __CMDLIB_H__

// HEADER FILES ------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// MACROS ------------------------------------------------------------------

/* the dec offsetof macro doesn't work very well... */
#define myoffsetof(type,identifier) ((size_t)&((type *)0)->identifier)

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

#define qerr_strlcat(DST,SRC,SIZE) {						\
	if (q_strlcat((DST),(SRC),(SIZE)) >= (SIZE))				\
		Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#define qerr_strlcpy(DST,SRC,SIZE) {						\
	if (q_strlcpy((DST),(SRC),(SIZE)) >= (SIZE))				\
		Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#if defined(__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define qerr_snprintf(DST,SIZE,fmt,args...) {					\
	if (q_snprintf((DST),(SIZE),fmt,##args) >= (SIZE))			\
		Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#else
#define qerr_snprintf(DST,SIZE,...) {						\
	if (q_snprintf((DST),(SIZE),__VA_ARGS__) >= (SIZE))			\
		Error("%s: %d: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#endif


// PUBLIC DATA DECLARATIONS ------------------------------------------------

// set these before calling CheckParm
extern int		myargc;
extern char		**myargv;

extern char	com_token[1024];
extern qboolean		com_eof;


// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

char	*q_strlwr (char *str);
char	*q_strupr (char *str);

/*
#ifdef PLATFORM_UNIX
int	Sys_kbhit(void);
#endif
*/

double	GetTime (void);

void	Error (const char *error, ...) __attribute__((format(printf,1,2), noreturn));
int	CheckParm (const char *check);

char	*COM_Parse (char *data);

int	ParseHex (const char *hex);
int	ParseNum (const char *str);

#endif	/* __CMDLIB_H__	*/

