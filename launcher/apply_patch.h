/*
 * apply_patch.h
 * hexen2 launcher: binary patch starter
 *
 * $Id$
 *
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef	_APPLY_PATCH_H
#define	_APPLY_PATCH_H

#define	XPATCH_NONE		0
#define	XPATCH_APPLIED		1UL
#define	XPATCH_FAIL		2UL

extern void *apply_patches (void *workdir);

extern int		thread_alive;

/* gui progress bar support: */
#include "xdelta3-mainopt.h"
extern xd3_progress_t	h2patch_progress;

#endif	/* _APPLY_PATCH_H */

