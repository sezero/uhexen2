/*
	compiler.h
	compiler specific definitions and settings
	used in the uhexen2 (Hammer of Thyrion) tree.

	$Id: compiler.h,v 1.3 2007-04-19 09:07:32 sezero Exp $
*/

#ifndef __HX2_COMPILER_H
#define __HX2_COMPILER_H

#if !defined(__GNUC__)
#define	__attribute__(x)
#endif	/* __GNUC__ */

#define _FUNC_PRINTF(n) __attribute__((format (printf, n, n+1)))

#endif	/* __HX2_COMPILER_H */

