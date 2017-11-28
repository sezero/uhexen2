/*
 * threads.c
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2011-2012 O.Sezer <sezero@users.sourceforge.net>
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
	OSVERSIONINFO vinfo;
	int numcpus;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);
	if (!GetVersionEx (&vinfo))
		COM_Error ("Couldn't get OS info");
	if (vinfo.dwMajorVersion < 4 || vinfo.dwPlatformId < VER_PLATFORM_WIN32_NT)
		return 1;
	GetSystemInfo(&info);
	numcpus = info.dwNumberOfProcessors;
	return (numcpus < 1) ? 1 : numcpus;
}

#elif defined(__linux__) || defined(__sun) || defined(sun) || defined(_AIX)
#include <unistd.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	return (numcpus < 1) ? 1 : numcpus;
}

#elif defined(PLATFORM_OSX)
#include <unistd.h>
#include <sys/sysctl.h>
#if !defined(HW_AVAILCPU)	/* using an ancient SDK? */
#define HW_AVAILCPU		25	/* needs >= 10.2 */
#endif
int Thread_GetNumCPUS (void)
{
	int numcpus;
	int mib[2];
	size_t len;

#if defined(_SC_NPROCESSORS_ONLN)	/* needs >= 10.5 */
	numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (numcpus != -1)
		return (numcpus < 1) ? 1 : numcpus;
#endif
	len = sizeof(numcpus);
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;
	sysctl(mib, 2, &numcpus, &len, NULL, 0);
	if (sysctl(mib, 2, &numcpus, &len, NULL, 0) == -1)
	{
		mib[1] = HW_NCPU;
		if (sysctl(mib, 2, &numcpus, &len, NULL, 0) == -1)
			return 1;
	}
	return (numcpus < 1) ? 1 : numcpus;
}

#elif defined(__sgi) || defined(sgi) || defined(__sgi__) /* IRIX */
#include <unistd.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = sysconf(_SC_NPROC_ONLN);
	if (numcpus < 1)
		numcpus = 1;
	return numcpus;
}

#elif defined(PLATFORM_BSD)
#include <unistd.h>
#include <sys/sysctl.h>
int Thread_GetNumCPUS (void)
{
	int numcpus;
	int mib[2];
	size_t len;

#if defined(_SC_NPROCESSORS_ONLN)
	numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (numcpus != -1)
		return (numcpus < 1) ? 1 : numcpus;
#endif
	len = sizeof(numcpus);
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	if (sysctl(mib, 2, &numcpus, &len, NULL, 0) == -1)
		return 1;
	return (numcpus < 1) ? 1 : numcpus;
}

#elif defined(__hpux) || defined(__hpux__) || defined(_hpux)
#include <sys/mpctl.h>
int Thread_GetNumCPUS (void)
{
	int numcpus = mpctl(MPC_GETNUMSPUS, NULL, NULL);
	return numcpus;
}

#elif defined(PLATFORM_OS2)
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#ifndef QSV_NUMPROCESSORS
#define QSV_NUMPROCESSORS 26
#endif
int Thread_GetNumCPUS (void)
{
	int numcpus = 1;
	DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, &numcpus, sizeof(numcpus));
	return (numcpus < 1) ? 1 : numcpus;
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

#define DEFAULT_STACKSIZ	0x100000	/* 1MB is enough for most */

#if defined(PLATFORM_WINDOWS)

#include <windows.h>
#include <process.h>

static int	numthreads;
static size_t	stacksiz;
static HANDLE		my_mutex;
static threadfunc_t	workfunc;

static unsigned __stdcall ThreadWorkerFunc (void *threadnum)
{
	workfunc (threadnum);
	return 0;
}

void InitThreads (int wantthreads, size_t needstack)
{
	if (needstack != 0)
		stacksiz = needstack;
	else	stacksiz = DEFAULT_STACKSIZ;

	numthreads = wantthreads;
	if (numthreads < 0)
		numthreads = Thread_GetNumCPUS ();
	if (numthreads < 1)
		numthreads = 1;
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	printf ("Setup for %d threads, 0x%x stack size\n",
			numthreads, (unsigned int)stacksiz);
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
	unsigned	IDThread;
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
		work_threads[i] = (HANDLE) _beginthreadex(NULL, /* no security attributes */
			stacksiz,			/* stack size */
			ThreadWorkerFunc,		/* thread function */
			(void *) i,			/* thread function arg */
			0,				/* run immediately */
			&IDThread);

		if (!work_threads[i])
			COM_Error ("_beginthreadex () failed");
	}

	for (i = 0; i < numthreads; i++)
	{
		WaitForSingleObject(work_threads[i], INFINITE);
	}
}


#elif (defined(__sgi) || defined(sgi) || defined(__sgi__)) \
   && !defined(USE_PTHREADS) /* original IRIX code of Quake */

#include <task.h>
#include <abi_mutex.h>
#include <sys/types.h>
#include <sys/prctl.h>

static int	numthreads;
static size_t	stacksiz;
static abilock_t	lck;

static void ThreadWorkerFunc (void *threadnum, size_t stksize)
{
	workfunc (threadnum);
}

void InitThreads (int wantthreads, size_t needstack)
{
	if (needstack != 0)
		stacksiz = needstack;
	else	stacksiz = DEFAULT_STACKSIZ;

	numthreads = wantthreads;
	if (numthreads < 0)
		numthreads = prctl(PR_MAXPPROCS);
	if (numthreads < 1)
		numthreads = 1;
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	printf ("Setup for %d threads, 0x%x stack size\n",
			numthreads, (unsigned int)stacksiz);
	if (numthreads <= 1)
		return;
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
				  NULL, stacksiz);
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

static int	numthreads;
static size_t	stacksiz;
static pthread_mutex_t	*my_mutex;
static threadfunc_t	workfunc;

static pthread_addr_t ThreadWorkerFunc (pthread_addr_t threadnum)
{
	workfunc ((void *)threadnum);
	return NULL;
}

void InitThreads (int wantthreads, size_t needstack)
{
	pthread_mutexattr_t	mattrib;

	if (needstack != 0)
		stacksiz = needstack;
	else	stacksiz = DEFAULT_STACKSIZ;

	numthreads = wantthreads;
	if (numthreads < 0)
		numthreads = 4;
	if (numthreads < 1)
		numthreads = 1;
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	printf ("Setup for %d threads, 0x%x stack size\n",
			numthreads, (unsigned int)stacksiz);
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
	if (pthread_attr_setstacksize (&attrib, stacksiz) == -1)
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

static int	numthreads;
static size_t	stacksiz;
static pthread_mutex_t	*my_mutex;
static threadfunc_t	workfunc;

static void *ThreadWorkerFunc (void *threadnum)
{
	workfunc (threadnum);
	return NULL;
}

void InitThreads (int wantthreads, size_t needstack)
{
	pthread_mutexattr_t mattrib;

	if (needstack != 0)
		stacksiz = needstack;
	else	stacksiz = DEFAULT_STACKSIZ;

	numthreads = wantthreads;
	if (numthreads < 0)
		numthreads = Thread_GetNumCPUS ();
	if (numthreads < 1)
		numthreads = 1;
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	printf ("Setup for %d threads, 0x%x stack size\n",
			numthreads, (unsigned int)stacksiz);
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
	if (pthread_attr_setstacksize (&attrib, stacksiz) == -1)
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


#elif defined(PLATFORM_OS2)

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include <process.h>

static int	work_threads[MAX_THREADS];
static int	numthreads;
static unsigned int	stacksiz;
static HMTX	my_mutex = NULLHANDLE;
static threadfunc_t	workfunc;

static void ThreadWorkerFunc (void *threadnum)
{
	LONG i = (LONG) threadnum;
	workfunc (threadnum);
	work_threads[i] = -1;
	/* the C library automatically calls
	 * _endthread() when we return here. */
}

void InitThreads (int wantthreads, size_t needstack)
{
	APIRET		rc;

	if (needstack != 0)
		stacksiz = needstack;
	else	stacksiz = DEFAULT_STACKSIZ;

	numthreads = wantthreads;
	if (numthreads < 0)
		numthreads = Thread_GetNumCPUS ();
	if (numthreads < 1)
		numthreads = 1;
	if (numthreads > MAX_THREADS)
		numthreads = MAX_THREADS;

	printf ("Setup for %d threads, 0x%x stack size\n",
			numthreads, (unsigned int)stacksiz);
	if (numthreads <= 1)
		return;

	if ((rc = DosCreateMutexSem(NULL, &my_mutex, 0, 0)) != NO_ERROR)
		COM_Error("CreateMutexSem failed (%lu)", rc);
}

void ThreadLock (void)
{
	if (numthreads > 1)
		DosRequestMutexSem (my_mutex, SEM_INDEFINITE_WAIT);
}

void ThreadUnlock (void)
{
	if (numthreads > 1)
		DosReleaseMutexSem (my_mutex);
}

/*
===============
RunThreadsOn
===============
*/
void RunThreadsOn (threadfunc_t func)
{
	LONG		i;

	if (numthreads <= 1)
	{
		work_threads[0] = -1;
		func (NULL);
		return;
	}

	workfunc = func;

	for (i = 0; i < numthreads; i++)
	{
		work_threads[i] = _beginthread(ThreadWorkerFunc, NULL, stacksiz, (void *)i);
		if (work_threads[i] == -1)
			COM_Error ("_beginthread() failed");
	}

	for (i = 0; i < numthreads; i++)
	{
		while (work_threads[i] != -1)
			DosSleep (100);
	}
}


#else	/* no threads  */

void InitThreads (int wantthreads, size_t needstack)
{
	printf ("Single-threaded at compile time\n");
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

