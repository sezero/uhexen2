#include "common.h"
#include "launch_bin.h"

extern int destiny;
extern int mp_support;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int conwidth;
extern int use_con;
extern int fxgamma;
extern int is8bit;
extern int use_fsaa;
extern int aasamples;
extern int vsync;
extern int use_lm1;
extern int gl_nonstd;
extern char gllibrary[256];
extern int midi;
extern int cdaudio;
extern int sound;
extern int sndrate;
extern int sndbits;
extern int lan;
extern int mouse;
extern int debug;
extern int use_heap;
extern int use_zone;
extern int heapsize;
extern int zonesize;
#ifndef DEMOBUILD
extern int h2game;
extern int hwgame;
#endif

static char *binary_name = NULL;
int	missingexe = 0;

const char *snddrv_names[MAX_SOUND][2] = {

	{ "-nosound", "No Sound"},
	{ "-sndoss" , "OSS"	},	// just a placeholder, it is default actually
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

void CheckExe (void)
{
	if (destiny == DEST_H2)
		binary_name=h2_binary_names[table[opengl_support][mp_support]];
	else if (destiny == DEST_HW)
		binary_name=hw_binary_names[opengl_support];
	else
	{
		printf("Warning: unknown destiny choice. Choosing Hexen II\n");
		binary_name=h2_binary_names[table[opengl_support][mp_support]];
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

