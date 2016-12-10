/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>
#include <time.h>

static char _my_tmzone[] = { '+', '0', '0', 0 };

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  __dpmi_regs r;
  struct tm tmblk;
  struct timeval tv_tmp;

  if (!tv)
    tv = &tv_tmp;

  r.h.ah = 0x2c;
  __dpmi_int(0x21, &r);

  tv->tv_usec = r.h.dl * 10000;
  tmblk.tm_sec = r.h.dh;
  tmblk.tm_min = r.h.cl;
  tmblk.tm_hour = r.h.ch;

  r.h.ah = 0x2a;
  __dpmi_int(0x21, &r);

  tmblk.tm_mday = r.h.dl;
  tmblk.tm_mon = r.h.dh - 1;
  tmblk.tm_year = (r.x.cx & 0x7ff) - 1900;

  tmblk.tm_wday = tmblk.tm_yday = 0;
  tmblk.tm_isdst = -1;
  tmblk.tm_zone = _my_tmzone;
  tmblk.tm_gmtoff = 0;

  tv->tv_sec = mktime(&tmblk);

  if(tz)
    tz->tz_minuteswest = tz->tz_dsttime = 0;/* not using timezones */
  return 0;
}
int __gettimeofday(struct timeval *, struct timezone *) __attribute__((alias("gettimeofday")));
