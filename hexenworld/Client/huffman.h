//	huffman.h
//	huffman encoding/decoding for use in hexenworld networking

extern void HuffInit (void);
extern void HuffEncode (unsigned char *in, unsigned char *out, int inlen, int *outlen);
extern void HuffDecode (unsigned char *in, unsigned char *out, int inlen, int *outlen);

#ifdef DEBUG_BUILD
extern void ZeroFreq (void);
extern void CalcFreq (unsigned char *packet, int packetlen);
extern void PrintFreqs (void);

extern	int	HuffIn;
extern	int	HuffOut;
#endif

