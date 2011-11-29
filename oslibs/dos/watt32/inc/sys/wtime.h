/*!\file sys/wtime.h
 *
 * Watt-32 time functions.
 */

#ifndef __SYS_WTIME_H
#define __SYS_WTIME_H

/*
 * The naming <sys/wtime.h> is required for those compilers that
 * have <sys/time.h> in the usual place but doesn't define
 * the following.
 */

#include <time.h>

#ifdef __BORLANDC__
#undef timezone   /* a macro in bcc 5+ */
#endif

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#if defined(__DJGPP__)
  #include <sys/time.h>
  #include <sys/times.h>

#elif defined(__MINGW32__)
  #include <sys/time.h>

#else
  struct timeval {
         time_t tv_sec;
         long   tv_usec;
       };
  #define STRUCT_TIMEVAL_DEFINED
#endif

#if !defined(__DJGPP__)
  struct timezone {
         int tz_minuteswest;
         int tz_dsttime;
       };

  struct tms {
         unsigned long tms_utime;
         unsigned long tms_cstime;
         unsigned long tms_cutime;
         unsigned long tms_stime;
       };
  #define STRUCT_TIMEZONE_DEFINED
  #define STRUCT_TMS_DEFINED

  __BEGIN_DECLS

  #define ITIMER_REAL  0
  #define ITIMER_PROF  1

  struct itimerval {
         struct timeval it_interval;  /* timer interval */
         struct timeval it_value;     /* current value */
       };

  W32_FUNC int getitimer (int, struct itimerval *);
  W32_FUNC int setitimer (int, struct itimerval *, struct itimerval *);

  W32_FUNC int gettimeofday (struct timeval *tp, struct timezone *tz);

  __END_DECLS

#endif      /* !__DJGPP__ */

#if !defined(_STRUCT_TIMESPEC) && !defined(_pthread_signal_h)
  #define _STRUCT_TIMESPEC
  struct timespec {
         time_t tv_sec;
         long   tv_nsec;
       };
#endif

#ifndef HZ
#define HZ 18.2F
#endif

__BEGIN_DECLS

W32_FUNC unsigned long net_times (struct tms *buffer);
W32_FUNC int           gettimeofday2 (struct timeval *tv, struct timezone *tz);

__END_DECLS

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#ifndef timerisset
  #define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)

  #define timercmp(tvp, uvp, cmp) ((tvp)->tv_sec cmp (uvp)->tv_sec || \
                                   ((tvp)->tv_sec == (uvp)->tv_sec &&  \
                                    (tvp)->tv_usec cmp (uvp)->tv_usec))

  #define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#endif  /* !__SYS_WTIME_H */
