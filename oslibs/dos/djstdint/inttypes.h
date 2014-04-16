/* Hacked from DJGPP v2.04 for use with older DJGPP versions */

/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */

#ifndef __dj_inttypes__h_
#define __dj_inttypes__h_

/* Get the type definitions.  */
#include <stdint.h>

/* ANSI/ISO C99 says these should not be visible in C++ unless
   explicitly requested.  */

#if !defined(__cplusplus) || defined(__STDC_FORMAT_MACROS)

#define PRId8       "hhd"
#define PRId16      "hd"
#define PRId32      "ld"
#define PRId64      "lld"
#define PRIdLEAST8  "hhd"
#define PRIdLEAST16 "hd"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 "lld"
#define PRIdFAST8   "hhd"
#define PRIdFAST16  "d"
#define PRIdFAST32  "d"
#define PRIdFAST64  "lld"
#define PRIdMAX     "lld"
#define PRIdPTR     "ld"

#define PRIi8       "hhi"
#define PRIi16      "hi"
#define PRIi32      "li"
#define PRIi64      "lli"
#define PRIiLEAST8  "hhi"
#define PRIiLEAST16 "hi"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 "lli"
#define PRIiFAST8   "hhi"
#define PRIiFAST16  "i"
#define PRIiFAST32  "i"
#define PRIiFAST64  "lli"
#define PRIiMAX     "lli"
#define PRIiPTR     "li"

#define PRIo8       "hho"
#define PRIo16      "ho"
#define PRIo32      "lo"
#define PRIo64      "llo"
#define PRIoLEAST8  "hho"
#define PRIoLEAST16 "ho"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 "llo"
#define PRIoFAST8   "hho"
#define PRIoFAST16  "o"
#define PRIoFAST32  "o"
#define PRIoFAST64  "llo"
#define PRIoMAX     "llo"
#define PRIoPTR     "lo"

#define PRIu8       "hhu"
#define PRIu16      "hu"
#define PRIu32      "lu"
#define PRIu64      "llu"
#define PRIuLEAST8  "hhu"
#define PRIuLEAST16 "hu"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 "llu"
#define PRIuFAST8   "hhu"
#define PRIuFAST16  "u"
#define PRIuFAST32  "u"
#define PRIuFAST64  "llu"
#define PRIuMAX     "llu"
#define PRIuPTR     "lu"

#define PRIx8       "hhx"
#define PRIx16      "hx"
#define PRIx32      "lx"
#define PRIx64      "llx"
#define PRIxLEAST8  "hhx"
#define PRIxLEAST16 "hx"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 "llx"
#define PRIxFAST8   "hhx"
#define PRIxFAST16  "x"
#define PRIxFAST32  "x"
#define PRIxFAST64  "llx"
#define PRIxMAX     "llx"
#define PRIxPTR     "lx"

#define PRIX8       "hhX"
#define PRIX16      "hX"
#define PRIX32      "lX"
#define PRIX64      "llX"
#define PRIXLEAST8  "hhX"
#define PRIXLEAST16 "hX"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 "llX"
#define PRIXFAST8   "hhX"
#define PRIXFAST16  "X"
#define PRIXFAST32  "X"
#define PRIXFAST64  "llX"
#define PRIXMAX     "llX"
#define PRIXPTR     "lX"

#define SCNd8       "hhd"
#define SCNd16      "hd"
#define SCNd32      "ld"
#define SCNd64      "lld"
#define SCNdLEAST8  "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 "lld"
#define SCNdFAST8   "hhd"
#define SCNdFAST16  "d"
#define SCNdFAST32  "d"
#define SCNdFAST64  "lld"
#define SCNdMAX     "lld"
#define SCNdPTR     "ld"

#define SCNi8       "hhi"
#define SCNi16      "hi"
#define SCNi32      "li"
#define SCNi64      "lli"
#define SCNiLEAST8  "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 "lli"
#define SCNiFAST8   "hhi"
#define SCNiFAST16  "i"
#define SCNiFAST32  "i"
#define SCNiFAST64  "lli"
#define SCNiMAX     "lli"
#define SCNiPTR     "li"

#define SCNo8       "hho"
#define SCNo16      "ho"
#define SCNo32      "lo"
#define SCNo64      "llo"
#define SCNoLEAST8  "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 "llo"
#define SCNoFAST8   "hho"
#define SCNoFAST16  "o"
#define SCNoFAST32  "o"
#define SCNoFAST64  "llo"
#define SCNoMAX     "llo"
#define SCNoPTR     "lo"

#define SCNu8       "hhu"
#define SCNu16      "hu"
#define SCNu32      "lu"
#define SCNu64      "llu"
#define SCNuLEAST8  "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 "llu"
#define SCNuFAST8   "hhu"
#define SCNuFAST16  "u"
#define SCNuFAST32  "u"
#define SCNuFAST64  "llu"
#define SCNuMAX     "llu"
#define SCNuPTR     "lu"

#define SCNx8       "hhx"
#define SCNx16      "hx"
#define SCNx32      "lx"
#define SCNx64      "llx"
#define SCNxLEAST8  "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 "llx"
#define SCNxFAST8   "hhx"
#define SCNxFAST16  "x"
#define SCNxFAST32  "x"
#define SCNxFAST64  "llx"
#define SCNxMAX     "llx"
#define SCNxPTR     "lx"

#endif /* !__cplusplus || __STDC_FORMAT_MACROS */

#if 0 /* don't need the intmax functions below */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  intmax_t quot;
  intmax_t rem;
} imaxdiv_t;

intmax_t imaxabs (intmax_t _j);
imaxdiv_t imaxdiv (intmax_t _numer, intmax_t _denom);
intmax_t strtoimax (const char *_nptr, char **_endptr, int _base);
uintmax_t strtoumax (const char *_nptr, char **_endptr, int _base);

#ifdef __cplusplus
}
#endif
#endif /* if 0 */

#endif /* __dj_inttypes__h_ */
