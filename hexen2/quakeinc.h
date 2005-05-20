/*
	quakeinc.h
	Quake header file wrapper

	$Id: quakeinc.h,v 1.1 2005-05-20 15:26:33 sezero Exp $
*/

#ifndef	QUAKE_INC_H

#ifdef _WIN32
// win32 include
#define QUAKE_INC_H	"winquake.h"

#else
// unix include
#define QUAKE_INC_H	"linquake.h"
#endif

#include QUAKE_INC_H

#endif	// QUAKE_INC_H
