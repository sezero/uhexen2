/* xdelta 3 - delta compression tools and library
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007,
 * 2008, 2009, 2010
 * Joshua P. MacDonald
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
 */

/* TODO: This code is heavily revised from 3.0z but still needs major
 * refactoring. */

typedef struct _main_blklru      main_blklru;
typedef struct _main_blklru_list main_blklru_list;

struct _main_blklru_list
{
  main_blklru_list  *next;
  main_blklru_list  *prev;
};

struct _main_blklru
{
  uint8_t          *blk;
  xoff_t            blkno;
  usize_t           size;
  main_blklru_list  link;
};

/* TODO: The value for MAX_LRU_SIZE has what significance? */
#define MAX_LRU_SIZE 32U  /* must be a power of 2 */
#define XD3_MINSRCWINSZ (XD3_ALLOCSIZE * MAX_LRU_SIZE)

XD3_MAKELIST(main_blklru_list,main_blklru,link);

static usize_t           lru_size = 0;
static main_blklru      *lru = NULL;  /* array of lru_size elts */
static main_blklru_list  lru_list;

static void main_lru_reset (void)
{
  lru_size = 0;
  lru = NULL;
}

static void main_lru_cleanup (void)
{
  if (lru != NULL)
    {
      main_free (lru[0].blk);
    }

  main_free (lru);
  lru = NULL;
}

/* This is called at different times for encoding and decoding.  The
 * encoder calls it immediately, the decoder delays until the
 * application header is received.  */
static int
main_set_source (xd3_stream *stream,
		 main_file *sfile, xd3_source *source)
{
  int ret = 0;
  usize_t i;
  xoff_t source_size = 0;
  usize_t blksize;

  XD3_ASSERT (lru == NULL);
  XD3_ASSERT (stream->src == NULL);
  XD3_ASSERT (use_options->srcwinsz >= XD3_MINSRCWINSZ);

  /* TODO: this code needs refactoring into FIFO, LRU, FAKE.  Yuck!
   * This is simplified from 3.0z which had issues with sizing the
   * source buffer memory allocation and the source blocksize. */

  /* LRU-specific */
  main_blklru_list_init (& lru_list);

      /* Either a regular file (possibly compressed) or a FIFO
       * (possibly compressed). */
      if ((ret = main_file_open (sfile, sfile->filename, XO_READ)) != 0)
	{
	  return ret;
	}

      if ((ret = main_file_stat (sfile, &source_size)) != 0)
        {
	  return ret;
        }

  /* Note: The API requires a power-of-two blocksize and srcwinsz
   * (-B).  The logic here will use a single block if the entire file
   * is known to fit into srcwinsz. */
  use_options->srcwinsz = xd3_pow2_roundup (use_options->srcwinsz);

  /* Though called "lru", it is not LRU-specific.  We always allocate
   * a maximum number of source block buffers.  If the entire file
   * fits into srcwinsz, this buffer will stay as the only
   * (lru_size==1) source block.  Otherwise, we know that at least
   * use_options->srcwinsz bytes are available.  Split the source window
   * into buffers. */
  if ((lru = (main_blklru*) main_malloc (MAX_LRU_SIZE *
					 sizeof (main_blklru))) == NULL)
    {
      ret = ENOMEM;
      return ret;
    }

  memset (lru, 0, sizeof(lru[0]) * MAX_LRU_SIZE);

  /* Allocate the entire buffer. */
  if ((lru[0].blk = (uint8_t*) main_malloc (use_options->srcwinsz)) == NULL)
    {
      ret = ENOMEM;
      return ret;
    }

  /* Main calls main_getblk_func() once before xd3_set_source().  This
   * is the point at which external decompression may begin.  Set the
   * system for a single block. */
  lru_size = 1;
  lru[0].blkno = (xoff_t) -1;
  blksize = use_options->srcwinsz;
  main_blklru_list_push_back (& lru_list, & lru[0]);
  XD3_ASSERT (blksize != 0);

  /* Initialize xd3_source. */
  source->blksize  = blksize;
  source->name     = sfile->filename;
  source->ioh      = sfile;
  source->curblkno = (xoff_t) -1;
  source->curblk   = NULL;

  if ((ret = main_getblk_func (stream, source, 0)) != 0)
    {
      use_options->debug_print("error reading source: %s: %s\n",
	  sfile->filename,
	  xd3_mainerror (ret));
      return ret;
    }

  source->onblk = lru[0].size;  /* xd3 sets onblk */

  /* If the size is not known or is greater than the buffer size, we
   * split the buffer across MAX_LRU_SIZE blocks (already allocated in
   * "lru"). */
  if (source_size > use_options->srcwinsz)
    {
      /* Modify block 0, change blocksize. */
      blksize = use_options->srcwinsz / MAX_LRU_SIZE;
      source->blksize = blksize;
      source->onblk = blksize;
      source->onlastblk = blksize;
      source->max_blkno = MAX_LRU_SIZE - 1;

      lru[0].size = blksize;
      lru_size = MAX_LRU_SIZE;

      /* Setup rest of blocks. */
      for (i = 1; i < lru_size; i += 1)
	{
	  lru[i].blk = lru[0].blk + (blksize * i);
	  lru[i].blkno = i;
	  lru[i].size = blksize;
	  main_blklru_list_push_back (& lru_list, & lru[i]);
	}
    }

      ret = xd3_set_source_and_size (stream, source, source_size);

  if (ret)
    {
      use_options->debug_print(XD3_LIB_ERRMSG (stream, ret));
      return ret;
    }

  XD3_ASSERT (stream->src == source);
  XD3_ASSERT (source->blksize == blksize);

  if (use_options->verbose)
    {
      use_options->debug_print("source %s %" Q "u bytes, blksize %u bytes, window %u bytes, #bufs %u\n",
	  sfile->filename, source_size, blksize, use_options->srcwinsz, lru_size);
    }

  return 0;
}

static int
main_getblk_lru (xd3_source *source, xoff_t blkno,
		 main_blklru** blrup, int *is_new)
{
  main_blklru *blru = NULL;
  usize_t i;

  (void) source;

  (*is_new) = 0;

      /* Sequential search through LRU. */
      for (i = 0; i < lru_size; i += 1)
	{
	  blru = & lru[i];
	  if (blru->blkno == blkno)
	    {
	      main_blklru_list_remove (blru);
	      main_blklru_list_push_back (& lru_list, blru);
	      (*blrup) = blru;
	      return 0;
	    }
	}

      XD3_ASSERT (! main_blklru_list_empty (& lru_list));
      blru = main_blklru_list_pop_front (& lru_list);
      main_blklru_list_push_back (& lru_list, blru);

  (*is_new) = 1;
  (*blrup) = blru;
  blru->blkno = (xoff_t) -1;
  return 0;
}

static int
main_read_seek_source (xd3_stream *stream,
		       xd3_source *source,
		       xoff_t      blkno) {
  xoff_t pos = blkno * source->blksize;
  main_file *sfile = (main_file*) source->ioh;
  main_blklru *blru;
  int is_new;
  usize_t nread = 0;
  int ret = 0;

  if (!sfile->seek_failed)
    {
      ret = main_file_seek (sfile, pos);

      if (ret == 0)
	{
	  sfile->source_position = pos;
	}
    }

  if (sfile->seek_failed || ret != 0)
    {
      /* For an unseekable file (or other seek error, does it
       * matter?) */
      if (sfile->source_position > pos)
	{
	  /* this shouldn't happen */
	  if (use_options->verbose)
	    {
	      use_options->debug_print("source can't seek backwards; requested block offset "
		  "%" Q "u source position is %" Q "u\n",
		  pos, sfile->source_position);
	    }

	  sfile->seek_failed = 1;
	  stream->msg = "non-seekable source: "
	    "copy is too far back (try raising -B)";
	  return XD3_TOOFARBACK;
	}

      /* There's a chance here, that an genuine lseek error will cause
       * xdelta3 to shift into non-seekable mode, entering a degraded
       * condition.  */
      if (!sfile->seek_failed && use_options->verbose)
	{
	  use_options->debug_print("seek error at offset %" Q "u: %s\n",
		  pos, xd3_mainerror (ret));
	}

      sfile->seek_failed = 1;

      while (sfile->source_position < pos)
	{
	  xoff_t skip_blkno;
	  usize_t skip_offset;

	  xd3_blksize_div (sfile->source_position, source,
			   &skip_blkno, &skip_offset);

	  /* Read past unused data */
	  XD3_ASSERT (pos - sfile->source_position >= source->blksize);
	  XD3_ASSERT (skip_offset == 0);

	  if ((ret = main_getblk_lru (source, skip_blkno,
				      & blru, & is_new)) != 0)
	    {
	      return ret;
	    }

	  XD3_ASSERT (is_new);
	  blru->blkno = skip_blkno;

	  if ((ret = main_read_primary_input (sfile,
					      (uint8_t*) blru->blk,
					      source->blksize,
					      & nread)) != 0)
	    {
	      return ret;
	    }

	  if (nread != source->blksize)
	    {
	      stream->msg = "non-seekable input is short";
	      return XD3_INVALID_INPUT;
	    }

	  sfile->source_position += nread;
	  blru->size = nread;

	  XD3_ASSERT (sfile->source_position <= pos);
	}
    }

  return 0;
}

/* This is the callback for reading a block of source.  This function
 * is blocking and it implements a small LRU.
 *
 * Note that it is possible for main_input() to handle getblk requests
 * in a non-blocking manner.  If the callback is NULL then the caller
 * of xd3_*_input() must handle the XD3_GETSRCBLK return value and
 * fill the source in the same way.  See xd3_getblk for details.  To
 * see an example of non-blocking getblk, see xdelta-test.h. */
static int
main_getblk_func (xd3_stream *stream,
		  xd3_source *source,
		  xoff_t      blkno)
{
  int ret = 0;
  xoff_t pos = blkno * source->blksize;
  main_file *sfile = (main_file*) source->ioh;
  main_blklru *blru;
  int is_new;
  usize_t nread = 0;

  if ((ret = main_getblk_lru (source, blkno, & blru, & is_new)) != 0)
    {
      return ret;
    }

  if (!is_new)
    {
      source->curblkno = blkno;
      source->onblk    = blru->size;
      source->curblk   = blru->blk;
      return 0;
    }

  if (pos != sfile->source_position)
    {
      /* Only try to seek when the position is wrong.  This means the
       * decoder will fail when the source buffer is too small, but
       * only when the input is non-seekable. */
      if ((ret = main_read_seek_source (stream, source, blkno)) != 0)
	{
	  return ret;
	}
    }

  XD3_ASSERT (sfile->source_position == pos);

  if ((ret = main_read_primary_input (sfile,
				      (uint8_t*) blru->blk,
				      source->blksize,
				      & nread)) != 0)
    {
      return ret;
    }

  /* Save the last block read, used to handle non-seekable files. */
  sfile->source_position = pos + nread;

  source->curblk   = blru->blk;
  source->curblkno = blkno;
  source->onblk    = nread;
  blru->size       = nread;
  blru->blkno      = blkno;

  return 0;
}
