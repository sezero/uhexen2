/* xdelta 3 - delta compression tools and library
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007,
 * 2008, 2009, 2010, 2011
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

/* This is all the extra stuff you need for convenience to users in a
 * command line application.  It contains these major components:
 *
 * 1. VCDIFF tools 2. external compression support (this is
 * POSIX-specific).  3. a general read/write loop that handles all of
 * the Xdelta decode/encode/VCDIFF-print functions 4. command-line
 * interpreter 5. an Xdelta application header which stores default
 * filename, external compression settings 6. output/error printing
 * 7. basic file support and OS interface
 */

/* On error handling and printing:
 *
 * The xdelta library sets stream->msg to indicate what condition
 * caused an internal failure, but many failures originate here and
 * are printed here.  The return convention is 0 for success, as
 * throughout Xdelta code, but special attention is required here for
 * the operating system calls with different error handling.  See the
 * main_file_* routines.  All errors in this file have a message
 * printed at the time of occurance.  Since some of these calls occur
 * within calls to the library, the error may end up being printed
 * again with a more general error message.
 */

/*********************************************************************/

#ifndef XD3_POSIX
#define XD3_POSIX 0
#endif
#ifndef XD3_STDIO
#define XD3_STDIO 0
#endif
#ifndef XD3_WIN32
#define XD3_WIN32 0
#endif
#ifndef XD3_AMIGA
#define XD3_AMIGA 0
#endif

/* Combines xd3_strerror() and strerror() */
const char* xd3_mainerror(int err_num);

/* If none are set, default to posix. */
#if (XD3_POSIX + XD3_STDIO + XD3_WIN32 + XD3_AMIGA) == 0
#undef XD3_POSIX
#define XD3_POSIX 1
#endif

#define PRINTHDR_SPECIAL -4378291

/* this is used as in XPR(NT XD3_LIB_ERRMSG (stream, ret)) to print an
 * error message from the library. */
#define XD3_LIB_ERRMSG(stream, ret) "%s: %s\n", \
    xd3_errstring (stream), xd3_mainerror (ret)

#include <stdio.h>  /* fprintf */

#if XD3_POSIX
#include <unistd.h> /* close, read, write... */
#include <sys/types.h>
#include <fcntl.h>
#endif

#if defined(_WIN32)
	/* */
#elif defined(__DJGPP__)
#include <dos.h>
#include <io.h>
#include <unistd.h>
#elif (XD3_AMIGA)
#include <proto/exec.h>
#include <proto/dos.h>
#else /* POSIX */
#include <unistd.h> /* lots */
#include <sys/stat.h> /* stat() and fstat() */
#endif

/**********************************************************************
 ENUMS and TYPES
 *********************************************************************/

/* main_file->mode values */
typedef enum
{
  XO_READ  = 0,
  XO_WRITE = 1,
} main_file_modes;

typedef struct _main_file        main_file;

/* The main_file object supports abstract system calls like open,
 * close, read, write, seek, stat.  The program uses these to
 * represent both seekable files and non-seekable files.  Source files
 * must be seekable, but the target input and any output file do not
 * require seekability.
 */
struct _main_file
{
#if XD3_STDIO
  FILE               *file;
#elif XD3_POSIX
  int                 file;
#elif XD3_WIN32
  HANDLE              file;
#elif XD3_AMIGA
  BPTR                file;
#endif

  int                 mode;          /* XO_READ and XO_WRITE */
  const char         *filename;      /* File name. */
  xoff_t              nread;         /* for input position */
  xoff_t              nwrite;        /* for output position */
  xoff_t              source_position;  /* for avoiding seek in getblk_func */
  int                 seek_failed;   /* after seek fails once, try FIFO */
};

#include "xdelta3-mainopt.h"

#define DEFAULT_VERBOSE 0

static xd3_options_t default_options =
{
	XD3_DEFAULT_IOPT_SIZE,	/* iopt_size */
	XD3_DEFAULT_WINSIZE,	/* winsize */
	XD3_DEFAULT_SRCWINSZ,	/* srcwinsz */
	XD3_DEFAULT_SPREVSZ,	/* sprevsz */

	1,			/* force overwrite */ /* was 0. */
	DEFAULT_VERBOSE,	/* verbose */
	1,			/* use_checksum */

	NULL,			/* progress data */
	NULL,			/* debug_print () */
	NULL			/* progress_log() */
};
static xd3_options_t *use_options = &default_options;

/* Static variables */
static uint8_t*        main_bdata = NULL;
static usize_t         main_bsize = 0;

static int main_input (main_file *ifile,
                       main_file *ofile, main_file *sfile);

static int main_getblk_func (xd3_stream *stream,
			     xd3_source *source,
			     xoff_t      blkno);
static void main_free (void *ptr);
static void* main_malloc (usize_t size);

static int main_file_open (main_file *xfile, const char* name, int mode);
static int main_file_stat (main_file *xfile, xoff_t *size);
static int main_file_seek (main_file *xfile, xoff_t pos);
static int main_read_primary_input (main_file   *file,
				    uint8_t     *buf,
				    usize_t      size,
				    usize_t     *nread);

/* The code in xdelta3-blk.h is essentially part of this unit, see
 * comments there. */
#include "xdelta3-blkcache.h"

static void
reset_defaults(void)
{
  use_options = &default_options;

  main_bdata = NULL;
  main_bsize = 0;

  main_lru_reset();
}

static void*
main_malloc1 (usize_t size)
{
  void* r = malloc (size);
  if (r == NULL && use_options->debug_print)
      use_options->debug_print("malloc: %s\n", xd3_mainerror (ENOMEM));
  return r;
}

static void*
main_malloc (usize_t size)
{
  void *r = main_malloc1 (size);
  return r;
}

static void*
main_alloc (void   *opaque,
	    usize_t  items,
	    usize_t  size)
{
  return main_malloc1 (items * size);
}

static void
main_free1 (void *opaque, void *ptr)
{
  free (ptr);
}

static void
main_free (void *ptr)
{
  if (ptr)
    {
      main_free1 (NULL, ptr);
    }
}

/* This ensures that (ret = errno) always indicates failure, in case errno was
 * accidentally not set.  If this prints there's a bug somewhere. */
static int
get_errno (void)
{
#if defined(_WIN32)
  DWORD err_num = GetLastError();
  if (err_num == NO_ERROR)
    {
      err_num = XD3_INTERNAL;
    }
  return err_num;
#elif (XD3_AMIGA)
  switch (IoErr()) {
  case ERROR_OBJECT_NOT_FOUND:
    return ENOENT;
  case ERROR_DISK_FULL:
   return ENOSPC;
  default:
   return EIO; /* better ?? */
  }
#else
  if (errno == 0)
    {
      if (use_options->debug_print)
	use_options->debug_print("BUG: expected errno != 0\n");
      errno = XD3_INTERNAL;
    }
  return errno;
#endif
}

const char*
xd3_mainerror(int err_num) {
#if !(defined(_WIN32) || defined(_WIN64))
	const char* x = xd3_strerror (err_num);
	if (x != NULL)
	  {
	    return x;
	  }
	return strerror(err_num);
#else
	static char err_buf[256];
	const char* x = xd3_strerror (err_num);
	if (x != NULL)
	  {
	    return x;
	  }
	memset (err_buf, 0, 256);
	FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM |
		       FORMAT_MESSAGE_IGNORE_INSERTS,
		       NULL, err_num,
		       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		       err_buf, 256, NULL);
	if (err_buf[0] != 0 && err_buf[strlen(err_buf) - 1] == '\n')
	  {
	    err_buf[strlen(err_buf) - 1] = 0;
	  }
	return err_buf;
#endif
}

/******************************************************************
 FILE BASICS
 ******************************************************************/

/* With all the variation in file system-call semantics, arguments,
 * return values and error-handling for the POSIX and STDIO file APIs,
 * the insides of these functions make me sick, which is why these
 * wrappers exist. */

/* dos/windows needs opening files in binary mode */
#if !defined(O_BINARY)
# if defined(_O_BINARY)
#  define O_BINARY _O_BINARY
# else
#  define O_BINARY	0
# endif
#endif

#define XOPEN_OPNAME (xfile->mode == XO_READ ? "read" : "write")
#define XOPEN_STDIO  (xfile->mode == XO_READ ? "rb" : "wb")
#define XOPEN_POSIX  (xfile->mode == XO_READ ? (O_RDONLY | O_BINARY)  :  \
				(O_WRONLY | O_CREAT | O_TRUNC | O_BINARY))
#define XOPEN_MODE   (xfile->mode == XO_READ ? 0 : 0666)
#define XOPEN_AMIGA  (xfile->mode == XO_READ ? MODE_OLDFILE : MODE_NEWFILE)

#define XF_ERROR(op, name, ret) \
  do {									\
	if (use_options->debug_print != NULL) {				\
	   use_options->debug_print("file %s failed: %s: %s: %s\n", (op),	\
			  XOPEN_OPNAME, (name), xd3_mainerror (ret));	\
	}								\
  } while (0)

#if XD3_STDIO
#define XFNO(f) fileno(f->file)
#elif XD3_POSIX
#define XFNO(f) f->file
#elif XD3_WIN32
#define XFNO(f) -1
#elif XD3_AMIGA
#define XFNO(f) -1
#endif

static void
main_file_init (main_file *xfile)
{
  memset (xfile, 0, sizeof (*xfile));

#if XD3_POSIX
  xfile->file = -1;
#endif
#if XD3_WIN32
  xfile->file = INVALID_HANDLE_VALUE;
#endif
}

static int
main_file_isopen (main_file *xfile)
{
#if XD3_STDIO
  return xfile->file != NULL;

#elif XD3_POSIX
  return xfile->file != -1;

#elif XD3_WIN32
  return xfile->file != INVALID_HANDLE_VALUE;

#elif XD3_AMIGA
  return xfile->file != 0;
#endif
}

static int
main_file_close (main_file *xfile)
{
  int ret = 0;

  if (! main_file_isopen (xfile))
    {
      return 0;
    }

#if XD3_STDIO
  ret = fclose (xfile->file);
  xfile->file = NULL;

#elif XD3_POSIX
  ret = close (xfile->file);
  xfile->file = -1;

#elif XD3_WIN32
  if (!CloseHandle(xfile->file)) {
    ret = get_errno ();
  }
  xfile->file = INVALID_HANDLE_VALUE;

#elif XD3_AMIGA
  ret = !Close (xfile->file);
  xfile->file = 0;
  if (ret) ret = get_errno ();
#endif

  if (ret != 0) { XF_ERROR ("close", xfile->filename, ret = get_errno ()); }
  return ret;
}

static void
main_file_cleanup (main_file *xfile)
{
  XD3_ASSERT (xfile != NULL);

  if (main_file_isopen (xfile))
    {
      main_file_close (xfile);
    }
}

static int
main_file_open (main_file *xfile, const char* name, int mode)
{
  int ret = 0;

  xfile->mode = mode;

  XD3_ASSERT (name != NULL);
  XD3_ASSERT (! main_file_isopen (xfile));
  if (name[0] == 0)
    {
      if (use_options->debug_print)
	  use_options->debug_print("invalid file name: empty string\n");
      return XD3_INVALID;
    }

#if XD3_STDIO
  xfile->file = fopen (name, XOPEN_STDIO);

  ret = (xfile->file == NULL) ? get_errno () : 0;

#elif XD3_POSIX
  if ((ret = open (name, XOPEN_POSIX, XOPEN_MODE)) == -1)
    {
      ret = get_errno ();
    }
  else
    {
      xfile->file = ret;
      ret = 0;
    }

#elif XD3_WIN32
  xfile->file = CreateFile(name,
			   (mode == XO_READ) ? GENERIC_READ : GENERIC_WRITE,
			   FILE_SHARE_READ,
			   NULL,
			   (mode == XO_READ) ?
			   OPEN_EXISTING :
			   (use_options->overwrite ? CREATE_ALWAYS : CREATE_NEW),
			   FILE_ATTRIBUTE_NORMAL,
			   NULL);
  if (xfile->file == INVALID_HANDLE_VALUE)
    {
      ret = get_errno ();
    }

#elif XD3_AMIGA
  xfile->file = Open((const STRPTR) name, XOPEN_AMIGA);
  ret = (xfile->file == 0) ? get_errno () : 0;
#endif
  if (ret) { XF_ERROR ("open", name, ret); }
  else     { xfile->nread = 0; }
  return ret;
}

static int
main_file_stat (main_file *xfile, xoff_t *size)
{
#if XD3_WIN32
  if (GetFileType(xfile->file) != FILE_TYPE_DISK)
    {
      return -1;
    }
# if (_WIN32_WINNT >= 0x0500)
  {
    LARGE_INTEGER li;
    if (GetFileSizeEx(xfile->file, &li) == 0)
      {
	return get_errno ();
      }
    *size = li.QuadPart;
  }
# else
  {
    DWORD filesize = GetFileSize(xfile->file, NULL);
    DWORD err_num;
    if (filesize == INVALID_FILE_SIZE)
      {
	err_num = GetLastError();
	if (err_num != NO_ERROR)
	  {
	    return err_num;
	  }
      }
    *size = filesize;
  }
# endif
#elif defined(__DJGPP__)
  long filesize = filelength (XFNO (xfile));
  if (filesize == -1)
    {
      return get_errno ();
    }
  *size = filesize;
#elif (XD3_AMIGA)
  struct FileInfoBlock *fib = (struct FileInfoBlock*)
				  AllocDosObject(DOS_FIB, NULL);
  long filesize = -1;
  if (fib != NULL)
    {
      if (ExamineFH(xfile->file, fib))
        filesize = fib->fib_Size;
      FreeDosObject(DOS_FIB, fib);
    }
  if (filesize < 0)
    return get_errno ();
  *size = filesize;
#else
  struct stat sbuf;
  if (fstat (XFNO (xfile), & sbuf) == -1)
    {
      return get_errno ();
    }

  if (! S_ISREG (sbuf.st_mode))
    {
      return ESPIPE;
    }
  (*size) = sbuf.st_size;
#endif
  return 0;
}

#if (XD3_POSIX)
/* POSIX-generic code takes a function pointer to read() or write().
 * This calls the function repeatedly until the buffer is full or EOF.
 * The NREAD parameter is not set for write, NULL is passed.  Return
 * is signed, < 0 indicate errors, otherwise byte count. */
typedef int (xd3_posix_func) (int fd, uint8_t *buf, usize_t size);

static int
xd3_posix_io (int fd, uint8_t *buf, usize_t size,
	      xd3_posix_func *func, usize_t *nread)
{
  int ret;
  usize_t nproc = 0;

  while (nproc < size)
    {
      int result = (*func) (fd, buf + nproc, size - nproc);

      if (result < 0)
	{
	  ret = get_errno ();
	  if (ret != EAGAIN && ret != EINTR)
	    {
	      return ret;
	    }
	  continue;
	}

      if (nread != NULL && result == 0) { break; }

      nproc += result;
    }
  if (nread != NULL) { (*nread) = nproc; }
  return 0;
}
#endif

#if (XD3_AMIGA)
/* Read() and Write() are unbuffered.  This calls Read() or Write()
 * repeatedly until the buffer is full or EOF.
 * The NREAD parameter is not set for write, NULL is passed.  Return
 * is signed, < 0 indicate errors, otherwise byte count. */
static int
xd3_amiga_io (BPTR fd, uint8_t *buf, usize_t size,
	      int is_read, usize_t *nread)
{
  usize_t nproc = 0;

  while (nproc < size)
    {
      LONG result = (is_read)?
		    Read(fd, buf + nproc, size - nproc) :
		    Write(fd, buf + nproc, size - nproc);

      if (result < 0) return get_errno ();

      if (nread != NULL && result == 0) break;

      nproc += result;
    }
  if (nread != NULL) { (*nread) = nproc; }
  return 0;
}
#endif

#if XD3_WIN32
static int
xd3_win32_io (HANDLE file, uint8_t *buf, usize_t size,
	      int is_read, usize_t *nread)
{
  int ret = 0;
  usize_t nproc = 0;

  while (nproc < size)
    {
      DWORD nproc2;
      if ((is_read ?
	   ReadFile (file, buf + nproc, size - nproc, &nproc2, NULL) :
	   WriteFile (file, buf + nproc, size - nproc, &nproc2, NULL)) == 0)
	{
	  ret = get_errno();
	  if (ret != ERROR_HANDLE_EOF && ret != ERROR_BROKEN_PIPE)
	    {
	      return ret;
	    }
	  /* By falling through here, we'll break this loop in the
	   * read case in case of eof or broken pipe. */
	}

      nproc += (usize_t) nproc2;

      if (nread != NULL && nproc2 == 0) { break; }
    }
  if (nread != NULL) { (*nread) = nproc; }
  return 0;
}
#endif

/* POSIX is unbuffered, while STDIO is buffered.  main_file_read()
 * should always be called on blocks. */
static int
main_file_read (main_file  *ifile,
		uint8_t    *buf,
		usize_t     size,
		usize_t    *nread,
		const char *msg)
{
  int ret = 0;

#if XD3_STDIO
  usize_t result;

  result = fread (buf, 1, size, ifile->file);

  if (result < size && ferror (ifile->file))
    {
      ret = get_errno ();
    }
  else
    {
      *nread = result;
    }

#elif XD3_POSIX
  ret = xd3_posix_io (ifile->file, buf, size, (xd3_posix_func*) &read, nread);
#elif XD3_WIN32
  ret = xd3_win32_io (ifile->file, buf, size, 1 /* is_read */, nread);
#elif XD3_AMIGA
  ret = xd3_amiga_io (ifile->file, buf, size, 1 /* is_read */, nread);
#endif

  if (ret)
    {
      if (use_options->debug_print)
	  use_options->debug_print("%s: %s: %s\n", msg, ifile->filename, xd3_mainerror (ret));
    }
  else
    {
      ifile->nread += (*nread);
    }

  return ret;
}

static int
main_file_write (main_file *ofile, uint8_t *buf, usize_t size, const char *msg)
{
  int ret = 0;

#if XD3_STDIO
  usize_t result;

  result = fwrite (buf, 1, size, ofile->file);

  if (result != size) { ret = get_errno (); }

#elif XD3_POSIX
  ret = xd3_posix_io (ofile->file, buf, size, (xd3_posix_func*) &write, NULL);

#elif XD3_WIN32
  ret = xd3_win32_io (ofile->file, buf, size, 0, NULL);

#elif XD3_AMIGA
  ret = xd3_amiga_io (ofile->file, buf, size, 0, NULL);

#endif

  if (ret)
    {
      if (use_options->debug_print)
	  use_options->debug_print("%s: %s: %s\n", msg, ofile->filename, xd3_mainerror (ret));
    }
  else
    {
      ofile->nwrite += size;
      if (use_options->progress_data != NULL)
	{
	  use_options->progress_data->current_file_written += size;
	  use_options->progress_data->current_written += size;
	  if (use_options->progress_log)
	    use_options->progress_log ();
	}
    }

  return ret;
}

static int
main_file_seek (main_file *xfile, xoff_t pos)
{
  int ret = 0;

#if XD3_STDIO
  if (fseek (xfile->file, pos, SEEK_SET) != 0) { ret = get_errno (); }

#elif XD3_POSIX
  if ((xoff_t) lseek (xfile->file, pos, SEEK_SET) != pos)
    { ret = get_errno (); }

#elif XD3_WIN32
# if (_WIN32_WINNT >= 0x0500)
  LARGE_INTEGER move, out;
  move.QuadPart = pos;
  if (SetFilePointerEx(xfile->file, move, &out, FILE_BEGIN) == 0)
    {
      ret = get_errno ();
    }
# else
  if (SetFilePointer(xfile->file, (LONG)pos, NULL, FILE_BEGIN) ==
      INVALID_SET_FILE_POINTER)
    {
      ret = get_errno ();
    }
# endif

#elif XD3_AMIGA
  if (Seek(xfile->file, pos, OFFSET_BEGINNING) < 0)
    ret = get_errno ();
#endif

  return ret;
}

/* This function simply writes the stream output buffer, if there is
 * any, for encode, decode and recode commands.  (The VCDIFF tools use
 * main_print_func()). */
static int
main_write_output (xd3_stream* stream, main_file *ofile)
{
  int ret;

  if (stream->avail_out > 0 &&
      (ret = main_file_write (ofile, stream->next_out,
			      stream->avail_out, "write failed")))
    {
      return ret;
    }

  return 0;
}

/*********************************************************************
 Main I/O routines
 **********************************************************************/

/* This function acts like the above except it may also try to
 * recognize a compressed input (source or target) when the first
 * buffer of data is read.  The EXTERNAL_COMPRESSION code is called to
 * search for magic numbers. */
static int
main_read_primary_input (main_file   *file,
			 uint8_t     *buf,
			 usize_t      size,
			 usize_t     *nread)
{
  return main_file_read (file, buf, size, nread, "input read failed");
}

/* Open the main output file, sets a default file name, initiate
 * recompression.  This function is expected to fprint any error
 * messages. */
static int
main_open_output (xd3_stream *stream, main_file *ofile)
{
  int ret;

      if ((ret = main_file_open (ofile, ofile->filename, XO_WRITE)))
	{
	  return ret;
	}

      if (use_options->verbose && use_options->debug_print)
	{
	  use_options->debug_print("output %s\n", ofile->filename);
	}

  return 0;
}

static usize_t
main_get_winsize (main_file *ifile) {
  xoff_t file_size = 0;
  usize_t size = use_options->winsize;

  if (main_file_stat (ifile, &file_size) == 0)
    {
      size = (usize_t) xd3_min(file_size, (xoff_t) size);
    }

  size = xd3_max(size, XD3_ALLOCSIZE);

  if (use_options->verbose && use_options->debug_print)
    {
      use_options->debug_print("input %s window size %u bytes\n",
	  ifile->filename, size);
    }

  return size;
}

/*********************************************************************
 Main routines
 ********************************************************************/

/* This is a generic input function.  It calls the xd3_encode_input or
 * xd3_decode_input functions and makes calls to the various input
 * handling routines above, which coordinate external decompression.
 */
static int
main_input (main_file   *ifile,
	    main_file   *ofile,
	    main_file   *sfile)
{
  int        ret;
  xd3_stream stream;
  usize_t    nread = 0;
  usize_t    winsize;
  int        stream_flags = 0;
  xd3_config config;
  xd3_source source;

  memset (& stream, 0, sizeof (stream));
  memset (& source, 0, sizeof (source));
  memset (& config, 0, sizeof (config));

  config.alloc = main_alloc;
  config.freef = main_free1;

  config.iopt_size = use_options->iopt_size;
  config.sprevsz = use_options->sprevsz;

  if (use_options->use_checksum == 0)
    {
      stream_flags |= XD3_ADLER32_NOVER;
    }

  main_bsize = winsize = main_get_winsize (ifile);
  if ((main_bdata = (uint8_t*) main_malloc (winsize)) == NULL)
    {
      return EXIT_FAILURE;
    }

  config.winsize = winsize;
  config.srcwin_maxsz = use_options->srcwinsz;
  config.getblk = main_getblk_func;
  config.flags = stream_flags;

  if ((ret = xd3_config_stream (& stream, & config)))
    {
      if (use_options->debug_print)
	  use_options->debug_print(XD3_LIB_ERRMSG (& stream, ret));
      return EXIT_FAILURE;
    }

  /* Main input loop. */
  do
    {
      xoff_t input_offset;
      xoff_t input_remain;
      usize_t try_read;

      input_offset = ifile->nread;

      input_remain = XOFF_T_MAX - input_offset;

      try_read = (usize_t) xd3_min ((xoff_t) config.winsize, input_remain);

      if ((ret = main_read_primary_input (ifile, main_bdata,
					  try_read, & nread)))
	{
	  return EXIT_FAILURE;
	}

      /* If we've reached EOF tell the stream to flush. */
      if (nread < try_read)
	{
	  stream.flags |= XD3_FLUSH;
	}

      xd3_avail_input (& stream, main_bdata, nread);

      /* If we read zero bytes after encoding at least one window... */
      if (nread == 0 && stream.current_window > 0) {
	break;
      }

    again:
      ret = xd3_decode_input (& stream);

      switch (ret)
	{
	case XD3_INPUT:
	  continue;

	case XD3_GOTHEADER:
	  {
	    XD3_ASSERT (stream.current_window == 0);

	    /* Need to process the appheader as soon as possible.  It may
	     * contain a suggested default filename/decompression routine for
	     * the ofile, and it may contain default/decompression routine for
	     * the sources. */
		/* Now open the source file. */
		  if ((sfile->filename != NULL) &&
		      (ret = main_set_source (& stream, sfile, & source)))
		  {
		    return EXIT_FAILURE;
		  }
	  }
	/* FALLTHROUGH */
	case XD3_WINSTART:
	  {
	    /* e.g., set or unset XD3_SKIP_WINDOW. */
	    goto again;
	  }

	case XD3_OUTPUT:
	  {
	    /* Defer opening the output file until the stream produces its
	     * first output for both encoder and decoder, this way we
	     * delay long enough for the decoder to receive the
	     * application header.  (Or longer if there are skipped
	     * windows, but I can't think of any reason not to delay
	     * open.) */
	    if (ofile != NULL &&
		! main_file_isopen (ofile) &&
		(ret = main_open_output (& stream, ofile)) != 0)
	      {
		return EXIT_FAILURE;
	      }

	    if ((ret = main_write_output(& stream, ofile)) &&
		(ret != PRINTHDR_SPECIAL))
	      {
		return EXIT_FAILURE;
	      }

	    if (ret == PRINTHDR_SPECIAL)
	      {
		xd3_abort_stream (& stream);
		ret = EXIT_SUCCESS;
		goto done;
	      }

	    ret = 0;

	    xd3_consume_output (& stream);
	    goto again;
	  }

	case XD3_WINFINISH:
	  {
	    goto again;
	  }

	default:
	  /* xd3_decode_input() error */
	  if (use_options->debug_print)
	      use_options->debug_print(XD3_LIB_ERRMSG (& stream, ret));
	  return EXIT_FAILURE;
	}
    }
  while (nread == config.winsize);
done:
  /* Close the inputs. (ifile must be open, sfile may be open) */
  main_file_close (ifile);
  if (sfile != NULL)
    {
      main_file_close (sfile);
    }

  /* If output file is not open yet because of delayed-open, it means
   * we never encountered a window in the delta, but it could have had
   * a VCDIFF header?  TODO: solve this elsewhere.  For now, it prints
   * "nothing to output" below, but the check doesn't happen in case
   * of option_no_output.  */
  if (ofile != NULL)
    {
      if (! main_file_isopen (ofile))
	{
	  if (use_options->debug_print)
	      use_options->debug_print("nothing to output: %s\n", ifile->filename);
	  return EXIT_FAILURE;
	}

      /* Have to close the output before calling
       * main_external_compression_finish, or else it hangs. */
      if (main_file_close (ofile) != 0)
	{
	  return EXIT_FAILURE;
	}
    }

  if ((ret = xd3_close_stream (& stream)))
    {
      if (use_options->debug_print)
	  use_options->debug_print(XD3_LIB_ERRMSG (& stream, ret));
      return EXIT_FAILURE;
    }

  xd3_free_stream (& stream);

  return EXIT_SUCCESS;
}

/* free memory before exit, reset single-use variables. */
static void
main_cleanup (void)
{
  main_free (main_bdata);
  main_bdata = NULL;
  main_bsize = 0;

  main_lru_cleanup();

  XD3_ASSERT (main_mallocs == 0);
}

int
xd3_main_patcher (xd3_options_t *options,
                  const char *srcfile,
                  const char *deltafile,
                  const char *outfile)
{
  main_file ifile;
  main_file ofile;
  main_file sfile;
  int ret;

  XD3_ASSERT (srcfile != NULL);
  XD3_ASSERT (deltafile != NULL);
  XD3_ASSERT (outfile != NULL);

  main_file_init (& ifile);
  main_file_init (& ofile);
  main_file_init (& sfile);

  reset_defaults();
  if (options != NULL)
    use_options = options;

  sfile.filename = srcfile;
  ofile.filename = outfile;
  ifile.filename = deltafile;

      if ((ret = main_file_open (& ifile, ifile.filename, XO_READ)))
	{
	  ret = EXIT_FAILURE;
	  goto cleanup;
	}

  ret = main_input (& ifile, & ofile, & sfile);

 cleanup:
  main_file_cleanup (& ifile);
  main_file_cleanup (& ofile);
  main_file_cleanup (& sfile);

  main_cleanup ();

  fflush (stdout);
  fflush (stderr);
  return ret;
}

#define READ_BUFSIZE 8192 /* BUFSIZ */
unsigned long
xd3_calc_adler32 (const char *srcfile)
{
  main_file ifile;
  unsigned long sum;
  uint8_t buf[READ_BUFSIZE];
  usize_t nread;

  XD3_ASSERT (srcfile != NULL);

  main_file_init (& ifile);
  ifile.filename = srcfile;
  if (main_file_open (& ifile, ifile.filename, XO_READ) != 0)
    return 1UL;

  nread = READ_BUFSIZE;
  sum = 1UL;
  while (nread == READ_BUFSIZE) /* otherwise:  short read == EOF */
    {
      if (main_read_primary_input (&ifile, buf, READ_BUFSIZE, & nread)
	  != 0)
	{
	  sum = 1UL;
	  break;
	}

      if (nread != 0)
	sum = adler32 (sum, buf, nread);
    }

  main_file_cleanup (& ifile);
  return sum;
}
