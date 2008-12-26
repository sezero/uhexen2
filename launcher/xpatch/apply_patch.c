/*
	apply_patch.c
	hexen2 launcher: binary patch starter

	$Id: apply_patch.c,v 1.14 2008-12-26 18:38:15 sezero Exp $

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

#include "common.h"
#include <stdint.h>
#include <time.h>

#include "md5.h"
#include "loki_xdelta.h"
#include "apply_patch.h"
#include "launcher_ui.h"

#define NUM_PATCHES	2

static const struct
{
	const char	*dir_name;	/* where the file is	*/
	const char	*filename;	/* file to patch	*/
	const char	*deltaname;	/* delta file to use	*/
	const char	*old_md5;	/* unpatched md5sum	*/
	const char	*new_md5;	/* md5sum after patch	*/
	size_t	old_size, new_size;
} patch_data[NUM_PATCHES] =
{
	{  "data1", "pak0.pak",
	   "data1pak0.xd",
	   "b53c9391d16134cb3baddc1085f18683",
	   "c9675191e75dd25a3b9ed81ee7e05eff",
	   21714275, 22704056
	},
	{  "data1", "pak1.pak",
	   "data1pak1.xd",
	   "9a2010aafb9c0fe71c37d01292030270",
	   "c2ac5b0640773eed9ebe1cda2eca2ad0",
	   76958474, 75601170
	}
};

int			thread_alive;

/* gui progress bar support: */
size_t		outsize, written_size;

static	unsigned long		rc;
static	char	dst[MAX_OSPATH],
		pat[MAX_OSPATH],
		out[MAX_OSPATH];

#define DELTA_DIR	"patchdata"
#define patch_tmpname	"uh2patch.tmp"

#define NEEDED_MODES	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
static int stat_and_fix_perms (const char *name, struct stat *s)
{
	if (stat(name, s) != 0)
	{
		ui_log_queue ("... unable to stat file!\n");
		return -1;
	}
	if (!S_ISREG(s->st_mode))
	{
		ui_log_queue ("... not a regular file!\n");
		return 1;
	}
	chmod (name, NEEDED_MODES);
	return 0;
}

void *apply_patches (void *workdir)
{
	int	i;
	struct stat	stbuf;
	char		*csum;
	time_t	starttime, temptime;
	unsigned long	elapsed;

	rc = XPATCH_NONE;

	time (&starttime);
	outsize = written_size = 0;
	for (i = 0; i < NUM_PATCHES; i++)
		outsize += patch_data[i].new_size;

	ui_log_queue ("Workdir: %s\n", (char *)workdir);

	/* delete our temp files from possible previous runs */
	for (i = 0; i < NUM_PATCHES; i++)
	{
		snprintf (out, sizeof(out), "%s/%s/%s", (char *)workdir,
						patch_data[i].dir_name,
							 patch_tmpname);
		if (access(out, F_OK) == 0)
		{
			remove (out);
		}
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
		if (stat_and_fix_perms(dst, &stbuf) != 0)
		{
			rc |= XPATCH_FAIL;
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

		if (stbuf.st_size != patch_data[i].old_size &&
			stbuf.st_size != patch_data[i].new_size)
		{
			rc |= XPATCH_FAIL;
			ui_log_queue ("... is an incompatible version!\n");
			thread_alive = 0;
			return &rc;
		}

		ui_log_queue ("... checksumming...\n");
		csum = MD5File(dst, NULL);
		if (csum == NULL)
		{
			free (csum);
			rc |= XPATCH_FAIL;
			ui_log_queue ("... md5_compute() failed!\n");
			thread_alive = 0;
			return &rc;
		}
		if (strcmp(csum, patch_data[i].new_md5) == 0)
		{
			free (csum);
			written_size += patch_data[i].new_size;
			ui_log_queue ("... already patched.\n");
			continue;
		}
		if (strcmp(csum, patch_data[i].old_md5) != 0)
		{
			free (csum);
			rc |= XPATCH_FAIL;
			ui_log_queue ("... is an incompatible version!\n");
			thread_alive = 0;
			return &rc;
		}

		free (csum);

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
		time (&temptime);
		if (loki_xpatch(pat, dst, out) < 0)
		{
			rc |= XPATCH_FAIL;
			if (access(out, F_OK) == 0)
			{
				remove (out);
			}
			ui_log_queue ("... patch failed!\n");
			thread_alive = 0;
			return &rc;
		}
		elapsed = time (NULL) - temptime;
		ui_log_queue ("... elapsed time %lum:%lus\n",
						elapsed / 60, elapsed % 60);

		ui_log_queue ("... verifying checksum...\n");
		csum = MD5File(out, NULL);
		if (csum == NULL)
		{
			free (csum);
			rc |= XPATCH_FAIL;
			ui_log_queue ("... md5_compute() failed!\n");
			thread_alive = 0;
			return &rc;
		}
		if (strcmp(csum, patch_data[i].new_md5) != 0)
		{
			free (csum);
			rc |= XPATCH_FAIL;
			remove (out);
			ui_log_queue ("... checksum after patching failed!\n");
			thread_alive = 0;
			return &rc;
		}

		free (csum);

		remove (dst);	/* not all implementations overwrite existing files */
		if (rename(out, dst) < 0)
		{
			rc |= XPATCH_FAIL;
			remove (out);
			ui_log_queue ("... failed renaming patched file!\n");
			thread_alive = 0;
			return &rc;
		}

		rc |= XPATCH_APPLIED;
		ui_log_queue ("... Patch successful.\n");
	}

	if (rc & XPATCH_APPLIED)
	{
		elapsed = time (NULL) - starttime;
		ui_log_queue ("All patches successful in %lum:%lus.\n",
						elapsed / 60, elapsed % 60);
	}

	thread_alive = 0;
	return &rc;
}

