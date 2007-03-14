/*
	config_file.h
	hexen2 launcher config file handling

	$Id: config_file.h,v 1.10 2007-03-14 21:04:25 sezero Exp $
*/

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#define LAUNCHER_CONFIG_FILE "launcher_options"

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
extern char gllibrary[MAX_OSPATH];
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

int write_config_file (void);
int read_config_file (void);

#endif	// CONFIG_FILE_H

