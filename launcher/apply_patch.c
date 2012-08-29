/*
 * apply_patch.c
 * hexen2 launcher: binary patch starter
 *
 * $Id$
 *
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "common.h"
#include <sys/time.h>

#include "md5.h"
#include "apply_patch.h"
#include "launcher_ui.h"

struct other_pak
{
	long			size;
	const char		*desc;
	struct other_pak const	*next;
};

static const struct other_pak pak0_oem1 = {
	22720659, "Continent of Blackmarsh (m3D, v1.10)",
	NULL
};

static const struct other_pak pak0_oem0 = {
	/* don't have this myself, therefore no patch. */
	22719295, "Continent of Blackmarsh (m3D, v1.08)",
	&pak0_oem1
};

static const struct other_pak pak0_demo1 = {
	27750257, "Demo (Nov. 1997, v1.11)",
	&pak0_oem0
};

static const struct other_pak pak0_demo0 = {
	23537707, "Demo (Aug. 1997, v1.03)",
	&pak0_demo1
};

#if 0
static const struct other_pak pak2_oem1 = {
	17742721, "Continent of Blackmarsh (m3D, v1.10)",
	NULL
};

static const struct other_pak pak2_oem0 = {
	/* don't have this myself, therefore no patch. */
	17739969, "Continent of Blackmarsh (m3D, v1.08)",
	&pak2_oem1
};
#endif

#define NUM_PATCHES	2

struct patch_pak
{
	const char	*dir_name;	/* where the file is	*/
	const char	*filename;	/* file to patch	*/
	const char	*deltaname;	/* delta file to use	*/
	const char	*old_md5;	/* unpatched md5sum	*/
	const char	*new_md5;	/* md5sum after patch	*/
	const char	*old_desc;
	const char	*new_desc;
	struct other_pak const	*other_data;
			/* possible descriptions of same-named pak
			 * versions not supported by this program. */
	long	old_size, new_size;
};

static const struct patch_pak patch_data[NUM_PATCHES] =
{
	{  "data1", "pak0.pak",
	   "data1pk0.xd3",
	   "b53c9391d16134cb3baddc1085f18683",
	   "c9675191e75dd25a3b9ed81ee7e05eff",
	   "retail, from Hexen II cdrom (v1.03)",
	   "retail, already patched (v1.11)",
	   &pak0_demo0,
	   21714275, 22704056
	},
	{  "data1", "pak1.pak",
	   "data1pk1.xd3",
	   "9a2010aafb9c0fe71c37d01292030270",
	   "c2ac5b0640773eed9ebe1cda2eca2ad0",
	   "retail, from Hexen II cdrom (v1.03)",
	   "retail, already patched (v1.11)",
	   NULL,
	   76958474, 75601170
	}
};

#define	H2PATCH_SRCWINSZ	(1<<23)	/* 8 MB is enough */

static xd3_options_t h2patch_options =
{
	XD3_DEFAULT_IOPT_SIZE,	/* iopt_size */
	XD3_DEFAULT_WINSIZE,	/* winsize */
	H2PATCH_SRCWINSZ,	/* srcwinsz */
	XD3_DEFAULT_SPREVSZ,	/* sprevsz */

	1,			/* force overwrite */ /* was 0. */
	0,			/* verbose */
	1,			/* use_checksum */

	&h2patch_progress,	/* progress_data */
	ui_log_queue,		/* debug_print() */
	NULL			/* progress_log() */
};

int			thread_alive;

/* gui progress bar support: */
xd3_progress_t		h2patch_progress;

static	unsigned long		rc;
static	char	dst[MAX_OSPATH],
		pat[MAX_OSPATH],
		out[MAX_OSPATH];
static	char	csum[CHECKSUM_SIZE+1];

#define DELTA_DIR	"patchdat"
#define cdrom_path	"install/hexen2/data1"
#define patch_tmpname	"uh2patch.tmp"

static int stat_and_fix_perms (const char *name, struct stat *s)
{
	chmod (name, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (stat(name, s) != 0)
		return -1;
	if (!S_ISREG(s->st_mode))
		return 1;
	return 0;
}

static long get_millisecs (void)
{
	struct timeval tv;

	gettimeofday (&tv, NULL);

	return (tv.tv_sec) * 1000L + (tv.tv_usec) / 1000;
}

static long starttime;

static void start_file_progress (long bytes)
{
	h2patch_progress.current_file_written = 0;
	h2patch_progress.current_file_total = bytes;
	starttime = get_millisecs ();
}

static void finish_file_progress (void)
{
	if (h2patch_progress.current_file_written != 0)
	{
		long elapsed = get_millisecs () - starttime;
		if (elapsed < 10000)
		{
			ui_log_queue ("... elapsed time %.2fs\n",
						elapsed / 1000.0);
		}
		else
		{
			elapsed /= 1000;
			ui_log_queue ("... elapsed time %lum:%02lus\n",
					elapsed / 60, elapsed % 60);
		}
	}
}

static const char *other_pak_desc (int num, long len)
{
	const struct other_pak *p = patch_data[num].other_data;

	for ( ; p != NULL; p = p->next)
	{
		if (len == p->size)
			return p->desc;
	}

	return "an unknown pak file";
}

void *apply_patches (void *workdir)
{
	int	i, ret;
	struct stat	stbuf;

	rc = XPATCH_NONE;
	memset (&h2patch_progress, 0, sizeof(xd3_progress_t));

	ui_log_queue ("Workdir: %s\n", (char *)workdir);

	for (i = 0; i < NUM_PATCHES; i++)
	{
		h2patch_progress.total_bytes += patch_data[i].new_size;
		/* delete our temp files from possible previous runs */
		snprintf (out, sizeof(out), "%s/%s/%s", (char *)workdir,
						patch_data[i].dir_name,
							 patch_tmpname);
		remove (out);
	}

	for (i = 0; i < NUM_PATCHES; i++)
	{
		ui_log_queue ("File %s/%s :\n", patch_data[i].dir_name,
						patch_data[i].filename);
		snprintf (dst, sizeof(dst), "%s/%s/%s", (char *)workdir,
						patch_data[i].dir_name,
						patch_data[i].filename);

		/* set the pak files' read+write permissions if we can:
		 * if the files were copied from the cdrom, some perms
		 * may be missing and access() would fail the R_OK|W_OK
		 * check. */
		ret = stat_and_fix_perms(dst, &stbuf);
		if (ret != 0)
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... cannot find!\n");
			thread_alive = 0;
			return &rc;
		}
		if (access(dst, R_OK|W_OK) != 0)
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... cannot access, check permissions!\n");
			thread_alive = 0;
			return &rc;
		}

		if (stbuf.st_size == patch_data[i].old_size)
		{
			ui_log_queue ("... looks like %s\n", patch_data[i].old_desc);
		}
		else if (stbuf.st_size == patch_data[i].new_size)
		{
			ui_log_queue ("... looks like %s\n", patch_data[i].new_desc);
		}
		else
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... looks like %s\n", other_pak_desc(i, stbuf.st_size));
			ui_log_queue ("... not supported by h2patch!\n");
			thread_alive = 0;
			return &rc;
		}

		ui_log_queue ("... checksumming...\n");
		memset (csum, 0, sizeof(csum));
		md5_compute(dst, csum);
		if (strcmp(csum, patch_data[i].new_md5) == 0)
		{
			h2patch_progress.current_written += patch_data[i].new_size;
			ui_log_queue ("... OK: already patched.\n");
			continue;
		}
		if (strcmp(csum, patch_data[i].old_md5) != 0)
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... file probably corrupted!\n");
			thread_alive = 0;
			return &rc;
		}

		snprintf (pat, sizeof(pat), "%s/%s/%s/%s", (char *)workdir,
					DELTA_DIR, patch_data[i].dir_name,
						   patch_data[i].deltaname);
		if (access(pat, R_OK) != 0)
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... delta file not found!\n");
			thread_alive = 0;
			return &rc;
		}

		snprintf (out, sizeof(out), "%s/%s/%s", (char *)workdir,
						patch_data[i].dir_name,
							 patch_tmpname);
		ui_log_queue ("... applying patch...\n");
		start_file_progress (patch_data[i].new_size);
		ret = xd3_main_patcher(&h2patch_options, dst, pat, out);
		finish_file_progress ();
		if (ret != 0)
		{
			rc |= XPATCH_FAIL;
			remove (out);
			ui_log_queue ("... patch failed!\n");
			thread_alive = 0;
			return &rc;
		}

		ui_log_queue ("... verifying checksum...\n");
		memset (csum, 0, sizeof(csum));
		md5_compute(out, csum);
		if (strcmp(csum, patch_data[i].new_md5) != 0)
		{
			rc |= XPATCH_FAIL;
			remove (out);
			ui_log_queue ("... checksum after patching failed!\n");
			thread_alive = 0;
			return &rc;
		}

		remove (dst);	/* not all implementations overwrite existing files */
		if (rename(out, dst) != 0)
		{
			rc |= XPATCH_FAIL;
			remove (out);
			ui_log_queue ("... failed renaming patched file!\n");
			thread_alive = 0;
			return &rc;
		}

		rc |= XPATCH_APPLIED;
		ui_log_queue ("... OK. Patch successful.\n");
	}

	if (rc & XPATCH_APPLIED)
	{
		ui_log_queue ("All patches successful.\n");
	}

	thread_alive = 0;
	return &rc;
}

