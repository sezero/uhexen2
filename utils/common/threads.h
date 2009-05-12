/*
	threads.h
	$Id: threads.h,v 1.3 2009-05-12 14:23:12 sezero Exp $
	Copyright (C) 1996-1997  Id Software, Inc.

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
		Boston, MA  02110-1301, USA
*/

#ifndef __H2UTILS_THREADS_H
#define __H2UTILS_THREADS_H

#ifdef PLATFORM_WINDOWS
#include <windows.h>
/* Thread Local Storage definitions */
#if defined(_MSC_VER) /* MS Visual Studio */
#define __threadlocal__ __declspec(thread)
#elif defined(__GNUC__) && ((__GNUC__ > 4) \
  || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
/* gcc >= 4.3, also needs binutils >= 2.19 */
#define __threadlocal__ __thread
#else	/* default to MS definition */
#define __threadlocal__ __declspec(thread)
#endif	/* TLS definitions */
#endif	/* PLATFORM_WINDOWS */

#if defined(__alpha) && defined(PLATFORM_WINDOWS)
	/* FIXME: __alpha shouldn't be needed.. */

extern	HANDLE	my_mutex;
#define	LOCK	WaitForSingleObject (my_mutex, INFINITE)
#define	UNLOCK	ReleaseMutex (my_mutex)

#elif defined(__osf__)	/* __alpha */

#include <pthread.h>
extern	pthread_mutex_t	*my_mutex;
#define LOCK	pthread_mutex_lock (my_mutex)
#define UNLOCK	pthread_mutex_unlock (my_mutex)

#else	/* no threads  */

#define LOCK
#define UNLOCK

#endif	/* __alpha  */

extern	int		numthreads;

typedef void (threadfunc_t) (void *);

void	InitThreads (void);
void	RunThreadsOn (threadfunc_t func);

#endif	/* __H2UTILS_THREADS_H */

