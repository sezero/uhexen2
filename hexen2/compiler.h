/*
	compiler.h
	compiler specific definitions and settings
	used in the uhexen2 (Hammer of Thyrion) tree.

	$Id: compiler.h,v 1.8 2007-10-15 17:45:15 sezero Exp $
*/

#ifndef __HX2_COMPILER_H
#define __HX2_COMPILER_H

#if !defined(__GNUC__)
#define	__attribute__(x)
#endif	/* __GNUC__ */

/* argument format attributes for function
 * pointers are supported for gcc >= 3.1
 */
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0))
#define	__fp_attribute__	__attribute__
#else
#define	__fp_attribute__(x)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define	__thisfunc__	__func__
#elif defined(__GNUC__) && __GNUC__ < 3
#define	__thisfunc__	__FUNCTION__
#elif defined(__GNUC__) && __GNUC__ > 2
#define	__thisfunc__	__func__
#elif defined(__WATCOMC__)
#define	__thisfunc__	__FUNCTION__
#elif defined(__LCC__)
#define	__thisfunc__	__func__
#elif defined(_MSC_VER) && _MSC_VER >= 1300	/* VC7++ */
#define	__thisfunc__	__FUNCTION__
#else	/* stupid fallback */
/*#define	__thisfunc__	__FILE__*/
#error	__func__ or __FUNCTION__ compiler token not supported? define one...
#endif


#endif	/* __HX2_COMPILER_H */

