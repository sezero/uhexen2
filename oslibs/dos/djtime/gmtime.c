/* based on public domain libnix for amiga */

#include <time.h>

/* Rules for leap-years:
 * 1. every 4th year is a leap year
 * 2. every 100th year is none
 * 3. every 400th is one 
 * 4. 1900 was none, 2000 is one
 */

static char _my_tmzone[] = { '+', '0', '0', 0 };

static const char monthtable[] = {
/* JanFebMarAprMayJunJulAugSepOktNov */
   31,29,31,30,31,30,31,31,30,31,30
};

struct tm *gmtime_r (const time_t *tt, struct tm *tm)
{
  signed long tim;
  int leapday = 0, leapyear = 0, i;

  tim = *tt;
  tm->tm_sec = tim % 60;
  tim /= 60;
  tm->tm_min = tim % 60;
  tim /= 60;
  tm->tm_hour = tim % 24;
  tim = tim / 24 + 719162;
  tm->tm_wday = (tim + 1) % 7;
  tm->tm_year = tim / 146097 * 400 - 1899;
  tim %= 146097;
  if (tim >= 145731)
  { leapyear++; /* The day is in one of the 400th */
    if (tim == 146096)
    { tim--; /* Be careful: The last of the 4 centuries is 1 day longer */
      leapday++;
    }
  }

  tm->tm_year += tim / 36524 * 100;
  tim %= 36524;
  if (tim >= 36159)
    leapyear--; /* The day is in one of the 100th */
  tm->tm_year += tim / 1461 * 4;
  tim %= 1461;
  if (tim >= 1095)
  { leapyear++; /* The day is in one of the 4th */
    if (tim == 1460)
    { tim--; /* Be careful: The 4th year is 1 day longer */
      leapday++;
    }
  }
  tm->tm_year += tim / 365;
  tim = tim % 365 + leapday;
  tm->tm_yday = tim;
  if (!leapyear && tim >= 31+28)
    tim++; /* add 1 for 29-Feb if no leap year */

  for (i=0; i<11; i++)
  {
    if (tim < monthtable[i])
      break;
    tim-=monthtable[i];
  }

  tm->tm_mon = i;
  tm->tm_mday = tim + 1;

  tm->tm_isdst = -1;
  tm->__tm_zone = _my_tmzone;
  tm->__tm_gmtoff = 0;

  return tm;
}

struct tm *gmtime (const time_t *tt) {
  static struct tm _mytm;
  return gmtime_r (tt, &_mytm);
}

struct tm *localtime(const time_t *) __attribute__((alias("gmtime")));/* not using time zones */
struct tm *localtime_r(const time_t *, struct tm *) __attribute__((alias("gmtime_r")));
struct tm *__gmtime_r (const time_t *, struct tm *) __attribute__((alias("gmtime_r")));
struct tm *__localtime_r(const time_t *, struct tm *) __attribute__((alias("gmtime_r")));

/*
 * 719162 number of days between 1.1.1 and 1.1.1970 
 *        if the calendar would go so far which it doesn't :-)
 *        this is true for all of the following.
 * 146097 number of days from 1.1.1 to 1.1.401
 * 145731 number of days from 1.1.1 to 1.1.400
 *  36524 number of days from 1.1.1 to 1.1.101
 *  36159 number of days from 1.1.1 to 1.1.100
 *   1461 number of days from 1.1.1 to 1.1.5
 *   1095 number of days from 1.1.1 to 1.1.4
 */
