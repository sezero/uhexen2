/* xdelta 3 - delta compression tools and library
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007,
 * 2008, 2009, 2010, 2011
 * Joshua P. MacDonald
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _XDELTA3_SIZDEF_H_
#define _XDELTA3_SIZDEF_H_

/* Default configured value of stream->winsize.  If the program
 * supplies xd3_encode_input() with data smaller than winsize the
 * stream will automatically buffer the input, otherwise the input
 * buffer is used directly.
 */
#ifndef XD3_DEFAULT_WINSIZE
#define XD3_DEFAULT_WINSIZE (1U << 23)
#endif

/* Default total size of the source window used in xdelta3-main.h */
#ifndef XD3_DEFAULT_SRCWINSZ
#define XD3_DEFAULT_SRCWINSZ (1U << 26)
#endif

/* When Xdelta requests a memory allocation for certain buffers, it
 * rounds up to units of at least this size.  The code assumes (and
 * asserts) that this is a power-of-two. */
#ifndef XD3_ALLOCSIZE
#define XD3_ALLOCSIZE (1U<<14)
#endif

/* The XD3_HARDMAXWINSIZE parameter is a safety mechanism to protect
 * decoders against malicious files.  The decoder will never decode a
 * window larger than this.  If the file specifies VCD_TARGET the
 * decoder may require two buffers of this size.
 *
 * 8-16MB is reasonable, probably don't need to go larger. */
#ifndef XD3_HARDMAXWINSIZE
#define XD3_HARDMAXWINSIZE (1U<<24)
#endif
/* The IOPT_SIZE value sets the size of a buffer used to batch
 * overlapping copy instructions before they are optimized by picking
 * the best non-overlapping ranges.  The larger this buffer, the
 * longer a forced xd3_srcwin_setup() decision is held off.  Setting
 * this value to 0 causes an unlimited buffer to be used. */
#ifndef XD3_DEFAULT_IOPT_SIZE
#define XD3_DEFAULT_IOPT_SIZE    (1U<<15)
#endif

/* The maximum distance backward to search for small matches */
#ifndef XD3_DEFAULT_SPREVSZ
#define XD3_DEFAULT_SPREVSZ (1U<<18)
#endif

/* The default compression level
 */
#ifndef XD3_DEFAULT_LEVEL
#define XD3_DEFAULT_LEVEL 3
#endif

/* Sizes and addresses within VCDIFF windows are represented as usize_t
 */
typedef unsigned int usize_t;

#endif /* _XDELTA3_SIZDEF_H_ */
