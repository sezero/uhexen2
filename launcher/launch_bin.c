/*
 * launch_bin.c
 * hexen2 launcher: binary launching
 *
 * $Id$
 *
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
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
#include "launcher_defs.h"
#include "games.h"
#include "config_file.h"
#include "launcher_ui.h"
#include "q_ctype.h"

#define AASAMPLES_CHARS	(4)	/* max digits for the aasamples value + 1 (nul) */
#define HEAPSIZE_CHARS	(8)	/* max digits for the heapsize value  + 1 (nul) */
#define ZONESIZE_CHARS	(8)	/* max digits for the zonesize value  + 1 (nul) */
#define STRING_BUFSIZE	(BINNAME_CHARS + AASAMPLES_CHARS + HEAPSIZE_CHARS + ZONESIZE_CHARS + MAX_EXTARGS)
static char string_buf[STRING_BUFSIZE];	/* holds the binary name to exec, first */

launcher_snddrv_t snd_drivers[] =
{
	{ SNDDRV_DEFAULT, ""/*none*/, "Default"	},
	{ SNDDRV_ID_NULL, "-nosound", "No Sound"},
	{ SNDDRV_ID_SDL,  "-sndsdl" , "SDL"	},
#if HAVE_OSS_SOUND
	{ SNDDRV_ID_OSS,  "-sndoss" , "OSS"	},
#endif
#if HAVE_SUN_SOUND
	{ SNDDRV_ID_SUN,  "-sndsun" , "SunAudio"},
#endif
#if HAVE_ALSA_SOUND
	{ SNDDRV_ID_ALSA, "-sndalsa", "ALSA"	},
#endif
	{ INT_MIN, NULL, NULL			}
};

const char *snd_rates[MAX_RATES] =
{
	"Default",
	"11025",
	"16000",
	"22050",
	"24000",
	"44100",
	"48000",
	 "8000"
};

static const char *resolution_args[RES_MAX][2] =
{
	{ "320",  "240" },
	{ "400",  "300" },
	{ "512",  "384" },
	{ "640",  "480" },
	{ "800",  "600" },
	{ "1024", "768" },
	{ "1280", "1024"},
	{ "1600", "1200"}
};


void launch_hexen2_bin (void)
{
	const char	*args[MAX_ARGS];
	char		*ptr;
	size_t		i, k;

	memset (string_buf, 0, STRING_BUFSIZE);
	ptr = &string_buf[0];

/* add the binary name first: */
	i = 0;
	if (opengl_support)
		strcpy (string_buf, BIN_OGL_PREFIX);
	if (destiny == DEST_HW)
		strcat (string_buf, HW_BINARY_NAME);
	else
		strcat (string_buf, H2_BINARY_NAME);
	string_buf[BINNAME_CHARS - 1] = '\0';
	args[i] = ptr;
	while (*ptr)
		ptr++;

	if (basedir_nonstd && game_basedir[0])
	{
		args[++i] = "-basedir";
		args[++i] = game_basedir;
	}

#if !defined(DEMOBUILD)
	if (destiny == DEST_H2 && mp_support)
		args[++i] = "-portals";
#endif	/* DEMOBUILD */

	args[++i] = (fullscreen) ? "-f" : "-w";

	args[++i] = "-width";
	args[++i] = resolution_args[resolution][0];
	args[++i] = "-height";
	args[++i] = resolution_args[resolution][1];

	if (opengl_support && use_con && conwidth < resolution)
	{
		args[++i] = "-conwidth";
		args[++i] = resolution_args[conwidth][0];
	}

	for (k = 1; snd_drivers[k].id != INT_MIN; k++)
	{
		if (sound == snd_drivers[k].id)
		{
			args[++i] = snd_drivers[k].cmd;
			break;
		}
	}

	if (sound != SNDDRV_ID_NULL)
	{
		if (sndrate != 0)
		{
			args[++i] = "-sndspeed";
			args[++i] = snd_rates[sndrate];
		}
		if (sndbits == 0)	/* 16-bit is default already */
		{
			args[++i] = "-sndbits";
			args[++i] = "8";
		}
		if (midi == 0)
			args[++i] = "-nomidi";
		if (cdaudio == 0)
			args[++i] = "-nocdaudio";
	}
	else
	{
	/* engine doesn't -nocdaudio upon -nosound,
	   but it simply is what the name implies */
		args[++i] = "-nocdaudio";
	}

#if !defined(DEMOBUILD)
	if (destiny == DEST_HW && hwgame > 0)
	{
		args[++i] = "-game";
		args[++i] = hwgame_names[hwgame].dirname;
	}
	else if (destiny == DEST_H2 && h2game > 0)
	{
		args[++i] = "-game";
		args[++i] = h2game_names[h2game].dirname;
		/* bot matches require -listen */
		if (h2game_names[h2game].is_botmatch)
		{
			args[++i] = "-listen";
			lan = 1;	/* -listen can't work with -nolan */
		}
	}
#endif	/* DEMOBUILD */

	if (lan == 0 && destiny == DEST_H2)
		args[++i] = "-nolan";

	if (!mouse)
		args[++i] = "-nomouse";

	if (opengl_support && fxgamma)
		args[++i] = "-3dfxgamma";

	if (opengl_support && is8bit)
		args[++i] = "-paltex";

	if (opengl_support && use_fsaa && aasamples)
	{
		args[++i] = "-fsaa";
		snprintf (++ptr, AASAMPLES_CHARS, "%i", aasamples);
		args[++i] = ptr;
		while (*ptr)
			ptr++;
	}

	if (opengl_support && vsync)
		args[++i] = "-vsync";

	if (opengl_support && use_lm1 == 1)	/* -lm_4 is default already */
		args[++i] = "-lm_1";

	if (gl_nonstd && opengl_support && gllibrary[0])
	{
		args[++i] = "-g";
		args[++i] = gllibrary;
	}

	if (use_heap && heapsize >= HEAP_MINSIZE)
	{
		args[++i] = "-heapsize";
		snprintf (++ptr, HEAPSIZE_CHARS, "%i", heapsize);
		args[++i] = ptr;
		while (*ptr)
			ptr++;
	}

	if (use_zone && zonesize >= ZONE_MINSIZE)
	{
		args[++i] = "-zone";
		snprintf (++ptr, ZONESIZE_CHARS, "%i", zonesize);
		args[++i] = ptr;
		while (*ptr)
			ptr++;
	}

	if (debug2)
		args[++i] = "-devlog";
	else if (debug)
		args[++i] = "-condebug";

/* parse the extra user arguments */
	if (use_extra && ext_args[0])
	{
		memcpy (++ptr, ext_args, MAX_EXTARGS - 1);
		string_buf[STRING_BUFSIZE - 1] = '\0';

		while (1)
		{
			if (!*ptr || (i + 2 >= MAX_ARGS))
				break;
			while (*ptr && q_isspace(*ptr))
				*ptr++ = '\0';
			if (*ptr)
			{
				args[++i] = ptr;
				while (*ptr && !q_isspace(*ptr))
					ptr++;
			}
		}
	}

/* terminate the list of args */
	args[++i] = NULL;

	write_config_file ();
	ui_quit ();

	printf ("\nLaunching %s\n", &string_buf[0]);
	printf ("Command line is :\n  ");
	i = 0;
	while (args[i])
	{
		printf (" %s", args[i]);
		i++;
	}
	printf ("\n\n");

	execv (&string_buf[0], (char * const *) args);

	exit (0);
}

