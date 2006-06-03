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

