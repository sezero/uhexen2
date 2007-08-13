/*
	apply_patch.h
	hexen2 launcher: binary patch starter

	$Id: apply_patch.h,v 1.6 2007-08-13 14:50:35 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#ifndef	_APPLY_PATCH_H
#define	_APPLY_PATCH_H

#define	XPATCH_NONE		0
#define	XPATCH_APPLIED		1UL
#define	XPATCH_FAIL		2UL

extern void *apply_patches (void *workdir);

extern int		thread_alive;

#endif	/* _APPLY_PATCH_H */

