/*
	apply_patch.c
	hexen2 launcher: binary patch starter

	$Id: apply_patch.c,v 1.5 2007-06-04 17:20:11 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "md5.h"
#include "loki_xdelta.h"
#include "apply_patch.h"

extern void Log_printf (const char *fmt, ...) __attribute__((format(printf,1,2)));

extern int		thread_alive;

#if defined(PATH_MAX)
#define MAX_OSPATH	PATH_MAX
#else
#define MAX_OSPATH	256
#endif

#define NUM_PATCHES	2

static const struct
{
	char	*dir_name;	/* where the file is	*/
	char	*filename;	/* file to patch	*/
	char	*deltaname;	/* delta file to use	*/
	char	*old_md5;	/* unpatched md5sum	*/
	char	*new_md5;	/* md5sum after patch	*/
} patch_data[NUM_PATCHES] =
{
	{  "data1", "pak0.pak", "data1pak0.xd",
	   "b53c9391d16134cb3baddc1085f18683" ,
	   "c9675191e75dd25a3b9ed81ee7e05eff"
	},
	{  "data1", "pak1.pak", "data1pak1.xd",
	   "9a2010aafb9c0fe71c37d01292030270" ,
	   "c2ac5b0640773eed9ebe1cda2eca2ad0"
	}
};

#define DELTA_DIR	"patchdata"

static	unsigned long		rc;

void *apply_patches (void *workdir)
{
	int			i;
	char	dst[MAX_OSPATH],
		pat[MAX_OSPATH],
		out[MAX_OSPATH];
	char	csum[CHECKSUM_SIZE+1];

	rc = XPATCH_NONE;

	for (i = 0; i < NUM_PATCHES; i++)
	{
		snprintf (dst, sizeof(dst), "%s/%s/%s", (char *)workdir, patch_data[i].dir_name, patch_data[i].filename);
		if ( access(dst, R_OK|W_OK) != 0 )
		{
			rc |= XPATCH_FAIL;
			Log_printf ("File %s not found\n", dst);
			thread_alive = 0;
			return &rc;
		}

		Log_printf ("Checksumming %s...\n", dst);
		md5_compute(dst, csum, 1);
		if ( !strcmp(csum, patch_data[i].new_md5) )
		{
			Log_printf ("File %s is already patched\n", dst);
			continue;
		}
		if ( strcmp(csum, patch_data[i].old_md5) )
		{
			rc |= XPATCH_FAIL;
			Log_printf ("File %s is an incompatible version\n", dst);
			thread_alive = 0;
			return &rc;
		}

		snprintf (pat, sizeof(pat), "%s/%s/%s/%s", (char *)workdir, DELTA_DIR, patch_data[i].dir_name, patch_data[i].deltaname);
		if ( access(pat, R_OK) != 0 )
		{
			rc |= XPATCH_FAIL;
			Log_printf ("File %s not found\n", pat);
			thread_alive = 0;
			return &rc;
		}

		snprintf (out, sizeof(out), "%s.xd1", dst);
		if ( access(out, F_OK) == 0 )
		{
			remove (out);
		}

		Log_printf ("Patching %s...\n", dst);
		if ( loki_xpatch(pat, dst, out) < 0 )
		{
			rc |= XPATCH_FAIL;
			if ( access(out, F_OK) == 0 )
			{
				remove (out);
			}
			Log_printf ("Failed patching %s\n", dst);
			thread_alive = 0;
			return &rc;
		}

		Log_printf ("Checksumming %s...\n", out);
		md5_compute(out, csum, 1);
		if ( strcmp(csum, patch_data[i].new_md5) )
		{
			rc |= XPATCH_FAIL;
			remove (out);
			Log_printf ("File %s failed checksum after patching\n", dst);
			thread_alive = 0;
			return &rc;
		}

		if ( rename(out, dst) < 0 )
		{
			rc |= XPATCH_FAIL;
			remove (out);
			Log_printf ("Failed renaming patched file to %s\n", patch_data[i].filename);
			thread_alive = 0;
			return &rc;
		}

		rc |= XPATCH_APPLIED;
		Log_printf ("Patch successful for %s\n", dst);
	}

	if (rc & XPATCH_APPLIED)
		Log_printf ("All patches successful\n");

	thread_alive = 0;
	return &rc;
}

