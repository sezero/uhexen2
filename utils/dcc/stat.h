#include "qcc.h"

typedef struct stat_cache{
 unsigned short op;
 def_t *a,*b,*c;
 struct stat_cache *next;
}stat_cache_t;
