#include "com_sys.h"
#include "launch_bin.h"

extern char *bin_dir;
extern int mp_support;
#ifndef DEMOBUILD
extern int with_om;
#endif
extern int opengl_support;
extern int fullscreen;
extern int resolution;
extern int midi;
extern int cdaudio;
extern int sound;
extern int joystick;
extern int lan;
extern int destiny;
extern int hwgame;

static char *binary_name = NULL;
unsigned missingexe = 0;

const char *snddrv_names[MAX_SOUND][2]={
	{ "-nosound", "No Sound"},
	{ "-sndoss" , "OSS"	},	// just a placeholder, it is default actually
	{ "-sndsdl" , "SDL"	},
	{ "-sndalsa", "ALSA"	},
};

const char *hwgame_names[MAX_HWGAMES][2]={
	{  NULL     , "DeathMatch"},
	{ "hexarena", "HexArena"},
	{ "hwctf"   , "Capture the Flag"},
	{ "siege"   , "Siege"},
	{ "db"      , "Dungeon Break"},
	{ "rk"      , "Rival Kingdoms"},
};

pid_t pid=0;

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
  char *args[12];

  args[i]=binary_name;	// i == 0

  i++;			// i == 1
  args[i]="-f";		// args[i]="--fullscreen";
  if (fullscreen == 0)
    args[i]="-w";	// args[i]="--windowed";

  i++;			// i == 2
  args[i]="-width";

  i++;			// i == 3
  args[i]=resolution_args[resolution];

  i++;
  args[i]=(char *)snddrv_names[sound][0];

  if (sound != 0) {
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

  if (joystick == 0) {
    i++;
    args[i]="-nojoy";
  }

  if ((lan == 0) && (destiny != DEST_HW)) {
    i++;
    args[i]="-nolan";
  }

#ifndef DEMOBUILD
  if (mp_support && (destiny == DEST_H2)) {
    if (with_om)  {
       i++;
       args[i]="-witholdmission";
    } else {
       i++;
       args[i]="-noold";
    }
  }
#endif

  if ((destiny == DEST_HW) && (hwgame > 0)) {
       i++;
       args[i]="-game";
       i++;
       args[i]=(char *)hwgame_names[hwgame][0];
  }

  i++;
  args[i]=NULL;

  printf("\nLaunching %s\n",binary_name);
  printf("Command line is :\n  ");
  for (i1 = 0; i1 <= i - 1; i1++)
    printf(" %s", args[i1]);
  printf("\n\n");

  gtk_main_quit();
  printf ("now execv()ing %s\n", binary_name);
  execv(binary_name, args);
}
