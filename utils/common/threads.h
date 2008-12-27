/*
	threads.h
	$Id: threads.h,v 1.1 2008-12-27 17:15:33 sezero Exp $
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
#endif

#ifdef __alpha

#  ifdef PLATFORM_WINDOWS
extern void* my_mutex;
#define	LOCK	WaitForSingleObject (my_mutex, INFINITE)
#define	UNLOCK	ReleaseMutex (my_mutex)

#  else /* windows */

#include <pthread.h>
extern  pthread_mutex_t *my_mutex;
#define LOCK	pthread_mutex_lock (my_mutex)
#define UNLOCK	pthread_mutex_unlock (my_mutex)

#  endif /* windows */

#else	/* __alpha  */

#define LOCK
#define UNLOCK

#endif	/* __alpha  */

extern	int		numthreads;

typedef void (threadfunc_t) (void *);

void	InitThreads (void);
void	RunThreadsOn ( threadfunc_t func );

#endif	/* __H2UTILS_THREADS_H */

