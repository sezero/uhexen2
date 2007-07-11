/*
	util_inc.h
	primary header for the utilities

	$Id: util_inc.h,v 1.3 2007-07-11 16:47:20 sezero Exp $
*/

#ifndef __UTILSINC_H
#define __UTILSINC_H

#include "q_types.h"

#include "compiler.h"
#include "arch_def.h"

#include <stdio.h>
#include <string.h>
#if !defined(PLATFORM_WINDOWS)
#include <strings.h>	/* strcasecmp and strncasecmp	*/
#endif	/* ! PLATFORM_WINDOWS */

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#endif	/* __UTILSINC_H */

