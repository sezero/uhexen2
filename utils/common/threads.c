/*
 * threads.c
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "threads.h"


/* Thread_GetNumCPUS () -- see:
 * http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
 */
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
int Thread_GetNumCPUS (void)
{
	SYSTEM_INFO info;
	int numcpus;
	GetSystemInfo(&info);
	numcpus = info.dwNumberOfProcessors;
	if (numcpus < 1)
		numcpus = 1;
	return numcpus;
}

#elif defined(__linux__) || defined(__SOLARIS__) || defined(_AIX)
#include <unistd.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (numcpus < 1)
		numcpus = 1;
	return numcpus;
}

#elif defined(__MACOSX) /* needs >= 10.4 */
#include <unistd.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (numcpus < 1)
		numcpus = 1;
	return numcpus;
}

#elif defined(__IRIX__)
#include <unistd.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = sysconf(_SC_NPROC_ONLN);
	if (numcpus < 1)
		numcpus = 1;
	return numcpus;
}

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/sysctl.h>
int Thread_GetNumCPUS (void)
{
	int numcpus, mib[4];
	size_t len = sizeof(numcpus);
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;
	sysctl(mib, 2, &numcpus, &len, NULL, 0);
	if (numcpus < 1)
	{
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &numcpus, &len, NULL, 0);
		if (numcpus < 1)
			numcpus = 1;
	}
	return numcpus;
}

#elif defined(__hpux) || defined(__hpux__) || defined(_hpux)
#include <sys/mpctl.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = mpctl(MPC_GETNUMSPUS, NULL, NULL);
	return numcpus;
}

#elif defined(PLATFORM_DOS)
int Thread_GetNumCPUS (void)
{
	return 1;
}

#else /* unknown OS */
int Thread_GetNumCPUS (void)
{
	return -2;
}
#endif /* GetNumCPUS */


/* THREAD FUNCTIONS: */

#if defined(PLATFORM_WINDOWS)

#include <windows.h>

int		numthreads = 1;
static HANDLE		my_mutex;
static threadfunc_t	workfunc;

static DWORD WINAPI ThreadWorkerFunc (LPVOID threadnum)
{
	workfunc ((void *)threadnum);
	return 0;
}

void InitThreads (void)
{
	if (numthreads == -1)
		numthreads = Thread_GetNumCPUS ();
	if (numthreads <= 1)
		return;

	my_mutex = CreateMutex(NULL, FALSE, NULL);
	if (my_mutex == NULL)
		COM_Error("CreateMutex failed");
}

void ThreadLock (void)
{
	if (numthreads > 1)
		WaitForSingleObject (my_mutex, INFINITE);
}

void ThreadUnlock (void)
{
	if (numthreads > 1)
		ReleaseMutex (my_mutex);
}

/*
===============
RunThreadsOn
===============
*/
void RunThreadsOn (threadfunc_t func)
{
	DWORD	IDThread;
	HANDLE	work_threads[MAX_THREADS];
	INT_PTR		i;

	if (numthreads <= 1)
	{
		func (NULL);
		return;
	}

	workfunc = func;

	for (i = 0; i < numthreads; i++)
	{
		work_threads[i] = CreateThread(NULL,	/* no security attrib */
			0x100000,			/* stack size */
			ThreadWorkerFunc,		/* thread function */
			(LPVOID) i,			/* thread function arg */
			0,				/* use default creation flags */
			&IDThread);

		if (work_threads[i] == NULL)
			COM_Error ("pthread_create failed");
	}

	for (i = 0; i < numthreads; i++)
	{
		WaitForSingleObject(work_threads[i], INFINITE);
	}
}


#elif defined(__IRIX__) /* defined(_MIPS_ISA) */ && !defined(USE_PTHREADS)

#include <task.h>
#include <abi_mutex.h>
#include <sys/types.h>
#include <sys/prctl.h>

int		numthreads = -1;
static abilock_t	lck;

static void ThreadWorkerFunc (void *threadnum, size_t stksize)
{
	workfunc (threadnum);
}

void InitThreads (void)
{
	if (numthreads == -1)
	{
		numthreads = prctl(PR_MAXPPROCS);
		if (numthreads > MAX_THREADS)
			numthreads = MAX_THREADS;
	}

	if (numthreads > 1)
		usconfig (CONF_INITUSERS, numthreads);
}

void ThreadLock (void)
{
	if (numthreads > 1)
		spin_lock (&lck);
}

void ThreadUnlock (void)
{
	if (numthreads > 1)
		release_lock (&lck);
}

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (threadfunc_t func)
{
	pid_t		pid[MAX_THREADS];
	long		i;

	if (numthreads <= 1)
	{
		func (NULL);
		return;
	}

	init_lock (&lck);
	workfunc = func;

	for (i = 0; i < numthreads - 1; i++)
	{
		pid[i] = sprocsp (ThreadWorkerFunc, PR_SALL, (void *)i,
				  NULL, 0x100000);	/* 1 MB stacks */
		if (pid[i] == -1)
		{
			perror ("sproc");
			COM_Error ("sproc failed");
		}
	}

	func ((void *)i);

	for (i = 0; i < numthreads - 1; i++)
		wait (NULL);
}


#elif defined(USE_PTHREADS)

#if defined(__osf__)
/* original OSF/1 code of Quake */

#include <pthread.h>

int		numthreads = 4;
static pthread_mutex_t	*my_mutex;
static threadfunc_t	workfunc;

static pthread_addr_t ThreadWorkerFunc (pthread_addr_t threadnum)
{
	workfunc ((void *)threadnum);
	return NULL;
}

void InitThreads (void)
{
	pthread_mutexattr_t	mattrib;

	if (numthreads <= 1)
		return;

	my_mutex = (pthread_mutex_t *) SafeMalloc (sizeof(*my_mutex));
	if (pthread_mutexattr_create (&mattrib) == -1)
		COM_Error ("pthread_mutex_attr_create failed");
	if (pthread_mutexattr_setkind_np (&mattrib, MUTEX_FAST_NP) == -1)
		COM_Error ("pthread_mutexattr_setkind_np failed");
	if (pthread_mutex_init (my_mutex, mattrib) == -1)
		COM_Error ("pthread_mutex_init failed");
}

void ThreadLock (void)
{
	if (numthreads > 1)
		pthread_mutex_lock (my_mutex);
}

void ThreadUnlock (void)
{
	if (numthreads > 1)
		pthread_mutex_unlock (my_mutex);
}

/*
===============
RunThreadsOn
===============
*/
void RunThreadsOn (threadfunc_t func)
{
	pthread_t	work_threads[MAX_THREADS];
	pthread_addr_t	status;
	pthread_attr_t	attrib;
	long		i;

	if (numthreads <= 1)
	{
		func (NULL);
		return;
	}

	workfunc = func;

	if (pthread_attr_create (&attrib) == -1)
		COM_Error ("pthread_attr_create failed");
	if (pthread_attr_setstacksize (&attrib, 0x100000) == -1)
		COM_Error ("pthread_attr_setstacksize failed");

	for (i = 0; i < numthreads; i++)
	{
		if (pthread_create(&work_threads[i], attrib,
					ThreadWorkerFunc,
					(pthread_addr_t)i) == -1)
			COM_Error ("pthread_create failed");
	}

	for (i = 0; i < numthreads; i++)
	{
		if (pthread_join (work_threads[i], &status) == -1)
			COM_Error ("pthread_join failed");
	}
}

#else /* common pthreads: */

#include <pthread.h>

int		numthreads = 1;
static pthread_mutex_t	*my_mutex;
static threadfunc_t	workfunc;

static void *ThreadWorkerFunc (void *threadnum)
{
	workfunc (threadnum);
	return NULL;
}

void InitThreads (void)
{
	pthread_mutexattr_t mattrib;

	if (numthreads <= 1)
		return;

	my_mutex = (pthread_mutex_t *) SafeMalloc (sizeof (*my_mutex));
	if (pthread_mutexattr_init (&mattrib) == -1)
		COM_Error ("pthread_mutex_attr_init failed");
	if (pthread_mutex_init (my_mutex, &mattrib) == -1)
		COM_Error ("pthread_mutex_init failed");
}

void ThreadLock (void)
{
	if (numthreads > 1)
		pthread_mutex_lock (my_mutex);
}

void ThreadUnlock (void)
{
	if (numthreads > 1)
		pthread_mutex_unlock (my_mutex);
}

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (threadfunc_t func)
{
	pthread_t	work_threads[MAX_THREADS];
	void		*status;
	pthread_attr_t	attrib;
	long		i;

	if (numthreads <= 1)
	{
		func (NULL);
		return;
	}

	if (pthread_attr_init (&attrib) == -1)
		COM_Error ("pthread_attr_init failed");
	if (pthread_attr_setstacksize (&attrib, 0x100000) == -1)
		COM_Error ("pthread_attr_setstacksize failed");

	workfunc = func;

	for (i = 0; i < numthreads; i++)
	{
		if (pthread_create (&work_threads[i], &attrib,
					ThreadWorkerFunc,
					(void *)i) == -1)
			COM_Error ("pthread_create failed");
	}

	for (i = 0; i < numthreads; i++)
	{
		if (pthread_join (work_threads[i], &status) == -1)
			COM_Error ("pthread_join failed");
	}
}
#endif	/* USE_PTHREADS */


#else	/* no threads  */

int		numthreads = 1;


void InitThreads (void)
{
	/* ( nothing ) */
}

void ThreadLock (void)
{
	/* ( nothing ) */
}

void ThreadUnlock (void)
{
	/* ( nothing ) */
}

/*
===============
RunThreadsOn
===============
*/
void RunThreadsOn (threadfunc_t func)
{
	func (NULL);
}

#endif	/* no threads  */

