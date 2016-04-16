/*
 * $Id: log.h,v 1.1.1.1 2000/04/07 19:44:51 tfrieden Exp $
 *
 * $Date: 2000/04/07 19:44:51 $
 * $Revision: 1.1.1.1 $
 *
 * (C) 1999 by Hyperion
 * All rights reserved
 *
 * This file is part of the MiniGL library project
 * See the file Licence.txt for more details
 *
 */

#ifndef __LOG_H
#define __LOG_H

/*
** Logging was removed in final version
*/

/*
** It is probably difficult to support logging of gl calls on other compilers,
** since the egcs/gcc preprocessor supports variable argument macros, and
** I am using it :)
*/

/*
#ifdef NLOGGING
#define LOG(level, func, format, args...)
#else
#ifdef __PPC__
extern int MGLDebugLevel;
#define LOG(level, func, format, args...) \
	if (MGLDebugLevel >= level)            \
	{                                       \
		kprintf("[MiniGL::%s] ", #func);     \
		kprintf(format , __VA_ARGS__ );           \
		kprintf("\n");                         \
	}
#else
extern int MGLDebugLevel;
#define LOG(level, func, format, args...) \
	if (MGLDebugLevel >= level)            \
	{                                       \
		mykprintf("[MiniGL::%s] ", #func);   \
		mykprintf(format , __VA_ARGS__ );         \
		mykprintf("\n");                       \
	}
#endif

#endif

*/
#endif
