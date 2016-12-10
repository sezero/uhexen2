#include <time.h>

char *ctime (const time_t *tt)
{
    return asctime (localtime (tt));
}
