/* from publicly available man pages */

#include <time.h>
#include <stdio.h>

char *asctime_r (const struct tm *tm, char *buffer)
{
    static const char wday_name[7][3] = {
      "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char mon_name[12][3] = {
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    sprintf(buffer, "%.3s %.3s%3d %02d:%02d:%02d %d\n",
      wday_name[tm->tm_wday], mon_name[tm->tm_mon],
      tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
      1900 + tm->tm_year);
    return buffer;
}
char *__asctime_r (const struct tm *, char *) __attribute__((alias("asctime_r")));

char *asctime (const struct tm *tm)
{
    static char buffer[26];
    return asctime_r (tm, buffer);
}
