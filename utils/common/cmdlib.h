/*
	cmdlib.h

	$Id: cmdlib.h,v 1.18 2007-05-13 12:04:47 sezero Exp $
*/

#ifndef __CMDLIB_H__
#define __CMDLIB_H__

// HEADER FILES ------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#ifdef _WIN32
#define Q_strncasecmp	strnicmp
#define Q_strcasecmp	stricmp
#else
#define Q_strncasecmp	strncasecmp
#define Q_strcasecmp	strcasecmp
#endif

// the dec offsetof macro doesn't work very well...
#define myoffsetof(type,identifier) ((size_t)&((type *)0)->identifier)


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

#define Q_strlcat_err(DST,SRC,SIZE) {								\
	if (Q_strlcat((DST),(SRC),(SIZE)) >= (SIZE))						\
		Error("%s:%d, strlcat: string buffer overflow!",__thisfunc__,__LINE__);		\
}
#define Q_strlcpy_err(DST,SRC,SIZE) {								\
	if (Q_strlcpy((DST),(SRC),(SIZE)) >= (SIZE))						\
		Error("%s:%d, strlcpy: string buffer overflow!",__thisfunc__,__LINE__);		\
}
#if defined(__GNUC__)
#define Q_snprintf_err(DST,SIZE,fmt,args...) {							\
	if (snprintf((DST),(SIZE),fmt,##args) >= (SIZE))					\
		Error("%s:%d, snprintf: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#else
#define Q_snprintf_err(DST,SIZE,...) {								\
	if (snprintf((DST),(SIZE),__VA_ARGS__) >= (SIZE))					\
		Error("%s:%d, snprintf: string buffer overflow!",__thisfunc__,__LINE__);	\
}
#endif


// PUBLIC DATA DECLARATIONS ------------------------------------------------

// set these before calling CheckParm
extern int		myargc;
extern char		**myargv;

extern char	com_token[1024];
extern qboolean		com_eof;


// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

char	*Q_strlwr (char *str);
char	*Q_strupr (char *str);

/*
#ifndef _WIN32
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

