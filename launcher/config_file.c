#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <errno.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "launch_bin.h"

#ifndef DEMOBUILD
#define AOT_USERDIR ".aot"
#else
#define AOT_USERDIR ".hexen2demo"
#endif
#define LAUNCHER_CONFIG_FILE "launcher_options"

#ifndef DEMOBUILD
int mp_support;
int with_om;
//int iamevil;
#endif
int opengl_support;
int fullscreen;
int resolution;
int midi;
int cdaudio;
int sound;
int lan;
int joystick;
int destiny;

void Sys_mkdir (char *path)
{
	mkdir (path, 0777);
}

int Sys_GetUserdir(char *buff, unsigned int len)
{
    struct passwd *pwent;

    pwent = getpwuid( getuid() );
    if ( pwent == NULL ) {
	perror( "getpwuid" );
	return 0;
    }

    if ( strlen( pwent->pw_dir ) + strlen( AOT_USERDIR) + 2 > (unsigned)len ) {
	return 0;
    }

    sprintf( buff, "%s/%s", pwent->pw_dir, AOT_USERDIR );
    Sys_mkdir(buff);

    return 1;
}

FILE * open_config_file(char *flags)
{
#define MAX_PATH 1024

  FILE *thefile;
  char *config_file_name =0;
  config_file_name = (char *)calloc(MAX_PATH, sizeof(char));

  if (Sys_GetUserdir(config_file_name, MAX_PATH) == 0) {
    return NULL;
  }

  strcat (config_file_name,"/");
  strcat (config_file_name,LAUNCHER_CONFIG_FILE);

  /*if (*flags == 'r')
    printf("Opening configuration file for reading: %s\n",config_file_name);
  else if (*flags == 'w')
    printf("Opening configuration file for writing: %s\n",config_file_name);
  else 
    printf("Opening configuration file with unknown flag: %s\n",config_file_name);
  */
  thefile = fopen(config_file_name, flags);

  // NULL check has to be done later
  return thefile;
}


void fill_default_options()
{
#ifndef DEMOBUILD
  mp_support=0;
  with_om=0;
#endif
  opengl_support=1;
  fullscreen=1;
  resolution=RES_640;
  sound=1;
  midi=1;
  cdaudio=1;
  joystick=0;
  lan=1;
}

int write_config_file()
{
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
    //printf("Config file wrote successfully\n");

    return 0;    
}


int read_config_file()
{
  FILE *cfg_file;
  char buff[512];

  cfg_file = open_config_file("r");
  if (cfg_file == NULL) {
//  printf("file does not exist.\n");
//  printf(" Creating default configuration file... ");
    write_config_file();
//  printf("done!\n");
    return 0;

  } else {
    do {
      fgets(buff, sizeof(buff), cfg_file);
      if (!feof(cfg_file))
	{
	  buff[strlen(buff)-1] = '\0';

	   if (strstr(buff, "destiny=") == buff)
	    {
	      destiny = atoi(buff + 8);
	      if (destiny != 0 || destiny != 1 )
		destiny = DEST_H2;
	    }
#ifndef DEMOBUILD
	  else if (strstr(buff, "mp_support=") == buff)
	    {
	      mp_support = atoi(buff + 11);
	      if (mp_support != 0 || mp_support != 1 )
		mp_support = 0;
	    }

	  else if (strstr(buff, "with_om=") == buff)
	    {
	      with_om = atoi(buff + 8);
	      if (with_om != 0 || with_om != 1 )
		with_om = 0;
	    }
#endif
	  else if (strstr(buff, "opengl_support=") == buff)
	    {
	      opengl_support = atoi(buff + 15);
	      if (opengl_support != 0 || opengl_support != 1 )
		opengl_support = 1;
	    }

	  else if (strstr(buff, "fullscreen=") == buff)
	    {
	      fullscreen = atoi(buff + 11);
	      if (fullscreen != 0 || fullscreen != 1 )
		fullscreen = 1;
	    }
	  else if (strstr(buff, "resolution=") == buff)
	    {
	      resolution = atoi(buff + 11);
	      if (resolution < 0 || resolution > 6 )
		resolution = RES_640;
	    }
	  else if (strstr(buff, "sound=") == buff)
	    {
	      sound = atoi(buff + 6);
	      if (sound != 0 || sound != 1 )
		sound = 1;
	    }
	  else if (strstr(buff, "midi=") == buff)
	    {
	      if (sound == 0) {
	        midi = 0;
	      } else {
		midi = atoi(buff + 5);
		if (midi != 0 || midi != 1 )
		   midi = 1;
	      }
	    }
	  else if (strstr(buff, "cdaudio=") == buff)
	    {
	      if (sound == 0) {
		 cdaudio = 0;
	      } else {
		cdaudio = atoi(buff + 8);
		if (cdaudio != 0 || cdaudio != 1 )
		   cdaudio = 1;
	      }
	    }
	  else if (strstr(buff, "joystick=") == buff)
	    {
	      joystick = atoi(buff + 9);
	      if (joystick != 0 || joystick != 1 )
		joystick = 0;
	    }
	  else if (strstr(buff, "lan=") == buff)
	    {
	      lan = atoi(buff + 4);
	      if (lan != 0 || lan != 1 )
		lan = 1;
	    }
	}
    }

    while(!feof(cfg_file));

    fclose (cfg_file);
    write_config_file();

  }

  return 0;
}
