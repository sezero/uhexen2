/*
	compiler.h
	compiler specific definitions and settings
	used in the uhexen2 (Hammer of Thyrion) tree.
	- standalone header
	- doesn't and must not include any other headers
	- shouldn't depend on arch_def.h, q_stdinc.h, or
	  any other headers

	$Id: compiler.h,v 1.9 2007-12-14 16:41:13 sezero Exp $

	Copyright (C) 2007  O.Sezer <sezero@users.sourceforge.net>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301  USA
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

