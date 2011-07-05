/* q_stdint.h: include a proper stdint.h header */

#ifndef __QSTDINT_H
#define __QSTDINT_H

#if defined(_MSC_VER) && (_MSC_VER < 1600)
/* MS Visual Studio provides stdint.h only starting with
 * version 2010.  Even in VS2010, there is no inttypes.h.. */
#include "msinttypes/stdint.h"

#elif defined(__DJGPP__) && (__DJGPP_MINOR__ < 4)
/* DJGPP < v2.04 doesn't have stdint.h and inttypes.h. */
#include "djstdint/stdint.h"

#else	/* assume presence of a stdint.h from the SDK. */
#include <stdint.h>
#endif

#endif	/* __QSTDINT_H */

