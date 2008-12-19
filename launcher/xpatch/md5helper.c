/**	$MirOS: src/lib/libc/hash/helper.c,v 1.5 2007/05/07 15:21:18 tg Exp $ */
/*	$OpenBSD: helper.c,v 1.8 2005/08/08 08:05:35 espie Exp $	*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dkuug.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include <sys/param.h>
#include <sys/stat.h>

#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "md5.h"

#ifndef MIN
#define MIN(a, b)	((a) < (b)? (a): (b))
#endif

/* ARGSUSED */
char *
MD5End(MD5_CTX *ctx, char *buf)
{
	int i;
	uint8_t digest[MD5_DIGEST_LENGTH];
#ifdef MD5_DIGEST_UPPERCASE
	static const char hex[] = "0123456789ABCDEF";
#else
	static const char hex[] = "0123456789abcdef";
#endif

	if (buf == NULL && (buf = malloc(MD5_DIGEST_STRING_LENGTH)) == NULL)
		return (NULL);

	MD5Final(digest, ctx);
	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		buf[i + i] = hex[digest[i] >> 4];
		buf[i + i + 1] = hex[digest[i] & 0x0f];
	}
	buf[i + i] = '\0';
	memset(digest, 0, sizeof(digest));
	return (buf);
}

char *
MD5FileChunk(const char *filename, char *buf, off_t off, off_t len)
{
	struct stat sb;
	uint8_t buffer[BUFSIZ];
	MD5_CTX ctx;
	int fd, save_errno;
	ssize_t nr;

	MD5Init(&ctx);

	if ((fd = open(filename, O_RDONLY)) < 0)
		return (NULL);
	if (len == 0) {
		if (fstat(fd, &sb) == -1) {
			close(fd);
			return (NULL);
		}
		len = sb.st_size;
	}
	if ((len < 0) || (off > 0 && lseek(fd, off, SEEK_SET) < 0))
		return (NULL);

	while ((nr = read(fd, buffer,
	    (size_t)(len ? MIN(BUFSIZ, len) : BUFSIZ))) > 0) {
		MD5Update(&ctx, buffer, (size_t)nr);
		if (len > 0 && (len -= nr) == 0)
			break;
	}

	save_errno = errno;
	close(fd);
	errno = save_errno;
	return (nr < 0 ? NULL : MD5End(&ctx, buf));
}

char *
MD5File(const char *filename, char *buf)
{
	return (MD5FileChunk(filename, buf, (off_t)0, (off_t)0));
}

char *
MD5Data(const uint8_t *data, size_t len, char *buf)
{
	MD5_CTX ctx;

	MD5Init(&ctx);
	MD5Update(&ctx, data, len);
	return (MD5End(&ctx, buf));
}

