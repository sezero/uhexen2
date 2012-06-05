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

/* easier to include these here */
#include "strl_fn.h"
#include "qsnprint.h"

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

double	COM_GetTime (void);

void	*SafeMalloc (size_t size);

void	COM_Error (const char *error, ...) __attribute__((__format__(__printf__,1,2), __noreturn__));
int	CheckParm (const char *check);

const char	*COM_Parse (const char *data);

int	ParseHex (const char *hex);
int	ParseNum (const char *str);

#endif	/* __CMDLIB_H__	*/

