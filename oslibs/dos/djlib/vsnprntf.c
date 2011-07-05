/* DJGPP < v2.04 doesn't have vsnprintf().  The following are
 * copied over from DJGPP 2.04.  */

/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <libc/file.h>

static __inline__ void __stropenw(FILE *p, char *str, int len)
{
  p->_flag = _IOWRT | _IOSTRG | _IONTERM;
  p->_ptr = str;
  p->_cnt = len;
}

static __inline__ void __strclosew(FILE *p)
{
  *p->_ptr = '\0';
}

int
vsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
  FILE _strbuf;
  int len;

  /* _cnt is an int in the FILE structure. To prevent wrap-around, we limit
   * n to between 0 and INT_MAX inclusively. */
  if (n > INT_MAX)
  {
    errno = EFBIG;
    return -1;
  }

  memset(&_strbuf, 0, sizeof(_strbuf));

  /* If n == 0, just querying how much space is needed. */
  if (n > 0)
    __stropenw(&_strbuf, str, n - 1);
  else
    __stropenw(&_strbuf, NULL, 0);

  len = _doprnt(fmt, ap, &_strbuf);

  /* Ensure nul termination */
  if (n > 0)
    __strclosew(&_strbuf);

  return len;
}
