/* Copyright (C) 2008 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <string.h>
#include <time.h>

#define TM_YEAR_BASE 1900

static const char *afmt[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
static const char *Afmt[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
  "Saturday",
};
static const char *bfmt[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec",
};
static const char *Bfmt[] = {
  "January", "February", "March", "April", "May", "June", "July",
  "August", "September", "October", "November", "December",
};

static size_t gsize;
static char *pt;

static int
_add(const char *str, int upcase)
{
  for (;; ++pt, --gsize)
  {
    if (!gsize)
      return 0;
    if (!(*pt = *str++))
      return 1;
    if (upcase)
    {
      if (*pt >= 'a' && *pt <= 'z')
          *pt -= ('a' - 'A');
    }
  }
}

static int
_conv(int n, int digits, char pad)
{
  static char buf[10];
  char *p = buf + sizeof(buf) - 2;

  do {
    *p-- = n % 10 + '0';
    n /= 10;
    --digits;
  } while (n > 0 && p > buf);

  if (pad)
    while (p > buf && digits-- > 0)
      *p-- = pad;
  return _add(++p, 0);
}

static size_t
_fmt(const char *format, const struct tm *t, int upcase)
{
  for (; *format; ++format)
  {
    if (*format == '%')
    {
      int pad = '0', space=' ';
      if (format[1] == '_')
	pad = space = ' ', format++;
      if (format[1] == '-')
	pad = space = 0, format++;
      if (format[1] == '0')
	pad = space = '0', format++;
      if (format[1] == '^')
	upcase = 1, format++;
      if (format[1] == 'E' || format[1] == 'O')
	format++;  /*  Only C/POSIX locale is supported.  */

      switch(*++format)
      {
      case '\0':
	--format;
	break;
      case 'A':
	if (t->tm_wday < 0 || t->tm_wday > 6)
	  return 0;
	if (!_add(Afmt[t->tm_wday], upcase))
	  return 0;
	continue;
      case 'a':
	if (t->tm_wday < 0 || t->tm_wday > 6)
	  return 0;
	if (!_add(afmt[t->tm_wday], upcase))
	  return 0;
	continue;
      case 'B':
	if (t->tm_mon < 0 || t->tm_mon > 11)
	  return 0;
	if (!_add(Bfmt[t->tm_mon], upcase))
	  return 0;
	continue;
      case 'b':
      case 'h':
	if (t->tm_mon < 0 || t->tm_mon > 11)
	  return 0;
	if (!_add(bfmt[t->tm_mon], upcase))
	  return 0;
	continue;
      case 'C':
	if (!_conv((t->tm_year+TM_YEAR_BASE)/100, 2, pad))
	  return 0;
	continue;
      case 'c':
	if (!_fmt("%a %b %e %H:%M:%S %Y", t, upcase))
	  return 0;
	continue;
      case 'e':
	if (!_conv(t->tm_mday, 2, ' '))
	  return 0;
	continue;
      case 'D':
	if (!_fmt("%m/%d/%y", t, upcase))
	  return 0;
	continue;
      case 'd':
	if (!_conv(t->tm_mday, 2, pad))
	  return 0;
	continue;
      case 'H':
	if (!_conv(t->tm_hour, 2, pad))
	  return 0;
	continue;
      case 'I':
	if (!_conv(t->tm_hour % 12 ?
		   t->tm_hour % 12 : 12, 2, pad))
	  return 0;
	continue;
      case 'j':
	if (!_conv(t->tm_yday + 1, 3, pad))
	  return 0;
	continue;
      case 'k':
	if (!_conv(t->tm_hour, 2, ' '))
	  return 0;
	continue;
      case 'l':
	if (!_conv(t->tm_hour % 12 ?
		   t->tm_hour % 12 : 12, 2, ' '))
	  return 0;
	continue;
      case 'M':
	if (!_conv(t->tm_min, 2, pad))
	  return 0;
	continue;
      case 'm':
	if (!_conv(t->tm_mon + 1, 2, pad))
	  return 0;
	continue;
      case 'n':
	if (!_add("\n", upcase))
	  return 0;
	continue;
      case 'p':
	if (!_add(t->tm_hour >= 12 ? "PM" : "AM", upcase))
	  return 0;
	continue;
      case 'R':
	if (!_fmt("%H:%M", t, upcase))
	  return 0;
	continue;
      case 'r':
	if (!_fmt("%I:%M:%S %p", t, upcase))
	  return 0;
	continue;
      case 'S':
	if (!_conv(t->tm_sec, 2, pad))
	  return 0;
	continue;
      case 'T':
	if (!_fmt("%H:%M:%S", t, upcase))
	  return 0;
	continue;
      case 't':
	if (!_add("\t", upcase))
	  return 0;
	continue;
      case 'u':
	if (!_conv(t->tm_wday==0 ? 7 : t->tm_wday, 1, pad))
	  return 0;
	continue;
      case 'U':
	if (!_conv((t->tm_yday + 7 - t->tm_wday) / 7,
		   2, pad))
	  return 0;
	continue;
      case 'W':
	if (!_conv((t->tm_yday + 7 -
		    (t->tm_wday ? (t->tm_wday - 1) : 6))
		   / 7, 2, pad))
	  return 0;
	continue;
      case 'w':
	if (!_conv(t->tm_wday, 1, pad))
	  return 0;
	continue;
      case 'X':
	if (!_fmt("%H:%M:%S", t, upcase))
	  return 0;
	continue;
      case 'x':
	if (!_fmt("%m/%d/%y", t, upcase))
	  return 0;
	continue;
      case 'y':
      case 'g':
	if (!_conv((t->tm_year + TM_YEAR_BASE) % 100, 2, pad))
	  return 0;
	continue;
      case 'Y':
      case 'G':
	if (!_conv(t->tm_year + TM_YEAR_BASE, 4, pad))
	  return 0;
	continue;
      case 'z':
	if (!_add(t->__tm_gmtoff<0 ? "-" : "+", 0))
	  return 0;
	if (!_conv(t->__tm_gmtoff<0 ? -t->__tm_gmtoff : t->__tm_gmtoff, 4, pad))
	  return 0;
	continue;
      case 'Z':
	if (!t->tm_zone || !_add(t->tm_zone, upcase))
	  return 0;
	continue;
      case '%':
	/*
	 * X311J/88-090 (4.12.3.5): if conversion char is
	 * undefined, behavior is undefined.  Print out the
	 * character itself as printf(3) does.
	 */
      default:
	break;
      }
    }
    if (!gsize--)
      return 0;
    *pt++ = *format;
  }
  return gsize;
}

size_t
strftime(char *s, size_t maxsize, const char *format, const struct tm *t)
{
  pt = s;
  if ((gsize = maxsize) < 1)
    return 0;
  if (_fmt(format, t, 0))
  {
    *pt = '\0';
    return maxsize - gsize;
  }
  return 0;
}
