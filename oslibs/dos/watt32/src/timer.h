/*!\file timer.h
 */
#ifndef _w32_TIMER_H
#define _w32_TIMER_H

#ifndef __SYS_WTIME_H
#include <sys/wtime.h>      /* struct timeval */
#endif

#define has_8254          NAMESPACE (has_8254)
#define has_rdtsc         NAMESPACE (has_rdtsc)
#define use_rdtsc         NAMESPACE (use_rdtsc)
#define using_int8        NAMESPACE (using_int8)
#define user_tick_active  NAMESPACE (user_tick_active)
#define user_tick_base    NAMESPACE (user_tick_base)
#define user_tick_msec    NAMESPACE (user_tick_msec)
#define clocks_per_usec   NAMESPACE (clocks_per_usec)
#define clockbits         NAMESPACE (clockbits)
#define ms_clock          NAMESPACE (ms_clock)
#define millisec_clock    NAMESPACE (millisec_clock)
#define start_time        NAMESPACE (start_time)
#define start_day         NAMESPACE (start_day)
#define init_timers       NAMESPACE (init_timers)
#define set_timeout       NAMESPACE (set_timeout)
#define chk_timeout       NAMESPACE (chk_timeout)
#define hires_timer       NAMESPACE (hires_timer)
#define set_timediff      NAMESPACE (set_timediff)
#define get_timediff      NAMESPACE (get_timediff)
#define timeval_diff      NAMESPACE (timeval_diff)
#define filetime_sec      NAMESPACE (filetime_sec)
#define get_cpu_speed     NAMESPACE (get_cpu_speed)
#define profile_start     NAMESPACE (profile_start)
#define profile_stop      NAMESPACE (profile_stop)
#define profile_init      NAMESPACE (profile_init)
#define profile_on        NAMESPACE (profile_on)
#define profile_dump      NAMESPACE (profile_dump)
#define profile_recv      NAMESPACE (profile_recv)
#define profile_file      NAMESPACE (profile_file)
#define profile_enable    NAMESPACE (profile_enable)

#define time_str          NAMESPACE (time_str)
#define hms_str           NAMESPACE (hms_str)
#define elapsed_str       NAMESPACE (elapsed_str)
#define init_timer_isr    NAMESPACE (init_timer_isr)
#define exit_timer_isr    NAMESPACE (exit_timer_isr)

/*
 * System clock at BIOS location 40:6C (dword). Counts upwards.
 */
#define BIOS_CLK 0x46C


W32_DATA BOOL   has_8254, has_rdtsc, use_rdtsc;
W32_DATA BOOL   user_tick_active, using_int8;
W32_DATA DWORD  clocks_per_usec, user_tick_msec;
W32_DATA time_t user_tick_base;
W32_DATA DWORD  start_time, start_day;

W32_FUNC void   init_timers    (void);
W32_FUNC DWORD  set_timeout    (DWORD msec);        /* return value for timeout */
W32_FUNC BOOL   chk_timeout    (DWORD timeout);     /* check if time expired    */
W32_FUNC int    cmp_timers     (DWORD t1, DWORD t2);
W32_FUNC DWORD  millisec_clock (void);
W32_FUNC int    hires_timer    (int on);
extern   DWORD  clockbits      (void);
extern   DWORD  ms_clock       (void);

W32_FUNC int    set_timediff   (long msec);
W32_FUNC long   get_timediff   (DWORD now, DWORD t);
W32_FUNC double timeval_diff   (const struct timeval *a, const struct timeval *b);
W32_FUNC double filetime_sec   (const void *filetime);

extern const char *time_str   (DWORD val);
extern const char *hms_str    (DWORD sec);
extern const char *elapsed_str(DWORD val);

W32_FUNC void init_timer_isr (void);
W32_FUNC void exit_timer_isr (void);

W32_FUNC void init_userSuppliedTimerTick (void);
W32_FUNC void userTimerTick (DWORD elapsedTimeMs);


  extern uint64 get_cpu_speed (void); /* moved here from below. */
#if defined(USE_PROFILER)
  extern BOOL   profile_enable;
  extern char   profile_file [MAX_PATHLEN+1];

/*extern uint64 get_cpu_speed (void);*/
  extern void   profile_start (const char *where);
  extern void   profile_stop  (void);
  extern int    profile_init  (void);
  extern int    profile_on    (void);
  extern void   profile_dump  (const uint64 *data, size_t num_elem);
  extern void   profile_recv  (const uint64 *put, const uint64 *get);

  #define PROFILE_START(func) profile_start (func)
  #define PROFILE_STOP()      profile_stop()
  #define PROFILE_RECV(p,g)   profile_recv (p,g)

#else
  #define PROFILE_START(func) ((void)0)
  #define PROFILE_STOP()      ((void)0)
  #define PROFILE_RECV(p,g)   ((void)0)
#endif


#endif

