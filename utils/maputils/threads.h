/*
	threads.h
	$Id: threads.h,v 1.5 2007-07-11 16:47:21 sezero Exp $
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

