#include "com_sys.h"
#include "launch_bin.h"

extern char *bin_dir;
extern int mp_support;
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int sndrate;
extern int sndbits;
extern int stereo;
extern int lan;
extern int destiny;
#ifndef DEMOBUILD
extern int h2game;
extern int hwgame;
#endif

static char *binary_name = NULL;
unsigned missingexe = 0;

const char *snddrv_names[MAX_SOUND][2]={
	{ "-nosound", "No Sound"},
	{ "-sndoss" , "OSS"	},	// just a placeholder, it is default actually
	{ "-sndsdl" , "SDL"	},
	{ "-sndalsa", "ALSA"	},
};

const char *snd_rates[MAX_RATES]={
	"Default",
	"11025",
	"22050",
	"44100",
	"48000",
	 "8000"
};

#ifndef DEMOBUILD
const char *h2game_names[MAX_H2GAMES][2]={
	{  NULL     , "(  None  )"	},
	{ "hcbots"  , "BotMatch: Hcbot"	},
	{ "apocbot" , "BotMatch: Apoc"	},
};

const char *hwgame_names[MAX_HWGAMES][2]={
	{  NULL     , "DeathMatch"	},
	{ "hexarena", "HexArena"	},
	{ "hwctf"   , "Capture the Flag"},
	{ "siege"   , "Siege"		},
	{ "db"      , "Dungeon Break"	},
	{ "rk"      , "Rival Kingdoms"	},
};
#endif

void CheckExe () {

	if (destiny == DEST_H2)
		binary_name=h2_binary_names[table[opengl_support][mp_support]];
	else if (destiny == DEST_HW)
		binary_name=hw_binary_names[opengl_support];
	else {
		printf("Warning: unknown destiny choice. Choosing Hexen II\n");
		binary_name=h2_binary_names[table[opengl_support][mp_support]];
	}

	if (access(binary_name, X_OK) != 0) {
		missingexe = 1;
		printf ("game binary %s missing or not executable\n", binary_name);
	} else {
		missingexe = 0;
		printf ("game binary %s found OK\n", binary_name);
	}
	return;
}


void launch_hexen2_bin() {

	unsigned short i=0, i1=0;
	char *args[20];

	args[i]=binary_name;

	i++;
	args[i] = (fullscreen == 1) ? "-fullscreen" : "-window";

	i++;
	args[i]="-width";
	i++;
	args[i]=resolution_args[resolution];

	i++;
	args[i]=(char *)snddrv_names[sound][0];

	if (sound != 0) {
		if (sndrate != 0) {
			i++;
			args[i]="-sndspeed";
			i++;
			args[i]=(char *)snd_rates[sndrate];
		}
		if (sndbits == 0) {	// 16-bit is default already
			i++;
			args[i]="-sndbits";
			i++;
			args[i]="8";
		}
		if (stereo == 0) {
			i++;
			args[i]="-sndmono";
		}
		if (midi == 0) {
			i++;
			args[i]="-nomidi";
		}
		if (cdaudio == 0) {
			i++;
			args[i]="-nocdaudio";
		}
	} else {
		i++;
		// engine doesn't -nocdaudio upon -nosound,
		// but it simply is what the name implies.
		args[i]="-nocdaudio";
	}

#ifndef DEMOBUILD
	if ((destiny == DEST_HW) && (hwgame > 0)) {
		i++;
		args[i]="-game";
		i++;
		args[i]=(char *)hwgame_names[hwgame][0];
	}
	else if ((destiny != DEST_HW) && (h2game > 0) && !mp_support) {
	// we only provide botmatch thingies, so -listen is necessary
		i++;
		args[i]="-listen";
		lan=1;	// -listen cannot work with -nolan
		i++;
		args[i]="-game";
		i++;
		args[i]=(char *)h2game_names[h2game][0];
	}
#endif

	if ((lan == 0) && (destiny != DEST_HW)) {
		i++;
 		args[i]="-nolan";
	}

	i++;
	args[i]=NULL;

	gtk_main_quit();

	printf("\nLaunching %s\n",binary_name);
	printf("Command line is :\n  ");
	for (i1 = 0; i1 <= i - 1; i1++)
		printf(" %s", args[i1]);
	printf("\n\n");

	execv(binary_name, args);
}

