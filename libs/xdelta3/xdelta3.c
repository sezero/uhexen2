/* xdelta 3 - delta compression tools and library
 * Copyright (C) 2001, 2003, 2004, 2005, 2006, 2007,
 * 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015. Joshua P. MacDonald
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   -------------------------------------------------------------------

			       Xdelta 3

   The goal of this library is to to implement both the (stand-alone)
   data-compression and delta-compression aspects of VCDIFF encoding, and
   to support a programming interface that works like Zlib
   (http://www.gzip.org/zlib.html). See RFC3284: The VCDIFF Generic
   Differencing and Compression Data Format.

   VCDIFF is a unified encoding that combines data-compression and
   delta-encoding ("differencing").

   VCDIFF has a detailed byte-code instruction set with many features.
   The instruction format supports an immediate size operand for small
   COPYs and ADDs (e.g., under 18 bytes).  There are also instruction
   "modes", which are used to compress COPY addresses by using two
   address caches.  An instruction mode refers to slots in the NEAR
   and SAME caches for recent addresses.  NEAR remembers the
   previous 4 (by default) COPY addresses, and SAME catches
   frequent re-uses of the same address using a 3-way (by default)
   256-entry associative cache of [ADDR mod 256], the encoded byte.
   A hit in the NEAR/SAME cache requires 0/1 ADDR bytes.

   VCDIFF has a default instruction table, but an alternate
   instruction tables may themselves be be delta-compressed and
   included in the encoding header.  This allows even more freedom.
   There are 9 instruction modes in the default code table, 4 near, 3
   same, VCD_SELF (absolute encoding) and VCD_HERE (relative to the
   current position).

   ----------------------------------------------------------------------

  			      Algorithms

   Aside from the details of encoding and decoding, there are a bunch
   of algorithms needed.

   1. STRING-MATCH.  A two-level fingerprinting approach is used.  A
   single loop computes the two checksums -- small and large -- at
   successive offsets in the TARGET file.  The large checksum is more
   accurate and is used to discover SOURCE matches, which are
   potentially very long.  The small checksum is used to discover
   copies within the TARGET.  Small matching, which is more expensive,
   usually dominates the large STRING-MATCH costs in this code - the
   more exhaustive the search, the better the results.  Either of the
   two string-matching mechanisms may be disabled.

   2. INSTRUCTION SELECTION.  The IOPT buffer here represents a queue
   used to store overlapping copy instructions.  There are two possible
   optimizations that go beyond a greedy search.  Both of these fall
   into the category of "non-greedy matching" optimizations.

   The first optimization stems from backward SOURCE-COPY matching.
   When a new SOURCE-COPY instruction covers a previous instruction in
   the target completely, it is erased from the queue.  Randal Burns
   originally analyzed these algorithms and did a lot of related work
   (\cite the 1.5-pass algorithm).

   The second optimization comes by the encoding of common very-small
   COPY and ADD instructions, for which there are special DOUBLE-code
   instructions, which code two instructions in a single byte.

   The cost of bad instruction-selection overhead is relatively high
   for data-compression, relative to delta-compression, so this second
   optimization is fairly important.  With "lazy" matching (the name
   used in Zlib for a similar optimization), the string-match
   algorithm searches after a match for potential overlapping copy
   instructions.  In Xdelta and by default, VCDIFF, the minimum match
   size is 4 bytes, whereas Zlib searches with a 3-byte minimum.  This
   feature, combined with double instructions, provides a nice
   challenge.  Search in this file for "black magic", a heuristic.

   3. STREAM ALIGNMENT.  Stream alignment is needed to compress large
   inputs in constant space.  See xd3_srcwin_move_point().

   4. WINDOW SELECTION.  When the IOPT buffer flushes, in the first call
   to xd3_iopt_finish_encoding containing any kind of copy instruction,
   the parameters of the source window must be decided: the offset into
   the source and the length of the window.  Since the IOPT buffer is
   finite, the program may be forced to fix these values before knowing
   the best offset/length.

   5. SECONDARY COMPRESSION.  VCDIFF supports a secondary encoding to
   be applied to the individual sections of the data format, which are
   ADDRess, INSTruction, and DATA.  Several secondary compressor
   variations are implemented here, although none is standardized yet.

   One is an adaptive huffman algorithm -- the FGK algorithm (Faller,
   Gallager, and Knuth, 1985).  This compressor is extremely slow.

   The other is a simple static Huffman routine, which is the base
   case of a semi-adaptive scheme published by D.J. Wheeler and first
   widely used in bzip2 (by Julian Seward).  This is a very
   interesting algorithm, originally published in nearly cryptic form
   by D.J. Wheeler. !!!NOTE!!! Because these are not standardized,
   secondary compression remains off by default.
   ftp://ftp.cl.cam.ac.uk/users/djw3/bred3.{c,ps}
   --------------------------------------------------------------------

			    Other Features

   1. USER CONVENIENCE

   For user convenience, it is essential to recognize Gzip-compressed
   files and automatically Gzip-decompress them prior to
   delta-compression (or else no delta-compression will be achieved
   unless the user manually decompresses the inputs).  The compressed
   represention competes with Xdelta, and this must be hidden from the
   command-line user interface.  The Xdelta-1.x encoding was simple, not
   compressed itself, so Xdelta-1.x uses Zlib internally to compress the
   representation.

   This implementation supports external compression, which implements
   the necessary fork() and pipe() mechanics.  There is a tricky step
   involved to support automatic detection of a compressed input in a
   non-seekable input.  First you read a bit of the input to detect
   magic headers.  When a compressed format is recognized, exec() the
   external compression program and create a second child process to
   copy the original input stream. [Footnote: There is a difficulty
   related to using Gzip externally. It is not possible to decompress
   and recompress a Gzip file transparently.  If FILE.GZ had a
   cryptographic signature, then, after: (1) Gzip-decompression, (2)
   Xdelta-encoding, (3) Gzip-compression the signature could be
   broken.  The only way to solve this problem is to guess at Gzip's
   compression level or control it by other means.  I recommend that
   specific implementations of any compression scheme store
   information needed to exactly re-compress the input, that way
   external compression is transparent - however, this won't happen
   here until it has stabilized.]

   2. APPLICATION-HEADER

   This feature was introduced in RFC3284.  It allows any application
   to include a header within the VCDIFF file format.  This allows
   general inter-application data exchange with support for
   application-specific extensions to communicate metadata.

   3. VCDIFF CHECKSUM

   An optional checksum value is included with each window, which can
   be used to validate the final result.  This verifies the correct source
   file was used for decompression as well as the obvious advantage:
   checking the implementation (and underlying) correctness.

   4. LIGHT WEIGHT

   The code makes efforts to avoid copying data more than necessary.
   The code delays many initialization tasks until the first use, it
   optimizes for identical (perfectly matching) inputs.  It does not
   compute any checksums until the first lookup misses.  Memory usage
   is reduced.  String-matching is templatized (by slightly gross use
   of CPP) to hard-code alternative compile-time defaults.  The code
   has few outside dependencies.
   ----------------------------------------------------------------------

		The default rfc3284 instruction table:
		    (see RFC for the explanation)

           TYPE      SIZE     MODE    TYPE     SIZE     MODE     INDEX
   --------------------------------------------------------------------
       1.  Run         0        0     Noop       0        0        0
       2.  Add    0, [1,17]     0     Noop       0        0      [1,18]
       3.  Copy   0, [4,18]     0     Noop       0        0     [19,34]
       4.  Copy   0, [4,18]     1     Noop       0        0     [35,50]
       5.  Copy   0, [4,18]     2     Noop       0        0     [51,66]
       6.  Copy   0, [4,18]     3     Noop       0        0     [67,82]
       7.  Copy   0, [4,18]     4     Noop       0        0     [83,98]
       8.  Copy   0, [4,18]     5     Noop       0        0     [99,114]
       9.  Copy   0, [4,18]     6     Noop       0        0    [115,130]
      10.  Copy   0, [4,18]     7     Noop       0        0    [131,146]
      11.  Copy   0, [4,18]     8     Noop       0        0    [147,162]
      12.  Add       [1,4]      0     Copy     [4,6]      0    [163,174]
      13.  Add       [1,4]      0     Copy     [4,6]      1    [175,186]
      14.  Add       [1,4]      0     Copy     [4,6]      2    [187,198]
      15.  Add       [1,4]      0     Copy     [4,6]      3    [199,210]
      16.  Add       [1,4]      0     Copy     [4,6]      4    [211,222]
      17.  Add       [1,4]      0     Copy     [4,6]      5    [223,234]
      18.  Add       [1,4]      0     Copy       4        6    [235,238]
      19.  Add       [1,4]      0     Copy       4        7    [239,242]
      20.  Add       [1,4]      0     Copy       4        8    [243,246]
      21.  Copy        4      [0,8]   Add        1        0    [247,255]
   --------------------------------------------------------------------

		     Reading the source: Overview

   This file includes itself in several passes to macro-expand certain
   sections with variable forms.  Just read ahead, there's only a
   little confusion.  I know this sounds ugly, but hard-coding some of
   the string-matching parameters results in a 10-15% increase in
   string-match performance.  The only time this hurts is when you have
   unbalanced #if/endifs.

   A single compilation unit tames the Makefile.  In short, this is to
   allow the above-described hack without an explodingMakefile.  The
   single compilation unit includes the core library features,
   configurable string-match templates, optional main() command-line
   tool, misc optional features, and a regression test.  Features are
   controled with CPP #defines, see Makefile.am.

   The initial __XDELTA3_C_HEADER_PASS__ starts first, the _INLINE_ and
   _TEMPLATE_ sections follow.  Easy stuff first, hard stuff last.

   Optional features include:

     xdelta3-main.h     The command-line interface, external compression
                        support, POSIX-specific, info & VCDIFF-debug tools.
     xdelta3-second.h   The common secondary compression routines.
     xdelta3-decoder.h  All decoding routines.
     xdelta3-djw.h      The semi-adaptive huffman secondary encoder.
     xdelta3-fgk.h      The adaptive huffman secondary encoder.
     xdelta3-test.h     The unit test covers major algorithms,
                        encoding and decoding.  There are single-bit
                        error decoding tests.  There are 32/64-bit file size
                        boundary tests.  There are command-line tests.
                        There are compression tests.  There are external
                        compression tests.  There are string-matching tests.
			There should be more tests...

   Additional headers include:

     xdelta3.h          The public header file.
     xdelta3-cfgs.h     The default settings for default, built-in
                        encoders.  These are hard-coded at
                        compile-time.  There is also a single
                        soft-coded string matcher for experimenting
                        with arbitrary values.
     xdelta3-list.h     A cyclic list template

   Misc little debug utilities:

     badcopy.c          Randomly modifies an input file based on two
                        parameters: (1) the probability that a byte in
                        the file is replaced with a pseudo-random value,
                        and (2) the mean change size.  Changes are
                        generated using an expoential distribution
                        which approximates the expected error_prob
			distribution.
   --------------------------------------------------------------------

   This file itself is unusually large.  I hope to defend this layout
   with lots of comments.  Everything in this file is related to
   encoding and decoding.  I like it all together - the template stuff
   is just a hack. */

#ifndef __XDELTA3_C_HEADER_PASS__
#define __XDELTA3_C_HEADER_PASS__

#include "xdelta3-iface.h"
#include "xdelta3.h"

#include <errno.h>
#include <string.h>

/***********************************************************************/

  /* header indicator bits */
#define VCD_SECONDARY (1U << 0)  /* uses secondary compressor */
#define VCD_CODETABLE (1U << 1)  /* supplies code table data */
#define VCD_APPHEADER (1U << 2)  /* supplies application data */
#define VCD_INVHDR    (~0x7U)

  /* window indicator bits */
#define VCD_SOURCE   (1U << 0)  /* copy window in source file */
#define VCD_TARGET   (1U << 1)  /* copy window in target file */
#define VCD_ADLER32  (1U << 2)  /* has adler32 checksum */
#define VCD_INVWIN   (~0x7U)

#define VCD_SRCORTGT (VCD_SOURCE | VCD_TARGET)

  /* delta indicator bits */
#define VCD_DATACOMP (1U << 0)
#define VCD_INSTCOMP (1U << 1)
#define VCD_ADDRCOMP (1U << 2)
#define VCD_INVDEL   (~0x7U)

typedef enum {
  VCD_DJW_ID    = 1,
  VCD_LZMA_ID   = 2,
  VCD_FGK_ID    = 16  /* Note: these are not standard IANA-allocated IDs! */
} xd3_secondary_ids;

typedef enum {
  DATA_SECTION, /* These indicate which section to the secondary
                 * compressor. */
  INST_SECTION, /* The header section is not compressed, therefore not
                 * listed here. */
  ADDR_SECTION
} xd3_section_type;

typedef unsigned int xd3_rtype;

/***********************************************************************/

#include "xdelta3-list.h"

/***********************************************************************/

#define VCDIFF_MAGIC1  0xd6  /* 1st file byte */
#define VCDIFF_MAGIC2  0xc3  /* 2nd file byte */
#define VCDIFF_MAGIC3  0xc4  /* 3rd file byte */
#define VCDIFF_VERSION 0x00  /* 4th file byte */

#define VCD_SELF       0     /* 1st address mode */
#define VCD_HERE       1     /* 2nd address mode */

#define MIN_MATCH         4U  /* VCDIFF code table: MIN_MATCH=4 */
#define MIN_RUN           8U  /* The shortest run, if it is shorter than this
			       * an immediate add/copy will be just as good.
			       * ADD1/COPY6 = 1I+1D+1A bytes, RUN18 =
			       * 1I+1D+1A. */

#define MAX_MODES         9  /* Maximum number of nodes used for
			      * compression--does not limit decompression. */


/* Update the run-length state */
#define NEXTRUN(c) do { if ((c) == run_c) { run_l += 1; } \
  else { run_c = (c); run_l = 1; } } while (0)

/***********************************************************************/




static int         xd3_decode_allocate (xd3_stream *stream, usize_t size,
					uint8_t **copied1, usize_t *alloc1);

static void*       xd3_alloc (xd3_stream *stream, usize_t elts, usize_t size);
static void        xd3_free  (xd3_stream *stream, void *ptr);

/***********************************************************************/

#define UINT32_OFLOW_MASK 0xfe000000U
#define UINT64_OFLOW_MASK 0xfe00000000000000ULL

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295U
#endif

#ifndef UINT64_MAX
#define UINT64_MAX 18446744073709551615ULL
#endif

#if SIZEOF_USIZE_T == 4
#define USIZE_T_MAX        UINT32_MAX
#define xd3_decode_size   xd3_decode_uint32_t
#define xd3_sizeof_size   xd3_sizeof_uint32_t
#define xd3_read_size     xd3_read_uint32_t
#elif SIZEOF_USIZE_T == 8
#define USIZE_T_MAX        UINT64_MAX
#define xd3_decode_size   xd3_decode_uint64_t
#define xd3_sizeof_size   xd3_sizeof_uint64_t
#define xd3_read_size     xd3_read_uint64_t
#endif

#if SIZEOF_XOFF_T == 4
#define XOFF_T_MAX        UINT32_MAX
#define xd3_decode_offset xd3_decode_uint32_t
#define xd3_emit_offset   xd3_emit_uint32_t
#elif SIZEOF_XOFF_T == 8
#define XOFF_T_MAX        UINT64_MAX
#define xd3_decode_offset xd3_decode_uint64_t
#define xd3_emit_offset   xd3_emit_uint64_t
#endif

#define USIZE_T_OVERFLOW(a,b) ((USIZE_T_MAX - (usize_t) (a)) < (usize_t) (b))
#define XOFF_T_OVERFLOW(a,b) ((XOFF_T_MAX - (xoff_t) (a)) < (xoff_t) (b))

const char* xd3_strerror (int ret)
{
  switch (ret)
    {
    case XD3_INPUT: return "XD3_INPUT";
    case XD3_OUTPUT: return "XD3_OUTPUT";
    case XD3_GETSRCBLK: return "XD3_GETSRCBLK";
    case XD3_GOTHEADER: return "XD3_GOTHEADER";
    case XD3_WINSTART: return "XD3_WINSTART";
    case XD3_WINFINISH: return "XD3_WINFINISH";
    case XD3_TOOFARBACK: return "XD3_TOOFARBACK";
    case XD3_INTERNAL: return "XD3_INTERNAL";
    case XD3_INVALID: return "XD3_INVALID";
    case XD3_INVALID_INPUT: return "XD3_INVALID_INPUT";
    case XD3_NOSECOND: return "XD3_NOSECOND";
    case XD3_UNIMPLEMENTED: return "XD3_UNIMPLEMENTED";
    }
  return NULL;
}

/***********************************************************************/

#define FGK_CASE(s) \
  s->msg = "unavailable secondary compressor: FGK Adaptive Huffman"; \
  return XD3_INTERNAL;

#define DJW_CASE(s) \
  s->msg = "unavailable secondary compressor: DJW Static Huffman"; \
  return XD3_INTERNAL;

#define LZMA_CASE(s) \
  s->msg = "unavailable secondary compressor: LZMA"; \
  return XD3_INTERNAL;

/***********************************************************************/

/* Process the inline pass. */
#define __XDELTA3_C_INLINE_PASS__
#include "xdelta3.c"
#undef __XDELTA3_C_INLINE_PASS__

#include "xdelta3-main.h"

#endif /* __XDELTA3_C_HEADER_PASS__ */
#ifdef __XDELTA3_C_INLINE_PASS__

/****************************************************************
 Instruction tables
 *****************************************************************/

/* The following code implements a parametrized description of the
 * code table given above for a few reasons.  It is not necessary for
 * implementing the standard, to support compression with variable
 * tables, so an implementation is only required to know the default
 * code table to begin decompression.  (If the encoder uses an
 * alternate table, the table is included in compressed form inside
 * the VCDIFF file.)
 *
 * Before adding variable-table support there were two functions which
 * were hard-coded to the default table above.
 * xd3_compute_default_table() would create the default table by
 * filling a 256-elt array of xd3_dinst values.  The corresponding
 * function, xd3_choose_instruction(), would choose an instruction
 * based on the hard-coded parameters of the default code table.
 *
 * Notes: The parametrized code table description here only generates
 * tables of a certain regularity similar to the default table by
 * allowing to vary the distribution of single- and
 * double-instructions and change the number of near and same copy
 * modes.  More exotic tables are only possible by extending this
 * code.
 *
 * For performance reasons, both the parametrized and non-parametrized
 * versions of xd3_choose_instruction remain.  The parametrized
 * version is only needed for testing multi-table decoding support.
 * If ever multi-table encoding is required, this can be optimized by
 * compiling static functions for each table.
 */

/* The XD3_CHOOSE_INSTRUCTION calls xd3_choose_instruction with the
 * table description when GENERIC_ENCODE_TABLES are in use.  The
 * IF_GENCODETBL macro enables generic-code-table specific code
 * (removed 10/2014). */
#define XD3_CHOOSE_INSTRUCTION(stream,prev,inst) \
  xd3_choose_instruction (prev, inst)

/* This structure maintains information needed by
 * xd3_choose_instruction to compute the code for a double instruction
 * by first indexing an array of code_table_sizes by copy mode, then
 * using (offset + (muliplier * X)) */
struct _xd3_code_table_sizes {
  uint8_t cpy_max;
  uint8_t offset;
  uint8_t mult;
};

/* This contains a complete description of a code table. */
struct _xd3_code_table_desc
{
  /* Assumes a single RUN instruction */
  /* Assumes that MIN_MATCH is 4 */

  uint8_t add_sizes;            /* Number of immediate-size single
				   adds (default 17) */
  uint8_t near_modes;           /* Number of near copy modes (default 4) */
  uint8_t same_modes;           /* Number of same copy modes (default 3) */
  uint8_t cpy_sizes;            /* Number of immediate-size single
				   copies (default 15) */

  uint8_t addcopy_add_max;      /* Maximum add size for an add-copy
				   double instruction, all modes
				   (default 4) */
  uint8_t addcopy_near_cpy_max; /* Maximum cpy size for an add-copy
				   double instruction, up through
				   VCD_NEAR modes (default 6) */
  uint8_t addcopy_same_cpy_max; /* Maximum cpy size for an add-copy
				   double instruction, VCD_SAME modes
				   (default 4) */

  uint8_t copyadd_add_max;      /* Maximum add size for a copy-add
				   double instruction, all modes
				   (default 1) */
  uint8_t copyadd_near_cpy_max; /* Maximum cpy size for a copy-add
				   double instruction, up through
				   VCD_NEAR modes (default 4) */
  uint8_t copyadd_same_cpy_max; /* Maximum cpy size for a copy-add
				   double instruction, VCD_SAME modes
				   (default 4) */

  xd3_code_table_sizes addcopy_max_sizes[MAX_MODES];
  xd3_code_table_sizes copyadd_max_sizes[MAX_MODES];
};

/* The rfc3284 code table is represented: */
static const xd3_code_table_desc __rfc3284_code_table_desc = {
  17, /* add sizes */
  4,  /* near modes */
  3,  /* same modes */
  15, /* copy sizes */

  4,  /* add-copy max add */
  6,  /* add-copy max cpy, near */
  4,  /* add-copy max cpy, same */

  1,  /* copy-add max add */
  4,  /* copy-add max cpy, near */
  4,  /* copy-add max cpy, same */

  /* addcopy */
  { {6,163,3},{6,175,3},{6,187,3},{6,199,3},{6,211,3},{6,223,3},
    {4,235,1},{4,239,1},{4,243,1} },
  /* copyadd */
  { {4,247,1},{4,248,1},{4,249,1},{4,250,1},{4,251,1},{4,252,1},
    {4,253,1},{4,254,1},{4,255,1} },
};

/* Computes code table entries of TBL using the specified description. */
static void
xd3_build_code_table (const xd3_code_table_desc *desc, xd3_dinst *tbl)
{
  uint8_t size1, size2;
  uint8_t mode;
  usize_t cpy_modes = 2U + desc->near_modes + desc->same_modes;
  xd3_dinst *d = tbl;

  (d++)->type1 = XD3_RUN;
  (d++)->type1 = XD3_ADD;

  for (size1 = 1; size1 <= desc->add_sizes; size1 += 1, d += 1)
    {
      d->type1 = XD3_ADD;
      d->size1 = size1;
    }

  for (mode = 0; mode < cpy_modes; mode += 1)
    {
      (d++)->type1 = XD3_CPY + mode;

      for (size1 = MIN_MATCH; size1 < MIN_MATCH + desc->cpy_sizes;
	   size1 += 1, d += 1)
	{
	  d->type1 = XD3_CPY + mode;
	  d->size1 = size1;
	}
    }

  for (mode = 0; mode < cpy_modes; mode += 1)
    {
      for (size1 = 1; size1 <= desc->addcopy_add_max; size1 += 1)
	{
	  usize_t max = (mode < 2U + desc->near_modes) ?
	    desc->addcopy_near_cpy_max :
	    desc->addcopy_same_cpy_max;

	  for (size2 = MIN_MATCH; size2 <= max; size2 += 1, d += 1)
	    {
	      d->type1 = XD3_ADD;
	      d->size1 = size1;
	      d->type2 = XD3_CPY + mode;
	      d->size2 = size2;
	    }
	}
    }

  for (mode = 0; mode < cpy_modes; mode += 1)
    {
      usize_t max = (mode < 2U + desc->near_modes) ?
	desc->copyadd_near_cpy_max :
	desc->copyadd_same_cpy_max;

      for (size1 = MIN_MATCH; size1 <= max; size1 += 1)
	{
	  for (size2 = 1; size2 <= desc->copyadd_add_max; size2 += 1, d += 1)
	    {
	      d->type1 = XD3_CPY + mode;
	      d->size1 = size1;
	      d->type2 = XD3_ADD;
	      d->size2 = size2;
	    }
	}
    }

  XD3_ASSERT (d - tbl == 256);
}

/* This function generates the static default code table. */
static const xd3_dinst*
xd3_rfc3284_code_table (void)
{
  static xd3_dinst __rfc3284_code_table[256];

  if (__rfc3284_code_table[0].type1 != XD3_RUN)
    {
      xd3_build_code_table (& __rfc3284_code_table_desc, __rfc3284_code_table);
    }

  return __rfc3284_code_table;
}

/***********************************************************************/

static inline void
xd3_swap_uint8p (uint8_t** p1, uint8_t** p2)
{
  uint8_t *t = (*p1);
  (*p1) = (*p2);
  (*p2) = t;
}

static inline void
xd3_swap_usize_t (usize_t* p1, usize_t* p2)
{
  usize_t t = (*p1);
  (*p1) = (*p2);
  (*p2) = t;
}

/* It's not constant time, but it computes the log. */
static int
xd3_check_pow2 (usize_t value, usize_t *logof)
{
  usize_t x = 1;
  usize_t nolog;
  if (logof == NULL) {
    logof = &nolog;
  }

  *logof = 0;

  for (; x != 0; x <<= 1, *logof += 1)
    {
      if (x == value)
	{
	  return 0;
	}
    }

  return XD3_INTERNAL;
}

static usize_t
xd3_pow2_roundup (usize_t x)
{
  usize_t i = 1;
  while (x > i) {
    i <<= 1U;
  }
  return i;
}

static usize_t
xd3_round_blksize (usize_t sz, usize_t blksz)
{
  usize_t mod = sz & (blksz-1);

  XD3_ASSERT (xd3_check_pow2 (blksz, NULL) == 0);

  if (mod == 0)
    {
      return sz;
    }

  if (sz > USIZE_T_MAXBLKSZ)
    {
      return USIZE_T_MAXBLKSZ;
    }

  return sz + (blksz - mod);
}

/***********************************************************************
 Adler32 stream function: code copied from Zlib, defined in RFC1950
 ***********************************************************************/

#define A32_BASE 65521L /* Largest prime smaller than 2^16 */
#define A32_NMAX 5552   /* NMAX is the largest n such that 255n(n+1)/2
			   + (n+1)(BASE-1) <= 2^32-1 */

#define A32_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define A32_DO2(buf,i)  A32_DO1(buf,i); A32_DO1(buf,i+1);
#define A32_DO4(buf,i)  A32_DO2(buf,i); A32_DO2(buf,i+2);
#define A32_DO8(buf,i)  A32_DO4(buf,i); A32_DO4(buf,i+4);
#define A32_DO16(buf)   A32_DO8(buf,0); A32_DO8(buf,8);

static uint32_t adler32 (uint32_t adler, const uint8_t *buf, usize_t len)
{
    uint32_t s1 = adler & 0xffffU;
    uint32_t s2 = (adler >> 16) & 0xffffU;
    int k;

    while (len > 0)
      {
        k    = (len < A32_NMAX) ? len : A32_NMAX;
        len -= k;

	while (k >= 16)
	  {
	    A32_DO16(buf);
	    buf += 16;
            k -= 16;
	  }

	if (k != 0)
	  {
	    do
	      {
		s1 += *buf++;
		s2 += s1;
	      }
	    while (--k);
	  }

        s1 %= A32_BASE;
        s2 %= A32_BASE;
    }

    return (s2 << 16) | s1;
}

/***********************************************************************
 Run-length function
 ***********************************************************************/

/*********************************************************************
 Integer encoder/decoder functions
 **********************************************************************/

/* Consume N bytes of input, only used by the decoder. */
#define DECODE_INPUT(n)             \
  do {                              \
  stream->total_in += (xoff_t) (n); \
  stream->avail_in -= (n);          \
  stream->next_in  += (n);          \
  } while (0)

#define DECODE_INTEGER_TYPE(PART,OFLOW)                                \
  while (stream->avail_in != 0)                                        \
    {                                                                  \
      usize_t next = stream->next_in[0];                               \
                                                                       \
      DECODE_INPUT(1);                                                 \
                                                                       \
      if (PART & OFLOW)                                                \
	{                                                              \
	  stream->msg = "overflow in decode_integer";                  \
	  return XD3_INVALID_INPUT;                                    \
	}                                                              \
                                                                       \
      PART = (PART << 7) | (next & 127);                               \
                                                                       \
      if ((next & 128) == 0)                                           \
	{                                                              \
	  (*val) = PART;                                               \
	  PART = 0;                                                    \
	  return 0;                                                    \
	}                                                              \
    }                                                                  \
                                                                       \
  stream->msg = "further input required";                              \
  return XD3_INPUT

#define READ_INTEGER_TYPE(TYPE, OFLOW)                                 \
  TYPE val = 0;                                                        \
  const uint8_t *inpt = (*inpp);                                        \
  usize_t next;                                                        \
                                                                       \
  do                                                                   \
    {                                                                  \
      if (inpt == maxp)						       \
	{                                                              \
	  stream->msg = "end-of-input in read_integer";                \
	  return XD3_INVALID_INPUT;                                    \
	}                                                              \
                                                                       \
      if (val & OFLOW)                                                 \
	{                                                              \
	  stream->msg = "overflow in read_intger";                     \
	  return XD3_INVALID_INPUT;                                    \
	}                                                              \
                                                                       \
      next = (*inpt++);                                                 \
      val  = (val << 7) | (next & 127);                                \
    }                                                                  \
  while (next & 128);                                                  \
                                                                       \
  (*valp) = val;                                                       \
  (*inpp) = inpt;                                                       \
                                                                       \
  return 0

#define IF_SIZEOF32(x) if (num < (1U   << (7 * (x)))) return (x);
#define IF_SIZEOF64(x) if (num < (1ULL << (7 * (x)))) return (x);

#if USE_UINT32
static inline uint32_t
xd3_sizeof_uint32_t (uint32_t num)
{
  IF_SIZEOF32(1);
  IF_SIZEOF32(2);
  IF_SIZEOF32(3);
  IF_SIZEOF32(4);
  return 5;
}

static inline int
xd3_decode_uint32_t (xd3_stream *stream, uint32_t *val)
{ DECODE_INTEGER_TYPE (stream->dec_32part, UINT32_OFLOW_MASK); }

static inline int
xd3_read_uint32_t (xd3_stream *stream, const uint8_t **inpp,
		   const uint8_t *maxp, uint32_t *valp)
{ READ_INTEGER_TYPE (uint32_t, UINT32_OFLOW_MASK); }
#endif

#if USE_UINT64
static inline int
xd3_decode_uint64_t (xd3_stream *stream, uint64_t *val)
{ DECODE_INTEGER_TYPE (stream->dec_64part, UINT64_OFLOW_MASK); }

#endif

/***********************************************************************
 Address cache stuff
 ***********************************************************************/

static int
xd3_alloc_cache (xd3_stream *stream)
{
  if (stream->acache.near_array != NULL)
    {
      xd3_free (stream, stream->acache.near_array);
    }

  if (stream->acache.same_array != NULL)
    {
      xd3_free (stream, stream->acache.same_array);
    }

  if (((stream->acache.s_near > 0) &&
       (stream->acache.near_array = (usize_t*)
	xd3_alloc (stream, stream->acache.s_near,
		   (usize_t) sizeof (usize_t)))
       == NULL) ||
      ((stream->acache.s_same > 0) &&
       (stream->acache.same_array = (usize_t*)
	xd3_alloc (stream, stream->acache.s_same * 256,
		   (usize_t) sizeof (usize_t)))
       == NULL))
    {
      return ENOMEM;
    }

  return 0;
}

void
xd3_init_cache (xd3_addr_cache* acache)
{
  if (acache->s_near > 0)
    {
      memset (acache->near_array, 0, acache->s_near * sizeof (usize_t));
      acache->next_slot = 0;
    }

  if (acache->s_same > 0)
    {
      memset (acache->same_array, 0, acache->s_same * 256 * sizeof (usize_t));
    }
}

static void
xd3_update_cache (xd3_addr_cache* acache, usize_t addr)
{
  if (acache->s_near > 0)
    {
      acache->near_array[acache->next_slot] = addr;
      acache->next_slot = (acache->next_slot + 1) % acache->s_near;
    }

  if (acache->s_same > 0)
    {
      acache->same_array[addr % (acache->s_same*256)] = addr;
    }
}

static int
xd3_decode_address (xd3_stream *stream, usize_t here,
		    usize_t mode, const uint8_t **inpp,
		    const uint8_t *max, uint32_t *valp)
{
  int ret;
  usize_t same_start = 2 + stream->acache.s_near;

  if (mode < same_start)
    {
      if ((ret = xd3_read_size (stream, inpp, max, valp)) != 0) { return ret; }

      switch (mode)
	{
	case VCD_SELF:
	  break;
	case VCD_HERE:
	  (*valp) = here - (*valp);
	  break;
	default:
	  (*valp) += stream->acache.near_array[mode - 2];
	  break;
	}
    }
  else
    {
      if (*inpp == max)
	{
	  stream->msg = "address underflow";
	  return XD3_INVALID_INPUT;
	}

      mode -= same_start;

      (*valp) = stream->acache.same_array[mode*256 + (**inpp)];

      (*inpp) += 1;
    }

  xd3_update_cache (& stream->acache, *valp);

  return 0;
}

/***********************************************************************
 Alloc/free
***********************************************************************/

static void*
__xd3_alloc_func (void* opaque, usize_t items, usize_t size)
{
  (void) opaque;
  return malloc ((size_t) items * (size_t) size);
}

static void
__xd3_free_func (void* opaque, void* address)
{
  (void) opaque;
  free (address);
}

static void*
xd3_alloc (xd3_stream *stream,
	   usize_t      elts,
	   usize_t      size)
{
  void *a = stream->alloc (stream->opaque, elts, size);

  if (a == NULL)
    {
      stream->msg = "out of memory";
    }

  return a;
}

static void
xd3_free (xd3_stream *stream,
	  void       *ptr)
{
  if (ptr != NULL)
    {
      stream->free (stream->opaque, ptr);
    }
}

void
xd3_free_stream (xd3_stream *stream)
{
  xd3_free (stream, stream->acache.near_array);
  xd3_free (stream, stream->acache.same_array);

  xd3_free (stream, stream->inst_sect.copied1);
  xd3_free (stream, stream->addr_sect.copied1);
  xd3_free (stream, stream->data_sect.copied1);

  if (stream->dec_lastwin != stream->dec_buffer)
    {
      xd3_free (stream, (uint8_t*) stream->dec_lastwin);
    }
  xd3_free (stream, stream->dec_buffer);

  xd3_free (stream, stream->dec_appheader);
  xd3_free (stream, stream->dec_codetbl);
  xd3_free (stream, stream->code_table_alloc);

  memset (stream, 0, sizeof (xd3_stream));
}

/****************************************************************
 Stream configuration
 ******************************************************************/

int
xd3_config_stream(xd3_stream *stream,
		  xd3_config *config)
{
  int ret;
  xd3_config defcfg;

  if (config == NULL)
    {
      config = & defcfg;
      memset (config, 0, sizeof (*config));
    }

  /* Initial setup: no error checks yet */
  memset (stream, 0, sizeof (*stream));

  stream->winsize = config->winsize ? config->winsize : XD3_DEFAULT_WINSIZE;

  stream->getblk    = config->getblk;
  stream->alloc     = config->alloc ? config->alloc : __xd3_alloc_func;
  stream->free      = config->freef ? config->freef : __xd3_free_func;
  stream->opaque    = config->opaque;
  stream->flags     = config->flags;

  /* Check static sizes. */
  if (sizeof (usize_t) != SIZEOF_USIZE_T ||
      sizeof (xoff_t) != SIZEOF_XOFF_T ||
      (ret = xd3_check_pow2(XD3_ALLOCSIZE, NULL)) != 0)
    {
      stream->msg = "incorrect compilation: wrong integer sizes";
      return XD3_INTERNAL;
    }

  /* Check/set secondary compressor. */
  switch (stream->flags & XD3_SEC_TYPE)
    {
    case 0:
      if (stream->flags & XD3_SEC_NOALL)
	{
	  stream->msg = "XD3_SEC flags require a secondary compressor type";
	  return XD3_INTERNAL;
	}
      break;
    case XD3_SEC_FGK:
      FGK_CASE (stream);
    case XD3_SEC_DJW:
      DJW_CASE (stream);
    case XD3_SEC_LZMA:
      LZMA_CASE (stream);
    default:
      stream->msg = "too many secondary compressor types set";
      return XD3_INTERNAL;
    }

  stream->code_table_desc = & __rfc3284_code_table_desc;
  stream->code_table_func = xd3_rfc3284_code_table;

  return 0;
}

/***********************************************************
 Getblk interface
 ***********************************************************/

/* This function interfaces with the client getblk function, checks
 * its results, updates max_blkno, onlastblk. */
static int
xd3_getblk (xd3_stream *stream, xoff_t blkno)
{
  int ret;
  xd3_source *source = stream->src;

  if (source->curblk == NULL || blkno != source->curblkno)
    {
      source->getblkno = blkno;

      if (stream->getblk == NULL)
	{
	  stream->msg = "getblk source input";
	  return XD3_GETSRCBLK;
	}

      ret = stream->getblk (stream, source, blkno);
      if (ret != 0)
	{
	  return ret;
	}
    }

  if (blkno > source->max_blkno)
    {
      source->max_blkno = blkno;
    }

  XD3_ASSERT (source->curblk != NULL);

  if (blkno == source->max_blkno)
    {
      /* In case the application sets the source as 1 block w/ a
       * preset buffer. */
      source->onlastblk = source->onblk;
    }
  return 0;
}

/***********************************************************
 Stream open/close
 ***************************************************************/

int
xd3_set_source (xd3_stream *stream,
		xd3_source *src)
{
  usize_t shiftby;

  stream->src = src;
  src->srclen  = 0;
  src->srcbase = 0;

  /* Enforce power-of-two blocksize so that source-block number
   * calculations are cheap. */
  if (xd3_check_pow2 (src->blksize, &shiftby) != 0)
    {
      src->blksize = xd3_pow2_roundup(src->blksize);
      xd3_check_pow2 (src->blksize, &shiftby);
    }

  src->shiftby = shiftby;
  src->maskby = (1 << shiftby) - 1;
  return 0;
}

int
xd3_set_source_and_size (xd3_stream *stream,
			 xd3_source *user_source,
			 xoff_t source_size) {
  int ret = xd3_set_source (stream, user_source);
  if (ret == 0)
    {
      xd3_blksize_div(source_size,
		      stream->src,
		      &stream->src->max_blkno,
		      &stream->src->onlastblk);
    }
  return ret;
}

void
xd3_abort_stream (xd3_stream *stream)
{
  stream->dec_state = DEC_ABORTED;
}

int
xd3_close_stream (xd3_stream *stream)
{
      switch (stream->dec_state)
	{
	case DEC_VCHEAD:
	case DEC_WININD:
	  /* TODO: Address the zero-byte ambiguity.  Does the encoder
	   * emit a window or not?  If so, then catch an error here.
	   * If not, need another routine to say
	   * decode_at_least_one_if_empty. */
	case DEC_ABORTED:
	  break;
	default:
	  /* If decoding, should be ready for the next window. */
	  stream->msg = "eof in decode";
	  return XD3_INVALID_INPUT;
	}

  return 0;
}

/**********************************************************
 Decoder stuff
 *************************************************/

#include "xdelta3-decode.h"

/*****************************************************************
 Client convenience functions
 ******************************************************************/

static int
xd3_process_stream (xd3_stream    *stream,
		    int          (*func) (xd3_stream *),
		    int            close_stream,
		    const uint8_t *input,
		    usize_t        input_size,
		    uint8_t       *output,
		    usize_t       *output_size,
		    usize_t        output_size_max)
{
  usize_t ipos = 0;
  usize_t n = xd3_min (stream->winsize, input_size);

  (*output_size) = 0;

  stream->flags |= XD3_FLUSH;

  xd3_avail_input (stream, input + ipos, n);
  ipos += n;

  for (;;)
    {
      int ret;
      switch ((ret = func (stream)))
	{
	case XD3_OUTPUT: { /* memcpy below */ break; }
	case XD3_INPUT: {
	  n = xd3_min(stream->winsize, input_size - ipos);
	  if (n == 0) 
	    {
	      goto done;
	    }
	  xd3_avail_input (stream, input + ipos, n);
	  ipos += n;
	  continue;
	}
	case XD3_GOTHEADER: { /* ignore */ continue; }
	case XD3_WINSTART: { /* ignore */ continue; }
	case XD3_WINFINISH: { /* ignore */ continue; }
	case XD3_GETSRCBLK:
	  {
	    /* When the getblk function is NULL, it is necessary to
	     * provide the complete source as a single block using
	     * xd3_set_source_and_size, otherwise this error.  The
	     * library should never ask for another source block. */
	    stream->msg = "library requested source block";
	    return XD3_INTERNAL;
	  }
	case 0:
	  {
	    /* xd3_encode_input/xd3_decode_input never return 0 */
	    stream->msg = "invalid return: 0";
	    return XD3_INTERNAL;
	  }
	default:
	  return ret;
	}

      if (*output_size + stream->avail_out > output_size_max)
	{
	  stream->msg = "insufficient output space";
	  return ENOSPC;
	}

      memcpy (output + *output_size, stream->next_out, stream->avail_out);

      *output_size += stream->avail_out;

      xd3_consume_output (stream);
    }
 done:
  return (close_stream == 0) ? 0 : xd3_close_stream (stream);
}

static int
xd3_process_memory (int          (*func) (xd3_stream *),
		    const uint8_t *input,
		    usize_t        input_size,
		    const uint8_t *source,
		    usize_t        source_size,
		    uint8_t       *output,
		    usize_t       *output_size,
		    usize_t        output_size_max,
		    int            flags) {
  xd3_stream stream;
  xd3_config config;
  xd3_source src;
  int ret;

  memset (& stream, 0, sizeof (stream));
  memset (& config, 0, sizeof (config));

  if (input == NULL || output == NULL) {
    stream.msg = "invalid input/output buffer";
    ret = XD3_INTERNAL;
    goto exit;
  }

  config.flags = flags;

  if ((ret = xd3_config_stream (&stream, &config)) != 0)
    {
      goto exit;
    }

  if (source != NULL)
    {
      memset (& src, 0, sizeof (src));

      src.blksize = source_size;
      src.onblk = source_size;
      src.curblk = source;
      src.curblkno = 0;

      if ((ret = xd3_set_source_and_size (&stream, &src, source_size)) != 0)
	{
	  goto exit;
	}
    }

  if ((ret = xd3_process_stream (& stream,
				 func, 1,
				 input, input_size,
				 output,
				 output_size,
				 output_size_max)) != 0)
    {
      goto exit;
    }

 exit:
  xd3_free_stream(&stream);
  return ret;
}

int
xd3_decode_stream (xd3_stream    *stream,
		   const uint8_t *input,
		   usize_t        input_size,
		   uint8_t       *output,
		   usize_t       *output_size,
		   usize_t        output_size_max)
{
  return xd3_process_stream (stream, & xd3_decode_input, 1,
			     input, input_size,
			     output, output_size, output_size_max);
}

int
xd3_decode_memory (const uint8_t *input,
		   usize_t        input_size,
		   const uint8_t *source,
		   usize_t        source_size,
		   uint8_t       *output,
		   usize_t       *output_size,
		   usize_t        output_size_max,
		   int            flags) {
  return xd3_process_memory (& xd3_decode_input,
			     input, input_size,
			     source, source_size,
			     output, output_size, output_size_max,
			     flags);
}

#endif /* __XDELTA3_C_INLINE_PASS__ */
