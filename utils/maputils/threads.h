
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __alpha

#  ifdef _WIN32
extern void* my_mutex;
#define	LOCK	WaitForSingleObject (my_mutex, INFINITE)
#define	UNLOCK	ReleaseMutex (my_mutex)

#  else //_win32

#include <pthread.h>
extern  pthread_mutex_t *my_mutex;
#define LOCK	pthread_mutex_lock (my_mutex)
#define UNLOCK	pthread_mutex_unlock (my_mutex)

#  endif //_win32

#else // __alpha

#define LOCK
#define UNLOCK

#endif // __alpha

extern	int		numthreads;

typedef void (threadfunc_t) (void *);

void	InitThreads (void);
void	RunThreadsOn ( threadfunc_t func );

