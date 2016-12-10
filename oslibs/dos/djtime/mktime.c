/* based on public domain libnix for amiga */

#include <time.h>

static const char monthtable[] = {
/* JanFebMarAprMayJunJulAugSepOktNov */
   31,29,31,30,31,30,31,31,30,31,30
};

time_t mktime(struct tm *t)
{
  time_t tim,year;
  int leapyear=0,i;

  year=t->tm_year+1899; /* years from year 1 */
  tim=year/400*146097;
  year%=400;
  if(year==399)
    leapyear++;
  tim+=year/100*36524;
  year%=100;
  if(year==99)
    leapyear--;
  tim+=year/4*1461;
  year%=4;
  if(year==3)
    leapyear++;
  tim+=year*365-719162+t->tm_mday-1;
  for(i=0;i<t->tm_mon;i++)
    tim+=monthtable[i];
  if(!leapyear&&t->tm_mon>1) /* Sub 1 if no leap year */
    tim--; /* tim contains now the number of days since 1.1.1970 */
  tim=((tim*24+t->tm_hour)*60+t->tm_min)*60+t->tm_sec;
  return tim;
}

/* See gmtime.c for more details on the magic numbers :-) */
