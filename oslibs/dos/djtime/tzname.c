#include <time.h>

static char _my_tmzone[] = { '+', '0', '0', 0 };

char * tzname[2] = {
  _my_tmzone,
  _my_tmzone
};
