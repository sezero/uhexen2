/*
	launch_bin.c
	hexen2 launcher: binary launching

	$Id: launch_bin.c,v 1.44 2007-05-31 21:27:26 sezero Exp $

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
#include "launcher_defs.h"
#include "games.h"
#include "config_file.h"

static char binary_name[16];

char *snddrv_names[MAX_SOUND][2] =
{
	{ "-nosound", "No Sound"},
#if HAVE_OSS_SOUND
	{ "-sndoss" , "OSS"	},
#endif
#if HAVE_SUN_SOUND
	{ "-sndoss" , "SUN"	},
#endif
	{ "-sndsdl" , "SDL"	},
#if HAVE_ALSA_SOUND
	{ "-sndalsa", "ALSA"	},
#endif
};

char *snd_rates[MAX_RATES] =
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

static char *resolution_args[RES_MAX][2] =
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
	int		i;
	char	*args[40];
	char	aasamples_str[8], heapsize_str[8], zonesize_str[8];

	binary_name[0] = '\0';
	if (opengl_support)
		strcpy (binary_name, "gl");
	if (destiny == DEST_HW)
		strcat (binary_name, HW_BINARY_NAME);
	else
		strcat (binary_name, H2_BINARY_NAME);

	i = 0;
	args[i] = binary_name;

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

	args[++i] = snddrv_names[sound][0];

	if (sound != 0)
	{
		if (sndrate != 0)
		{
			args[++i] = "-sndspeed";
			args[++i] = snd_rates[sndrate];
		}
		if (sndbits == 0)	// 16-bit is default already
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
		// engine doesn't -nocdaudio upon -nosound,
		// but it simply is what the name implies.
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
		// bot matches require -listen
		if (h2game_names[h2game].is_botmatch)
		{
			args[++i] = "-listen";
			lan = 1;	// -listen cannot work with -nolan
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
		snprintf (aasamples_str, 8, "%i", aasamples);
		args[++i] = aasamples_str;
	}

	if (opengl_support && vsync)
		args[++i] = "-vsync";

	if (opengl_support && use_lm1 == 1)	// -lm_4 is default already
		args[++i] = "-lm_1";

	if (gl_nonstd && opengl_support && strlen(gllibrary))
	{
		args[++i] = "-g";
		args[++i] = gllibrary;
	}

	if (use_heap && heapsize >= HEAP_MINSIZE)
	{
		args[++i] = "-heapsize";
		snprintf (heapsize_str, 8, "%i", heapsize);
		args[++i] = heapsize_str;
	}

	if (use_zone && zonesize >= ZONE_MINSIZE)
	{
		args[++i] = "-zone";
		snprintf (zonesize_str, 8, "%i", zonesize);
		args[++i] = zonesize_str;
	}

	if (debug2)
		args[++i] = "-devlog";
	else if (debug)
		args[++i] = "-condebug";

	// finish the list of args
	args[++i] = NULL;

	gtk_main_quit();

	printf ("\nLaunching %s\n", binary_name);
	printf ("Command line is :\n  ");
	i = 0;
	while (args[i])
	{
		printf (" %s", args[i]);
		i++;
	}
	printf ("\n\n");

	execv (binary_name, args);
}

