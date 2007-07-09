/*
	huffman.h
	huffman encoding/decoding for use in hexenworld networking

	$Id: huffman.h,v 1.6 2007-07-09 19:30:20 sezero Exp $
*/

#ifndef __H2W_HUFFMAN_H
#define __H2W_HUFFMAN_H

extern void HuffInit (void);
extern void HuffEncode (unsigned char *in, unsigned char *out, int inlen, int *outlen);
extern void HuffDecode (unsigned char *in, unsigned char *out, int inlen, int *outlen, const int maxlen);

#define	_DEBUG_HUFFMAN	0

#if _DEBUG_HUFFMAN
extern void PrintFreqs (void);
#endif	/* _DEBUG_HUFFMAN */

#endif	/* __H2W_HUFFMAN_H */

