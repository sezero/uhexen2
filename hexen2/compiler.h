/*
	compiler.h
	compiler specific definitions and settings
	used in the uhexen2 (Hammer of Thyrion) tree.

	$Id: compiler.h,v 1.5 2007-05-13 11:48:33 sezero Exp $
*/

#ifndef __HX2_COMPILER_H
#define __HX2_COMPILER_H

#if !defined(__GNUC__)
#define	__attribute__(x)
#endif	/* __GNUC__ */


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
#error	__func__ or __FUNCTION__ compiler token not supported? define one...
//#define	__thisfunc__	__FILE__
#endif


#endif	/* __HX2_COMPILER_H */

