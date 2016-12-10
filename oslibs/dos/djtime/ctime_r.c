#include <time.h>

extern char * asctime_r (const struct tm *, char *);

char *ctime_r (const time_t *tt, char *buffer)
{
    return asctime_r (localtime (tt), buffer);
}
char *__ctime_r (const time_t *, char *) __attribute__((alias("ctime_r")));
