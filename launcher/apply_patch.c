/*
 * apply_patch.c
 * hexen2 launcher: binary patch starter
 *
 * $Id$
 *
 * Copyright (C) 2005-2013  O.Sezer <sezero@users.sourceforge.net>
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

#include "apply_patch.h"
#include "launcher_ui.h"

struct other_pak
{
	long			size;
	unsigned long		sum;	/* adler32 */
	const char		*desc;
	struct other_pak const	*next;
};

static const struct other_pak pak0_oem1 = {
	22720659,	0xE9D25D16,
	"Continent of Blackmarsh (m3D, v1.10)",
	NULL
};

static const struct other_pak pak0_oem0 = {
	/* don't have this myself, therefore no patch. */
	22719295,	/**/ ~0UL,
	"Continent of Blackmarsh (m3D, v1.08)",
	&pak0_oem1
};

static const struct other_pak pak0_demo1 = {
	27750257,	0xED96172E,
	"Demo (Nov. 1997, v1.11)",
	&pak0_oem0
};

static const struct other_pak pak0_demo0 = {
	23537707,	0x88A46443,
	"Demo (Aug. 1997, v0.42)",
	&pak0_demo1
};

#if 0
static const struct other_pak pak2_oem1 = {
	17742721,	0x5595110E,
	"Continent of Blackmarsh (m3D, v1.10)",
	NULL
};

static const struct other_pak pak2_oem0 = {
	/* don't have this myself, therefore no patch. */
	17739969,	/**/ ~0UL,
	"Continent of Blackmarsh (m3D, v1.08)",
	&pak2_oem1
};
#endif

#define NUM_PATCHES	2

struct patch_pak
{
	const char	*dir_name;	/* where the file is	*/
	const char	*filename;	/* file to patch	*/
	const char	*deltaname;	/* delta file to use	*/
	const char	*old_desc;
	const char	*new_desc;

	struct other_pak const	*other_data;
			/* possible descriptions of same-named pak
			 * versions not supported by this program. */

	unsigned long	old_sum, new_sum;	/* adler32	*/
	long	old_size, new_size;
};

static const struct patch_pak patch_data[NUM_PATCHES] =
{
	{  "data1", "pak0.pak",
	   "data1pk0.xd3",
	   "retail, from Hexen II cdrom (v1.03)",
	   "retail, already patched (v1.11)",
	   &pak0_demo0,
	   0xCFF397B9, 0xDCF2218F,
	   21714275, 22704056
	},
	{  "data1", "pak1.pak",
	   "data1pk1.xd3",
	   "retail, from Hexen II cdrom (v1.03)",
	   "retail, already patched (v1.11)",
	   NULL,
	   0x8C787960, 0xD56A2FE8,
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

static const char *request_m3d_feedback (const char *desc)
{
	static const char msg[] = "Please report this pak to the uHexen2 developers!";
	static char txt[256];
	snprintf (txt, sizeof(txt), "%s\n... %s", desc, msg);
	return txt;
}

static const char *other_pak_desc (int num, long len)
{
	const struct other_pak *p = patch_data[num].other_data;

	for ( ; p != NULL; p = p->next)
	{
		if (len == p->size)
		{
			if (p->sum == ~0UL) /* v1.08: wanna hear it */
				return request_m3d_feedback (p->desc);
			return p->desc;
		}
	}

	return "an unknown pak file";
}

void *apply_patches (void *workdir)
{
	int	i, ret;
	struct stat	stbuf;
	unsigned long	csum;

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

		/* paks copied off of a cdrom might fail R_OK|W_OK */
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

		if (stbuf.st_size == patch_data[i].new_size)
		{
			ui_log_queue ("... looks like %s\n", patch_data[i].new_desc);
			ui_log_queue ("... checksumming...");
			csum = xd3_calc_adler32(dst);
			if (csum == patch_data[i].new_sum)
				ui_log_queue (" OK ");
			else	ui_log_queue ("\n... WARNING: checksum mismatch! file corrupted?\n");
			h2patch_progress.current_written += patch_data[i].new_size;
			ui_log_queue ("... skipped.\n");
			continue;
		}
		if (stbuf.st_size != patch_data[i].old_size)
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... looks like %s\n", other_pak_desc(i, stbuf.st_size));
			ui_log_queue ("... not supported by h2patch!\n");
			thread_alive = 0;
			return &rc;
		}

		ui_log_queue ("... looks like %s\n", patch_data[i].old_desc);

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
			ui_log_queue ("... patch failed! file corrupted?\n");
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

