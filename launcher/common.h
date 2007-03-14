/*
	common.h
	common header for hexen2 launcher

	$Id: common.h,v 1.5 2007-03-14 21:04:24 sezero Exp $
*/

#ifndef	LAUNCHER_COMMON_INC_H
#define	LAUNCHER_COMMON_INC_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>

#define MAX_OSPATH	256

// from launch_bin.c
void CheckExe (void);
void launch_hexen2_bin (void);

#endif	// LAUNCHER_COMMON_INC_H

