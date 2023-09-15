/* xdelta 3 - delta compression tools and library
 * Copyright (C) Joshua P. MacDonald
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

#ifndef XD3_IFACE_H_
#define XD3_IFACE_H_

/* interfacing with xdelta3:  */

#include "xdelta3-config.h"

#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0))
#define XD_FUNCP_PRINTF(_one,_two)	__attribute__((__format__ (__printf__,_one,_two)))
#else
#define XD_FUNCP_PRINTF(_one,_two)
#endif
typedef struct _xd3_progress_t
{
/* a progress bar can be made using the output bytes */
	unsigned long	current_file_written;
	unsigned long	current_file_total;
	unsigned long		current_written;
	unsigned long		total_bytes;
} xd3_progress_t;

typedef struct _xd3_options_t
{
	usize_t		iopt_size;
	usize_t		winsize;
	usize_t		srcwinsz;
	usize_t		sprevsz;
/* booleans: */
	int		overwrite; /* force overwrite */
	int		verbose;
	int		use_checksum;
/* progress bar data: */
	xd3_progress_t	*progress_data;
/* message printing : */
	void	(*debug_print) (const char *fmt, ...) XD_FUNCP_PRINTF(1,2);
			/* print debug messages from the xdelta side.  */
	void	(*progress_log)(void);
			/* print progress from the xdelta side. only
			   for single-threaded command-line versions.  */
} xd3_options_t;

#ifdef __cplusplus
extern "C" {
#endif

extern xd3_options_t *use_options;

extern int xd3_main_patcher (xd3_options_t * /* opts   */,
			     const char * /* srcfile   */,
			     const char * /* deltafile */,
			     const char * /* outfile   */);

extern uint32_t xd3_calc_adler32 (const char *srcfile);

#ifdef __cplusplus
}
#endif

#endif /* XD3_IFACE_H_ */
