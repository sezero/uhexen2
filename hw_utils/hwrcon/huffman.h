/*
	huffman.h
	huffman encoding/decoding for use in hexenworld networking

	$Id: huffman.h,v 1.4 2007-03-14 21:04:22 sezero Exp $
*/

#ifndef __H2W_HUFFMAN_H
#define __H2W_HUFFMAN_H

extern void HuffInit (void);
extern void HuffEncode (unsigned char *in, unsigned char *out, int inlen, int *outlen);
extern void HuffDecode (unsigned char *in, unsigned char *out, int inlen, int *outlen, const int maxlen);

#endif	/* __H2W_HUFFMAN_H */

