#include "com_sys.h"
#include "launcher_defs.h"

#define LAUNCHER_CONFIG_FILE "launcher_options"

int destiny		= DEST_H2;
int hwgame		= 0;
int mp_support		= 0;
#ifndef DEMOBUILD
int with_om		= 0;
#endif
int opengl_support	= 1;
int fullscreen		= 1;
int resolution		= RES_640;
int sound		= 1;
int midi		= 1;
int cdaudio		= 1;
int joystick		= 0;
int lan			= 1;

extern char userdir[1024];

FILE * open_config_file(char *flags) {

#define MAX_PATH 1024

  FILE *thefile;
  char *config_file_name =0;
  config_file_name = (char *)calloc(MAX_PATH, sizeof(char));

  strcat (config_file_name,userdir);
  strcat (config_file_name,"/");
  strcat (config_file_name,LAUNCHER_CONFIG_FILE);
  thefile = fopen(config_file_name, flags);
  free (config_file_name);
// NULL check has to be done later
  return thefile;
}


int write_config_file() {

  FILE *cfg_file;
  cfg_file = open_config_file("w");
  if (cfg_file == NULL) {
    fprintf(stderr, " Error: couldn't open config file for writing\n");
    return 1;

  } else {

    fprintf(cfg_file, "# Linux Hexen2 Launcher Options file\n\n");
    fprintf(cfg_file, "# This file has been automatically generated\n\n");

    fprintf(cfg_file, "destiny=%d\n",destiny);
#ifndef DEMOBUILD
    fprintf(cfg_file, "hwgame=%d\n",hwgame);
    fprintf(cfg_file, "mp_support=%d\n",mp_support);
    fprintf(cfg_file, "with_om=%d\n",with_om);
#endif
    fprintf(cfg_file, "opengl_support=%d\n",opengl_support);
    fprintf(cfg_file, "fullscreen=%d\n",fullscreen);
    fprintf(cfg_file, "resolution=%d\n",resolution);
    fprintf(cfg_file, "sound=%d\n",sound);
    fprintf(cfg_file, "midi=%d\n",midi);
    fprintf(cfg_file, "cdaudio=%d\n",cdaudio);
    fprintf(cfg_file, "joystick=%d\n",joystick);
    fprintf(cfg_file, "lan=%d\n",lan);
  }
  fclose (cfg_file); 
  printf("Options saved successfully.\n");
  return 0;    
}

int read_config_file() {

  FILE *cfg_file;
  char buff[512];

  cfg_file = open_config_file("r");
  if (cfg_file == NULL) {
    printf("Creating default configuration file.....\n");
    write_config_file();
    return 0;

  } else {
    printf("Reading configuration file.... ");
    do {
      fgets(buff, sizeof(buff), cfg_file);
      if (!feof(cfg_file)) {

	  buff[strlen(buff)-1] = '\0';
	  if (strstr(buff, "destiny=") == buff) {
	      destiny = atoi(buff + 8);
	      if (destiny != DEST_H2 && destiny != DEST_HW )
		destiny = DEST_H2;
	  }
#ifndef DEMOBUILD
	  else if (strstr(buff, "hwgame=") == buff) {
	      hwgame = atoi(buff + 7);
	      if (hwgame < 0 || hwgame >= MAX_HWGAMES)
		hwgame = 0;
	  }
	  else if (strstr(buff, "mp_support=") == buff) {
	      mp_support = atoi(buff + 11);
	      if (mp_support != 0 && mp_support != 1 )
		mp_support = 0;
	  }
	  else if (strstr(buff, "with_om=") == buff) {
	      with_om = atoi(buff + 8);
	      if (with_om != 0 && with_om != 1 )
		with_om = 0;
	  }
#endif
	  else if (strstr(buff, "opengl_support=") == buff) {
	      opengl_support = atoi(buff + 15);
	      if (opengl_support != 0 && opengl_support != 1 )
		opengl_support = 1;
	  }
	  else if (strstr(buff, "fullscreen=") == buff) {
	      fullscreen = atoi(buff + 11);
	      if (fullscreen != 0 && fullscreen != 1 )
		fullscreen = 1;
	  }
	  else if (strstr(buff, "resolution=") == buff) {
	      resolution = atoi(buff + 11);
	      if (resolution < 0 || resolution > RES_MAX )
		resolution = RES_640;
	  }
	  else if (strstr(buff, "sound=") == buff) {
	      sound = atoi(buff + 6);
	      if (sound != 0 && sound != 1 )
		sound = 1;
	  }
	  else if (strstr(buff, "midi=") == buff) {
		midi = atoi(buff + 5);
		if (midi != 0 && midi != 1 )
		   midi = 1;
	  }
	  else if (strstr(buff, "cdaudio=") == buff) {
		cdaudio = atoi(buff + 8);
		if (cdaudio != 0 && cdaudio != 1 )
		   cdaudio = 1;
	  }
	  else if (strstr(buff, "joystick=") == buff) {
	      joystick = atoi(buff + 9);
	      if (joystick != 0 && joystick != 1 )
		joystick = 0;
	  }
	  else if (strstr(buff, "lan=") == buff) {
	      lan = atoi(buff + 4);
	      if (lan != 0 && lan != 1 )
		lan = 1;
	  }
      }

    } while(!feof(cfg_file));
    printf ("OK.\n");
    fclose (cfg_file);
  }

  return 0;
}
