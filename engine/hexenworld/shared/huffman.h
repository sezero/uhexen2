/*
 * huffman.h -- huffman encoding/decoding for use in hexenworld networking
 * $Id: huffman.h,v 1.7 2007-11-12 14:02:55 sezero Exp $
 *
 * Copyright (C) 1997-1998  Raven Software Corp.
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

#ifndef __H2W_HUFFMAN_H
#define __H2W_HUFFMAN_H

extern void HuffInit (void);
extern void HuffEncode (const unsigned char *in, unsigned char *out, int inlen, int *outlen);
extern void HuffDecode (const unsigned char *in, unsigned char *out, int inlen, int *outlen, const int maxlen);

#define	_DEBUG_HUFFMAN	0

#if _DEBUG_HUFFMAN
extern void PrintFreqs (void);
#endif	/* _DEBUG_HUFFMAN */

#endif	/* __H2W_HUFFMAN_H */

