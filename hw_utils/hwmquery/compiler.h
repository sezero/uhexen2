/*
 * Compiler specific definitions and settings
 * used in the uhexen2 (Hammer of Thyrion) tree.
 */

#ifndef __HX2_COMPILER_H
#define __HX2_COMPILER_H

#ifdef __GNUC__
#define _FUNC_PRINTF(n) __attribute__((format (printf, n, n+1)))
#else
#define _FUNC_PRINTF(n)
#endif	/* __GNUC__ */


#endif	/* __HX2_COMPILER_H */

