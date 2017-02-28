
/* different MSVC names for <io.h> functions:
 * new MSVC versions spit deprecation warnings.
 * make SURE to include this AFTER you include
 * io.h and/or conio.h
 */

/* when it is windows-only code, we use the
 * _prefixed versions, but not all compiler
 * suits support all stuff, hence this header.
 */

/* MSVC compiler favors the _prefixed ones.
 *
 * MinGW is compatible with that, so it's OK
 * with this, too.
 *
 * LCC has similar behavior: even if some of
 * the functions are declared without prefix,
 * it adds defines to help the situation most
 * of the time.
 *
 * OpenWatcom 1.7 and older doesn't have most
 * of the prefixed ones, but 1.8 does.
 */

#ifndef _CRT_QUIRKS
#define _CRT_QUIRKS

#if (defined(_WIN32) || defined(_WIN64))

#if defined(_MSC_VER) || defined(__GNUC__)
/* io.h stuff: */
#  define read		_read
#  define write		_write
#  define open		_open
#  define close		_close
#  define dup		_dup
#  define access	_access
#  define rmdir		_rmdir
#  define unlink	_unlink
/* conio.h stuff: */
#  define kbhit		_kbhit
#  define getch		_getch
#  define getche	_getche
#  define putch		_putch
#  define ungetch	_ungetch
#endif	/* _MSC_VER, __GNUC__ */

#if defined(__LCC__)
#  define _rmdir	rmdir
#endif	/* __LCC__ */

#if defined(__WATCOMC__)
#  define _putch	putch
#if (__WATCOMC__ < 1280)
#  define _read		read
#  define _write	write
#  define _unlink	unlink
#  define _kbhit	kbhit
#  define _ungetch	ungetch
#endif
#endif	/* __WATCOMC__ */

#endif	/* _WIN32/_WIN64 */
#endif	/* _CRT_QUIRKS */

