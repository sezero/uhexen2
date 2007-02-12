#ifndef _MD5_H_
#define _MD5_H_

#define CHECKSUM_SIZE   32

typedef struct {
    unsigned int A,B,C,D;	  /* chaining variables */
    unsigned int  nblocks;
    unsigned char buf[64];
    int  count;
} MD5_CONTEXT;

void md5_init( MD5_CONTEXT *ctx );

/* The routine updates the message-digest context to
 * account for the presence of each of the characters inBuf[0..inLen-1]
 * in the message whose digest is being computed.
 */
void md5_write( MD5_CONTEXT *hd, unsigned char *inbuf, size_t inlen);

/* The routine final terminates the message-digest computation and
 * ends with the desired message digest in mdContext->digest[0...15].
 * The handle is prepared for a new MD5 cycle.
 * Returns 16 bytes representing the digest.
 */

void md5_final( MD5_CONTEXT *hd );

/* Compute the MD5 sum of a file.
   md5sum[] must be at least CHECKSUM_SIZE+1 chars long.
   If 'unpack' is true, then the checksum will be on the uncompressed
   contents. (Currently only implemented for gzip compressed files.)
 */
int md5_compute(const char *path, char md5sum[], int unpack);

/* Get the ASCII representation of a binary MD5 checksum */
const char *get_md5(unsigned char *binsum);

/* Reverse operation: translate an ASCII checksum to binary */
unsigned char *get_md5_bin(const char *asciisum);

#endif
