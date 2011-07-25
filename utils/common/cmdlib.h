/*
	cmdlib.h

	$Id$
*/

#ifndef __CMDLIB_H__
#define __CMDLIB_H__

// HEADER FILES ------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// MACROS ------------------------------------------------------------------

#undef	min
#undef	max
#define	q_min(a, b)	(((a) < (b)) ? (a) : (b))
#define	q_max(a, b)	(((a) > (b)) ? (a) : (b))

#if defined(PLATFORM_WINDOWS)
#define q_strncasecmp	_strnicmp
#define q_strcasecmp	_stricmp
#elif defined(PLATFORM_DOS)
#define q_strncasecmp	strnicmp
#define q_strcasecmp	stricmp
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

/* snprintf, vsnprintf : always use our versions. */
/* platform dependant (v)snprintf function names: */
#if defined(PLATFORM_WINDOWS)
#define	snprintf_func		_snprintf
#define	vsnprintf_func		_vsnprintf
#else
#define	snprintf_func		snprintf
#define	vsnprintf_func		vsnprintf
#endif

extern int q_snprintf (char *str, size_t size, const char *format, ...) __attribute__((__format__(__printf__,3,4)));
extern int q_vsnprintf(char *str, size_t size, const char *format, va_list args)
									__attribute__((__format__(__printf__,3,0)));

/* these qerr_ versions of functions error out if they detect, well, an error.
 * their first two arguments must the name of the caller function (see compiler.h
 * for the __thisfunc__ macro) and the line number, which should be __LINE__ .
 */
extern size_t qerr_strlcat (const char *caller, int linenum, char *dst, const char *src, size_t size);
extern size_t qerr_strlcpy (const char *caller, int linenum, char *dst, const char *src, size_t size);
extern int qerr_snprintf (const char *caller, int linenum, char *str, size_t size, const char *format, ...)
									__attribute__((__format__(__printf__,5,6)));


// PUBLIC DATA DECLARATIONS ------------------------------------------------

/* set myargc and myargv properly
 * before calling CheckParm()  */
extern int		myargc;
extern char		**myargv;

extern char	com_token[1024];
extern qboolean		com_eof;


// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

char	*q_strlwr (char *str);
char	*q_strupr (char *str);

double	GetTime (void);

void	*SafeMalloc (size_t size);

void	Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
int	CheckParm (const char *check);

const char	*COM_Parse (const char *data);

int	ParseHex (const char *hex);
int	ParseNum (const char *str);

#endif	/* __CMDLIB_H__	*/

