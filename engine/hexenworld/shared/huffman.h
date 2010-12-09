/*
	huffman.h
	huffman encoding/decoding for use in hexenworld networking

	$Id: huffman.h,v 1.7 2007-11-12 14:02:55 sezero Exp $
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

