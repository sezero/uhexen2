/*
	loki_xdelta.h
	$Id: loki_xdelta.h,v 1.4 2007-12-08 09:16:52 sezero Exp $

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

#ifndef	_LOKI_XDELTA_H
#define	_LOKI_XDELTA_H

/* XDelta is linked in, for space reasons .. I wish it didn't use glib.. */
extern int loki_xdelta(const char *old, const char *new, const char *out);
extern int loki_xpatch(const char *pat, const char *old, const char *out);

/* gui progress bar support: */
extern size_t		outsize, written_size;

#endif	/* _LOKI_XDELTA_H */

