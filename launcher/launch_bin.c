#include "common.h"
#include "launcher_defs.h"
#include "config_file.h"

static char binary_name[16];
int	missingexe = 0;

const char *snddrv_names[MAX_SOUND][2] = {

	{ "-nosound", "No Sound"},
	{ "-sndoss" , "OSS"	},
	{ "-sndsdl" , "SDL"	},
#if defined(__linux__)
	{ "-sndalsa", "ALSA"	},
#endif
};

const char *snd_rates[MAX_RATES] = {

	"Default",
	"11025",
	"22050",
	"44100",
	"48000",
	 "8000"
};

#ifndef DEMOBUILD
const char *h2game_names[MAX_H2GAMES][2] = {

	{  NULL     , "(  None  )"	},
	{ "hcbots"  , "BotMatch: Hcbot"	},
	{ "apocbot" , "BotMatch: Apoc"	},
};

const char *hwgame_names[MAX_HWGAMES][2] = {

	{  NULL     , "DeathMatch"	},
	{ "hexarena", "HexArena"	},
	{ "hwctf"   , "Capture the Flag"},
	{ "siege"   , "Siege"		},
	{ "db"      , "Dungeon Break"	},
	{ "rk"      , "Rival Kingdoms"	},
};
#endif

/* [resolution]
   -width values only. corresponding -height is in the game binary */
static char *resolution_args[] = {

	"320",
	"400",
	"512",
	"640",
	"800",
	"1024",
	"1280",
	"1600"
};

void CheckExe (void)
{
	binary_name[0] = '\0';

	if (opengl_support)
		sprintf (binary_name, "gl");

	if (destiny == DEST_H2)
		strcat (binary_name, H2_BINARY_NAME);
	else if (destiny == DEST_HW)
		strcat (binary_name, HW_BINARY_NAME);
	else
	{
		printf("Warning: unknown destiny choice. Choosing Hexen II\n");
		strcat (binary_name, H2_BINARY_NAME);
	}

	if (access(binary_name, X_OK) != 0)
	{
		missingexe = 1;
		printf ("game binary %s missing or not executable\n", binary_name);
	}
	else
	{
		missingexe = 0;
		printf ("game binary %s found OK\n", binary_name);
	}
	return;
}


void launch_hexen2_bin (void)
{
	int		i;
	char	*args[32];
	char	tmparg1[8], tmparg2[8], tmparg3[8];

	i = 0;
	args[i] = binary_name;

	if (destiny == DEST_H2 && mp_support)
		args[++i] = "-portals";

	if (fullscreen)
		args[++i] = "-fullscreen";
	else
		args[++i] = "-window";

	args[++i] = "-width";
	args[++i] = resolution_args[resolution];

	if (opengl_support && use_con && conwidth < resolution)
	{
		args[++i] = "-conwidth";
		args[++i] = resolution_args[conwidth];
	}

	args[++i] = (char *)snddrv_names[sound][0];

	if (sound != 0)
	{
		if (sndrate != 0)
		{
			args[++i] = "-sndspeed";
			args[++i] = (char *)snd_rates[sndrate];
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

#ifndef DEMOBUILD
	if ((destiny == DEST_HW) && (hwgame > 0))
	{
		args[++i] = "-game";
		args[++i] = (char *)hwgame_names[hwgame][0];
	}
	else if ((destiny != DEST_HW) && (h2game > 0) && !mp_support)
	{
	// we only provide botmatch thingies, so -listen is necessary
		args[++i] = "-listen";
		lan = 1;	// -listen cannot work with -nolan
		args[++i] = "-game";
		args[++i] = (char *)h2game_names[h2game][0];
	}
#endif

	if ((lan == 0) && (destiny != DEST_HW))
		args[++i] = "-nolan";

	if (!mouse)
		args[++i] = "-nomouse";

	if ((opengl_support) && (fxgamma))
		args[++i] = "-3dfxgamma";

	if ((opengl_support) && (is8bit))
		args[++i] = "-paltex";

	if ((opengl_support) && (use_fsaa) && (aasamples))
	{
		args[++i] = "-fsaa";
		snprintf (tmparg3, 8, "%i", aasamples);
		args[++i] = tmparg3;
	}

	if ((opengl_support) && (vsync))
		args[++i] = "-vsync";

	if ((opengl_support) && (use_lm1 == 1)) // -lm_4 is default already
		args[++i] = "-lm_1";

	if ((opengl_support) && (gl_nonstd) && (strlen(gllibrary) != 0))
	{
		args[++i] = "--gllibrary";
		args[++i] = gllibrary;
	}

	if ((use_heap) && (heapsize >= HEAP_MINSIZE))
	{
		args[++i] = "-heapsize";
		snprintf (tmparg1, 8, "%i", heapsize);
		args[++i] = tmparg1;
	}

	if ((use_zone) && (zonesize >= ZONE_MINSIZE))
	{
		args[++i] = "-zone";
		snprintf (tmparg2, 8, "%i", zonesize);
		args[++i] = tmparg2;
	}

	if (debug)
		args[++i] = "-condebug";

	// finish the list of args
	args[++i] = NULL;

	gtk_main_quit();

	printf ("\nLaunching %s\n",binary_name);
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

