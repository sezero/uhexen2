#ifndef __W32_ZONEMEM_H__
#define __W32_ZONEMEM_H__

#if defined(USE_ZONEMEM)

#undef malloc
#undef realloc
#undef calloc
#undef free
#undef strdup

#include "zone.h"
#define malloc(s)    Z_Malloc((s),Z_SECZONE)
#define calloc(n,s)  Z_Malloc((n)*(s),Z_SECZONE)
#define realloc(p,s) Z_Realloc((p),(s),Z_SECZONE)
#define free(p)      Z_Free((p))
#define strdup(p)    Z_Strdup((p))

#endif /* USE_ZONEMEM */

#endif /* __W32_ZONEMEM_H__ */
